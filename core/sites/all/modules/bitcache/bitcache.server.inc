<?php
// $Id: bitcache.server.inc,v 1.14 2009/03/18 07:04:05 arto Exp $

//////////////////////////////////////////////////////////////////////////////

module_load_include('inc', 'bitcache');

//////////////////////////////////////////////////////////////////////////////
// Menu callbacks

function bitcache_server($id = NULL) {
  // The REST API must be explicitly enabled by the administrator:
  if (!BITCACHE_SERVER) {
    die(drupal_access_denied());
  }

  $method  = $_SERVER['REQUEST_METHOD'];
  $request = $_REQUEST['q']; // unlike $_GET['q'], aliases not expanded here
  $query   = array_diff_key($_GET, array('q' => NULL));

  // Attempt to dispatch the request to the Bitcache server implementation.
  if (strpos($request, BITCACHE_ALIAS) === 0) {
    $request = substr($request, strlen(BITCACHE_ALIAS));

    // Ensure that the bitstream index is accessed with a trailing slash.
    if (!$request) {
      die(drupal_goto(BITCACHE_ALIAS . '/', $query, NULL, 301));
    }

    $server = new Bitcache_DrupalServer(
      bitcache_get_repository(NULL),
      array('tmpdir' => file_directory_temp())
    );
    $server->$method($request, $query);
    die();
  }

  // A path alias has been defined for the Bitcache server root URL, but the
  // incoming request was still using an internal path; redirect the client
  // to the correct address.
  if (strpos($request, BITCACHE_BASE) === 0) {
    $request = substr($request, strlen(BITCACHE_BASE));
    die(drupal_goto(BITCACHE_ALIAS . ($request ? $request : '/'), $query, NULL, 301));
  }

  die(drupal_not_found()); // should never get here
}

//////////////////////////////////////////////////////////////////////////////
// Drupal server implementation

class Bitcache_DrupalServer extends Bitcache_Server {

  function index($request, array $query = array(), $body = TRUE) {
    static $formats = array(
      'text/plain'       => 'text',
      //'text/html'        => 'html', // TODO
      'application/json' => 'json',
      'text/x-ntriples'  => 'ntriples',
    );

    // Content negotiation based on the HTTP Accept header and, secondarily,
    // on the ?format= URL query parameter.
    $format = isset($_GET['format']) ? $_GET['format'] : NULL;
    foreach (explode(',', $_SERVER['HTTP_ACCEPT']) as $accept) {
      list($mime, ) = explode(';', $accept);
      if (isset($formats[$mime])) {
        $format = $mime;
        break;
      }
    }
    $format = isset($_GET['format']) ? $_GET['format'] : $format;
    $format = isset($formats[$format]) ? $format : 'text/plain';

    $this->header('Content-Type: '. $format .'; charset=ascii');

    if ($body) {
      $handler = 'index_'. $formats[$format];
      $this->$handler();
    }

    return TRUE;
  }

  protected function index_json() {
    $index = array();
    foreach ($this->repo as $id => $stream) {
      $index[$id] = $stream->size();
    }
    print drupal_to_js($index);
  }

  protected function index_ntriples() {
    foreach ($this->repo as $id => $stream) {
      printf("<%s> <%s> \"%d\"^^<http://www.w3.org/2001/XMLSchema#integer> .\n",
        $subject   = bitcache_resolve_id($id),
        $predicate = 'http://purl.org/dc/terms/extent',
        $object    = $stream->size());
    }
  }

  protected function allow($method, $id = NULL) {
    $method = ($method == 'GET' && !$id) ? 'INDEX' : 'GET'; // TODO: should be handled in base class

    $methods = array_filter(variable_get('bitcache_features', array('index', 'get', 'post', 'put', 'delete')), 'is_string');
    if (($method != 'HEAD' && empty($methods[strtolower($method)])) || ($method == 'HEAD' && empty($methods['get']))) {
      return FALSE;
    }

    switch ($method) {
      case 'INDEX':
        return user_access('list bitstreams');
      case 'HEAD':
      case 'GET':
        foreach (bitcache_get_modules() as $module) {
          if (($result = module_invoke($module, 'bitcache', 'access', $id)) !== NULL) {
            return $result;
          }
        }
        return user_access('access bitstreams');
      case 'POST':
      case 'PUT':
        return user_access('upload bitstreams');
      case 'DELETE':
        return user_access('delete bitstreams');
    }
    return parent::allow($method, $id);
  }

  protected function headers($id, $stream) {
    parent::headers($id, $stream);
    foreach (module_invoke_all('bitcache', 'download', $id, $stream) as $key => $value) {
      $this->header($key . ': ' . $value);
    }

    // Override Drupal's Cache-Control headers to enable at least some forms of caching:
    // @see http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.29
    // @see http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.9
    // @see http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html#sec14.21
    $this->header('Last-Modified: ' . gmdate('D, d M Y H:i:s') . ' GMT');
    $this->header('Expires: ' . gmdate('D, d M Y H:i:s', time() + 365 * 86400) . ' GMT');
    $this->header('Cache-Control: private, no-transform');
  }

  protected function created($id, $stream = NULL) {
    watchdog('bitcache', 'Bitstream created: %id.', array('%id' => $id), WATCHDOG_NOTICE, l(t('view'), 'admin/content/bitcache/view/' . $id));
    parent::created($id);
    //module_invoke_all('bitcache', 'insert', $id, $stream); // handled in repository
  }

  protected function deleted($id) {
    watchdog('bitcache', 'Bitstream deleted: %id.', array('%id' => $id), WATCHDOG_NOTICE, l(t('view'), 'admin/content/bitcache'));
    parent::deleted($id);
    //module_invoke_all('bitcache', 'delete', $id); // handled in repository
  }

}
