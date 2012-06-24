<?php
// vim:filetype=php expandtab tabstop=2 softtabstop=2 shiftwidth=2 autoindent smartindent
// $Id: tribune.moderation.inc,v 1.4.2.1 2008/12/01 08:21:52 seeschloss Exp $

function tribune_delete_post($id, $node) {
  db_query("UPDATE {tribune}
         SET last_modified = %d, moderated = 1
         WHERE post_id = %d
           AND tribune_id = %d",
          time(),
          $id,
          $node->nid);
  return t("Post number !id was moderated", array("!id" => $id));
}

function tribune_undelete_post($id, $node) {
  db_query("UPDATE {tribune}
         SET last_modified = %d, moderated = 0
         WHERE post_id = %d
           AND tribune_id = %d",
          time(),
          $id,
          $node->nid);
  return t("Post number !id was restored", array("!id" => $id));
}

function tribune_moderation_form($form, $node) {
  $form = array();

  $form['tribune_id'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );

  $form['ban_expressions'] = array(
    '#type'       => 'fieldset',
    '#description'    => t("All these boxes can contain any number of regular expressions (parsed with <code>!php_url</code>), one pattern per line. Posts which match any of these patterns will be silently dropped (the patterns are not case-sensitive).<br />For example, <code>Mozilla/4</code> in the 'Banned useragents' box will ban all Internet Explorer users.", array('!php_url' => l('preg_match', 'http://php.net/preg_match'))),
  );
  $form['ban_expressions']['banned_useragents'] = array(
    '#type'       => 'textarea',
    '#title'      => t("Banned useragents"),
    '#default_value'  => tribune_get_option($node, 'banned_useragents'),
  );
  $form['ban_expressions']['banned_logins'] = array(
    '#type'       => 'textarea',
    '#title'      => t("Banned usernames"),
    '#default_value'  => tribune_get_option($node, 'banned_usernames'),
  );
  $form['ban_expressions']['banned_messages'] = array(
    '#type'       => 'textarea',
    '#title'      => t("Banned expressions"),
    '#default_value'  => tribune_get_option($node, 'banned_messages'),
  );
  $form['ban_expressions']['submit'] = array(
    '#type'     => 'submit',
    '#value'    => t('Save'),
  );

  if (tribune_access("delete moderated posts", $node)) {
    $nb_moderated = db_result(db_query("SELECT COUNT(*) FROM {tribune} WHERE moderated = 1 AND tribune_id = %d", array($node->nid)));;
    $form['delete_moderated'] = array(
      '#type'     => 'fieldset',
      '#title'    => t("Delete all moderated messages"),
      '#collapsible'  => TRUE,
      '#collapsed'  => TRUE,
      '#description'  => t('All currently moderated messages will be definitively deleted from the database. They will not disappear from the moderators\'s opened tribunes however, this is a limitation of Ajax reloading, but trying to restore them will not do anything. There are currently <strong>!nb</strong> moderated messages.', array('!nb' => $nb_moderated)),
    );
    $form['delete_moderated']['delete_moderated'] = array(
      '#type'     => 'submit',
      '#value'    => t('Delete all moderated messages'),
      '#submit'   => array('tribune_delete_moderated_messages'),
    );
  }

  if (tribune_access("delete all posts", $node)) {
    $form['purge_tribune'] = array(
      '#type'     => 'fieldset',
      '#title'    => t("Delete all messages"),
      '#collapsible'  => TRUE,
      '#collapsed'  => TRUE,
      '#description'  => t('<strong>All</strong> the tribune messages will be <strong>definitively</strong> deleted, use with caution.'),
    );
    $form['purge_tribune']['purge_tribune'] = array(
      '#type'     => 'submit',
      '#value'    => t('Delete all messages'),
      '#submit'   => array('tribune_delete_all_messages'),
    );
  }

  return $form;
}

function tribune_moderation_form_submit($form, $form_values) {
  $tribune_id = $form_values['values']['tribune_id'];

  tribune_set_option($tribune_id, "banned_useragents", $form_values['values']['banned_useragents']);
  tribune_set_option($tribune_id, "banned_usernames",  $form_values['values']['banned_usernames'] );
  tribune_set_option($tribune_id, "banned_messages",   $form_values['values']['banned_messages']  );

  drupal_set_message(t("The banned patterns have been updated."), "status");
}

function tribune_delete_moderated_messages($form, $form_values) {
  $tribune_id = $form_values['values']['tribune_id'];

  db_query("DELETE FROM {tribune}
                  WHERE moderated = 1
                    AND tribune_id = %d",
                  array($tribune_id)
                );

  drupal_set_message(t("All moderated messages have been deleted."), "status");
}

function tribune_delete_all_messages() {
  $tribune_id = $form_values['values']['tribune_id'];

  db_query("DELETE FROM {tribune}
                  WHERE tribune_id = %d",
                  array($tribune_id)
                );

  drupal_set_message(t("ALL the messages have been deleted."), "status");
}

