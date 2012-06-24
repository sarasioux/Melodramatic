<?php
// $Id$

/**
 * @file
 * Module admin page callbacks.
 */

//////////////////////////////////////////////////////////////////////////////
// File attach settings

/**
 * Implements the settings page.
 *
 * @return
 *   The form structure.
 */
function file_attach_admin_settings() {
  $form = array();

  // Attachments settings.
  $form['attachments'] = array('#type' => 'fieldset', '#title' => t('Attachments settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['attachments']['file_attach_reuse_nodes'] = array(
    '#type'          => 'radios',
    '#title'         => t('Attaching already uploaded files'),
    '#options'       => array(FALSE => t('Create a copy of the file node'), TRUE => t('Use existing file node')),
    '#description'   => t('When already uploaded file is selected to be attached to the node, a new file node might be created pointing to the same file or existing file node can be reused and attached to the node.'),
    '#default_value' => FILE_ATTACH_REUSE_NODES,
  );

  // Visibility settings.
  $form['visibility'] = array('#type' => 'fieldset', '#title' => t('Visibility settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['visibility']['file_attach_show_attachments'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Attachments'),
    '#description'   => t('Show the attached files.'),
    '#default_value' => FILE_ATTACH_SHOW_ATTACHMENTS,
  );
  $form['visibility']['file_attach_show_previews'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Previews'),
    '#description'   => t('Show the available generated previews for the attached files.'),
    '#default_value' => FILE_ATTACH_SHOW_PREVIEWS,
  );

  // Vocabulary settings.
  if (module_exists('taxonomy')) {
    $options = array();
    foreach (taxonomy_get_vocabularies('file') as $vid => $vocab) {
      $options[$vid] = $vocab->name;
    }
    $form['vocabs'] = array('#type' => 'fieldset', '#title' => t('Vocabulary settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
    $form['vocabs']['file_attach_vocabularies_all'] = array(
      '#type'          => 'radios',
      '#title'         => t('Vocabularies to inherit terms from'),
      '#options'       => array(TRUE => t('All'), FALSE => t('Only bellow selected vocabularies')),
      '#description'   => t('Select the vocabularies from which file attachments will inherit the taxonomy terms of the parent node on the creation of the attachment.'),
      '#default_value' => FILE_ATTACH_VOCABULARIES_ALL,
    );
    $form['vocabs']['file_attach_vocabularies'] = array(
      '#type'          => 'select',
      '#title'         => t('Choose vocabularies to inherit terms from'),
      '#options'       => $options,
      '#multiple'      => TRUE,
      '#description'   => t('If "Only bellow selected vocabularies" are selected, then choose the vocabularies.'),
      '#default_value' => unserialize(FILE_ATTACH_VOCABULARIES),
    );
  }

  // Organic groups integration.
  if (module_exists('og')) {
    $form['og'] = array('#type' => 'fieldset', '#title' => t('Organic groups integration'), '#collapsible' => TRUE, '#collapsed' => FALSE);
    $form['og']['file_attach_og_inheritance'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('OG membership inheritance'),
      '#description'   => t('Should attachments be put in the same groups as the parent node on the creation of the attachment.'),
      '#default_value' => FILE_ATTACH_OG_INHERITANCE,
    );
    if (module_exists('og_vocab')) {
      $form['og']['file_attach_og_vocabularies'] = array(
        '#type'          => 'checkbox',
        '#title'         => t('Inherit from group vocabularies'),
        '#default_value' => FILE_ATTACH_OG_VOCABULARIES,
        '#description'   => t('Taxonomy terms will be inherited from vocabularies defined in the group\'s scope by og_vocab module automatically even if they are not explicitly selected in "Choose vocabularies to inherit terms from".'),
      );
    }
  }

  if (module_exists('file_gallery')) {
    // Popup settings
    $form['popup'] = array('#type' => 'fieldset', '#title' => t('Popup settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
    $form['popup']['file_attach_popup_size'] = array(
      '#type'          => 'textfield',
      '#title'         => t('Size of file gallery selector popup window'),
      '#default_value' => FILE_ATTACH_POPUP_SIZE,
      '#size'          => 10,
      '#maxlength'     => 255,
      '#description'   => t('The default size is 590x500.'),
    );
    $form['popup']['file_attach_popup_per_page'] = array(
      '#type'          => 'textfield',
      '#title'         => t('Files per page'),
      '#default_value' => FILE_ATTACH_POPUP_PER_PAGE,
      '#size'          => 3,
      '#maxlength'     => 255,
      '#description'   => t('Sets the number of files to be displayed on a gallery page.'),
    );
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
function file_attach_admin_settings_validate($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      if (file_exists('file_gallery')) {
        $size = $values['file_attach_popup_size'];
        if (empty($size) || !preg_match('/^[0-9]+x[0-9]+$/', $size)) {
          form_set_error('file_attach_popup_size', t('Invalid value %value specified for popup size.', array('%value' => $size)));
        }
      }
      break;
  }
}

/**
 * Submit hook for the settings form.
 */
function file_attach_admin_settings_submit($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      variable_set('file_attach_reuse_nodes', $values['file_attach_reuse_nodes']);
      variable_set('file_attach_show_attachments', $values['file_attach_show_attachments']);
      variable_set('file_attach_show_previews', $values['file_attach_show_previews']);
      variable_set('file_attach_vocabularies_all', $values['file_attach_vocabularies_all']);
      variable_set('file_attach_vocabularies', $values['file_attach_vocabularies']);
      if (module_exists('og')) {
        variable_set('file_attach_og_inheritance', $values['file_attach_og_inheritance']);
        if (module_exists('og_vocab'))
          variable_set('file_attach_og_vocabularies', $values['file_attach_og_vocabularies']);
      }
      if (module_exists('file_gallery')) {
        variable_set('file_attach_popup_size', $values['file_attach_popup_size']);
        variable_set('file_attach_popup_per_page', $values['file_attach_popup_per_page']);
      }
      drupal_set_message(t('The configuration options have been saved.'));
      break;
    case t('Reset to defaults'):
      variable_del('file_attach_reuse_nodes');
      variable_del('file_attach_show_attachments');
      variable_del('file_attach_show_previews');
      variable_del('file_attach_vocabularies_all');
      variable_del('file_attach_vocabularies');
      variable_del('file_attach_og_inheritance');
      variable_del('file_attach_og_vocabularies');
      variable_del('file_attach_popup_size');
      variable_del('file_attach_popup_per_page');
      drupal_set_message(t('The configuration options have been reset to their default values.'));
      break;
  }
}

