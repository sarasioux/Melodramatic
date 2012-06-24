<?php
// $Id$

/**
 * @file
 * Module admin page callbacks.
 */

//////////////////////////////////////////////////////////////////////////////
// File slideshow settings

/**
 * Implements the settings page.
 *
 * @return
 *   The form structure.
 */
function file_slideshow_admin_settings() {
  $form = array();

  // PDF settings
  $form['pdf'] = array('#type' => 'fieldset', '#title' => t('PDF settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['pdf']['file_slideshow_pdf'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Enable Extraction of the PDF metadata'),
    '#default_value' => FILE_SLIDESHOW_PDF,
    '#description'   => t('Check this box if you want to extract any of the following metadata.'),
  );
  $pdf = _file_slideshow_pdf();
  foreach ($pdf as $name => $data) {
    $pdf_data_default[$name] = $data['default'];
  }
  $pdf_data = variable_get('file_slideshow_pdf_data', $pdf_data_default);
  $i = $j = 0;
  foreach ($pdf as $name => $data) {
    $form['pdf']['pdf_'. $i][$j]['file_slideshow_pdf_'. $name] = array(
      '#type'          => 'checkbox',
      '#title'         => $data['name'],
      '#default_value' => $pdf_data[$name],
    );
    $j++;
    if ($j == 3) {
      $i++;
      $j = 0;
    }
  }

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
function file_slideshow_admin_settings_validate($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  switch ($op) {
    case t('Save configuration'):
      break;
  }
}

/**
 * Submit hook for the settings form.
 */
function file_slideshow_admin_settings_submit($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      variable_set('file_slideshow_pdf', $values['file_slideshow_pdf']);
      $pdf = _file_slideshow_pdf();
      $pdf_data = array();
      foreach ($pdf as $name => $data) {
        $pdf_data[$name] = $values['file_slideshow_pdf_'. $name];
      }
      variable_set('file_slideshow_pdf_data', $pdf_data);
      drupal_set_message(t('The configuration options have been saved.'));
      break;
    case t('Reset to defaults'):
      variable_del('file_slideshow_pdf');
      variable_del('file_slideshow_pdf_data');
      drupal_set_message(t('The configuration options have been reset to their default values.'));
      break;
  }
}

