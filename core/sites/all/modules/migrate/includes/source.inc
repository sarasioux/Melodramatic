<?php

/**
 * @file
 * Define base for migration sources.
 */

/**
 * Abstract base class for source handling.
 *
 * Derived classes are expected to define __toString(), returning a string
 * describing the source and significant options. See
 * MigrateSourceSQL for an example.
 */
abstract class MigrateSource implements Iterator {
  /**
   * The current row from the quey
   *
   * @var stdClass
   */
  protected $currentRow;

  /**
   * Number of eligible rows processed so far (used for itemlimit checking)
   *
   * @var int
   */
  protected $numProcessed = 0;

  /**
   * Number of rows intentionally ignored (prepareRow() returned FALSE)
   *
   * @var int
   */
  protected $numIgnored = 0;
  public function getIgnored() {
    return $this->numIgnored;
  }

  /**
   * Reset numIgnored back to 0.
   */
  public function resetStats() {
    $this->numIgnored = 0;
  }

  /**
   * The primary key of the current row
   *
   * @var array
   */
  protected $currentKey;
  public function getCurrentKey() {
    return $this->currentKey;
  }

  /**
   * Derived classes must implement fields(), returning a list of available
   * source fields.
   *
   * @return array
   *  Keys: machine names of the fields (to be passed to addFieldMapping)
   *  Values: Human-friendly descriptions of the fields.
   */
  abstract public function fields();

  /**
   * Whether this instance should cache the source count;
   * @var boolean
   */
  protected $cacheCounts = FALSE;

  /**
   * Return a count of available source records, from the cache if appropriate.
   *
   * @param boolean $refresh
   */
  public function count($refresh = FALSE) {
    $cache_key = md5((string)$this);
    // If a refresh is requested, or we're not caching counts, ask the derived
    // class to get the count from the source.
    if ($refresh || !$this->cacheCounts) {
      $count = $this->computeCount();
      cache_set($cache_key, $count, 'cache');
    }
    else {
      // Caching is in play, first try to retrieve a cached count.
      $cache_object = cache_get($cache_key, 'cache');
      if (is_object($cache_object)) {
        // Success
        $count = $cache_object->data;
      }
      else {
        // No cached count, ask the derived class to count 'em up, and cache
        // the result
        $count = $this->computeCount();
        cache_set($cache_key, $count, 'cache');
      }
    }
    return $count;
  }

  /**
   * Derived classes must implement computeCount(), to retrieve a fresh count of
   * source records.
   */
  //abstract public function computeCount();

  /**
   * Class constructor.
   *
   * @param array $options
   *  Optional array of options.
   */
  public function __construct($options = array()) {
    if (!empty($options['cache_counts'])) {
      $this->cacheCounts = TRUE;
    }
  }

  /**
   * Default implementations of Iterator methods - many derivations will find
   * these adequate and will only need to implement rewind() and next()
   */

  /**
   * Implementation of Iterator::current() - called when entering a loop
   * iteration, returning the current row
   */
  public function current() {
    return $this->currentRow;
  }

  /**
   * Implementation of Iterator::key - called when entering a loop iteration, returning
   * the key of the current row. It must be a scalar - we will serialize
   * to fulfill the requirement, but using getCurrentKey() is preferable.
   */
  public function key() {
    return serialize($this->currentKey);
  }

  /**
   * Implementation of Iterator::valid() - called at the top of the loop, returning
   * TRUE to process the loop and FALSE to terminate it
   */
  public function valid() {
    return !is_null($this->currentRow);
  }
}
