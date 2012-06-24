<?php
// vim:filetype=php expandtab tabstop=2 softtabstop=2 shiftwidth=2 autoindent smartindent
// $Id: tribune.admin.filters.inc,v 1.7.2.2 2009/03/29 18:40:18 seeschloss Exp $

function tribune_admin_filters($node = NULL) {
  if (!$node) {
    drupal_set_message(t('These settings are only used for tribunes owned by
      users who do not have the permission to configure filters, and are not
      used at all for tribunes owned by users who do have this permission.'));
  }

  $output .= drupal_get_form("tribune_admin_filters_form", $node, "before");
  $output .= drupal_get_form("tribune_admin_filters_form", $node, "after");
  $output .= drupal_get_form("tribune_admin_filters_reparse_form", $node);
  return $output;
}

function theme_tribune_admin_filters_form($form) {
  $headers = array(
    array('data' => t('Enabled'), 'class' => "checkbox"),
    t('Name'),
    t('Settings'),
    t('Description'),
  );

  $rows = array();
  foreach (element_children($form['filters']) as $filter_name) {
    if (isset($form['filters'][$filter_name]['status'])) {
      $element = &$form['filters'][$filter_name];
      $row = array();
      $row[] = array('data' => drupal_render($element['status']), 'class' => "checkbox");
      $row[] = array('data' => drupal_render($element['name']));
      $row[] = array('data' => drupal_render($element['settings']));
      $row[] = array('data' => drupal_render($element['description']), 'class' => "description");

      $rows[] = $row;
    }
  }

  $output = "";
  if ($rows) {
    $output .= theme("table", $headers, $rows);
  }

  $output .= drupal_render($form);

  $fieldset = array(
    '#type' => "fieldset",
    '#title' => $form['title']['#value'],
    '#description' => $form['description']['#value'],
  );
  $fieldset['contents'] = array(
    '#value' => $output,
  );

  return drupal_render($fieldset);
}

function tribune_admin_filters_form($form, $node, $execution_time = "before") {
  $filters = _tribune_filters_info();
  $all_filters = array();
  
  if (isset($node) and is_array($node->tribune_settings['enabled_filters'])) {
    $enabled_filters = $node->tribune_settings['enabled_filters'];
  } else {
    $enabled_filters = variable_get('tribune_enabled_filters', array('totoz' => "totoz", "url" => "url"));
  }

  $form = array('#tree' => TRUE);

  foreach ($filters as $filter_name => $attributes) {
    if (($execution_time == "before" and $attributes['before_slip'])
     or ($execution_time == "after"  and $attributes['after_slip'])) {
      $enabled = (isset($enabled_filters[$filter_name]) and $enabled_filters[$filter_name]);

      $all_filters[] = $filter_name;

      $row = array();
      $row['status'] = array(
        '#type' => 'checkbox',
        '#return_value' => TRUE,
        '#default_value' => $enabled,
      );
      $row['name'] = array(
        '#value' => $filter_name,
      );
      if ($node) {
        $settings_path = 'node/'. $node->nid .'/filters/settings/'. $filter_name;
      } else {
        $settings_path = 'admin/settings/tribune/filters/settings/'. $filter_name;
      }
      $row['settings'] = array(
        '#value' => $enabled ? $filters[$filter_name]['settings'] ? l(t('Settings'), $settings_path) : t('None') : "",
      );
      $row['description'] = array(
        '#value' => $filters[$filter_name]['description'],
      );

      $form['filters'][$filter_name] = $row;
    }
  }
  $form['title'] = array(
    '#type' => "value",
    '#value' => $execution_time == "before" ? t("Filters run before the <em>slip</em>") : t("Filters run after the <em>slip</em>"),
  );
  $form['description'] = array(
    '#type' => "value",
    '#value' => $execution_time == "before" ? t("These filters can only use limited HTML, and can post new messages.") : t("These filters can use full HTML, but can not post new messages."),
  );

  $form['filters-list'] = array(
    '#type' => 'hidden',
    '#value' => serialize($all_filters),
  );

  if ($node) {
    $form['tribune_id'] = array(
      '#type' => 'value',
      '#value' => $node->nid,
    );
  }

  $form['submit'] = array(
    '#type' => "submit",
    '#value' => t("Save"),
    '#submit' => array("tribune_admin_filters_enable_disable"),
  );

  return $form;
}

function tribune_admin_filters_enable_disable($form, $f) {
  $node = NULL;
  if (isset($form['tribune_id'])) {
    $node = node_load($form['tribune_id']['#value']);
    $enabled_filters = $node->tribune_settings['enabled_filters'] ? $node->tribune_settings['enabled_filters'] : array('totoz' => "totoz", "url" => "url");
  } else {
    $enabled_filters = variable_get('tribune_enabled_filters', array('totoz' => "totoz", "url" => "url"));
  }

  $all_filters = unserialize($form['#post']['filters-list']);

  if (is_array($form['#post']['filters'])) foreach ($form['#post']['filters'] as $filter_name => $filter) {
    if (isset($filter['status']) and $filter['status']) {
      if (!isset($enabled_filters[$filter_name]) or !$enabled_filters[$filter_name]) {
        $enabled_filters[$filter_name] = $filter_name;
        drupal_set_message(t("The '@filter_name' filter has been enabled.", array('@filter_name' => $filter_name)), "status");
      }
    }
  }

  if (is_array($all_filters)) foreach ($all_filters as $filter_name) {
    if (!isset($form['#post']['filters'][$filter_name]) and isset($enabled_filters[$filter_name])) {
      unset($enabled_filters[$filter_name]);
      drupal_set_message(t("The '@filter_name' filter has been disabled.", array('@filter_name' => $filter_name)), "status");
    }
  }

  if ($node) {
    $node->tribune_settings['enabled_filters'] = $enabled_filters;
    node_save($node);
  } else {
    variable_set('tribune_enabled_filters', $enabled_filters);
  }
}

function tribune_admin_filters_reparse_form($form, $node) {
  $form = array();

  $form['nid'] = array(
    '#type'   => 'value',
    '#value'  => $node->nid,
  );
  $form['tribune_reparse_messages'] = array(
    '#type'   => 'submit',
    '#value'  => t('Parse visible messages again'),
    '#submit' => array('tribune_admin_parse_last_messages'),
  );

  return $form;
}

function tribune_admin_parse_last_messages($form, $p) {
  $node = node_load($form['nid']['#value']);

  tribune_parse_last_messages($node);
}

function tribune_filter_settings($node, $filter_name) {
  global $tribune;
  $tribune = $node;

  $filters = _tribune_filters_info();

  if (isset($node) and is_array($node->tribune_settings['enabled_filters'])) {
    $enabled_filters = $node->tribune_settings['enabled_filters'];
  } else {
    $enabled_filters = variable_get('tribune_enabled_filters', array('totoz' => "totoz", "url" => "url"));
  }

  if (isset($enabled_filters[$filter_name]) and isset($filters[$filter_name]['settings'])) {
    if (!$node) {
      drupal_set_title(t("Filter settings &mdash; ". $filter_name));
      $breadcrumb = drupal_get_breadcrumb();
      $breadcrumb[] = l(t("Filter settings"), "admin/settings/tribune/filters");
      $breadcrumb[] = l($filter_name, "admin/settings/tribune/filters/". $filter_name);
      drupal_set_breadcrumb($breadcrumb);
    }

    $function = $filters[$filter_name]['settings'];

    return drupal_get_form($function, $node);
  }

  drupal_not_found();
}

