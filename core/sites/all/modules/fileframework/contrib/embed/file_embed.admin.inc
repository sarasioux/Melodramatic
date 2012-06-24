<?php
// $Id$

/**
 * @file
 * Module admin page callbacks.
 */

//////////////////////////////////////////////////////////////////////////////
// File embed settings

/**
 * Implements the settings page.
 *
 * @return
 *   The form structure.
 */
function file_embed_admin_settings() {
  $form = array();

  // Button settings
  $options = array(t('Show on textareas on every page except the listed pages.'), t('Show on textareas only on the listed pages.'));
  $form['button'] = array('#type' => 'fieldset', '#title' => t('Button settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['button']['file_embed_button_option'] = array(
    '#type'          => 'radios',
    '#title'         => t('Display file embed button on'),
    '#options'       => $options,
    '#default_value' => FILE_EMBED_BUTTON_OPTION,
  );
  $form['button']['file_embed_button_pages'] = array(
    '#type'          => 'textarea',
    '#title'         => t('Pages'),
    '#default_value' => FILE_EMBED_BUTTON_PAGES,
    '#description'   => t("Enter one page per line as Drupal paths. The '*' character is a wildcard. Example paths are '%blog' for the blog page and %blog-wildcard for every personal blog. %front is the front page.", array('%blog' => 'blog', '%blog-wildcard' => 'blog/*', '%front' => '<front>')),
    '#rows'          => 3,
  );
  $form['button']['file_embed_button_fields'] = array(
    '#type'          => 'textarea',
    '#title'         => t('Fields'),
    '#default_value' => FILE_EMBED_BUTTON_FIELDS,
    '#description'   => t('Enter one field name per line for those textarea fields you wish the file embed button to be displayed for.'),
    '#rows'          => 3,
  );
  $form['button']['file_embed_button_type'] = array(
    '#type'          => 'select',
    '#title'         => t('Display file embed button as'),
    '#default_value' => FILE_EMBED_BUTTON_TYPE,
    '#options'       => array('link' => t('Text link'), 'image' => t('Image button')),
    '#description'   => t('Select whether to display an "add existing image" text link under textareas, or whether to show a graphical image button instead.'),
  );

  // Popup settings
  $form['popup'] = array('#type' => 'fieldset', '#title' => t('Popup settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['popup']['file_embed_popup_size'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Size of file gallery selector popup window'),
    '#default_value' => FILE_EMBED_POPUP_SIZE,
    '#size'          => 10,
    '#maxlength'     => 255,
    '#description'   => t('The default size is 590x500.'),
  );
  $form['popup']['file_embed_popup_per_page'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Files per page'),
    '#default_value' => FILE_EMBED_POPUP_PER_PAGE,
    '#size'          => 3,
    '#maxlength'     => 255,
    '#description'   => t('Sets the number of files to be displayed on a gallery page.'),
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
function file_embed_admin_settings_validate($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      $size = $values['file_embed_popup_size'];
      if (empty($size) || !preg_match('/^[0-9]+x[0-9]+$/', $size)) {
        form_set_error('file_embed_popup_size', t('Invalid value %value specified for popup size.', array('%value' => $size)));
      }
      break;
  }
}

/**
 * Submit hook for the settings form.
 */
function file_embed_admin_settings_submit($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      variable_set('file_embed_button_option', $values['file_embed_button_option']);
      variable_set('file_embed_button_pages', $values['file_embed_button_pages']);
      variable_set('file_embed_button_fields', $values['file_embed_button_fields']);
      variable_set('file_embed_button_type', $values['file_embed_button_type']);
      variable_set('file_embed_popup_size', $values['file_embed_popup_size']);
      variable_set('file_embed_popup_per_page', $values['file_embed_popup_per_page']);
      drupal_set_message(t('The configuration options have been saved.'));
      break;
    case t('Reset to defaults'):
      variable_del('file_embed_button_option');
      variable_del('file_embed_button_pages');
      variable_del('file_embed_button_fields');
      variable_del('file_embed_button_type');
      variable_del('file_embed_popup_size');
      variable_del('file_embed_popup_per_page');
      drupal_set_message(t('The configuration options have been reset to their default values.'));
      break;
  }
}

