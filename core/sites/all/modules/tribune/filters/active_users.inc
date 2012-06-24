<?php
// vim:filetype=php expandtab tabstop=2 softtabstop=2 shiftwidth=2 autoindent smartindent
// $Id: active_users.inc,v 1.2.2.1 2008/12/01 08:21:53 seeschloss Exp $

function tribune_active_users_info() {
  return t('Show active users (with the <code>/users</code>) command.');
}

function tribune_active_users_filter(&$post, &$help) {
  global $tribune;

  if (preg_match(':^/users?$:', $post['message'], $matches)) {
    $nicks = array();
    foreach (tribune_get_active_users($tribune) as $key => $data) {
      if ($data['uid']) {
        $nicks[] = $data['name'];
      } else {
        $nicks[] = "<i>".$data['name']."</i>";
      }
    }

    $new_message = array(
      'info' => tribune_variable_get('tribune_active_users_name', 'Users'),
      'message' => tribune_filters_print_clock($post)." ".t("Active users: ").implode(', ', $nicks),
    );

    if (tribune_variable_get('tribune_active_users_authentified', FALSE)) {
      $new_message['login'] = $new_message['info'];
    }

    return array($new_message);
  }
}

function tribune_active_users_settings($form) {
  $form = array();

  $form['tribune_active_users_name'] = array(
    '#type' => "textfield",
    '#title' => t("Display name"),
    '#default_value' => tribune_variable_get('tribune_active_users_name', 'Users'),
  );

  $form['tribune_active_users_authentified'] = array(
    '#type' => "checkbox",
    '#title' => t("Appear to be authentified"),
    '#default_value' => tribune_variable_get('tribune_active_users_authentified', FALSE),
    '#description' => t("Whether this filter will appear to be 'anonymous' or 'authentified'. Since the filter does not correspond to a real user, posting as authentified may result in 'nickname collisions'."),
  );

  return tribune_settings_form($form);
}

function tribune_active_users_help() {
  return 'Type "/users" to see a list of users viewing this tribune.';
}
