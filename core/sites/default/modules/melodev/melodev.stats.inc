<?php

/**
 * @file
 * Special functionality for stats on Melo.
 *
 * Trying to build on the existing accesslog system,
 * this file contains functions that do additional logging
 * and tweaking required for Melo's purposes.
 */


/*  Stats System  */

/**
 * @defgroup melodev_statshooks Melodev Stats Hooks
 *
 * Implemented hooks for tracking Melo stats.
 *
 * @{
 */
 
/**
 * Implementation of hook_user().
 */
function melodev_stats_user($op, &$edit, &$account, $category = NULL) {

  // Create URLs
  switch($op) {
    case 'view':
      global $user;
      melodev_userhit($user->uid, $account->uid);
    break;
  }
  
}

/**
 * Implementation of hook_nodeapi();
 */
function melodev_stats_nodeapi(&$node, $op, $teaser, $page) {
  switch ($op) {
    case 'insert':
      melodev_save_stat($node->uid, $node->type);
      break;
    case 'delete':
      melodev_save_stat($node->uid, $node->type, -1);
      break;
    case 'update':
      // @TODO:  Add something to check if node authorship changed
      break;
    case 'view':
      switch($node->type) {
        case 'folder':
        case 'blog':
        case 'photo':
        case 'favorite':
          global $user;
          melodev_userhit($user->uid, $node->uid);
        break;
      }
      break;
  }
}

/**
 * Implementation of hook_comment().
 */
function melodev_stats_comment($comment, $op) {
  global $user;
  switch ($op) {
    case 'insert':
      melodev_save_stat($user->uid, 'comment');
      break;
    case 'delete':
      melodev_save_stat($comment->uid, 'comment', -1);
      break;
  }
}

/**
 * Implementation of hook_touchapi().
 */
function melodev_touchapi($uid, $id, $type) {
  // type "touch" means how many people i have touched
  melodev_save_stat($uid, $type->name);

  // type "touched" means people who have touched me
  $node = node_load($id);
  melodev_save_stat($node->uid, $type->name.'ed');
  
  // Save an access log of this
  db_query("INSERT INTO {accesslog} (title, path, url, hostname, uid, sid, timer, timestamp) values('%s', '%s', '%s', '%s', %d, '%s', %d, %d)", strip_tags($node->title), $_GET['q'], referer_uri(), ip_address(), $uid, session_id(), timer_read('page'), time());

}

/**
* @} End of "melodev_statshooks"
*/

/**
 * @defgroup melodev_statshelpers Melodev Stats Helpers
 *
 * Helper functions for the Melo Stats system.
 *
 * @{
 */
 
/**
 * Save a Melo Stat.
 *
 * Use overwrite for karma, instead of totalling up just replace every time.
 */
function melodev_save_stat($uid, $type, $count=1, $overwrite=false) {
  if($arr = db_fetch_array(db_query('select id from stats_today where uid = %d and type = "%s"', $uid, $type))) {
    if($count > 0) { $op = 'total +'; } else { $op = 'total -'; }
    if($overwrite == true) { $op = ''; }
    db_query('update stats_today set total = '.$op.' %d where id = %d', abs($count), $arr['id']);
  } else {
    $account = user_load($uid);
    db_query('insert into stats_today (uid, name, type, total) values(%d, "%s", "%s", %d)', $uid, $account->name, $type, $count);
  }
}

/**
 * Get a Melo Stat.
 */
function melodev_get_stat($uid, $type) {
  if($arr = db_fetch_array(db_query('select total from stats_today where uid = %d and type = "%s"', $uid, $type))) {
    return $arr['total'];
  } else {
    return 0;
  }
}

/**
 * Load stats for one user.
 */
function melodev_stats_get_user($account) {
  $stats = array();
  
  // Today
  $stats['today'] = array();
  $result = db_query('select type, total from stats_today where uid = %d and type in(select distinct field_stats_arguments from {content_type_stat}) order by type asc', $account->uid);
  while($obj = db_fetch_object($result)) {
    $stats['today'][$obj->type] = number_format($obj->total);
  }
  
  // Alltime
  $stats['alltime'] = array();
  $result = db_query('select type, total from stats_alltime where uid = %d and type in(select distinct field_stats_arguments from {content_type_stat}) order by type asc', $account->uid);
  while($obj = db_fetch_object($result)) {
    $stats['alltime'][$obj->type] = number_format($obj->total);
  }
  
  return $stats;
  
}

/**
 * Get a stat label.
 * 
 * @TODO Make this admin setting configurable.
 */
function melodev_get_stat_label($type) {
  $labels = array(
    'blog' => t('Posts'),
    'hits' => t('Hits'),
    'virgin' => t('Virgins'),
    'comment' => t('Gspots'),
    'bang' => t('Bangs'),
    'touch' => t('Touches'),
    'banged' => t('Banged'),
    'touched' => t('Touched'),
    'karma' => t('Karma'),
    'folder' => t('Folders'),
    'favorite' => t('Favorites'),
    'poll' => t('Polls'),
    'cliquepost' => t('Clique Posts'),
    'color_scheme' => t('Colors'),
    'photo' => t('Photos'),
    'profile' => t('Profile'),
    'clique' => t('Cliques'),
    'fpart' => t('Front Page Art'),
    'profile_guestbook_html' => t('Disabling Guestbook HTML'),
    'profile_logged_in' => t('Disabling Anonymous Visitors'),
    'gifts' => t('Elite Gifts'),
    'subscriptions' => t('Elite Months'),
    'recurring' => t('Active Subscriptions'),
    'dollars' => t('Money Spent'),
    'friend' => t('Friends'),
    'enemy' => t('Enemies'),
    'tab' => t('Tabs'),
    'friendof' => t('Friend Of'),
    'enemyof' => t('Enemy Of'),
    'tabof' => t('Tab Of'),
    'spammers' => t('Spam Kills'),
    'years' => t('Years as a Member'),
  );
  if(isset($labels[$type])) {
    return $labels[$type];
  }
  return $type;
}

/**
 * Give a user a hit.
 */
function melodev_userhit($hitter_uid, $hit_uid) {
  // Can't give yourself hits
  if($hitter_uid != $hit_uid) {
    if($obj = db_fetch_object(db_query('select timestamp from {accesslog_hit_timer} where hitter_uid = %d and hit_uid = %d', $hitter_uid, $hit_uid))) {
      if($obj->timestamp < (mktime() - HIT_LIMIT_THRESHOLD)) {
        db_query('update {accesslog_hit_timer} set timestamp = %d where hitter_uid = %d and hit_uid = %d', mktime(), $hitter_uid, $hit_uid);
        melodev_save_stat($hit_uid, 'hits');
        return true;
      }
    } else {
      db_query('insert into {accesslog_hit_timer} (hitter_uid, hit_uid, timestamp) values(%d, %d, %d)', $hitter_uid, $hit_uid, mktime());
      melodev_save_stat($hit_uid, 'hits');
      return true;
    }
  }
  return false;
}

/**
* @} End of "melodev_statshelpers"
*/

/**
 * Implementation of hook_views_data()
 */
function melodev_stats_views_data() {

  // Today
  
  // Define the join fields
  $data['stats_today']['table']['group'] = t('Stats Today');
  $data['stats_today']['table']['join'] = array(
    // ...to the user table
    'users' => array(
      'left_field' => 'uid',
      'field' => 'uid',
    ),
  );
  // sid
  $data['stats_today']['id'] = array(
    'title' => t('Stat ID'),
    'help' => t('The id of this stat record.'),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'stats_today',
      'field' => 'id',
     ),
  );
  // owner uid
  $data['stats_today']['uid'] = array(
    'title' => t('User: UID'),
    'help' => t('The user id of the stat user.'),
     'filter' => array(
      'handler' => 'views_handler_filter_string',
     ),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'users',
      'field' => 'uid',
     ),
  );
  // type
  $data['stats_today']['type'] = array(
    'title' => t('Type'),
    'help' => t('The stat type.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
     'filter' => array(
      'handler' => 'views_handler_filter_string',
     ),
     'sort' => array(
      'handler' => 'views_handler_sort',
     ),
     'argument' => array(
      'handler' => 'views_handler_argument_string',
     ),
  );
  // total
  $data['stats_today']['total'] = array(
    'title' => t('Total'),
    'help' => t("Total value of the stat."), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'numeric' => TRUE,
      'name field' => 'points', // display this field in the summary
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  
  // Recent

  // Define the join fields
  $data['stats_recent']['table']['group'] = t('Stats Recent');
  $data['stats_recent']['table']['join'] = array(
    // ...to the user table
    'users' => array(
      'left_field' => 'uid',
      'field' => 'uid',
    ),
  );
  // sid
  $data['stats_recent']['id'] = array(
    'title' => t('Stat ID'),
    'help' => t('The id of this stat record.'),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'stats_recent',
      'field' => 'id',
     ),
  );
  // owner uid
  $data['stats_recent']['uid'] = array(
    'title' => t('User: UID'),
    'help' => t('The user id of the stat user.'),
     'filter' => array(
      'handler' => 'views_handler_filter_string',
     ),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'users',
      'field' => 'uid',
     ),
  );
  // type
  $data['stats_recent']['type'] = array(
    'title' => t('Type'),
    'help' => t('The stat type.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
     'filter' => array(
      'handler' => 'views_handler_filter_string',
     ),
     'sort' => array(
      'handler' => 'views_handler_sort',
     ),
     'argument' => array(
      'handler' => 'views_handler_argument_string',
     ),
  );
  // total
  $data['stats_recent']['total'] = array(
    'title' => t('Total'),
    'help' => t("Total value of the stat."), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'numeric' => TRUE,
      'name field' => 'points', // display this field in the summary
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // Alltime
  
  // Define the join fields
  $data['stats_alltime']['table']['group'] = t('Stats Alltime');
  $data['stats_alltime']['table']['join'] = array(
    // ...to the user table
    'users' => array(
      'left_field' => 'uid',
      'field' => 'uid',
    ),
  );
  // sid
  $data['stats_alltime']['id'] = array(
    'title' => t('Stat ID'),
    'help' => t('The id of this stat record.'),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'stats_alltime',
      'field' => 'id',
     ),
  );
  // owner uid
  $data['stats_alltime']['uid'] = array(
    'title' => t('User: UID'),
    'help' => t('The user id of the stat user.'),
     'filter' => array(
      'handler' => 'views_handler_filter_string',
     ),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'users',
      'field' => 'uid',
     ),
  );
  // type
  $data['stats_alltime']['type'] = array(
    'title' => t('Type'),
    'help' => t('The stat type.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
     ),
     'filter' => array(
      'handler' => 'views_handler_filter_string',
     ),
     'sort' => array(
      'handler' => 'views_handler_sort',
     ),
     'argument' => array(
      'handler' => 'views_handler_argument_string',
     ),
  );
  // total
  $data['stats_alltime']['total'] = array(
    'title' => t('Total'),
    'help' => t("Total value of the stat."), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'numeric' => TRUE,
      'name field' => 'points', // display this field in the summary
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );


  return $data;
}