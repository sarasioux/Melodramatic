<?php
function _webmail_plus_get_filters($user=NULL) {
  if(!$user) global $user;
  
  $result = db_query("SELECT * FROM {webmail_plus_user_filters} WHERE uid=%d AND active=1", $user->uid);
  
  while($filter = db_fetch_object($result)) {
    $filters[]=$filter;
  }
  
  return $filters;
}


function _webmail_plus_matches_filter($message,$filter) {

  //if(!$message || !$filter) return FALSE;
  
  
  
  $success=FALSE;
  
  // check matches on from
  if($filter->match_from) {
    if(strstr($message->from, $filter->match_from)) {
      $success=TRUE;
    }
    else {
      $success=FALSE;
    }
  }
  
  // check matches on to
  if($filter->match_to) {
    if(strstr($message->to, $filter->match_to)) {
      $success=TRUE;
    }
    else {
      $success=FALSE;
    }
  }
    
  // check matches on cc
  if($filter->match_cc) {
    if(strstr($message->cc, $filter->match_cc)) {
      $success=TRUE;
    }
    else {
      $success=FALSE;
    }
  }

  // check matches on subject
  if($filter->match_subject) {
    if(strstr($message->subject, $filter->match_subject)) {
      $success=TRUE;
    }
    else {
      $success=FALSE;
    }  
  }
    
  // check matches on has words
  if($filter->match_has_words) {
    if(strstr($message->body, $filter->match_has_words)) {
      $success=TRUE;
    }
    else {
      $success=FALSE;
    }    
  }
    
    // check matches on has words
  if($filter->match_has_attachments>0) {
    if($message->attachments>0) {
      $success=TRUE;
    }
    else {
      $success=FALSE;
    }    
  }
    

  
  /*
  if($success) {
    echo "message matched the filter<br>\n";
  }
  else {
    echo "message did not match the filter<br>\n";
  }
  */
  
  return $success;
}

function _webmail_plus_apply_filter($message, $filter) {
  global $user;
  global $_webmail_plus_config;
  
  // apply only if the message matches the filter
  if(!_webmail_plus_matches_filter($message, $filter)) return FALSE;

  $message_key = $user->uid."!".$message->folder."@".$message->message_uid;
  
        
  
  // mark as read
  if($filter->action_mark_as_read) {
    _webmail_plus_connect($message->folder);
    mail_api_flag_unread($message->message_uid);
    db_query("UPDATE {webmail_plus_user_headers} SET seen=1 WHERE uid=%d AND folder='%s' AND message_uid=%d", $user->uid, $message->folder, $message->message_uid);
  }

  // apply tags
  if($filter->action_apply_tags && $filter->action_apply_tags_list) {
    require_once 'webmail_plus.tag_functions.inc';
    
    $tags_array = preg_split("/\s|,|;/", $filter->action_apply_tags_list);
    foreach($tags_array as $tag_id=>$tag) {
      _webmail_plus_add_tag($message_key, $tag);
    }
  }
  
  // move to junk
  if($filter->action_move_to_junk) {
    require_once 'webmail_plus.functions.inc';
    _webmail_plus_move_message($message_key, mail_api_junk_path($_webmail_plus_config -> domain));
    db_query("DELETE FROM {webmail_plus_user_headers} WHERE uid=%d AND folder='%s' AND message_uid=%d", $user->uid, $message->folder, $message->message_uid);
    
  }
  
  // move to trash
  if($filter->action_move_to_trash) {
    //echo "moving a message to trash";
    require_once 'webmail_plus.functions.inc';
    _webmail_plus_move_message($message_key, mail_api_trash_path($_webmail_plus_config -> domain));
    db_query("DELETE FROM {webmail_plus_user_headers} WHERE uid=%d AND folder='%s' AND message_uid=%d", $user->uid, $message->folder, $message->message_uid);
  }
  return TRUE;

}

function _webmail_plus_log_filter($user=NULL, $message_id, $filter_id=NULL) {
  if(!$user) global $user;
  
  if(_webmail_plus_filter_applied($user, $message_id)) return;
  
  $rs = db_query("INSERT INTO {webmail_plus_user_filter_log}(uid,message_id,filter_id) VALUES(%d,'%s',%d)", $user->uid,$message_id,$filter_id);

  return $rs;
}

function _webmail_plus_filter_applied($user=NULL, $message_id) {
  if(!$user) global $user;
  $message_id=trim($message_id);
  if($message_id=="") return FALSE;
  
  //echo "message_id: ".$message_id."<br>\n";
  
  $rs = db_fetch_object(db_query("SELECT COUNT(*) AS count FROM {webmail_plus_user_filter_log} WHERE uid=%d AND message_id='%s'", $user->uid, $message_id));

  
  return $rs->count;
}
?>