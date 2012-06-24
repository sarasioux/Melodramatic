<?php
/**
 * this file is here for testing purposes only!!!
 *
 * @return unknown
 */

function webmail_plus_message_view() {
  return drupal_get_form('webmail_plus_message_view_form');
}

function webmail_plus_message_view_form() {
   require_once "webmail_plus.functions.inc";
   require_once 'webmail_plus.tag_functions.inc';

   global $user, $_webmail_plus_config;



   $message_id = arg(2);

   //list($uid, $message_folder, $message_uid) = split("-", $message_id);
   $message_info = webmail_plus_parse_message_key($message_id);

   //print_r($message_info);

   $mail_api_connection = webmail_plus_connect($message_info->folder);
   //mail_api_connect($webmail_plus_config->domain, $user->name, $webmail_plus_config->password, $message_folder);

   $message_header_raw = mail_api_header($message_info->message_uid);
   $message_header = webmail_plus_parse_header($message_header_raw, TRUE);

   $message_body = mail_api_body($message_info->message_uid);
   


   //$structure = mail_api_get_structure($message_info->message_uid);
   //wpd($structure);
   
   //$map = mail_api_get_map($message_info->message_uid);
   //wpd($map);   
   
   $plaintext = mail_api_get_plaintext($message_info->message_uid);
   wpd($plaintext); 
   
   $attachments = imapwu_api_get_attachment_files($message_info->message_uid);
   wpd($attachments);
}

function theme_webmail_plus_message_view($form) {

  $content = form_render($form['top_buttons']);
  unset($form['top_buttons']);

  $content = form_render($form['message_header']);
  unset($form['message_header']);

  $content = form_render($form['message_body']['body']);
  unset($form['message_header']['body']);

  return $content;
}


function webmail_plus_message_view_form_submit($form, &$form_state) {
  global $user;



  if($form_state['clicked_button']['#id']=='reply') {
    drupal_goto('webmail_plus/compose/reply/'.$form_state['values']['message_id']);
  }

  if($form_state['clicked_button']['#id']=='replyall') {
    drupal_goto('webmail_plus/compose/replyall/'.$form_state['values']['message_id']);
  }

  if($form_state['clicked_button']['#id']=='forward') {
    drupal_goto('webmail_plus/compose/forward/'.$form_state['values']['message_id']);
  }

  // if tags are empty dont do anything
  if($form_state['values']['tags']=="") return;

  require_once 'webmail_plus.tag_functions.inc';

  $tags = trim($form_state['values']['tags']);

  // slipt tags by comma, semicolon or space
  $tags_array = preg_split("/\s|,|;/", $tags);
  $tags_array = array_unique($tags_array);

  // handle tags if there are any
  if(sizeof($tags_array)>0) {
    //drupal_set_message('tags: '.$tags);


    // remove all tags
    webmail_plus_flush_tags($form_state['values']['folder'], $form_state['values']['message_uid']);

    // assign new tags
    foreach($tags_array as $index=>$tag) {
      $tag = strtolower(trim($tag));
      if($tag=="") continue;

      //drupal_set_message('adding tag '.$tag.' to the message '.$form_state['values']['message_uid']);

      // add new tag
      webmail_plus_add_tag($form_state['values']['folder'], $form_state['values']['message_uid'], $tag);

    }
  }
}
