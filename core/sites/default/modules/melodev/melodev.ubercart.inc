<?php

/**
 * @file
 * Controls custom functionality for Ubercart.
 *
 * Adds a new product feature that enables the elite account gifting.
 */
 
 require_once 'melodev.ubercart.ca.inc';

/**
 * Implementation of hook_product_feature().
 */
function melodev_product_feature() {
  $features[] = array(
    'id' => 'gift',
    'title' => t('Product Gift'),
    'callback' => 'melodev_gift_feature_form',
    'delete' => 'melodev_gift_feature_delete',
    'settings' => 'melodev_gift_feature_settings',
  );

  return $features;
}

function melodev_gift_feature_form($form_state, $node, $feature) {
  // Choose products from the store
  $result = db_query('select nid, title from node where type = "%s" and nid != %d', 'product', $node->nid);
  $options = array();
  while($obj = db_fetch_object($result)) {
    $options[$obj->nid] = $obj->title;
  }

  // Check if editing or adding to set default values.
  if (!empty($feature)) {
    $product_gift = db_fetch_object(db_query("SELECT * FROM {uc_gift_products} WHERE pfid = %d", $feature['pfid']));
    $default = $product_gift->pid;
  } else {
      $form['pfid'] = array(
        '#type' => 'value',
        '#value' => $feature['pfid'],
      );
    $default = variable_get('uc_gift_default_product', NULL);
  }
    
  $form['pid'] = array(
    '#type' => 'select',
    '#title' => 'Default Product to Gift',
    '#default_value' => $default,
    '#options' => $options,
  );
    
  return uc_product_feature_form($form);
}

/**
 * @see melodev_gift_feature_form()
 */
function melodev_gift_feature_form_submit($form, &$form_state) {
  $pfid = $form_state['values']['pfid'];
  $nid = $form_state['values']['nid'];
  $pid = $form_state['values']['pid'];
  
  $node = node_load($pid);
  
  $data = array(
    'pfid' => $pfid,
    'nid' => $nid,
    'fid' => 'gift',
    'description' => 'Gift product <strong>'.$node->title.'</strong> on purchase.'
    );
  $form_state['redirect'] = uc_product_feature_save($data);
  
  // Save it to our gift table
  if(is_numeric($pfid)) {
    db_query('update {uc_gift_products} set pid = %d where pfid = %d', $pid, $pfid);
  } else {
    $pfid = db_last_insert_id('uc_product_features', 'pfid');
    db_query('insert into {uc_gift_products} (pfid, nid, pid) values(%d, %d, %d)', $pfid, $nid, $pid);
  }
}

/**
 * Form builder for gift settings.
 *
 * @ingroup forms
 */
function melodev_gift_feature_settings() {

  // Choose products from the store
  $result = db_query('select nid, title from node where type = "%s"', 'product');
  $options = array();
  while($obj = db_fetch_object($result)) {
    $options[$obj->nid] = $obj->title;
  }
  $form['uc_gift_default_product'] = array(
    '#type' => 'select',
    '#title' => 'Default Product to Gift',
    '#default_value' => variable_get('uc_gift_default_product', NULL),
    '#options' => $options,
  );
  
  return $form;
}

/**
 * Delete all data associated with a given product feature.
 *
 * @param $pfid
 *   An Ubercart product feature ID.
 */
function melodev_gift_feature_delete($pfid) {
  db_query("DELETE FROM {uc_gift_products} WHERE pfid = %d", $pfid);
}