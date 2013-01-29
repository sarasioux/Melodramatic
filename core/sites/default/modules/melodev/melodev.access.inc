<?php

/**
 * @file
 * Defines access control and logging.
 *
 * Specific access functions have been stored
 * in this file to allow easy manipulation.
 */

/**
 * How long in between visits should a hit count.
 */
define('HIT_LIMIT_THRESHOLD', (60*10)); // 10 minutes

/**
 * Implementation of hook_node_grants().
 *
 * Tell the node access system what GIDs the user belongs to for each realm.
 */
function melodev_node_grants($account, $op) {
  if ($op == 'view') {
    if($account->uid > 0) {
      $enemies = melodev_load_enemy_bans($account->uid);
      $friends = array_diff(melodev_load_friended($account->uid), $enemies);
      if(!is_array($friends)) {
        $friends = array();
      }
      $grants['friends'] = $friends;
      $grants['friend_author'] = array($account->uid=>$account->uid);
      if(user_access('access all FO posts', $account)) {
        $grants['friend_admin'] = array(TRUE);
      }
      
      $grants['private_author'] = array($account->uid);
      if(user_access('access all private posts', $account)) {
        $grants['private_admin'] = array(TRUE);
      }
    }
    $grants['public'] = array(true);
  }
  return $grants;
}

/**
 * Implementation of hook_node_access_records().
 *
 * All node access modules must implement this hook. If the module is
 * interested in the privacy of the node passed in, return a list
 * of node access values for each grant ID we offer.
 */
function melodev_node_access_records($node) {
  // We only care about the node if it's been marked private or FO. If not, it is
  // treated just like any other node and we completely ignore it.
  
  $grants = array();
  
  if($node->permlevel == 'friends') {
    $grants[] = array(
      'realm' => 'friend_author',
      'gid' => $node->uid,
      'grant_view' => TRUE,
      'grant_update' => TRUE,
      'grant_delete' => TRUE,
      'priority' => 0,
    );

    $grants[] = array(
      'realm' => 'friends',
      'gid' => $node->uid,
      'grant_view' => TRUE,
      'grant_update' => FALSE,
      'grant_delete' => FALSE,
      'priority' => 0,
    );
    $grants[] = array(
      'realm' => 'friend_admin',
      'gid' => TRUE,
      'grant_view' => TRUE,
      'grant_update' => TRUE,
      'grant_delete' => TRUE,
      'priority' => 0,
    );
  } else if($node->permlevel == 'private') {
    $grants[] = array(
      'realm' => 'private_author',
      'gid' => $node->uid,
      'grant_view' => TRUE,
      'grant_update' => TRUE,
      'grant_delete' => TRUE,
      'priority' => 0,
    );
    $grants[] = array(
      'realm' => 'private_admin',
      'gid' => TRUE,
      'grant_view' => TRUE,
      'grant_update' => TRUE,
      'grant_delete' => TRUE,
      'priority' => 0,
    );
  }
  
 //   comment out for migration
 return $grants;  
}

/**
 * Implementation of hook_nodeapi().
 *
 * - "delete", "insert", and "update":
 * The module must track the access status of the node.
 */
function melodev_access_nodeapi(&$node, $op, $arg = 0, $arg2 = null) {
  switch ($op) {
    case 'load':
    case 'presave':
      static $melo_folders;
      if(!is_array($melo_folders)) {
        $melo_folders = array();
      }
      // Find out what access level of folder its in
      if($node->type == 'folder') {
        if($permlevel = $node->field_folder_permission[0]['value']) {
          $node->permlevel = $permlevel;
        } else {
          $node->permlevel = 'private';
        }
        $melo_folders[$node->nid] = $node->permlevel;
      } else if(isset($node->field_folder_reference)) {
        // Post has a folder id
        if($fid = $node->field_folder_reference[0]['nid']) {
        
          // Check if we have it in our array
          if(isset($melo_folders[$fid])) {
            $node->permlevel = $melo_folders[$fid];
          } else {
            // Load the folder permission
             $obj = db_fetch_object(db_query('select field_folder_permission_value from {content_type_folder} where nid = %d', $fid));
             $node->permlevel = $obj->field_folder_permission_value;
          }
          
        // No folder ID found
        } else {
          drupal_set_message('An error occured attempting to find folder id for post '.$node->nid.'.');
          watchdog('melodev', 'An error occured attempting to find folder id for post '.$node->nid.'.');
        }
      }
      break;
      
    case 'view':
      if($arg2 == true) {
        switch($node->type) {
          case 'folder':
          case 'blog':
          case 'photo':
          case 'favorite':
            global $user;
            // Figure out if I'm an enemy
            melodev_check_enemy_ban($user->uid, $node->uid, $node->name);
            if($arg2) {
              // Check if i have "logged in only" pref
              $account = melodev_user_load($node->uid);
              if($user->uid < 1 && $account->profile_logged_in == 1) {
                // Save this access attempt
                drupal_set_message('You must be logged in to visit '.$account->name.'.');
                // Hard redirect
                header('Location: /user/login');
                exit();
              }
            }
          break;
        }
      }
    break;
  }
}

/**
 * Process logs confirmation form.
 */
function melodev_access_logs_confirm() {
  $form = array();
  return confirm_form($form,
    t('Are you sure you want to process all unprocessed access logs?'),
    !empty($_GET['destination']) ? $_GET['destination'] : 'admin/reports',
    t('This action might take a while.'),
    t('Process'), t('Cancel')
  );
}

/**
 * Process logs submit form.
 */
function melodev_access_logs_confirm_submit($form, $form_state, $cron=false) {
  // Find all unprocessed logs
  $result = db_query('select aid, path, uid from accesslog where aid not in (select aid from accesslog_owner) limit 10000');
  $logs = 0;
  $unknown = array();
  while($log = db_fetch_object($result)) {
    if(melodev_access_savelog($log)) {
      $logs++;
    } else {
      $unknown[] = $log;
    }
  }
  if(!$cron) { drupal_set_message('Wrote '.$logs.' log ownership records.'); }
  if(count($unknown) > 0) {
    $log = '<pre>' .
      "<b>Unknown logs:\n</b>" .
      print_r($unknown, true) .
      '</pre>';
    watchdog('melodev_access', count($unknown).' unknown logs encountered. [logs]', array('[logs]'=>$log));
    if(!$cron) { drupal_set_message(count($unknown).' unknown logs encountered.'); }
  }
  if(!$cron) { drupal_goto('admin'); }
}

/**
 * Add additional logs to an access record.
 */
function melodev_access_savelog($log) {
  $path = explode('/', $log->path);
  $wrote = false;
  // Fuck Access Denied's
  if($log->title == 'Access Denied') {
    $wrote = melodev_access_writelog($log->aid, null, $path[0]);
  }
  switch($path[0]) {
  
    // User view
    case 'user':
      if(is_numeric($path[1])) {
        $wrote = melodev_access_writelog($log->aid, $path[1], t('Visited your (public) folder.'));
      } else {
        switch($path[1]) {
          case 'me':
          case 'login':
          case 'register':
          case 'reset':
          case 'password':
          case null:
            $wrote = melodev_access_writelog($log->aid, 0, 'user');
          break;
          default:
            $wrote = melodev_access_writelog($log->aid, null, 'user');
          break;
        }
      }
    break;
    
    // Node view
    case 'node':
      if(is_numeric($path[1])) {
        $wrote = melodev_access_writelog($log->aid, null, $path[0]);
        
        switch($path[2]) {
          case 'edit':
            $wrote = melodev_access_writelog($log->aid, null, 'node edit');        
          break;
          case 'delete':
            $wrote = melodev_access_writelog($log->aid, null, 'node delete');        
          break;
        }
        if(!$wrote) {
          $obj = db_fetch_object(db_query('select uid, title, type from node where nid = %d', $path[1]));
          // Ignore some types
          switch($obj->type) {
            case 'tribune':
              $wrote = melodev_access_writelog($log->aid, 0, 'wall');
            break;          
            case 'blog':
              $wrote = melodev_access_writelog($log->aid, $obj->uid, t('Visited your '.$obj->type.' (').l($obj->title, 'node/'.$path[1]).').');
            break;
            default:
              $wrote = melodev_access_writelog($log->aid, $obj->uid, t('Visited your '.$obj->type.' (').l($obj->title, 'node/'.$path[1]).').');
            break;
          }
        }
      } else {
        switch($path[1]) {
          case 'add':
            $wrote = melodev_access_writelog($log->aid, null, 'node add');
          break;
          case 'undefined':
            $wrote = melodev_access_writelog($log->aid, null, 'node undefined');
          break;
          default:
            $wrote = melodev_access_writelog($log->aid, null, 'node');
          break;
        }
      }
    break;
    
    // Touch
    case 'touch':
      if(is_numeric($path[1])) {
        $obj = db_fetch_object(db_query('select uid, title, type from node where nid = %d', $path[1]));
        $wrote = melodev_access_writelog($log->aid, $obj->uid, t('Touched your '.$obj->type.' (').l($obj->title, 'node/'.$path[1]).').');
      }
    break;
    
    // Bang
    case 'bang':
      if(is_numeric($path[1])) {
        $obj = db_fetch_object(db_query('select uid, title, type from node where nid = %d', $path[1]));
        $wrote = melodev_access_writelog($log->aid, $obj->uid, t('Banged your '.$obj->type.' (').l($obj->title, 'node/'.$path[1]).').');
      }
    break;  
    
    // Groups
    case 'group':
      switch($path[1]) {
        case 'mytracker':
        case 'tracker':
        case null:
          $wrote = melodev_access_writelog($log->aid, 0, 'group-'.$path[0]);
        break;
      }
    break;
    
    // Enemies
    case 'enemy':
      if($account = user_load(array('name'=>$path[1]))) {
        $wrote = melodev_access_writelog($log->aid, $account->uid, t('Tried to visit your journal.'));
      } else {
        $wrote = melodev_access_writelog($log->aid, 0, $path[0]);
      }
    break;
    
    // Other Views
    case 'admin':
    case 'batch':
    case 'god':
    case 'home':
    case 'mymelo':
    case 'devel':
    case 'slot_machine':
    case 'vud':
    case 'userpoints':
    case 'blog':
    case 'myuserpoints':
    case 'users':
    case 'elite':
    case 'stats':
    case 'sites':
    case 'logout':
    case 'users.php':
    case 'post':
    case 'eliteness':
    case 'images':
    case 'userform':
    case 'userform.php':
    case 'og':
    case 'mycliques':
    case 'cliques':
    case 'images':
    case 'profile_pictures':
    case 'about':
    case 'store':
    case 'elite.php':
    case 'melodev':
    case 'api':
    case 'js':
    case 'favicon.gif':
    case 'webmail_plus':
    case 'comment': // Maybe we want comment, not sure yet
    case 'flag': // Maybe we want flag, not sure yet
    default:
      $wrote = melodev_access_writelog($log->aid, null, $path[0]);
    break;  
  }
  // Check for titles we know
  if(!$wrote && isset($log->title)) {
    switch($log->title) {
      case 'Page not found':
      case 'User autocomplete':
        $wrote = melodev_access_writelog($log->aid, null, $log->title . ' ('.$log->path.')');
      break;
    }
  }
  return $wrote;  
}

/**
 * Write an access log to the database.
 */
function melodev_access_writelog($aid, $uid, $msg=null) {
    return db_query('insert into {accesslog_owner} (aid, uid, message) values(%d, %d, "%s")', $aid, $uid, $msg);
}

/**
 * Implementation of hook_views_data()
 */
function melodev_access_views_data() {
  $data['accesslog_owner']['table']['group'] = t('Accesslog Owner');
  $data['accesslog_owner']['table']['join'] = array(
    // ...to the accesslog table
    'accesslog' => array(
      'left_field' => 'aid',
      'field' => 'aid',
    ),
  );
  // aid
  $data['accesslog_owner']['aid'] = array(
    'title' => t('Access Log ID'),
    'help' => t('The id of this action log.'),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'accesslog',
      'field' => 'aid',
     ),
  );

  // owner uid
  $data['accesslog_owner']['uid'] = array(
    'title' => t('Owner: UID'),
    'help' => t('The user id of the owner of this access log.'),
     'filter' => array(
      'handler' => 'views_handler_filter_string',
     ),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'users',
      'field' => 'uid',
     ),
  );
  // uid
  $data['accesslog_owner']['uid_current'] = array(
    'real field' => 'uid',
    'title' => t('Current'),
    'help' => t('Filter the view to the currently logged in user.'),
    'filter' => array(
      'handler' => 'views_handler_filter_user_current',
      'type' => 'yes-no',
    ),
  );
  
  
  // message
  $data['accesslog_owner']['message'] = array(
    'title' => t('Message'),
    'help' => t('The message provided with the access log.'),

    'field' => array(
      'handler' => 'views_handler_field_markup',
      'format' => FILTER_FORMAT_DEFAULT,
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

  return $data;
}