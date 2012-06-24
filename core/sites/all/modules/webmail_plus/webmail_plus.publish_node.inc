<?php
require_once 'webmail_plus.functions.inc';
/*
 * this file is conditionally includes when publishing a new node is necessary
 */


//wpd($_SESSION);

//wpd($form);
$message_key = arg(4);
//echo $message_key;

$message_info = webmail_plus_parse_key($message_key);

$mailapi_connection = _webmail_plus_connect($message_info->folder);

$header_raw = mail_api_header($message_info->message_uid);
$header_parsed = webmail_plus_parse_header($header_raw, TRUE);


//wpd($header_parsed);

$body = mail_api_get_plaintext($message_info->message_uid);
if($body=="") {
  $body = mail_api_body($message_info->message_uid);
} 

// remove crazy characters from the body
$body = preg_replace('/[^[:alnum:][:punct:]\n\s]/', '', $body);


$form['title']['#default_value']=_webmail_plus_render_subject($header_parsed['subject']);
$form['body_field']['body']['#default_value']=$body;

if(module_exists('file') && module_exists('file_attach')) {
  

  
  //wpd($_SESSION);
  
  $attachments = mail_api_get_attachment_files($message_info->message_uid);
  
  //wpd($attachments);
  
  // save the attachments

  if(is_array($attachments)) {
    /*
    $form['attachments']['#type']='fieldset';
    $form['attachments']['#title']=t('Attachments');
    $form['attachments']['#collapsible']=TRUE;
    $form['attachments']['#collapsed']=FALSE;
    
  
    $form['attachments']['webmail_plus_message_key']= array(
      '#type' => 'hidden',
      '#value' => $message_key
    );
    */
    
    // define files object
    $files_obj = new stdClass();
    $files_obj -> files = array();
        
    foreach($attachments as $attachment_id=>$attachment_info) {
        //wpd($attachment_info);
      
        //echo file_directory_temp();
        
        // fetch the attachment
        $part_body = mail_api_get_part($message_info->message_uid, $attachment_info['part_number']);
        // and store it
        if(!file_put_contents(file_directory_temp().'/'.$attachment_info['file'], $part_body)) {
          
        }
        else {
          $node = (object)array('nosave' => TRUE);
          
          $file = (object)array('filepath' => file_directory_temp().'/'.$attachment_info['file'], 'filesize' => $attachment_info['size']);
   
  
          file_node_save($node, $file);
          $file->size = $file->filesize;
          $file->type = $file->filemime;
          $file->name = $attachment_info['file'];
          $file->list = 1;
          $file->sid = 's_'. (!empty($_SESSION['file_attach_files']) ? max(array_map(create_function('$a', 'return substr($a, 2);'), array_keys($_SESSION['file_attach_files']))) + 1 : '0');
          
          //print_r($file);
          
          //file_node_save($node, $this_file_obj);
          $files_obj -> files[$file->sid] = $file;
          $_SESSION['file_attach_files'][$file->sid] = $file;
        }      
        
    }
    
    $form_attach = _file_attach_form($files_obj);
    
    // only merge arrays if there are attachments
    if(is_array($form['attachments']['wrapper']['files']) && $form_attach['files']) {
    
      $form['attachments']['wrapper']['files'] = array_merge($form['attachments']['wrapper']['files'] , $form_attach['files']);
    }
  }

}

