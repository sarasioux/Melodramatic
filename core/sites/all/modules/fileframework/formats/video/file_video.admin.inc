<?php
// $Id$

/**
 * @file
 * Module admin page callbacks.
 */

//////////////////////////////////////////////////////////////////////////////
// File video settings

/**
 * Implements the settings page.
 *
 * @return
 *   The form structure.
 */
function file_video_admin_settings() {
  $form = array();

  // Flowplayer ettings
  $form['flowplayer'] = array('#type' => 'fieldset', '#title' => t('Flowplayer settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['flowplayer']['file_video_flowplayer_version'] = array(
    '#type'          => 'textfield',
    '#title'         => t('Flowplayer version'),
    '#default_value' => FILE_VIDEO_FLOWPLAYER_VERSION,
    '#size'          => 10,
    '#maxlength'     => 255,
    '#description'   => t('Specify version of the installed !flowplayer player. It is recommended that you install this player in order to play video files in the browser. To install, !download the latest version of the player and unzip it to !path under the Drupal directory.', array('!flowplayer' => l('Flowplayer', 'http://flowplayer.org/'), '!download' => l(t('download'), 'http://flowplayer.org/download.html'), '!path' => drupal_get_path('module', 'file') .'/vendor/flowplayer/')),
  );

  // getID3 settings
  $form['getid3'] = array('#type' => 'fieldset', '#title' => t('getID3 settings'), '#collapsible' => TRUE, '#collapsed' => FALSE);
  $form['getid3']['file_video_getid3'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Enable Extraction of the <a href="http://getid3.sourceforge.net" title="getID()">getID</a> metadata'),
    '#default_value' => FILE_VIDEO_GETID3,
    '#description'   => t('Check this box if you want to extract any of the following metadata.'),
  );
  $getid3 = _file_video_getid3();
  foreach ($getid3 as $name => $data) {
    $getid3_data_default[$name] = $data['default'];
  }
  $getid3_data = variable_get('file_video_getid3_data', $getid3_data_default);
  $i = $j = 0;
  foreach ($getid3 as $name => $data) {
    $form['getid3']['getid3_'. $i][$j]['file_video_getid3_'. $name] = array(
      '#type'          => 'checkbox',
      '#title'         => $data['name'],
      '#default_value' => $getid3_data[$name],
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
function file_video_admin_settings_validate($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  switch ($op) {
    case t('Save configuration'):
      break;
  }
}

/**
 * Submit hook for the settings form.
 */
function file_video_admin_settings_submit($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      variable_set('file_video_flowplayer_version', $values['file_video_flowplayer_version']);
      variable_set('file_video_getid3', $values['file_video_getid3']);
      $getid3 = _file_video_getid3();
      $getid3_data = array();
      foreach ($getid3 as $name => $data) {
        $getid3_data[$name] = $values['file_video_getid3_'. $name];
      }
      variable_set('file_video_getid3_data', $getid3_data);
      drupal_set_message(t('The configuration options have been saved.'));
      break;
    case t('Reset to defaults'):
      variable_del('file_video_flowplayer_version');
      variable_del('file_video_getid3');
      variable_del('file_video_getid3_data');
      drupal_set_message(t('The configuration options have been reset to their default values.'));
      break;
  }
}

