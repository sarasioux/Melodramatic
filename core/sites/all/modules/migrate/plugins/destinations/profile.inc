<?php
// $Id

/**
 * @file
 * Support for profile.module fields as a destination.
 */

/**
 * Destination class handling profile.module data.
 */
class MigrateProfileUserHandler extends MigrateDestinationHandler {

  public function __construct() {
    $this->registerTypes(array('user'));
  }

  /**
   * Returns a list of fields available to be mapped for users.
   *
   * @return array
   *  Keys: machine names of the fields (to be passed to addFieldMapping)
   *  Values: Human-friendly descriptions of the fields.
   */
  public function fields() {
    $fields = array();
    if (module_exists('profile')) {
      foreach (profile_categories() as $category) {
        $result = _profile_get_fields($category['name'], FALSE);
        while ($field = db_fetch_object($result)) {
          $fields[$field->name] = $field->category . ': ' . $field->title;
        }
      }
    }
    return $fields;
  }

  public function prepare(stdClass $account, stdClass $row) {
    if (module_exists('profile')) {
      static $fields;
      if (!isset($fields)) {
        $fields = array();
        foreach (profile_categories() as $category) {
          $result = _profile_get_fields($category['name'], FALSE);
          while ($field = db_fetch_object($result)) {
            $fields[$field->name] = $field;
          }
        }
      }

      foreach ($fields as $key => $field) {
        if (isset($account->$key)) {
          switch ($field->type) {
            case 'date':
              $timestamp = MigrationBase::timestamp($account->$key);
              $account->$key = array(
                'month' => date('n', $timestamp),
                'day' => date('j', $timestamp),
                'year' => date('Y', $timestamp),
              );
              break;
            default:
              // Do nothing. Staight mapping is sufficient.
          }
        }
      }
    }
  }
}