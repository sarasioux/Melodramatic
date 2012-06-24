<?php
// $Id: bitcache.inc,v 1.16 2009/03/18 06:24:00 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// Constants

@define('BITCACHE_ID_FORMAT', '/^[a-z0-9]{40}$/');
@define('BITCACHE_LAST_MODIFIED', TRUE);
@define('BITCACHE_POST_MAX_SIZE', (int)substr(ini_get('post_max_size'), 0, -1) * pow(2, array_search(strtolower(substr(ini_get('post_max_size'), -1)), array(1 => '', 10 => 'k', 20 => 'm', 30 => 'g'))));

//////////////////////////////////////////////////////////////////////////////
// Bitstream implementation

/**
 * Implements a wrapper for accessing individual bitstreams.
 */
class Bitcache_Stream {
  public $id, $data, $size, $type;
  public $compressed = FALSE;
  public $encrypted  = FALSE;

  function __construct($id, $data, array $options = array()) {
    $this->id   = $id;
    $this->data = $data;
    foreach ($options as $k => $v) {
      $this->$k = $v;
    }
    $this->stat();
  }

  /**
   * Permits access to the bitstream's contents.
   *
   * @param  string $mode    the access type
   * @see    fopen()
   */
  function open($mode = 'rb') {
    return $this->stream = NULL; // TODO
  }

  /**
   * Closes access to the bitstream's contents.
   *
   * @see    fclose()
   */
  function close() {
    if (!empty($this->stream)) {
      $result = FALSE; // TODO
      unset($this->stream);
      return $result;
    }
  }

  function path() {
    // TODO: register 'bitcache://' stream wrapper
  }

  /**
   * Returns the bitstream's fingerprint.
   */
  function id() {
    return $this->id;
  }

  /**
   * Returns the bitstream's contents.
   *
   * @see    file_get_contents()
   */
  function data() {
    return $this->data;
  }

  /**
   * Makes more information about the bitstream available.
   */
  function stat() {
    if (is_null($this->size)) {
      $this->size = $this->size();
    }
    if (is_null($this->type)) {
      $this->type = $this->type();
    }
  }

  /**
   * Returns the bitstream's byte length.
   */
  function size() {
    return !is_null($this->size) ? $this->size : $this->size = strlen($this->data);
  }

  /**
   * Attempts to determine the bitstream's MIME content type.
   *
   * @see    finfo_buffer()
   */
  function type() {
    if (extension_loaded('fileinfo') && function_exists('finfo_buffer') && ($finfo = finfo_open(FILEINFO_MIME))) {
      if (($type = finfo_buffer($finfo, $this->data()))) { // @since PHP 5.3.0, fileinfo 0.1.0
        finfo_close($finfo);
        return $type;
      }
    }

    // Attempts to detect a file's MIME type using the Unix `file' utility.
    // MIME content type format defined in http://www.ietf.org/rfc/rfc1521.txt
    if (($proc = proc_open('file -bi -', array(0 => array('pipe', 'r'), 1 => array('pipe', 'w')), $pipes))) {
      list($stdin, $stdout, $stderr) = $pipes;
      fwrite($stdin, $this->data());
      fclose($stdin);
      $output = stream_get_contents($stdout);
      fclose($stdout);
      if (preg_match('!([\w-]+/[\w\d_+-]+)!', $output, $matches)) {
        return $matches[1];
      }
    }

    return 'application/octet-stream'; // default to binary
  }
}

//////////////////////////////////////////////////////////////////////////////
// Repository implementation

/**
 * Specifies the API contract for repository-like objects.
 */
abstract class Bitcache_Repository implements Countable {
  public $name, $uri, $options;

  function create() {}
  function rename($old_name, $new_name, array $options = array()) {}
  function destroy() {}

  function open() {}
  function close() {}

  function exists($id) {
    return FALSE;
  }

  abstract function get($id);
  abstract function put($id, $data);

  function put_file($id, $filepath, $move = FALSE) {
    if (($id = $this->put($id, file_get_contents($filepath))) && $move) {
      unlink($filepath);
    }
    return $id;
  }

  abstract function delete($id);

  function count() {
    return iterator_count($this);
  }

  function size() {
    $size = 0;
    foreach ($this as $id => $stream) {
      $size += $stream->size();
    }
    return $size;
  }

  protected function created($id, $stream = NULL) {
    if (function_exists('module_invoke_all')) {
      module_invoke_all('bitcache', 'insert', $id, $stream); // Drupal-specific
    }
  }

  protected function deleted($id) {
    if (function_exists('module_invoke_all')) {
      module_invoke_all('bitcache', 'delete', $id); // Drupal-specific
    }
  }
}

/**
 * Implements an aggregating multi-repository proxy.
 */
class Bitcache_AggregateRepository extends Bitcache_Repository implements IteratorAggregate {
  public $repos;

  function __construct(array $repos, array $options = array()) {
    if (empty($repos)) {
      trigger_error('Must be given at least one repository object', E_USER_ERROR);
    }
    $this->repos = $repos;
  }

  /**
   * Returns the sum total of the repositories' total byte sizes.
   */
  function size() {
    $total = 0;
    foreach ($this->repos as $repo) {
      $total += $repo->size();
    }
    return $total;
  }

  /**
   * Returns the sum total of the repositories' bitstream counts.
   *
   * @see    Countable
   */
  function count() {
    return iterator_count($this->getIterator());
  }

  /**
   * Returns an AppendIterator that provides seamless iteration over all
   * bitstreams in all constituent repositories, one after the other.
   *
   * @see    IteratorAggregate
   */
  function getIterator() {
    $iterator = new AppendIterator();
    foreach ($this->repos as $repo) {
      $iterator->append($repo);
    }
    return $iterator;
  }

  /**
   * Checks for the specified bitstream in any and all repositories.
   *
   * @param  string $id      the bitstream identifier
   */
  function exists($id) {
    return $this->op('exists', FALSE, $id);
  }

  /**
   * Returns the specified bitstream from the first repository that has it
   * and from which it is actually retrievable (i.e. no error occurs).
   *
   * @param  string $id      the bitstream identifier
   * @see    Bitcache_Stream
   */
  function get($id) {
    return $this->op('get', array(FALSE, NULL), $id);
  }

  /**
   * Stores the new bitstream into the first repository that accepts it.
   *
   * @param  string $id      the bitstream identifier
   */
  function put($id, $data) {
    return $this->op('put', FALSE, !$id ? sha1($data) : $id, $data);
  }

  /**
   * Stores the new bitstream into the first repository that accepts it.
   *
   * @param  string $id      the bitstream identifier
   */
  function put_file($id, $filepath, $move = FALSE) {
    return $this->op('put_file', FALSE, !$id ? sha1_file($filepath) : $id, $filepath, $move);
  }

  /**
   * Deletes the specified bitstream from any and all repositories.
   *
   * @param  string $id      the bitstream identifier
   */
  function delete($id) {
    return $this->op('delete', array(TRUE, FALSE, NULL), $id);
  }

  /**
   * Performs the same API operation on all constituent repositories until a
   * stop condition (i.e. an affirmative result) is reached.
   */
  protected function op($op, $continue_while = NULL) {
    $args = array_slice(func_get_args(), 2);
    $continue_while = is_array($continue_while) ? $continue_while : array($continue_while);

    foreach ($this->repos as $repo) {
      $result = call_user_func_array(array($repo, $op), $args);
      if (!in_array($result, $continue_while, TRUE)) {
        break;
      }
    }
    return $result;
  }
}

//////////////////////////////////////////////////////////////////////////////
// Server implementation

/**
 * Implements a Bitcache REST API-compliant HTTP server.
 */
class Bitcache_Server {
  function __construct($repo, array $options = array()) {
    $this->repo    = is_object($repo) ? $repo : new Bitcache_FileRepository(array('location' => (string)$repo)); // FIXME
    $this->options = $options;
  }

  /**
   * Handles OPTIONS requests. Determines the set of valid HTTP methods for
   * the bitstream index or a bitstream resource.
   *
   * @param  string $request the request URI
   * @param  array  $query   associative array of query string arguments
   * @link   <http://www.w3.org/Protocols/rfc2616/rfc2616-sec9.html#sec9.2>
   */
  function options($request, array $query = array()) {
    $this->header('Content-Type: text/plain; charset=ascii');

    if (!($id = $this->resolve('OPTIONS', $request))) {
      // Options for the bitstream index
      $methods = array_filter(array('OPTIONS', 'GET', 'POST'), array($this, 'allow'));
    }
    else {
      // Options for a bitstream resource
      $methods = array('OPTIONS', 'GET', 'HEAD', 'PUT', 'DELETE');
      foreach ($methods as $key => $method) {
        if (!$this->allow($method, $id))
          unset($methods[$key]);
      }
    }
    $this->header('Accept: ' . implode(', ', $methods));

    return TRUE;
  }

  /**
   * Handles INDEX requests. Outputs the full index of available bitstreams.
   *
   * @param  string $request the request URI
   * @param  array  $query   associative array of query string arguments
   */
  function index($request, array $query = array(), $body = TRUE) {
    $this->header('Content-Type: text/plain; charset=ascii');

    if ($body) {
      $this->index_text();
    }

    return TRUE;
  }

  /**
   * Handles HEAD requests.
   *
   * @param  string $request the request URI
   * @param  array  $query   associative array of query string arguments
   * @link   <http://www.w3.org/Protocols/rfc2616/rfc2616-sec9.html#sec9.4>
   */
  function head($request, array $query = array()) {
    return $this->get($request, $query, FALSE);
  }

  /**
   * Handles GET requests.
   *
   * @param  string $request the request URI
   * @param  array  $query   associative array of query string arguments
   * @link   <http://www.w3.org/Protocols/rfc2616/rfc2616-sec9.html#sec9.3>
   */
  function get($request, array $query = array(), $body = TRUE) {
    if (!($id = $this->resolve('GET', $request))) {
      return $this->index($request, $query, $body);
    }

    // <http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html#sec10.4.5>
    if (!($stream = $this->repo->get($id))) {
      return $this->abort(404, 'Not Found');
    }

    // <http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html#sec10.5.4>
    if ((($data = $stream->open()) || ($data = $stream->data())) === NULL) {
      return $this->abort(503, 'Service Unavailable');
    }

    $this->headers($id, $stream);

    if ($body) {
      // <http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.13>
      $this->header('Content-Length: ' . $stream->size());

      if (is_string($data)) {
        print $data;
      }
      else { // stream resource
        $this->passthru($id, $stream);
      }
    }

    $stream->close();

    return TRUE;
  }

  /**
   * Handles POST requests.
   *
   * @param  string $request the request URI
   * @param  array  $query   associative array of query string arguments
   * @link   <http://www.w3.org/Protocols/rfc2616/rfc2616-sec9.html#sec9.5>
   */
  function post($request, array $query = array()) {
    // We only support POST requests to the bitstream index.
      // <http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html#sec10.4.1>
    if ($request != '/')
      return $this->abort(400, 'Bad Request');

    $this->upload(NULL);

    return TRUE;
  }

  /**
   * Handles PUT requests.
   *
   * @param  string $request the request URI
   * @param  array  $query   associative array of query string arguments
   * @link   <http://www.w3.org/Protocols/rfc2616/rfc2616-sec9.html#sec9.6>
   */
  function put($request, array $query = array()) {
    $this->upload($this->resolve('PUT', $request));

    return TRUE;
  }

  /**
   * Handles DELETE requests.
   *
   * @param  string $request the request URI
   * @param  array  $query   associative array of query string arguments
   * @link   <http://www.w3.org/Protocols/rfc2616/rfc2616-sec9.html#sec9.7>
   */
  function delete($request, array $query = array()) {
    $id = $this->resolve('DELETE', $request);

    if (!$this->repo->delete($id))
      return $this->abort(503, 'Service Unavailable');

    $this->deleted($id);

    return TRUE;
  }

  protected function resolve($method, $request) {
    if ($request[0] != '/')
      return $this->abort(400, 'Bad Request');

    $id = substr($request, 1); // strip off the leading '/'

    // We don't support requests for anything else except the root
    // collection '/' and for bitstreams identified by fingerprints of the
    // correct length and written in all lowercase hexadecimal letters
    // '/[a-z0-9]+'. Note that the reason for returning 400 Bad Request
    // instead of 404 Not Found is to differentiate actually invalid
    // requests from valid requests for an unknown bitstream.
    if (!empty($id) && !preg_match(BITCACHE_ID_FORMAT, $id))
      return $this->abort(400, 'Bad Request');

    if (!$this->allow($method, $id))
      return $this->abort(403, 'Forbidden');

    return $id;
  }

  protected function allow($method, $id = NULL) {
    return TRUE; // Can be overridden in subclasses
  }

  protected function upload($id = NULL) {
    // TODO: $_SERVER['CONTENT_TYPE'] != 'application/x-www-form-urlencoded'

    // <http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html#sec10.4.12>
    if (!isset($_SERVER['CONTENT_LENGTH']) || !ctype_digit($_SERVER['CONTENT_LENGTH']))
      return $this->abort(411, 'Length Required');

    // <http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html#sec10.4.14>
    if (BITCACHE_POST_MAX_SIZE && (int)$_SERVER['CONTENT_LENGTH'] > BITCACHE_POST_MAX_SIZE)
      return $this->abort(413, 'Request Entity Too Large');

    $tmpfile = $this->receive();
    $sha1 = sha1_file($tmpfile);

    // <http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html#sec10.4.10>
    // If the actual fingerprint does not match the fingerprint provided us
    // by the client's PUT request, we are dealing either with in-transit
    // data corruption or a buggy/malicious client. In any case, time to
    // call it quits.
    if (!empty($id) && $sha1 != $id)
      return $this->abort(409, 'Conflict');

    if (!$this->repo->put_file($sha1, $tmpfile, TRUE))
      return $this->abort(503, 'Service Unavailable');

    $this->created($sha1, $stream);

    return TRUE;
  }

  protected function created($id, $stream) {
    // <http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html#sec10.2.2>
    $this->status(201, 'Created');

    $this->header('Content-Type: application/octet-stream'); // FIXME?
    $this->header('ETag: "' . $id . '"');
    $this->redirect('/' . $id);
  }

  protected function deleted($id) {
    // <http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html#sec10.2.5>
    $this->status(204, 'No Content');
  }

  protected function abort($code, $msg = '') {
    $this->status($code, $msg);

    $this->header('Content-Type: text/plain; charset=ascii');
    die(trim("$code $msg") . "\n");
  }

  protected function redirect($path = '/') {
    $this->header('Location: ' . $path);
  }

  protected function status($code, $msg = '') {
    $this->header(trim("HTTP/1.1 $code $msg"));
  }

  protected function header($text, $replace = TRUE) {
    header($text, $replace);
  }

  protected function headers($id, $stream) {
    // <http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.35>
    //$this->header('Accept-Ranges: bytes'); // TODO: support partial downloads

    // <http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.11>
    // <http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.19>
    $this->header('ETag: "' . $id . '"');

    // <http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.15>
    // A Content-SHA1 header is not specified in HTTP/1.1, but it does
    // specify Content-MD5; this is a straightforward, if non-standard,
    // extension and modernization of that same concept.
    $this->header('Content-SHA1: ' . $id);

    // <http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.17>
    $this->header('Content-Type: ' . $stream->type());
  }

  protected function passthru($id, $stream) {
    fpassthru($stream->stream);
  }

  protected function receive() {
    $input   = fopen('php://input', 'rb');
    $tmpdir  = function_exists('sys_get_temp_dir') ? sys_get_temp_dir() : '/tmp';
    $tmpfile = tempnam($tmpdir, 'bitcache');
    $output  = fopen($tmpfile, 'wb');
    stream_copy_to_stream($input, $output);
    fclose($input);
    fclose($output);
    return $tmpfile;
  }

  protected function index_text() {
    foreach ($this->repo as $id => $stream) {
      printf("%s\t%d\n", $id, $stream->size());
    }
  }
}
