<?php

/**
 * @file
 * Custom karma functionality.
 *
 * Since there were no modules existing to do exactly what karma
 * does, this is a mish-mash of solutions that add a lightweight
 * but effective karma system to Drupal.
 */

/**
 * Admin settings form.
 */
function melodev_karma_settings() {

  $form = array();
  
  // Grant Karma
  $form['grant'] = array(
    '#type' => 'fieldset',
    '#title' => 'Grant Karma',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  
  // Stats
  $form['grant']['stats'] = array(
    '#type' => 'fieldset',
    '#title' => 'Karma for Stats',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
    );
  $form['grant']['stats']['melodev_karma_hits'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Multiplier for Hits'),
    '#default_value' => variable_get('melodev_karma_hits', '0'),
    '#size'          => 5,
    '#maxlength'     => 5,
    );
  $form['grant']['stats']['melodev_karma_years'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Multiplier for Years as a Member'),
    '#default_value' => variable_get('melodev_karma_years', '0'),
    '#size'          => 5,
    '#maxlength'     => 5,
    );
  $form['grant']['stats']['melodev_karma_spammers'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Multiplier for Spammers'),
    '#default_value' => variable_get('melodev_karma_spammers', '0'),
    '#size'          => 5,
    '#maxlength'     => 5,
    );
  
  // Nodes
  $form['grant']['nodes'] = array(
    '#type' => 'fieldset',
    '#title' => 'Karma for Node Stats',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
    );
 foreach (node_get_types() as $type => $name) {
    $form['grant']['nodes']['melodev_karma_' . $type] = array(
      '#type'          => 'textfield',
      '#title'         => t('Multiplier for posting a !node-name', array('!node-name' => $name->name)),
      '#default_value' => variable_get('melodev_karma_' . $type, '0'),
      '#size'          => 5,
      '#maxlength'     => 5,
      );
  }
  
  // Comments
  $form['grant']['comments'] = array(
    '#type' => 'fieldset',
    '#title' => 'Karma for Comments',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
    );
  $form['grant']['comments']['melodev_karma_comment'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Multiplier for posting a comment'),
    '#default_value' => variable_get('melodev_karma_comment', '0'),
    '#size'          => 5,
    '#maxlength'     => 5,
    );  
  $form['grant']['comments']['melodev_karma_comment_received'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Multiplier for receiving a comment'),
    '#default_value' => variable_get('melodev_karma_comment_recevied', '0'),
    '#size'          => 5,
    '#maxlength'     => 5,
    );  
  $form['grant']['comments']['melodev_karma_virgin'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Multiplier for getting a virgin'),
    '#default_value' => variable_get('melodev_karma_virgin', '0'),
    '#size'          => 5,
    '#maxlength'     => 5,
    );
    
  // Touches
  $form['grant']['touches'] = array(
    '#type' => 'fieldset',
    '#title' => 'Karma for Touches',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
    );
  foreach(touches_get_touch_types() as $id => $type) {
    $form['grant']['touches']['melodev_karma_'.$type->name] = array(
      '#type'          => 'textfield',
      '#title'         => t('Multiplier for giving a !touch-name', array('!touch-name' => $type->label)),
      '#default_value' => variable_get('melodev_karma_' . $type->name, '0'),
      '#size'          => 5,
      '#maxlength'     => 5,
      );
  }
  $form['grant']['touches']['melodev_karma_touches'] = array(
      '#type'          => 'textfield',
      '#title'         => t('Multiplier for touch totals for users'),
      '#default_value' => variable_get('melodev_karma_touches', '0'),
      '#size'          => 5,
      '#maxlength'     => 5,
      );
  
  // Preferences
  $form['grant']['prefs'] = array(
    '#type' => 'fieldset',
    '#title' => 'Karma for Preferences',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
    );
  $res = _profile_get_fields(MELO_PREFS_CATEGORY);
  while($field = db_fetch_object($res)) {
    $form['grant']['prefs']['melodev_pref_'.$field->fid] = array(
      '#type'          => 'textfield',
      '#title'         => t('Karma for preference "!pref-name"', array('!pref-name' => $field->title)),
      '#default_value' => variable_get('melodev_pref_' . $field->fid, '0'),
      '#size'          => 5,
      '#maxlength'     => 5,
      );
  }
  
  
  // Orders
  $form['grant']['orders'] = array(
    '#type' => 'fieldset',
    '#title' => 'Karma for Orders',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
    );
  $arr = array('gifts', 'subscriptions', 'years', 'dollars', 'recurring');
  foreach($arr as $key) {
    $form['grant']['orders']['melodev_karma_'.$key] = array(
      '#type'          => 'textfield',
      '#title'         => t('Karma for "!order-name"', array('!order-name' => $key)),
      '#default_value' => variable_get('melodev_karma_' . $key, '0'),
      '#size'          => 5,
      '#maxlength'     => 5,
      );
  }
  
  // Flags
  $form['grant']['flags'] = array(
    '#type' => 'fieldset',
    '#title' => 'Karma for Flags',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
    );
  $arr = array('friend', 'enemy', 'tab', 'friendof', 'enemyof', 'tabof');
  foreach($arr as $key) {
    $form['grant']['flags']['melodev_karma_'.$key] = array(
      '#type'          => 'textfield',
      '#title'         => t('Karma for "!flag-name"', array('!flag-name' => $key)),
      '#default_value' => variable_get('melodev_karma_' . $key, '0'),
      '#size'          => 5,
      '#maxlength'     => 5,
      );
  }

  
  // Grant Roles
  $form['roles'] = array(
    '#type' => 'fieldset',
    '#title' => 'Grant Roles',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => 'Define what karma levels a user needs to reach in order to receive roles.  Leave as 0 to ignore this role from karma granting.'
  );
  
  $roles = user_roles(true);
  foreach($roles as $rid => $name) {
    $form['roles']['karma_rid_'.$rid] = array(
      '#type' => 'textfield',
      '#title' => $name,
      '#default_value' => variable_get('karma_rid_'.$rid, 0),
      '#size' => 5
      );
  }
    
  return system_settings_form($form);

}

/**
 * Calculate karma function.
 */
function melodev_karma($account=null, $update=false) {
  $start = mktime() + microtime();
  
  // Use current user if none supplied
  if(is_null($account)) {
    global $user;
    $account = $user;
  } else if(is_numeric($account)) {
    $account = user_load($account);
  }
  
  // If we're not updating, select from a table
  if(!$update) {
    if($obj = db_fetch_object(db_query('select * from {karma} where uid = %d', $account->uid))) {
      return array('karma'=>$obj->karma);
    } else {
      return array('karma'=>0);
    }
  }

  $key = 'alltime';
  $karma = 0;  
  $return = array();
  
  // Add stats to karma
  $stats = melodev_stats_get_user($account);
  if(isset($stats[$key])) {
    foreach($stats[$key] as $type => $count) {
      $count = str_replace(',', '', $count);
      $m = variable_get('melodev_karma_'.$type, 0);
      if($m != 0) {
        $stat = round($m*sqrt($count));
        $return[$type] = $stat;
        $karma += $stat;
      }
    }
  }
  
  // Add touches/bangs to karma
  $criteria = array(
    'uid' => $account->uid,
//    'function' => 'sum'
  );
  $results = votingapi_select_votes($criteria);
  $total = db_result(db_query('select sum(value) from {votingapi_vote} where uid = %d', $account->uid));
  /*
  echo '<pre>';
  echo $total."\n";
  print_r($criteria);
  print_r($results);
  exit();
  */
  
  // Add Orders to karma
  $orders = array('gifts'=>0, 'subscriptions'=>0, 'years'=>0);
  $total = 0;
  $sql = "SELECT o.order_id, o.order_total FROM {uc_orders} AS o LEFT JOIN {uc_order_statuses} AS os ON o.order_status = os.order_status_id WHERE o.uid = %d AND os.order_status_id ='completed'";
  $result = db_query($sql, $account->uid);
  while($obj = db_fetch_object($result)) {
    $total += $obj->order_total;
    $order = uc_order_load($obj->order_id);
    foreach($order->products as $product) {
      switch($product->nid) {
        case MELO_ELITE_GIFT_NID:
          $orders['gifts']++;
        break;
        case MELO_ELITE_SUBSCRIPTION_NID:
          $orders['subscriptions']++;
        break;
        case MELO_ELITE_YEARLY_NID:
          $orders['years']++;
        break;
      }
    }
  }
  foreach($orders as $key => $count) {
    $m = variable_get('melodev_karma_'.$key, 0);
    $stat = round($m*$count);
    if($stat > 0) {
      $return[$key] = $stat;
      $karma += $stat;
    }
  }
  // Add total dollars
  $m = variable_get('melodev_karma_dollars', 0);
  $stat = round($m*$total);
  if($stat > 0) {
    $return['dollars'] = $stat;
    $karma += $stat;
  }
  /*
  // Add recurring membership
  if($fees = uc_recurring_user_fees($account)) {
    $m = variable_get('melodev_karma_recurring', 0);
    $return['recurring'] = $m;
    $karma += $m;
  }
  */
  // Subtract bad prefs from karma
  $res = _profile_get_fields(MELO_PREFS_CATEGORY);
  while($field = db_fetch_object($res)) {
    $pref = variable_get('melodev_pref_'.$field->fid, 0);
    if($pref != 0) {
      // If the user has this preference
      if($account->{$field->name}) {
        $return[$field->name] = $pref;
        $karma += $pref;
      }
    }
  }
  
  // Add/Subtract flags from karma
  $flags = flag_get_user_flags('user', null, $account->uid);
  if(is_array($flags)) {
    foreach($flags as $name => $items) {
      $count = count($items);
      $m = variable_get('melodev_karma_'.$name, 0);
      if($m != 0) {
        $stat = round($m*sqrt($count));
        if($stat > 0) {
          $return[$name] = $stat;
          $karma += $stat;
        }
      }
    }
  }
  
  // Add/Subtract years of membership
  $years = floor((mktime() - $account->created)/(60*60*24*365));
  $m = variable_get('melodev_karma_years', 0);
  $stat = $m*$years;
  $return['years'] = $stat;
  $karma += $stat;
  
  // Add/Subtrack Bonus karma
  $bonuses = db_query('select * from karma_bonus where uid = %d', $account->uid);
  while($bonus = db_fetch_object($bonuses)) {
    $return[$bonus->name] = $bonus->karma;
    $karma += $bonus->karma;
  }
  
  /**************  Finish Up Karma  ************/
  
  $return['karma'] = $karma;
  
  // Whether to update stats or karma table
  if($update) {
    if(db_fetch_object(db_query('select * from {karma} where uid = %d', $account->uid))) {
      db_query('update {karma} set karma = %d where uid = %d', $karma, $account->uid);
    } else {
      db_query('insert into {karma} (uid, karma) values(%d, %d)', $account->uid, $karma);
    }
    // We're updating the user's karma value so update their roles as well
    melodev_karma_roles($account, $karma);
  }
  return $return;
}

/**
 * Calculate a user's touches.
 */
function melodev_karma_touches($account) {
  $karma = melodev_karma($account);
  $karma = $karma['karma'];
  $m = variable_get('melodev_karma_touches', 0);
  $touches = round($karma * $m);
  if($touches <= 0) {
    $touches = 5;
  }
  return $touches;
}

/**
 * Add or remove karma roles as necessary.
 */
function melodev_karma_roles($account, $karma) {
  $roles = user_roles(true);
  foreach($roles as $rid => $name) {
    // This role is controlled by karma
    $level = variable_get('karma_rid_'.$rid, 0);
    if($level != 0) {    
      // Allow for negative karma roles
      if($level < 0) {
        $acctroles = $account->roles;
        if(!is_array($acctroles)) { $acctroles = array(); }
        // Give the role
        if($karma <= $level && !in_array($name, $acctroles)) {
          user_multiple_role_edit(array($account->uid), 'add_role', $rid);
        // Take the role away
        } else if($karma > $level && in_array($name, $acctroles)) {
          user_multiple_role_edit(array($account->uid), 'add_role', $rid);
        }        
        
      // Positive karma roles
      } else {
        $acctroles = $account->roles;
        if(!is_array($acctroles)) { $acctroles = array(); }
        // Give the role
        if($karma >= $level && !in_array($name, $acctroles)) {
          user_multiple_role_edit(array($account->uid), 'add_role', $rid);
          watchdog('melodev.karma', 'Grant role '.$name.' ('.$rid.') to '.$account->name.' ('.$account->uid.') with karma '.$karma.'.');
        // Take the role away
        } else if($karma < $level && in_array($name, $acctroles)) {
          user_multiple_role_edit(array($account->uid), 'remove_role', $rid);
          watchdog('melodev.karma', 'Remove role '.$name.' ('.$rid.') from '.$account->name.' ('.$account->uid.') with karma '.$karma.'.');
        } else {
        }
      }
    }
    // end role is controlled by karma
  } // end foreach
}