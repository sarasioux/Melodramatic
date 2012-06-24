<?php
// $Id$

/**
 * @file
 * Module admin page callbacks.
 */

//////////////////////////////////////////////////////////////////////////////
// File taxonomy settings

/**
 * Implements the settings page.
 *
 * @return
 *   The form structure.
 */
function file_browser_admin_settings() {
  $form = array();

  // General settings
  $form['general'] = array('#type' => 'fieldset', '#title' => t('General settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['general']['file_browser_blocks'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Enable file browser blocks'),
    '#default_value' => FILE_BROWSER_BLOCKS,
    '#description'   => t('Whether a file browser block should be created for each taxonomy vocabulary which supports file nodes.'),
  );

  // Vocabulary settings
  $options = array();
  foreach (taxonomy_get_vocabularies('file') as $vid => $vocabulary) {
    $options[$vid] = $vocabulary->name;
  }
  $form['vocabs'] = array('#type' => 'fieldset', '#title' => t('Vocabulary settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['vocabs']['file_browser_vocabularies_all'] = array(
    '#type'          => 'radios',
    '#title'         => t('Vocabularies to be shown in the browser'),
    '#options'       => array(TRUE => t('All'), FALSE => t('Only bellow selected vocabularies')),
    '#description'   => t('Select the vocabularies which will be shown in the file browser.'),
    '#default_value' => FILE_BROWSER_VOCABULARIES_ALL,
  );
  $form['vocabs']['file_browser_vocabularies'] = array(
    '#type'          => 'select',
    '#title'         => t('Include cross-cutting vocabularies'),
    '#default_value' => unserialize(FILE_BROWSER_VOCABULARIES),
    '#options'       => $options,
    '#multiple'      => TRUE,
    '#description'   => t('Only the selected vocabularies will be shown in the file browser.'),
  );

  // Display settings
  $form['display'] = array('#type' => 'fieldset', '#title' => t('Display settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['display']['file_browser_hide_empty'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Hide empty folders'),
    '#default_value' => FILE_BROWSER_HIDE_EMPTY,
    '#description'   => t('Whether taxonomy terms that have no sub-terms or file nodes associated with them should be hidden. This can be useful to reduce load time in situations where the vocabulary is very large (e.g. for free-tagging vocabularies).'),
  );
  $form['display']['file_browser_embed_previews'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Embed thumbnail in file preview block'),
    '#default_value' => FILE_BROWSER_EMBED_PREVIEWS,
    '#description'   => t('Whether the file preview block should include an embedded thumbnail of the file\'s content, for those file formats which provide thumbnail capabilities.'),
  );
  $form['display']['file_browser_folder_properties'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Show folder\'s files count and size'),
    '#default_value' => FILE_BROWSER_FOLDER_PROPERTIES,
    '#description'   => t('Whether the number and the total count of the size of all files below the term should be displayed along with the term name in the browser.'),
  );
  $form['display']['file_browser_folder_links'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Show folder\'s contextual menu'),
    '#default_value' => FILE_BROWSER_FOLDER_LINKS,
    '#description'   => t('Show a contextual menu for a folder.'),
  );

  // Organic groups integration
  if (module_exists('og')) {
    $form['og'] = array('#type' => 'fieldset', '#title' => t('Organic groups integration'), '#collapsible' => TRUE, '#collapsed' => FALSE);
    if (module_exists('og_vocab')) {
      $form['og']['file_browser_og_vocabularies'] = array(
        '#type'          => 'checkbox',
        '#title'         => t('Include group vocabularies'),
        '#default_value' => FILE_BROWSER_OG_VOCABULARIES,
        '#description'   => t('Vocabularies defined in the group\'s scope by og_vocab module will be included in the browser automatically even if they are not explicitly selected in "Include cross-cutting vocabularies".'),
      );
    }
    $form['og']['file_browser_og_upload'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Enable selection of audience on file upload'),
      '#default_value' => FILE_BROWSER_OG_UPLOAD,
      '#description'   => t('Whether the file upload block should allow selection of the audience/groups for the files to be uploaded.'),
    );
    $form['og']['file_browser_og_preview'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Show groups in file preview block'),
      '#default_value' => FILE_BROWSER_OG_PREVIEW,
      '#description'   => t('Whether the file preview block should include information about the audience/groups the currently selected file is targeted at.'),
    );
    if (module_exists('og_vocab')) {
      $form['og']['file_browser_og_unfiled'] = array(
        '#type'          => 'textfield',
        '#title'         => t('Associate non-categorized file nodes with group folder'),
        '#default_value' => FILE_BROWSER_OG_UNFILED,
        '#size'          => 20,
        '#maxlength'     => 255,
        '#description'   => t('Input a folder name here if you wish to enforce that all unfiled file nodes are automatically associated with a category of that name in the og_vocab.module vocabularies for the groups the file belongs to. Leave empty to disable this functionality.'),
      );
      $form['og']['file_browser_og_create'] = array(
        '#type'          => 'checkbox',
        '#title'         => t('Allow new folders (terms) creation'),
        '#default_value' => FILE_BROWSER_OG_CREATE,
        '#description'   => t('Whether the creation of new folders in the groups\' vocabularies root should be allowed.'),
      );
    }
  }

  // Location module integration
  if (module_exists('location')) {
    $form['location'] = array('#type' => 'fieldset', '#title' => t('Location module integration'), '#collapsible' => TRUE, '#collapsed' => FALSE);
    $form['location']['file_browser_location_preview'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Show geolocation information in file preview block'),
      '#default_value' => FILE_BROWSER_LOCATION_PREVIEW,
      '#description'   => t('Whether to display latitude and longitude information for spatially-enabled nodes. <em>Requires location.module.</em>'),
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
function file_browser_admin_settings_validate($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  switch ($op) {
    case t('Save configuration'):
      break;
  }
}

/**
 * Submit hook for the settings form.
 */
function file_browser_admin_settings_submit($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      variable_set('file_browser_blocks', $values['file_browser_blocks']);
      variable_set('file_browser_vocabularies_all', $values['file_browser_vocabularies_all']);
      variable_set('file_browser_vocabularies', $values['file_browser_vocabularies']);
      variable_set('file_browser_hide_empty', $values['file_browser_hide_empty']);
      variable_set('file_browser_embed_previews', $values['file_browser_embed_previews']);
      variable_set('file_browser_folder_properties', $values['file_browser_folder_properties']);
      variable_set('file_browser_folder_links', $values['file_browser_folder_links']);
      if (module_exists('og')) {
        variable_set('file_browser_og_vocabularies', $values['file_browser_og_vocabularies']);
        variable_set('file_browser_og_upload', $values['file_browser_og_upload']);
        variable_set('file_browser_og_preview', $values['file_browser_og_preview']);
        if (module_exists('og_vocab')) {
          variable_set('file_browser_og_unfiled', $values['file_browser_og_unfiled']);
          variable_set('file_browser_og_create', $values['file_browser_og_create']);
        }
      }
      if (module_exists('location'))
        variable_set('file_browser_location_preview', $values['file_browser_location_preview']);
      drupal_set_message(t('The configuration options have been saved.'));
      break;
    case t('Reset to defaults'):
      variable_del('file_browser_blocks');
      variable_del('file_browser_vocabularies_all');
      variable_del('file_browser_vocabularies');
      variable_del('file_browser_hide_empty');
      variable_del('file_browser_embed_previews');
      variable_del('file_browser_folder_properties');
      variable_del('file_browser_folder_links');
      variable_del('file_browser_og_vocabularies');
      variable_del('file_browser_og_upload');
      variable_del('file_browser_og_preview');
      variable_del('file_browser_og_unfiled');
      variable_del('file_browser_og_create');
      variable_del('file_browser_location_preview');
      drupal_set_message(t('The configuration options have been reset to their default values.'));
      break;
  }
}

