<?php

/**
 * @file
 * Individual admin pages being output by Melodev.
 *
 */

/**
 * @defgroup melodev_admin_pages Melodev Admin Pages
 *
 * Custom administration pages on Melodramatic.com.
 *
 * @{
 */

/**
 * Page callback for front page poll rotation.
 */
function melodev_rotate_fppoll_page() {
  module_load_include('inc', 'melodev', 'melodev.cron');
  if(melodev_cron_fppoll_rotate(true)) {
    $msg = 'The front page poll has been rotated.';
  } else {
    $msg = 'No front page poll found.';
  }
  drupal_set_message($msg);
  drupal_goto('<front>');
}

/**
 * Page callback for front page art rotation.
 */
function melodev_rotate_fpart_page() {
  if(melodev_cron_fpart_rotate(true)) {
    $msg = 'The front page art has been rotated.';
  } else {
    $msg = 'No front page art found.';
  }
  drupal_set_message($msg);
  drupal_goto('<front>');
}

/**
 * Page callback for Block Cache Management
 */
function melodev_block_cache_page() {
  
  $output = '<p>Manage block cache.</p>';
  
  // Fetch regions
  $block_regions = system_region_list('melo') + array(BLOCK_REGION_NONE => 'none');

  // Fetch and sort blocks
  $blocks = _block_rehash();
  usort($blocks, '_block_compare');
  $regions = array();
  foreach($blocks as $block) {
    $key = $block_regions[$block[region]];
    $regions[$key][] = $block;
  }

  // Blockcache options.
  $block_cache_options = array(
    BLOCK_NO_CACHE => t('Do not cache'),
    BLOCK_CACHE_GLOBAL => t('Cache once for everything (global)'),
    BLOCK_CACHE_PER_PAGE => t('Per page'),
    BLOCK_CACHE_PER_ROLE => t('Per role'),
    BLOCK_CACHE_PER_ROLE | BLOCK_CACHE_PER_PAGE => t('Per role per page'),
    BLOCK_CACHE_PER_USER => t('Per user'),
    BLOCK_CACHE_PER_USER | BLOCK_CACHE_PER_PAGE => t('Per user per page'),
  );
  
  // Display output
  foreach($regions as $key => $blocks) {
    if($key == 'none') {
      continue;
    }
    $rows = array();
    $head = array('Block Title', 'Pages', 'Cache', 'Edit');
    $output .= '<h1 class="title">'.$key.'</h1>';
    foreach($blocks as $block) {
      $bcvar = $block['module'].'_'.$block['delta'];
      if(variable_get('bc_refresh_'.$bcvar, false) || variable_get('bc_life_'.$bcvar, false)) {
        $cache = '<strong>'.$block_cache_options[$block['cache']].'</strong>';
      } else {
        $cache = 'Permanent';
      }
      $row = array($block['info'], '<pre style="font-size:0.8em">'.$block['pages'].'</pre>', $cache, 
                   l('edit', 'admin/build/block/configure/'.$block['module'].'/'.$block['delta'], array('query'=>'destination=admin/build/cache_browser/blocks')), 
                   );
      $rows[] = $row;
    }
    $output .= theme('table', $head, $rows);
//    $output .= '<pre>'.print_r($blocks, true).'</pre>';
  }
  
  return $output;
}

/**
 * Page callback for View Cache Management
 */
function melodev_view_cache_page() {
  $output = '<p>Manage view cache.</p>';
  
  $views = views_get_all_views(true);
  
  $caches = array();
  foreach($views as $name => $view) {
    $displays = $view->display;
    foreach($displays as $dispname => $display) {
      $cachetxt = '';
      if(is_array($display->display_options['cache'])) {
        $cache = $display->display_options['cache'];
        if($cache['type'] == 'time') {
          $cachetxt .= 'Cache Type: <strong>Time</strong><br />';
          $cachetxt .= 'Results: <strong>'.$cache['results_lifespan'].'</strong><br />';
          $cachetxt .= 'Display: <strong>'.$cache['output_lifespan'].'</strong><br />';
        } else {
          $cachetxt .= '<pre>'.print_r($cache, true).'</pre>';
        }
      } else {
        $cachetxt .= '<em>No Cache</em>';
      }
      $caches[] = array($name, $dispname, $cachetxt);
    }
  }
  $output .= theme('table', array('View Name', 'Display Name', 'Cache'), $caches);
  
  /*
  $output .= print_r($views, true);
  $output .= '</pre>';
  */
  
  return $output;
}

/**
 * Rebuild User Access.
 */
function melodev_rebuild_user_access() {
  global $user;
  $uid = arg(3);
  if(!is_numeric($uid) || $uid < 1) {
    drupal_set_message('Please select a valid user to rebuild.');
    drupal_goto();
  }
  if(arg(4) != 'rebuild') {
    $account = user_load($uid);
    $form_state = array('uid' => $user->uid);
    $output = drupal_get_form('melodev_rebuild_user_access_form', $form_state, $account);
    return $output;
  }
}

/**
 * Reimport User Entries.
 */
function melodev_reimport_user_entries() {
  global $user;
  $uid = arg(4);
  if(!is_numeric($uid) || $uid < 1) {
    drupal_set_message('Please select a valid user to reimport.');
    drupal_goto();
  }
  if(arg(5) != 'rebuild') {
    $account = user_load($uid);
    $form_state = array('uid' => $user->uid);
    $output = drupal_get_form('melodev_reimport_user_entries_form', $form_state, $account);
    return $output;
  }
}

/**
 * Unlock a cron.
 */
function melodev_cron_page_unlock() {
  $cronjob = arg(4);
  variable_set('lock_'.$cronjob, false);
  drupal_set_message('Cron lock on <b>'.$cronjob.'</b> has been cleared.');
  drupal_goto('admin/god');
}

/**
 * Add User to Reimport Entries Queue.
 */
function melodev_reimport_user_queue() {

  global $user;
  $uid = arg(4);
  if(!is_numeric($uid) || $uid < 1) {
    drupal_set_message('Please select a valid user to reimport.');
    drupal_goto();
  }
  
  // Make sure they're not already queued
  if(db_fetch_object(db_query('select * from {melodev_reimport_user} where uid = %d', $uid))) {
    drupal_set_message('User with ID '.$uid.' has already been queued for reimport.  If you think this message is an error please contact a developer.');
    drupal_goto('user/'.$uid);
  }

  // Save to reimport queue for cron processing
  db_query('insert into melodev_reimport_user (uid) values(%d)', $uid);
  drupal_set_message('User with ID '.$uid.' has been queued for reimport.');
  drupal_goto('user/'.$uid);
}

/**
 * Reimport one Node.
 */
function melodev_reimport_node_page() {
  $nid = arg(4);
  if(!is_numeric($nid) || $nid < 1) {
    drupal_set_message('Please select a valid node to reimport.');
    drupal_goto();
  }
  $node = node_load($nid);
  if(!is_object($node) || $node->type != 'blog') {
    drupal_set_message('Please select a valid node to reimport.');
    drupal_goto();
  }
  $oldid = db_result(db_query('select oldid from melodev_node_map where newid = %d', $nid));
  if(!is_numeric($oldid) || $oldid < 1) {
    drupal_set_message('Could not find a node map for nid '.$nid.'.');
    drupal_goto('node/'.$nid);  
  }
  // Reimport this node
  if(melodev_reimport_node($oldid, $node, true)) {
    drupal_set_message('Successfully reimported node '.$nid.'.');
    drupal_goto('node/'.$nid);  
  } else {
    drupal_set_message('Unable to reimport node '.$nid.'.');
    drupal_goto('node/'.$nid);  
  }
}

/**
 * Elite Expiration Comparison
 */
function melodev_elite_expiration_comparison() {
  $table = array();
  $res = melodev_pg_good_query('select distinct * from elite_accts order by entity_id asc');
  $i=1;
  foreach($res as $row) {
    $newexpire = db_result(db_query('select expiration from uc_roles_expirations where uid = %d and rid = %d', $row['entity_id'], MELO_ELITE_RID));
    if($row['expire_date'] > $newexpire) {
      $oldexpire = '<span class="red">'.date('M/d/Y', $row['expire_date']).'</span>';
    } else {
      $oldexpire = date('M/d/Y', $row['expire_date']);
    }
    if($newexpire > 0) {
      $newview = date('M/d/Y', $newexpire);
    }
    if($newexpire < mktime()) {
      $newview = '<font color="#aaaaaa">'.$newview.'</font>';
    }
    $estexp = db_result(db_query('select expiration from melodev_expiration_estimate where uid = %d', $row['entity_id']));
    if($estexp) {
      $estview = date('M/d/Y', $estexp);
      if(abs($estexp - $row['expire_date']) < 100) {
        $estdiff = '<b>'.format_interval(abs($estexp - $row['expire_date'])).'</b>';
      } else {
        $estdiff = format_interval(abs($estexp - $row['expire_date']));
      }
    } else {
      $estview = '';
      $estdiff = '';
    }
    $table[] = array(
      $i,
      l($row['entity_id'], 'admin/god/orders/'.$row['entity_id']),
      $oldexpire,
      $newview,
      format_interval(abs($newexpire - $row['expire_date'])),
      $estview,
      $estdiff,
      );
    $i++;
  }
  $head = array('#', 'UID', 'Old Expiration', 'New Expiration', 'Difference', 'Estimated', 'Difference');
  $output .= theme('table', $head, $table);
  return $output;
}

/**
 * Historic 2.0 => 3.0 Order Report.
 */
function melodev_historic_order_report() {
  $statuses = array(
 	'INITIAL'=>0
	,'PENDING'=>1
	,'PAID'=>2
	,'PACKAGED'=>3
	,'SHIPPED'=>4
	,'COMPLETE'=>5
	,'REVERSE'=>9
  );
  
  $uid = arg(3);
  if(!is_numeric($uid) || $uid < 1) {
    drupal_set_message('Please select a valid user to reimport.');
    drupal_goto();
  } else {
    $account = user_load($uid);
  }
  drupal_set_title('Historic Orders Report for '.$account->name);
  $output = '<p>This report displays a comparison of Melo 2.0 order history to Melo 3.0 order history for a particular user.  This report will be required for testing the ongoing changes to elite accounts and old database imports.';
  
  // Comparison Summaries
  $output .= '<h2 class="title">Comparison Summary</h2>';
  
  // Elite expiration comparisons
  $res = melodev_pg_good_query('select expire_date from elite_accts where entity_id = '.$uid.' limit 1');
  $oldexpire = $res[0]['expire_date'];
  $newexpire = db_result(db_query('select expiration from uc_roles_expirations where uid = %d and rid = %d', $uid, MELO_ELITE_RID));
  if($oldexpire > $newexpire) {
    $oldexpire = '<span class="red">'.date('M/d/Y', $oldexpire).'</span>';
  } else {
    $oldexpire = date('M/d/Y', $oldexpire);
  }
  // Order Count comparisons
  $res = melodev_pg_good_query('select count(id) as total from product_order where purchaser_entity_id = '.$uid);
  $oldtotal = $res[0]['total'];
  $newtotal = db_result(db_query('select count(order_id) from uc_orders where uid = %d', $uid));
  if($oldtotal > $newtotal) {
    $oldtotal = '<span class="red">'.$oldtotal.'</span>';
  }
  // Payment Count comparisons
  $res = melodev_pg_good_query('select count(payment_id) as paytotal from payments where entity_id = '.$uid);
  $paytotal = $res[0]['paytotal'];
  $newpaytotal = db_result(db_query('select count(receipt_id) from uc_payment_receipts where uid = %d', $uid));
  if($paytotal > $newpaytotal) {
    $paytotal = '<span class="red">'.$paytotal.'</span>';
  }
  // Subscriptions comparisons
  $res = melodev_pg_good_query('select count(*) as paytotal from subscription where purchaser_entity_id = '.$uid.' and active = 1');
  $subtotal = $res[0]['paytotal'];
  $newsubtotal = db_result(db_query('select count(*) from {uc_paypal_subscriptions} where uid = %d', $uid));
  if($subtotal > $newsubtotal) {
    $subtotal = '<span class="red">'.$subtotal.'</span>';
  }
  $table = array(
    array('Account Expires:', $oldexpire, date('M/d/Y', $newexpire)),
    array('Order Count:', $oldtotal, $newtotal),
    array('Payment Count:', $paytotal, $newpaytotal),
    array('Subscription Count:', $subtotal, $newsubtotal),
    );
  $head = array('', 'Old Value', 'New Value');
  $output .= theme('table', $head, $table);

  // Estimate real Expiration
  $output .= '<h2 class="title">Estimated Expiration</h2>';
  $array = melodev_estimate_current_expiration($uid);
  $expiration = $array['expiration'];
  $output .= '<p>Truthiest Expiration: <strong>'.date('M/d/Y', $expiration).'</strong></p>';
  $output .= '<strong>Time Blocks:</strong>';
  //<pre>'.print_r($array['timeblocks'], true).'</pre><br />';
  $output .= theme('table', array('payment_id', 'days', 'start', 'end', 'payment date'), $array['timeblocks']);
  
  // New Orders
  $output .= '<h2 class="title">New Order History</h2>';
  $output .= '<p>These are orders that have been created since December 28th 2009, when Melo was down for upgrades.</p>';
  $table = array();
  $res = db_query('select order_id, order_status, order_total, product_count, payment_method, created from uc_orders where uid = %d and created > 1261980000', $uid);
  $k=0;
  while($row = db_fetch_array($res)) {
    $newrow = $row;
    $newrow['created'] = date('M/d/Y g:ia', $row['created']);
    $newrow['order_id'] = l($newrow['order_id'], 'user/'.$uid.'/order/'.$newrow['order_id']);
    $table[] = $newrow; 
  }
  $output .= theme('table', array('Order ID', 'Status', 'Total', 'Products', 'Payment Method', 'Created'), $table);
  
  // Old Orders
  $output .= '<h2 class="title">Old Order History</h2>';
  $table = array();
  $res = melodev_pg_good_query('select id, createdate, checkoutdate, orderstatus, payment_id from product_order where purchaser_entity_id = '.$uid.' order by createdate desc');
  $k=0;
  foreach($res as $row) {
    if($row['orderstatus'] < 2) {
      continue;
    }
    if($k % 2) {
      $class = 'admin-even';
    } else {
      $class = 'admin-odd';
    }
    // get the order items
    $items = melodev_pg_good_query('select productid, receiver_entity_id, quantity from product_order_items where orderid = '.$row['id']);
    // old row
    $oldrow = array(
      'data'=>array(
        '<strong>OLD</strong>',
        $row['id'], 
        date('M/d/Y', $row['checkoutdate']), 
        array_search($row['orderstatus'], $statuses),
        theme('table', array('Product ID', 'Receiver ID', 'Quantity'), $items),
        $row['payment_id']
      ),
      'class' => $class,      
    );
    $table[] = $oldrow;
    // new row
    $newid = db_result(db_query('select newid from {melodev_node_map} where oldid = %d and nodetype = "%s"', $row['id'], 'order'));
    if(is_numeric($newid)) {
      $newrow = db_fetch_array(db_query('select * from {uc_orders} where order_id = %d', $newid));
      $pid = db_result(db_query('select newid from {melodev_node_map} where oldid = %d and nodetype = "%s"', $row['payment_id'], 'payment'));
//      $payment = db_fetch_array(db_query('select * from {uc_payment_receipts} where order_id = %d', $newid));
      $newrow = array(
        'data'=>array(
          '<strong>NEW</strong>',
          l($newid, 'user/'.$uid.'/order/'.$newid), 
          date('M/d/Y', $newrow['modified']), 
          $newrow['order_status'],
          '',
          $pid,
        ),
        'class' => $class,      
      );
      $table[] = $newrow;
    }
    $k++;
  }
  $head = array('', 'ID', 'Checkout', 'Status', 'Items', 'Payment ID');
  $output .= theme('table', $head, $table, array('class'=>'admin-orders'));
  
  // Old Payments
  $output .= '<h2 class="title">Payment History</h2>';
  $table = array();
  $res = melodev_pg_good_query('select * from payments where entity_id = '.$uid.' order by timestamp desc');
  foreach($res as $row) {
    if($k % 2) {
      $class = 'admin-even';
    } else {
      $class = 'admin-odd';
    }
    // Check for matching new payment
    $newid = db_result(db_query('select newid from {melodev_node_map} where oldid = %d and nodetype = "%s"', $row['payment_id'], 'payment'));
    if(!$newid) {
      $old = '<span class="red">OLD</span>';
    } else {
      $old = 'OLD';
    }
    $table[] = array('data'=>array(
      $old,
      $row['payment_id'],
      $row['orderid'],
      $row['payment_type'],
      $row['txn_id'],
      $row['order_item'],
      '$'.$row['payment_amount'],
      date('M/d/Y', $row['timestamp']),
    ), 'class'=>$class);
    if($newid) {
      $payment = db_fetch_array(db_query('select * from {uc_payment_receipts} where receipt_id = %d', $newid));
      $table[] = array('data'=>array(
        'NEW',
        $payment['receipt_id'],
        $payment['order_id'],
        $payment['method'],
        str_replace('PayPal transaction ID: ', '', $payment['comment']),
        '',
        '$'.round($payment['amount'], 2),
        date('M/d/Y', $payment['received']),
      ), 'class'=>$class);
    }
    $k++;
  }
  $head = array('', 'ID', 'Order ID', 'Type', 'TXN ID', 'Item', 'Total', 'Date');
  $output .= theme('table', $head, $table, array('class'=>'admin-orders'));
    
  //        *************************************
  //        **       Old Received Items        **
  //        *************************************
  $output .= '<h2 class="title">Old Received Items</h2><p>Items that were received by this user, but not purchased by this user.  i.e. Gifts.</p>';
  $table = array();
  $res = melodev_pg_good_query('select * from product_order_items where receiver_entity_id = '.$uid.' and orderid not in(select id from product_order where purchaser_entity_id = '.$uid.') order by id desc');
  foreach($res as $row) {
    if($k % 2) {
      $class = 'admin-even';
    } else {
      $class = 'admin-odd';
    }
    // get the order
    $orders = melodev_pg_good_query('select * from product_order where id = '.$row['orderid']);
    $order = $orders[0];
    $newid = db_result(db_query('select newid from melodev_node_map where oldid = %d and nodetype = "%s"', $row['orderid'], 'order'));
    if(is_numeric($newid)) {
      $newview = l($newid, 'admin/store/orders/'.$newid);
    } else {
      $newview = '';
    }
    // get payment for this order
    $payments = melodev_pg_good_query('select payment_id, timestamp, payment_amount from payments where orderid = '.$order['id'].' order by timestamp desc');
    foreach($payments as $i => $pay) {
      $payments[$i]['timestamp'] = date('M/d/Y', $pay['timestamp']);
      $payments[$i]['payment_amount'] = '$'.$pay['payment_amount'];
    }
    $payview = theme('table', array(), $payments);
    $newrow = array(
      'product_id' => $row['productid'],
      'purchaser' => $order['purchaser_entity_id'],
      'created' => date('M/d/Y', $order['createdate']),
      'checkout' => date('M/d/Y', $order['checkoutdate']),
      'order_id' => $order['id'],
      'new_order_id' => $newview,
      'payments' => $payview,
      );
    $table[] = $newrow;
    $k++;
  }
  $head = array('Product', 'Purchaser', 'Created', 'Checkout', 'Order ID', 'New Order ID', 'Payments');
  $output .= theme('table', $head, $table, array('class'=>'admin-orders'));
    
  // Elite Time Blocks
  $output .= '<h2 class="title">Old Elite Time Blocks</h2>';
  $table = array();
  $res = melodev_pg_good_query('select distinct * from elite_time where entity_id = '.$uid.' order by start_date desc');
  foreach($res as $row) {
    $table[] = array(
      $row['time_id'],
      $row['payment_id'],
      $row['total_days'],
      date('M/d/Y', $row['start_date']),
      date('M/d/Y', $row['end_date']),
      $row['item_id']
    );
  }
  $head = array('ID', 'Payment ID', 'Days', 'Start', 'End', 'Item ID');
  $output .= theme('table', $head, $table, array('class'=>'admin-orders'));    

  return $output;
}

/**
 * Page callback for God page
 */
function melodev_god_page() {

  $output .= '<h2 class="title">Support</h2>
              <ul>
              <li>Visit <a href="http://melodramatic.zendesk.com">http://melodramatic.zendesk.com</a></li>
              <li>Process user tickets, and assign them to Sara when they need escalation.</li>
              </ul>';


  // FP
  $output .= '<h2 class="title">Front Page</h2>';
  $art_time = variable_get('melodev_cron_fpart_rotate', 0);
  if((mktime() - $art_time) > (60*60*24)) { $art_class = 'red'; } else { $art_class=''; }
  $poll_time = variable_get('melodev_cron_fppoll_rotate', 0);
  if((mktime() - $poll_time) > (60*60*24)) { $poll_class = 'red'; } else { $poll_class=''; }
  $post_time = db_result(db_query('select max(changed) from node where type = "article" and status = 1'));
  if((mktime() - $post_time) > (60*60*24)) { $post_class = 'red'; } else { $post_class=''; }
    
  $output .= '<p>The front page art & poll gets rotated automatically every 24 hours.  If there is nothing available it will try again every 15 minutes until it finds something.  You can manually rotate the art & poll here.</p>';
  $rows = array(
    array(  '<span class="mymelo-type-photo mymelo-type"></span> &nbsp; Art', 
            '<span class=" '.$art_class.'">'.format_interval(mktime() - $art_time).' ago</span>', 
            date('m/d/y g:ia', $art_time), 
            l('Rotate Now', 'admin/god/fpart/rotate').' | '.l('Pick Submissions', 'admin/god/fpart')
         ),
    array(  '<span class="mymelo-type-poll mymelo-type"></span> &nbsp; Poll', 
            '<span class=" '.$poll_class.'">'.format_interval(mktime() - $poll_time).' ago</span>', 
            date('m/d/y g:ia', $poll_time), 
            l('Rotate Now', 'admin/god/fppoll/rotate').' | '.l('Pick Submissions', 'admin/god/fppoll')
         ),
    array(  '<span class="mymelo-type-blog mymelo-type"></span> &nbsp; Post', 
            '<span class=" '.$post_class.'">'.format_interval(mktime() - $post_time).' ago</span>', 
            date('m/d/y g:ia', $post_time), 
            l('Post New', 'node/article/add')
         ),
    );
  $head = array('Type', 'Last Rotated', '', 'Action');
  $output .= theme('table', $head, $rows);

  $output .= '
  <br /><br />
  <h2 class="title">Crons</h2>';
  $output .= '<p>Various crons run throughout the day to keep our data in sync and cleaned up.  Below is a report of when the various crons last run and what state they are currently in.</p>';
  // Multiple times a day
  $output .= '<h3>Multiple Times a Day</h3>';
  
  $freq_primary_cron = 10; // minutes
  $freq_elite_logs = 5; // minutes
  $freq_url_alias = 15; // minutes
  
  if((mktime() - variable_get('cron_last', 0)) > (60*$freq_primary_cron)) { $cronago = '<span class="red">%s</span>'; } else { $cronago = '%s'; }
  $cronago = str_replace('%s', format_interval(mktime() - variable_get('cron_last', 0)).' ago', $cronago);
  if((mktime() - variable_get('melodev_cron_process_logs', 0)) > (60*$freq_elite_logs)) { $logsago = '<span class="red">%s</span>'; } else { $logsago = '%s'; }
  $logsago = str_replace('%s', format_interval(mktime() - variable_get('melodev_cron_process_logs', 0)).' ago', $logsago);
  if((mktime() - variable_get('melodev_cron_process_aliases', 0)) > (60*$freq_url_alias)) { $aliago = '<span class="red">%s</span>'; } else { $aliago = '%s'; }
  $aliago = str_replace('%s', format_interval(mktime() - variable_get('melodev_cron_process_aliases', 0)).' ago', $aliago);
  
  $logtxt = '';

  if($loglock = melodev_cron_check_lock('melodev_cron_process_logs')) {
    $logtxt = '<span title="Locked since: '.date('M d Y, g:ia', $loglock).'" class="red"><b>X</b></span>';
    $logtxt .= ' <a href="/admin/god/cron/unlock/melodev_cron_process_logs">Unlock</a>';
  }
  $alitxt = '';
  if($alilock = melodev_cron_check_lock('melodev_cron_process_aliases')) {
    $alitxt = '<span title="Locked since: '.date('M d Y, g:ia', $alilock).'" class="red"><b>X</b></span>';
    $alitxt .= '<a href="/admin/god/cron/unlock/melodev_cron_process_aliases">Unlock</a>';
  }

  $rows = array(
    array('', 'Primary Cron', $freq_primary_cron.' minutes', $cronago, date('m/d/y g:ia', variable_get('cron_last', 0)), l('Run Now', 'admin/reports/status/run-cron')),
    array($logtxt, 'Elite Logs', $freq_elite_logs.' minutes', $logsago, date('m/d/y g:ia', variable_get('melodev_cron_process_logs', 0)), l('Process Now', 'admin/reports/processlogs')),
    array($alitxt, 'URL Aliases', $freq_url_alias.' minutes', $aliago, date('m/d/y g:ia', variable_get('melodev_cron_process_aliases', 0)), ''),
  );
  $head = array('Lock', 'Cron', 'Frequency', 'Last Run', '', 'Action');
  $output .= theme('table', $head, $rows);

  $output .= '<h3>Once a Day</h3>';
  
  if((mktime() - variable_get('melodev_cron_stats_daily', 0)) > (60*60*24)) { $logsago = '<span class="red">%s</span>'; } else { $logsago = '%s'; }
  $statsago = str_replace('%s', format_interval(mktime() - variable_get('melodev_cron_stats_daily', 0)).' ago', $logsago);
  if((mktime() - variable_get('melodev_cron_touches_reset', 0)) > (60*60*24)) { $touchago = '<span class="red">%s</span>'; } else { $touchago = '%s'; }
  $touchago = str_replace('%s', format_interval(mktime() - variable_get('melodev_cron_touches_reset', 0)).' ago', $touchago);
  if((mktime() - variable_get('melodev_cron_tagclouds', 0)) > (60*60*24)) { $tagago = '<span class="red">%s</span>'; } else { $tagago = '%s'; }
  $tagago = str_replace('%s', format_interval(mktime() - variable_get('melodev_cron_tagclouds', 0)).' ago', $tagago);
  $rows = array(
    array('Nightly Stats', $statsago, date('m/d/y g:ia', variable_get('melodev_cron_stats_daily', 0)), ''),
    array('Nightly Touches', $touchago, date('m/d/y g:ia', variable_get('melodev_cron_touches_reset', 0)), l('Reset Touches', 'admin/build/touches/reset')),
    array('Nightly Tag Clouds', $tagago, date('m/d/y g:ia', variable_get('melodev_cron_tagclouds', 0)), ''),
    );
  $head = array('Cron', 'Last Run', '', 'Action');
  $output .= theme('table', $head, $rows);
  
  // Crons  
  /*
  $crontxt = drupal_get_path('module', 'melodev') . '/crontab.txt';
  $fp = fopen($crontxt, 'r');
  $crontab = fread($fp, filesize($crontxt));
  fclose($fp);
  $output .= '<h3>Crontab</h3>';
  $output .= '<pre style="font-size: 9px">'.$crontab."\n</pre>";
  
  $output .= '<br /><h3>Karma</h3>';
  $output .= 'Nightly Karma Last Calculated: <strong>'.date('m/d/y g:ia', variable_get('melodev_cron_karma', 0)).'</strong> (<em>'.format_interval(mktime() - variable_get('melodev_cron_karma', 0)).' ago</em>)<br />';
  $output .= 'Nightly Karma Offset: <strong>'.variable_get('melodev_cron_karma_offset', 0).'</strong><br />(<em>If Offset is greater than 0 cron is currently running, does about 205 iterations.</em>)<br />';
  */
  return $output;
}

/**
* @} End of "melodev_pages"
*/

/**
 * User Access Rebuild Form.
 */
  function melodev_rebuild_user_access_form($form, &$form_state, $account) {
    return confirm_form(
        array(
          'account' => array(
            '#type' => 'value',
            '#value' => $account,
          ),
        ),
        t('Are you sure you want to rebuild node access records for user %user?', array('%user' => $account->name)),
        'user/'.$account->uid,
        t('This action may take a while.'),
        t('Rebuild Access Records'),
        t('Cancel')
      );
  }
/**
 * User Access Rebuild Form Submit.
 */
  function melodev_rebuild_user_access_form_submit($form, &$form_state) {
    $account = $form_state['values']['account'];
    if(is_numeric($account->uid) && $account->uid > 0) {
      $uid = $account->uid;
      $batch = array(
        'title' => t('Rebuilding Access for User ID '.$uid),
        'operations' => array(
          array('melodev_rebuild_user_access_batch', array($uid)),
        ),
        'finished' => 'melodev_rebuild_user_access_batch_finished',
      );
      batch_set($batch);
    }
  }
/**
 * Reimport User Entries Form.
 */
  function melodev_reimport_user_entries_form($form, &$form_state, $account) {
    return confirm_form(
        array(
          'account' => array(
            '#type' => 'value',
            '#value' => $account,
          ),
        ),
        t('Are you sure you want to reimport journal entries for user %user?', array('%user' => $account->name)),
        'user/'.$account->uid,
        t('This action may take a while.'),
        t('Reimport Journal'),
        t('Cancel')
      );
  }
/**
 * Reimport User Entries Form Submit.
 */
  function melodev_reimport_user_entries_form_submit($form, &$form_state) {
    $account = $form_state['values']['account'];
    if(is_numeric($account->uid) && $account->uid > 0) {
      $uid = $account->uid;
      $batch = array(
        'title' => t('Reimporting Journals for User ID '.$uid),
        'operations' => array(
          array('melodev_reimport_user_entries_batch', array($uid)),
        ),
        'finished' => 'melodev_reimport_user_entries_batch_finished',
      );
      batch_set($batch);
    }
  }
  
/**
 * Recalculate Elite Expirations
 */
  function melodev_recalculate_expiration_form($form, &$form_state) {
    $form = array();
    return confirm_form(
        $form,
        t('Are you sure you want to recalculate all elite account expirations?'),
        '<front>',
        t('This action may take a while.'),
        t('Recalculate Elite Expirations'),
        t('Cancel')
      );
  }
/**
 * User Access Rebuild Form Submit.
 */
  function melodev_recalculate_expiration_form_submit($form, &$form_state) {
    $batch = array(
      'title' => t('Recalculating Elite Expirations'),
      'operations' => array(
        array('melodev_reset_elite_expires_batch', array($uid)),
      ),
      'finished' => 'melodev_reset_elite_expires_batch_finished',
    );
    batch_set($batch);
  }
  
/**
 * Page callback for elite reports.
 */
function melodev_elite_report_page() {
  include('melodev.experimental.pages.admin.inc');
  $output = melodev_elite_report();
  return $output;
}