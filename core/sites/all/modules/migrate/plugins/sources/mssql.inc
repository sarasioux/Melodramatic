<?php

/**
 * @file
 * Define a MigrateSource for importing from Microsoft SQL Server databases.
 */

/**
 * Implementation of MigrateSource, to handle imports from remote MS SQL Server db servers.
 */
class MigrateSourceMSSQL extends MigrateSource {
  /**
   * Array containing information for connecting to SQL Server:
   *  servername - Hostname of the SQL Server
   *  username - Username to connect as
   *  password - Password for logging in
   *  database (optional) - Database to select after connecting
   *
   * @var array
   */
  protected $configuration;

  /**
   * The active MS SQL Server connection for this source.
   *
   * @var resource
   */
  protected $connection;

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
   * By default, mssql_query fetches all results - severe memory problems with
   * big tables. So, we will fetch a batch at a time.
   *
   * @var int
   */
  protected $batchSize;

  /**
   * Return an options array for MS SQL sources.
   *
   * @param int $batch_size
   *  Number of rows to pull at once (defaults to 500).
   * @param boolean $cache_counts
   *  Indicates whether to cache counts of source records.
   */
  static public function options($batch_size, $cache_counts) {
    return compact('batch_size', 'cache_counts');
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
    $this->batchSize = isset($options['batch_size']) ? $options['batch_size'] : 500;
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
      if (!extension_loaded('mssql')) {
        throw new Exception(t('You must configure the mssql extension in PHP.'));
      }

      if (isset($this->configuration['port'])) {
        $host = $this->configuration['servername'] . ':' . $this->configuration['port'];
      }
      else {
        $host = $this->configuration['servername'];
      }
      $this->connection = mssql_connect(
        $host,
        $this->configuration['username'],
        $this->configuration['password'],
        TRUE);
      if (isset($this->configuration['database'])) {
        return mssql_select_db($this->configuration['database'], $this->connection);
      }
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
    migrate_instrument_start('MigrateSourceMSSQL count');
    if ($this->connect()) {
      $result = mssql_query($this->countQuery);
      $count = reset(mssql_fetch_object($result));
    }
    else {
      // Do something else?
      $count = FALSE;
    }
    migrate_instrument_stop('MigrateSourceMSSQL count');
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

    migrate_instrument_start('mssql_query');
    $this->connect();
    $this->result = mssql_query($this->query, $this->connection, $this->batchSize);
    migrate_instrument_stop('mssql_query');

    $this->next();
  }

  /**
   * Implementation of Iterator::next() - called at the bottom of the loop implicitly,
   * as well as explicitly from rewind().
   */
  public function next() {
    migrate_instrument_start('MigrateSourceMSSQL next');

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
    migrate_instrument_stop('MigrateSourceMSSQL next');
  }

  /**
   * Convenience function to return the next row of the result set as an
   * object, dealing with the difference between the end of the batch and the
   * end of all data.
   */
  public function getNextObject() {
    migrate_instrument_start('mssql_fetch_object');
    $row = mssql_fetch_object($this->result);
    migrate_instrument_stop('mssql_fetch_object');

    // Might be totally out of data, or just out of this batch - request another
    // batch and see
    if (!is_object($row)) {
      migrate_instrument_start('mssql_fetch_batch');
      mssql_fetch_batch($this->result);
      migrate_instrument_stop('mssql_fetch_batch');

      migrate_instrument_start('mssql_fetch_object');
      $row = mssql_fetch_object($this->result);
      migrate_instrument_stop('mssql_fetch_object');
    }
    if (is_object($row)) {
      return $row;
    }
    else {
      return FALSE;
    }
  }
}
