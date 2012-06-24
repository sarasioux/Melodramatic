<?php
function webmail_plus_solr_search() {
  
  global $_webmail_plus_config, $user;

  
    
  $keyword = arg(2);
  $keyword_sha1 = sha1($keyword);
  
  drupal_set_title(t('Email search results for %keyword', array('%keyword' => $keyword)));
  
  _webmail_plus_solr_do_search($keyword);
  
  $form['header'] = array('#type' => 'value', '#value' => array(
    theme('table_select_header_cell'),
    array('data' => t('From/To'), 'field' => 'from'),
    array('data' => t('Folder'), 'field' => 'folder'),
    array('data' => t('Subject'), 'field' => 'subject'),
    array('data' => t('Date'), 'field' => 'date', 'sort' => 'desc'),
    array('data' => t('Attachments'), 'field' => 'attachments'),
    )
  );
  
  $header = array(
    array('data' => t('From/To'), 'field' => "`from`"),
    array('data' => t('Folder'), 'field' => "`folder`"),
    array('data' => t('Subject'), 'field' => '`subject`'),
    array('data' => t('Date'), 'field' => '`date`', 'sort' => 'desc'),
    array('attachments' => t('Attachments'), 'field' => 'attachments')  
  ); 
    
  $new_tablesort = _webmail_plus_escape_reserved_db_words(tablesort_sql($header));
  

  $sql = "SELECT * FROM {webmail_plus_solr_results_headers} WHERE uid=".$user->uid." AND keyword_sha1='".$keyword_sha1."'";
  
  //$result = pager_query($sql, 50);
  $result = pager_query($sql.' '.$_SESSION['webmail_plus']['sort_order'], $_webmail_plus_config->messages_per_page); 

  // include webmail functions
  require_once drupal_get_path('module', 'webmail_plus').'/webmail_plus.functions.inc';
  
  while($header = db_fetch_object($result)) {

   $key = $header->message_key;
   $selected[$key] = '';
   
   $form['tofrom'][$key] = array('#value' => l(_webmail_plus_combine_to_from($header->to, $header->from), 'webmail_plus/message_view/'.$key, array('html'=>TRUE)));
   $form['folder'][$key] = array('#value' => l($header->folder, 'webmail_plus/message_view/'.$key));
   $form['subject'][$key] = array('#value' => l(_webmail_plus_render_subject($header->subject), 'webmail_plus/message_view/'.$key));    
   $form['date'][$key] = array('#value' => l(_webmail_plus_render_date($header->date), 'webmail_plus/message_view/'.$key));
   $form['attachments'][$key] = array('#value' => l($header->attachments, 'webmail_plus/message_view/'.$key));     
   
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

function theme_webmail_plus_solr_search($form) {



  $output = drupal_render($form['action']);

  if (isset($form['date'])) {

     foreach (element_children($form['date']) as $key) {
       $row = array();

       if($form['seen'][$key]['#value']==1) {
         //drupal_set_message('webmail_plus_read_message');
         $class = "webmail_plus_read_message";
       }
       else {
         //drupal_set_message('webmail_plus_unread_message');
         $class = "webmail_plus_unread_message";
       }

       $rows[] = array(
          array('data'=>drupal_render($form['selected'][$key]),'class'=>$class),
          array('data'=>drupal_render($form['tofrom'][$key]),'class'=>$class),
          array('data'=>drupal_render($form['folder'][$key]),'class'=>$class),
          array('data'=>drupal_render($form['subject'][$key]),'class'=>$class),
          array('data'=>drupal_render($form['date'][$key]),'class'=>$class),
          array('data'=>drupal_render($form['attachments'][$key]),'class'=>$class .' attachments')
       );        
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

function _webmail_plus_solr_do_search($keyword, $user=NULL) {
	
  if(!$keyword) return FALSE;
  if(!$user) global $user;
  $keyword_sha1 = sha1($keyword);
	
  //drupal_set_message('running search for '.$keyword);
	  
  //drupal_set_message('deleting things from the results tables');

  db_query("DELETE FROM {webmail_plus_solr_results} WHERE uid=%d AND keyword_sha1='%s'", $user->uid, $keyword_sha1);
  db_query("DELETE FROM {webmail_plus_solr_results_headers} WHERE uid=%d AND keyword_sha1='%s'", $user->uid, $keyword_sha1);
  
  // get Solr configs
  $solr_url = variable_get('webmail_plus_solr_url', NULL);
  
  
  $solr_url_parts = parse_url($solr_url);

  
  // load the Solr class
  //set_include_path('./'. drupal_get_path('module', 'apachesolr') .'/SolrPhpClient/');
  //include_once('Apache/Solr/Service.php');  

  
  try {

    $solr = apachesolr_get_solr($solr_url_parts['host'], $solr_url_parts['port'], $solr_url_parts['path']);
    $ping = $solr->ping();
    // If there is no $solr object, there is no server available, so don't continue.
    if (!$ping) {
      drupal_set_message('Webmail Plus could not connect to the search engine. Please notify the site administrator.', 'error', TRUE);      
      throw new Exception(t('No Solr instance available when checking requirements.'));
    }

  }
  catch (Exception $e) {
    watchdog('Webmail Plus Solr', $e->getMessage(), NULL, WATCHDOG_ERROR);
  }
  
   
  //$query_fields=array('msg_from', 'msg_to', 'msg_cc', 'msg_bcc', 'msg_subject', 'msg_body', 'msg_attach_filenames');
  /*
  $query_fields=array('msg_from', 'msg_to', 'msg_subject', 'msg_body', 'msg_attach_filenames');
  $query_array[]='drupal_uid:'.$user->uid;
  
  foreach($query_fields as $field_id=>$field_name) {
    $query_array[]=$field_name.':'.$keyword;
  }
  */
  
  
  //$query=implode(' ', $query_array);
  //$query.="fl=*,score";

  //drupal_set_message($query);
  //echo $query;
  
  $query = "drupal_uid:".$user->uid." AND ".$keyword;
  //$query = $keyword;


  // this is needed since solr 1.4
  $params = array(
  	'qt' => 'standard',
    'fl' => '*,score',
  );
  
  
  try {
    $response = $solr->search($query, 0, 9999, $params);
  }
  catch (Exception $e) {
    drupal_set_message('An error occurred at the search engine. Please notify the site administrator.', 'error', TRUE);      
    
    watchdog('Webmail Plus Solr', $e->getMessage(), NULL, WATCHDOG_ERROR);
  }

  $total = $response->response->numFound;

  
  if($total<=0) return;
  //echo "user: ".$user->uid."<br>\n";
  //echo "total: $total<br>\n";
  
  //db_query("TRUNCATE TABLE {webmail_plus_solr_results_headers}");
  
   
  $relevancy_score = variable_get('webmail_plus_solr_min_relevancy', 0.15);
  
  // insert these into the table
  foreach ($response->response->docs as $doc) {

    
    if(!preg_match("/^[0-9]{1,6}\![\w-_\.\s]{1,128}@\w{1,6}$/", $doc->id)) continue;

    //if($doc->score<$relevancy_score) continue;
    
    //wpd($doc);
    // parse message key
//    preg_match("/^([0-9]{1,6})\!([\w-_\.\s]{1,128})@(\w{1,6})$/", $doc->msg_id, $matches);
//    if($matches[1]!=$user->uid) continue;
  
    
    db_query("INSERT INTO {webmail_plus_solr_results}(uid,keyword_sha1,message_key) VALUES(%d,'%s', '%s')", $doc->drupal_uid, $keyword_sha1, $doc->id);
  }
  

  // create new ones
  db_query("INSERT INTO {webmail_plus_solr_results_headers}
SELECT 
headers .`uid`, 
results.`keyword_sha1`,
headers .`folder`,
headers.`message_key`,
headers .`subject`,
headers .`from`,
headers .`to`,
headers .`cc`,
headers .`date`,
headers .`message_id`,
headers .`size`,
headers .`message_uid`,
headers .`msgno`,
headers .`recent`,
headers .`flagged`,
headers .`answered`,
headers .`deleted`,
headers .`seen`,
headers .`draft`,
headers .`header`,
headers .`teaser`,
headers .`attachments`
FROM {webmail_plus_solr_results}  AS results
LEFT JOIN {webmail_plus_user_headers} AS headers
ON results.message_key=headers.message_key
WHERE results.`uid`=%d 
AND results.`keyword_sha1`='%s'",
 $user->uid, $keyword_sha1);
	
}

?>