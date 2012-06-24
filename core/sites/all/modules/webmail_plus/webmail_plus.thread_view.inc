<?php
function webmail_plus_thread_view() {
  return drupal_get_form('webmail_plus_thread_view_form');
}

function webmail_plus_thread_view_form() {
  require_once "webmail_plus.functions.inc";
  require_once 'webmail_plus.tag_functions.inc';

  global $user, $webmail_plus_config;

  $form['top_buttons'] = array(
    '#type' => 'item',
    '#prefix' => '<div class="container-inline">', '#suffix' => '</div>'
  );

  $form['top_buttons']['reply'] = array(
   '#type' => 'submit',
   '#value' => t('Reply'),
   '#id' => 'reply'
  );

  $form['top_buttons']['replyall'] = array(
   '#type' => 'submit',
   '#value' => t('Reply All'),
   '#id' => 'replyall'
  );


  $form['top_buttons']['forward'] = array(
    '#type' => 'submit',
    '#value' => t('Forward'),
    '#id' => 'forward'
  );

  $form['top_buttons']['delete'] = array(
   '#type' => 'submit',
   '#value' => t('Delete'),
   '#id' => 'delete'
  );




  $thread_id = arg(2);
  $thread_info = webmail_plus_parse_thread_key($thread_id);

  //print_r($thread_info);

  $mail_api_connection = _webmail_plus_connect($thread_info->folder);


  // FIXME if there are no messages in the thread (for some crazy reason) we need to show an error

  $thread_messages = db_query("SELECT * FROM {webmail_plus_user_threads_map} WHERE tid=%d AND uid=%d AND folder='%s'", $thread_info->thread_id, $user->uid, $thread_info->folder);

  $message_count = db_affected_rows();

  //drupal_set_message('found '.$message_count);


  $thread_tags = array();

  while($thread_message = db_fetch_object($thread_messages)) {


    //wpd($thread_message);
    
    $message_index++;
    //webmail_plus_dump($thread_message);
    $message_header_raw = mail_api_header($thread_message->message_uid);
    //wpd($message_header_raw);
    
    $message_header_arr = webmail_plus_parse_header($message_header_raw, TRUE);


    if($message_index==1) {
      drupal_set_title(_webmail_plus_render_subject($message_header_arr['subject']));

    }

    if($message_index==$message_count) {
      $collapsed = FALSE;
    }
    else {
      $collapsed = TRUE;
    }

    // create a fieldset
    $form['message_'.$thread_message->message_uid] = array(
      '#type' => 'fieldset',
      '#title' => trim(_webmail_plus_extract_name($message_header_arr['from'])),
      '#collapsible' => TRUE,
      '#collapsed' => $collapsed,
    );

    $body = "";
    $body = "<strong>".t('From').":</strong> ".$message_header_arr['from']."\n";
    if($message_header_arr['cc']) {
      $body = "<strong>".t('Cc').":</strong> ".$message_header_arr['cc']."\n";
    }
    $body .= "<strong>".t('Date').":</strong> ".$message_header_arr['date']."\n";
    $body .= "<br>\n";
    
    // fetch message body
    $message_body = mail_api_get_plaintext($thread_message->message_uid);
    if($message_body=="") {
      $message_body = mail_api_body($thread_message->message_uid);
    } 
   
    $body .= $message_body;


    $form['message_'.$thread_message->message_uid]['body'] = array(
     '#value' => $body,
     '#prefix' => '<pre>',
     '#suffix' => '</pre>',
   );

   // collect the tags for the tread
   $message_tags_array = _webmail_plus_get_message_tags($user->uid.'!'.$thread_info->folder.'@'.$thread_message->message_uid);

   
   if(is_array($thread_tags) && is_array($message_tags_array)) {
    $thread_tags = array_merge($thread_tags, $message_tags_array);
   }
   elseif (is_array($message_tags_array)) {
     $thread_tags = $message_tags_array;
   }


  }
  
  
   if(sizeof($thread_tags)>0) {
    $all_thread_tags = implode(" ", array_unique($thread_tags));
   }
   else {
     $all_thread_tags = "";
   }

   


   // user settings
   $form['message_tags'] = array(
    '#type' => 'fieldset',
    '#title' => t('Message Tags'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE
   );


   
   $form['message_tags']['tags'] = array(
     '#type' => 'textfield',
     '#default_value' => $all_thread_tags,
     '#prefix' => '<div class="container-inline">',
     '#autocomplete_path' => 'webmail_plus/tag_list_autocomplete',
   );



   $form['message_tags']['submit'] = array(
     '#type' => 'submit',
     '#value' => t('Update Tags'),
     '#weight' => 10,
     '#suffix' => '</div>'
   );


   $form['message_tags']['thread_id'] = array(
     '#type' => 'hidden',
     '#default_value' => $thread_id,
   );
  //drupal_set_message('folder '.$thread_info->folder.' thread '.$thread_info->thread_id);

  webmail_plus_thread_flag($thread_info->folder, $thread_info->thread_id, 'seen', 1);


  return $form;

}


/*
function theme_webmail_plus_thread_view($form) {



  $content = form_render($form['message_header']);
  unset($form['message_header']);

  $content = form_render($form['message_body']['body']);
  unset($form['message_header']['body']);

  $content = form_render($form['message_body']['body']);
  
  return $content;
}
*/


function webmail_plus_thread_view_form_submit($form, &$form_state) {
  global $user;

  //print_r($form_state);

  if($form_state['clicked_button']['#id']=='reply') {
    $form_state['redirect']='webmail_plus/compose/reply/'.$form_state['values']['thread_id'];
  }

  if($form_state['clicked_button']['#id']=='replyall') {
    $form_state['redirect']='webmail_plus/compose/replyall/'.$form_state['values']['thread_id'];
  }

  if($form_state['clicked_button']['#id']=='forward') {
    $form_state['redirect']='webmail_plus/compose/forward/'.$form_state['values']['thread_id'];
  }

  require_once 'webmail_plus.tag_functions.inc';

  $tags = trim($form_state['values']['tags']);



  // slipt tags by comma, semicolon or space
  $tags_array = preg_split("/\s|,|;/", $tags);
  $tags_array = array_unique($tags_array);

  //print_r($tags_array);
  //echo "size ".sizeof($tags_array);


  //list($uid, $thread_folder, $tid) = split("-", $form_state['values']['thread_id']);
  $thread_info = webmail_plus_parse_thread_key($form_state['values']['thread_id']);

  // now tag all messages
  $thread_messages = db_query("SELECT * FROM {webmail_plus_user_threads_map} WHERE tid=%d AND uid=%d AND folder='%s'", $thread_info->thread_id, $thread_info->uid, $thread_info->folder);


  while($thread_message = db_fetch_object($thread_messages)) {
    //webmail_plus_dump($thread_message);

    // remove all tags from the message
    _webmail_plus_flush_tags($user->uid."!".$thread_info->folder."@".$thread_message->message_uid);



    if($tags=="") continue;

    foreach($tags_array as $index=>$tag) {
      $tag = strtolower(trim($tag));
      
      //echo "tag: $tag<br>\n";
      
      if($tag=="") continue;

      //echo "adding tag $tag to $thread_info->folder@$thread_message->message_uid";
      
      _webmail_plus_add_tag($user->uid."!".$thread_info->folder."@".$thread_message->message_uid, $tag);

    }

  }



}


