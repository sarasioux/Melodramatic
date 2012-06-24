<?php

/**
 * @file
 * Define a MigrateSource class for importing from Oracle databases.
 */

/**
 * Implementation of MigrateSource, to handle imports from remote Oracle servers.
 */
class MigrateSourceOracle extends MigrateSource {
  /**
   * Array containing information for connecting to Oracle:
   *  username - Username to connect as
   *  password - Password for logging in
   *  connection_string - See http://us.php.net/manual/en/function.oci-connect.php.
   *
   * @var array
   */
  protected $configuration;

  /**
   * The active Oracle connection for this source.
   *
   * @var resource
   */
  protected $connection;
  public function getConnection() {
    return $this->connection;
  }

  /**
   * The SQL query from which to obtain data. Is a string.
   */
  protected $query;

  /**
   * The result object from executing the query - traversed to process the
   * incoming data.
   */
  protected $result;

  /**
   * Number of eligible rows processed so far (used for itemlimit checking)
   *
   * @var int
   */
  protected $numProcessed = 0;

  /**
   * Character set to use in retrieving data.
   *
   * @var string
   */
  protected $characterSet;

  /**
   * Return an options array for Oracle sources.
   *
   * @param string $character_set
   *  Character set to use in retrieving data. Defaults to 'UTF8'.
   * @param boolean $cache_counts
   *  Indicates whether to cache counts of source records.
   */
  static public function options($character_set = 'UTF8', $cache_counts = FALSE) {
    return compact('character_set', 'cache_counts');
  }

  /**
   * Simple initialization.
   */
  public function __construct(array $configuration, $query, $count_query,
      array $fields, array $options = array()) {
    parent::__construct($options);
    $this->query = $query;
    $this->countQuery = $count_query;
    $this->configuration = $configuration;
    $this->fields = $fields;
    if (empty($options['character_set'])) {
      $this->characterSet = 'UTF8';
    }
    else {
      $this->characterSet = $options['character_set'];
    }
  }

  /**
   * Return a string representing the source query.
   *
   * @return string
   */
  public function __toString() {
    return $this->query;
  }

  /**
   * Connect lazily to the DB server.
   */
  protected function connect() {
    if (!isset($this->connection)) {
      if (!extension_loaded('oci8')) {
        throw new Exception(t('You must configure the oci8 extension in PHP.'));
      }
      $this->connection = oci_connect($this->configuration['username'],
        $this->configuration['password'], $this->configuration['connection_string'],
        $this->characterSet);
    }
    if ($this->connection) {
      return TRUE;
    }
    else {
      $e = oci_error();
      throw new Exception($e['message']);
      return FALSE;
    }
  }

  /**
   * Returns a list of fields available to be mapped from the source query.
   *
   * @return array
   *  Keys: machine names of the fields (to be passed to addFieldMapping)
   *  Values: Human-friendly descriptions of the fields.
   */
  public function fields() {
    // The fields are passed to the constructor for this plugin.
    return $this->fields;
  }

  /**
   * Return a count of all available source records.
   */
  public function computeCount() {
    migrate_instrument_start('MigrateSourceOracle count');
    if ($this->connect()) {
      $statement = oci_parse($this->connection, $this->countQuery);
      if (!$statement) {
        $e = oci_error($this->connection);
        throw new Exception($e['message'] . "\n" . $e['sqltext']);
      }
      $result = oci_execute($statement);
      if (!$result) {
        $e = oci_error($statement);
        throw new Exception($e['message'] . "\n" . $e['sqltext']);
      }
      $count_array = oci_fetch_array($statement);
      $count = reset($count_array);
    }
    else {
      // Do something else?
      $count = FALSE;
    }
    migrate_instrument_stop('MigrateSourceOracle count');
    return $count;
  }

  /**
   * Implementation of Iterator::rewind() - called before beginning a foreach loop.
   */
  public function rewind() {
    $migration = Migration::currentMigration();
    $this->result = NULL;
    $this->currentRow = NULL;
    $this->numProcessed = 0;
    $this->numIgnored = 0;
    $map = $migration->getMap();

    $keys = array();
    foreach ($map->getSourceKey() as $field_name => $field_schema) {
      // Allow caller to provide an alias to table containing the primary key.
      if (!empty($field_schema['alias'])) {
        $field_name = $field_schema['alias'] . '.' . $field_name;
      }
      $keys[] = $field_name;
    }

    /*
     * Replace :criteria placeholder with idlist or highwater clauses. We
     * considered supporting both but it is not worth the complexity. Run twice
     * instead.
     */
    $idlist = $migration->getOption('idlist');
    if (isset($idlist)) {
      // TODO: Sanitize. not critical as this is admin supplied data in drush.
      $this->query = str_replace(':criteria',
        $keys[0] . ' IN (' . $idlist . ')', $this->query);
    }
    else {
      $highwaterField = $migration->getHighwaterField();
      if (isset($highwaterField['name']) && $highwater = $migration->getHighwater()) {
        if (empty($highwaterField['alias'])) {
          $highwater_name = $highwaterField['name'];
        }
        else {
          $highwater_name = $highwaterField['alias'] . '.' . $highwaterField['name'];
        }
        $this->query = str_replace(':criteria', "$highwater_name > '$highwater'", $this->query);
      }
      else {
        // No idlist or highwater. Replace :criteria placeholder with harmless WHERE
        // clause instead of empty since we don't know if an AND follows.
        $this->query = str_replace(':criteria', '1=1', $this->query);
      }
    }

    migrate_instrument_start('oracle_query');
    $this->connect();
    $this->result = oci_parse($this->connection, $this->query);
    if (!$this->result) {
      $e = oci_error($this->connection);
      throw new Exception($e['message'] . "\n" . $e['sqltext']);
    }
    $status = oci_execute($this->result);
    if (!$status) {
      $e = oci_error($this->result);
      throw new Exception($e['message'] . "\n" . $e['sqltext']);
    }
    migrate_instrument_stop('oracle_query');

    $this->next();
  }

  /**
   * Implementation of Iterator::next() - called at the bottom of the loop implicitly,
   * as well as explicitly from rewind().
   */
  public function next() {
    migrate_instrument_start('MigrateSourceOracle next');

    $migration = Migration::currentMigration();

    // Enforce the itemlimit
    $itemlimit = $migration->getItemLimit();
    while ($this->currentRow = $this->getNextObject()) {
      // Enforce the itemlimit
      if ($itemlimit && $this->numProcessed >= $itemlimit) {
        $this->currentRow = NULL;
        return;
      }

      // Check the map - if it's already mapped, and not marked for update, skip it
      $map = $migration->getMap();
      $this->currentKey = array();
      foreach ($map->getSourceKey() as $field_name => $field_schema) {
        $this->currentKey[$field_name] = $this->currentRow->$field_name;
      }
      $map_row = $map->getRowBySource($this->currentKey);
      if ($map_row && $map_row['needs_update'] == 0) {
        continue;
      }
      // Add map info to the row, if present
      if ($map_row) {
        foreach ($map_row as $field => $value) {
          $field = 'migrate_map_' . $field;
          $this->currentRow->$field = $value;
        }
      }

      // Allow the Migration to prepare this row. prepareRow() can return boolean
      // FALSE to stop processing this row. To add/modify fields on the
      // result, modify $row by reference.
      $return = TRUE;
      if (method_exists($migration, 'prepareRow')) {
        $return = $migration->prepareRow($this->currentRow);
      }

      if ($return !== FALSE) {
        $this->numProcessed++;
        break;
      }
      else {
        $this->numIgnored++;
      }
    }
    if (!is_object($this->currentRow)) {
      $this->currentRow = NULL;
    }

    migrate_instrument_stop('MigrateSourceOracle next');
  }

  /**
   * Convenience function to return the next row of the result set as an
   * object, making sure NULLs are represented as PHP NULLs and that LOBs are
   * returned directly without special handling.
   */
  public function getNextObject() {
    migrate_instrument_start('MigrateSourceOracle getNextObject');
    $row = oci_fetch_array($this->result, OCI_ASSOC | OCI_RETURN_NULLS | OCI_RETURN_LOBS);
    migrate_instrument_stop('MigrateSourceOracle getNextObject');
    if (!empty($row)) {
      return (object)$row;
    }
    else {
      return FALSE;
    }
  }
}
