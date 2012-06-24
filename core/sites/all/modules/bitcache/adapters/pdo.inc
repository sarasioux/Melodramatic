<?php
// $Id: pdo.inc,v 1.2 2009/03/13 10:06:11 arto Exp $

//////////////////////////////////////////////////////////////////////////////
// PDO database backend

/**
 * Implements a PHP Data Objects (PDO) database backend for Bitcache.
 *
 * @see http://php.net/manual/en/book.pdo.php
 */
class Bitcache_PDORepository extends Bitcache_Repository implements Iterator {
  public $dsn, $user, $password, $table;

  function __construct(array $options = array()) {
    // TODO: parse ;user, ;password and ;table from DSN
    $this->name     = !empty($options['name'])     ? $options['name']     : NULL;
    $this->dsn      = !empty($options['dsn'])      ? $options['dsn']      : NULL;
    $this->user     = !empty($options['user'])     ? $options['user']     : NULL;
    $this->password = !empty($options['password']) ? $options['password'] : NULL;
    $this->table    = !empty($options['table'])    ? $options['table']    : BITCACHE_TABLE_DEFAULT;
    $this->pdo      = new PDO($this->dsn, $this->user, $this->password);
    $this->driver   = $this->pdo->getAttribute(PDO::ATTR_DRIVER_NAME);
    $this->options  = $options;
    $this->create();
  }

  function create() {
    $blob = ($this->driver == 'pgsql') ? 'BYTEA' : 'LONGBLOB'; // special case for PostgreSQL
    $this->pdo->exec("CREATE TABLE {$this->table} (id CHAR(40) PRIMARY KEY, data $blob NOT NULL)");
  }

  // Iterator interface

  function rewind() {
    if (empty($this->iter)) {
      $this->iter = $this->pdo->prepare("SELECT id, LENGTH(data) AS size, data FROM {$this->table} ORDER by id");
    }
    $this->iter->execute();
    $this->row = NULL;
    $this->next();
  }

  function key() {
    return $this->row->id;
  }

  function current() {
    return new Bitcache_Stream($this->row->id, $this->row->data, array('size' => (int)$this->row->size));
  }

  function next() {
    if (!empty($this->iter)) {
      $this->row = $this->iter->fetch(PDO::FETCH_OBJ);
    }
  }

  function valid() {
    return !empty($this->iter) && $this->row;
  }

  // Countable interface

  function count() {
    return (int)$this->pdo->query("SELECT COUNT(id) AS count FROM {$this->table}")->fetchColumn();
  }

  // Bitcache_Repository interface

  function size() {
    return (int)$this->pdo->query("SELECT SUM(LENGTH(data)) AS size FROM {$this->table}")->fetchColumn();
  }

  function exists($id) {
    $query = $this->pdo->prepare("SELECT COUNT(id) FROM {$this->table} WHERE id = ?");
    return $query->execute(array($id)) ? (bool)$query->fetchColumn() : FALSE;
  }

  function get($id) {
    $query = $this->pdo->prepare("SELECT LENGTH(data) AS size, data FROM {$this->table} WHERE id = ?");
    if ($query->execute(array($id)) && ($row = $query->fetch(PDO::FETCH_OBJ))) {
      return new Bitcache_Stream($id, $row->data, array('size' => (int)$row->size));
    }
  }

  function put($id, $data) {
    $id = !$id ? sha1($data) : $id;
    if (!$this->exists($id) && ($id = ($this->pdo->prepare("INSERT INTO {$this->table} (id, data) VALUES (?, ?)")->execute(array($id, $data)) ? $id : FALSE))) {
      $this->created($id);
    }
    return $id;
  }

  function delete($id) {
    if (($result = $this->pdo->prepare("DELETE FROM {$this->table} WHERE id = ?")) && $result->execute(array($id))) {
      if (($result = ($result->rowCount() > 0) ? TRUE : NULL)) { // FALSE would mean something went wrong
        $this->deleted($id);
      }
    }
    return $result;
  }
}
