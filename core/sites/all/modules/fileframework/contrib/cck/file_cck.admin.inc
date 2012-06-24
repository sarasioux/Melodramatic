<?php
// $Id$

/**
 * @file
 * Module admin page callbacks.
 */

//////////////////////////////////////////////////////////////////////////////
// File cck settings

/**
 * Implements the settings page.
 *
 * @return
 *   The form structure.
 */
function file_cck_admin_settings() {
  $form = array();

  // Vocabulary settings.
  if (module_exists('taxonomy')) {
    $options = array();
    foreach (taxonomy_get_vocabularies('file') as $vid => $vocab) {
      $options[$vid] = $vocab->name;
    }
    $form['vocabs'] = array('#type' => 'fieldset', '#title' => t('Vocabulary settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
    $form['vocabs']['file_cck_vocabularies_all'] = array(
      '#type'          => 'radios',
      '#title'         => t('Vocabularies to inherit terms from'),
      '#options'       => array(TRUE => t('All'), FALSE => t('Only bellow selected vocabularies')),
      '#description'   => t('Select the vocabularies from which field file will inherit the taxonomy terms of the parent node on the creation of the field.'),
      '#default_value' => FILE_CCK_VOCABULARIES_ALL,
    );
    $form['vocabs']['file_cck_vocabularies'] = array(
      '#type'          => 'select',
      '#title'         => t('Choose vocabularies to inherit terms from'),
      '#options'       => $options,
      '#multiple'      => TRUE,
      '#description'   => t('If "Only bellow selected vocabularies" are selected, then choose the vocabularies.'),
      '#default_value' => unserialize(FILE_CCK_VOCABULARIES),
    );
  }

  // Organic groups integration.
  if (module_exists('og')) {
    $form['og'] = array('#type' => 'fieldset', '#title' => t('Organic groups integration'), '#collapsible' => TRUE, '#collapsed' => FALSE);
    $form['og']['file_cck_og_inheritance'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('OG membership inheritance'),
      '#description'   => t('Should cck field files be put in the same groups as the parent node.'),
      '#default_value' => FILE_CCK_OG_INHERITANCE,
    );
    if (module_exists('og_vocab')) {
      $form['og']['file_cck_og_vocabularies'] = array(
        '#type'          => 'checkbox',
        '#title'         => t('Inherit from group vocabularies'),
        '#default_value' => FILE_CCK_OG_VOCABULARIES,
        '#description'   => t('Taxonomy terms will be inherited from vocabularies defined in the group\'s scope by og_vocab module automatically even if they are not explicitly selected in "Choose vocabularies to inherit terms from".'),
      );
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
function file_cck_admin_settings_validate($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  switch ($op) {
    case t('Save configuration'):
      break;
  }
}

/**
 * Submit hook for the settings form.
 */
function file_cck_admin_settings_submit($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      variable_set('file_cck_vocabularies_all', $values['file_cck_vocabularies_all']);
      variable_set('file_cck_vocabularies', $values['file_cck_vocabularies']);
      if (module_exists('og')) {
        variable_set('file_cck_og_inheritance', $values['file_cck_og_inheritance']);
        if (module_exists('og_vocab'))
          variable_set('file_cck_og_vocabularies', $values['file_cck_og_vocabularies']);
      }
      drupal_set_message(t('The configuration options have been saved.'));
      break;
    case t('Reset to defaults'):
      variable_del('file_cck_vocabularies_all');
      variable_del('file_cck_vocabularies');
      variable_del('file_cck_og_inheritance');
      variable_del('file_cck_og_vocabularies');
      drupal_set_message(t('The configuration options have been reset to their default values.'));
      break;
  }
}

