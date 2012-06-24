<?php
// $Id: google_website_optimizer.admin.inc,v 1.2.2.4 2009/06/23 12:14:27 neochief Exp $

/**
 * @file
 * Easily insert the nessesary javascript code for your Google Website Optimizer tests.
 */

/**
 * Controller for google_website_optimizer tests.
 */
function google_website_optimizer_admin_overview() {
  $output = '';

  $tests = google_website_optimizer_test_load();
  if ($tests) {
    $header = array(t('Test'), t('Operations'));
    foreach ($tests as $test) {
      $rows[] = array(array('data' => check_plain($test->name), 'valign' => 'top'), array('data' => l(t('edit'), 'admin/settings/google_website_optimizer/edit/'. urlencode($test->test_id)) .' '. l(t('delete'), 'admin/settings/google_website_optimizer/delete/'. urlencode($test->test_id)), 'valign' => 'top'));
    }
    $output .= theme('table', $header, $rows);
    $output .= '<p>'. l(t('Add new test'), 'admin/settings/google_website_optimizer/add') .'</p>';
  }
  else {
    drupal_set_message(t('No tests found. Click here to <a href="!create-test-url">create a new test</a>.', array('!create-test-url' => url('admin/settings/google_website_optimizer/add'))));
  }

  return $output;
}

/**
 * Return an HTML form for test configuration.
 */
function google_website_optimizer_admin_form($form_state, $test_id = NULL) {
  if (isset($test_id)) {
    if (isset($form_state['values'])) {
      $edit = $form_state['values'];
    }
    else {
      $edit = (array)google_website_optimizer_test_load($test_id);
    }

    $form['test_id'] = array(
      '#type' => 'hidden',
      '#value' => $edit['test_id'],
    );
    $btn = t('Update test');
  }
  else {
    $btn = t('Create test');
  }

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Test name'),
    '#default_value' => $edit['name'],
    '#size' => 40,
    '#maxlength' => 128,
    '#description' => t('Enter a name for this test. This should correspond to the name of your test in Google Website Optimizer to make it easy to identify.'),
    '#required' => TRUE,
  );

  $form['testing_page'] = array(
    '#type' => 'fieldset',
    '#title' => t('Testing Page'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $form['testing_page']['test_page_path'] = array(
    '#type' => 'textarea',
    '#title' => t('Testing Page'),
    '#default_value' => $edit['test_page_path'],
    '#description' => t("Enter one page per line as Drupal paths (e.g. <em>node/1</em> or <em>blog/some-article</em>). The '*' character is a wildcard."),
    '#required' => TRUE,
  );

  $form['testing_page']['control_script'] = array(
    '#type' => 'textarea',
    '#title' => t('Control Script'),
    '#default_value' => $edit['control_script'],
    '#description' => t("Enter the control script provided by Google Website Optimizer."),
    '#required' => TRUE,
  );

  $form['testing_page']['tracking_script'] = array(
    '#type' => 'textarea',
    '#title' => t('Tracking Script'),
    '#default_value' => $edit['tracking_script'],
    '#description' => t("Enter the tracking script provided by Google Website Optimizer."),
    '#required' => TRUE,
  );

  $form['variation_pages'] = array(
    '#type' => 'fieldset',
    '#title' => t('Variation pages (for A/B tests only)'),
    '#collapsible' => TRUE,
    '#collapsed' => $edit['variation_page_path'] == '',
  );

  $form['variation_pages']['variation_page_path'] = array(
    '#type' => 'textarea',
    '#title' => t('Variation Pages'),
    '#default_value' => $edit['variation_page_path'],
    '#description' => t("Enter one page per line as Drupal paths. The '*' character is a wildcard."),
  );

  $form['variation_pages']['variation_tracking_script'] = array(
    '#type' => 'textarea',
    '#title' => t('Variation Tracking Script'),
    '#default_value' => $edit['variation_tracking_script'],
    '#description' => t("Enter the variation tracking script provided by Google Website Optimizer."),
  );

  $form['conversion_page'] = array(
    '#type' => 'fieldset',
    '#title' => t('Conversion Page'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $form['conversion_page']['conversion_page_path'] = array(
    '#type' => 'textarea',
    '#title' => t('Conversion Page(s)'),
    '#default_value' => $edit['conversion_page_path'],
    '#description' => t("Enter one page per line as Drupal paths. The '*' character is a wildcard."),
    '#required' => TRUE,
  );

  $form['conversion_page']['conversion_script'] = array(
    '#type' => 'textarea',
    '#title' => t('Conversion Script'),
    '#default_value' => $edit['conversion_script'],
    '#description' => t("Enter the conversion script provided by Google Website Optimizer."),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $btn
  );

  return $form;
}

/**
 * Save a test to the database.
 */
function google_website_optimizer_admin_form_submit($form, &$form_state) {
  if ($form_state['values']['test_id']) { //It's an existing test
    drupal_write_record('google_website_optimizer', $form_state['values'], 'test_id');
    drupal_set_message(t('Test updated.'));
  } 
  else { //It's a new test
    drupal_write_record('google_website_optimizer', $form_state['values']);
    drupal_set_message(t('Test created.'));
  }
  $form_state['redirect'] = 'admin/settings/google_website_optimizer';
}

/**
 * Test removal confirmation.
 */
function google_website_optimizer_delete_confirm($form_state, $test_id) {
  $test = google_website_optimizer_test_load($test_id);

  $form['test_id'] = array(
    '#type' => 'value',
    '#value' => $test_id,
  );

  return confirm_form($form, t('Are you sure you want to delete test @name?', array('@name' => $test->name)), 'admin/settings/google_website_optimizer', NULL, t('Delete test'));
}

/**
 * Submit detelete confirm — remove a test from the database.
 */
function google_website_optimizer_delete_confirm_submit($form, &$form_state) {
  db_query("DELETE FROM {google_website_optimizer} WHERE test_id = %d", $form_state['values']['test_id']);
  drupal_set_message(t('Test removed.'));
  $form_state['redirect'] = 'admin/settings/google_website_optimizer';
}
