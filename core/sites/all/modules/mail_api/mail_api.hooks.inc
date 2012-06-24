<?php
// $Id: mail_api.hooks.inc,v 1.2.2.2 2009/05/14 18:39:03 firmidea Exp $

/**
 * @file
 * Implements the hooks for Mail API
 *
 */

$mail_api_server_info="";


/**
 * Invoke Mail API hook in a module based on users mail domain.
 */
function mail_api_invoke() {
   global $_mail_api_module;

   if ($_mail_api_module=="") return FALSE;

   $args = func_get_args();


   $mail_api_server_info = mail_api_server_configuration($domain);
   $mail_api_server_info['protocol_module']='maildir_api';

   array_unshift($args, $mail_api_server_info['protocol_module']);
   $args[0]=$_mail_api_module;


   if (!function_exists($args[0] .'_'. $args[1])) {
     drupal_set_message('Unknown function '. $args[0] .'_'. $args[1] .' was called.', 'error');
   }

   $rs = call_user_func_array('module_invoke', $args);

   return $rs;
}


/**
* connect to a server of choice
*/
function mail_api_connect($domain, $username, $password, $folder="") {


   global $_mail_api_module, $mail_api_server_info;

   if (!$domain) {
      if (MAIL_API_DEBUG) drupal_set_message('domain not passed');
      return FALSE;
   }


   $mail_api_server_info = mail_api_server_configuration($domain);

   if (MAIL_API_DEBUG) drupal_set_message("mail_api_connect $username/$password");

   $_mail_api_module = strtolower(trim($mail_api_server_info->protocol)) .'_api';

   $rs = module_invoke($_mail_api_module, 'connect', $mail_api_server_info->hostname, $username . $mail_api_server_info->login_suffix, $password, $mail_api_server_info->port, $folder, $mail_api_server_info->options);

   return $rs;
}


/**
returns the name of the module that's used to access mail storage
*/
function mail_api_module() {
   global $_mail_api_module;

   return $_mail_api_module;
}

/**
returns status information on a mailbox
option list
    * SA_MESSAGES - set status->messages to the number of messages in the mailbox
    * SA_RECENT - set status->recent to the number of recent messages in the mailbox
    * SA_UNSEEN - set status->unseen to the number of unseen (new) messages in the mailbox
    * SA_UIDNEXT - set status->uidnext to the next uid to be used in the mailbox
    * SA_UIDVALIDITY - set status->uidvalidity to a constant that changes when uids for the mailbox may no longer be valid
    * SA_ALL - set all of the above
*/
function mail_api_status($mailbox="INBOX", $options=SA_ALL) {
   if (MAIL_API_DEBUG) drupal_set_message("mail_api_status $mailbox $options");

   $rs = mail_api_invoke('status', $mailbox, $options);

   return $rs;
}


/**
 * Get list of mail servers.
 */
function mail_api_get_servers() {
  $result = db_query("SELECT * FROM {mail_api_servers} WHERE suspended=0");

  $servers = array();

  while ($server = db_fetch_array($result)) {
    $servers[] = $server;
  }

  return $servers;
}



/**
 * Get the domain name from the user's email address.
 */
function mail_api_get_domain() {
  global $user;

  if ($_SESSION['mail_api']) {
    $domain = end(explode('@', $user->name));

    return $domain;
  }
  return FALSE;
}


/**
 * Implementation of hook_init().
 */
function mail_api_init() {
  global $user;
  /*
  if($domain = mail_api_get_domain()) {
    // if we find the user's mail server domain name
    // get the server's configuration information
    $server_info = mail_api_server_configuration($domain);
    if ($auth = mail_api_invoke('mail_api_authenticate', $user->name, mail_api_get_password(), $domain, $server_info)) {
      // if successfully authenticated
      // register a shutdown function to log out of the mail server
      // at the end of this page load.
      register_shutdown_function('mail_api_invoke', 'mail_api_logout');
    }
  }
  */
}



/**
 * Implementation of hook_user().
 */
function mail_api_user($op, &$edit, &$account, $category = NULL) {
  switch ($op) {
    case 'login':
      break;
    case 'logout':
      // if logging out
      // clear all mail server session information
      unset($_SESSION['mail_api']);
      break;
  }
}

/**
 * Get configured server configuration by domain name from the database
 */
function mail_api_server_configuration($domain) {
  $result = db_fetch_object(db_query("SELECT * FROM {mail_api_servers} WHERE domain = '%s' AND suspended=0", $domain));

  return $result;
}



/**
takes a domain and returns the module used to connect to this domain
*/
function mail_api_get_module_by_domain($domain="") {
   $connection = db_fetch_object(db_query("SELECT * FROM {mail_api_servers} WHERE domain='%s'", $domain));
   $module = strtolower($connection->protocol);
   return $module;
}



/**
 * Gets the number of messages in the current mailbox
 *
 * @param unknown_type $request
 * @return unknown
 */
function mail_api_num_msg() {
  $rs = mail_api_invoke('num_msg', $request);
  return $rs;
}


// FIXME here

/**
 * Returns headers for all messages in a mailbox
 *
 * @return unknown
 */
function mail_api_headers() {
  $rs = mail_api_invoke('headers', $request);
  return $rs;
}

/**
 * Returns a message header
 * if ARRAY is passed, returns an array
 *
 * @return unknown
 */
function mail_api_header($msgno) {

  $rs = mail_api_invoke('header', $msgno, FT_UID);
  return $rs;



}

/**
 * Read the message body
 * FT_UID - The msg_number  is a UID
 * FT_PEEK - Do not set the \Seen flag if not already set
 * FT_INTERNAL - The return string is in internal format, will not canonicalize to CRLF.
 *
 */
function mail_api_body($msg_number, $options=FT_UID, $seen=0) {
  $rs = mail_api_invoke('body', $msg_number, $options & $seen);
  return $rs;
}


/**
 * Read an overview of the information in the headers of the given message
 *
 * @return unknown
 */
function mail_api_overview($sequence="", $options="") {
  //drupal_set_message('mail_api_overview');
  $rs = mail_api_invoke('overview', $sequence, $options);
  return $rs;
}

/**
 * Check current mailbox
 *
 * @return unknown
 */
function mail_api_check() {
  $rs = mail_api_invoke('check');
  return $rs;
}


/**
 * Attempt to authenticate the user against the appropriate mail server.
 */
function mail_api_authenticate($form_values = array('name' => '', 'pass' => '')) {
  global $user;

  if (!valid_email_address($form_values['name']) || empty($form_values['pass'])) {
    return FALSE;
  }

  $domain = end(explode('@', $form_values['name']));

  $server_info = mail_api_server_configuration($domain);
  if (module_invoke($server_info['protocol_module'], 'mail_api_authenticate', $form_values['name'], $form_values['pass'], $domain, $server_info)) {
    user_external_login_register($form_values['name'], 'mail_api');
    user_authenticate_finalize($form_values);
    $_SESSION['mail_api'] = array(
      'protocol_module' => $server_info['protocol_module'],
    );
    mail_api_set_password($form_values['pass']);
    return $user;
  }
  return FALSE;
}

/**
 * Get the user's password from the $_SESSION variable.
 */
function mail_api_get_password() {
  if ($_SESSION['mail_api']['enc'] && module_exists('aes')) {
    // if the AES module is installed, and the 'enc' session variable
    // is TRUE, use aes_decrypt to decrypt the module.
    return aes_decrypt($_SESSION['mail_api']['pass']);
  }
  else {
    // otherwise just return the password stored in the session.
    return $_SESSION['mail_api']['pass'];
  }
}

/**
 * Set the user's password in the $_SESSION variable.
 */
function mail_api_set_password($password) {
  if (module_exists('aes') && $_SESSION['mail_api']['pass'] = aes_encrypt($password)) {
    // if the AES module is installed, and setting the password for the
    // session in encrypted form works:
    $_SESSION['mail_api']['enc'] = TRUE;
  }
  else {
    // otherwise set the password in the session variable in plain text
    $_SESSION['mail_api']['pass'] = $password;
    $_SESSION['mail_api']['enc'] = FALSE;
  }
}

/**
 * Return array of encryption options for SMTP
 */
function mail_api_encryption_options() {
  return array(t('None'), t('SSL'), t('TLS'));
}

function mail_api_protocol($action, $name) {
   //die($action.' - '.$name);

}

/**
 * Return list of supported protocols.
 */
function mail_api_protocols() {


   // Invoke a hook in all enabled modules that implement it.
  $protocols = module_invoke_all('mail_api_protocols');

  return $protocols;
}

/**
 * Return the quota for the mail account of the user.
 */
function mail_api_account_quota() {
  return mail_api_invoke('mail_api_account_quota');
}



/**
 * Get folders from cache.
 * pass * to fetch all mailboxes
 * @return: Return an array of folder information.
 */
function mail_api_mailboxes($pattern="*") {
  if (MAIL_API_DEBUG) drupal_set_message("mail_api_mailboxes");

  $mailboxes = mail_api_invoke('mailboxes', $pattern);

  if (!is_array($mailboxes)) return FALSE;
  
  foreach ($mailboxes as $id => $object) {
    $name = trim($object->name);

    if ($name=='') continue;

    $rs[]=mail_api_format_mailbox($name);
  }

  return $rs;
}

/**
 * Create new folder in user's mail server account
 */
function mail_api_create_folder($folder_path) {
  // Call hook_mail_api_create_folder() on the appropriate mail server module.
  $success = mail_api_invoke('mail_api_create_folder', $folder_path);

  return $success;
}

/**
 * Move/rename folder in user's mail server account
 */
function mail_api_move_folder($folder_path, $new_folder_path) {
  // Call hook_mail_api_move_folder() on the appropriate mail server module.
  $success = mail_api_invoke('mail_api_move_folder', $folder_path, $new_folder_path);

  // Rebuild the folder cache
  /*.
  if ($success) {
    mail_api_rebuild_folder_cache();
  }
  */

  return $success;
}

/**
 * Delete folder in user's mail server account
 */
function mail_api_delete_folder($folder_path) {
  // Call hook_mail_api_delete_folder() on the appropriate mail server module.
  $success = mail_api_invoke('mail_api_delete_folder', $folder_path);

  // Rebuild the folder cache
  /*.
  if ($success) {
    mail_api_rebuild_folder_cache();
  }
  */

  return $success;
}

/**
 * Get all tags from mail server and store them in the cache.
 */
function mail_api_rebuild_tag_cache() {
  global $user;

  // get user's tags from the mail server
  if ($tags = mail_api_invoke('mail_api_tags')) {
    // store them in the database
    db_query('DELETE FROM {mail_api_tag_cache} WHERE uid = %d', $user->uid);
    foreach ($tags as $tag) {
      db_query('INSERT INTO {mail_api_tag_cache} (uid, name) VALUES (%d, "%s")', $user->uid, $tag['name']);
    }
  }
}

/**
 * Get tags from cache.
 *
 * @return: Returns an array of tags.
 */
function mail_api_tags() {
  global $user;
  $result = db_query('SELECT * FROM {mail_api_tag_cache} WHERE uid = %d', $user->uid);

  while ($tag = db_fetch_array($result)) {
    $tags[] = $tag;
  }

  return $tags;
}

/**
 * Rename a tag.
 */
function mail_api_rename_tag($tag, $new_tag) {
  // Call hook_mail_api_rename_tag() on the appropriate mail server module.
  $success = mail_api_invoke('mail_api_rename_tag', $tag, $new_tag);

  // Rebuild the tag cache.
  if ($success) {
    mail_api_rebuild_tag_cache();
  }

  return $success;
}

/**
 * Delete a tag.
 */
function mail_api_delete_tag($tag) {
  // Call hook_mail_api_delete_tag() on the appropriate mail server module.
  $success = mail_api_invoke('mail_api_delete_tag', $tag);

  // Rebuild the tag cache.
  if ($success) {
    mail_api_rebuild_tag_cache();
  }

  return $success;
}

/**
 * Get a page of results from a search.
 */
function mail_api_pager_search($keywords = '', $folders = array(), $flags = array(), $header_fields = array(), $tags = array(), $sections = array(), $sort = array(), $limit = 25, $element = 0) {
  global $pager_page_array, $pager_total, $pager_total_items;
  $page = isset($_GET['page']) ? $_GET['page'] : '';

  // Call hook_mail_api_pager_search().
  $search_results = mail_api_invoke('mail_api_pager_search', $keywords, $folders, $flags, $header_fields, $tags, $sections, $sort, $limit);

  $pager_page_array = explode(',', $page);

  $pager_total_items[$element] = $search_results['count'];
  $pager_total[$element] = ceil($pager_total_items[$element] / $limit);
  $pager_page_array[$element] = max(0, min((int)$pager_page_array[$element], ((int)$pager_total[$element]) - 1));

  return $search_results;
}

/**
 * Send an email.
 */
function mail_api_send($message) {
  global $user;

  // Get domain name of user's account.
  $domain = mail_api_get_domain();
  // Get server configuration for that domain.
  $server = mail_api_server_configuration($domain);

  if ($server->outbox) {
    // if the mail server supports sending messages, do this
    return mail_api_invoke('mail_api_send', $message);
  }
  elseif (module_exists('smtp')) {
    // this is based off the drupal_mail_wrapper() function from the SMTP Auth module
    // we could not use that function as is, however, since it uses site-wide settings
    // and we are using mail server specific and user specific settings

    $mail = new phpmailer(); //Create a new phpmailer object.
    $username = $server->smtp_primary_username;
    $password = $server->smtp_primary_password;

    $from_name = $user->name;
    $from = $user->name;

    $auth = $server->smtp_primary_auth ? TRUE : FALSE;

    if (!$username) {
      // if there's not a smtp username set for this mail server
      // use the user's username and password
      $mail->Username = $user->name;
      $mail->Password = mail_api_get_password();
    }
    else {
      // else use the configured smtp username/passwords for this server.
      $mail->Username = $username;
      $mail->Password = $password;
    }

    //Take care of the email headers.
    foreach ($message['headers'] as $name => $value) {
      //watchdog('error', 'Key: ' . $name . ' Value: ' . $value);
      if (strtolower($name) == 'content-type' && strpos(strtolower($value), 'text/html') !== FALSE) {
        $mail->IsHTML(TRUE);
      }
      else if (strtolower($name) == 'content-type' && strpos(strtolower($value), 'multipart/mixed') !== FALSE) {
        //$body passed to smtp should already be formatted. add multipart header and tell phpmailer to leave it alone
        $mail->AddCustomHeader($name .': '. $value);
        $mail->message_type = 'pre';
      }
      else if (strtolower($name) == 'reply-to') {
        $mail->AddReplyTo = $value;
      }
      else if (strtolower($name) == 'return-path') {
        if (trim($value) !=  '') {
          $mail->Sender = $value;
        }
      }
      else if (strtolower($name) == 'content-transfer-encoding') {
        $mail->Encoding = $value;
      }
      else if (strtolower($name) == 'mime-version') {
        // just ommit MIME-Version it since it will be set by PHP-Mailer
      }
      else if (strtolower($name) == 'bcc') {
        $bccrecipients = split(',', $value);
        foreach ($bccrecipients as $bccrecipient) {
          if ( strpos($bccrecipient, '<') !== FALSE ) {
            $bccparts = explode(' <', $bccrecipient);
            $bccname = $bccparts[0];
            $bccaddr = rtrim($bccparts[1], '>');
          }
          else {
            $bccname = '';
            $bccaddr = $bccrecipient;
          }
          $mail->AddBCC($bccaddr, $bccname);
        }
      }
      else { //Else the header key is not special, just add it.
        $mail->AddCustomHeader($name .': '. $value); //Add header line.
      }
    }

    $enc_options = mail_api_encryption_options();
    switch (strtolower($enc_options[$server->smtp_primary_encryption])) {
      case 'ssl':
        $mail->Protocol = 'ssl://';
        break;
      case 'tls':
        $mail->Protocol = 'tls://';
        break;
      case 'none':
        $mail->Protocol = '';
    }

    $mail->Host = $server->smtp_primary_hostname;
    $mail->Port = $server->smtp_primary_port;
    $mail->Mailer = 'smtp';
    $mail->SMTPAuth = $auth;

    $mail->CharSet = 'utf-8';

    $mail->From = $from;
    $mail->FromName = $from_name;

    $torecipients = split(',', $message['to']);
    foreach ($torecipients as $torecipient) {
      if (strpos($torecipient, '<') !== FALSE) {
        $toparts = explode(' <', $torecipient);
        $toname = $toparts[0];
        $toaddr = rtrim($toparts[1], '>');
      }
      else {
        $toname = "";
        $toaddr = $torecipient;
      }
      $mail->AddAddress($toaddr, $toname);
    }

    $mail->Subject = $message['subject'];
    $mail->Body = $message['body'];

    watchdog('mail_api', t('Sending mail to: !to', array('!to' => $message['to'])));

    //Try to send email, if it fails set watchdog entry.
    if (!$mail->Send()) {
      watchdog('mail_api', t('Error sending e-mail from !from to !to: .  Attempting any configured backup mail delivery methods.', array('!from' => $from, '!to' => $message['to'])) . $mail->ErrorInfo, NULL, WATCHDOG_ERROR);

      // change to backup smtp server information
      $mail->Host = $server->smtp_backup_hostname;
      $mail->Port = $server->smtp_backup_port;

      switch (strtolower($enc_options[$server->smtp_backup_encryption])) {
        case 'ssl':
          $mail->Protocol = 'ssl://';
          break;
        case 'tls':
          $mail->Protocol = 'tls://';
          break;
        case 'none':
          $mail->Protocol = '';
      }

      $username = $server->smtp_backup_username;
      $password = $server->smtp_backup_password;
      $auth = $server->smtp_backup_auth ? TRUE : FALSE;

      if ($username == '' && $password == '') {
        $mail->Username = $user->name;
        $mail->Password = mail_api_get_password();
      }
      else {
        $mail->Username = $username;
        $mail->Password = $password;
      }

      $mail->SMTPAuth = $auth;

      if (!$mail->Send()) {
        watchdog('mail_api', t('Error sending e-mail from !from to !to: from backup SMTP server.', array('!from' => $from, '!to' => $message['to'])) . $mail->ErrorInfo, NULL, WATCHDOG_ERROR);
        return FALSE;
      }
    }

    $mail->SmtpClose();
    return TRUE;
  }
  else {
    // error: no supported methods for sending emails
    watchdog('mail_api', t('Error: no supported/configured methods for sending emails.'), NULL, WATCHDOG_ERROR);
    return FALSE;
  }
}


function mail_api_format_mailbox($mailbox) {
  global $mail_api_server_info;


  if (!$mail_api_server_info->hide_folder_prefix) return $mailbox;

  if (strstr($mailbox, $mail_api_server_info->folder_prefix)) {
    $new = substr($mailbox, strpos($mailbox, $mail_api_server_info->folder_prefix)+strlen($mail_api_server_info->folder_prefix), strlen($mailbox));
  }

  return $new;

}




/**
 * returns the path to the inbox folder
 *
 * @param unknown_type $domain
 * @return unknown
 */
function mail_api_inbox_path($domain) {
  if ($domain=="") return FALSE;
  $object = mail_api_server_configuration($domain);
  
  return $object->inbox_folder;
}

/**
 * returns the path to the sent folder
 *
 * @param unknown_type $domain
 * @return unknown
 */
function mail_api_sent_path($domain) {
  if ($domain=="") return FALSE;
  $object = mail_api_server_configuration($domain);
  
  return $object->sent_folder;  
}

/**
 * returns the path to the drafts folder
 *
 * @param unknown_type $domain
 * @return unknown
 */
function mail_api_drafts_path($domain) {
  if ($domain=="") return FALSE;
  $object = mail_api_server_configuration($domain);
  
  return $object->drafts_folder;  
}

/**
 * returns the path to the junk folder
 *
 * @param unknown_type $domain
 * @return unknown
 */
function mail_api_junk_path($domain) {
  if ($domain=="") return FALSE;
  $object = mail_api_server_configuration($domain);
  
  return $object->junk_folder;  
}



/**
 * returns the path to the trash folder
 *
 * @param unknown_type $domain
 * @return unknown
 */
function mail_api_trash_path($domain) {
  if ($domain=="") return FALSE;
  $object = mail_api_server_configuration($domain);
  
  return $object->trash_folder;  
}

/**
 * appends a message to the mailbox
 *
 */
function mail_api_append($message) {
  $rs = mail_api_invoke('append', $message);
  return $rs;
}

/**
 * moves a message from the current folder to the specified folder
 *
 * @param unknown_type $message_uid
 * @param unknown_type $folder
 */
function mail_api_move_message($message_uid, $folder) {
  $rs = mail_api_invoke('move_message', $message_uid, $folder);
  return $rs; 
}

/**
 * deletes a message from the current folder
 *
 * @param unknown_type $message_uid
 * @param unknown_type $folder
 */
function mail_api_delete_message($message_uid) {
  $rs = mail_api_invoke('delete_message', $message_uid);
  return $rs; 
}



/**
 * flags a message in the current folder as read
 *
 * @param unknown_type $message_uid
 * @return unknown
 */
function mail_api_flag_read($message_uid) {
  $rs = mail_api_invoke('mark_read', $message_uid);
  return $rs; 
}

/**
 * flags a message in the current folder as unread
 *
 * @param unknown_type $message_uid
 * @return unknown
 */
function mail_api_flag_unread($message_uid) {
  $rs = mail_api_invoke('mark_unread', $message_uid);
  return $rs; 
}


function mail_api_get_structure($message_uid) {
  $rs = mail_api_invoke('get_structure', $message_uid);
  return $rs; 
}

function mail_api_get_map($message_uid) {
  $rs = mail_api_invoke('get_map', $message_uid);
  return $rs; 
}

/**
 * gets the plain text of the message body, the set_seen flag will set the message as seen if used
 *
 * @param unknown_type $message_uid
 * @param unknown_type $seen
 * @return unknown
 */
function mail_api_get_plaintext($message_uid, $set_seen=NULL) {
  $rs = mail_api_invoke('get_plaintext', $message_uid, $seen);
  return $rs; 
}

function mail_api_get_attachment_overview($message_uid) {
  $rs = mail_api_invoke('get_attachment_overview', $message_uid);
  return $rs; 
}

function mail_api_get_attachment_files($message_uid) {
  $rs = mail_api_invoke('get_attachment_files', $message_uid);
  return $rs; 
}

function mail_api_get_part($message_uid, $part_number) {
  $rs = mail_api_invoke('get_part', $message_uid, $part_number);
  return $rs; 
}



function mail_api_get_part_object($message_uid, $part_number) {
  $rs = mail_api_invoke('get_part_object', $message_uid, $part_number);
  return $rs; 
}

function mail_api_get_part_mime_type($message_uid, $part_number) {
  $rs = mail_api_invoke('get_part_mime_type', $message_uid, $part_number);
  return $rs; 
}