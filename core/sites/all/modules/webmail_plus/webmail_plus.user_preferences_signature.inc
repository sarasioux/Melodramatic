<?php
function webmail_plus_preferences_signature($user=NULL) {
  global $_webmail_plus_user_preferences;

  if(!$user) global $user;
  
  $webmail_plus_user_preferences = webmail_plus_get_user_preferences();


  $form['signature'] = array(
  '#type' => 'fieldset',
    '#title' => t('Email Signature'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['signature']['email_signature'] = array(
    '#type' => 'textarea',
    '#title' => t('Email Signature'),
    '#default_value' => $webmail_plus_user_preferences->email_signature
  );

  $form['signature']['email_signature_append'] = array(
    '#type' => 'checkbox',
    '#title' => t('Append the signature automatically.'),
    '#default_value' => $webmail_plus_user_preferences->email_signature_append
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function webmail_plus_preferences_signature_submit($form, &$form_state) {
  //webmail_plus_dump($form_state);

  webmail_plus_set_user_preference('email_signature', $form_state['values']['email_signature']);
  webmail_plus_set_user_preference('email_signature_append', $form_state['values']['email_signature_append']);

  drupal_set_message(t('Your signature preferences were updated successfully.'));
}