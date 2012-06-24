<?php
// $Id: dba.inc,v 1.1 2009/03/13 10:06:11 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// GDBM database backend

/**
 * Implements a GDBM database backend.
 *
 * @see http://php.net/manual/en/book.dba.php
 */
class Bitcache_DBARepository extends Bitcache_Repository implements Iterator {
  public $table;

  function __construct(array $options = array()) {
    $this->name    = !empty($options['name']) ? $options['name'] : NULL;
    $this->handler = !empty($options['adapter']) ? $options['adapter'] : 'gdbm';
    $this->path    = !empty($options['location']) ? $options['location'] : bitcache_tmpname() . '.' . $this->handler;
    $this->options = $options;
    $this->create();
  }

  function create() {
    $this->open('c') && $this->close();
  }

  function rename($old_name, $new_name, array $options = array()) {
    // TODO
  }

  function destroy() {
    unlink($this->path);
  }

  function open($mode = 'r') {
    $this->close();
    return $this->db = dba_open($this->path, $mode, $this->handler);
  }

  function close() {
    if (!empty($this->db)) {
      //dba_sync($this->db);
      dba_close($this->db);
      $this->db = NULL;
    }
  }

  // Iterator interface

  function rewind() {
    $this->open();
    $this->id = dba_firstkey($this->db);
  }

  function key() {
    return $this->id;
  }

  function current() {
    return new Bitcache_Stream($this->id, dba_fetch($this->id, $this->db));
  }

  function next() {
    if (($this->id = dba_nextkey($this->db)) === FALSE) {
      $this->close();
      unset($this->id);
    }
  }

  function valid() {
    return !empty($this->db) && !empty($this->id);
  }

  // Bitcache_Repository interface

  function count() {
    $count = 0;
    if ($this->open('r')) {
      if (($id = dba_firstkey($this->db))) do {
        $count++;
      } while ($id = dba_nextkey($this->db));
      $this->close();
    }
    return $count;
  }

  function size($physical = TRUE) {
    if ($physical) {
      return ($size = @filesize($this->path)) ? $size : 0;
    }
    else { // logical
      $size = 0;
      if ($this->open('r')) {
        if (($id = dba_firstkey($this->db))) do {
          $size += strlen(dba_fetch($id, $this->db));
        } while ($id = dba_nextkey($this->db));
        $this->close();
      }
      return $size;
    }
  }

  function exists($id) {
    if ($this->open('r')) {
      $result = dba_exists($id, $this->db);
      $this->close();
      return $result;
    }
    return FALSE;
  }

  function get($id) {
    if ($this->open('r')) {
      $result = dba_fetch($id, $this->db);
      $this->close();
      return $result ? new Bitcache_Stream($id, $result) : NULL;
    }
    return FALSE;
  }

  function put($id, $data) {
    $id = !$id ? sha1($data) : $id;
    if ($this->open('w')) {
      $result = @dba_insert($id, $data, $this->db); // prevent warning if it already exists
      $this->close();
      if ($result) {
        $this->created($id);
      }
      return $id;
    }
    return FALSE;
  }

  function delete($id) {
    if ($this->open('w')) {
      $result = dba_delete($id, $this->db);
      $this->close();
      if ($result) {
        $this->deleted($id);
      }
      return $result;
    }
    return FALSE;
  }
}
