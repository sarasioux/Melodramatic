<?php

/**
 * @file
 * This file contains the Conditional Actions hooks and functions necessary to make the
 * gift-related entity, conditions, events, and actions work.
 */

/******************************************************************************
 * Conditional Actions Hooks                                                  *
 ******************************************************************************/

/**
 * Implementation of hook_ca_entity().
 *
 * An entity is defined in order to get a role expiration down
 * to token in the email.
 */
function _uc_roles_ca_entity() {
  // CA entity for a role expiration object.
  $entities['uc_roles_expiration'] = array(
    '#title' => t('Ubercart role expiration'),
    '#type' => 'object',
  );

  return $entities;
}

/**
 * Implementation of hook_ca_predicate().
 */
function melodev_ca_predicate() {
  $predicates = array();

  // Renew all the roles on an order when the status matches what's set in the roles admin settings.
  $predicates['uc_gift'] = array(
    '#title' => t('Grant purchased gifts'),
    '#description' => t('Grant purchased gifts if the order status matches.'),
    '#trigger' => 'uc_order_status_update',
    '#status' => 1,
    '#class' => 'gift',
    '#conditions' => array(
      '#operator' => 'AND',
      '#conditions' => array(
        array(
          '#name' => 'uc_order_status_condition',
          '#title' => t('If the original order status was not Completed.'),
          '#argument_map' => array(
            'order' => 'order',
          ),
          '#settings' => array(
            'negate' => TRUE,
            'order_status' => 'completed',
          ),
        ),
        array(
          '#name' => 'uc_order_status_condition',
          '#title' => t('If the updated order status is Completed.'),
          '#argument_map' => array(
            'order' => 'updated_order',
          ),
          '#settings' => array(
            'order_status' => 'completed',
          ),
        ),
      ),
    ),
    '#actions' => array(
      array(
        '#name' => 'uc_gift_grant',
        '#title' => t('Grant all gifts for this order.'),
        '#argument_map' => array(
          'order' => 'order',
        ),
        '#settings' => array(
          'message' => FALSE,
        ),
      ),
    ),
  );

  $order_args = array(
    'order' => 'order',
    'expiration' => 'expiration',
  );

  $user_args = array(
    'account' => 'account',
    'expiration' => 'expiration',
  );


  return $predicates;
}

/**
 * Implementation of hook_ca_action().
 */
function melodev_ca_action() {
  // Grant a gift.
  $actions['uc_gift_grant'] = array(
    '#title' => t('Grant the gifts on an order.'),
    '#category' => t('gift'),
    '#callback' => 'melodev_gift_action',
    '#arguments' => array(
      'order' => array(
        '#entity' => 'uc_order',
        '#title' => t('Order'),
      ),
    ),
  );

  return $actions;
}

/**
 * Implementation of hook_ca_trigger().
 */
function _uc_roles_ca_trigger() {
  $order = array(
    '#entity' => 'uc_order',
    '#title' => t('Order'),
  );
  $account = array(
    '#entity' => 'user',
    '#title' => t('User'),
  );
  $expiration = array(
    '#entity' => 'uc_roles_expiration',
    '#title' => t('Role expiration'),
  );

  $triggers['uc_roles_notify_grant'] = array(
    '#title' => t('E-mail for granted roles'),
    '#category' => t('Notification'),
    '#arguments' => array(
      'order' => $order,
      'expiration' => $expiration,
    ),
  );

  $triggers['uc_roles_notify_revoke'] = array(
    '#title' => t('E-mail for revoked roles'),
    '#category' => t('Notification'),
    '#arguments' => array(
      'account' => $account,
      'expiration' => $expiration,
    ),
  );

  $triggers['uc_roles_notify_renew'] = array(
    '#title' => t('E-mail for renewed roles'),
    '#category' => t('Notification'),
    '#arguments' => array(
      'order' => $order,
      'expiration' => $expiration,
    ),
  );

  $triggers['uc_roles_notify_reminder'] = array(
    '#title' => t('E-mail for role expiration reminders'),
    '#category' => t('Notification'),
    '#arguments' => array(
      'account' => $account,
      'expiration' => $expiration,
    ),
  );

  return $triggers;
}

/**
 * Grants another product to another user on purchase.
 */
function melodev_gift_action($order, $settings) {
  // Load the order's user and exit if not available.
  if (!($account = user_load($order->uid))) {
    return;
  }

  // Loop through all the products on the order.
  foreach ($order->products as $product) {
    // Look for any gift features assigned to the product.
    $gifts = db_query("SELECT * FROM {uc_gift_products} WHERE nid = %d", $product->nid);
    while ($gift = db_fetch_object($gifts)) {
      // Get the gifted username
      $name = $product->data['attributes']['Username'][0];
      if($acct = user_load(array('name'=>$name))) {
        // Grant the selected product to this user
        melodev_gift_create_order($acct, $gift->pid, $order->order_id);
        // Note the granting on existing order
      	uc_order_comment_save($order->order_id, 0, t('Your gift was given to '.$name.'.'));
      }
    }
  }
}

/**
 * Grant a product to a user.
 */
function melodev_gift_create_order($account, $productnid, $order_id=null) {
	// Create a completed order for the user
	$order = uc_order_new($account->uid, 'in_checkout');
	$product = node_load($productnid);
	$order_id = $order->order_id;
	
	// Add our product to this order
	uc_order_product_save($order->order_id, $product);

	// Reload the order
	$order = uc_order_load($order_id);

	// Save order comments
	uc_order_comment_save($order->order_id, 0, t('Gift product created for order '.l($order_id, 'admin/store/orders/'.$order_id).'.'));
	watchdog('melodev.uc', 'Gift order created for user '.$account->uid.'.');
	
	// Set status completed
	if(!uc_order_update_status($order_id, 'completed')) {
		watchdog('melodev.uc', 'Unable to update order status for gift '.$order_id.'.');
	}
	return $order_id;
}