<?
function webmail_plus_solr_cron() {
  
  
	$_debug = variable_get('webmail_plus_solr_debug', FALSE);
	
	

	
	// clear everything
	
	db_query("TRUNCATE TABLE {webmail_plus_solr_history}");
	db_query("TRUNCATE TABLE {webmail_plus_solr_map}");
	db_query("TRUNCATE TABLE {webmail_plus_solr_results}");
	db_query("TRUNCATE TABLE {webmail_plus_solr_results_headers}");

	
	// fetch solr config
	$solr_url = variable_get('webmail_plus_solr_url', NULL);
	//cho $solr_url."<br>\n";
	
	$solr_url_parts = parse_url($solr_url);  
	$solr_batch_size = variable_get('webmail_plus_solr_batch_size', 10);
	if($solr_batch_size<=10) $solr_batch_size=10;  
	
	if($_debug) echo "batch size: $solr_batch_size<br>\n";
	
	$solr_ignore_folders = variable_get('webmail_plus_solr_ignore_folders', NULL);
	$solr_ignore_folders_arr = explode(',', $solr_ignore_folders);
	foreach($solr_ignore_folders_arr as $key=>$value) {
		$solr_folder_blacklist[trim($value)]=trim($value);
	}

	
	
    try {
      $solr = apachesolr_get_solr($solr_url_parts['host'], $solr_url_parts['port'], $solr_url_parts['path']);
      //$solr->clearCache();
      
      $ping = @$solr->ping();
      // If there is no $solr object, there is no server available, so don't continue.
      if (!$ping) {
        throw new Exception(t('No Solr instance available when checking requirements.'));
      }
    }
    catch (Exception $e) {
      watchdog('Apache Solr', $e->getMessage(), NULL, WATCHDOG_ERROR);
    }

    $queue = db_query("SELECT * FROM {webmail_plus_solr_queue} LIMIT %d", $solr_batch_size);
    
    while($queue_item = db_fetch_object($queue)) {

      
      // load user information, use cache to speed things up
      if($cached_users[$queue_item->uid]) {
        $user_name = $cached_users[$queue_item->uid]['name'];
        $user_password = $cached_users[$queue_item->uid]['password'];
      }
      else {
    	  $user_node = user_load($queue_item->uid);
    	  $user_password = _webmail_plus_get_user_password($user);

    	  $cached_users[$queue_item->uid] = array(
    	    'name' => $user_node-> name,
    	    'password' => $user_password
    	  );
      }
      
	  
	  
      $message_key_parts = webmail_plus_parse_message_key($queue_item->message_key);
      
      // if the folder is in the backlist, simply remove the action, do not index it
	  if($solr_folder_blacklist[$message_key_parts->folder]) {
	    _webmail_plus_solr_delete_queue($queue_item->uid, $queue_item->action, $queue_item->message_key);
	    continue;
	  }

	  // if the message is already in the map, skip it
	  if(_webmail_plus_solr_check_map($queue_item->message_key, $user)) {
	    _webmail_plus_solr_delete_queue($queue_item->uid, $queue_item->action, $queue_item->message_key);
	    continue;
	  }
	  
      // queue up additions
      if($queue_item->action=='ADD') {
    	  $mail_api_connection = webmail_plus_solr_connect($user_name, $user_password, $message_key_parts->folder);
    	  $message_header = webmail_plus_parse_header(mail_api_header($message_key_parts->message_uid), TRUE);
    	  $message_plaintext = mail_api_get_plaintext($message_key_parts->message_uid);
    	  $message_attachments = mail_api_get_attachment_files($message_key_parts->message_uid);	  


    	  unset($document);
    	  $document = new Apache_Solr_Document();
    	  $document->id = $queue_item->message_key;
    	  $document->drupal_uid = $queue_item->uid;
    	  $document->msg_from = apachesolr_strip_ctl_chars(trim($message_header['from']));
    	  $document->msg_to = apachesolr_strip_ctl_chars(trim($message_header['to']));
    	  $document->msg_cc = apachesolr_strip_ctl_chars(trim($message_header['cc']));
    	  $document->msg_bcc = apachesolr_strip_ctl_chars(trim($message_header['bcc']));
    	  $document->msg_subject = apachesolr_strip_ctl_chars(trim($message_header['subject']));
    	  $document->msg_date = apachesolr_strip_ctl_chars(date("Y-m-d\TH:i:s\Z", strtotime($message_header['date'])));
    	  $document->msg_plain = apachesolr_strip_ctl_chars(trim($message_plaintext));
    	  $document->msg_body = apachesolr_strip_ctl_chars(trim($message_plaintext));
    	  $document->msg_num_attach = $msg_num_attach;
    	  $document->msg_attach_filenames = $msg_attach_filenames;  

    	  $docs_to_add[]=$document;
    	      	  
          /*
    	  $document = new Apache_Solr_Document();
    	  $document -> id = $queue_item->message_key;
    	  $document -> body = apachesolr_strip_ctl_chars(trim($message_plaintext));

    	  */
      }   
      
      // queue up deletions
      if($queue_item->action=='DELETE') {
        $docs_to_delete[]=$queue_item->message_key;
      }
      
      
      _webmail_plus_solr_delete_queue($queue_item->uid, $queue_item->action, $queue_item->message_key);
           
    }
    
    
    if($_debug) echo wpd($docs_to_add);
    
    // now submit things to solr
    if (is_object($solr) && count($docs_to_add)) {
      //wpd($docs_to_add);
      
      // add the documents
      try {
        watchdog('Webmail Plus Solr', 'Adding @count documents.', array('@count' => count($docs_to_add)));
        $solr->addDocuments($docs_to_add);
      }
      catch (Exception $e) {
        watchdog('Webmail Plus Solr', $e->getMessage(), NULL, WATCHDOG_ERROR);
      }
      

    }
    
    if (is_object($solr) && count($docs_to_delete)) {
      // remove the others
      foreach($docs_to_delete as $index=>$message_key) {
        $solr->deleteById($message_key);
      }
    }
    
    if(count($docs_to_add) || count($docs_to_delete)) {
      $solr->commit();
      $solr->optimize(FALSE, FALSE);
    }
      
}

// backported this from apachesolr-6.x-1.0-beta6
if(!function_exists('apachesolr_strip_ctl_chars')) {
  function apachesolr_strip_ctl_chars($text) {
    // See:  http://w3.org/International/questions/qa-forms-utf-8.html
    // Printable utf-8 does not include any of these chars below x7F
    return preg_replace('@[\x00-\x08\x0B\x0C\x0E-\x1F]@', ' ', $text);
  }
}

// backported this from apachesolr-6.x-1.0-beta6
if(!function_exists(apachesolr_clean_text)) {
  function apachesolr_clean_text($text) {
    return strip_tags(preg_replace('@[\x00-\x08\x0B\x0C\x0E-\x1F]@', ' ', $text));
  }
}
?>