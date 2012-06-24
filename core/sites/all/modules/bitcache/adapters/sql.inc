<?php
// $Id: sql.inc,v 1.3 2009/03/13 10:06:11 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// SQL database backend

/**
 * Implements a Drupal-specific SQL database backend.
 */
class Bitcache_SQLRepository extends Bitcache_Repository implements Iterator {
  public $table;

  function __construct(array $options = array()) {
    $this->name    = !empty($options['name']) ? $options['name'] : NULL;
    $this->table   = !empty($this->name) ? BITCACHE_TABLE_PREFIX . $this->name : BITCACHE_TABLE_DEFAULT;
    $this->table   = !empty($options['table']) ? $options['table'] : $this->table;
    $this->table   = db_escape_table($this->table);
    $this->options = array(); // TODO
    $this->options = array_merge($this->options, $options);
    $this->create();
  }

  function create() {
    if (!db_table_exists($this->table)) {
      $schema = bitcache_get_schema(BITCACHE_TABLE_DEFAULT);
      db_create_table($results, $this->table, $schema);
      bitcache_get_schema($this->table, TRUE); // clear the schema cache
      return $results[0]['success'];
    }
    return TRUE;
  }

  function rename($old_name, $new_name, array $options = array()) {
    $old_table = $this->table; // FIXME
    $new_table = db_escape_table(BITCACHE_TABLE_PREFIX . $new_name);
    db_rename_table($results, $old_table, $new_table);
    return $results[0]['success'];
  }

  function destroy() {
    db_drop_table($results, $this->table);
    return $results[0]['success'];
  }

  // Iterator interface

  function rewind() {
    $this->query = db_query("SELECT id, LENGTH(data) AS size, data FROM {{$this->table}} ORDER by id");
    $this->row   = NULL;
    $this->next();
  }

  function key() {
    return $this->row->id;
  }

  function current() {
    return new Bitcache_Stream($this->row->id, db_decode_blob($this->row->data), array('size' => (int)$this->row->size));
  }

  function next() {
    if (!empty($this->query)) {
      $this->row = db_fetch_object($this->query);
    }
  }

  function valid() {
    return !empty($this->query) && $this->row;
  }

  // Countable interface

  function count() {
    return (int)db_result(db_query("SELECT COUNT(id) AS count FROM {{$this->table}}"));
  }

  // Bitcache_Repository interface

  function size() {
    return (int)db_result(db_query("SELECT SUM(LENGTH(data)) AS size FROM {{$this->table}}"));
  }

  function exists($id) {
    return (bool)db_result(db_query("SELECT COUNT(id) FROM {{$this->table}} WHERE id = '%s'", $id));
  }

  function get($id) {
    if (($row = db_fetch_object(db_query("SELECT LENGTH(data) AS size, data FROM {{$this->table}} WHERE id = '%s'", $id)))) {
      return new Bitcache_Stream($id, db_decode_blob($row->data), array('size' => (int)$row->size));
    }
  }

  function put($id, $data) {
    $id = !$id ? sha1($data) : $id;
    if (!$this->exists($id) && ($id = (db_query("INSERT INTO {{$this->table}} (id, data) VALUES ('%s', %b)", $id, $data) ? $id : FALSE))) {
      $this->created($id);
    }
    return $id;
  }

  function delete($id) {
    if (($result = db_query("DELETE FROM {{$this->table}} WHERE id = '%s'", $id))) {
      if (($result = (db_affected_rows() > 0) ? TRUE : NULL)) { // FALSE would mean something went wrong
        $this->deleted($id);
      }
    }
    return $result;
  }
}
