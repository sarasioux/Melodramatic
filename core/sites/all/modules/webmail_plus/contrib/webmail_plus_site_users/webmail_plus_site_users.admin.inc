<?php
function webmail_plus_site_users_admin_settings() {
  $form = array();
  
  $form['webmail_plus_site_users_first_name_field'] = array(
  	'#type' => 'textfield',
    '#title' => t('First Name'),
    '#default_value' => variable_get('webmail_plus_site_users_first_name_field', NULL)
  );

  $form['webmail_plus_site_users_last_name_field'] = array(
  	'#type' => 'textfield',
    '#title' => t('Last Name'),
    '#default_value' => variable_get('webmail_plus_site_users_last_name_field', NULL)
  );
  
  return system_settings_form($form);
  
}
 
