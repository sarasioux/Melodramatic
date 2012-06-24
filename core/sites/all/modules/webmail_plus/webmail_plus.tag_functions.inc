<?php

/*
 * a set of functions designed to work with tags
 */
function _webmail_plus_add_tag($message_key, $tag) {
  global $user;

  $message_parts = webmail_plus_parse_message_key($message_key);
  $tag = strtolower(trim($tag));


  // if the tag is already assigned return true
  if(_webmail_plus_check_tag($message_key, $tag)) return TRUE;

  db_query("INSERT INTO {webmail_plus_user_tags} (uid,folder,message_uid,tag) VALUES(%d, '%s', %d, '%s')", $user->uid, $message_parts->folder, $message_parts->message_uid, $tag);

  return TRUE;
}

/*
 * returns TRUE if this tag is already assigned to the message
 */
function _webmail_plus_check_tag($message_key, $tag) {
  global $user;

  $message_parts = webmail_plus_parse_message_key($message_key);
  $tag = strtolower(trim($tag));


  $count = db_result(db_query("SELECT COUNT(*) FROM {webmail_plus_user_tags} WHERE uid=%d AND folder='%s' AND message_uid=%d AND tag='%s'", $user->uid, $message_parts->folder, $message_parts->message_uid, $tag));

  if($count>0) {
    return TRUE;
  } else {
    return FALSE;
  }

}

/**
 * removes a particular tag from the message
 *
 * @param unknown_type $folder
 * @param unknown_type $message_uid
 * @param unknown_type $tag
 * @return unknown
 */
function webmail_plus_remove_tag($message_key, $tag) {
  global $user;

  $message_parts = webmail_plus_parse_message_key($message_key);
  $tag = strtolower(trim($tag));


  db_query("DELETE FROM {webmail_plus_user_tags} WHERE uid=%d AND folder='%s' AND message_uid=%d AND tag='%s'", $user->uid, $message_parts->folder, $message_parts->message_uid, $tag);

  return TRUE;
}

/*
 * completely removes the tag from the users space
 */
function _webmail_plus_delete_tag($tag) {
  global $user;
  $tag=trim($tag);
  //drupal_set_message('removing tag '.$tag);
  
  db_query("DELETE FROM {webmail_plus_user_tags} WHERE uid=%d AND tag='%s'", $user->uid, $tag);
}

/*
 * if a message is moved between folders we still want to keep the tags associated
 * this function comes in handy for that
 */
function _webmail_plus_remap_tags($old_message_key, $new_message_key) {
  global $user;
  
  $old_message_parts = webmail_plus_parse_message_key($old_message_key);
  $new_message_parts = webmail_plus_parse_message_key($new_message_key);
  
  return db_query("UPDATE {webmail_plus_user_tags} SET folder='%s', message_uid=%d WHERE folder='%s' AND message_uid=%d AND uid=%d", $new_message_parts->folder, $new_message_parts->message_uid, $old_message_parts->folder, $old_message_parts->message_uid, $user->uid);
  
}

/**
 * removes all tags associated with the message
 *
 * @param unknown_type $folder
 * @param unknown_type $message_uid
 * @param unknown_type $folder
 * @param unknown_type $message_uid
 * @return unknown
 */
function _webmail_plus_flush_tags($message_key) {
  global $user;

  $message_parts = webmail_plus_parse_message_key($message_key);
  db_query("DELETE FROM {webmail_plus_user_tags} WHERE uid=%d AND folder='%s' AND message_uid=%d", $user->uid, $message_parts->folder, $message_parts->message_uid);

  return TRUE;

}

/**
 * returns all tags associated with a message
 */
function _webmail_plus_get_message_tags($message_key) {
  global $user;
  $message_parts = webmail_plus_parse_message_key($message_key);
  

  $result = db_query("SELECT CONCAT(uid,'-',folder,'-',message_uid), tag FROM {webmail_plus_user_tags} WHERE uid=%d AND folder='%s' AND message_uid=%d", $user->uid, $message_parts->folder, $message_parts->message_uid);

  while($record = db_fetch_object($result)) {
    $tag = $record->tag;
    $tag = trim($tag);
    if($tag=="") continue;
    $tags[]=$record->tag;
  }



  return $tags;

}

/**
 * returns all tags created by the user
 * takes UID as input
 * if UID is blank, uses current uid
 *
 * @param unknown_type $uid
 */
function webmail_plus_get_user_tags($uid="", $max=NULL) {
  global $user;

  if($uid=="") $uid = $user->uid;
  if(!$max) {
    $result = db_query("SELECT tag, count( * ) AS popularity FROM {webmail_plus_user_tags} WHERE uid=%d GROUP BY tag ORDER BY popularity DESC", $uid);
  }
  else {
    $result = db_query("SELECT tag, count( * ) AS popularity FROM {webmail_plus_user_tags} WHERE uid=%d GROUP BY tag ORDER BY popularity DESC LIMIT %d", $uid, $max);  	
  }
  
  while($record = db_fetch_object($result)) {
    $this_tag = trim($record->tag);
    if($this_tag=='') continue;
    $tags[]=$this_tag;
  }

  return $tags;

}


?>