<?php
function webmail_plus_preferences_general() {
  global $user;

  $webmail_plus_user_preferences = webmail_plus_get_user_preferences();


  $form['general'] = array(
	'#type' => 'fieldset',
    '#title' => t('General Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );


  $options = array(
    '25' => '25',
    '50' => '50',
    '75' => '75',
    '100' => '100'
  );

  $form['general']['conversations_per_page'] = array(
    '#type' => 'select',
    '#title' => t('Conversations per page'),
  	'#options' => $options,
    '#default_value' => $webmail_plus_user_preferences->conversations_per_page
  );

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

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}


function theme_webmail_plus_preferences_general($form) {
  $content = drupal_render($form);
  return $content;
}

function webmail_plus_preferences_general_submit($form, &$form_state) {
  global $user;

  // first check if there's something there already
  $count = db_result(db_query('SELECT COUNT(*) FROM {webmail_plus_user_preferences} WHERE uid=%d', $user->uid));

  if($count>0) {
    db_query("UPDATE {webmail_plus_user_preferences} SET conversations_per_page=%d, email_signature='%s' WHERE uid=%d", $form_state['values']['conversations_per_page'], $form_state['values']['email_signature'], $user->uid);

  } else {
    db_query("INSERT INTO {webmail_plus_user_preferences}(uid, conversations_per_page, email_signature) VALUES(%d, %d, '%s')", $user->uid,  $form_state['values']['conversations_per_page'], $form_state['values']['email_signature']);
  }

  drupal_set_message(t('Your Webmail Plus preferences were updated'));

  $form_state['redirect']='user/'.$user->uid.'/webmail_plus';
}
?>