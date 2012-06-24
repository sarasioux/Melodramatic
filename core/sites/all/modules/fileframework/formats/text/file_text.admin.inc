<?php
// $Id$

/**
 * @file
 * Module admin page callbacks.
 */

//////////////////////////////////////////////////////////////////////////////
// File text settings

/**
 * Implements the settings page.
 *
 * @return
 *   The form structure.
 */
function file_text_admin_settings() {
  $form = array();

  // Preview settings
  $form['preview'] = array('#type' => 'fieldset', '#title' => t('Preview settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['preview']['file_text_preview_lines'] = array(
    '#type'          => 'textfield',
    '#title'         => t('How many text lines should be shown in a text previews'),
    '#default_value' => FILE_TEXT_PREVIEW_LINES,
    '#size'          => 10,
    '#maxlength'     => 255,
    '#description'   => t('Specify the maximum number of text lines to be shown when displaying preview of the text files. Set the value to zero to show all lines.'),
  );
  $form['preview']['file_text_preview_string'] = array(
    '#type'          => 'textfield',
    '#title'         => t('The preview end message.'),
    '#default_value' => FILE_TEXT_PREVIEW_STRING,
    '#size'          => 50,
    '#maxlength'     => 255,
    '#description'   => t('The string that will be displayed at the end of the text preview to indicate that the preview is not a complete text file.'),
  );

  // Code preview settings
  $form['code'] = array('#type' => 'fieldset', '#title' => t('Code preview settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['code']['file_text_code'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Enable <a href="http://www.palfrader.org/code2html" title="Code2HTML">Code2HTML</a> syntax highlighted HTML preview'),
    '#default_value' => FILE_TEXT_CODE,
    '#description'   => t('Check this box if you want a syntax highlighted HTML previews to be created for the "text/plain" MIME files. The "code2html" utility should be in the system\'s path.'),
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
function file_text_admin_settings_validate($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      $preview_lines = $values['file_text_preview_lines'];
      if (!ctype_digit($preview_lines)) {
        form_set_error('file_text_preview_lines', t('Invalid value %value specified for the number of lines.', array('%value' => $preview_lines)));
      }
      $code = $values['file_text_code'];
      $code2html = exec('which code2html 2>/dev/null');
      if (!empty($code) && empty($code2html)) {
        form_set_error('file_text_code', t('Code2HTML cannot be enabled because code2html utility is not found in the system path.'));
      }
      break;
  }
}

/**
 * Submit hook for the settings form.
 */
function file_text_admin_settings_submit($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      variable_set('file_text_preview_lines', $values['file_text_preview_lines']);
      variable_set('file_text_preview_string', $values['file_text_preview_string']);
      variable_set('file_text_code', $values['file_text_code']);
      drupal_set_message(t('The configuration options have been saved.'));
      break;
    case t('Reset to defaults'):
      variable_del('file_text_preview_lines');
      variable_del('file_text_preview_string');
      variable_del('file_text_code');
      drupal_set_message(t('The configuration options have been reset to their default values.'));
      break;
  }
}

