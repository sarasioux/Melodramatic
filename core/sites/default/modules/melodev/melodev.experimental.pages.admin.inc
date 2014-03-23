<?php

/**
 * @file
 * Experimental code file.
 *
 */

/**
 * Page callback for elite reports.
 */
function melodev_elite_report() {
  $output = 'An overall report of what\'s going on in the world of elites on Melo.';
  
  
  $output .= '<h2>Scenario 1: New Elite Monthlies</h2>';
  $output .= '<p>These are users who have purchased a new recurring monthly account since Melo relaunched, at the cost of $2.99 a month.';
  
  // Load any monthly signups we've had lately
  $rows = array();
  $subs = db_query('select * from {uc_order_products} p, {uc_orders} o where o.order_id = p.order_id and o.order_status = "%s" and p.nid = %d order by o.order_id desc limit 5', 'completed',  MELO_ELITE_SUBSCRIPTION_NID);
  while($sub = db_fetch_array($subs)) {
    $order = uc_order_load($sub['order_id']);
    $row = array(
      'date' => format_date($order->modified),
      'name' => $order->delivery_first_name.' '.$order->delivery_last_name,
      'price' => '$'.$order->order_total,
      'id' => l($order->order_id, 'admin/store/orders/'.$order->order_id)
      );
    $rows[] = $row;
  }
  $output .= theme('table', array('Date', 'Name', 'Price', 'Order ID'), $rows);
  
  
  
  $output .= '<h2>Scenario 2: New Elite Yearlies</h2>';
  $output .= '<p>These are users who have purchased a new year of elite since the relaunch at the cost of $29.99.';
  
  // Load any yearly signups we've had lately
  $rows = array();
  $subs = db_query('select * from {uc_order_products} p, {uc_orders} o where o.order_id = p.order_id and o.order_status = "%s" and o.order_total > 0 and p.nid = %d order by o.order_id desc limit 5', 'completed',  MELO_ELITE_YEARLY_NID);
  while($sub = db_fetch_array($subs)) {
    $order = uc_order_load($sub['order_id']);
    $row = array(
      'date' => format_date($order->modified),
      'name' => $order->delivery_first_name.' '.$order->delivery_last_name,
      'price' => '$'.$order->order_total,
      'id' => l($order->order_id, 'admin/store/orders/'.$order->order_id)
      );
    $rows[] = $row;
  }
  $output .= theme('table', array('Date', 'Name', 'Price', 'Order ID'), $rows);  
  
  
  $output .= '<h2>Scenario 3: Recent New Elite Recurrings</h2>';
  $output .= '<p>These are users who have been rebilled for an elite account they purchased after relaunch.';

  // Load any monthly signups we've had lately
  $rows = array();
  $subs = db_query('select * from uc_payment_receipts p, uc_orders o where o.order_total < 3 and o.order_total > 0 and p.order_id = o.order_id and (p.received - o.created) > (60*60*24*10) order by p.received desc limit 5');
  while($sub = db_fetch_array($subs)) {
    $order = uc_order_load($sub['order_id']);
    $row = array(
      'date' => format_date($order->modified),
      'name' => $order->delivery_first_name.' '.$order->delivery_last_name,
      'price' => '$'.$order->order_total,
      'id' => l($order->order_id, 'admin/store/orders/'.$order->order_id)
      );
    $rows[] = $row;
  }
  $output .= theme('table', array('Date', 'Name', 'Price', 'Order ID'), $rows);
  
  
  
  $output .= '<h2>Scenario 4: Legacy Elite Recurrings</h2>';
  $output .= '<p>These are users who have been rebilled for an elite account they purchased after relaunch.';
  
  // Load legacy subscription payments
  $rows = array();
  $subs = db_query('select * from uc_payment_receipts p, uc_orders o where o.order_total = 3 and p.order_id = o.order_id and (p.received - o.created) > (60*60*24*10) order by p.received desc limit 5');
  while($sub = db_fetch_array($subs)) {
    $order = uc_order_load($sub['order_id']);
    $row = array(
      'date' => format_date($sub['received']),
      'name' => $order->delivery_first_name.' '.$order->delivery_last_name,
      'price' => '$'.$order->order_total,
      'id' => l($order->order_id, 'admin/store/orders/'.$order->order_id)
      );
    $rows[] = $row;
  }
  $output .= theme('table', array('Date', 'Name', 'Price', 'Order ID'), $rows);   
  
  
  $output .= '<h2>Scenario 5: New Yearly Gift</h2>';
  $output .= '<p>These are users who have purchased an elite yearly gift.';
  
  // Load any yearly gifts we've had lately
  $rows = array();
  $subs = db_query('select * from {uc_order_products} p, {uc_orders} o where o.order_id = p.order_id and o.order_status = "%s" and p.nid = %d order by o.order_id desc limit 5', 'completed',  MELO_ELITE_GIFT_NID);
  while($sub = db_fetch_array($subs)) {
    $order = uc_order_load($sub['order_id']);
    $row = array(
      'date' => format_date($order->modified),
      'name' => $order->delivery_first_name.' '.$order->delivery_last_name,
      'price' => '$'.$order->order_total,
      'id' => l($order->order_id, 'admin/store/orders/'.$order->order_id)
      );
    $rows[] = $row;
  }
  $output .= theme('table', array('Date', 'Name', 'Price', 'Order ID'), $rows);  
  
  
  
  return $output;
}

/**
 * Page callback for elite reports.
 */
function melodev_watching() {
  
  $output = "<script type='text/javascript'>\n";

// User lifetime query
//select date(from_unixtime(created)) as day, round(avg(access - created)/(60*60*24)) as avg_lifetime from users where access > 0 group by day order by day desc limit 30;

  // New/Active Users
  $users = array();
  $subs = db_query('select x.day, x.new_users, y.active_users from (SELECT date(from_unixtime(created)) as day, count(*) as new_users FROM `users` where access > 0 group by day order by day desc limit 30) x, (SELECT date(from_unixtime(access)) as day, count(*) as active_users FROM `users` group by day order by day desc limit 30) y where x.day = y.day');
  while($sub = db_fetch_array($subs)) {
    $users[] = $sub;
  }
  $output .= "var users = ".json_encode($users)."\n";
  
  // Products sold
  $orders = array();
  $subs = db_query('select * from (select date(from_unixtime(created)) as date, title, sum(qty) as quantity from uc_order_products p, uc_orders o where o.order_id = p.order_id and o.order_status = "completed" group by date, nid order by date desc limit 30) a order by date asc');
  while($sub = db_fetch_array($subs)) {
    $orders[] = $sub;
  }
  $output .= "var product_quantity = ".json_encode($orders)."\n";
  
  
  $output .= "\n</script>\n";
  
  return $output;
}