<?php

/**
 * @file
 * Support for node destinations.
 */

// TODO:
// Make sure this works with updates, explicit destination keys

/**
 * Destination class implementing migration into nodes.
 */
class MigrateDestinationNode extends MigrateDestinationEntity {
  static public function getKeySchema() {
    return array(
      'nid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'ID of destination node',
      ),
    );
  }

  /**
   * Return an options array for node destinations.
   *
   * @param string $language
   *  Default language for nodes created via this destination class.
   * @param string $text_format
   *  Default text format for nodes created via this destination class.
   */
  static public function options($language, $text_format) {
    return compact('language', 'text_format');
  }

  /**
   * Basic initialization
   *
   * @param string $bundle
   *  A.k.a. the content type (page, article, etc.) of the node.
   * @param array $options
   *  Options applied to nodes.
   */
  public function __construct($bundle, array $options = array()) {
    parent::__construct('node', $bundle, $options);
  }

  /**
   * Returns a list of fields available to be mapped for the node type (bundle)
   *
   * @return array
   *  Keys: machine names of the fields (to be passed to addFieldMapping)
   *  Values: Human-friendly descriptions of the fields.
   */
  public function fields() {
    $fields = array();
    // First the core (node table) properties
    $fields['nid'] = t('Node: Existing node ID');
    $fields['uid'] = t('Node: Authored by (uid)');
    $fields['revision_uid'] = t('Node: Modified (uid)');
    $fields['created'] = t('Node: Created timestamp');
    $fields['changed'] = t('Node: Modified timestamp');
    $fields['body'] = t('Node: Body');
    $fields['teaser'] = t('Node: Teaser');
    $fields['status'] = t('Node: Published');
    $fields['promote'] = t('Node: Promoted to front page');
    $fields['sticky'] = t('Node: Sticky at top of lists');
    $fields['revision'] = t('Node: Create new revision');
    $fields['language'] = t('Node: Language (fr, en, ...)');

    $node_type = node_type_load($this->bundle);
    if ($node_type->has_title) {
      $fields['title'] = t('Node:') . ' ' . $node_type->title_label;
    }

    // Then add in anything provided by handlers
    $fields += migrate_handler_invoke_all('Node', 'fields', $this->entityType, $this->bundle);

    return $fields;
  }

  /**
   * Delete a node
   *
   * @param $nid
   *  Array containing the node ID to be deleted.
   */
  public function rollback(array $nid) {
    static $count = 0;
    migrate_instrument_start('node_delete');
    $this->prepareRollback($nid);
    node_delete(reset($nid));
    // Periodically flush the node cache
    if ($count++ > 500) {
      $count = 0;
      node_load(1, NULL, TRUE);
    }
    $this->completeRollback($nid);
    migrate_instrument_stop('node_delete');
  }

  /**
   * Import a single node.
   *
   * @param $node
   *  Node object to build. Prefilled with any fields mapped in the Migration.
   * @param $row
   *  Raw source data object - passed through to prepare/complete handlers.
   * @return array
   *  Array of key fields (nid only in this case) of the node that was saved if
   *  successful. FALSE on failure.
   */
  public function import(stdClass $node, stdClass $row) {
    $migration = Migration::currentMigration();
    // Updating previously-migrated content?
    if (isset($row->migrate_map_destid1)) {
      if (isset($node->nid)) {
        if ($node->nid != $row->migrate_map_destid1) {
          throw new MigrateException(t("Incoming nid !nid and map destination nid !destid1 don't match",
            array('!nid' => $node->nid, '!destid1' => $row->migrate_map_destid1)));
        }
      }
      else {
        $node->nid = $row->migrate_map_destid1;
      }
      // Get the existing vid, so updates don't generate notices
      $node->vid = db_select('node', 'n')
                   ->fields('n', array('vid'))
                   ->condition('nid', $node->nid)
                   ->execute()
                   ->fetchField();
    }

    if ($migration->getSystemOfRecord() == Migration::DESTINATION) {
      if (!isset($node->nid)) {
        throw new MigrateException(t('System-of-record is DESTINATION, but no destination nid provided'));
      }
      $old_node = node_load($node->nid);
      if (!isset($node->created)) {
        $node->created = $old_node->created;
      }
      if (!isset($node->vid)) {
        $node->vid = $old_node->vid;
      }
      if (!isset($node->status)) {
        $node->status = $old_node->status;
      }
      if (!isset($node->uid)) {
        $node->uid = $old_node->uid;
      }
    }

    // Set some required properties.
    // Set type before invoking prepare handlers - they may take type-dependent actions.
    $node->type = $this->bundle;
    $type_info = node_get_types('type', $node->type);
    if (empty($node->nid)) {
      // Avoids notice in forum module and maybe more - http://drupal.org/node/839770 is similar.
      $node->nid = NULL;
    }

    // Save changed timestamp and set it later, node_save sets it unconditionally
    if (isset($node->changed)) {
      $changed = MigrationBase::timestamp($node->changed);
    }

    if (isset($node->created)) {
      $node->created = MigrationBase::timestamp($node->created);
    }

    // Generate teaser from Body if teaser not specified and Body is in use.
    if ($type_info->has_body && empty($node->teaser) && !empty($node->body)) {
      $node->teaser = node_teaser($node->body);
    }

    // Set defaults as per content type settings. Can't call node_object_prepare()
    // since that clobbers existing values.
    $node_options = variable_get('node_options_'. $node->type, array('status', 'promote'));
    foreach (array('status', 'promote', 'sticky') as $key) {
      if (!isset($node->$key)) {
        $node->$key = in_array($key, $node_options);
      }
    }
    if (!isset($node->comment) && module_exists('comment')) {
      $node->comment = variable_get("comment_$node->type", COMMENT_NODE_READ_WRITE);
    }
    if (!isset($node->revision)) {
      $node->revision = 0; // Saves disk space and writes. Can be overridden.
    }

    // Invoke migration prepare handlers
    $this->prepare($node, $row);

    // Trying to update an existing node
    if ($migration->getSystemOfRecord() == Migration::DESTINATION) {
      // Incoming data overrides existing data, so only copy non-existent fields
      foreach ($old_node as $field => $value) {
        // An explicit NULL in the source data means to wipe to old value (i.e.,
        // don't copy it over from $old_node)
        if (property_exists($node, $field) && $node->$field === NULL) {
          // Ignore this field
        }
        elseif (!isset($node->$field)) {
          $node->$field = $old_node->$field;
        }
      }
    }

    if (isset($node->nid)) {
      $updating = TRUE;
    }
    else {
      $updating = FALSE;
    }

    migrate_instrument_start('node_save');
    node_save($node);
    migrate_instrument_stop('node_save');

    if (isset($node->nid)) {
      if ($updating) {
        $this->numUpdated++;
      }
      else {
        $this->numCreated++;
      }

      // Unfortunately, http://drupal.org/node/722688 was not accepted, so fix
      // the changed timestamp
      if (isset($changed)) {
        db_update('node')
          ->fields(array('changed' => $changed))
          ->condition('nid', $node->nid)
          ->execute();
        $node->changed = $changed;
      }

      // Potentially fix uid and timestamp in node_revisions.
      $query = db_update('node_revisions')
                 ->condition('vid', $node->vid);
      if (isset($changed)) {
        $fields['timestamp'] = $changed;
        $node->timestamp = $changed;
      }
      $revision_uid = isset($node->revision_uid) ? $node->revision_uid : $node->uid;
      if ($revision_uid != $GLOBALS['user']->uid) {
        $fields['uid'] = $revision_uid;
      }
      if (!empty($fields)) {
        // We actually have something to update.
        $query->fields($fields);
        $query->execute();
      }
      $return = array($node->nid);
    }
    else {
      $return = FALSE;
     }
    $this->complete($node, $row);

    // Clear the node cache periodically
    static $count = 0;
    if ($count++ > 100) {
      node_load('foo', NULL, TRUE);
      $count = 0;
    }

    return $return;
  }
}
