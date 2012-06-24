<?php
// $Id: webmail_plus.view_folder.inc,v 1.1.2.10 2009/05/14 18:30:30 firmidea Exp $

function webmail_plus_view_folder($folder = 'INBOX') {
  return drupal_get_form('webmail_plus_view_folder_form');
}

function webmail_plus_view_folder_form() {
  global $user, $_webmail_plus_config;

  _webmail_plus_check_connection();


  
  require_once 'webmail_plus.user.inc';
  require_once 'webmail_plus.functions.inc';
  require_once 'webmail_plus.tag_functions.inc';

  
  if($_GET['list']) {

    $tags=$_GET['tags'];
    $tags_array = explode(",", $tags);
    $tags_clean = implode(", ", $tags_array);
    
    $list=$_GET['list'];    
    $list_array = explode(",", $list);
    $list_clean = implode(", ", $list_array);
    $list_fixed = "'".implode("', '", $list_array)."'";
    drupal_set_title(t('All messages tagged with %tag', array('%tag'=>$tags_clean)));
    
  }
  elseif (arg(2)) {
    $folder  = arg(2);
    drupal_set_title(t('Folder: %folder', array('%folder'=>$folder)));
    
  }
  else {
    $folder = $webmail_plus_config->inbox_folder;
    drupal_set_title(t('Folder: %folder', array('%folder'=>$folder)));
    
  }


  $form['action'] = array(
    '#type' => 'item',
    '#prefix' => '<div class="container-inline actions">', '#suffix' => '</div>'
  );

  $form['action']['folder'] = array(
    '#type' => 'hidden',
    '#value' => $folder
  );


  $form['action']['delete'] = array(
    '#type' => 'submit',
    '#title' => t('Delete'),
    '#value' => t('Delete'),
    '#id' => 'delete'
  );

   // these should always be present
   $options['mark_read'] = t('Mark Read');
   $options['mark_unread'] = t('Mark Unread');

   // move to folder
   //$mail_api_connection = mail_api_connect($webmail_plus_config->domain, $user->name, $webmail_plus_config->password, "", $webmail_plus_config->options);

   $mail_api_connection = _webmail_plus_connect();

   //$mailboxes = mail_api_mailboxes();

   //print_r($mailboxes);

   
   //$mailboxes = __webmail_plus_resolve_folder_list(mail_api_mailboxes());
   
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

   // tag with

   $user_tags = webmail_plus_get_user_tags();

   if (sizeof($user_tags)>0) {
     foreach($user_tags as $tag_id=>$tag) {
       $options['tag_with_'.$tag] = t('Tag with').' '.$tag;
     }
   }

   $form['action']['operation'] = array(
		'#type' => 'select',
		'#options' => $options,
		'#default_value' => 'suspend'
   );

   $form['action']['update'] = array(
   		'#type' => 'submit',
   		'#value' => t('Update'),
      '#id' => 'update'
   );

   
   if($folder==mail_api_trash_path($_webmail_plus_config->domain)) {
	   $form['action']['empty_trash'] = array(
	      '#type' => 'submit',
	      '#value' => t('Empty Trash'),
	      '#id' => 'empty_trash'
	   );   	
   	
   	
   }

  if($folder=='Sent' || $folder=='Drafts') {
    
     $form['header'] = array('#type' => 'value', '#value' => array(
      theme('table_select_header_cell'),
      array('data' => t('To'), 'field' => '`to`'),
      array('data' => t('Subject'), 'field' => 'subject'),
      array('data' => t('Date'), 'field' => 'date'),
      array('data' => t('Attachments'), 'field' => 'attachments'),
    ));
  
    // this provides table sorting
    $header = array(
      array('data' => t('To'), 'field' => "`to`"),
      array('data' => t('Subject'), 'field' => '`subject`'),
      array('data' => t('Date'), 'field' => '`date`'),
      array('data' => t('Attachments'), 'field' => 'attachments')
      
    );   
  }
  elseif($list) {

    $form['header'] = array('#type' => 'value', '#value' => array(
      theme('table_select_header_cell'),
      array('data' => t('From/To'), 'field' => 'from'),
      array('data' => t('Folder'), 'field' => 'folder'),      
      array('data' => t('Subject'), 'field' => 'subject'),
      array('data' => t('Date'), 'field' => 'date'),
      array('data' => t('Attachments'), 'field' => 'attachments'),
      
    ));
  
    // this provides table sorting
    $header = array(
      array('data' => t('From/To'), 'field' => "`from`"),
      array('data' => t('Subject'), 'field' => '`subject`'),
      array('data' => t('Date'), 'field' => '`date`'),
      array('data' => t('Attachments'), 'field' => 'attachments')
      
    );

  }
  else {
     $form['header'] = array('#type' => 'value', '#value' => array(
      theme('table_select_header_cell'),
      array('data' => t('From'), 'field' => 'from'),
      array('data' => t('Subject'), 'field' => 'subject'),
      array('data' => t('Date'), 'field' => 'date'),
      array('data' => t('Attachments'), 'field' => 'attachments'),
      
    ));
  
    // this provides table sorting
    $header = array(
      array('data' => t('From'), 'field' => "`from`"),
      array('data' => t('Subject'), 'field' => '`subject`'),
      array('data' => t('Date'), 'field' => '`date`'),
      array('data' => t('Attachments'), 'field' => 'attachments')
      
    );
  }
  
  


  if($_GET['sort'] && $_GET['order']) {
    $new_tablesort = _webmail_plus_escape_reserved_db_words(tablesort_sql($header));
    $_SESSION['webmail_plus']['sort_order']=$new_tablesort;
    webmail_plus_set_user_preference('sort_order', $new_tablesort); 
  }
  else {
    if($_SESSION['webmail_plus']['sort_order']) {
      $new_tablesort = $_SESSION['webmail_plus']['sort_order'];
    }
    else {
      $new_tablesort = "ORDER BY `date` DESC";
      $_SESSION['webmail_plus']['sort_order']=$new_tablesort;
      webmail_plus_set_user_preference('sort_order', $new_tablesort); 
    }
  }
    

  
  if($list) {

    foreach($list_array as $index=>$message_key) {
      $sql = "SELECT * FROM {webmail_plus_user_headers} WHERE uid=".$user->uid." AND CONCAT(uid,'!',folder,'@',message_uid) IN(".$list_fixed.")";
    }

  }
  else {
    $sql = "SELECT * FROM {webmail_plus_user_headers} WHERE uid=".$user->uid." AND folder='".$folder."'";
  }

  //$result = pager_query($sql, 50);
  $result = pager_query($sql.' '.$new_tablesort, $_webmail_plus_config->messages_per_page);
  
  

  while($header = db_fetch_object($result)) {

    
    
   $folder = $header->folder;
   
   $key = $user->uid."!".$folder."@".$header->message_uid;
   //echo $key."<br>\n";

   $selected[$key] = '';
   
   if($list) {
   
    $form['tofrom'][$key] = array('#value' => l(_webmail_plus_combine_to_from($header->to, $header->from), 'webmail_plus/message_view/'.$key, array('html'=>TRUE)));
    $form['folder'][$key] = array('#value' => l($header->folder, 'webmail_plus/message_view/'.$key, array('html'=>TRUE)));
    $form['subject'][$key] = array('#value' => l(_webmail_plus_render_subject($header->subject), 'webmail_plus/message_view/'.$key, array('html'=>TRUE)));
    $form['date'][$key] = array('#value' => l(_webmail_plus_render_date($header->date), 'webmail_plus/message_view/'.$key, array('html'=>TRUE)));
    $form['attachments'][$key] = array('#value' => l($header->attachments, 'webmail_plus/message_view/'.$key, array('html'=>TRUE)));
      
   }
   elseif($folder==mail_api_sent_path($_webmail_plus_config->domain)) {

     
     $form['to'][$key] = array('#value' => l(_webmail_plus_render_from($header->to), 'webmail_plus/message_view/'.$key, array('html'=>TRUE)));
     $form['subject'][$key] = array('#value' => l(_webmail_plus_render_subject($header->subject), 'webmail_plus/message_view/'.$key, array('html'=>TRUE)));
     $form['date'][$key] = array('#value' => l(_webmail_plus_render_date($header->date), 'webmail_plus/message_view/'.$key, array('html'=>TRUE)));
     $form['attachments'][$key] = array('#value' => l($header->attachments, 'webmail_plus/message_view/'.$key, array('html'=>TRUE)));
   }
   elseif($folder==mail_api_drafts_path($_webmail_plus_config->domain)) {
     $form['to'][$key] = array('#value' => l(_webmail_plus_render_recepient($header->to), 'webmail_plus/compose/draft/'.$key, array('html'=>TRUE)));
     $form['subject'][$key] = array('#value' => l(_webmail_plus_render_subject($header->subject), 'webmail_plus/compose/draft/'.$key, array('html'=>TRUE)));
     $form['date'][$key] = array('#value' => l(_webmail_plus_render_date($header->date), 'webmail_plus/compose/draft/'.$key, array('html'=>TRUE)));
     $form['attachments'][$key] = array('#value' => l($header->attachments, 'webmail_plus/compose/draft/'.$key, array('html'=>TRUE)));     
   }

   else {
     $form['from'][$key] = array('#value' => l(_webmail_plus_render_from($header->from), 'webmail_plus/message_view/'.$key, array('html'=>TRUE)));
     $form['subject'][$key] = array('#value' => l(_webmail_plus_render_subject($header->subject), 'webmail_plus/message_view/'.$key, array('html'=>TRUE)));
     $form['date'][$key] = array('#value' => l(_webmail_plus_render_date($header->date), 'webmail_plus/message_view/'.$key, array('html'=>TRUE)));
     $form['attachments'][$key] = array('#value' => l($header->attachments, 'webmail_plus/message_view/'.$key, array('html'=>TRUE)));     
   }
      
   if($header->seen==1) {
     $form['seen'][$key] = array('#value' => 1);
   }
   else {
     $form['seen'][$key] = array('#value' => 0);
   }

  }

  $form['selected'] = array('#type' => 'checkboxes', '#options' => $selected);
  $form['pager'] = array('#value' => theme('pager', NULL, 50, 0));


  //print_r($form);

  return $form;
}


function theme_webmail_plus_view_folder_form($form) {

  $folder  = arg(2);
  

  //print_r($form);
 
  
  $output = drupal_render($form['action']);

  if (isset($form['date'])) {

     foreach (element_children($form['date']) as $key) {
       $row = array();


       //print_r($form['seen'][$key]);

       //print_r($form['unread'][$key]);
       if($form['seen'][$key]['#value']==1) {
         //drupal_set_message('webmail_plus_read_message');
         $class = "webmail_plus_read_message";
       }
       else {
         //drupal_set_message('webmail_plus_unread_message');
         $class = "webmail_plus_unread_message";
       }

       if($folder=='Sent' || $folder=='Drafts') {
         
         $rows[] = array(
          array('data'=>drupal_render($form['selected'][$key]),'class'=>$class),
          array('data'=>drupal_render($form['to'][$key]),'class'=>$class),
          array('data'=>drupal_render($form['subject'][$key]),'class'=>$class),
          array('data'=>drupal_render($form['date'][$key]),'class'=>$class),
          array('data'=>drupal_render($form['attachments'][$key]),'class'=>$class .' attachments')
         );

       }
       elseif($_GET['list']) {
         
          $rows[] = array(
            array('data'=>drupal_render($form['selected'][$key]),'class'=>$class),
            array('data'=>drupal_render($form['tofrom'][$key]),'class'=>$class),
            array('data'=>drupal_render($form['folder'][$key]),'class'=>$class),
            array('data'=>drupal_render($form['subject'][$key]),'class'=>$class),
            array('data'=>drupal_render($form['date'][$key]),'class'=>$class),
            array('data'=>drupal_render($form['attachments'][$key]),'class'=>$class .' attachments')
         );       
       }
       else {
         //echo "hee hee";
         
          $rows[] = array(
          array('data'=>drupal_render($form['selected'][$key]),'class'=>$class),
          array('data'=>drupal_render($form['from'][$key]),'class'=>$class),
          array('data'=>drupal_render($form['subject'][$key]),'class'=>$class),
          array('data'=>drupal_render($form['date'][$key]),'class'=>$class),
          array('data'=>drupal_render($form['attachments'][$key]),'class'=>$class .' attachments')
         );        
       }

       unset($form['seen'][$key]);
       unset($form['to'][$key]);
       
     }

  } else {
    $rows[] = array(array('data' => t('No messages available.'), 'colspan' => '4'));
  }

  $output .= theme('table', $form['header']['#value'], $rows);

  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);

  return $output;

}

/**
 * implementation of hook_submit
 *
 * @param unknown_type $form_id
 * @param unknown_type $form_state
 */
function webmail_plus_view_folder_form_submit($form_id, &$form_state) {
  global $user, $_webmail_plus_config, $_webmail_plus_user_preferences;

  
  // try to define these outside of the loop
  $folder=substr($form_state['values']['operation'], 8, strlen($form_state['values']['operation'])-8);
  
  $tag=substr($form_state['values']['operation'], 9, strlen($form_state['values']['operation'])-9);
  
      

    
  // empty trash
  if($form_state['clicked_button']['#id']=='empty_trash') {
    _webmail_plus_connect($_SESSION['webmail_plus']['active_folder']);
        
    $overview = mail_api_overview();
    foreach($overview as $message_id=>$message_info) {
    	mail_api_delete_message($message_info->uid);
    }
  } 
  
  
  // go thru all the messages and apply the action
  foreach($form_state['values']['selected'] as $message_id=>$message_checked) {
    if ($message_checked==0) continue;


    $message_parts = webmail_plus_parse_message_key($message_checked);
    
    //wpd($message_parts);
    
    //webmail_plus_dump($message_parts);
    
    // perform drop down operations
    if ($form_state['clicked_button']['#id']=='update') {
      
      if (preg_match("/move_to_/", $form_state['values']['operation'])) {

        //drupal_set_message('moving stuff');
        $folder = preg_match("/move_to_(.*)/", $form_state['values']['operation'], $matches);
        $folder = $matches[1];

        //drupal_set_message('moving message '.$message_parts->message_uid.' to '.$folder);

        _webmail_plus_connect($_SESSION['webmail_plus']['active_folder']);
        //webmail_plus_dump($_SESSION['webmail_plus']);
        
        if($_webmail_plus_config->debug) watchdog('webmail_plus', 'Moving message %message_uid to %folder', array('%message_uid'=>$message_parts->message_uid, '%folder'=>$folder));
        //mail_api_move_message($message_parts->message_uid, $folder);
        _webmail_plus_move_message($message_checked, $folder);
        // create a message object
        $delete_message_object = new stdClass;
        $delete_message_object -> uid = $user -> uid;
        $delete_message_object -> message_key = $message_checked;
        module_invoke_all('webmail_plus_delete_message', $delete_message_object);
        
      }
      
      // handle mass tagging
      if (preg_match("/tag_with_/", $form_state['values']['operation'])) {
        if($_webmail_plus_config->debug) watchdog('webmail_plus', 'Tagging message %message_uid to %tag', array('%message_uid'=>$message_parts->message_uid, '%folder'=>$tag));        
        _webmail_plus_add_tag($message_checked, $tag);
      }
      

      
      //drupal_set_message($form_state['values']['operation']);
      
      // set the message as unread
      if($form_state['values']['operation']=='mark_unread') {
        //drupal_set_message('marking unread');
        
        if($_webmail_plus_config->debug) watchdog('webmail_plus', 'Flagging message %message_uid in folder %folder as unread.', array('%message_uid'=>$message_parts->message_uid, '%folder'=>$message_parts->folder));
        
        _webmail_plus_connect($message_parts->folder);
        mail_api_flag_unread($message_parts->message_uid);
        _webmail_plus_flag_unread($message_checked);
        
      }      

      // set the message as read
      if($form_state['values']['operation']=='mark_read') {
        //drupal_set_message('marking read');
        if($_webmail_plus_config->debug) watchdog('webmail_plus', 'Flagging message %message_uid in folder %folder as unread.', array('%message_uid'=>$message_parts->message_uid, '%folder'=>$message_parts->folder));
        
        _webmail_plus_connect($message_parts->folder);
        mail_api_flag_read($message_parts->message_uid);
        _webmail_plus_flag_read($message_checked);
      }            
      
    }

    


            
    // handle delete action, move messages to the trash folder
    if ($form_state['clicked_button']['#id']=='delete') {
      _webmail_plus_connect($message_parts->folder);
      
      //wpd('deleting '.$message_parts->message_uid.' from '.$message_parts->folder);
      
        // if we're in the trash folder we delete messages permanently
      if($message_parts->folder==mail_api_trash_path($_webmail_plus_config->domain)) {
        //wpd('permanently');
        if($_webmail_plus_config->debug) watchdog('webmail_plus', 'Deleting %message_uid', array('%message_uid'=>$message_parts->message_uid));
        _webmail_plus_flush_tags($message_checked); // remove tags associated with this message  
        _webmail_plus_delete_message($message_checked);  // permanently remove the message
      }
      else {       // otherwise we just move them to trash
       if($_webmail_plus_config->debug) watchdog('webmail_plus', 'Moving %message_uid to %folder', array('%message_uid'=>$message_parts->message_uid, '%folder'=>mail_api_trash_path($_webmail_plus_config->domain)));
        _webmail_plus_flush_tags($message_checked); // remove tags associated with this message  
       _webmail_plus_move_message($message_checked, mail_api_trash_path($_webmail_plus_config->domain)); // move the message to trash
      }
        
      
        
      // create a message object
      $delete_message_object = new stdClass;
      $delete_message_object -> uid = $user -> uid;
      $delete_message_object -> message_key = $message_checked;
      module_invoke_all('webmail_plus_delete_message', $delete_message_object);
    }    
      



  }

}




?>
