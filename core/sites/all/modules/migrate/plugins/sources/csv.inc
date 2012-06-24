<?php

/**
 * @file
 * Define a MigrateSource for importing from comma separated values file.
 */

/**
 * Implementation of MigrateSource, to handle imports from CVV file.
 */
class MigrateSourceCSV extends MigrateSource {
  /**
   * Number of eligible rows processed so far (used for itemlimit checking)
   *
   * @var int
   */
  protected $numProcessed = 0;

  /**
   * List of available source fields.
   *
   * @var array
   */
  protected $fields = array();

  /**
   * Parameters for the fgetcsv() call.
   *
   * @var array
   */
  protected $fgetcsv = array();

  protected $usingHighwater = FALSE;

  /**
   * The number of rows in the CSV file before the data starts.
   *
   * @var integer
   */
  protected $headerRows = 0;

  /**
   * Simple initialization.
   *
   * @param atring $path
   *  The path to the source file
   * @param array $csvcolumns
   *  Keys are integers. values are array(field name, description).
   * @param array $options
   *  Options applied to this source.
   * @param array $fields
   *  Optional - keys are field names, values are descriptions. Use to override
   *  the default descriptions, or to add additional source fields which the
   *  migration will add via other means (e.g., prepareRow()).
   */
  public function __construct($path, array $csvcolumns = array(), array $options = array(), array $fields = array()) {
    parent::__construct($options);
    $this->file = $path;
    if (!empty($options['header_rows'])) {
      $this->headerRows = $options['header_rows'];
    }
    else {
      $this->headerRows = 0;
    }
    $this->options = $options;
    $this->fields = $fields;
    // fgetcsv specific options
    foreach (array('length' => NULL, 'delimiter' => ',', 'enclosure' => '"', 'escape' => '\\') as $key => $default) {
      $this->fgetcsv[$key] = isset($options[$key]) ? $options[$key] : $default;
    }
    // One can either pass in an explicit list of column names to use, or if we have
    // a header row use the names from that
    if ($this->headerRows && empty($csvcolumns)) {
      $this->csvcolumns = array();
      $result = fopen($this->file, 'r');
      // Skip all but the last header
      for ($i = 0; $i < $this->headerRows - 1; $i++) {
        fgets($this->result);
      }

      $row = fgetcsv($result, $this->fgetcsv['length'], $this->fgetcsv['delimiter'],
        $this->fgetcsv['enclosure']);
      foreach ($row as $header) {
        $header = trim($header);
        $this->csvcolumns[] = array($header, $header);
      }
      fclose($result);
    }
    else {
      $this->csvcolumns = $csvcolumns;
    }
  }


  /**
   * Return a string representing the source query.
   *
   * @return string
   */
  public function __toString() {
    return $this->file;
  }

  /**
   * Returns a list of fields available to be mapped from the source query.
   *
   * @return array
   *  Keys: machine names of the fields (to be passed to addFieldMapping)
   *  Values: Human-friendly descriptions of the fields.
   */
  public function fields() {
    foreach ($this->csvcolumns as $int => $values) {
      $fields[$values[0]] = $values[1];
    }

    // Any caller-specified fields with the same names as extracted fields will
    // override them; any others will be added
    if ($this->fields) {
      $fields = $this->fields + $fields;
    }

    return $fields;
  }

  /**
   * Return a count of all available source records.
   */
  public function computeCount() {
    // If the data may have embedded newlines, the file line count won't reflect
    // the number of CSV records (one record will span multiple lines). We need
    // to scan with fgetcsv to get the true count.
    if (!empty($this->options['embedded_newlines'])) {
      $result = fopen($this->file, 'r');
      // Skip all but the last header
      for ($i = 0; $i < $this->headerRows; $i++) {
        fgets($result);
      }
      $count = 0;
      while (fgetcsv($result, $this->fgetcsv['length'], $this->fgetcsv['delimiter'], $this->fgetcsv['enclosure'])) {
        $count++;
      }
      fclose($result);
    }
    else {
      // TODO. If this takes too much time/memory, use exec('wc -l')
      $count = count(file($this->file));
      $count -= $this->headerRows;
    }
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

    migrate_instrument_start('MigrateSourceCSV execute');
    $this->result = fopen($this->file, 'r');
    migrate_instrument_stop('MigrateSourceCSV execute');

    // Load up the first row, skipping the header if necessary
    for ($i = 0; $i < $this->headerRows; $i++) {
      fgets($this->result);
    }
    $this->next();
  }

  /**
   * Implementation of Iterator::next() - called at the bottom of the loop implicitly,
   * as well as explicitly from rewind().
   */
  public function next() {
    $migration = Migration::currentMigration();
    $this->currentRow = NULL;
    $this->currentKey = NULL;

    $itemlimit = $migration->getItemLimit();
    if ($itemlimit && $this->numProcessed >= $itemlimit) {
      return;
    }

    // get next row
    migrate_instrument_start('MigrateSourceCSV next');
    $map = $migration->getMap();
    // @todo $this->fgetcsv['escape'] not used as it gives warning in 5.2 - http://drupal.org/node/1039808.
    while ($row = fgetcsv($this->result, $this->fgetcsv['length'], $this->fgetcsv['delimiter'], $this->fgetcsv['enclosure'])) {
      // Set meaningful keys for the columns mentioned in $this->csvcolumns().
      foreach ($this->csvcolumns as $int => $values) {
        list($key, $description) = $values;
        // Copy value to more descriptive string based key and then unset original.
        $row[$key] = $row[$int];
        unset($row[$int]);
      }
      $this->currentRow = (object) $row;

      foreach ($map->getSourceKey() as $field_name => $field_schema) {
        $this->currentKey[$field_name] = $this->currentRow->$field_name;
      }

      $map_row = $migration->getMap()->getRowBySource($this->currentKey);
      if (!$map_row) {
        // Unmigrated row, take it
      }
      elseif ($map_row && $map_row['needs_update'] == 1) {
        // We always want to take this row if needs_update = 1
      }
      else {
        if ($this->usingHighwater) {
          // With highwater, we want to take this row if it's above the highwater
          // mark
          $highwaterField = $migration->getHighwaterField();
          $highwaterField = $highwaterField['name'];
          if ($this->currentRow->$highwaterField <= $migration->getHighwater()) {
            $this->currentRow = NULL;
            continue;
          }
        }
        else {
          // With no highwater, we want to take this row if it's not in the map table
          if ($map_row) {
            $this->currentRow = NULL;
            continue;
          }
        }
      }
      // Add map info to the row, if present
      if ($map_row) {
        foreach ($map_row as $field => $value) {
          $field = 'migrate_map_' . $field;
          $this->currentRow->$field = $value;
        }
      }

      // Add some debugging, just for the first row.
      if (empty($this->numProcessed)) {
        $migration->showMessage(print_r($this->currentRow, TRUE), 'debug');
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
        $this->currentRow = NULL;
        $this->numIgnored++;
      }
    }
    if (!is_object($this->currentRow)) {
      $this->currentRow = NULL;
    }
    migrate_instrument_stop('MigrateSourceCSV next');
  }
}
