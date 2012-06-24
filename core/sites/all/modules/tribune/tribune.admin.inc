<?php
// vim:filetype=php expandtab tabstop=2 softtabstop=2 shiftwidth=2 autoindent smartindent
// $Id: tribune.admin.inc,v 1.6.2.4 2009/04/12 16:56:53 seeschloss Exp $

function tribune_admin() {
  drupal_set_title(t("Tribune settings"));
  $breadcrumb = drupal_get_breadcrumb();
  $breadcrumb[] = l(t("Tribune settings"), "admin/settings/tribune");
  drupal_set_breadcrumb($breadcrumb);

  $form['tribune_max_message_size'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Maximum length of a comment'),
    '#default_value'  => variable_get('tribune_max_message_size', 512),
    '#size'           => 4,
    '#maxlength'      => 4,
    '#description'    => t('The maximum length of a comment.'),
  );

  $form['tribune_history_size'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Number of comments displayed'),
    '#default_value'  => variable_get('tribune_history_size', 25),
    '#size'           => 3,
    '#maxlength'      => 3,
    '#description'    => t('The number of comments to display.'),
  );

  $form['tribune_history_block_size'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Number of comments displayed in the block'),
    '#default_value'  => variable_get('tribune_history_block_size', 10),
    '#size'           => 3,
    '#maxlength'      => 3,
    '#description'    => t('The number of comments to display in the block.'),
  );

  $form['tribune_xml_size'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Number of comments displayed in the XML backend'),
    '#default_value'  => variable_get('tribune_xml_size', 100),
    '#size'           => 3,
    '#maxlength'      => 3,
    '#description'    => t('The number of comments to display in the XML backend.'),
  );

  $form['tribune_rss_size'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Number of comments displayed in the RSS feed'),
    '#default_value'  => variable_get('tribune_rss_size', 50),
    '#size'           => 3,
    '#maxlength'      => 3,
    '#description'    => t('The number of comments to display in the RSS feed.'),
  );

  $form['tribune_reload_delay'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Reload delay'),
    '#default_value'  => variable_get('tribune_reload_delay', 30),
    '#size'           => 3,
    '#maxlength'      => 3,
    '#description'    => t('The delay between automatic reloading of the tribune, in seconds (0 to disable Ajax reloading).'),
  );

  $form['tribune_idle_delay'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Idle delay'),
    '#default_value'  => variable_get('tribune_idle_delay', 0),
    '#size'           => 5,
    '#maxlength'      => 5,
    '#description'    => t('The delay before the tribune stop automatically reloading when an user is inactive, in seconds (0 to disable idle mode).'),
  );

  $form['tribune_name'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Tribune name'),
    '#default_value'  => variable_get('tribune_name', variable_get('site_name', '')),
    '#description'    => t('The name of your tribune, needed for people who use Coincoins.'),
  );

  $form['tribune_message'] = array(
    '#type'           => 'textarea',
    '#title'          => t('Tribune message'),
    '#default_value'  => variable_get('tribune_message', ''),
    '#description'    => t('Warning or information to display at the top of the tribune. You can use any HTML code you want, but you have to insert linebreaks yourself.'),
  );

  $form['tribune_flood_protection_delay'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Flood protection'),
    '#default_value'  => variable_get('tribune_flood_protection_delay', 3),
    '#size'           => 3,
    '#maxlength'      => 3,
    '#description'    => t('Minimum time before someone can post again, in seconds.'),
  );

  $form['tribune_order'] = array(
    '#type'           => 'radios',
    '#title'          => t('Message order'),
    '#default_value'  => variable_get('tribune_order', 'top_to_bottom'),
    '#options'        => array('top_to_bottom' => t("Last messages at the bottom"), 'bottom_to_top' => t("Last messages at the top")),
    '#description'    => t('How the messages are displayed. Displaying the messages top-to-bottom makes it easier to follow real discussions, but displaying the latest one at the top is sometimes less confusing.'),
  );

  $form['tribune_pager'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Show pager'),
    '#default_value'  => variable_get('tribune_pager', FALSE),
    '#description'    => t('Show a pager on the main page.'),
  );

  $form['tribune_antispam'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Insert a hidden antispam field'),
    '#default_value'  => variable_get('tribune_antispam', TRUE),
    '#description'    => t('An antispam field (called "email" and hidden through CSS) will be added to the post form. If it is not empty, the post will be ignored, this usually works very well against spammers.'),
  );

  return system_settings_form($form);
}

function tribune_admin_validate($form_id, $form_values) {
  if (!is_numeric($form_values['values']['tribune_max_message_size'])) {
    form_set_error("tribune_max_message_size", t("The maximum comment length must be an integer"));
  }

  if (!is_numeric($form_values['values']['tribune_history_size'])) {
    form_set_error("tribune_history_size", t("The number of comments displayed must be an integer"));
  }

  if (!is_numeric($form_values['values']['tribune_history_block_size'])) {
    form_set_error("tribune_history_block_size", t("The number of comments displayed in the block must be an integer"));
  }

  if (!is_numeric($form_values['values']['tribune_xml_size'])) {
    form_set_error("tribune_xml_size", t("The number of comments in the XML backend must be an integer"));
  }

  if (!is_numeric($form_values['values']['tribune_reload_delay'])) {
    form_set_error("tribune_reload_delay", t("The reload delay must be a time in seconds"));
  }

  if (!is_numeric($form_values['values']['tribune_idle_delay'])) {
    form_set_error("tribune_idle_delay", t("The idle delay must be a time in seconds"));
  }

  if (strlen($form_values['values']['tribune_message']) > 2048) {
    $form_values['values']['tribune_message'] =
      substr($form_values['values']['tribune_message'], 0, 2048);
  }

  if (!is_numeric($form_values['values']['tribune_flood_protection_delay'])) {
    form_set_error("tribune_flood_protection_delay", t("The flood protection delay must be a time in seconds"));
  }
}

function tribune_admin_migration() {
  $form = array();

  $form['migration'] = array(
    '#type' => 'fieldset',
    '#title' => t('Tribune migration'),
    '#description' => t('You have recently updated your tribune from 1.x to
    2.x. Tribunes are now nodes, and posts from your former tribune are not
    yet associated with any tribune node. This page allows you to move all
    your existing posts to a new tribune node, or alternatively to
    delete all your existing posts if you do not want to keep them.'),
  );

  $form['migration']['create_node'] = array(
    '#type' => 'checkbox',
    '#disabled' => TRUE,
    '#value' => TRUE,
    '#title' => t('Create a new tribune node'),
    '#description' => t('You have not created any tribune node yet. A new
    node will be created before moving all the existing posts into it.'),
  );

  $form['migration']['set_alias'] = array(
    '#type' => 'checkbox',
    '#default_value' => TRUE,
    '#title' => t('Set path alias /tribune -> new node'),
    '#description' => t('You have the path module activated, if this
    box is checked, an alias will be set so that the "/tribune" path will
    work and show your new tribune node. Otherwise, your new tribune will
    be accessible only with its node/<em>123</em> path. You can always
    change this later.'),
  );
  if (!function_exists('path_set_alias')) {
    $form['migration']['set_alias']['#value'] = FALSE;
    $form['migration']['set_alias']['#disabled'] = TRUE;
    $form['migration']['set_alias']['#description'] .= ' '. t('This option is not available to you because the path module is not activated.');
  }

  $form['migration']['migrate'] = array(
    '#type' => 'submit',
    '#value' => t('Migrate posts'),
    '#submit' => array('tribune_admin_migration_submit'),
  );

  $form['#redirect'] = FALSE;

  return $form;
}

function tribune_admin_migration_submit(&$form, $form_state) {
  if ($form_state['values']['create_node']) {
    global $user;
    $node = new stdClass();

    $node->nid = 0;
    $node->type = 'tribune';
    $node->uid = $user->uid;
    $node->status = 1;
    $node->created = time();

    $node->title = variable_get('tribune_name', variable_get('site_name', ''));
    $node->body = variable_get('tribune_message', '');
    $node->body_filter = 2;

    $node->tribune_settings['max_message_size'] = variable_get('tribune_max_message_size', 512);
    $node->tribune_settings['history_size'] = variable_get('tribune_history_size', 25);
    $node->tribune_settings['xml_size'] = variable_get('tribune_xml_size', 100);
    $node->tribune_settings['rss_size'] = variable_get('tribune_rss_size', 50);
    $node->tribune_settings['reload_delay'] = variable_get('tribune_reload_delay', 30);
    $node->tribune_settings['idle_delay'] = variable_get('tribune_idle_delay', 0);
    $node->tribune_settings['flood_protection_delay'] = variable_get('tribune_flood_protection_delay', 3);
    $node->tribune_settings['posts_order'] = (variable_get('tribune_posts_order', 'top_to_bottom') == 'top_to_bottom');
    $node->tribune_settings['show_pager'] = variable_get('tribune_show_pager', FALSE);
    $node->tribune_settings['enabled_filters'] = variable_get('tribune_enabled_filters', array('totoz' => 'totoz', 'url' => 'url'));

    node_save($node);

    if ($form_state['values']['set_alias']) {
      path_set_alias('node/'. $node->nid, 'tribune');
      path_set_alias('node/'. $node->nid .'/tribune.xml', 'tribune.xml');
    }

    db_query('UPDATE {tribune} SET tribune_id = %d WHERE tribune_id = 0', $node->nid);

    drupal_set_message(t('Your tribune node was created, you can see it here: !url. Do not forget to set the !permissions_url if you want users to have access to your tribunes. If you had a tribune block, you will have to set it again.', array('!url' => l($node->title, 'node/'. $node->nid), '!permissions_url' => l('permissions', 'admin/user/permissions'))));
    header('Location: '. url('node/'. $node->nid));
  }
}

