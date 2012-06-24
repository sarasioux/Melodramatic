<?php
// $Id$

/**
 * @file
 * Module admin page callbacks.
 */

//////////////////////////////////////////////////////////////////////////////
// File restriction settings

/**
 * Implements the settings page.
 *
 * @return
 *   The form structure.
 */
function file_restriction_admin_settings() {
  $form = array();

  $form['general'] = array('#type' => 'fieldset', '#title' => t('All users'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['general']['file_restriction_extensions_allowed'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Allowed file extensions'),
    '#default_value' => FILE_RESTRICTION_EXTENSIONS_ALLOWED,
    '#maxlength'     => 255,
    '#description'   => t('Only files with those extensions will be possible to upload to the website. Separate extensions with a space and do not include the leading dot. Empty means that all file types are allowed.'),
  );
  $form['general']['file_restriction_size'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Max file size'),
    '#default_value' => FILE_RESTRICTION_SIZE,
    '#size'          => 10,
    '#maxlength'     => 255,
    '#description'   => t('Maximum size of the files which are allowed to be uploaded. The size should be specified in MB. Zero means that no size restriction is applied.') . t('Your PHP settings limit the maximum file size per upload to %size.', array('%size' => format_size(file_upload_max_size()))),
  );
  $form['general']['file_restriction_quota'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Max size of all uploaded files'),
    '#default_value' => FILE_RESTRICTION_QUOTA,
    '#size'          => 10,
    '#maxlength'     => 255,
    '#description'   => t('Maximum size of the all uploaded files by the user. The size should be specified in MB. Zero means that no quotas are applied.'),
  );
  $form['general']['file_restriction_quota_generated'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Include generated files'),
    '#description'   => t('Count generated (converted) files when calculating total files\' size.'),
    '#default_value' => FILE_RESTRICTION_QUOTA_GENERATED,
  );

  $roles = user_roles(FALSE, 'upload files');
  $form['roles'] = array('#type' => 'value', '#value' => $roles);
  $roles_extensions_allowed = variable_get('file_restriction_roles_extensions_allowed', array());
  $roles_size = variable_get('file_restriction_roles_size', array());
  $roles_quota = variable_get('file_restriction_roles_quota', array());
  $roles_quota_generated = variable_get('file_restriction_roles_quota_generated', array());
  foreach ($roles as $rid => $role) {
    $form['settings_role_'. $rid] = array(
      '#type' => 'fieldset',
      '#title' => t('Settings for @role', array('@role' => $role)),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['settings_role_'. $rid]['file_restriction_extensions_allowed_'. $rid] = array(
      '#type'          => 'textfield',
      '#title'         => t('Allowed file extensions'),
      '#default_value' => isset($roles_extensions_allowed[$rid]) ? $roles_extensions_allowed[$rid] : FILE_RESTRICTION_EXTENSIONS_ALLOWED,
      '#maxlength'     => 255,
      '#description'   => t('Only files with those extensions will be possible to upload to the website. Separate extensions with a space and do not include the leading dot. Empty means that all file types are allowed.'),
    );
    $form['settings_role_'. $rid]['file_restriction_size_'. $rid] = array(
      '#type'          => 'textfield',
      '#title'         => t('Max file size'),
      '#default_value' => isset($roles_size[$rid]) ? $roles_size[$rid] : FILE_RESTRICTION_SIZE,
      '#size'          => 10,
      '#maxlength'     => 255,
      '#description'   => t('Maximum size of the files which are allowed to be uploaded. The size should be specified in MB. Zero means that no size restriction is applied.') . t('Your PHP settings limit the maximum file size per upload to %size.', array('%size' => format_size(file_upload_max_size()))),
    );
    $form['settings_role_'. $rid]['file_restriction_quota_'. $rid] = array(
      '#type'          => 'textfield',
      '#title'         => t('Max size of all uploaded files'),
      '#default_value' => isset($roles_quota[$rid]) ? $roles_quota[$rid] : FILE_RESTRICTION_QUOTA,
      '#size'          => 10,
      '#maxlength'     => 255,
      '#description'   => t('Maximum size of the all uploaded files by the user. The size should be specified in MB. Zero means that no quotas are applied.'),
    );
    $form['settings_role_'. $rid]['file_restriction_quota_generated_'. $rid] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Include generated files'),
      '#description'   => t('Count generated (converted) files when calculating total files\' size.'),
      '#default_value' => isset($roles_quota_generated[$rid]) ? $roles_quota_generated[$rid] : FILE_RESTRICTION_QUOTA_GENERATED,
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
function file_restriction_admin_settings_validate($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      $exceed_max_msg = t('Your PHP settings limit the maximum file size per upload to %size.', array('%size' => format_size(file_upload_max_size())));
      if (!is_numeric($values['file_restriction_size']) || $values['file_restriction_size'] < 0) {
        form_set_error('file_restriction_size', t('A file size should be a positive inumber.'));
      }
      if ($values['file_restriction_size'] * 1024 * 1024 > file_upload_max_size()) {
        form_set_error('file_restriction_size', $exceed_max_msg);
      }
      if ($values['file_restriction_size'] > $values['file_restriction_quota']) {
        form_set_error('file_restriction_size', t('Max file size is bigger than max size of all uploaded files.'));
      }
      if (!is_numeric($values['file_restriction_quota']) || $values['file_restriction_quota'] < 0) {
        form_set_error('file_restriction_quota', t('A disk quota should be a positive number.'));
      }

      foreach ($values['roles'] as $rid => $role) {
        if (!is_numeric($values['file_restriction_size_'. $rid]) || $values['file_restriction_size_'. $rid] < 0) {
          form_set_error('file_restriction_size_'. $rid, t('A %role file size should be a positive inumber.', array('%role' => $role)));
        }
        if ($values['file_restriction_size_'. $rid] * 1024 * 1024 > file_upload_max_size()) {
          form_set_error('file_restriction_size_'. $rid, $exceed_max_msg);
        }
        if ($values['file_restriction_size_'. $rid] > $values['file_restriction_quota_'. $rid]) {
          form_set_error('file_restriction_size_'. $rid, t('Max %role file size is bigger than max size of all uploaded files.', array('%role' => $role)));
        }
        if (!is_numeric($values['file_restriction_quota_'. $rid]) || $values['file_restriction_quota_'. $rid] < 0) {
          form_set_error('file_restriction_quota_'. $rid, t('A %role disk quota should be a positive number.', array('%role' => $role)));
        }
      }
      break;
  }
}

/**
 * Submit hook for the settings form.
 */
function file_restriction_admin_settings_submit($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      variable_set('file_restriction_extensions_allowed', trim($values['file_restriction_extensions_allowed']));
      variable_set('file_restriction_size', $values['file_restriction_size']);
      variable_set('file_restriction_quota', $values['file_restriction_quota']);
      variable_set('file_restriction_quota_generated', $values['file_restriction_quota_generated']);
      $roles_extensions_allowed = array();
      $roles_size = array();
      $roles_quota = array();
      $roles_quota_generated = array();
      foreach ($values['roles'] as $rid => $role) {
        $roles_extensions_allowed[$rid] = trim($values['file_restriction_extensions_allowed_'. $rid]);
        $roles_size[$rid] = $values['file_restriction_size_'. $rid];
        $roles_quota[$rid] = $values['file_restriction_quota_'. $rid];
        $roles_quota_generated[$rid] = $values['file_restriction_quota_generated_'. $rid];
      }
      variable_set('file_restriction_roles_extensions_allowed', $roles_extensions_allowed);
      variable_set('file_restriction_roles_size', $roles_size);
      variable_set('file_restriction_roles_quota', $roles_quota);
      variable_set('file_restriction_roles_quota_generated', $roles_quota_generated);
      drupal_set_message(t('The configuration options have been saved.'));
      break;
    case t('Reset to defaults'):
      variable_del('file_restriction_extensions_allowed');
      variable_del('file_restriction_size');
      variable_del('file_restriction_quota');
      variable_del('file_restriction_quota_generated');
      variable_del('file_restriction_roles_extensions_allowed');
      variable_del('file_restriction_roles_size');
      variable_del('file_restriction_roles_quota');
      variable_del('file_restriction_roles_quota_generated');
      drupal_set_message(t('The configuration options have been reset to their default values.'));
      break;
  }
}

