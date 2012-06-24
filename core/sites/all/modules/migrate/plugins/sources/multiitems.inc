<?php

/**
 * @file
 * Support for migration from sources where data spans multiple lines
 * (ex. xml, json) and IDs for the items are part of each item and multiple
 * items reside in a single file.
 */

/**
 * Extend the MigrateItems class to provide a means to obtain a list of IDs to
 * be migrated from a given source (e.g., MigrateItemsXML extends MigrateItem to
 * obtain a list of IDs from an XML document).  This class also provides a means
 * to obtain the data for a given migratable item given its ID.
 */
abstract class MigrateItems {
  public function __construct() {}

  /**
   * Implementors are expected to return a string representing where the listing
   * is obtained from (a URL, file directory, etc.)
   *
   * @return string
   */
  abstract public function __toString();

  /**
   * Implementors are expected to return an array of unique IDs, suitable for
   * passing to the MigrateItem class to retrieve the data for a single item.
   *
   * @return array
   */
  abstract public function getIdList();

  /**
   * Implementors are expected to return a count of IDs available to be migrated.
   *
   * @API: Commented out so not to break implementations which haven't implemented
   *  this yet - require it in the next API.
   *
   * @return int
   */
  //abstract public function computeCount();

  /**
   * Deprecated - implement computeCount() instead
   */
  //abstract public function count($refresh = TRUE);

  /**
   * Implementors are expected to return an object representing a source item.
   *
   * @param mixed $id
   *
   * @return stdClass
   */
  abstract public function getItem($id);
}


/**
 * Implementation of MigrateItems, for providing a list of IDs and for
 * retrieving a parsed XML document given an ID from this list.
 */

/**
 * Implementation of MigrateSource, providing the semantics of iterating over
 * IDs provided by a MigrateItems and retrieving data from a MigrateItems.
 */
class MigrateSourceMultiItems extends MigrateSource {
  /**
   * MigrateItems object used to obtain the list of IDs and source for
   * all objects.
   *
   * @var MigrateItems
   */
  protected $itemsClass;

  /**
   * List of available source fields.
   *
   * @var array
   */
  protected $fields = array();

  /**
   * Keep the current source ID handy while iterating.
   * @var string
   */
  protected $id;

  /**
   * Simple initialization.
   */
  public function __construct(MigrateItems $items_class, $fields = array(), $options = array()) {
    parent::__construct($options);

    $this->itemsClass = $items_class;
    $this->fields = $fields;
  }

  /**
   * Return a string representing the source.
   *
   * @return string
   */
  public function __toString() {
    return (string) $this->itemsClass;
  }

  /**
   * Returns a list of fields available to be mapped from the source query.
   * Since we can't reliably figure out what "fields" are in the source,
   * it's up to the implementing Migration constructor to fill them in.
   *
   * @return array
   *  Keys: machine names of the fields (to be passed to addFieldMapping)
   *  Values: Human-friendly descriptions of the fields.
   */
  public function fields() {
    return $this->fields;
  }

  /**
   * It's the list class that knows how many records are available, so ask it.
   *
   * @return int
   */
  public function computeCount() {
      // @API: Support old count method for now.
    if (method_exists($this->itemsClass, 'computeCount')) {
      return $this->itemsClass->computeCount();
    }
    else {
      return $this->itemsClass->count();
    }
  }

  /**
   * Implementation of Iterator::rewind() - called before beginning a foreach loop.
   */
  public function rewind() {
    $migration = Migration::currentMigration();
    migrate_instrument_start('MigrateSourceList rewind');
    $this->numProcessed = 0;
    $this->numIgnored = 0;
    $idlist = $migration->getOption('idlist');
    if ($idlist) {
      $this->idList = explode(',', $idlist);
    }
    else {
      $this->idList = $this->itemsClass->getIdList();
    }
    migrate_instrument_stop('MigrateSourceList rewind');
    // Load up the first row
    $this->next();
  }

  /**
   * Implementation of Iterator::next() - called at the bottom of the loop implicitly,
   * as well as explicitly from rewind().
   */
  public function next() {
    $migration = Migration::currentMigration();
    migrate_instrument_start('MigrateSourceList next');
    $this->currentRow = NULL;
    $this->currentKey = NULL;

    // Enforce the itemlimit
    $itemlimit = $migration->getItemLimit();

    // Get next item (next ID not already in the map, unless needs_update=1)
    while ($this->id = array_shift($this->idList)) {
      // Skip empty IDs
      if (empty($this->id)) {
        continue;
      }
      // Enforce the itemlimit
      if ($itemlimit && $this->numProcessed >= $itemlimit) {
        return;
      }
      // Check the map - if it's already mapped, and not marked for update, skip it
      $map_row = $migration->getMap()->getRowBySource(array($this->id));
      if ($map_row && $map_row['needs_update'] == 0) {
        continue;
      }
      // TODO: Also check message table (non-informational entries here indicate failed items, we don't
      // want to run through them again)
      $this->currentRow = $this->itemsClass->getItem($this->id);
      if ($this->currentRow) {
        $this->currentKey = array($this->id);
        // Save the ID using the map source key - it will be used for mapping
        $sourceKey = $migration->getMap()->getSourceKey();
        $key_name = key($sourceKey);
        $this->currentRow->$key_name = $this->id;
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
          // OK, got a valid one, break out and return
          $this->numProcessed++;
          break;
        }
        else {
          $this->numIgnored++;
        }
      }
      else {
        $this->currentKey = NULL;
      }
    }

    migrate_instrument_stop('MigrateSourceList next');
  }
}

