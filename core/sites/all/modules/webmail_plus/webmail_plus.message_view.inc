<?php
function webmail_plus_message_view_form() {
	
  /*
  $newkey = webmail_plus_crypt_make_key();
  echo "newkey: $newkey<br>\n";
  
  $key =  webmail_plus_crypt_get_key();
  echo "key: $key<br>\n";
  
  
  $encrypted = webmail_plus_crypt_encrypt('te123st');
  echo "encrypted: $encrypted<br>\n";
  
  $decrypted =  webmail_plus_crypt_decrypt($encrypted);
  echo "decrypted: $decrypted<br>\n";
  
  */

  
  require_once "webmail_plus.functions.inc";
  require_once 'webmail_plus.tag_functions.inc';

  //module_load_include('inc', 'webmail_plus', 'functions');  
  //module_load_include('inc', 'webmail_plus', 'tag_functions');   
   
   //wpd(_webmail_plus_get_user_folders());
     
   global $user, $_webmail_plus_config;
   drupal_add_js(drupal_get_path('module', 'webmail_plus') .'/js/webmail_plus.js');
   drupal_add_css(drupal_get_path('module', 'webmail_plus').'/css/webmail_plus_print.css', 'module', 'print');
   

   $message_id = arg(2);
   

   //list($uid, $message_folder, $message_uid) = split("-", $message_id);
   $message_info = webmail_plus_parse_message_key($message_id);


   $mail_api_connection = _webmail_plus_connect($message_info->folder);
   //mail_api_connect($webmail_plus_config->domain, $user->name, $webmail_plus_config->password, $message_folder);

   $message_header_raw = mail_api_header($message_info->message_uid);
   $message_header = webmail_plus_parse_header($message_header_raw, TRUE);
   $body = mail_api_get_plaintext($message_info->message_uid);
   if($body=="") {
     $body = mail_api_body($message_info->message_uid);
   } 
   $attachments = mail_api_get_attachment_files($message_info->message_uid);
   
   drupal_set_title(_webmail_plus_render_subject($message_header['subject']));


   // Action buttons on the bottom of the message
   $form['top_buttons'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<div class="container-inline">', 
    '#suffix' => '</div>',
    '#attributes' => array(
      'id' => 'webmail_plus_top_buttons'
      )
   );

   $form['top_buttons']['reply'] = array(
    '#type' => 'submit',
    '#id' => 'reply',
    '#value' => t('Reply')
   );

   $form['top_buttons']['forward'] = array(
    '#type' => 'submit',
    '#id' => 'forward',
    '#value' => t('Forward')
   );
   
   
  // action drop-down
  // these should always be present
  $options['reply_all'] = t('Reply All');
  //$options['forward'] = t('Forward');
  $options['mark_read'] = t('Mark Read');
  $options['mark_unread'] = t('Mark Unread');

  $mailboxes = _webmail_plus_get_user_folders();
  if (is_array($mailboxes)) {
    foreach($mailboxes as $mailbox_id => $mailbox_name) {

      // if for some reason the mailbox is empty, skip it
      if ($mailbox_name=="") continue;

      // do not show the current mailbox as an option
      if ($mailbox_name==$folder) continue;
      $options['move_to_'.$mailbox_name] = t('Move to').' '.$mailbox_name;
    }
  }
     
  $user_tags = webmail_plus_get_user_tags(NULL, $_webmail_plus_config -> max_tags_in_dropdown);

  if (sizeof($user_tags)>0) {
    foreach($user_tags as $tag_id=>$tag) {
      $options['tag_with_'.$tag] = t('Tag with').' '.$tag;
    }
  }

  $publishable_nodes =  _webmail_plus_publishable_nodes();
  if (sizeof($publishable_nodes)>0) {
    foreach($publishable_nodes as $node_id=>$node) {
      $options['publish_as_'.$node_id] = t('Publish as').' '.$node;
    }
  }  
  
  
   $form['top_buttons']['top_operation'] = array(
	'#type' => 'select',
	'#options' => $options,
	'#default_value' => 'suspend',
    '#id' => 'top_operation' 
   );  
  
   $form['top_buttons']['top_update'] = array(
     '#type' => 'submit',
   	 '#value' => t('OK'),
     '#id' => 'top_update'
   );
     
  
  $form['top_buttons']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#id' => 'delete'
  );

  
  
  if(_webmail_plus_get_newer_message(strtotime($message_header['date']),$message_info->folder)) {
	  $form['top_buttons']['newer'] = array(
	    '#type' => 'submit',
	    '#value' => '< '.t('Newer'),
	    '#id' => 'newer'
	  );
  }
  
  if(_webmail_plus_get_older_message(strtotime($message_header['date']),$message_info->folder)) {
	  $form['top_buttons']['older'] = array(
	    '#type' => 'submit',
	    '#value' => t('Older').' >',
	    '#id' => 'older'
	  );
  }
  

  
  
  if($message_id) {
    $form['message_id'] = array(
   	  '#type' => 'hidden',
      '#id' => 'message_id',
      '#value' => $message_id,
   );    
  }

  

   $from_parsed = _webmail_plus_parse_email($message_header['from']);
  
   //wpd(_webmail_plus_extract_email($message_header['from']));
   
   $from_address_book = _webmail_plus_add_to_address_book($message_header['from']);
      
   $form['message_header']['from'] = array(
     '#title' => t('From'),
     '#value' => $message_header['from'],
     '#prefix' => '<div class="from container-inline"><strong>From:</strong> ',
     '#suffix' => ' '.$from_address_book.' </div>'
   );
   

   $form['message_header']['to'] = array(
     '#title' => t('To'),
     '#value' => _webmail_plus_present_email_list($message_header['to']),
     '#prefix' => '<div class="container-inline"><strong>'.t('To').':</strong> ',
     '#suffix' => '</div>'
   );
   
      
   if($message_header['cc']) {
     $form['message_header']['cc'] = array(
       '#title' => t('CC'),
       '#value' => _webmail_plus_present_email_list($message_header['cc']),
       '#prefix' => '<div class="container-inline"><strong>'.t('CC').':</strong> ',
       '#suffix' => '</div>'
     );
   }

   if($message_header['bcc']) {
     $form['message_header']['bcc'] = array(
       '#title' => t('CC'),
       '#value' => _webmail_plus_present_email_list($message_header['bcc']),
       '#prefix' => '<div class="container-inline"><strong>'.t('BCC').':</strong> ',
       '#suffix' => '</div>'
     );
   }   
   
   $form['message_header']['date'] = array(
     '#value' => format_date(strtotime($message_header['date']), 'custom', 'M j, Y g:i A'),
     '#prefix' => '<div class="container-inline"><strong>'.t('Date').':</strong> ',
     '#suffix' => '</div>'
   );

   $form['message_header']['header_buttons'] = array(
   	'#type' => 'item',
    '#value' => '<div class="hideshow-buttons"><a href="message-header">'.t('Show Full Header').'</a></div>',
    '#attributes' => array(
      'class' => 'webmail_plus_header_buttons'
    ),

   );

   $form['message_header']['full_header']['header'] = array(
	 '#type'	=> 'item',
     '#value' => htmlentities($message_header_raw),
     '#prefix' => '<div class="message-header hide-form"><pre>',
     '#suffix' => '</pre></div>',
   );


   


  // $form['message_body'] = array(
  // 	'#type' => 'fieldset',
  //  '#title' => t('Message Body'),
  //  '#collapsible' => TRUE,
  //  '#collapsed' => FALSE,
  // );

   $form['message_body']['body'] = array(
	  '#type' => 'item',
     '#value' => preg_replace("/\n/", "<br>\n", $body),
 //    '#prefix' => '<pre>',
 //    '#suffix' => '</pre>',
   );

   
   

  
   if(!empty($attachments)) {
     $form['attachments'] = array(
       '#type' => 'fieldset',
	   '#attributes' => array('class' => 'attachments'),
       '#title' => t('Attachments'),
       '#collapsible' => FALSE,
       '#collapsed' => FALSE,
     );
   	
     // put this into forms
	     foreach($attachments as $attachment_id=>$attachment_info) {
		    if(!empty($attachment_info['file'])) {
		      $form['attachments'][$attachment_id] = array(
		        '#type' => 'checkbox',
		        '#title' => l($attachment_info['file'], 'webmail_plus/download_attachment/'.$message_id.'/'.$attachment_info['part_number']),
		      );
			   
		    }
	     }

   }
   
   
   // user settings
   $form['message_tags'] = array(
   	'#type' => 'fieldset',
    '#title' => t('Message Tags'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#attributes' => array(
      'id' => 'webmail_plus_message_tags'
      )
   );

   // get tags assigned to this message
   $message_tags_array = _webmail_plus_get_message_tags($message_id);
   //wpd($message_tags_array);



   if(sizeof($message_tags_array)>0) {
     $message_tags = implode(" ", $message_tags_array);

   } else {
     $message_tags = "";
   }

   $form['message_tags']['tags'] = array( 
     '#type' => 'textfield',
     '#default_value' => $message_tags,
  	 '#prefix' => '<div class="container-inline">',
     '#autocomplete_path' => 'webmail_plus/tag_list_autocomplete',

   );

   $form['message_tags']['folder'] = array(
     '#type' => 'hidden',
     '#default_value' => $message_info->folder,
   );

   $form['message_tags']['message_uid'] = array(
     '#type' => 'hidden',
     '#default_value' => $message_info->message_uid,
     '#descrption' => t('Tags separated by space, comma or semicolon.')
   );

   $form['message_tags']['submit'] = array(
     '#type' => 'submit',
     '#value' => t('Update Tags'),
     '#weight' => 10,
     '#suffix' => '</div>',
     '#id' => 'update_tags'
   );


   // set message as seen
   webmail_plus_message_flag($message_info->folder, $message_info->message_uid, 'seen', 1);

   
   /*
   // Action buttons on the bottom of the message
   $form['bottom_buttons'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<div class="container-inline">', 
    '#suffix' => '</div>',
    '#attributes' => array(
      'id' => 'webmail_plus_bottom_buttons'
      )
   );

   $form['bottom_buttons']['reply'] = array(
    '#type' => 'submit',
    '#id' => 'reply',
    '#value' => t('Reply')
   );





   $form['bottom_buttons']['bottom_operation'] = array(
		'#type' => 'select',
		'#options' => $options,
		'#default_value' => 'suspend',
        '#id' => 'bottom_operation'
   );  
  
   $form['bottom_buttons']['bottom_update'] = array(
     '#type' => 'button',
   	 '#value' => t('Update'),
     '#attributes' => array(
      'id' => 'bottom_update',
      'onclick' => 'this.form.submit()'
     )
   );  
  
  $form['bottom_buttons']['publish_txt'] = array(

    '#value' => t('Publish as '),
    '#id' => 'publish_text'
  );


  //wpd($_webmail_plus_config->allowed_publish_nodes);
  // create the correct publish array      

  if(sizeof($_webmail_plus_config->allowed_publish_nodes)>0) { 
	  $available_nodes_raw = node_get_types(); 
	  foreach($_webmail_plus_config->allowed_publish_nodes as $node_type=>$junk) {
	  	$publish_options[$node_type]=$available_nodes_raw[$node_type]->name;
	  }

    $form['bottom_buttons']['publish_as'] = array(
      '#type' => 'select',
      '#options' => _webmail_plus_publishable_nodes()
     );   

	  $form['bottom_buttons']['publish'] = array(
	    '#type' => 'submit',
	    '#value' => t('Publish'),
	    '#id' => 'publish'
	  );  
  }

  
  $form['bottom_buttons']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#id' => 'delete'
  );
  
  if(_webmail_plus_get_newer_message(strtotime($message_header['date']),$message_info->folder)) {
	  $form['bottom_buttons']['newer'] = array(
	    '#type' => 'submit',
	    '#value' => t('Newer'),
	    '#id' => 'newer'
	  );
  }
  
  if(_webmail_plus_get_older_message(strtotime($message_header['date']),$message_info->folder)) {
      //drupal_set_message('older');
	  $form['bottom_buttons']['older'] = array(
	    '#type' => 'submit',
	    '#value' => t('Older'),
	    '#id' => 'older'
	  );
  }
  */
   
   // if this form is submitted it needs to be redirected
   $form_state['redirect'] = 'webmail_plus/message_view/'.$user->uid.'-'.$message_folder.'-'.$message_uid;

   return $form;

}


function theme_webmail_plus_message_view_form($form) {
  //wpd($form);
  return drupal_render($form);
}


function webmail_plus_message_view_form_validate($form, &$form_state) {

}


function webmail_plus_message_view_form_submit($form, &$form_state) {
  global $user, $_webmail_plus_config;

  

  //module_load_include('inc', 'webmail_plus', 'functions');  
  //module_load_include('inc', 'webmail_plus', 'tag_functions');
    
  require_once 'webmail_plus.functions.inc';
  require_once 'webmail_plus.tag_functions.inc';
  
  $message_parts = webmail_plus_parse_message_key($form_state['values']['message_id']);
      
  // redirects user to a newer message
  if($form_state['clicked_button']['#id']=='older') {

    $this_message_timestamp = db_result(db_query("SELECT `date` FROM {webmail_plus_user_headers} WHERE uid=%d AND `message_key`='%s' LIMIT 1", $user->uid, $form_state['values']['message_id']));
    $older_message_key = _webmail_plus_get_older_message($this_message_timestamp, $message_parts->folder);
    $form_state['redirect']='webmail_plus/message_view/'.$older_message_key;
  }
    
  // redirects user to a newer message
  if($form_state['clicked_button']['#id']=='newer') {
  	$this_message_timestamp = db_result(db_query("SELECT `date` FROM {webmail_plus_user_headers} WHERE uid=%d AND `message_key`='%s' LIMIT 1", $user->uid, $form_state['values']['message_id']));
  	$newer_message_key = _webmail_plus_get_newer_message($this_message_timestamp, $message_parts->folder);
  	$form_state['redirect']='webmail_plus/message_view/'.$newer_message_key;
  }
  
  if($form_state['clicked_button']['#id']=='delete') {
    _webmail_plus_flush_tags($form_state['values']['message_id']);  // remove tags associated with this message
  	_webmail_plus_move_message($form_state['values']['message_id'], mail_api_trash_path($_webmail_plus_config->domain));
  	$form_state['redirect']='webmail_plus/gateway';
  }

  if($form_state['clicked_button']['#id']=='reply') {
    $form_state['redirect']='webmail_plus/compose/reply/'.$form_state['values']['message_id'];
  }


  if($form_state['clicked_button']['#id']=='forward') {
    $form_state['redirect']='webmail_plus/compose/forward/'.$form_state['values']['message_id'];
  }


  

  
  // decide which button was clicked
  if($form_state['clicked_button']['#id']=='top_update') {
    $operation = $form_state['values']['top_operation'];
  }
  elseif($form_state['clicked_button']['#id']=='bottom_update') {
    $operation = $form_state['values']['bottom_operation'];
  }

  // we will need these values
  if(preg_match("/move_to_/", $operation) || preg_match("/tag_with_/", $operation)) {
    $message_parts = webmail_plus_parse_message_key($form_state['values']['message_id']);
  }
    
  if($operation=='reply_all') {
    $form_state['redirect']='webmail_plus/compose/replyall/'.$form_state['values']['message_id'];
  }
  elseif($operation=='forward') {
    $form_state['redirect']='webmail_plus/compose/forward/'.$form_state['values']['message_id'];
  }
  elseif($operation=='mark_read') {
   $form_state['redirect']='webmail_plus/gateway';
  }
  elseif($operation=='mark_unread') {
    _webmail_plus_connect($message_parts->folder);
    mail_api_flag_unread($message_parts->message_uid);
    $form_state['redirect']='webmail_plus/gateway';
  }
  elseif(preg_match("/move_to_/", $operation)) {
    $rs = preg_match("/move_to_(.*)/", $operation, $matches);
	$destination = $matches[1];
	//wpd($destination);
	drupal_set_message('moving message '.$form_state['values']['message_id'].' to '.$destination);
    _webmail_plus_connect($message_parts->folder);
	_webmail_plus_move_message($form_state['values']['message_id'], $destination);
    $form_state['redirect']='webmail_plus/gateway';
  }
  elseif(preg_match("/tag_with_/", $operation)) {
    $rs = preg_match("/tag_with_(.*)/", $operation, $matches);
	$tag = $matches[1];
	_webmail_plus_add_tag($form_state['values']['message_id'], $tag);
  }
  elseif(preg_match("/publish_as_/", $operation)) { 
    $rs = preg_match("/publish_as_(.*)/", $operation, $matches);
    $node_type = $matches[1];
    
    //drupal_set_message('publishing '.$message_parts->folder.'/'.$message_parts->message_uid.' as '.$node_type);
    unset($_SESSION['file_attach_files']);
    $form_state['redirect']='node/add/'.$node_type.'/webmail_plus/'.$form_state['values']['message_id'];
    //drupal_goto('node/add/'.$node_type.'/webmail_plus/'.$form_state['values']['message_id']);
  }
  
  
  if($form_state['clicked_button']['#id']=='update_tags' ) {
    _webmail_plus_flush_tags($form_state['values']['message_id']);
    
    if($form_state['values']['tags']) {
      $tags_array = preg_split("/\s|,|;/", $form_state['values']['tags']);
      $tags_array = array_unique($tags_array);   

      foreach($tags_array as $index=>$tag) {
        $tag=trim($tag);
        if($tag=="") continue;
     	_webmail_plus_add_tag($form_state['values']['message_id'], $tag);
      }
    }
  }
    
}


/*
 * returns an array of nodes that a user can publish as
 */
function _webmail_plus_publishable_nodes() {
	global $_webmail_plus_config;
	
	//wpd($_webmail_plus_config->allowed_publish_nodes);
  $available_nodes_raw = node_get_types(); 
  
  foreach($_webmail_plus_config->allowed_publish_nodes as $node_type=>$node_value) {
  	if($node_value=='0') continue;
  	
  	if($available_nodes_raw[$node_type])  $publish_options[$node_type]=$available_nodes_raw[$node_type]->name;
  }

  
  return $publish_options;
}


