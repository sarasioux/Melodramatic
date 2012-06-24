<?php
// vim:filetype=php expandtab tabstop=2 softtabstop=2 shiftwidth=2 autoindent smartindent
// $Id: tribune.history.inc,v 1.7.2.2 2008/12/01 10:19:57 seeschloss Exp $

function tribune_history_page($node) {
  return drupal_get_form('tribune_history_form_date',   $node).
         drupal_get_form('tribune_history_form_user',   $node).
         drupal_get_form('tribune_history_form_string', $node);
}

function tribune_history_form_date($form_id, $node) {
  $form = array();

  $form['tribune_id'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );

  $form['by-date'] = array(
    '#type' => 'fieldset',
    '#title' => t('Messages by date'),
    '#description' => t('See all the messages posted at a specific date.'),
  );
  $form['by-date']['by-date-date'] = array(
    '#type' => 'date',
  );
  $form['by-date']['by-date-submit'] = array(
    '#type' => 'submit',
    '#value' => t('By date'),
    '#submit' => array('tribune_history_by_date_handler'),
  );

  return $form;
}

function tribune_history_form_user($form_id, $node) {
  $form = array();

  $form['tribune_id'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );

  $form['by-user'] = array(
    '#type' => 'fieldset',
    '#title' => t('Messages by user'),
    '#description' => t('See all the messages posted by an user.'),
  );
  $form['by-user']['by-user-user'] = array(
    '#type' => 'textfield',
    '#autocomplete_path' => 'user/autocomplete',
  );
  $form['by-user']['by-user-submit-registered'] = array(
    '#prefix' => '<div class="container-inline">',
    '#type' => 'submit',
    '#value' => t('Registered user'),
    '#submit' => array('tribune_history_by_user_registered_handler'),
  );
  $form['by-user']['by-user-submit-anonymous'] = array(
    '#type' => 'submit',
    '#value' => t('Anonymous user'),
    '#suffix' => '</div>',
    '#submit' => array('tribune_history_by_user_anonymous_handler'),
  );

  return $form;
}

function tribune_history_form_string($form_id, $node) {
  $form = array();

  $form['tribune_id'] = array(
    '#type' => 'value',
    '#value' => $node->nid,
  );

  $form['by-string'] = array(
    '#type' => 'fieldset',
    '#title' => t('Search messages'),
    '#description' => t('Search messages containing a string.'),
  );
  $form['by-string']['by-string-string'] = array(
    '#type' => 'textfield',
  );
  $form['by-string']['by-string-submit'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
    '#submit' => array('tribune_history_by_string_handler'),
  );

  return $form;
}

function tribune_history_by_date_handler($form) {
  $tribune_id = $form['tribune_id']['#value'];
  $date = mktime(0, 0, 0, $form['#post']['by-date-date']['month'], $form['#post']['by-date-date']['day'], $form['#post']['by-date-date']['year']);

  header("Location: ". url('node/'. $tribune_id .'/history/date/'. date("Ymd", $date)));
  exit();
}

function tribune_history_by_user_registered_handler($form) {
  $tribune_id = $form['tribune_id']['#value'];
  header("Location: ". url('node/'. $tribune_id .'/history/user/'. $form['#post']['by-user-user']));
  exit();
}

function tribune_history_by_user_anonymous_handler($form) {
  $tribune_id = $form['tribune_id']['#value'];
  header("Location: ". url('node/'. $tribune_id .'/history/anonymous/'. $form['#post']['by-user-user']));
  exit();
}

function tribune_history_by_string_handler($form) {
  $tribune_id = $form['tribune_id']['#value'];
  header("Location: ". url('node/'. $tribune_id .'/history/search/'. $form['#post']['by-string-string']));
  exit();
}

function tribune_history_show_posts($tribune_id, $posts, $node) {
  drupal_add_css(drupal_get_path('module', 'tribune') .'/css/tribune.page.css');

  $tribune_id = "tribune-page";
  drupal_add_js(array(
    "tribune" => array(
      "tribunes" => array(
        $tribune_id => array(
          "reload_delay" => 0,
          "block" => FALSE,
          "path" => array(
            "post" => url("tribune/post"),
            "json_post" => url("tribune/post-json"),
            "json_posts" => url("tribune/newposts-json"),
            "filters" => url("tribune/filter"),
          ),
        ),
      ),
    ),
  ), "setting");

  drupal_add_js(drupal_get_path('module', 'tribune') .'/js/tribune.ajax.js');
  drupal_add_js('misc/jquery.form.js');
  foreach(_tribune_get_filters_help($node) as $filter => $function) {
    $function();
  }
  $date = strtotime($date);

  $contents = "<ul class='tribune-posts tribune-page' id='ul-". $tribune_id ."'>\n";

  $user  = user_load_self(array());
  $login = $user[1]->name;

  if (sizeof($posts) > 0) foreach ($posts as $post) {
    if (!$post['moderated'] || tribune_access("moderate tribune", $node)) {
      $contents .= theme("tribune_post", $post, $node) ."\n";

      $_tribune_current_post_id = -1;
    }
  }

  $contents .= "</ul>\n";

  return $contents;
}

function tribune_history_select_posts($node, $where, $arguments) {
  $history_size = $node->tribune_settings['history_size'];

  $posts = array();

  array_unshift($arguments, $node->nid);

  $r = pager_query("SELECT *
    FROM {tribune} 
    WHERE tribune_id = %d
    AND ". $where,
    $history_size,
    0,
    NULL,
    $arguments
  );

  while ($row = db_fetch_array($r)) {
    $posts[$row['post_id']] = tribune_post_from_row($row);
  }

  return $posts;
}

/**
  * Returns all the posts for a day, in a formatted string
  * @param $date Show the posts at this date
  * @return String containing the posts in an HTML unordered list
  */
function tribune_history_by_date($date, $node) {
  $tribune_id = "tribune-history-date-". check_plain($date);

  $posts = tribune_history_select_posts($node,
    "SUBSTRING(post_time FROM 1 FOR 8) = '%s'
    ORDER BY post_id ASC",
    array(
      $date
    )
  );

  return tribune_history_show_posts($tribune_id, $posts, $node).theme("pager", array(), $history_size);
}

/**
  * Returns all the posts for an user, in a formatted string
  * @param $user Show the posts from this user
  * @return String containing the posts in an HTML unordered list
  */
function tribune_history_by_user($user, $node) {
  $tribune_id = "tribune-history-user-". check_plain($user);
  $history_size = $node->tribune_settings['history_size'];

  $posts = tribune_history_select_posts($node,
    "login = '%s'
    ORDER BY post_id DESC",
    array(
      $user
    )
  );

  return tribune_history_show_posts($tribune_id, $posts, $node).theme("pager", array(), $history_size);
}

function tribune_history_by_user_anonymous($user, $node) {
  $tribune_id = "tribune-history-user-". check_plain($user);
  $history_size = $node->tribune_settings['history_size'];

  $user = '%'. str_replace('*', '%', $user) .'%';

  $posts = tribune_history_select_posts($node,
    "info LIKE '%s'
    AND login = ''
    ORDER BY post_id DESC",
    array(
      $user
    )
  );

  return tribune_history_show_posts($tribune_id, $posts, $node).theme("pager", array(), $history_size);
}

function tribune_highlight_string($needle, $haystack) {
  if (preg_match('/<a.*href=.*'. $needle .'.*>/iu', $haystack)) {
    return $haystack;
  }
  else {
    return preg_replace('/'. $needle .'/iu', '<strong>\0</strong>', $haystack);
  }
}

function tribune_history_by_string($string, $node) {
  $tribune_id = "tribune-history-string-". check_plain($string);
  $history_size = $node->tribune_settings['history_size'];

  $string_sql = '%'. str_replace('*', '%', $string) .'%';

  $posts = tribune_history_select_posts($node,
    "message LIKE '%s'
    ORDER BY post_id DESC",
    array(
      $string_sql
    )
  );

  $string_regexp = str_replace('*', '.*', $string);

  foreach ($posts as $id => $post) {
    $posts[$id]['parsed_message'] = tribune_highlight_string($string_regexp, $post['parsed_message']);
  }

  return tribune_history_show_posts($tribune_id, $posts, $node).theme("pager", array(), $history_size);
}

function tribune_history_search($node, $type, $argument) {
  switch ($type) {
    case 'date':
      return tribune_history_by_date($argument, $node);
      break;
    case 'user':
      return tribune_history_by_user($argument, $node);
      break;
    case 'anonymous':
      return tribune_history_by_user_anonymous($argument, $node);
      break;
    case 'search':
      return tribune_history_by_string($argument, $node);
      break;
  }
}

