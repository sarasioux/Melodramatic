<?php

/**
 * @file
 * Individual pages being output by Melodev.
 *
 * Includes any pages that aren't drupal_get_form callbacks.
 */

/**
 * @defgroup melodev_pages Melodev Pages
 *
 * Custom pages on Melodramatic.com.
 *
 * @{
 */

/**
 * Page callback to support old Paypal payments
 * 
 */
function melodev_legacy_paypal_ipn() {
  $txn_id = check_plain($_POST['txn_id']);
  watchdog('melodev.paypal', 'Received IPN for txn_id '.$txn_id.'.');
  // Try to get it from the subscr_id
  $subscr_id = check_plain($_POST['subscr_id']);
  $order_id = db_result(db_query('select order_id from {uc_paypal_subscriptions} where txn_id = "%s"', $subscr_id));
  // Next try to get it from the map
  if(!is_numeric($order_id)) {
    $order_id = db_result(db_query('select order_id from melodev_order_txn_map where txn_id = "%s"', $txn_id));
  }
  if(is_numeric($order_id) && $order_id > 0) {
    melodev_legacy_paypal_buttons_ipn($order_id);
  } else {
    watchdog('melodev.paypal', 'Unable to load mapped order id for txn_id '.$txn_id.'.');
  }
}

/**
 * Process PayPal IPN.
 *
 * This function is adapted from uc_paypal_ipn() in uc_paypal.pages.inc
 * from the core Ubercart PayPal module.
 */
function melodev_legacy_paypal_buttons_ipn($order_id = 0) {

  // Assign posted variables to local variables
  $payment_status = check_plain($_POST['payment_status']);
  $payment_amount = check_plain($_POST['mc_gross']);
  $payment_currency = check_plain($_POST['mc_currency']);
  $payment_fee = check_plain($_POST['mc_fee']);
  $receiver_email = check_plain($_POST['receiver_email']);
  $txn_id = check_plain($_POST['txn_id']);
  $txn_type = check_plain($_POST['txn_type']);
  $payer_email = check_plain($_POST['payer_email']);

  $req = '';

  foreach ($_POST as $key => $value) {
    $value = urlencode(stripslashes($value));
    $req .= $key .'='. $value .'&';
  }

  $req .= 'cmd=_notify-validate';

  if (variable_get('uc_paypal_wpp_server', '') == 'https://api-3t.paypal.com/nvp') {
    $host = 'https://www.paypal.com/cgi-bin/webscr';
  }
  else {
    $host = variable_get('uc_paypal_wps_server', 'https://www.sandbox.paypal.com/cgi-bin/webscr');
  }

  $response = drupal_http_request($host, array(), 'POST', $req);

  if (array_key_exists('error', $response)) {
    watchdog('melodev.paypal', 'IPN failed with HTTP error @error, code @code.', array('@error' => $response->error, '@code' => $response->code), WATCHDOG_ERROR);
    return;
  }

  if (strcmp($response->data, 'VERIFIED') == 0) {
    watchdog('melodev.paypal', 'IPN transaction verified.');

/* This begins a section of modifications for PayPal Buttons functionality. */

    // If order has no user id, check for existing user with payer_email
    // as current mail address.
    if (!$order->uid) {
      if ($account = user_load(array('mail' => $payer_email)) && $account && $account->uid) {
        $order->uid = $account->uid;
      }
    }

    // If order has no primary e-mail, use the payer_email.
    if (empty($order->primary_email)) {
      $order->primary_email = $payer_email;
      uc_order_save($order);
    }

    // If order has no billing address, use address from IPN.
    // Also, set delivery address.
    if (empty($order->billing_first_name) && empty($order->billing_last_name) && empty($order->billing_street1) && empty($order->billing_city)) {
      $first_name = check_plain($_POST['first_name']);
      $last_name = check_plain($_POST['last_name']);
      $street = check_plain($_POST['address_street']);
      $street2 = '';
      if (!empty($street) && $pos = strpos("\n", $street)) {
        $street2 = trim(substr($street, $pos+1));
        $street = trim(substr($street, 0, $pos));
      }
      $city = check_plain($_POST['address_city']);
      $state = check_plain($_POST['address_state']);
      $zip = check_plain($_POST['address_zip']);
      $country = check_plain($_POST['address_country_code']);

      $order->billing_first_name = $first_name;
      $order->billing_last_name = $last_name;
      $order->billing_street1 = $street;
      $order->billing_street2 = $street2;
      $order->billing_city = $city;
      $order->billing_postal_code = $zip;
      $order->billing_country = db_result(db_query("SELECT country_id FROM {uc_countries} WHERE country_iso_code_2 = '%s'", trim($country)));
      $order->billing_zone = db_result(db_query("SELECT zone_id FROM {uc_zones} WHERE zone_country_id = %d AND zone_code = '%s'", $order->billing_country, trim($state)));

      $order->delivery_first_name = $first_name;
      $order->delivery_last_name = $last_name;
      $order->delivery_street1 = $street;
      $order->delivery_street2 = $street2;
      $order->delivery_city = $city;
      $order->delivery_postal_code = $zip;
      $order->delivery_country = $order->billing_country;
      $order->delivery_zone = $order->billing_zone;

      uc_order_save($order);
    }

    // Add store comment for additional information supplied
    // at PayPal site.
    $comment = trim(check_plain($_POST['memo']));
    if (!empty($comment)) {
      uc_order_comment_save($order_id, 0, t('Customer note from PayPal IPN: ') . $comment, 'order');
    }

/* Duplicate txn_id check removed here. PayPal uses the same txn_id for each payment for a subscription.
   This ends a section of modifications for PayPal Buttons functionality. */

    db_query("INSERT INTO {uc_payment_paypal_ipn} (order_id, txn_id, txn_type, mc_gross, status, receiver_email, payer_email, received) VALUES (%d, '%s', '%s', '%s', '%s', '%s', '%s', %d)",
      $order_id, $txn_id, $txn_type, $payment_amount, $payment_status, $receiver_email, $payer_email, time());

    $context = array(
      'revision' => 'formatted-original',
      'location' => 'paypal-ipn',
    );
    $options = array(
      'sign' => FALSE,
    );

    switch ($payment_status) {
      case 'Canceled_Reversal':
        uc_order_comment_save($order_id, 0, t('PayPal has cancelled the reversal and returned !amount !currency to your account.', array('!amount' => uc_price($payment_amount, $context, $options), '!currency' => $payment_currency)), 'admin');
        break;

      case 'Completed':
        $comment = t('PayPal transaction ID: @txn_id', array('@txn_id' => $txn_id));
        uc_payment_enter($order_id, 'paypal_wps', $payment_amount, $order->uid, NULL, $comment);
        uc_cart_complete_sale($order);
        uc_order_comment_save($order_id, 0, t('Payment of @amount @currency submitted through PayPal.', array('@amount' => uc_price($payment_amount, $context, $options), '@currency' => $payment_currency)), 'order', 'payment_received');
        uc_order_comment_save($order_id, 0, t('PayPal IPN reported a payment of @amount @currency.', array('@amount' => uc_price($payment_amount, $context, $options), '@currency' => $payment_currency)));
        break;

      case 'Denied':
        uc_order_comment_save($order_id, 0, t("You have denied the customer's payment."), 'admin');
        break;

      case 'Expired':
        uc_order_comment_save($order_id, 0, t('The authorization has failed and cannot be captured.'), 'admin');
        break;

      case 'Failed':
        uc_order_comment_save($order_id, 0, t("The customer's attempted payment from a bank account failed."), 'admin');
        break;

      case 'Pending':
        uc_order_update_status($order_id, 'paypal_pending');
        uc_order_comment_save($order_id, 0, t('Payment is pending at PayPal: @reason', array('@reason' => _uc_paypal_pending_message(check_plain($_POST['pending_reason'])))), 'admin');
        break;

      // You, the merchant, refunded the payment.
      case 'Refunded':
        $comment = t('PayPal transaction ID: @txn_id', array('@txn_id' => $txn_id));
        uc_payment_enter($order_id, 'paypal_wps', $payment_amount, $order->uid, NULL, $comment);
        break;

      case 'Reversed':
        watchdog('melodev.paypal', 'PayPal has reversed a payment!', array(), WATCHDOG_ERROR);
        uc_order_comment_save($order_id, 0, t('Payment has been reversed by PayPal: @reason', array('@reason' => _uc_paypal_reversal_message(check_plain($_POST['reason_code'])))), 'admin');
        break;

      case 'Processed':
        uc_order_comment_save($order_id, 0, t('A payment has been accepted.'), 'admin');
        break;

      case 'Voided':
        uc_order_comment_save($order_id, 0, t('The authorization has been voided.'), 'admin');
        break;
    }

/* This begins a section of modifications for PayPal Buttons functionality. */

    // Trigger additional handlers for processed IPNs
    uc_paypal_buttons_process_ipn($order_id, $_POST, $order);

/* This ends a section of modifications for PayPal Buttons functionality. */

  }
  elseif (strcmp($response->data, 'INVALID') == 0) {
    watchdog('melodev.paypal', 'IPN transaction failed verification.', array(), WATCHDOG_ERROR);
    uc_order_comment_save($order_id, 0, t('An IPN transaction failed verification for this order.'), 'admin');
  }
}


function melodev_user_edit_page($category='account') {
  global $user;
  if($category == 'content_profile') {
    module_load_include('inc', 'node', 'node.pages');
    $type = 'profile';
    drupal_set_title(check_plain($user->name));
    $node = content_profile_load($type, $user->uid);
    if (!$node) {
      $node = array('uid' => $user->uid, 'name' => (isset($user->name) ? $user->name : ''), 'type' => $type, 'language' => '');
    }
    $output = drupal_get_form($type .'_node_form', $node);
  } else if($category == 'Delete') {
    module_load_include('inc', 'user', 'user.pages');
    $output = drupal_get_form('user_confirm_delete', $user);
  } else {
    module_load_include('inc', 'user', 'user.pages');
    drupal_set_title(check_plain($user->name));
    $output = drupal_get_form('user_profile_form', $user, $category);
  }
  return $output;
}

/**
 * Page callback for elite sales.
 *
 * @TODO Make this internationalizationizable.
 */
function melodev_elite_page() {
  global $user;
  
  drupal_add_css(path_to_theme() . '/css/elite.css');
  
  $output = '';
  
  // USER IS ELITE
  if(in_array('elite', $user->roles)) {
    $output .= '<div id="elite-expiration"><h1 class="title">Expiration</h1>';
    $found = false;
    $expirations = db_query("SELECT * FROM {uc_roles_expirations} WHERE uid = %d", $user->uid);
    while ($expiration = db_fetch_object($expirations)) {
      $found = true;
      $output .= 'You are elite until <strong>'.format_date($expiration->expiration, 'custom', 'M d, Y').'</strong>, or <strong>'.format_interval($expiration->expiration - mktime(), 2).'</strong>.';
    }
    if(!$found) {
      $output .= 'You are elite until <strong>forever</strong>.  Gee, isn\'t that nice?';
    }
    $output .= '</div>';

/*    
    if($fees = uc_recurring_user_fees($user)) {
      $output .= '<div id="elite-subscription"><h1 class="title">Your Elite Subscription</h1>'.$fees.'</div>';
    } else {
      $output .= l('Renew your membership', 'cart/add/e-p465q1m0', array('query'=>'destination=cart/checkout', 'class'=>'elite-renew'));
    }
*/  
  
  // USER IS NOT ELITE
  }  else {
    if($user->uid == 1) { $output .= l('edit elite node', 'node/464/edit', array('query'=>'destination=elite')); }
  
    $output .= '<div id="go-elite">';
    $output .= '<h1 class="title">It\'s time for you to go ELITE.</h1>';
    $node = node_load(array('nid'=>464));
    $output .= node_view($node, false, true);    
    $output .= '</div>';
  }

  // CHECK IF USER HAS RECEIVED ANY ELITE GIFTS
  if($user->uid > 0) {
    $sql = "SELECT o.order_id, op.order_product_id, op.data FROM {uc_orders} AS o LEFT JOIN {uc_order_statuses} AS os ON o.order_status = os.order_status_id LEFT JOIN {uc_order_products} AS op ON o.order_id = op.order_id WHERE os.order_status_id ='completed' and op.data like '%\"%s\"%'";
    $result = db_query($sql, $user->name);
    $orders = array();
    while($obj = db_fetch_object($result)) {
      $orders[$obj->order_product_id] = uc_order_load($obj->order_id);
    }
    if(count($orders) > 0) {
      $output .= '<div id="elite-gifts"><h1 class="title">You\'ve Received Gifts!</h1>';
      $rows = array();
      foreach($orders as $order_product_id => $order) {
        $acct = user_load($order->uid);
        foreach($order->products as $product) {
          // Account for multiple gifts in one order
          if($product->order_product_id == $order_product_id) {
            $rows[] = array(format_date($order->created), l($acct->name, 'user/'.$acct->uid), t('Elite Year'));
          }
        }
      }
      $head = array('Date', 'Gifter', 'Gift');
      $output .= theme('table', $head, $rows);
      $output .= '</div>';
    }
  }
  
  // GIFTING
  $output .= '<div id="elite-gift"><h1 class="title">'.l('Give the Gift of Elite', 'node/469').'</h1>';
  $output .= l('Click here to make someone\'s day.', 'node/469', array('query'=>'destination=cart/checkout'));
  $output .= '</div>';

  // HELP
  $output .= '<div id="elite-help"><h1 class="title">I Need Help!</h1>';
  $output .= 'Please '.l('send us an email', 'contact').' and we will get back to you ASAP.  The more information you give us the better and your problems will be resolved right away.';
  $output .= '</div>';
  
  return $output;
}

/**
 * Page callback for enemies.
 */
function melodev_enemy_page() {
  global $user;
  // Is logged in
  if($user->uid < 1) {
    drupal_set_message('You must be logged in to have enemies.');
    drupal_goto('<front>');
  }
  // URL user is valid
  if($enemy = user_load(array('name'=>arg(1)))) {
    // User is an enemy
    if(!melodev_is_enemy($user->uid, $enemy->uid)) {
      drupal_set_message('That user is not your enemy.');
      drupal_goto('user/'.$enemy->uid);
    }
  
    drupal_set_title('You have made a powerful enemy, my friend.');
    
    $flag = flag_get_flag('enemy');
    $flags = flag_get_user_flags('user', $enemy->uid);
    if(!isset($flags['enemy'])) {
      $enemy_link = $flag->theme('flag', $enemy->uid);
    } else {
      $enemy_link = $flag->theme('unflag', $enemy->uid);
    }
    
    /*
    // No reasons for now
    $output = '<div class="reasons">';
    $reasons = melodev_get_enemy_reasons($user->uid, $enemy->uid);
    $output .= '<h2>Reasons they hate you:</h2>';
    $output .= '<p>'.$reasons[$enemy->uid].'</p>';
    $output .= '<h2>Reasons you hate them:</h2>';
    $output .= '<p>'.$reasons[$user->uid].'</p>';
    $output .= '</div>';
    */
    
    $output .= '<div class="links">';
    $output .= $enemy_link.' | '.l('Return to Melo', '<front>');
    $output .= '</div>';
    return $output;
    
  
  } else {
    drupal_set_message('No user by the name of <i>'.arg(1).'</i> exists.');
    drupal_goto('<front>');
  }
  exit();
}

/**
 * Page callback for karma display.
 */
function melodev_karma_page($account=null) {

  drupal_set_title('Your Karma');

  if(!$account) {
    global $user;
    $account = $user;
  }

  // Calculate it
  $karma = melodev_karma($account, true);
  
  $output = '';
  
  // Load elite permissions
  $elite = array();
  $result = db_query("SELECT p.perm FROM {role} r INNER JOIN {permission} p ON p.rid = r.rid WHERE r.rid = %d", MELO_ELITE_RID);
  while($row = db_fetch_object($result)) {
    $elite = explode(',', $row->perm);
  }
  foreach($elite as $k => $v) {
    $elite[$k] = trim($v);
  }
  
  // Load flags
  $flags = array();
  $res = db_query('select * from {flags}');
  while($flag = db_fetch_object($res)) {
    $flag->roles = explode(',', $flag->roles);
    foreach($flag->roles as $rid) {
      $flags[$rid][] = $flag;
    }
  }
  $myflags = flag_get_flags(NULL, NULL, $account);
  
  // Show Karma Rewards
  $output .= '<h2 class="title">'.t('Karma Rewards').'</h2>';
  $rows = array();
  $roles = user_roles(true);
  foreach($roles as $rid => $name) {
    if($level = variable_get('karma_rid_'.$rid, false)) {
      $rows[] = array('<strong>'.t('Level !level'.'</strong>', array('!level'=>$level)), ($karma['karma'] >= $level) ? '<div class="confirm"></div>' : '<div class="deny"></div>', '');
      // Load permissions for this role
      $result = db_query('SELECT r.rid, p.perm FROM {role} r LEFT JOIN {permission} p ON r.rid = p.rid WHERE r.rid = %d', $rid);
      while($row = db_fetch_array($result)) {
        if($row['perm']) {
          $perms = explode(',', $row['perm']);
          foreach($perms as $perm) {
              $perm = trim($perm);
              // Check for elite
              if(in_array($perm, $elite)) {
                if(in_array('elite', $account->roles)) {
                  $elitetxt = '<div class="confirm"></div>';
                } else {
                  $elitetxt = '<div class="deny"></div>';
                }
              } else {
                $elitetxt = '';
              }
              // Only include if we've made a nicer node for it
              $res = db_query('select nid from {content_type_feature} where field_feature_keyword_value = "%s"', $perm);
              if($row = db_fetch_array($res)) {
                $node = node_load($row['nid']);
                // a href="/feature/email" rel="lightmodal[elite|width:700|height:500][Elite: Email]
                $permview = l($node->title, 'node/'.$node->nid, array('attributes'=>array('rel'=>'lightmodal[elite|width:700|height:500]['.$node->title.']')));
              } else if(user_access('administer nodes')) {
                $permview = $perm;
              }
              $rows[] = array($permview, (user_access($perm)) ? '<div class="confirm"></div>' : '<div class="deny"></div>', $elitetxt);

          }
        }
      }
      // Load input formats for this role

      // Build query for selecting the format(s) based on the user's roles.
      $query = 'SELECT * FROM {filter_formats} WHERE roles like "%%,%d,%%"';
      $result = db_query($query, $rid);
      while ($format = db_fetch_object($result)) {
        if(is_numeric($format->format)) {
          // Check for elite
          if(eregi(MELO_ELITE_RID, $format->roles)) {
            if(in_array('elite', $account->roles)) {
              $elitetxt = '<div class="confirm"></div>';
            } else {
              $elitetxt = '<div class="deny"></div>';
            }
          } else {
            $elitetxt = '';
          }
          $rows[] = array($format->name, (filter_access($format->format)) ? '<div class="confirm"></div>' : '<div class="deny"></div>', $elitetxt);
        }
      }
      
      // Load flags for this role
      if(isset($flags[$rid])) {
        foreach($flags[$rid] as $flag) {
          // Check if I have it
          if(isset($myflags[$flag->name])) {
            $have = '<div class="confirm"></div>';
          } else {
            $have = '<div class="deny"></div>';
          }
          // Check if elites get it
          if(in_array(MELO_ELITE_RID, $flag->roles)) {
            if(in_array('elite', $account->roles)) {
              $elitetxt = '<div class="confirm"></div>';
            } else {
              $elitetxt = '<div class="deny"></div>';
            }
          } else {
            $elite = '';
          }
          // Check if there's a help node for it
          $res = db_query('select nid from {content_type_feature} where field_feature_keyword_value = "%s"', $flag->name);
          if($row = db_fetch_array($res)) {
            $node = node_load($row['nid']);
            $flagview = l($node->title, 'node/'.$node->nid, array('attributes'=>array('rel'=>'lightmodal[elite|width:700|height:500]['.$node->title.']')));
          } else {
            $flagview = 'Flag' . $flag->title . l('add feature', 'node/add/feature', array('query'=>'field_feature_keyword_value='.$flag->name));
          }
          $rows[] = array($flagview, $have, $elitetxt);
        }
      }

    }
  }
  $head = array('Reward', 'Success', 'Elite');
  $output .= theme('table', $head, $rows);
  
  $output .= '<br /><br />';
  
  // Do the Karma Breakdown
  $output .= '<h2 class="title">'.t('Karma Breakdown').'</h2>';
  $output .= '<p class="description">'.t('Karma points are not a direct result of your stats, but rather a complicated algorithm similar to an RPG.  The idea is that the more you do one thing, the less points you\'ll get for it.  This encourages wider participation, and discourages spamming, as well as keeping things interesting for long-time members.  The chart below will tell you where you\'re getting your karma from, but not your current stat levels.').'</p>';
  $rows = array();
  foreach($karma as $label => $value) {
    if($label != 'karma') {
      $rows[] = array(melodev_get_stat_label($label), number_format($value));
    }
  }
  $rows[] = array('<strong>'.t('Grand Karma Total').'</strong>', '<strong>'.number_format($karma['karma']).'</strong>');
  $head = array('Your Good Deeds and Bad Behavior', 'Karma Points');
  $output .= theme('table', $head, $rows);
  
  
  return $output;
}

/**
 * Page callback for My Melo You page
 */
function melodev_you_page() {
  global $user;
  
  drupal_set_title('All About You');
  
  $output = $user->name;
  
  return $output;
}


/**
 * Page callback for the Random Tour
 */
function melodev_random_tour() {
  global $user;
  
  $random_levels = array(
    10 => 'You\'re not bad.',
    50 => 'You\'re alright.',
    100 => 'You\'re pretty cool.',
    500 => 'You\'re kinda hot.',
    1000 => 'I think I love you.',
    1500 => 'Are you serious?',
    2000 => 'This is getting kind of intense.',
    2500 => 'You\'re alright.',
    3000 => 'You\'re kind of intense.',
    4000 => 'You\'re kind of creepy.',
    5000 => 'You\'re out of control.',
    6000 => 'You smell nice.',
    7000 => 'You\'re really getting somewhere now.',
    8000 => 'You\'re almost to something now.',
    9000 => 'Not really, I was just leading you on.',
    10000 => 'YOU WIN!  THIS IS IT!  YOU\'RE THE BEST!',
    25000 => 'I told you that was it, it\'s over, really.',
    75000 => 'I don\'t even know what to say.',
    150000 => '*speechless*',
    250000 => 'I think we\'ve come to the end of our journey together.',
    500000 => 'You made it all the way.  And for that you have my undying love.  The code to the secret treasure buried on Vulcan Point is 4-8-15-16-23-32.  Be swift my child.',
  );

  $arg = arg(1);
  $output = '';
  
  switch($arg) {
    case 'start':
      // Decide on our total
      if($prevtour = db_fetch_object(db_query('select * from {karma_bonus} where uid = %d and name like "Random-%" order by timestamp desc limit 1', $user->uid))) {
        $prevtotal = substr($prevtour->name, 7);
        foreach($random_levels as $level => $message) {
          if($prevtotal >= $level) {
            continue;
          } else {
            $total = $level;
            break;
          }
        }
      } else {
        foreach($random_levels as $level => $message) {
          $total = $level;
          break;
        }
      }
    
      // Clear any old tours
      db_query('delete from {random_tour} where uid = %d and type = "%s"', $user->uid, 'random');
      // Create a random tour for this user
      $users = array();
      $result = db_query('select distinct uid from {users} where uid > 0 and uid != %d and status = 1 and created < %d order by access desc limit %d', $user->uid, (mktime() - (60*60*24)), $total);
      while($row = db_fetch_array($result)) {
        $users[$row['uid']] = $row['uid'];
      }
      // Pick the first user
      $first_user = array_rand($users);
      $tour = array(
        'begin' => mktime(),
        'last_update' => mktime(), 
        'last_user' => 0,
        'active_user' => $first_user,
        'users' => $users,
        'seen_users' => array(),
        'total_users' => $total,
        );
      $random = (object)array(
        'uid' => $user->uid,
        'type' => 'random',
        'last_update' => mktime(), // This is the last time the user was changed
        'tour' => serialize($tour),
        );
      drupal_write_record('random_tour', $random);
      drupal_goto('user/'.$first_user);
    break;
    
    case 'next':
      if($row = db_fetch_object(db_query('select * from {random_tour} where active = 1 and uid = %d and type = "%s"', $user->uid, 'random'))) {
        $tour = unserialize($row->tour);
        
        // Check if it's been less than 25 seconds
        if((mktime() - $tour['last_update']) < 25) {
          $tour['last_update'] = mktime();
          $row->tour = serialize($tour);
          drupal_write_record('random_tour', $row, array('uid', 'type'));
          drupal_set_message('Yeeeeeeah, that wasn\'t enough time.');
          drupal_goto('user/'.$tour['active_user']);
        }
        
        // Otherwise update the tour
        $active = $tour['active_user'];
        $tour['seen_users'][$active] = (mktime() - $tour['last_update']);
        unset($tour['users'][$active]);
        $tour['last_user'] = $active;
        
        // Get a new user
        if($active = array_rand($tour['users'])) {
          $tour['active_user'] = $active;
          // Save the tour
          $tour['last_update'] = mktime();
          $row->tour = serialize($tour);
          $row->last_update =mktime();
          drupal_write_record('random_tour', $row, array('uid', 'type'));
          
        // Check if they finished it
        } else if(count($tour['users']) == 0 && count($tour['seen_users']) == $tour['total_users'] && $tour['total_users'] > 0) {
          // Save a karma bonus
          $level = count($tour['seen_users']);
          $karma = variable_get('melodev_karma_random_tour', 0.1) * $level;
          $remark = $random_levels[$level];
          $message = 'Congratulations.  You completed a random tour at level '.$level.' and for that you have received '.$karma.' karma.  '.$remark;
          $bonus = (object)array(
            'uid'=>$user->uid,
            'karma' => $karma,
            'name' => 'Random-'.$level,
            'message' => $message,
            'timestamp' => mktime()
            );
          drupal_write_record('karma_bonus', $bonus);
          // Delete the random tour
          db_query('delete from {random_tour} where uid = %d and type = "%s"', $user->uid, 'random');
          // Redirect
          drupal_set_message($message);
          drupal_goto('random');
        } else {

        }
/*
        echo '<pre>';
        echo 'active='.$active."\n";
        print_r($tour);
        print_r($row);
        exit();
*/
        drupal_goto('user/'.$active);        
      } else {
        drupal_set_message('No active tours were found.');
        drupal_goto('random');
      }
    break;
    case 'pause':
      // Pause this tour
      db_query('update {random_tour} set active = 0 where uid = %d and type = "%s"', $user->uid, 'random');
      drupal_set_message('Random tour pause successful.');
      drupal_goto('random');
    break;

    case 'cancel':
      // Cancel this tour
      db_query('delete from {random_tour} where uid = %d and type = "%s"', $user->uid, 'random');
      drupal_set_message('Random tour has been cancelled.  You suck.');
      drupal_goto('random');
    break;

    case 'reactivate':
      // Reactivate this tour
      db_query('update {random_tour} set active = 1 where uid = %d and type = "%s"', $user->uid, 'random');
      $row = db_fetch_object(db_query('select * from {random_tour} where active = 1 and uid = %d and type = "%s"', $user->uid, 'random'));
      $tour = unserialize($row->tour);
      // Cancel it if you can't find an active user
      if(!is_numeric($tour['active_user']) || $tour['active_user'] < 1) {
        drupal_set_message('Sorry, an error occured attempting to reactive your random tour, so we had to cancel it.  Better luck next time.');
        db_query('delete from {random_tour} where uid = %d and type = "%s"', $user->uid, 'random');
        drupal_goto('random');
      }
      drupal_set_message('Random tour successfully reactivated.');
      drupal_goto('user/'.$tour['active_user']);
    break;
  
    default:
      // Random Tour Page
      drupal_set_title('Welcome to the Random Tour');
      
      $output .= t('Take a ride on the random tour, and get to know some of the highlights of the population on Melodramatic.com.  Depending on your level, we\'ll randomly select a batch of people for you to browse through.  Make friends, whore for karma, kill some time, and really see Melo for everything it is.  Karma rewards are available for successful completion of the tours.');
      
      // Check for an active tour
      if($tour = db_fetch_object(db_query('select * from {random_tour} where uid = %d and type = "%s"', $user->uid, 'random'))) {
        $data = unserialize($tour->tour);
        if($tour->active == 1) {
          $output .= '<p>You have an <strong>active</strong> tour <strong>('.count($data['seen_users']).'/'.count($data['users']).')</strong>.  '.l('Pause Tour', 'random/pause').' | '.l('Cancel Tour', 'random/cancel').'</p>';
        } else {
          $output .= '<p>You have a <strong>paused</strong> tour <strong>('.count($data['seen_users']).'/'.count($data['users']).')</strong>.  '.l('Reactivate Tour', 'random/reactivate').' | '.l('Cancel Tour', 'random/cancel').'</p>';
        }
      } else {
        $output .= '<center><h2 class="title">'.l('Start Your Random Tour Now', 'random/start').'</h2></center>';
      }
    break;
  }  
  return $output;
}

/**
 * Page callback for My Melo You page
 */
function melodev_mycliques_page() {
  return 'coming soon';
}

/**
 * Page callback for My Melo You page
 */
function melodev_active_users_load() {
  global $user;
  // Load wall active users
  $node = node_load(24);
  $wall_users = tribune_get_active_users($node);
  $wall_ids = array();
  foreach($wall_users as $wuser) {
    $wall_ids[] = $wuser['uid'];
  }
  $view = views_get_view('ActiveUsers');
  $view->pager['items_per_page'] = 200;
  $view->execute();
  $wall = array();
  $friends = array();
  $others = array();
  foreach($view->result as $key => $row) {
    // Skip yourself
    if($row->uid == $user->uid) {
      continue;
    }
    $found = false;
    // Check if one of my friends
    $flags = flag_get_user_flags('user', $row->uid);
    if(isset($flags['friend'])) {
      $friends[] = $row;
      $found = true;
    }
    // Check if on the wall
    if(in_array($row->uid, $wall_ids)) {
      $wall[] = $row;
      $found = true;
    }
    // Otherwise they go in the rest
    if(!$found) {
      $others[] = $row;
    }
  }
//  $output = '<h1>Active Users ('.count($view->result).')</h1>';
  $output = '';
  if(count($friends) > 0) {
    $output .= '<h2>Friends</h2>';
    $view->result = $friends;
    $output .= $view->render();
  }
  if(count($wall) > 0) {
    $output .= '<h2>On the Wall</h2>';
    $view->result = $wall;
    $output .= $view->render();
  }
  if(count($others) > 0) {
    $output .= '<h2>Everyone Else</h2>';
    $view->result = $others;
    $output .= $view->render();
  }  
  
  $vars = array(
    'html_content' => $output, 
    'success' => 1,
    );
  
  // Now print it in XML
  $print = '';
  foreach ($vars as $name => $value) {
    switch ($name) {
      case 'html_content':
      case 'error_message':
      case 'success_message':
        $print .= sprintf('<%s><![CDATA[%s]]></%s>', $name, $value, $name);
        break;
      default :
        $print .= sprintf('<%s>%s</%s>', $name, $value, $name);
        break;
    }//end - switch
  }//end - foreach
  $print = '<response>
' . $print ."</response>";

  header("content-type: text/xml");
  echo $print;
  exit;
}

/**
* @} End of "melodev_pages"
*/