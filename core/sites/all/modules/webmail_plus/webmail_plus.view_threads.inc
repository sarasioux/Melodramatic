<?php
// $Id: webmail_plus.view_threads.inc,v 1.1.2.8 2009/05/14 18:30:30 firmidea Exp $
function webmail_plus_view_threads($folder = 'INBOX') {

  return drupal_get_form('webmail_plus_view_threads_form');

}


function webmail_plus_view_threads_form() {
  global $user, $_webmail_plus_config, $_webmail_plus_user_preferences;

  if($_SESSION['webmail_plus']['connection_disabled']) {
    drupal_set_message(t('Your email password is not correct. Please click !link to set it.', array('!link' => l('here', 'user/'.$user->uid.'/webmail_plus/password'))), 'error', FALSE);
    return FALSE;
  }  
  
  require_once 'webmail_plus.user.inc';
  require_once 'webmail_plus.functions.inc';
  
//  $rs = webmail_plus_get_thread_messages(2);
//  webmail_plus_dump($rs);


  //webmail_plus_dump($_webmail_plus_user_preferences);

  $folder  = arg(2);

  //echo $folder;

  if ($folder=="") {
    $folder = $webmail_plus_config->inbox_folder;
  }

  $tags = arg(3);
  $list = arg(4);

  if($list) {
    $list_array = explode(",", $list);
    $list_clean = implode(", ", $list_array);
    $list_fixed = "'".implode("', '", $list_array)."'";

    $tags_array = explode(",", $tags);
    $tags_clean = implode(", ", $tags_array);

    drupal_set_title(t('All threads tagged with %tag', array('%tag'=>$tags_clean)));
  }
  else {
    drupal_set_title(t('Folder: %folder', array('%folder'=>$folder)));
  }

  $form['action'] = array(
    '#type' => 'fieldset',
    '#title' => t('Actions'),
    '#prefix' => '<div class="container-inline">', '#suffix' => '</div>'
  );

  $form['action']['folder'] = array(
    '#type' => 'hidden',
    '#value' => $folder
  );


  $form['action']['delete'] = array(
    '#type' => 'submit',
    '#id' => 'delete',
    '#title' => t('Delete'),
    '#value' => t('Delete')
  );

   // these should always be present
   $options['mark_read'] = t('Mark Read');
   $options['mark_unread'] = t('Mark Unread');

   /*
   $mail_api_connection = _webmail_plus_connect();
   $mailboxes = mail_api_mailboxes();
   */

   //print_r($mailboxes);

   if (sizeof($mailboxes)>0) {
     foreach($mailboxes as $mailbox_id => $mailbox_name) {

       // if for some reason the mailbox is empty, skip it
       if ($mailbox_name=="") continue;

       // do not show the current mailbox as an option
       if ($mailbox_name==$folder) continue;
       $options['move_to_'.$mailbox_name] = t('Move to').' '.$mailbox_name;
     }
   }

   // tag with
   require_once 'webmail_plus.tag_functions.inc';
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
      '#id' => 'update',
      '#value' => t('Update')
   );

   $form['header'] = array(
    '#type' => 'value',
    '#value' => array(
      theme('table_select_header_cell'),
      array('data' => t('From'), 'field' => 'from'),
      array('data' => t('Subject'), 'field' => 'subject'),
      array('data' => t('Date'), 'field' => 'date', 'sort' => 'desc'),
    )
  );

  // this provides table sorting
  $header = array(
    array('data' => t('From'), 'field' => "`from`"),
    array('data' => t('Subject'), 'field' => '`subject`'),
    array('data' => t('Date'), 'field' => '`date`', 'sort' => 'desc')
  );

  
  $new_tablesort = _webmail_plus_escape_reserved_db_words(tablesort_sql($header));
  
  //webmail_plus_dump($_webmail_plus_user_preferences);
  
      
  if($_SESSION['webmail_plus']['sort_order']=="") {
    if($_webmail_plus_user_preferences->sort_order) {
      $_SESSION['webmail_plus']['sort_order'] = $_webmail_plus_user_preferences->sort_order;
    }
    else {
      $_SESSION['webmail_plus']['sort_order'] = $new_tablesort;
    }
  }
  else {
    if($_SESSION['webmail_plus']['sort_order']!=$new_tablesort) {
      $_SESSION['webmail_plus']['sort_order']=$new_tablesort;
      webmail_plus_set_user_preference('sort_order', $new_tablesort);
      
    }
  }
  


  
  //drupal_set_message($_SESSION['webmail_plus']['sort_order']);
  
  /*
  webmail_plus_dump($_webmail_plus_user_preferences->sort_order);

  if(trim($tablesort)=='ORDER BY from ASC') {
    //drupal_set_message('overriding tablesort');
    $tablesort = $_webmail_plus_user_preferences->sort_order;
  }
  else {
    $tablesort = preg_replace("/from/", "`from`", $tablesort);
    $tablesort = db_escape_string($tablesort);
    webmail_plus_set_user_preference('sort_order', $tablesort);
  }

  webmail_plus_dump($tablesort);
  */
  
  //echo $tablesort;


  if($list) {

    //echo $list_fixed;

    foreach($list_array as $index=>$thread_key) {
      $sql = "SELECT * FROM {webmail_plus_user_threads} WHERE uid=".$user->uid." AND CONCAT(uid,'$',folder,'@',tid) IN(".$list_fixed.")";
    }

  }
  else {
    $sql = "SELECT * FROM {webmail_plus_user_threads} WHERE uid=".$user->uid." AND folder='".$folder."'";
  }

  //  $result = pager_query($sql, 50);
  $result = pager_query($sql.' '.$_SESSION['webmail_plus']['sort_order'], 50);


  //drupal_set_message($sql.' '.$_SESSION['webmail_plus']['sort_order']);
  
  while($header = db_fetch_object($result)) {
    $key = $user->uid."$".$header->folder."@".$header->tid;
    $selected[$key] = '';

    //print_r($header);

    if($header->message_count>1) {
      $form['from'][$key] = array('#value' => l(_webmail_plus_render_from($header->from).' ('.$header->message_count.')', 'webmail_plus/thread_view/'.$key));

    }
    else {

      $form['from'][$key] = array('#value' => l(_webmail_plus_render_from($header->from), 'webmail_plus/thread_view/'.$key));

    }

    $form['subject'][$key] = array('#value' => l(_webmail_plus_render_subject($header->subject), 'webmail_plus/thread_view/'.$key));
    $form['date'][$key] = array('#value' => l(_webmail_plus_render_date($header->date), 'webmail_plus/thread_view/'.$key));

    if($header->seen==1) {
      $form['seen'][$key] = array('#value' => 1);
    }
    else {
      $form['seen'][$key] = array('#value' => 0);
    }

  }

  $form['selected'] = array('#type' => 'checkboxes', '#options' => $selected);
  $form['pager'] = array('#value' => theme('pager', NULL, 50, 0));





  return $form;


}

function theme_webmail_plus_view_threads_form($form) {
  $output = drupal_render($form['action']);




  if (isset($form['from'])) {

     foreach (element_children($form['from']) as $key) {
       $row = array();

       //webmail_plus_dump($form['seen'][$key]);
       //webmail_plus_dump($form['seen'][$key]['#value']);


       if($form['seen'][$key]['#value']==1) {
         $class = "webmail_plus_read_message";
       }
       else {
         $class = "webmail_plus_unread_message";
       }

       //drupal_set_message($class);

       $rows[] = array(
        array('data'=>drupal_render($form['selected'][$key]),'class'=>$class),
        array('data'=>drupal_render($form['from'][$key]),'class'=>$class),
        array('data'=>drupal_render($form['subject'][$key]),'class'=>$class),
        array('data'=>drupal_render($form['date'][$key]),'class'=>$class)
       );

       /*
       $row[] = drupal_render($form['selected'][$key]);
       $row[] = drupal_render($form['from'][$key]);
       $row[] = drupal_render($form['subject'][$key]);
       $row[] = drupal_render($form['date'][$key]);
       $rows[] = $row;
       */
       unset($form['seen'][$key]);
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

function webmail_plus_view_threads_form_submit($form_id, &$form_state) {
  //webmail_plus_dump($form_state);
  
  //$folder=substr($form_state['values']['operation'], 8, strlen($form_state['values']['operation'])-8);
  
  //drupal_set_message($folder);
  
  $tag=substr($form_state['values']['operation'], 9, strlen($form_state['values']['operation'])-9);
  //drupal_set_message($tag);
  
  foreach($form_state['values']['selected'] as $thread_id=>$thread_checked) {
    if ($thread_checked==0) continue;
    
    
    // split the thread key into parts
    $thread_parts = webmail_plus_parse_thread_key($thread_checked);
    //webmail_plus_dump($thread_parts);
    
    // removes all the messages in the thread
    if ($form_state['clicked_button']['#id']=='delete' && is_array($form_state['clicked_button']['#post']['selected'])) {
      //drupal_set_message('deleting threads');
      webmail_plus_delete_thread($thread_parts->thread_id, $thread_parts->uid);
    }

    
    if (preg_match("/tag_with_/", $form_state['values']['operation'])) {

        //drupal_set_message('tagging a message with '.$tag);
        if($_webmail_plus_config->debug) watchdog('webmail_plus', 'Tagging message %message_uid to %tag', array('%message_uid'=>$message_parts->message_uid, '%folder'=>$tag));
        

        webmail_plus_tag_thread($thread_parts->thread_id, $tag, $thread_parts->uid);

    }   
    
    // mark thread as read
    if($form_state['values']['operation']=='mark_read') {
        if($_webmail_plus_config->debug) watchdog('webmail_plus', 'Flagging thread %thread_uid as read.', array('%thread_uid'=>$thread_parts->thread_id));
        webmail_plus_flag_thread_read($thread_parts->thread_id, $thread_parts->uid);
    }     
    
    // mark thread as unread
    if($form_state['values']['operation']=='mark_unread') {
        if($_webmail_plus_config->debug) watchdog('webmail_plus', 'Flagging thread %thread_uid as unread.', array('%thread_uid'=>$thread_parts->thread_id));
        webmail_plus_flag_thread_unread($thread_parts->thread_id, $thread_parts->uid);
    } 
      
    
  }    
  // $rs = webmail_plus_get_thread_messages(1, "INBOX", 1);
  
    
}


/**
 * returns an array with message ids
 *
 * @param unknown_type $tid
 * @param unknown_type $uid
 * @param unknown_type $folder
 */
function webmail_plus_get_thread_messages($tid, $uid=NULL) {
  if($tid=="") return FALSE;
  
  if($uid=="") {
    global $user;
    if($user->uid<=0) return FALSE;
    $uid=$user->uid;
  }
  
  $results = db_query("SELECT * FROM {webmail_plus_user_threads_map} WHERE tid=%d AND uid=%d", $tid, $uid);
  
  while($record = db_fetch_object($results)) {
    //webmail_plus_dump($record);
    $rs[]=$record;
  }
  
  return $rs;
}

/**
 * adds a tag to all messages in the thread
 *
 * @param unknown_type $tid
 * @param unknown_type $tag
 * @param unknown_type $uid
 * @return unknown
 */
function webmail_plus_tag_thread($tid, $tag, $uid=NULL) {
  
  //drupal_set_message("tagging thread $tid with $tag");
  
  if($tid=="" || $tag=="") return FALSE;
  
  if($uid=="") {
    global $user;
    if($user->uid<=0) return FALSE;
    $uid=$user->uid;
  }
  
  $thread_messages = webmail_plus_get_thread_messages($tid, $uid);
  
  foreach($thread_messages as $message_id=>$message_info) {
    webmail_plus_add_tag($message_info->folder, $message_info->message_uid, $tag);
  }
  
  return TRUE;
}

/**
 * marks all messages in the thread as read
 *
 * @param unknown_type $tid
 * @param unknown_type $uid
 * @return unknown
 */
function webmail_plus_flag_thread_read($tid, $uid=NULL) {
  if($tid=="") return FALSE;
  
  if($uid=="") {
    global $user;
    if($user->uid<=0) return FALSE;
    $uid=$user->uid;
  }  
  
  $thread_messages = webmail_plus_get_thread_messages($tid, $uid);
  
  foreach($thread_messages as $message_id=>$message_info) {
    _webmail_plus_connect($message_info->folder);
    mail_api_flag_read($message_info->message_uid);
  }
  
  return TRUE;  
}

/**
 * marks all messages in the thread as unread
 *
 * @param unknown_type $tid
 * @param unknown_type $uid
 * @return unknown
 */
function webmail_plus_flag_thread_unread($tid, $uid=NULL) {
  if($tid=="") return FALSE;
  
  if($uid=="") {
    global $user;
    if($user->uid<=0) return FALSE;
    $uid=$user->uid;
  }  
  
  $thread_messages = webmail_plus_get_thread_messages($tid, $uid);
  
  foreach($thread_messages as $message_id=>$message_info) {
    _webmail_plus_connect($message_info->folder);
    mail_api_flag_unread($message_info->message_uid);
  }
  
  return TRUE;  
}

/**
 * removes all messages in the thread
 *
 * @param unknown_type $tid
 * @param unknown_type $uid
 * @return unknown
 */
function webmail_plus_delete_thread($tid, $uid=NULL) {
  if($tid=="") return FALSE;
  
  if($uid=="") {
    global $user;
    if($user->uid<=0) return FALSE;
    $uid=$user->uid;
  }  
  
  global $_webmail_plus_config, $_webmail_plus_user_preferences;
  
    
  $thread_messages = webmail_plus_get_thread_messages($tid, $uid);
  
  foreach($thread_messages as $message_id=>$message_info) {
    _webmail_plus_connect($message_info->folder);
    mail_api_move_message($message_info->message_uid, mail_api_trash_path($_webmail_plus_config->domain));
  }  
  
  return TRUE;
}

