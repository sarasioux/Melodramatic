<?php
// $Id$

/**
 * @file
 * Module admin page callbacks.
 */

//////////////////////////////////////////////////////////////////////////////
// File gallery settings

/**
 * Implements the settings page.
 *
 * @return
 *   The form structure.
 */
function file_gallery_admin_settings() {
  $form = array();

  // Display settings
  $form['display'] = array('#type' => 'fieldset', '#title' => t('Display settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['display']['file_gallery_per_page'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Files per page'),
    '#default_value' => FILE_GALLERY_PER_PAGE,
    '#size'          => 3,
    '#maxlength'     => 255,
    '#description'   => t('Sets the number of files to be displayed on a gallery page.'),
  );
  $form['display']['file_gallery_embed_size'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Size of embedded previews'),
    '#default_value' => FILE_GALLERY_EMBED_SIZE,
    '#size'          => 10,
    '#maxlength'     => 255,
    '#description'   => t('The default size is 130x150.'),
  );
  $form['display']['file_gallery_popup_size'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Max size of thickbox popup previews'),
    '#default_value' => FILE_GALLERY_POPUP_SIZE,
    '#size'          => 10,
    '#maxlength'     => 255,
    '#description'   => t('The default size is 800x600.'),
  );
  $form['display']['file_gallery_hide_empty'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Hide empty folders'),
    '#default_value' => FILE_GALLERY_HIDE_EMPTY,
    '#description'   => t('Whether taxonomy terms that have no sub-terms or file nodes associated with them should be hidden. This can be useful to reduce load time in situations where the vocabulary is very large (e.g. for free-tagging vocabularies).'),
  );
  $form['display']['file_gallery_navigation'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Show navigation'),
    '#default_value' => FILE_GALLERY_NAVIGATION,
    '#description'   => t('The navigation inside the gallery is displayed as a site breadcrumb. However when the theme does not display a breadcrumb, this option allows displaying the navigation on the top of the gallery page.'),
  );

  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Save configuration'),
  );
  $form['reset'] = array(
    '#type'  => 'submit',
    '#value' => t('Reset to defaults'),
  );

  return $form;
}

/**
 * Validate hook for the settings form.
 */
function file_gallery_admin_settings_validate($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      $size = $values['file_gallery_embed_size'];
      if (empty($size) || !preg_match('/^[0-9]+x[0-9]+$/', $size)) {
        form_set_error('file_gallery_embed_size', t('Invalid value %value specified for embedded preview size.', array('%value' => $size)));
      }
      $size = $values['file_gallery_popup_size'];
      if (empty($size) || !preg_match('/^[0-9]+x[0-9]+$/', $size)) {
        form_set_error('file_gallery_popup_size', t('Invalid value %value specified for popup preview size.', array('%value' => $size)));
      }
      break;
  }
}

/**
 * Submit hook for the settings form.
 */
function file_gallery_admin_settings_submit($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      variable_set('file_gallery_per_page', $values['file_gallery_per_page']);
      variable_set('file_gallery_embed_size', $values['file_gallery_embed_size']);
      variable_set('file_gallery_popup_size', $values['file_gallery_popup_size']);
      variable_set('file_gallery_hide_empty', $values['file_gallery_hide_empty']);
      variable_set('file_gallery_navigation', $values['file_gallery_navigation']);
      drupal_set_message(t('The configuration options have been saved.'));
      break;
    case t('Reset to defaults'):
      variable_del('file_gallery_per_page');
      variable_del('file_gallery_embed_size');
      variable_del('file_gallery_popup_size');
      variable_del('file_gallery_hide_empty');
      variable_del('file_gallery_navigation');
      drupal_set_message(t('The configuration options have been reset to their default values.'));
      break;
  }
}

