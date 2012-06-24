<?php

    // fetch the attachments
  $attachments = mail_api_get_attachment_files($message_info->message_uid);
  
  //wpd($attachments);
  if(is_array($attachments)) {
    $files_obj = new stdClass();
    $files_obj -> files = array();
    
      foreach($attachments as $attachment_id=>$attachment_info) {
        //wpd($attachment_info);
      
        //echo file_directory_temp();
        
        // fetch the attachment
        $part_body = mail_api_get_part($message_info->message_uid, $attachment_info['part_number']);
        // and store it
        file_put_contents(file_directory_temp().'/'.$attachment_info['file'], $part_body);
        
       
        $file_bitcache_id = bitcache_put_file(NULL, file_directory_temp().'/'.$attachment_info['file']);
        $this_file_obj = new stdClass();
        $this_file_obj->list = 1;
        $this_file_obj -> sid = 's_'.$attachment_id;
        $this_file_obj -> nid = NULL;
        $this_file_obj -> vid = NULL;
        $this_file_obj -> uri = 'bitcache://'.$file_bitcache_id;
        $this_file_obj -> size = $attachment_info['size'];
        $this_file_obj -> name = $attachment_info['file'];
        $this_file_obj -> weight = 0;
        
        
        $files_obj -> files['n_'.$attachment_id] = $this_file_obj;
        $_SESSION['file_attach_files']['s_'.$attachment_id] = $this_file_obj;    
        
      }
  }
  
  $form_attach = _file_attach_form($files_obj);
  
  _file_attach_form_fieldset($form, $form_attach);
  
  $form['attachments']['wrapper']['files'] = array_merge($form['attachments']['wrapper']['files'] , $form_attach['files']);
  
// ------


/**
 * fetches a password that was stored in the webmails password storage, for now plain-text
 *
 * @param unknown_type $uid
 * @return unknown
 */
function webmail_plus_get_stored_user_password($uid="") {
  global $user;

  $password = db_result(db_query("SELECT `password` FROM {webmail_plus_user_passwords} WHERE uid=%d", $user->uid));

  return $password;
}

/**
 * Returns the password associated with the UID
 * if UID is passed, a password for that UID will be looked up, otherwise the current UID is assumed
 *
 * @param unknown_type $uid
 * @return unknown
 */
function webmail_plus_get_aes_user_password($uid="") {
   global $user;

   /*
    * if the AES module is activated we can fetch passwords directly
    */
   if (module_exists('aes')) {
     $pass = aes_get_password($user->uid, true);
     return $pass;
   }



   $encrypted__password = db_result(db_query("SELECT password FROM {webmail_plus_user_passwords} WHERE uid=%d", $uid));
   $encrypted_password = trim($encrypted_password);

   if (empty($encrypted_password)) return FALSE;

   $decrypted_password = trim(aes_decrypt($encrypted_password));
   return $decrypted_password;
}

function webmail_plus_update_threads($folder="") {
  global $user, $_webmail_plus_config, $_webmail_plus_connection;
  require_once "webmail_plus.functions.inc";


  //print_r($_SESSION);

  if($_SESSION['webmail_plus']['connection_disabled']) return FALSE;


  if ($folder=="") $folder="INBOX";



  $mail_api_connection = webmail_plus_connect($folder);
  $overview = mail_api_overview();

  $threads = array();

  $all_message_ids = array();

  if(sizeof($overview)<=0) {

    // truncate tables first
    db_query("DELETE FROM {webmail_plus_user_threads} WHERE uid=%d AND folder='%s'", $user->uid, $folder);
    db_query("DELETE FROM {webmail_plus_user_threads_map} WHERE uid=%d AND folder='%s'", $user->uid, $folder);

    // then return
    return FALSE;
  }




  foreach ($overview as $index => $header) {
    //echo "processing $index\n";

    //webmail_plus_dump($header);

    $raw = mail_api_header($header->msgno, $options);
    $header_arr = webmail_plus_parse_header($raw);

    //webmail_plus_dump($header_arr['Subject']);

    $references = webmail_plus_extract_references($raw);
    //webmail_plus_dump($references);

    unset($this_thread);
    foreach($references as $reference=>$junk) {

      // see if this email already belongs to a thread
      if($reference_to_thread_map[$reference]) {
        $this_thread = $reference_to_thread_map[$reference];
        break;
      }

    }



    // if the message wasn't referenced, create a thread for it
    if(!$this_thread) {
      $this_thread = sizeof($reference_to_thread_map)+1;
      //echo "$index wasn't found so we created $this_thread\n";

    } else {
      //echo "$index was found in $this_thread\n";
    }



    // go thru all references and map them out
    foreach($references as $reference=>$junk) {

      if($reference_to_thread_map[$reference]) continue;

      $reference_to_thread_map[$reference]=$this_thread;


      // this is used for reference
      $thread_to_reference_map[$this_thread][]=$reference;

      // prevent duplicate headers from being inserted

      if(is_array($thread_email[$this_thread])) {
        foreach($thread_email[$this_thread] as $email_id=>$email_header) {
          if($email_header->message_id==$header->message_id) continue(2);
        }
      }

      // this is used to generate records in the thread view
      $thread_email[$this_thread][]=$header;

    }
  }



  //webmail_plus_dump($reference_to_thread_map);

  //echo "folder is $folder<br>\n";

  // clean the tasbles
  /*
  db_query("TRUNCATE TABLE {webmail_plus_user_threads_map}");
  db_query("TRUNCATE TABLE {webmail_plus_user_threads}");
  */



  db_query("DELETE FROM {webmail_plus_user_threads} WHERE uid=%d AND folder='%s'", $user->uid, $folder);
  db_query("DELETE FROM {webmail_plus_user_threads_map} WHERE uid=%d AND folder='%s'", $user->uid, $folder);


  //webmail_plus_dump($thread_email);

  if(sizeof($thread_email)<=0) return;

  // now insert this all into the tables
  foreach($thread_email as $thread_id=>$thread_emails) {

    //echo "thread $thread_id<br>";

    $min_message_time = 2147483647; // max 32 bit unix time, good till 2038
    $max_message_time = 0; // well d'oh
    $from_names = array();

    foreach($thread_emails as $email_id=>$email_header) {




      //drupal_set_message('inserting '.$thread_id.'/'.$user->uid.'/'.$folder.'/'.$email_header->uid);
      // map the message to a thread
      db_query("INSERT INTO {webmail_plus_user_threads_map}(tid,uid,folder,message_uid) VALUES(%d, %d, '%s', %d)", $thread_id, $user->uid, $folder, $email_header->uid);


      // collect all names
      $from_name=trim(_webmail_plus_extract_name($email_header->from));
      //echo "got $from_name<br>";

      $from_names[$from_name]=$from_name;

      // track the earliest message in the thread
      $email_time = strtotime($email_header->date);
      if($email_time<$min_message_time) {
        $min_message_time = $email_time;
        $min_message_uid = $email_header->uid;
        $min_message_from = $email_header->from;
        $min_message_subject = $email_header->subject;
      }

      // track the oldest message, since we need to sort by it
      if($email_time>$max_message_time) {
        $max_message_time = $email_time;
      }

    }


    // FIXME these arent set yet, need to figure this out
    $thread_recent = 0;
    $thread_flagged = 0;
    $thread_answered = 0;
    $thread_deleted = 0;
    $thread_seen = 0;
    $thread_draft = 0;


    //print_r($from_names);

    $from_list = implode(", ", $from_names);

    // insert a message into a thread
    db_query("INSERT INTO {webmail_plus_user_threads}(`tid`, `uid`, `folder`, `from`, `subject`, `date`, `attachments`, `message_count`, `recent`, `flagged`, `answered`, `deleted`, `seen`, `draft`) VALUES(%d, %d, '%s', '%s', '%s', %d, %d, %d, %d, %d, %d, %d, %d, %d)", $thread_id, $user->uid, $folder, $from_list, $min_message_subject, $max_message_time, 0, sizeof($thread_emails), $thread_recent, $thread_flagged, $thread_answered, $thread_deleted, $thread_seen, $thread_draft);



  }

  //webmail_plus_dump($thread_email);

}

?>