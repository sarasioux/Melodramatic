<?php


function webmail_plus_download_attachment() {
  global $user, $_webmail_plus_config;
  $message_id = arg(2);
  $part_number = arg(3);
   
  $message_info = webmail_plus_parse_message_key($message_id);
  $mail_api_connection = _webmail_plus_connect($message_info->folder);

  $part_info = mail_api_get_part_object($message_info->message_uid, $part_number);
  if(empty($part_info)) {

    drupal_set_title("This attachment doesn't exist");
    $content .= "The attachmnet you're trying to download does not exist.";
    return $content;
    exit();
  } 
  
  
  // determine file name
  $file_name = 'unknown.txt';
  foreach($part_info -> parameters as $id=>$parameter_obj) { 
  	if(strtolower($parameter_obj->attribute)!='name') continue;
  	$file_name = $parameter_obj->value;
  }
  

  $part_body = mail_api_get_part($message_info->message_uid, $part_number);
   
  
  
  header("Pragma: public");
  header("Expires: 0");
  header('Content-Type: ' . mail_api_get_part_mime_type($message_info->message_uid, $part_number));
  header('Content-Length: ' . $part_info -> bytes);
  header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
  header("Cache-Control: private",false);
  //header("Content-Disposition: inline");
  header('Content-Disposition: attachment; filename="'.$file_name.'"');
  echo $part_body;
}



/*
function webmail_plus_download_attachment_form() {
  global $user, $webmail_plus_config, $mail_storage;


  $folder = arg(2);
  $mid = arg(3);
  $part = arg(4);

  $mail_storage -> active_folder(__webmail_plus_expand_folder($_SESSION['webmail_plus_active_folder']));
  $map = $mail_storage -> get_map($mid);


  $part_info = $mail_storage -> get_part_object($mid, $part);

  if(empty($part_info)) {

    drupal_set_title("This attachment doesn't exist");
    $content .= "The attachmnet you're trying to download does not exist.";
    return $content;
    exit();
  }


  $part = $mail_storage -> get_part($mid,$part);



  // write the headers
  header("Pragma: public");
  header("Expires: 0");
  header('Content-Type: ' . $mail_storage -> get_part_mime_type($mid, $part));
  header('Content-Length: ' . $part_info -> bytes);
  header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
  header("Cache-Control: private",false);
  //header("Content-Disposition: inline");
  header('Content-Disposition: attachment; filename="'.$part_info -> parameters[0]->value.'"');
  echo $part;

  //die("getting $folder/$mid/$part");
}
*/

