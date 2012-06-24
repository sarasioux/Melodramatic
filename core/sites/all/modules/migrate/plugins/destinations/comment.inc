<?php

/**
 * @file
 * Support for comment destinations.
 */

// TODO:
// Make sure this works with updates, explicit destination keys

/**
 * Destination class implementing migration into comments.
 */
class MigrateDestinationComment extends MigrateDestinationEntity {
  static public function getKeySchema() {
    return array(
      'cid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => 'ID of destination entity',
      ),
    );
  }

  /**
   * Return an options array for comment destinations.
   *
   * @param string $language
   *  Default language for comments created via this destination class.
   * @param string $text_format
   *  Default text format for comments created via this destination class.
   */
  static public function options($language, $text_format) {
    return compact('language', 'text_format');
  }

  /**
   * Basic initialization
   *
   * @param string $bundle
   *  A.k.a. the content type (page, article, etc.) of the ... comment?.
   * @param array $options
   *  Options applied to comments.
   */
  public function __construct($bundle, array $options = array()) {
    parent::__construct('comment', $bundle, $options);
  }

  /**
   * Returns a list of fields available to be mapped for comments attached to
   * a particular bundle (node type)
   *
   * @return array
   *  Keys: machine names of the fields (to be passed to addFieldMapping)
   *  Values: Human-friendly descriptions of the fields.
   */
  public function fields() {
    $fields = array();
    // First the core (comment table) properties
    $fields['cid'] = t('Comment: Existing comment ID');
    $fields['pid'] = t('Comment: Parent (by Drupal ID)');
    $fields['nid'] = t('Comment: Node (by Drupal ID)');
    $fields['uid'] = t('Comment: User (by Drupal ID)');
    $fields['subject'] = t('Comment: Subject');
    $fields['comment'] = t('Comment: Body');
    $fields['hostname'] = t('Comment: Hostname/IP address');
    $fields['timestamp'] = t('Comment: Modified timestamp');
    $fields['status'] = t('Comment: Status');
    $fields['thread'] = t('Comment: Thread');   // ???
    $fields['name'] = t('Comment: User name (not username)');
    $fields['mail'] = t('Comment: Email address');
    $fields['homepage'] = t('Comment: Homepage');
    $fields['language'] = t('Comment: Language');

    // Then add in anything provided by handlers
    $fields += migrate_handler_invoke_all('Comment', 'fields', $this->entityType, $this->bundle);

    return $fields;
  }

  /**
   * Delete a comment
   *
   * @param $cid
   *  Array of comment ID fieldss to be deleted.
   */
  public function rollback(array $cid) {
    $path = drupal_get_path('module', 'comment') . '/comment.admin.inc';
    include_once($path);
    // Backdoor deletion - query stolen from comment_delete()
    $form = array();
    $form_state = array();
    $comment = db_fetch_object(db_query(
      'SELECT c.*, u.name AS registered_name, u.uid
       FROM {comments} c
       LEFT JOIN {users} u ON u.uid = c.uid
       WHERE c.cid = %d',
      reset($cid)));
    // May have been deleted as the child of a comment previously deleted
    if ($comment) {
      $this->prepareRollback($cid);
      $form['#comment'] = $comment;
      comment_confirm_delete_submit($form, $form_state);
      $this->completeRollback($cid);
    }
  }

  /**
   * Import a single comment.
   *
   * @param $comment
   *  Comment object to build. Prefilled with any fields mapped in the Migration.
   * @param $row
   *  Raw source data object - passed through to prepare/complete handlers.
   * @return array
   *  Array of key fields (cid only in this case) of the comment that was saved if
   *  successful. FALSE on failure.
   */
  public function import(stdClass $comment, stdClass $row) {
    $migration = Migration::currentMigration();
    // Updating previously-migrated content?
    if (isset($row->migrate_map_destid1)) {
      if (isset($comment->cid)) {
        if ($comment->cid != $row->migrate_map_destid1) {
          throw new MigrateException(t("Incoming cid !cid and map destination nid !destid1 don't match",
            array('!cid' => $comment->cid, '!destid1' => $row->migrate_map_destid1)));
        }
      }
      else {
        $comment->cid = $row->migrate_map_destid1;
      }
    }

    // Fix up timestamps
    if (isset($comment->timestamp)) {
      $comment->timestamp = MigrationBase::timestamp($comment->timestamp);
    }

    if ($migration->getSystemOfRecord() == Migration::DESTINATION) {
      if (!isset($comment->cid)) {
        throw new MigrateException(t('System-of-record is DESTINATION, but no destination cid provided'));
      }
      $rawcomment = $comment;
      $old_comment = _comment_load($comment->cid);
      if (!isset($comment->nid)) {
        $comment->nid = $old_comment->nid;
      }
      $this->prepare($comment, $row);
      foreach ($rawcomment as $field => $value) {
        $old_comment->$field = $comment->$field;
      }
      $comment = $old_comment;
    }
    else {
      // Set some default properties.
      $defaults = array(
        'language' => $this->language,
        'subject' => '',
        'comment' => '',
        'uid' => 0,
        'status' => COMMENT_PUBLISHED,
        'cid' => 0,
        'pid' => 0,
        'format' => filter_fallback_format(),
      );
      foreach ($defaults as $field => $value) {
        if (!isset($comment->$field)) {
          $comment->$field = $value;
        }
      }
      $this->prepare($comment, $row);
    }

    // Make sure we have a nid
    if (!isset($comment->nid) || !$comment->nid) {
      throw new MigrateException(t('No node ID provided for comment'));
    }

    // comment_save() hardcodes hostname, so if we're trying to set it we
    // need to save it and apply it after
    if (isset($comment->hostname)) {
      $hostname = $comment->hostname;
    }

    // Convert to unix timestamp as needed
    if (isset($comment->timestamp)) {
      $comment->timestamp = MigrationBase::timestamp($comment->timestamp);
    }

    if (isset($comment->cid) && $comment->cid) {
      $updating = TRUE;
    }
    else {
      $updating = FALSE;
    }
    migrate_instrument_start('comment_save');
    $comment = (array)$comment;
    $comment['cid'] = comment_save($comment);
    $comment = (object)$comment;
    migrate_instrument_stop('comment_save');
    if (isset($hostname) && isset($comment->cid) && $comment->cid > 0) {
      db_update('comments')
        ->fields(array('hostname' => $hostname))
        ->condition('cid', $comment->cid)
        ->execute();
    }
    $this->complete($comment, $row);
    if (isset($comment->cid) && $comment->cid > 0) {
      $return = array($comment->cid);
      if ($updating) {
        $this->numUpdated++;
      }
      else {
        $this->numCreated++;
      }
    }
    else {
      $return = FALSE;
    }
    return $return;
  }
}
