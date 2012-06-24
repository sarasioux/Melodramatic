<?php
// $Id: webmail_plus.view_tag.inc,v 1.1.2.10 2009/05/14 18:30:30 firmidea Exp $

/**
 * shows messages by tags
 *
 * @return unknown
 */


// FIXME: need to add tablesort


function webmail_plus_view_tag() {
  //drupal_set_message('webmail_plus_messages_by_tag_form');

  require_once 'webmail_plus.user.inc';
  require_once 'webmail_plus.functions.inc';
  require_once 'webmail_plus.tag_functions.inc';


  //drupal_set_message('folder '.arg(0));
  global $user, $_webmail_plus_config;



  $tags = arg(2);



  if(preg_match("/\s|,|;/",$tags)) {
    $tags_array = preg_split("/\s|,|;/", $tags);
    $tags_array = array_unique($tags_array);
  } else {
    $tags_array[] = $tags;
  }


  $tags_list=implode(",", $tags_array);


  $sql_where = "";
  foreach($tags_array as $index=>$tag) {
    if($sql_where!="") $sql_where.=" OR ";
    $sql_where.="tag='$tag'";
  }

  $sql = "SELECT DISTINCT(CONCAT(uid,'!',folder,'@',message_uid)) AS message_key FROM {webmail_plus_user_tags} WHERE uid=".$user->uid." AND ".$sql_where;
  $result = db_query($sql);



  
  
  // assemble the message list
  $matches=0;
  while($header = db_fetch_object($result)) {
    $message_array[]=$header->message_key;
    $matches++;
  }


  
  // remove a dangling tag
  if($matches<=0) {
    foreach($tags_array as $id=>$tag) {
      //drupal_set_message('removing tag '.$tag);
      _webmail_plus_delete_tag($tag);
    }
  }
  
  $message_list=implode(",",$message_array);
    
  drupal_goto('webmail_plus/view_folder', 'tags='.$tags_list.'&list='.$message_list);

  /*
  if( $_webmail_plus_config->thread_messages) {
    // handle threads

    $message_fixed = "'".implode("', '", $message_array)."'";
    //echo $message_fixed;

    $sql = "SELECT CONCAT(uid,'$',folder,'@',tid) AS thread_key FROM {webmail_plus_user_threads_map} WHERE uid=".$user->uid." AND CONCAT(uid,'!',folder,'@',message_uid) IN(".$message_fixed.")";

    $result = db_query($sql);
    while($header = db_fetch_object($result)) {
      $thread_array[]=$header->thread_key;
    }

    $thread_list = implode(",", $thread_array);
    drupal_goto('webmail_plus/view_threads', 'tags='.$tags_list.'&list='.$thread_list);

  }
  else {

    // handle messages



  }
  */



}


function theme_webmail_plus_view_tag_form($form) {



  $output = drupal_render($form['action']);

  if(isset($form['from'])) {

     foreach (element_children($form['from']) as $key) {
       $row = array();

       $row[] = drupal_render($form['selected'][$key]);
       $row[] = drupal_render($form['from'][$key]);
       $row[] = drupal_render($form['subject'][$key]);
       $row[] = drupal_render($form['date'][$key]);
       $rows[] = $row;
     }

  } else {
    $rows[] = array(array('data' => t('No messages.'), 'colspan' => '4'));
  }

  $output .= theme('table', $form['header']['#value'], $rows);

  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);

  return $output;

}

/*
 * FIXME this is highly infefficient since it works with a huge dataset
 */
function webmail_plus_update_view_tag($tags="") {
  global $user;


  //drupal_set_message('fetching all headers that match tags '.$tags);


  $tags=trim($tags);
  if($tags=="") {
    $content.=t('No tags were supplied.');
    return $content;
  }

  $tags_array = preg_split("/\s|,|;/", $tags);
  $tags_array = array_unique($tags_array);
  $tag_list = implode(",",$tags_array);  // this is used for queries

  //echo $tag_list;

  // remove previous results
  db_query("DELETE FROM {webmail_plus_user_headers_by_tag} WHERE uid=%d AND tags='%s'", $user->uid, $tag_list);


  // build a query to fetch all message ids with the assigned tags
  $sql = "SELECT * FROM {webmail_plus_user_tags} WHERE uid=".$user->uid." AND ";
  foreach($tags_array as $tag_index=>$tag) {
    if($clause!="") $clause .= " OR ";
    $clause .= "tag='".$tag."'";
  }

  $full_sql = $sql . $clause;
  //drupal_set_message($full_sql);


  $result = db_query($full_sql);

  while($match = db_fetch_object($result)) {

    //print_r($match);

    $header = db_fetch_object(db_query("SELECT * FROM {webmail_plus_user_headers} WHERE uid=%d AND folder='%s' AND message_uid=%d", $user->uid, $match->folder, $match->message_uid, $user->uid, $match->folder, $match->message_uid));

    //print_r($header);

    db_query("INSERT INTO {webmail_plus_user_headers_by_tag}
      	(`uid`, `tags`, `folder`, `subject`, `from`, `to`, `cc`, `date`, `message_id`, `size`, `message_uid`, `msgno`, `recent`, `flagged`, `answered`, `deleted`, `seen`, `draft`)

      	VALUES(%d, '%s', '%s', '%s', '%s', '%s', '%s', %d, '%s', %d, '%s', '%s', %d, %d, %d, %d, %d, %d)",
      $user -> uid,
      $tags,
      $header -> folder,
      $header -> subject,
      $header -> from,
      $header -> to,
      $header -> cc,
      $header -> date,
      $header -> message_id,
      $header -> size,
      $header -> message_uid,
      $header -> msgno,
      $header -> recent,
      $header -> flagged,
      $header -> answered,
      $header -> deleted,
      $header -> seen,
      $header -> draft,
      $header -> charset
      );
  }


}
?>
