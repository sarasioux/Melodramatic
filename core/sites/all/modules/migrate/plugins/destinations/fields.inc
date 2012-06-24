<?php

/**
 * @file
 * Support for processing CCK fields
 */

class MigrateFieldsNodeHandler extends MigrateDestinationHandler {
  public function __construct() {
    $this->registerTypes(array('node'));
  }

  public function fields($entity_type, $bundle) {
    $fields = array();
    if (module_exists('content')) {
      $content_info = _content_type_info();
      foreach ($content_info['content types'][$bundle]['fields'] as $machine_name => $instance) {
        $fields[$machine_name] = t('Node:') . ' ' . $instance['widget']['label'] .
          ' (' . $instance['type'] . ')';
      }
    }
    return $fields;
  }

  public function prepare($entity, stdClass $row) {
    migrate_instrument_start('MigrateDestinationEntity->prepareFields');
    // Look for Field API fields attached to this destination and handle appropriately
    $migration = Migration::currentMigration();
    $destination = $migration->getDestination();
    $entity_type = $destination->getEntityType();
    $bundle = $destination->getBundle();
    $info = content_types($bundle);
    $instances = $info['fields'];
    foreach ($instances as $machine_name => $instance) {
      if (property_exists($entity, $machine_name)) {
        // Normalize to an array
        if (!is_array($entity->$machine_name)) {
          $entity->$machine_name = array($entity->$machine_name);
        }
        $entity->$machine_name = migrate_field_handler_invoke_all($entity,
          $instance, $entity->$machine_name);
      }
    }
    migrate_instrument_stop('MigrateDestinationEntity->prepareFields');
  }

  public function complete($entity, stdClass $row) {
    migrate_instrument_start('MigrateDestinationEntity->completeFields');
    // Look for Field API fields attached to this destination and handle appropriately
    $migration = Migration::currentMigration();
    $destination = $migration->getDestination();
    $entity_type = $destination->getEntityType();
    $bundle = $destination->getBundle();
    $info = content_types($bundle);
    $instances = $info['fields'];
    foreach ($instances as $machine_name => $instance) {
      if (property_exists($entity, $machine_name)) {
        // Normalize to an array
        if (!is_array($entity->$machine_name)) {
          $entity->$machine_name = array($entity->$machine_name);
        }
        migrate_field_handler_invoke_all($entity,
          $instance, $entity->$machine_name, 'complete');
      }
    }
    migrate_instrument_stop('MigrateDestinationEntity->completeFields');
  }
}

abstract class MigrateFieldHandler extends MigrateHandler {
  // abstract function arguments(...)
  abstract public function prepare($entity, array $instance, array $values);
}

class MigrateTextFieldHandler extends MigrateFieldHandler {
  public function __construct() {
    $this->registerTypes(array('text', 'text_long', 'text_with_summary'));
  }

  static function arguments($summary = NULL, $format = NULL, $language = NULL) {
    $arguments = array();
    if (!is_null($summary)) {
      $arguments['summary'] = $summary;
   }
    if (!is_null($format)) {
      $arguments['format'] = $format;
    }
    if (!is_null($language)) {
      $arguments['language'] = $language;
    }
    return $arguments;
  }

  public function prepare($entity, array $instance, array $values) {
    if (isset($values['arguments'])) {
      $arguments = $values['arguments'];
      unset($values['arguments']);
    }
    else {
      $arguments = array();
    }

    $migration = Migration::currentMigration();
    $destination = $migration->getDestination();

    // Setup the standard Field API array for saving.
    $delta = 0;
    foreach ($values as $value) {
      $item = array();
      if (isset($arguments['summary'])) {
        $item['summary'] = $arguments['summary'];
      }
      $format = isset($arguments['format']) ?
        $arguments['format'] : $destination->getTextFormat();
      $item['format'] = $item['value_format'] = $format;
      $item['value'] = $value;

      $return[$delta] = $item;
      $delta++;
    }

    return isset($return) ? $return : NULL;
  }
}

class MigrateValueFieldHandler extends MigrateFieldHandler {
  public function __construct() {
    $this->registerTypes(array('value', 'list', 'list_boolean', 'list_number',
      'list_text', 'number_integer', 'number_decimal', 'number_float'));
  }

  public function prepare($entity, array $instance, array $values) {
    // Setup the standard Field API array for saving.
    $delta = 0;
    foreach ($values as $value) {
      $return[$delta]['value'] = $value;
      $delta++;
    }
    if (!isset($return)) {
      $return = NULL;
    }
    return $return;
  }
}

class MigrateFileFieldHandler extends MigrateFieldHandler {
  public function __construct() {
    $this->registerTypes(array('filefield'));
  }

  /**
   * Prepare file data for saving as a Field API file field. The job of this
   * handler is to make sure the file itself ends up in the right place, the
   * files table row is set up, and the files array returned for each file.
   *
   * @return array
   *  Field API array suitable for inserting in the destination object.
   */
  public function prepare($entity, array $instance, array $values) {
    $migration = Migration::currentMigration();
    $return = array();
    $arguments = $values['arguments'];
    unset($values['arguments']);
    // One can override a file_function via CLI or drushrc.php
    if ($migration->getOption('file_function')) {
      $file_function = $migration->getOption('file_function');
    }
    else {
      $file_function = $arguments['file_function'];
    }
    foreach ($values as $delta => $file_path) {
      if (!$file_path) {
        continue;
      }
      // Handle JSON input
      if ($file_path{0} == '{') {
        $properties = json_decode($file_path, TRUE);
        $file_path = $properties['path'];
        // Properties passed in with the image override any set via arguments
        if (!empty($properties['alt'])) {
          $arguments['alt'] = $properties['alt'];
        }
        if (!empty($properties['title'])) {
          $arguments['title'] = $properties['title'];
        }
        if (!empty($properties['description'])) {
          $arguments['description'] = $properties['description'];
        }
        if (!empty($properties['list'])) {
          $arguments['list'] = $properties['list'];
        }
      }
      $message_saved = FALSE;
      if ($arguments['source_path']) {
        $full_path = rtrim($arguments['source_path'], "/\\") .
          '/' . ltrim($file_path, "/\\");
      }
      else {
        $full_path = $file_path;
      }

      // Set destination_dir, considering user tokens which filefield natively supports.
      // Also load $account or build a stub (faster)
      $account = user_load(array('uid' => $entity->uid));
      if (!module_exists('filefield_paths') && isset($entity->uid) && strpos($instance['widget']['file_path'], '[')) {
        $destination_dir = filefield_widget_file_path($instance, $account);
      }
      else {
        // Set a default destination_dir.
        $destination_dir = file_directory_path() . '/' . $instance['widget']['file_path'];
      }

      // file_check_directory does not do $recursive so we create dir ourselves.
      @mkdir($destination_dir, 0777, TRUE);
      $destination_file = rtrim($destination_dir, "/\\") .
        '/' . basename($full_path);

      migrate_instrument_start('MigrateFileFieldHandler file_function');
      switch ($file_function) {
        // These physically transfer the file to the destination, applying
        // the file_replace setting if the file already exists
        case 'file_copy':
        case 'file_move':
          $remote = FALSE;

          // Check that source exists. If not, mark the entity as 'needs_update' and bail.
          // Sometimes the source file arrives later, when rsync is slower than DB.
          if (!is_file($full_path)) {
            // is_file() won't handle URLs, check to see if we have a remote file
            // (only relevant in the file_copy case)
            if ($file_function == 'file_copy') {
              $headers = @get_headers($full_path);
              if (is_array($headers) && preg_match('%^HTTP/[0-9]\.[0-9] 200 OK$%', $headers[0])) {
                $remote = TRUE;
              }
            }
            if (!$remote) {
              $message = t('Source file does not exist: !path', array('!path' => $full_path));
              if ($arguments['throw_error']) {
                throw new MigrateException($message, MigrationBase::MESSAGE_ERROR);
              }
              else {
                $migration->saveMessage($message, MigrationBase::MESSAGE_INFORMATIONAL);
              }
              $message_saved = TRUE;
              $migration->needsUpdate = TRUE;
              continue;
            }
          }
          // TODO: FILE_EXISTS_REPLACE is hardcoded
          // TODO: Should retrieve and pass validators in 2nd arg
          if ($remote) {
            $temporary_file = file_directory_temp() . "/" . basename($full_path);
            $result = @copy($full_path, $temporary_file);
            if ($result) {
              $file = field_file_save_file($temporary_file, array(), $destination_dir, $account);
              file_delete($temporary_file);
            }
            else {
              $migration->saveMessage(t('Unable to copy file from !source',
                array('!source' => $source->uri)));
            }
          }
          else {
            $file = field_file_save_file($full_path, array(), $destination_dir, $account);
            if ($file_function == 'file_move') {
              // TODO: Optimize by using rename()
              unlink($full_path);
            }
          }
          break;
        case 'file_fast':
          static $fid;
          // Keep re-using an existing file.
          if (!isset($fid)) {
            $full_path = 'misc/druplicon.png';
            $file = field_file_save_file($full_path, array(), $destination_dir, $account);
  //          $file = file_copy($source, $destination_dir, FILE_EXISTS_RENAME);
            $fid = $file['fid'];
          }
          else {
            $file = array('fid' => $fid);
          }
          break;
        case 'file_link':
          // The file is copied by some outside process (e.g., rsync), and we
          // just need to make sure it's present and has a files table row.
          // Not present - skip
          migrate_instrument_start('file_link: file_exists');
          if (!file_exists($full_path)) {
            migrate_instrument_stop('file_link: file_exists');
            $message = t('File does not exist in Drupal files directory: !path',
              array('!path' => $full_path));
            if ($arguments['throw_error']) {
              throw new MigrateException($message, MigrationBase::MESSAGE_ERROR);
            }
            else {
              $migration->saveMessage($message, MigrationBase::MESSAGE_INFORMATIONAL);
            }
            $message_saved = TRUE;
      // TODO     $migration->needsUpdate = TRUE;
            continue;
          }
          migrate_instrument_stop('file_link: file_exists');

          // Files table entry exists? Use that...
          migrate_instrument_start('file_link: select existing');
          // Note that indexing files.filepath can be very helpful.
          $file = db_select('files', 'f')
                  ->fields('f')
                  ->condition('filepath', $full_path)
                  ->execute()
                  ->fetchAssoc();
          migrate_instrument_stop('file_link: select existing');
          if (!$file) {
            migrate_instrument_start('file_link: create file record');
            $file = new stdClass;
            $file->uri = $full_path;
            $file->uid = isset($entity->uid) ? $entity->uid : 0;
            $file->filename = basename($full_path);
            $file->filepath = $full_path;
            $file->filemime = file_get_mimetype($full_path);
            $file->timestamp = $_SERVER['REQUEST_TIME'];
            $file->filesize = filesize($full_path);
            $file->status = FILE_STATUS_PERMANENT;

            drupal_write_record('files', $file);
            $file = (array)$file;
            migrate_instrument_stop('file_link: create file record');
          }
          break;
        default:
          $migration->saveMessage(t('Unrecognized file_function: !func', array('!func' => $file_function)));
          $message_saved = TRUE;
          break;
      }
      migrate_instrument_stop('MigrateFileFieldHandler file_function');

      if (isset($file) && is_array($file)) {
        // Build up a return object.
        $file['list'] = isset($arguments['list']) ? $arguments['list'] : NULL;
        $file['data'] = array(
          'alt' => isset($arguments['alt']) ? $arguments['alt'] : NULL,
          'title' => isset($arguments['title']) ? $arguments['title'] : NULL,
          'description' => isset($arguments['description']) ? $arguments['description'] : NULL,
          'process' => TRUE, // This activates filefield_paths crazy move code.
        );
        $return[] = $file;
      }
      elseif (!$message_saved) {
        throw new MigrateException(t('Unable to create file record for !path',
          array('!path' => $full_path)), Migration::MESSAGE_ERROR);
      }
    }
    return $return;
  }

  /*
   * Arguments for a file_field migration.
   *
   * @param source_path
   *   Path to source file.
   * @param file_function
   *   file_fast, file_move, or file_copy.
   * @param file_replace
   *   Value of $replace in that file function. Does not apply to file_fast(). Defaults to FILE_EXISTS_RENAME.
   * @param language
   *   Language of the text (defaults to destination language)
   * @param alt
   *   String to be used as the alt value for all file fields, or
   *   array('source_field' => 'source_field_name') to obtain the alt value from
   *   the query field named source_field_name.
   * @param title
   *   String to be used as the title value for all file fields, or
   *   array('source_field' => 'source_field_name') to obtain the title value from
   *   the query field named source_field_name.
   * @param description
   *   String to be used as the description value for all file fields, or
   *   array('source_field' => 'source_field_name') to obtain the description value from
   *   the query field named source_field_name.
   * @param list
   *   String to be used as the list value for all file fields, or
   *   array('source_field' => 'source_field_name') to obtain the list value from
   *   the query field named source_field_name.
   * @param throw_error
   *  A non-existent file generates an INFORMATIONAL message by default. If this
   *  parameter is TRUE, throw an error instead (abort node creation).
   *
   */
  static function arguments($source_path = NULL, $file_function = 'file_copy',
      $file_replace = FILE_EXISTS_RENAME, $alt = NULL,
      $title = NULL, $description = NULL,
      $list = NULL, $throw_error = FALSE) {
    return get_defined_vars();
  }
}

class MigrateNodeReferenceFieldHandler extends MigrateFieldHandler {
  public function __construct() {
    $this->registerTypes(array('nodereference'));
  }

  public function prepare($entity, array $instance, array $values) {
    if (isset($values['arguments'])) {
      $arguments = $values['arguments'];
      unset($values['arguments']);
    }
    else {
      $arguments = array();
    }

    // Setup the standard Field API array for saving.
    $delta = 0;
    $return = array();
    foreach ($values as $value) {
      // Don't save empty references
      if ($value) {
        $return[$delta]['nid'] = $value;
        $delta++;
      }
    }
    return $return;
  }
}

class MigrateUserReferenceFieldHandler extends MigrateFieldHandler {
  public function __construct() {
    $this->registerTypes(array('userreference'));
  }

  public function prepare($entity, array $instance, array $values) {
    if (isset($values['arguments'])) {
      $arguments = $values['arguments'];
      unset($values['arguments']);
    }
    else {
      $arguments = array();
    }

    // Setup the standard Field API array for saving.
    $delta = 0;
    $return = array();
    foreach ($values as $value) {
      // Don't save empty references
      if ($value) {
        $return[$delta]['uid'] = $value;
        $delta++;
      }
    }
    return $return;
  }
}

class MigrateEmailFieldHandler extends MigrateFieldHandler {
  public function __construct() {
    $this->registerTypes(array('email'));
  }

  // Copied from ValueFieldHandler::prepare except the propname is email.
  public function prepare($entity, array $instance, array $values) {
    // Setup the standard Field API array for saving.
    $delta = 0;
    foreach ($values as $value) {
      $return[$delta]['email'] = $value;
      $delta++;
    }
    if (!isset($return)) {
      $return = NULL;
    }
    return $return;
  }
}