<?php
// $Id: file.inc,v 1.2 2009/03/13 10:06:11 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// File system backend

/**
 * Implements a file-system-backed Bitcache repository.
 */
class Bitcache_FileRepository extends Bitcache_Repository implements Iterator {
  public $path;

  function __construct(array $options = array()) {
    $this->name    = !empty($options['name']) ? $options['name'] : NULL;
    $this->path    = !empty($options['location']) ? $options['location'] : $options['path'];
    $this->uri     = preg_match('!^[\w]+://!', $this->path) ? $this->path : 'file://' . realpath($this->path);
    $this->options = array(
      'depth' => defined('BITCACHE_DEPTH') ? BITCACHE_DEPTH : 0,
      'width' => defined('BITCACHE_WIDTH') ? BITCACHE_WIDTH : 0,
    );
    $this->options = array_merge($this->options, $options);
    $this->create();
  }

  function create() {
    // Create the repository directory if it doesn't exist yet:
    if (!is_dir($this->path)) {
      if (function_exists('file_check_directory')) {
        file_check_directory($this->path, TRUE, 'location'); // Drupal-specific
      }
    }
  }

  function rename($old_name, $new_name, array $options = array()) {
    if (isset($options['location']) && $options['location'] === TRUE) {
      $old_location = $this->path;
      $new_location = preg_replace('!' . preg_quote($old_name, '!') . '$!', $new_name, $old_location);
      if ($new_location != $old_location) {
        return rename($old_location, $new_location);
      }
    }
  }

  function destroy(array $options = array()) {
    if (!isset($options['contents']) || $options['contents'] !== FALSE) {
      return $this->rmdir_rf($this->path); // delete the contents of the repository directory
    }
  }

  /**
   * Determines how much disk space is occupied by the repository.
   */
  function size() { // FIXME
    $info = explode("\t", shell_exec('du -sk '. $this->path));
    return (int)$info[0] * 1024;
  }

  /**
   * Determines the total bitstream count in the repository.
   *
   * @see    Countable
   */
  function count() {
    return iterator_count($this);
  }

  /**
   * Rewinds the iterator back to the repository's first bitstream.
   *
   * @see    Iterator
   */
  function rewind() {
    if (!empty($this->dir)) {
      closedir($this->dir);
    }
    $this->dir = is_dir($this->path) ? opendir($this->path) : NULL;
    $this->next();
  }

  /**
   * Returns the iterator's current key.
   *
   * @see    Iterator
   */
  function key() {
    return $this->file;
  }

  /**
   * Returns the iterator's currently-accessible bitstream.
   *
   * @see    Iterator
   */
  function current() {
    return new Bitcache_FileStream(NULL, $this->path($this->file)); // FIXME
  }

  /**
   * Advances the iterator to the next bitstream.
   *
   * @see    Iterator
   */
  function next() {
    if (!empty($this->dir)) {
      while (($this->file = readdir($this->dir)) !== FALSE) {
        if (preg_match(BITCACHE_ID_FORMAT, $this->file)) {
          break;
        }
      }
    }
  }

  /**
   * Checks whether the iterator's current element is valid.
   *
   * @see    Iterator
   */
  function valid() {
    return !empty($this->dir) && $this->file !== FALSE;
  }

  /**
   * Checks whether a specified bitstream exists in the repository.
   *
   * @param  string $id      the bitstream identifier
   */
  function exists($id) {
    return is_readable($this->path($id));
  }

  /**
   * Retrieves a specified bitstream from the repository.
   *
   * @param  string $id      the bitstream identifier
   * @see    Bitcache_FileStream
   */
  function get($id) {
    if (is_readable($filepath = $this->path($id))) {
      return new Bitcache_FileStream($id, $filepath);
    }
  }

  /**
   * Stores the contents of a bitstream in the repository.
   *
   * @param  string $id      the bitstream identifier
   */
  function put($id, $data) {
    $id = !$id ? sha1($data) : $id;
    if (($id = file_put_contents($this->path($id), $data) >= 0 ? $id : FALSE)) {
      $this->created($id);
    }
    return $id;
  }

  /**
   * Stores the contents of a bitstream in the repository.
   *
   * @param  string $id      the bitstream identifier
   */
  function put_file($id, $filepath, $move = FALSE) {
    $id = !$id ? sha1_file($filepath) : $id;
    if (($id = (($move ? rename($filepath, $this->path($id)) : copy($filepath, $this->path($id))) ? $id : FALSE))) {
      $this->created($id);
    }
    return $id;
  }

  /**
   * Deletes a bitstream from the repository.
   *
   * @param  string $id      the bitstream identifier
   */
  function delete($id) {
    if (($result = ($this->exists($id) ? @unlink($this->path($id)) : NULL))) {
      $this->deleted($id);
    }
    return $result;
  }

  /**
   * Constructs a file system path for the specified bitstream.
   */
  protected function path($id = NULL) {
    $path = array($this->path);
    if ($id) {
      if (empty($this->options['depth'])) {
        // no action needed
      }
      else if ($this->options['depth'] == 1) {
        $path = array($this->path, substr($id, 0, $this->options['width']));
        $this->mkdir(implode('/', $path));
      }
      else if (($split = (int)$this->options['depth'] * (int)$this->options['width']) < strlen($id)) {
        $path = array_merge($path, str_split(substr($id, 0, $split), (int)$this->options['width']));
        $this->mkdir(implode('/', $path));
      }
      $path[] = $id;
    }
    return implode('/', $path);
  }

  protected function mkdir($path, $mode = 0775, $recursive = TRUE) {
    if (!is_dir($path)) {
      mkdir($path, $mode, $recursive);
    }
  }

  /**
   * Recursive version of rmdir(); use with extreme caution.
   *
   * @param $dirname
   *   the top-level directory that will be recursively removed
   * @param $callback
   *   optional predicate function for determining if a file should be removed
   */
  protected function rmdir_rf($dirname, $callback = NULL) {
    $empty = TRUE; // begin with an optimistic mindset

    foreach (glob($dirname . '/*', GLOB_NOSORT) as $file) {
      if (is_dir($file)) {
        if (!$this->rmdir_rf($file, $callback)) {
          $empty = FALSE;
        }
      }
      else if (is_file($file)) {
        if (function_exists($callback) && !$callback($file)) {
          $empty = FALSE;
          continue;
        }
        @unlink($file);
      }
      else {
        $empty = FALSE; // it's probably a symbolic link
      }
    }

    // The reason for this elaborate safeguard is that Drupal will log even
    // warnings that should have been suppressed with the @ operator.
    // Otherwise, we'd just rely on the FALSE return value from rmdir().
    return ($empty && @rmdir($dirname));
  }
}

//////////////////////////////////////////////////////////////////////////////

class Bitcache_FileStream extends Bitcache_Stream {
  public $path;

  function __construct($id, $path, array $options = array()) {
    $this->id   = $id;
    $this->path = $path;
    foreach ($options as $k => $v) {
      $this->$k = $v;
    }
    $this->stat();
  }

  function open($mode = 'rb') {
    return $this->stream = fopen($this->path, $mode);
  }

  function close() {
    if (!empty($this->stream)) {
      $result = fclose($this->stream);
      unset($this->stream);
      return $result;
    }
  }

  function data() {
    return file_get_contents($this->path);
  }

  function size() {
    return !is_null($this->size) ? $this->size : $this->size = filesize($this->path);
  }

  function type() {
    if (extension_loaded('fileinfo') && ($finfo = finfo_open(FILEINFO_MIME))) {
      if (($type = finfo_file($finfo, $this->path))) {
        finfo_close($finfo);
        return $type;
      }
    }
    if (function_exists('mime_content_type')) {
      if (($type = mime_content_type($this->path))) {
        return $type;
      }
    }
    if (file_exists($this->path)) {
      // Attempts to detect a file's MIME type using the Unix `file' utility.
      // MIME content type format defined in http://www.ietf.org/rfc/rfc1521.txt
      $output = exec('file -bi '. escapeshellarg($this->path), $ignore, $status);
      if ($status == 0 && preg_match('!([\w-]+/[\w\d_+-]+)!', $output, $matches))
        return $matches[1];
    }

    return 'application/octet-stream'; // default to binary
  }
}
