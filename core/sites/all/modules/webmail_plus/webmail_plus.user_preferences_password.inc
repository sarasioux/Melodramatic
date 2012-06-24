<?php

function webmail_plus_preferences_password() {

  global $user, $_webmail_plus_config;

  //echo $_webmail_plus_config->authentication;

  /**
   * i was gonna hide this menu but due to drupal caching this isn't a good idea. instead i show an error message
   */
  if($_webmail_plus_config->authentication!='user_defined') {
    drupal_set_message(t('Webmail Plus was not configured to allow password changes.'), 'error', FALSE);
  }
  else {

    $form['password'] = array(
      '#title' => t('Email Password'),
      '#type' => 'password',
      '#required' => TRUE,
    );

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Set Password')
    );

    return $form;

  }
}

function theme_webmail_plus_preferences_password($form) {
}



function webmail_plus_preferences_password_submit($form, &$form_state) {
  global $_webmail_plus_config;
  webmail_plus_set_user_password($form_state['values']['password']);

  $_webmail_plus_config->password = $form_state['values']['password'];
  
  // remove the connection_disabled flag
  unset($_SESSION['webmail_plus']['connection_disabled']);
  drupal_set_message(t('Your email password was changed successfully.'), 'notice', FALSE);

  $form_state['redirect']='webmail_plus/gateway';
}


?>