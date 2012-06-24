<?php

/**
 * @file
 * Support for paths in core Drupal objects
 */

class MigratePathNodeHandler extends MigrateDestinationHandler {
  public function __construct() {
    $this->registerTypes(array('node'));
  }

  public function fields() {
    return array('path' => t('Node: Path alias'));
  }
}
