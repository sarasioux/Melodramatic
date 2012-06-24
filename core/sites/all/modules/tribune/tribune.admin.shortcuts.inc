<?php
// vim:filetype=php expandtab tabstop=2 softtabstop=2 shiftwidth=2 autoindent smartindent
// $Id: tribune.admin.shortcuts.inc,v 1.1.2.1 2009/04/10 23:32:38 seeschloss Exp $

function tribune_admin_shortcuts($node) {
  $output  = tribune_admin_shortcuts_table($node);
  $output .= drupal_get_form("tribune_admin_shortcuts_form_new_text", $node);
  $output .= drupal_get_form("tribune_admin_shortcuts_form_new_wrap", $node);
  return $output;
}

function tribune_admin_shortcuts_delete($node, $shortcut_id) {
  unset($node->tribune_settings['shortcuts'][$shortcut_id]);
  tribune_update($node);
  drupal_goto("node/". $node->nid ."/shortcuts");
}

function tribune_admin_shortcuts_table($node) {
  $headers = array(
    t('Shortcut'),
    t('Text inserted'),
    t('Type'),
    "",
  );
  $rows = array();

  if (is_array($node->tribune_settings['shortcuts'])) foreach ($node->tribune_settings['shortcuts'] as $shortcut_id => $shortcut) {
    if (isset($shortcut['text'])) {
      $rows[] = array(
        array('data' => $shortcut['modifier'] .'+'. $shortcut['key']),
        array('data' => check_plain($shortcut['text'])),
        array('data' => t('text')),
        array('data' => l("Delete this shortcut", "node/". $node->nid ."/shortcuts/delete/". $shortcut_id)),
      );
    } else {
      $rows[] = array(
        array('data' => $shortcut['modifier'] .'+'. $shortcut['key']),
        array('data' => check_plain($shortcut['before']) .'<em>selection</em>'. check_plain($shortcut['after'])),
        array('data' => t('tag')),
        array('data' => l("Delete this shortcut", "node/". $node->nid ."/shortcuts/delete/". $shortcut_id)),
      );
    }
  }

  return theme("table", $headers, $rows);
}

function tribune_admin_shortcuts_form_new_text($form, $node) {
  $form = array();
  $form['fieldset'] = array(
    '#type' => "fieldset",
    '#title' => t("Add a simple text shortcut"),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#tree' => FALSE,
  );

  $form['fieldset']['modifier'] = array(
    '#type' => 'select',
    '#default_value' => "alt",
    '#options' => array("alt", "ctrl"),
    '#title' => t("Modifier"),
  );
  $form['fieldset']['key'] = array(
    '#type' => 'textfield',
    '#default_value' => "",
    '#maxlength' => 1,
    '#size' => 2,
    '#title' => t("Key"),
  );
  $form['fieldset']['text'] = array(
    '#type' => 'textfield',
    '#default_value' => "",
    '#size' => 20,
    '#title' => "text",
    '#description' => t("This text will be inserted when the key combination is pressed. If there is a selection, it will be replaced."),
  );
  $form['fieldset']['nid'] = array(
    '#type' => "value",
    '#value' => $node->nid,
  );
  $form['fieldset']['submit'] = array(
    '#type' => "submit",
    '#value' => t("Save"),
    '#submit' => array("tribune_admin_shortcuts_save_new_text"),
  );
  return $form;
}

function tribune_admin_shortcuts_save_new_text($form_id, $form_values) {
  $modifiers = array("alt", "ctrl");

  $modifier = $modifiers[$form_values['values']['modifier']];
  $key = $form_values['values']['key'];
  $text = $form_values['values']['text'];

  $node = node_load($form_values['values']['nid']);

  $node->tribune_settings['shortcuts'][] = array(
    'modifier' => $modifier,
    'key' => $key,
    'text' => $text,
  );
  tribune_update($node);
}

function tribune_admin_shortcuts_form_new_wrap($form, $node) {
  $form = array();
  $form['fieldset'] = array(
    '#type' => "fieldset",
    '#title' => t("Add a shortcut to insert a tag"),
    '#description' => t("These shortcuts will add text before and after the curent selection. If there is no selection, both parts are added and the cursor is set between the two parts."),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#tree' => FALSE,
  );

  $form['fieldset']['modifier'] = array(
    '#type' => 'select',
    '#default_value' => "alt",
    '#options' => array("alt", "ctrl"),
    '#title' => t("Modifier"),
  );
  $form['fieldset']['key'] = array(
    '#type' => 'textfield',
    '#default_value' => "",
    '#maxlength' => 1,
    '#size' => 2,
    '#title' => t("Key"),
  );
  $form['fieldset']['before'] = array(
    '#type' => 'textfield',
    '#default_value' => "",
    '#size' => 20,
    '#title' => "before",
    '#description' => t("This text will be inserted before the current selection."),
  );
  $form['fieldset']['after'] = array(
    '#type' => 'textfield',
    '#default_value' => "",
    '#size' => 20,
    '#title' => "after",
    '#description' => t("This text will be inserted after the current selection."),
  );
  $form['fieldset']['nid'] = array(
    '#type' => "value",
    '#value' => $node->nid,
  );
  $form['fieldset']['submit'] = array(
    '#type' => "submit",
    '#value' => t("Save"),
    '#submit' => array("tribune_admin_shortcuts_save_new_wrap"),
  );
  return $form;
}

function tribune_admin_shortcuts_save_new_wrap($form_id, $form_values) {
  $modifiers = array("alt", "ctrl");

  $modifier = $modifiers[$form_values['values']['modifier']];
  $key = $form_values['values']['key'];
  $before = $form_values['values']['before'];
  $after = $form_values['values']['after'];

  $node = node_load($form_values['values']['nid']);

  $node->tribune_settings['shortcuts'][] = array(
    'modifier' => $modifier,
    'key' => $key,
    'before' => $before,
    'after' => $after,
  );
  tribune_update($node);
}
