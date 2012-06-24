<?php
// vim:filetype=php expandtab tabstop=2 softtabstop=2 shiftwidth=2 autoindent smartindent
// $Id: nick.inc,v 1.12.2.1 2008/12/01 08:21:53 seeschloss Exp $

function tribune_nick_info() {
  return t('Allows anonymous users to change their userinfo.');
}

function tribune_nick_filter(&$post, &$help) {
  if (preg_match(':^/nick(.+)?$:', $post['message'], $matches)) {
    if ($matches[1]) {
      $_SESSION['tribune_nickname'] = trim($matches[1]);
    }
    else {
      $tribune_active_users = tribune_variable_get('tribune_active_users', array());
      foreach ($tribune_active_users as $key => $data) {
        if ($data['name'] == check_plain($_SESSION['tribune_nickname'])) {
          unset ($tribune_active_users[$key]);
        }
      }
      variable_set('tribune_active_users', $tribune_active_users);
      unset($_SESSION['tribune_nickname']);
    }
    $post = FALSE;

    if (isset($_SESSION['tribune_nickname']) and $_SESSION['tribune_nickname']) {
      $help = t('Type "/nick" to reset your display name (which is <em>@nickname</em>), or "/nick <em>aragua</em>" to change it.',
          array('@nickname' => $_SESSION['tribune_nickname']));
    }
    else {
      $help = t('Type "/nick <em>otine</em>" to change your display name to "otine" (or register/login).');
    }
  }

  if ($post and isset($_SESSION['tribune_nickname'])) {
    $post['info'] = $_SESSION['tribune_nickname'];
  }
}

function tribune_nick_help() {
  $user = array_shift(user_load_self(array()));
  if ($user->uid) {
    return '';
  }
  else {
    return 'Type "/nick <em>otine</em>" to change your display name to "otine" (or register/login).';
  }
}
