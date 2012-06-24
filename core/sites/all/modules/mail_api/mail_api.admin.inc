<?php
// $Id: mail_api.admin.inc,v 1.2.2.2 2009/05/14 18:39:03 firmidea Exp $

/**
 * @file
 * Administrative section for Mail API module
 *
 */

function mail_api_admin($type = 'list') {


  if ($type == 'list') {
    return drupal_get_form('mail_api_settings_overview');
  }
  else if ($type == 'add') {
     return drupal_get_form('mail_api_add_server');
  }


}

/**
 * Main Mail API settings page.
 *
 * This will show links that any other modules have put in the
 * admin/settings/mail_api path.  This is recommended for any
 * spam filtering modules, mail server modules, front-end modules,
 * etc., that need some site-wide configurations.
 */
function mail_api_settings() {
  $item = menu_get_item();
  $content = system_admin_menu_block($item);
  return theme('mail_api_settings', $content);
}

/**
 * Theme the Mail API settings page.
 */
function theme_mail_api_settings($content) {

  $output = '';

  if ($content) {
    $output = '<dl class="admin-list">';
    foreach ($content as $item) {
      $output .= '<dt>'. l($item['title'], $item['href'], $item['options']) .'</dt>';
      $output .= '<dd>'. $item['description'] .'</dd>';
    }
    $output .= '</dl>';
  }

  return $output;
}

/**
 * List mail server configurations.
 */
function mail_api_settings_overview() {

  $servers = mail_api_get_servers();
  $protocols = mail_api_protocols();
  $encryption = mail_api_encryption_options();


  $options['unsuspend']='Unsuspend selected servers';
  $options['suspend']='Suspend selected servers';
  $options['delete']='Delete selected servers';

  $form['action'] = array(
    '#type' => 'fieldset', '#title' => t('Actions'),
    '#prefix' => '<div class="container-inline">', '#suffix' => '</div>'
  );

  $form['action']['operation'] = array('#type' => 'select', '#options' => $options, '#default_value' => 'suspend');
  $form['action']['submit'] = array('#type' => 'submit', '#value' => t('Update'));

  $form['header'] = array('#type' => 'value', '#value' => array(
    theme('table_select_header_cell'),
    array('data' => t('Domain'), 'field' => 'domain'),
    array('data' => t('Protocol'), 'field' => 'protocol'),
    //array('data' => t('Hostname'), 'field' => 'hostname'),
    //array('data' => t('Port'), 'field' => 'port'),
    //array('data' => t('Encryption'), 'field' => 'encryption'),
    array('data' => t('Status'), 'field' => 'suspended'),
    array('data' => t('Operations'))
  ));

  $result = pager_query('SELECT * FROM {mail_api_servers}', 50);

  $destination = drupal_get_destination();

  while ($server = db_fetch_object($result)) {
    
    $selected[$server->id] = '';
    $server_extra = array();
    $server_extra_str = "";
    
    // we need to mark suspended servers
    if ($server->suspended==1) {
      $server_extra[] = t('suspended');
    }

   // create a string to append to the domain
   if ($server_extra) $server_extra_str = " (". implode(", ", $server_extra) .")";
   
   $form['domain'][$server->id] = array('#value' => trim($server->domain . $server_extra_str));
   $form['protocol'][$server->id] = array('#value' => $server->protocol);
   
   if ($server->suspended) {
    $form['status'][$server->id] = array('#value' => t('Suspended'));
   }
   else {
     $form['status'][$server->id] = array('#value' => t('Active'));
   }
   $form['operations'][$server->id] = array('#value' => l(t('edit'), 'admin/settings/mail_api/edit/'. $server->id, array('query' => $destination)));
  }

  $form['selected'] = array('#type' => 'checkboxes', '#options' => $selected);
  $form['pager'] = array('#value' => theme('pager', NULL, 50, 0));

  return $form;
}


function theme_mail_api_settings_overview($form) {

  if (sizeof($form['domain'])<=0) {
    $content .= t('No servers configured. Click \'Add Server\' above to configure a new server.');
    return $content;
  }
  
  $output = drupal_render($form['action']);
  
  if (isset($form['protocol'])) {
    foreach (element_children($form['protocol']) as $key) {
      $row = array();
      $row[] = drupal_render($form['selected'][$key]);
      $row[] = drupal_render($form['domain'][$key]);
      //$row[] = drupal_render($form['name'][$key]);
      $row[] = drupal_render($form['protocol'][$key]);
      //$row[] = drupal_render($form['hostname'][$key]);
      //$row[] = drupal_render($form['port'][$key]);
      //$row[] = drupal_render($form['encryption'][$key]);
      $row[] = drupal_render($form['status'][$key]);
      $row[] = drupal_render($form['operations'][$key]);
      $rows[] = $row;
    }

  }
  else {
    $rows[] = array(array('data' => t('No servers available.'), 'colspan' => '6'));
  }

  $output .= theme('table', $form['header']['#value'], $rows);

  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);

  return $output;

}

/**
 * hook_submit processes the mail_api_settings_overview form
 *
 * @param unknown_type $form_id
 * @param unknown_type $form_state
 */
function mail_api_settings_overview_submit($form_id, &$form_state) {
  $destination = drupal_get_destination();
  
  if ($form_state['values']['operation'] == 'suspend' && sizeof($form_state['values']['selected'])>0) {
    foreach ($form_state['values']['selected'] as $id => $selected) {
      if ($id != $selected) continue;
      db_query("UPDATE {mail_api_servers} SET suspended=1 WHERE id=%d", $id);
    }
  }

  if ($form_state['values']['operation'] == 'unsuspend' && sizeof($form_state['values']['selected'])>0) {
    foreach ($form_state['values']['selected'] as $id => $selected) {

      if ($id != $selected) continue;
      db_query("UPDATE {mail_api_servers} SET suspended=0 WHERE id=%d", $id);

    }
  }


  if ($form_state['values']['operation'] == 'delete' && sizeof($form_state['values']['selected'])>0) {
    foreach ($form_state['values']['selected'] as $id => $selected) {
      if ($id != $selected) continue;
      db_query("DELETE FROM {mail_api_servers} WHERE id=%d", $id);
    }
  }
  
  $form_state['redirect'] = 'admin/settings/mail_api';

}

function mail_api_edit_server() {
  return drupal_get_form('mail_api_edit_server_form');
}


function mail_api_edit_server_form() {

  //drupal_set_message('editing!');

  $id = arg(4);
  $server  = db_fetch_object(db_query("SELECT * FROM {mail_api_servers} WHERE id=%d", $id));


  $encryption = mail_api_value_mirror_key(mail_api_encryption_options());
  $protocols = mail_api_protocols();



  $form['mail_api'] = array(
    '#title' => t('Mail Server Information'),
    '#type' => 'fieldset',
  );

  $form['mail_api']['id'] = array(
    '#type' => 'hidden',
    '#value' => $id
  );

  /*
  $form['mail_api']['mail_api_connection_name'] = array(
    '#title' => t('Name'),
    '#description' => t('Give this connection a unique name.'),
    '#type' => 'textfield',
    '#default_value' => $server->name,
    '#required' => TRUE,
  );
  */

  $form['mail_api']['mail_api_protocol'] = array(
    '#title' => t('Protocol'),
    '#type' => 'select',
    '#default_value' => $server->protocol,
    '#options' => $protocols,
  );

  $form['mail_api']['mail_api_protocol_module'] = array(
    '#type' => 'hidden',
    '#value' => $protocols[$name]['module'],
  );

  $form['mail_api']['mail_api_hostname'] = array(
    '#title' => t('Server Hostname'),
    '#description' => t('Host name or IP address of the mail server host'),
    '#type' => 'textfield',
    '#default_value' => $server->hostname,
    '#required' => TRUE,
  );

  $form['mail_api']['mail_api_port'] = array(
    '#title' => t('Server Port'),
    '#type' => 'textfield',
    '#default_value' => $server->port,
    '#required' => TRUE,
  );

  $form['mail_api']['mail_api_encryption_values'] = array(
    '#type' => 'value',
    '#value' => $protocols[$name]['encryption'],
  );

  $form['mail_api']['mail_api_encryption'] = array(
    '#title' => t('Encryption'),
    '#description' => t('Encryption method used to log into the server'),
    '#type' => 'select',
    '#default_value' => $server->encryption,
    '#options' => $encryption
  );

  $form['mail_api']['mail_api_domain'] = array(
    '#title' => t('Domain'),
    '#description' => t('Domain associated with this connection, e.g. domain.com.'),
    '#type' => 'textfield',
    '#default_value' => $server->domain,
    '#description' => t('The domain name in the users email address.  This is how Mail API can determine which server module to use for a user to access their email.'),
  );

  $form['mail_api']['mail_api_folder_prefix'] = array(
    '#title' => t('Folder Prefix'),
    '#description' => t('A prefix for folders, e.g. INBOX.'),
    '#type' => 'textfield',
    '#default_value' => $server->folder_prefix,
    '#description' => t('Some servers store folders as subfolders of INBOX so the full folder name looks like INBOX.folder. If this is the case, specify \'INBOX.\'.'),
  );

  $form['mail_api']['mail_api_hide_folder_prefix'] = array(
    '#title' => t('Hide Folder Prefix'),
    '#description' => t('Hide folder prefix.'),
    '#type' => 'checkbox',
    '#default_value' => $server->hide_folder_prefix,
    '#description' => t('If your folders have a prefix and you want to hide it, check this box.'),
  );

  $form['mail_api']['mail_api_inbox_folder'] = array(
    '#title' => t('Inbox Folder'),
    '#description' => t('Full path to the Inbox folder.'),
    '#type' => 'textfield',
    '#default_value' => $server->inbox_folder,
    '#description' => t('The folder where all incoming mail arroves. If not sure set to INBOX'),
  );

  $form['mail_api']['mail_api_trash_folder'] = array(
    '#title' => t('Trash Folder'),
    '#description' => t('Full path to the Trash folder.'),
    '#type' => 'textfield',
    '#default_value' => $server->trash_folder,
    '#description' => t('If your server uses a folder prefix, specify PREFIX.Trash, e.g. INBOX.Trash.'),
  );

  $form['mail_api']['mail_api_sent_folder'] = array(
    '#title' => t('Sent Folder'),
    '#description' => t('Full path to the Sent folder.'),
    '#type' => 'textfield',
    '#default_value' => $server->sent_folder,
    '#description' => t('If your server uses a folder prefix, specify PREFIX.Sent, e.g. INBOX.Sent.'),
  );

  $form['mail_api']['mail_api_drafts_folder'] = array(
    '#title' => t('Drafts Folder'),
    '#description' => t('Full path to the Drafts folder.'),
    '#type' => 'textfield',
    '#default_value' => $server->drafts_folder,
    '#description' => t('If your server uses a folder prefix, specify PREFIX.Drafts, e.g. INBOX.Drafts.'),
  );

  $form['mail_api']['mail_api_outbox_folder'] = array(
    '#title' => t('Outbox Folder'),
    '#type' => 'textfield',
    '#default_value' => $server->outbox_folder,
    '#description' => t("This is the path to the 'Outbox' folder, if the mail server supports sending emails in this manner.  If this folder does not exist in the user's account, it will be created.  Any SMTP settings will be ignored if this is used."),
  );


  $form['mail_api']['mail_api_options'] = array(
    '#title' => t('Connection options'),
    '#description' => t('Additional options, if any, to pass to the connection. Usually these are IMAP options.'),
    '#type' => 'textfield',
    '#default_value' => $server->options,
    '#description' => t('Some connections may require additional options, please specify them here.'),
  );



  $form['mail_api']['mail_api_login_suffix'] = array(
    '#title' => t('Login Suffix'),
    '#description' => t('A suffix supplied with the username during login.'),
    '#type' => 'textfield',
    '#default_value' => $server->login_suffix,
    '#description' => t('If your mail server requires a string appended to the username, e.g. user@domain.com or user+domain.com, enter it here.'),
  );




  $form['buttons']['submit'] = array('#type' => 'submit', '#value' => t('Save configuration') );
  $form['buttons']['reset'] = array('#type' => 'submit', '#value' => t('Reset to defaults') );


  return $form;

}

function theme_mail_api_edit_server_form($form) {
  return drupal_render($form);
}


function mail_api_edit_server_form_validate($form, &$form_state) {
  $count = db_result(db_query("SELECT COUNT(*) FROM {mail_api_servers} WHERE id<>%d AND domain='%s'", $form_state['values']['id'], $form_state['values']['mail_api_domain']));

  if ($count>0) form_set_error('mail_api][domain', t('A connection for this domain already exists.'));
}

function mail_api_edit_server_form_submit($form, &$form_state) {


  if (module_exists('aes')) {
    if ($form_state['values']['smtp_primary_password']) {
      $smtp_primary_password = aes_encrypt($form_state['values']['smtp_primary_password']);
    }

    if ($form_state['values']['smtp_backup_password']) {
      $smtp_backup_password = aes_encrypt($form_state['values']['smtp_backup_password']);
    }
  }
  else {
    $smtp_primary_password = $form_state['values']['smtp_primary_password'];
    $smtp_backup_password = $form_state['values']['smtp_backup_password'];
  }



  db_query(
    "UPDATE {mail_api_servers} SET protocol='%s', protocol_module='%s', hostname='%s', port='%s', encryption='%s', domain='%s', folder_prefix='%s', hide_folder_prefix=%d, inbox_folder='%s', trash_folder='%s', sent_folder='%s', drafts_folder='%s', outbox_folder='%s', options='%s', login_suffix='%s'  WHERE id=%d",
    $form_state['values']['mail_api_protocol'],
    $form_state['values']['mail_api_protocol_module'],
    $form_state['values']['mail_api_hostname'],
    (int) $form_state['values']['mail_api_port'],
    $form_state['values']['mail_api_encryption'],
    $form_state['values']['mail_api_domain'],
    $form_state['values']['mail_api_folder_prefix'],
    $form_state['values']['mail_api_hide_folder_prefix'],
    $form_state['values']['mail_api_inbox_folder'],
    $form_state['values']['mail_api_trash_folder'],
    $form_state['values']['mail_api_sent_folder'],
    $form_state['values']['mail_api_drafts_folder'],
    $form_state['values']['mail_api_outbox_folder'],
    $form_state['values']['mail_api_options'],
    $form_state['values']['mail_api_login_suffix'],
    $form_state['values']['id']
  );

  drupal_set_message(t('The server configuration has been updated.'));

  $form_state['redirect'] = 'admin/settings/mail_api';


}


/**
 * Create a form for adding new mail server configuration to database.
 */
function mail_api_add_server() {
  $encryption = mail_api_value_mirror_key(mail_api_encryption_options());


  //print_r($encryption);

  $protocols = mail_api_protocols();


  /*
  foreach($protocols as $name => $value) {
    if($name == $p_name['name']) {
      break;
    }
  }
  */

  $form['mail_api'] = array(
    '#title' => t('Mail Server Information'),
    '#type' => 'fieldset',
  );

  /*
  $form['mail_api']['mail_api_connection_name'] = array(
    '#title' => t('Name'),
    '#description' => t('Give this connection a unique name.'),
    '#type' => 'textfield',
    '#required' => TRUE,
  );
  */

  $form['mail_api']['mail_api_protocol'] = array(
    '#title' => t('Protocol'),
    '#type' => 'select',
    '#options' => $protocols,
  );

  $form['mail_api']['mail_api_protocol_module'] = array(
    '#type' => 'hidden',
    '#value' => $protocols[$name]['module'],
  );

  $form['mail_api']['mail_api_hostname'] = array(
    '#title' => t('Server Hostname'),
    '#description' => t('Host name or IP address of the mail server host'),
    '#type' => 'textfield',
    '#required' => TRUE,
  );
  $form['mail_api']['mail_api_port'] = array(
    '#title' => t('Server Port'),
    '#type' => 'textfield',
    '#default_value' => 143,
    '#required' => TRUE,
  );
  $form['mail_api']['mail_api_encryption_values'] = array(
    '#type' => 'value',
    '#value' => $protocols[$name]['encryption'],
  );
  $form['mail_api']['mail_api_encryption'] = array(
    '#title' => t('Encryption'),
    '#description' => t('Encryption method used to log into the server'),
    '#type' => 'select',
    '#options' => $encryption
  );

  $form['mail_api']['mail_api_domain'] = array(
    '#title' => t('Domain'),
    '#description' => t('Domain associated with this connection, e.g. domain.com.'),
    '#type' => 'textfield',
    '#description' => t('The domain name in the users email address.  This is how Mail API can determine which server module to use for a user to access their email.'),
  );


  $form['mail_api']['mail_api_folder_prefix'] = array(
    '#title' => t('Folder Prefix'),
    '#description' => t('A prefix for folders, e.g. INBOX.'),
    '#type' => 'textfield',
    '#default_value' => $server->folder_prefix,
    '#description' => t('Some servers store folders as subfolders of INBOX so the full folder name looks like INBOX.folder. If this is the case, specify \'INBOX.\'.'),
  );

  $form['mail_api']['mail_api_hide_folder_prefix'] = array(
    '#title' => t('Hide Folder Prefix'),
    '#description' => t('Hide folder prefix.'),
    '#type' => 'checkbox',
    '#description' => t('If your folders have a prefix and you want to hide it, check this box.'),
  );

  $form['mail_api']['mail_api_inbox_folder'] = array(
    '#title' => t('Inbox Folder'),
    '#description' => t('Full path to the Inbox folder.'),
    '#type' => 'textfield',
    '#default_value' => 'INBOX',
    '#description' => t('The folder where all incoming mail arrives.'),
  );

  $form['mail_api']['mail_api_trash_folder'] = array(
    '#title' => t('Trash Folder'),
    '#description' => t('Full path to the Trash folder.'),
    '#type' => 'textfield',
    '#default_value' => 'Trash',
    '#description' => t('If your server uses a folder prefix, specify PREFIX.Trash, e.g. INBOX.Trash.'),
  );

  $form['mail_api']['mail_api_sent_folder'] = array(
    '#title' => t('Sent Folder'),
    '#description' => t('Full path to the Sent folder.'),
    '#type' => 'textfield',
    '#default_value' => 'Sent',
    '#description' => t('If your server uses a folder prefix, specify PREFIX.Sent, e.g. INBOX.Sent.'),
  );

  $form['mail_api']['mail_api_drafts_folder'] = array(
    '#title' => t('Drafts Folder'),
    '#description' => t('Full path to the Drafts folder.'),
    '#type' => 'textfield',
    '#default_value' => 'Drafts',
    '#description' => t('If your server uses a folder prefix, specify PREFIX.Drafts, e.g. INBOX.Drafts.'),
  );

  $form['mail_api']['mail_api_outbox'] = array(
    '#title' => t('Outbox Folder'),
    '#type' => 'textfield',
    '#description' => t("This is the path to the 'Outbox' folder, if the mail server supports sending emails in this manner.  If this folder does not exist in the user's account, it will be created.  Any SMTP settings will be ignored if this is used."),
  );


  $form['mail_api']['mail_api_options'] = array(
    '#title' => t('Connection options'),
    '#description' => t('Additional options, if any, to pass to the connection.'),
    '#type' => 'textfield',
    '#default_value' => '/notls',
    '#description' => t('Some connections may require additional options, please specify them here.'),
  );


  $form['mail_api']['mail_api_login_suffix'] = array(
    '#title' => t('Login Suffix'),
    '#description' => t('A suffix supplied with the username during login.'),
    '#type' => 'textfield',
    '#description' => t('If your mail server requires a string appended to the username, e.g. user@domain.com or user+domain.com, enter it here.'),
  );


  /*
  $form['smtp'] = array(
    '#title' => t('SMTP Server Configuration'),
    '#type' => 'fieldset',
  );
  $form['smtp']['primary'] = array(
    '#title' => t('Primary SMTP Server'),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['smtp']['primary']['smtp_primary_hostname'] = array(
    '#title' => t('Hostname'),
    '#type' => 'textfield',
  );
  $form['smtp']['primary']['smtp_primary_port'] = array(
    '#title' => t('Port'),
    '#type' => 'textfield',
  );
  $form['smtp']['primary']['smtp_primary_encryption_values'] = array(
    '#type' => 'value',
    '#value' => $encryption,
  );
  $form['smtp']['primary']['smtp_primary_encryption'] = array(
    '#title' => t('Encryption'),
    '#type' => 'select',
    '#options' => $form['smtp']['primary']['smtp_primary_encryption_values']['#value'],
  );
  $form['smtp']['primary']['auth'] = array(
    '#title' => t('SMTP Authentication'),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['smtp']['primary']['auth']['smtp_primary_auth'] = array(
    '#title' => t('Enable SMTP Authentication'),
    '#type' => 'checkbox',
    '#default_value' => FALSE,
    '#description' => t('Leave username and password blank to use user credentials.'),
  );
  $form['smtp']['primary']['auth']['smtp_primary_username'] = array(
    '#title' => t('Username'),
    '#type' => 'textfield',
    '#description' => t('Remove user name to clear password.'),
  );
  $form['smtp']['primary']['auth']['smtp_primary_password'] = array(
    '#title' => t('Password'),
    '#type' => 'password',
    '#description' => t('Leave this blank unless setting a new password.'),
  );

  $form['smtp']['backup'] = array(
    '#title' => t('Backup SMTP Server'),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['smtp']['backup']['smtp_backup_hostname'] = array(
    '#title' => t('Hostname'),
    '#type' => 'textfield',
  );
  $form['smtp']['backup']['smtp_backup_port'] = array(
    '#title' => t('Port'),
    '#type' => 'textfield',
  );
  $form['smtp']['backup']['smtp_backup_encryption_values'] = array(
    '#type' => 'value',
    '#value' => $encryption,
  );
  $form['smtp']['backup']['smtp_backup_encryption'] = array(
    '#title' => t('Encryption'),
    '#type' => 'select',
    '#options' => $form['smtp']['backup']['smtp_backup_encryption_values']['#value'],
  );
  $form['smtp']['backup']['auth'] = array(
    '#title' => t('SMTP Authentication'),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['smtp']['backup']['auth']['smtp_backup_auth'] = array(
    '#title' => t('Enable SMTP Authentication'),
    '#type' => 'checkbox',
    '#default_value' => FALSE,
    '#description' => t('Leave username and password blank to use user credentials.'),
  );
  $form['smtp']['backup']['auth']['smtp_backup_username'] = array(
    '#title' => t('Username'),
    '#type' => 'textfield',
    '#description' => t('Remove user name to clear password.'),
  );
  $form['smtp']['backup']['auth']['smtp_backup_password'] = array(
    '#title' => t('Password'),
    '#type' => 'password',
    '#description' => t('Leave this blank unless setting a new password.'),
  );

  */
  $form['buttons']['submit'] = array('#type' => 'submit', '#value' => t('Save configuration') );
  $form['buttons']['reset'] = array('#type' => 'submit', '#value' => t('Reset to defaults') );

  return $form;
}


/**
 * Insert new mail server configuration into database.
 */
function mail_api_add_server_submit($form, &$form_state) {

  if (module_exists('aes')) {
    $smtp_primary_password = aes_encrypt($form_state['values']['smtp_primary_password']);
    $smtp_backup_password = aes_encrypt($form_state['values']['smtp_backup_password']);;
  }
  else {
    $smtp_primary_password = $form_state['values']['smtp_primary_password'];
    $smtp_backup_password = $form_state['values']['smtp_backup_password'];
  }

  db_query(
    'INSERT INTO {mail_api_servers} (
    `protocol`,
    `protocol_module`,
    `hostname`,
    `port`,
    `encryption`,
    `domain`,
    `folder_prefix`,
    `hide_folder_prefix`,
    `inbox_folder`,
    `trash_folder`,
    `sent_folder`,
    `drafts_folder`,
    `outbox_folder`,
    `options`,
    `login_suffix`

    ) VALUES (
    "%s",
    "%s",
    "%s",
    %d,
    "%s",
    "%s",
    "%s",
    %d,
    "%s",
    "%s",
    "%s",
    "%s",
    "%s",
    "%s",
    "%s"
    )',
    $form_state['values']['mail_api_protocol'],
    $form_state['values']['mail_api_protocol_module'],
    $form_state['values']['mail_api_hostname'],
    (int) $form_state['values']['mail_api_port'],
    $form_state['values']['mail_api_encryption'],
    $form_state['values']['mail_api_domain'],
    $form_state['values']['mail_api_folder_prefix'],
    $form_state['values']['mail_api_hide_folder_prefix'],
    $form_state['values']['mail_api_inbox_folder'],
    $form_state['values']['mail_api_trash_folder'],
    $form_state['values']['mail_api_sent_folder'],
    $form_state['values']['mail_api_drafts_folder'],
    $form_state['values']['mail_api_outbox_folder'],
    $form_state['values']['mail_api_options'],
    $form_state['values']['mail_api_login_suffix']
  );

  drupal_set_message(t('The server configuration has been saved.'));

  $form_state['redirect'] = 'admin/settings/mail_api';
}




function mail_api_add_server_validate($form, &$form_state) {
  /*
  if(strlen($form_state['values']['mail_api_connection_name'])>USERNAME_MAX_LENGTH) {
    form_set_error('mail_api][connection_name', t('Connection name can not be longer than 64 characters.'));

  }
  */

  $count = db_result(db_query("SELECT COUNT(*) FROM {mail_api_servers} WHERE domain='%s'", $form_state['values']['mail_api_domain']));
  
  if ($count>0) form_set_error('mail_api][connection_name', t('A connection with this name already exists.'));
}

/**
 * takes an array and returns an array where each key equals to value
 *
 * @param unknown_type $array
 * @return unknown
 */
function mail_api_value_mirror_key($array) {
   if (sizeof($array)<=0) return FALSE;

   foreach ($array as $key => $value) {
      $new[$value]=$value;
   }

   return $new;
}