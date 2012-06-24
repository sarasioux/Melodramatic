<?php
// $Id$

/**
 * @file
 * Module admin page callbacks.
 */

//////////////////////////////////////////////////////////////////////////////
// File antivirus settings

/**
 * Implements the settings page.
 *
 * @return
 *   The form structure.
 */
function file_antivirus_admin_settings() {
  $form = array();

  $form['antivirus'] = array('#type' => 'fieldset', '#title' => t('ClamAV'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['antivirus']['file_antivirus_clamav'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Enable <a href="http://www.clamav.net" title="ClamAV">ClamAV</a> anti-virus scans'),
    '#default_value' => FILE_ANTIVIRUS_CLAMAV,
  );
  $form['antivirus']['file_antivirus_clamav_method'] = array(
    '#type'          => 'radios',
    '#title'         => t('Select anti-virus scan method'),
    '#options'       => array(FALSE => t('Connect to a \'clamav-daemon\' TCP socket'), TRUE => t('Run a \'clamscan\' program')),
    '#default_value' => FILE_ANTIVIRUS_CLAMAV_METHOD,
    '#description'   => t('A preferable way is to use a \'clamav-daemon\' since it ensures much faster performance then a \'clamscan\' program. Daemon should have a read access to the web server\'s temporal files in order to scan them.'),
  );
  $form['antivirus']['file_antivirus_clamav_allow'] = array(
    '#type'          => 'radios',
    '#title'         => t('Allow file uploads if the daemon is not running or program is not found'),
    '#options'       => array(FALSE => t('No'), TRUE => t('Yes')),
    '#default_value' => FILE_ANTIVIRUS_CLAMAV_ALLOW,
    '#description'   => t('Choose if you want to allow file uploads when scan is enabled, but the daemon is not running or program is not found.'),
  );
  // Deamon.
  $clamav_daemon_version = _file_antivirus_check_clamav(array('host' => FILE_ANTIVIRUS_CLAMAV_HOST, 'port' => FILE_ANTIVIRUS_CLAMAV_PORT, 'version' => TRUE));
  $clamav_daemon_version = preg_match('/ClamAV/', $clamav_daemon_version) ? $clamav_daemon_version : NULL;
  $message = isset($clamav_daemon_version) ? t('The \'clamav-daemon\' %version is found at %host:%port.', array('%host' => FILE_ANTIVIRUS_CLAMAV_HOST, '%port' => FILE_ANTIVIRUS_CLAMAV_PORT, '%version' => $clamav_daemon_version)) : t('The clamav-daemon is not found at %host:%port.', array('%host' => FILE_ANTIVIRUS_CLAMAV_HOST, '%port' => FILE_ANTIVIRUS_CLAMAV_PORT));
  $form['daemon'] = array('#type' => 'fieldset', '#title' => t('ClamAV daemon'), '#description' => t('These settings will be used if you have chosen "Connect to a \'clamav-daemon\' TCP socket" above. '. $message), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['daemon']['file_antivirus_clamav_host'] = array(
    '#type'          => 'textfield',
    '#title'         => t('\'clamav-daemon\' host'),
    '#default_value' => FILE_ANTIVIRUS_CLAMAV_HOST,
    '#maxlength'     => 255,
    '#description'   => t('A hostname \'clamav-daemon\' is running on. For most cases it will run on the same machine as a webserver.'),
  );
  $form['daemon']['file_antivirus_clamav_port'] = array(
    '#type'          => 'textfield',
    '#title'         => t('\'clamav-daemon\' TCP port'),
    '#default_value' => FILE_ANTIVIRUS_CLAMAV_PORT,
    '#size'          => 6,
    '#maxlength'     => 8,
    '#description'   => t('A TCP port \'clamav-daemon\' is listening to. Default port for the daemon is 3310.'),
  );
  // Utility.
  $form['utility'] = array('#type' => 'fieldset', '#title' => t('ClamAV program'), '#description' => t('These settings will be used if you have chosen "Run a \'clamscan\' program" above.'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  if ($clamav_utility_path = exec('which clamscan 2>/dev/null')) {
    $clamav_utility_version = _file_antivirus_check_clamav(array('path' => $clamav_utility_path));
    $clamav_utility_version = preg_match('/ClamAV/', $clamav_utility_version) ? $clamav_utility_version : NULL;
  }
  $message = isset($clamav_utility_version) ? t('The \'clamscan\' %version is found at %path. If the field is left empty the default utility will be used.', array('%path' => $clamav_utility_path, '%version' => $clamav_utility_version)) : t('The \'clamscan\' is not found. Please, specify the full path.');
  $form['utility']['file_antivirus_clamav_path'] = array(
    '#type'          => 'textfield',
    '#title'         => t('A path to the scanning utility \'clamscan\''),
    '#default_value' => FILE_ANTIVIRUS_CLAMAV_PATH,
    '#maxlength'     => 255,
    '#description'   => t('Full path to the \'clamscan\' utility.') .' '. $message,
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
function file_antivirus_admin_settings_validate($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      $clamav_path = $values['file_antivirus_clamav_path'];
      if ($values['file_antivirus_clamav_method'] && !empty($clamav_path)) {
        if (!preg_match('/clamscan$/', $clamav_path)) {
          form_set_error('file_antivirus_clamav_path', t('Invalid path %path specified for the \'clamscan\' utility. The path should end with a \'clamscan\'.', array('%path' => $clamav_path)));
        }
        else if (!file_exists($clamav_path)) {
          form_set_error('file_antivirus_clamav_path', t('Invalid path %path specified for the \'clamscan\' utility. Program not found.', array('%path' => $clamav_path)));
        }
        else if (!preg_match('/ClamAV/', _file_antivirus_check_clamav(array('path' => $clamav_path)))) {
          form_set_error('file_antivirus_clamav_path', t('Invalid path %path specified for the \'clamscan\' utility. Utility have not returned it\'s version string.', array('%path' => $clamav_path)));
        }
      }
      if (!$values['file_antivirus_clamav_method']) {
        if (empty($values['file_antivirus_clamav_host'])) {
          form_set_error('file_antivirus_clamav_host', t('A hostname should be provided to connect to.'));
        }
        if (empty($values['file_antivirus_clamav_port'])) {
          form_set_error('file_antivirus_clamav_port', t('A port should be provided to connect to.'));
        }
        else if (!ctype_digit($values['file_antivirus_clamav_port'])) {
          form_set_error('file_antivirus_clamav_port', t('A port should only contain digits.'));
        }
      }
      break;
  }
}

/**
 * Submit hook for the settings form.
 */
function file_antivirus_admin_settings_submit($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      variable_set('file_antivirus_clamav', $values['file_antivirus_clamav']);
      variable_set('file_antivirus_clamav_method', $values['file_antivirus_clamav_method']);
      variable_set('file_antivirus_clamav_allow', $values['file_antivirus_clamav_allow']);
      variable_set('file_antivirus_clamav_host', $values['file_antivirus_clamav_host']);
      variable_set('file_antivirus_clamav_port', $values['file_antivirus_clamav_port']);
      variable_set('file_antivirus_clamav_path', $values['file_antivirus_clamav_path']);
      drupal_set_message(t('The configuration options have been saved.'));
      break;
    case t('Reset to defaults'):
      variable_del('file_antivirus_clamav');
      variable_del('file_antivirus_clamav_method');
      variable_del('file_antivirus_clamav_allow');
      variable_del('file_antivirus_clamav_host');
      variable_del('file_antivirus_clamav_port');
      variable_del('file_antivirus_clamav_path');
      drupal_set_message(t('The configuration options have been reset to their default values.'));
      break;
  }
}

