<?php


function webmail_plus_preferences_presentation($user=NULL) {
	
  global $_webmail_plus_config;

  if(!$user) global $user;

  $user_preferences = webmail_plus_get_user_preferences(NULL, TRUE);

  $form['general'] = array(
  '#type' => 'fieldset',
    '#title' => t('General Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

 $form['general']['thread_messages'] = array(
   '#type' => 'checkbox',
   '#title' => t('Group similar messages into threads'),
   '#default_value' => $user_preferences['thread_messages'],
   '#description' => t('Message replies will be organized into a coversation.')
 );  
  
  
  $form['general']['messages_per_page'] = array(
    '#type' => 'select',
    '#title' => t('Number of messages per page'),
    '#options' => array('25'=>'25', '50'=>'50', '75'=>'75', '100'=>'100'),
    '#default_value' => $user_preferences['messages_per_page'],
    '#description' => t('A number of messages to show per page before paginating.'),
  );

  if(!$_webmail_plus_config->email_format) $_webmail_plus_config->email_format='plaintext';
  
  $form['general']['email_format'] = array(
    '#type' => 'select',
    '#title' => t('Preferred email format'),
    '#default_value' => $user_preferences['email_format'],
    '#options' => array('plaintext' => t('Plain Text'), 'html' => t('HTML')),      
    '#description' => t('Defines the format used for outbound email.')    
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}


function theme_webmail_plus_preferences_presentation($form) {
  $content = drupal_render($form);
  return $content;
}

function webmail_plus_preferences_presentation_submit($form, &$form_state) {
  global $user, $_webmail_plus_config;
  
  foreach($form_state['values'] as $key=>$value) {
    if(preg_match("/^(op|submit|form_*)/", $key)) continue;
    
    webmail_plus_set_user_preference($key, $value);
  }
  
  drupal_set_message(t('Your Webmail Plus preferences were updated'));

  $form_state['redirect']='user/'.$user->uid.'/webmail_plus';
}
?>