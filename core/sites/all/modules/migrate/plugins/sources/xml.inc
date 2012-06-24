<?php

/**
 * @file
 * Support for migration from XML sources.
 *
 * NOTE: There are two methods supported in this file.
 *
 * 1) List - ids are listed in an index xml file and the data for each item is
 *      stored in a separate xml file per item. Use MigrateSourceList class
 *      as the source.
 *
 * 2) MultiItems - ids are part of the item and all items are stored in a
 *      single xml file. Use MigrateSourceMultiItems class as the source.
 *
 * Both of these methods are described in more detail in the wine migration
 * example.
 */

/*===========================================================================*/
/*                              List Method                                  */
/*===========================================================================*/
/**
 * Implementation of MigrateList, for retrieving a list of IDs to be migrated
 * from an XML document.
 */
class MigrateListXML extends MigrateList {
  /**
   * A URL pointing to an XML document containing a list of IDs to be processed.
   *
   * @var string
   */
  protected $listUrl;

  public function __construct($list_url) {
    parent::__construct();
    $this->listUrl = $list_url;
    // Suppress errors during parsing, so we can pick them up after
    libxml_use_internal_errors(TRUE);
  }

  /**
   * Our public face is the URL we're getting items from
   *
   * @return string
   */
  public function __toString() {
    return $this->listUrl;
  }

  /**
   * Load the XML at the given URL, and return an array of the IDs found within it.
   *
   * @return array
   */
  public function getIdList() {
    migrate_instrument_start("Retrieve $this->listUrl");
    $xml = simplexml_load_file($this->listUrl);
    migrate_instrument_stop("Retrieve $this->listUrl");
    if ($xml) {
      return $this->getIDsFromXML($xml);
    }
    else {
      $migration = Migration::currentMigration();
      $migration->showMessage(t('Loading of !listurl failed:',
          array('!listurl' => $this->listUrl)));
      foreach (libxml_get_errors() as $error) {
        $migration->showMessage($error);
      }
      return NULL;
    }
  }

  /**
   * Given an XML object, parse out the IDs for processing and return them as an
   * array. The default implementation assumes the IDs are simply the values of
   * the top-level elements - in most cases, you will need to override this to
   * reflect your particular XML structure.
   *
   * @param SimpleXMLElement $xml
   *
   * @return array
   */
  protected function getIDsFromXML(SimpleXMLElement $xml) {
    $ids = array();
    foreach ($xml as $element) {
      $ids[] = (string)$element;
    }
    return array_unique($ids);
  }

  /**
   * Return a count of all available IDs from the source listing. The default
   * implementation assumes the count of top-level elements reflects the number
   * of IDs available - in many cases, you will need to override this to reflect
   * your particular XML structure.
   */
  public function computeCount() {
    $xml = simplexml_load_file($this->listUrl);
    // Number of sourceid elements beneath the top-level element
    $count = count($xml);
    return $count;
  }
}

/**
 * Implementation of MigrateItem, for retrieving a parsed XML document given
 * an ID provided by a MigrateList class.
 */
class MigrateItemXML extends MigrateItem {
  /**
   * A URL pointing to an XML document containing the data for one item to be
   * migrated.
   *
   * @var string
   */
  protected $itemUrl;

  public function __construct($item_url) {
    parent::__construct();
    $this->itemUrl = $item_url;
    // Suppress errors during parsing, so we can pick them up after
    libxml_use_internal_errors(TRUE);
  }

  /**
   * Implementors are expected to return an object representing a source item.
   *
   * @param mixed $id
   *
   * @return stdClass
   */
  public function getItem($id) {
    // Make sure we actually have an ID
    if (empty($id)) {
      return NULL;
    }
    $item_url = $this->constructItemUrl($id);
    // And make sure we actually got a URL to fetch
    if (empty($item_url)) {
      return NULL;
    }
    // Get the XML object at the specified URL;
    $xml = $this->loadXmlUrl($item_url);
    if ($xml) {
      $return = new stdclass;
      $return->xml = $xml;
      return $return;
    }
    else {
      $migration = Migration::currentMigration();
      $message =  t('Loading of !objecturl failed:', array('!objecturl' => $item_url));
      foreach (libxml_get_errors() as $error) {
        $message .= "\n" . $error->message;
      }
      $migration->getMap()->saveMessage(
        array($id), $message, MigrationBase::MESSAGE_ERROR);
      libxml_clear_errors();
      return NULL;
    }
  }

  /**
   * The default implementation simply replaces the :id token in the URL with
   * the ID obtained from MigrateListXML. Override if the item URL is not
   * so easily expressed from the ID.
   *
   * @param mixed $id
   */
  protected function constructItemUrl($id) {
    return str_replace(':id', $id, $this->itemUrl);
  }

  /**
   * Default XML loader - just use Simplexml directly. This can be overridden for
   * preprocessing of XML (removal of unwanted elements, caching of XML if the
   * source service is slow, etc.)
   */
  protected function loadXmlUrl($item_url) {
    return simplexml_load_file($item_url);
  }
}

/**
 * Adds xpath info to field mappings for XML sources
 */
class MigrateXMLFieldMapping extends MigrateFieldMapping {
  /**
   * The xpath used to retrieve the data for this field from the XML.
   *
   * @var string
   */
  protected $xpath;
  public function getXpath() {
    return $this->xpath;
  }

  /**
   * Add an xpath to this field mapping
   *
   * @param string $xpath
   */
  public function xpath($xpath) {
    $this->xpath = $xpath;
    return $this;
  }
}

/**
 * Migrations using XML sources should extend this class instead of Migration.
 */
abstract class XMLMigration extends Migration {
  /**
   * Override the default addFieldMapping(), so we can create our special
   * field mapping class.
   * TODO: Find a cleaner way to just substitute a different mapping class
   *
   * @param string $destinationField
   *  Name of the destination field.
   * @param string $sourceField
   *  Name of the source field (optional).
   */
  protected function addFieldMapping($destination_field, $source_field = NULL) {
    // Warn of duplicate mappings
    if (!is_null($destination_field) && isset($this->fieldMappings[$destination_field])) {
      $this->showMessage(
        t('!name addFieldMapping: !dest was previously mapped, overridden',
          array('!name' => $this->machineName, '!dest' => $destination_field)),
        'warning');
    }
    $mapping = new MigrateXMLFieldMapping($destination_field, $source_field);
    if (is_null($destination_field)) {
      $this->fieldMappings[] = $mapping;
    }
    else {
      $this->fieldMappings[$destination_field] = $mapping;
    }
    return $mapping;
  }

  /**
   * A normal $data_row has all the input data as top-level fields - in this
   * case, however, the data is embedded within a SimpleXMLElement object in
   * $data_row->xml. Explode that out to the normal form, and pass on to the
   * normal implementation.
   */
  protected function applyMappings() {
    // We only know what data to pull from the xpaths in the mappings.
    foreach ($this->fieldMappings as $mapping) {
      $source = $mapping->getSourceField();
      if ($source) {
        $xpath = $mapping->getXpath();
        if ($xpath) {
          // Derived class may override applyXpath()
          $this->sourceValues->$source = $this->applyXpath($this->sourceValues, $xpath);
        }
      }
    }
    parent::applyMappings();
  }

  /**
   * Default implementation - straightforward xpath application
   *
   * @param $data_row
   * @param $xpath
   */
  public function applyXpath($data_row, $xpath) {
    $result = $data_row->xml->xpath($xpath);
    if ($result) {
      if (count($result) > 1) {
        $return = array();
        foreach ($result as $record) {
          $return[] = (string)$record;
        }
        return $return;
      }
      else {
        return (string)$result[0];
      }
    }
    else {
      return NULL;
    }
  }
}

/*===========================================================================*/
/*                           MultiItems Method                               */
/*===========================================================================*/
/**
 * Implementation of MigrateItems, for providing a list of IDs and for
 * retrieving a parsed XML document given an ID from this list.
 */
class MigrateItemsXML extends MigrateItems {
  /**
   * A URL pointing to an XML document containing the ids and data.
   *
   * @var string
   */
  protected $xmlUrl;

  /**
   * xpath identifying the element used for each item
   */
  protected $itemXpath;
  public function getItemXpath() {
    return $this->itemXpath;
  }

  /**
   * xpath identifying the subelement under itemXpath that holds the id for
   * each item.
   */
  protected $itemIDXpath;
  public function getIDXpath() {
    return $this->itemIDXpath;
  }

  public function __construct($xml_url, $item_xpath='item', $itemID_xpath='id') {
    parent::__construct();
    $this->xmlUrl = $xml_url;
    $this->itemXpath = $item_xpath;
    $this->itemIDXpath = $itemID_xpath;

    // Suppress errors during parsing, so we can pick them up after
    libxml_use_internal_errors(TRUE);
  }

  /**
   * Our public face is the URL we're getting items from
   *
   * @return string
   */
  public function __toString() {
    return 'url = ' . $this->xmlUrl . ' | item xpath = ' . $this->itemXpath .
                                      ' | item ID xpath = ' . $this->itemIDXpath;
  }

  /**
   * Load the XML at the given URL, and return an array of the IDs found
   * within it.
   *
   * @return array
   */
  public function getIdList() {
    migrate_instrument_start("Retrieve $this->xmlUrl");
    $xml = simplexml_load_file($this->xmlUrl);
    migrate_instrument_stop("Retrieve $this->xmlUrl");
    if ($xml) {
      return $this->getIDsFromXML($xml);
    }
    else {
      $migration = Migration::currentMigration();
      $migration->showMessage(t('Loading of !xmlUrl failed:',
          array('!xmlUrl' => $this->xmlUrl)));
      foreach (libxml_get_errors() as $error) {
        $migration->showMessage($error);
      }
      return NULL;
    }
  }

  /**
   * Given an XML object, parse out the IDs for processing and return them as
   * an array. The location of the IDs in the XML are based on the item xpath
   * and item ID xpath set in the constructor.
   *    eg, xpath = itemXpath . '/' . itemIDXpath
   * IDs are cached.  The list of IDs are returned from the cache except when
   * this is the first call (ie, cache is NULL) OR the refresh parameter is
   * TRUE.
   *
   * @param SimpleXMLElement $xml
   * @param boolean $refresh
   *
   * @return array
   */
  protected $cache_ids = NULL;
  protected function getIDsFromXML(SimpleXMLElement $xml, $refresh=FALSE) {
    if ($refresh !== TRUE && $this->cache_ids != NULL) {
      return $this->cache_ids;
    }

    $this->cache_ids = NULL;
    $ids = array();
    $full_xpath = $this->itemXpath . '/' . $this->itemIDXpath;

    $result = $xml->xpath($full_xpath);
    if ($result) {
      if (count($result) > 1) {
        foreach ($result as $id) {
          $ids[] = (string)$id;
        }
      }
      else {
        $ids[] = (string)$result[0];
      }
    }
    $this->cache_ids = array_unique($ids);
    return $this->cache_ids;
  }

  /**
   * Return a count of all available IDs from the source listing.
   */
  public function computeCount() {
    $count = 0;
    $xml = simplexml_load_file($this->xmlUrl);
    if ($xml) {
      $ids = $this->getIDsFromXML($xml, TRUE);
      $count = count($ids);
    }
    return $count;
  }

  /**
   * Load the XML at the given URL, and return an array of the Items found
   * within it.
   *
   * @return array
   */
  public function getAllItems() {
    $xml = simplexml_load_file($this->xmlUrl);
    if ($xml) {
      return $this->getItemsFromXML($xml);
    }
    else {
      $migration = Migration::currentMigration();
      $migration->showMessage(t('Loading of !xmlUrl failed:',
          array('!xmlUrl' => $this->xmlUrl)));
      foreach (libxml_get_errors() as $error) {
        $migration->showMessage($error);
      }
      return NULL;
    }
  }

  /**
   * Given an XML object, parse out the items for processing and return them as
   * an array. The location of the items in the XML are based on the item xpath
   * set in the constructor.  Items are cached.  The list of items are returned
   * from the cache except when this is the first call (ie, cache is NULL) OR
   * the refresh parameter is TRUE.
   *
   * Items are cached as an array of key=ID and value=stdclass object with
   * attribute xml containing the xml SimpleXMLElement object of the item.
   *
   * @param SimpleXMLElement $xml
   * @param boolean $refresh
   *
   * @return array
   */
  protected $cache_items = NULL;
  public function getItemsFromXML(SimpleXMLElement $xml, $refresh=FALSE) {
    if ($refresh !== FALSE && $this->cache_items != NULL) {
      return $this->cache_items;
    }

    $this->cache_items = NULL;
    $items = array();
    $result = $xml->xpath($this->itemXpath);

    if ($result) {
      if (count($result) > 1) {
        foreach ($result as $item_xml) {
          $id = $this->getItemID($item_xml);
          $item = new stdclass;
          $item->xml = $item_xml;
          $items[$id] = $item;
        }
      }
      else {
        $item_xml = $result[0];
        $id = $this->getItemID($item_xml);
        $item = new stdclass;
        $item->xml = $item_xml;
        $items[$id] = $item;
      }
      $this->cache_items = $items;
      return $items;
    }
    else {
      return NULL;
    }
  }

  /**
   * Get the item ID from the itemXML based on itemIDXpath.
   *
   * @return string
   */
  protected function getItemID($itemXML){
    return $this->getElementValue($itemXML, $this->itemIDXpath);
  }

  /**
   * Get an element from the itemXML based on an xpath.
   *
   * @return string
   */
  protected function getElementValue($itemXML,$xpath){
    $value = NULL;
    if ($itemXML) {
      $result = $itemXML->xpath($xpath);
      if ($result)
        $value = (string)$result[0];
    }
    return $value;
  }

  /**
   * Implementors are expected to return an object representing a source item.
   * Items are cached as an array of key=ID and value=stdclass object with
   * attribute xml containing the xml SimpleXMLElement object of the item.
   *
   * @param mixed $id
   *
   * @return stdClass
   */
  public function getItem($id) {
    // Make sure we actually have an ID
    if (empty($id)) {
      return NULL;
    }
    $items = $this->getAllItems();
    $item = $items[$id];
    if ($item) {
      return $item;
    }
    else {
      $migration = Migration::currentMigration();
      $message =  t('Loading of item XML for ID !id failed:', array('!id' => $id));
      foreach (libxml_get_errors() as $error) {
        $message .= "\n" . $error->message;
      }
      $migration->getMap()->saveMessage(
        array($id), $message, MigrationBase::MESSAGE_ERROR);
      libxml_clear_errors();
      return NULL;
    }
  }
}
