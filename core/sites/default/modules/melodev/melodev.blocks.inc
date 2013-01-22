<?php

/**
 * @file
 * Custom Melo blocks.
 */
 
 /**
 * Implementation of hook_block().
 */
function melodev_block($op = 'list', $delta = 0, $edit = array()) {
  $blocks = array();
  if ($op == 'view') {
  
    switch($delta) {
      /*  Profile Blocks  */
      case '0':
        // Picture
        $block['subject'] = '';
        $block['content'] = melodev_profile_picture();
      break;
      case '1':  
        // About Me
        $block['subject'] = 'About Me';
        $block['content'] = melodev_profile_aboutme();
      break;
      case '2':  
        // Interact
        $block['subject'] = 'Interact';
        $block['content'] = melodev_profile_interact();
      break;
      case '3':
        // Stats
        $block['subject'] = 'Statistics';
        $block['content'] = melodev_profile_stats();
      break;
      case '4':
        // Frontpage Stats
        $block['subject'] = 'Top Stat Spotlight';
        $block['content'] = melodev_get_top_stats();
      break;
      case '5':
        // User Info
        $block['subject'] = 'User Info';
        $block['content'] = melodev_user_info();
      break;
      case '6':
        // Who's Online
        $arr = melodev_whos_online();
        $block['subject'] = 'Who\'s Online <span style="font-size: 0.6em; position: absolute; margin-left: 45px; ">'.$arr['total'].'</span>';
        $block['content'] = $arr['content'];
      break;
      case '7':
        // Elite Checkout
        $block['subject'] = 'Elite Checkout';
        $block['content'] = melodeve_elite_checkout();
      break;
      case '8':
        // Jump to User
        $block['subject'] = 'Jump to User';
        $block['content'] = melodev_jumpto_user_block();
      break;
      case '9':
        // Active Wall Users
        $block['subject'] = 'Active Wall Users';
        $block['content'] = melodev_active_wall_users();
      break;
      case '10':
        // Personal Tag Cloud
        $block['subject'] = 'Tagging';
        $block['content'] = melodev_personal_tag_cloud();      
      break;
    }
    return $block;
  }
  elseif ($op == 'list') {
    /*  Profile Blocks  */
    $blocks[0]['info'] = t('Profile: Picture');
    $blocks[0]['cache'] = BLOCK_NO_CACHE;

    $blocks[1]['info'] = t('Profile: About Me');
    $blocks[1]['cache'] = BLOCK_NO_CACHE;
    
    $blocks[2]['info'] = t('Profile: Interact');
    $blocks[2]['cache'] = BLOCK_NO_CACHE;
    
    $blocks[3]['info'] = t('Profile: Stats');
    $blocks[3]['cache'] = BLOCK_NO_CACHE;
    
    $blocks[4]['info'] = t('Stats: Top Users Today');
    
    $blocks[5]['info'] = t('Profile: User Info');
    $blocks[5]['cache'] = BLOCK_NO_CACHE;
    
    $blocks[6]['info'] = t('Users: Who\'s Online');
    $blocks[6]['cache'] = BLOCK_NO_CACHE;
    
    $blocks[7]['info'] = t('Elite: Checkout');
    $blocks[7]['cache'] = BLOCK_NO_CACHE;
    
    $blocks[8]['info'] = t('Melo: Jump to User');
    $blocks[8]['cache'] = BLOCK_NO_CACHE;

    $blocks[9]['info'] = t('Melo: Active Wall Users');
    $blocks[9]['cache'] = BLOCK_NO_CACHE;
    
    $blocks[10]['info'] = t('Profile: Personal Tag Cloud');
    $blocks[10]['cache'] = BLOCK_NO_CACHE;
    
    return $blocks;
  }
}

/*  Melo Blocks  */

/**
 * @defgroup melodev_profileblocks Melodev Profile Blocks
 *
 * Custom blocks for the user profile.
 *
 * @{
 */
 
/**
 * Implementation of hook_blockcache.
 */
function melodev_blockcache_skip($block, $cid) {

  global $user;
  if($user->uid == 223853) {
    drupal_set_message($_REQUEST['q']);
  }

  // Don't cache blocks on mysite url
  if($_REQUEST['q'] == 'mysite' || $_REQUEST['q'] == 'user') {
    watchdog('melodev', 'Skipping block cache on mysite.');
    return true;
  }
  
  return false;
}
 
/**
 * User profile picture in the left column.
 */
function melodev_profile_picture() {
  $uid = melodev_get_profile_uid();
  $output = '';
  if(is_numeric($uid)) {
    $account = melodev_user_load($uid);
    
    if (!empty($account->picture) && file_exists($account->picture)) {
      $picture = $account->picture;
    }
    else if (variable_get('user_picture_default', '')) {
      $picture = variable_get('user_picture_default', '');
    }
    
    // Add user picture
    $output .= '<h1>'.$account->name.'</h1>';
    $output .= l(theme('imagecache', 'user_photo', $picture), 'user/'.$account->uid, array('html'=>true));
  } else {
    $output .= 'no picture for uid '.$uid;
  }
  return $output;
}

/**
 * About Me profile block.
 */
function melodev_profile_aboutme() {
  $uid = melodev_get_profile_uid();
  if(is_numeric($uid)) {
    $account = melodev_user_load($uid);
    profile_view_profile($account);
    if(isset($account->content['About Me']['profile_quote']['#value'])) {
      $output .= '<p>'.$account->content['About Me']['profile_quote']['#value'].'</p>';
    }
    if(is_array($account->content['About Me'])) {
      foreach($account->content['About Me'] as $key => $arr) {
        if(is_array($arr) && $key != 'profile_quote') {
          $output .= '<strong>'.$arr['#title'].':</strong><br />'.$arr['#value'].'<br /><br />';
        }
      }
    }
  } else {
    $output .= 'no user found with id '.$uid;
  }
  return $output;
}

/**
 * Interact profile block with friends/enemies/tabs.
 */
function melodev_profile_interact() {
  global $user;
  $uid = melodev_get_profile_uid();
  if(is_numeric($uid)) {
  
    $txtlinks = array();
  
    // Add friends/enemies interaction
    if($user->uid > 0) {
      $juser = melodev_user_load($uid);
      $uid1 = $user->uid;
      $uid2 = $uid;
      if ($uid1 != $uid2) {
      
        // Load all flags on this user
        $flags = flag_get_user_flags('user', $uid2);
        $theirflags = flag_get_user_flags('user', $uid1, $uid2);
      
        // Friends
        $flag = flag_get_flag('friend');
        // Check my friend status
        if(!isset($flags['friend'])) {
          $links[] = $flag->theme('flag', $uid2);
        } else {
          $links[] = $flag->theme('unflag', $uid2);
        }
        // Check their friend status
        if(isset($theirflags['friend'])) {
          $txtlinks[] = $juser->name.' considers you a friend.';
        }
        
        // Enemies
        $flag = flag_get_flag('enemy');
        // Check my enemy status
        if(!isset($flags['enemy'])) {
          $links[] = $flag->theme('flag', $uid2);
        } else {
          $links[] = $flag->theme('unflag', $uid2);
        }
        // Check their enemy status
        if(isset($theirflags['enemy'])) {
          $txtlinks[] = $juser->name.' considers you an enemy.';
        }
        
        // Tab
        $flag = flag_get_flag('tab');
        // Check my tab status
        if(!isset($flags['tab'])) {
          $links[] = $flag->theme('flag', $uid2);
        } else {
          $links[] = $flag->theme('unflag', $uid2);
        }
      }
      
      if(count($txtlinks) > 0) {
        foreach($txtlinks as $link) {
          $links[] = $txtlinks;
        }
      }
      
      if(count($links) > 0) {
        $output .= theme('item_list', $links);
      }
      
      // Admin stuff
      if(in_array('god', $user->roles)) {
        $table = array();
        // User management
        if(user_access('administer users')) {
          $table[] = array(l('Edit User', 'user/'.$uid2.'/edit', array('attributes'=>array('class' => 'admin-link',
            'title'=>'USE WITH CAUTION' ))));
          $table[] = array(l('Rebuild Access', 'admin/god/access/'.$uid2, array('attributes'=>array('class' => 'admin-link',
            'title'=>'Use if users complain that they can\'t edit posts, or their private/FO posts are available to the public.' ))));
          $table[] = array(l('Queue for Reimport', 'admin/god/reimport/queue/'.$uid2, array('attributes'=>array('class' => 'admin-link',
            'title'=>'Use this when users complain about half-missing posts.' ))));
        }
        if(user_access('administer nodes')) {
          $table[] = array(l('Reimport Now', 'admin/god/reimport/user/'.$uid2, array('attributes'=>array('class' => 'admin-link',
            'title'=>'Use to reimport one user, will take quite a while to run.' ))));
        }
        // Money management
        if(user_access('view all orders')) {
          $table[] = array(l('Order History', 'user/'.$uid2.'/orders', array('attributes'=>array('class' => 'admin-link',
            'title'=>'View User Order History.' ))));
          $table[] = array(l('Historic Order Report', 'admin/god/orders/'.$uid2, array('attributes'=>array('class' => 'admin-link',
            'title'=>'Compare Melo 2.0 orders to Melo 3.0 orders.' ))));
        }
        $output .= '<br /><h2 class="title">Admin Menu</h2>';
        $output .= theme('table', array(), $table);
      }
    }
  }
  return $output;
}

/**
 * Stats profile block.
 */
function melodev_profile_stats() {
  $uid = melodev_get_profile_uid();
  $account = melodev_user_load($uid);
  $stats = melodev_stats_get_user($account);
  
  $output = '<h2 class="title">'.t('Today').':</h2>';
  if(count($stats['today']) > 0) {
    $rows = array();
    foreach($stats['today'] as $key => $stat) {
      $row = array(array('data'=>melodev_get_stat_label($key), 'class'=>'firstcol'), $stat);
      if($key == 'karma') {
        continue;
      } else {
        $rows[] = $row;
      }
    }
    if($karmarow) {
      $rows[] = $karmarow;
      unset($karmarow);
    }
    $output .= theme('table', array(), $rows);
  } else {
    $output .= '<p>'.t('No recent activity.').'</p>';
  }
  
  $output .= '<h2 class="title">'.t('All Time').':</h2>';
  if(count($stats['alltime']) > 0) {
    $rows = array();
    foreach($stats['alltime'] as $key => $stat) {
      $row = array(array('data'=>melodev_get_stat_label($key), 'class'=>'firstcol'), $stat);
      if($key == 'karma') {
        continue;
      } else {
        $rows[] = $row;
      }
    }
    if($karma = melodev_karma($account)) {
      $rows[] = array(array('data'=>t('Karma'), 'class'=>'firstcol'), number_format($karma['karma']));
    }
    $output .= theme('table', array(), $rows);
  } else {
    $output .= '<p>'.t('No recent activity.').'</p>';
  }
  
  // Add other details
  $output .= '<h2 class="title">'.t('Details').'</h2>';
  $rows = array(
    array(array('data'=>t('Joined'), 'class'=>'firstcol'), format_date($account->created, 'custom', 'M.d.y')),
    array(array('data'=>t('Online'), 'class'=>'firstcol'), format_date($account->access, 'custom', 'M.d.y')),
    );
  $output .= theme('table', array(), $rows);
  
  if(in_array('elite', $account->roles)) {
    $output .= '<p>'.t('This member is elite.').'</p>';
  }
  
  return $output;
}

/**
 * Personal tag cloud block.
 */
function melodev_personal_tag_cloud() {
  $uid = melodev_get_profile_uid();
  if($account = melodev_user_load($uid)) {
    if(user_access('personal tag cloud', $account)) {
      $tags = unserialize(db_result(db_query('select tags from {melo_personal_tagcloud} where type = "%s" and id = %d', 'user', $account->uid)));
      if(is_array($tags)) {
        shuffle($tags);
        $newtags = array();
        $i=0;
        foreach($tags as $tag) {
          $newtags[] = $tag;
          $i++;
          if($i > 50) {
            break;
          }
        }
        $show = array();
        foreach($tags as $tag) {
          $show[] = l($tag->name, 'users/'.$account->name.'/tags/'.$tag->name, array('attributes'=>array('class'=>'tagadelic level'.$tag->weight)));
        }
        $output = implode(' ', $show);
        return $output;
      }
    }
  }
}
/**
* @} End of "melodev_profileblocks"
*/

/**
 * Provide a small user info block at the top of every page.
 */
function melodev_user_info() {
  global $user;
  profile_load_profile($user);
  
  // Welcome
  $output .= t('Welcome back, !username', array('!username'=>l($user->name, 'user/'.$user->uid))).' | ';

  // Active Users
  $interval = time() - variable_get('user_block_seconds_online', 900);
  $users = db_result(db_query('SELECT count(DISTINCT u.uid) FROM {users} u INNER JOIN {sessions} s ON u.uid = s.uid WHERE s.timestamp >= %d AND s.uid > 0', $interval));
  $output .= '<span id="active-users">'.l('Active Users', 'active-users') .': <strong>'.number_format($users).'</strong></span>';
  // array('!users'=>'<strong id="active-users">'.l(number_format($users), 'active-users').'</strong>'));

  $output .= ' | ';
  
  // Karma
  $karma = melodev_karma($user);
  $output .= l('Karma', 'mymelo/you/karma').': <strong>'.number_format($karma['karma']).'</strong>';
  
  $output .= ' | ';
  
  // Touches (global limit)
  $touches = touches_get_limit($user->uid, 0);
  $output .= t('Touches: !touches', array('!touches'=>'<strong id="my-touches">'.number_format($touches).'</strong>'));
  
  $output .= ' | ';
  
  // Logout
  $output .= l('Logout', 'logout');
  
  // Gspot alerts
  $view = $user->profile_last_gspot_view_timestamp;
  $last = $user->profile_last_gspot_timestamp;
  if($view <= $last && $last > 0) {
    $output .= ' | <span class="red">'.l('You have new gspots waiting!', 'mymelo/gspots/incoming').'</span>';
  }
  $view = $user->profile_last_reply_view_timestamp;
  $last = $user->profile_last_reply_timestamp;
  if($view <= $last && $last > 0) {
    $output .= ' | <span class="red">'.l('You have new gspot replies waiting!', 'mymelo/gspots/replies').'</span>';
  }  
  
  // Check if we're on a random tour
  if($tourrow = db_fetch_object(db_query('select * from random_tour where uid = %d and type = "%s" and active = 1', $user->uid, 'random'))) {
    $tour = unserialize($tourrow->tour);
    if($tour['last_user'] > 0) {
      $last_user = melodev_user_load($tour['last_user']);
      $last_user = $last_user->name;
      $last_url = 'user/'.$tour['last_user'];
    } else {
      $last_user = 'Beginning';
      $last_url = 'random';
    }
    $active_user = melodev_user_load($tour['active_user']);
    $output .= '<br /><span class="red"><strong>Taking the Random Tour ('.count($tour['seen_users']).'/'.$tour['total_users'].')</strong></span> | Last User: '.l($last_user, $last_url).' | Current User: '.l($active_user->name, 'user/'.$tour['active_user']).' | '.l('Next User', 'random/next') . ' | '.l('Pause Tour', 'random/pause');
    /*
    $output .= '<br /><pre>'.print_r($tour, true).'</pre>';
    echo $output;
    exit();
    */
  }
  
  return $output;
}

/**
 * Custom Who's Online block.
 */
function melodev_whos_online() {
  // Count users active within the defined period.
  $interval = time() - variable_get('user_block_seconds_online', 900);

  // Perform database queries to gather online user lists.  We use s.timestamp
  // rather than u.access because it is much faster.
//  $anonymous_count = sess_count($interval);
  $authenticated_users = db_query('SELECT DISTINCT u.uid, u.name, s.timestamp FROM {users} u INNER JOIN {sessions} s ON u.uid = s.uid WHERE s.timestamp >= %d AND s.uid > 0 ORDER BY s.timestamp DESC', $interval);
  $authenticated_count = 0;
  $max_users = variable_get('user_block_max_list_count', 10);
  $items = array();
  while ($account = db_fetch_object($authenticated_users)) {
    if ($max_users > 0 && !isset($items[$account->uid])) {
      $items[$account->uid] = $account;
      $max_users--;
      $authenticated_count++;
    }
  }

  // Get Wall Active Users
  if(function_exists('tribune_get_active_users')) {
     $node = melodev_node_load(24);
     $wall_users = tribune_get_active_users($node);
     // Reorganize by userid
     foreach($wall_users as $name => $walluser) {
       $items[$walluser['uid']]->wall = true;
     }
  }
    
  // Display a list of currently online users.
  $max_users = variable_get('user_block_max_list_count', 10);
  if ($authenticated_count && $max_users) {
    // Style the list
    $rows = array();
    foreach($items as $uid => $item) {
      if($uid == 0) {
        continue;
      }
      if($item->wall == true) {
        $rows[] = '<span class="red">'.theme('username', $item).'</span>';
      } else {
        $rows[] = theme('username', $item);
      }
    }
    $output = implode(', ', $rows);
  }
  
  // Format the output with proper grammar.
//  $output .= '<p>'. t('%members and %visitors', array('%members' => format_plural($authenticated_count, '1 member', '@count members'), '%visitors' => format_plural($anonymous_count, '1 guest', '@count guests'))) . '</p>';

  $return = array('content'=>$output, 'total'=>$authenticated_count);
  return $return;
}

/**
 * Elite checkout 1,2,3 block.
 */
function melodeve_elite_checkout() {
  $output = '<ul class="elite-checkout">';
  
  if(arg(0) == 'cart' && arg(1) == 'checkout' && !arg(2)) { $class = 'active'; } else { $class = ''; }
  $output .= '<li id="elite-checkout-1" class="'.$class.'">billing info</li>';
  
  if(arg(0) == 'cart' && arg(1) == 'checkout' && arg(2) == 'review') { $class = 'active'; } else { $class = ''; }
  $output .= '<li id="elite-checkout-2" class="'.$class.'">review order</li>';
  
  if(arg(0) == 'cart' && arg(1) == 'checkout' && arg(2) == 'complete') { $class = 'active'; } else { $class = ''; }
  $output .= '<li id="elite-checkout-3" class="'.$class.'">confirmation</li>';
  
  $output .= '<li id="elite-checkout-4" class="'.$class.'">eliteness!</li>';
  
  $output .= '</ul>';
  
  return $output;
}

/**
 * Home Page Top Stats block.
 */
function melodev_get_top_stats() {

  // Load all stats nodes
  $nodes = array();
  $result = db_query('select nid from node where type = "stat" and status = 1');
  while($row = db_fetch_array($result)) {
    $nodes[] = melodev_node_load($row['nid']);
  }
  $random = array_rand($nodes);
  $output = '<div class="top-stat-spotlight">'.node_view($nodes[$random], true, false, false).'</div>';
  return $output;

  // Skip certain stats
  $skip = array('photo', 'article', 'profile', 'feed', 'folder', 'help', 'contexthelp', 'color_scheme', 'favorite', 'stat', 'feature', 'page', 'fpart', 'poll');
  $skip = '\''.implode('\', \'', $skip).'\'';

  $result = db_query('select type, uid, max(total) as total from stats_today where uid > 0 and type not in ('.$skip.') group by type order by total desc');
  $rows = array();
  while($obj = db_fetch_object($result)) {
    $account = melodev_user_load($obj->uid);
    
    // Get the user picture
    if (!empty($account->picture) && file_exists($account->picture)) {
      $picture = $account->picture;
    }
    else if (variable_get('user_picture_default', '')) {
      $picture = variable_get('user_picture_default', '');
    }
    $picture = l(theme('imagecache', 'user_photo', $picture), 'user/'.$account->uid, array('html'=>true));
    
    $table = theme('table', array(), array(array(t('Stat:'), '<strong>'.melodev_get_stat_label($obj->type).'</strong>'), array(t('Total:'), '<strong>'.number_format($obj->total).'</strong>')));
    
    $view = '<div class="top-stat-spotlight">'.$picture . '<h1>' . l($account->name, 'user/'.$account->uid) . '</h1>'.$table.'<blockquote>'.strip_tags($account->profile_quote).'</blockquote></div>';
    $rows[] = $view;
  }
  
  // Return 1 random stat
  $random = array_rand($rows);
  return $rows[$random];
}

/**
 * Jump to user block.
 */
function melodev_jumpto_user_block() {
  $output = drupal_get_form('melodev_jumpto_user_form');
  return $output;
}

/**
 * Active wall users block.
 */
function melodev_active_wall_users() {
  $output = '<div id="active-wall-users"><strong>Active Wall Users:</br></div>';
  return $output;
}