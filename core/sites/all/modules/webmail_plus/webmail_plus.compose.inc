<?php

// $Id: webmail_plus.compose.inc,v 1.1.2.7 2009/05/14 18:30:30 firmidea Exp $


function webmail_plus_compose_form() {
  global $user, $_webmail_plus_config, $_webmail_plus_user_preferences;
  
  
  unset($_SESSION['file_attach_files']);
  unset($_SESSION['webmail_plus']['this_message']['attachments']);
  
  //wpd($_SESSION);
  
  drupal_add_js(drupal_get_path('module', 'webmail_plus') .'/js/webmail_plus.js');
  drupal_add_js(drupal_get_path('module', 'webmail_plus') .'/js/compose.js');

  // FIXME integrating Content Profiles
  require_once 'webmail_plus.functions.inc';


  //webmail_plus_dump($node);

  $form=array();
  /*
  $form['#attributes'] = array(
      'onSubmit' => 'return webmail_plus_validate_compose()'
  );
  */

  
  $form['top_buttons'] = array(
    '#type' => 'fieldset'
  );

  // dummy
  $form['top_buttons']['dummy'] = array(
    '#type' => 'submit',
    '#prefix' => "<span style='visibility:hidden'>",
    '#suffix' => "</span>"
  );
  
  // top buttons
  $form['top_buttons']['send'] = array(
    '#type' => 'submit',
    '#title' => t('Send'),
    '#value' => t('Send'),
    '#id' => 'send_top',
    '#attributes' => array(
      'onClick' => 'return webmail_plus_validate_compose()'  
      )
  );

  $form['top_buttons']['save_draft'] = array(
    '#type' => 'submit',
    '#title' => t('Save Now'),
    '#value' => t('Save Now'),
    '#id' => 'save_draft',
  );
  
  $form['top_buttons']['discard'] = array(
    '#type' => 'submit',
    '#title' => t('Discard'),
    '#value' => t('Discard'),
    '#id' => 'discard',    
    '#attributes' => array(
      'onClick' => 'return webmail_plus_discard_compose()'
    ),
  
  );
  
  $action = arg(2);
  $message_key = arg(3);
  $to_field = arg(4);
  $cc_field = arg(5);
  $bcc_field = arg(6);
  $subject_field = arg(7);
  $body_field = arg(8);
  
  
  $form['message_key'] = array(
    '#type' => 'hidden',
    '#value' => $message_key
  );
  
  
  $message_type = webmail_plus_message_type($message_key);


  if($action=='reply' || $action=='replyall' || $action=='forward') {

    if($message_type=='thread') {
      
      $thread_info = webmail_plus_parse_thread_key($message_key);
//      $query="SELECT * FROM webmail_plus_user_threads_map WHERE tid=$thread_info->thread_id AND uid=$user->uid AND folder='$thread_info->folder'";
//      echo $query;
      
      $thread_messages = db_query("SELECT tid,uid,folder,message_uid, CONCAT(uid,'!',folder,'@',message_uid) AS message_key FROM {webmail_plus_user_threads_map} WHERE tid=%d AND uid=%d AND folder='%s'", $thread_info->thread_id, $user->uid, $thread_info->folder);

      while($thread_message = db_fetch_object($thread_messages)) {
        //print_r($thread_message);
        $last_folder = $thread_message->folder;
        $last_message_uid = $thread_message->message_uid;
      }


      $mail_api_connection = _webmail_plus_connect($last_folder);
      $message_header_raw = mail_api_header($last_message_uid);
      $message_header_arr = webmail_plus_parse_header($message_header_raw, TRUE);

      $message_body = mail_api_get_plaintext($last_message_uid);
      if($message_body=="") $body = mail_api_body($last_message_uid);
      
      
    }
    elseif($message_type=='message') {
      $message_info = webmail_plus_parse_message_key($message_key);

      $mail_api_connection = _webmail_plus_connect($message_info->folder);
      $message_header_raw = mail_api_header($message_info->message_uid);
      $message_header_arr = webmail_plus_parse_header($message_header_raw, TRUE);
      $message_body = mail_api_get_plaintext($message_info->message_uid);
      if($message_body=="") $message_body = mail_api_body($message_info->message_uid);
      
      

//      require_once 'webmail_plus.charset_functions.inc';
//      $subject = $message_header_arr['subject'];
//      echo "decoded header: ".decodeHeader($subject);
      
      //$message_header_arr['subject']=decodeHeader($message_header_arr['subject']);
      
      
    }
  }
    
    
    switch($action) {
  
      case 'reply':
        if(preg_match("/&lt;(.*)&gt;/", $message_header_arr['from'], $frommatch)) {
	      $message_to = $frommatch[1] .', '; 
		} 
		else {
		  $message_to = $message_header_arr['from'];
		}
        $message_subject = t('Re:')." ".$message_header_arr['subject'];
        $message_body = webmail_plus_prepend_text($message_body);
        
        break;
      
      case 'replyall':
        if(preg_match("/&lt;(.*)&gt;/", $message_header_arr['from'], $frommatch)) {
	      $message_to = $frommatch[1] .', ';; 
        } 
		else {
		  $message_to = $message_header_arr['from'];
		}
		
        $message_cc=$message_header_arr['cc'];
        $message_subject = t('Re:')." ".$message_header_arr['subject'];
        $message_body = webmail_plus_prepend_text($message_body);
        
        break;
        
      case 'forward':
        $message_to='';
        $message_subject = t('Fwd:')." ".$message_header_arr['subject'];   
        $message_body = webmail_plus_prepend_text($message_body);
         
        break;
        
      case 'prefill':
        $message_to = $to_field;
        $message_cc = $cc_field;
        $message_bcc = $bcc_field;
        $message_subject = $subject_field;
        $message_body = $body_field;       
        break;
        
      case 'draft':
        $message_info = webmail_plus_parse_message_key($message_key);
  
        $mail_api_connection = _webmail_plus_connect($message_info->folder);
        $message_header_raw = mail_api_header($message_info->message_uid);
        $message_header_arr = webmail_plus_parse_header($message_header_raw, TRUE);
        $body = mail_api_get_plaintext($message_info->message_uid);
        
        $message_to = $message_header_arr['to'];
        $message_cc = $message_header_arr['cc'];
        $message_bcc = $message_header_arr['bcc'];
        $message_subject = $message_header_arr['subject'];
        
        $message_body = $body;          
        break;
        
      default:
        $message_to = $_SESSION['webmail_plus']['this_message']['to'];
        $message_cc = $_SESSION['webmail_plus']['this_message']['cc'];
        $message_bcc = $_SESSION['webmail_plus']['this_message']['bcc'];
        $message_subject = $_SESSION['webmail_plus']['this_message']['subject'];
        $message_body = $_SESSION['webmail_plus']['this_message']['body'];
    
    }
   


  // take parameters from GET
  if($_GET['to']) $message_to=$_GET['to'];
  if($_GET['cc']) $message_cc=$_GET['cc'];
  if($_GET['bcc']) $mesasge_bcc=$_GET['bcc'];
  if($_GET['subject']) $message_subject=$_GET['subject'];
  if($_GET['body']) $message_body=$_GET['body'];
  
  
      
  $form['to'] = array(
    '#type' => 'textfield',
    '#title' => t('To'),
    '#default_value' => html_entity_decode($message_to),
    '#id' => 'email_to',
    '#size' => 105,
    '#maxlength' => 1048576,
    '#description' => t('Provide a list of comma-separated email addresses')
  );
  
  $form['ccbcc'] = array(
    '#type' => 'item',
	'#value' => '<div class="hideshow-buttons"><a href="cc-container">'.t('Add CC').'</a><a href="bcc-container">'.t('Add BCC').'</a></div>',
  );
	
  $form['cc']['cc'] = array(
 	'#prefix' => '<div class="cc-container hide-form">',
	'#suffix' => '</div>',
    '#type' => 'textfield',
    '#title' => t('CC'), 
    '#default_value' => html_entity_decode($message_cc),
    '#id' => 'email_cc',
    '#size' => 105,
    '#maxlength' => 1048576,
    '#description' => t('Provide a list of comma-separated email addresses')  
  );


  $form['bcc']['bcc'] = array(
	'#prefix' => '<div class="bcc-container hide-form">',
	'#suffix' => '</div>',
    '#type' => 'textfield',
    '#title' => t('BCC'),
    '#default_value' => html_entity_decode($message_bcc),
    '#id' => 'email_bcc',
    '#size' => 105,
    '#maxlength' => 1048576,
    '#description' => t('Provide a list of comma-separated email addresses')  

  
  );
  
  
  //echo $_webmail_plus_config->address_book;
  //echo _webmail_plus_get_autocomplete_url();
  
  // enable autocomplete if spawar_support is available
  if($_webmail_plus_config->address_book) {
    /*
    $form['to']['#autocomplete_path'] = _webmail_plus_get_autocomplete_url();
    $form['cc']['cc']['#autocomplete_path'] = _webmail_plus_get_autocomplete_url();
    $form['bcc']['bcc']['#autocomplete_path'] = _webmail_plus_get_autocomplete_url();
	*/
    $form['to']['#autocomplete_path'] = 'webmail_plus/address_autocomplete';
    $form['cc']['cc']['#autocomplete_path'] = 'webmail_plus/address_autocomplete';
    $form['bcc']['bcc']['#autocomplete_path'] = 'webmail_plus/address_autocomplete';
  }  
  

  $form['subject'] = array(
    '#type' => 'textfield',
    '#title' => t('Subject'),
    '#default_value' => html_entity_decode($message_subject),
    '#id' => 'email_subject',
    '#maxlength' => 1048576
  );

  if($message_cc) {
    $cc_collapsed = FALSE;
  }
  else {
    $cc_collapsed = TRUE;
  }

  
  // append a signature if it's allowed and it's set
  $user_preferences = webmail_plus_get_user_preferences($user->uid);
  
  if($_webmail_plus_config->allow_signature && $user_preferences->email_signature_append && $user_preferences->email_signature) {
  	$message_body.="\n\n--\n".$user_preferences->email_signature; 
  }
  
  
  $message_body = preg_replace('/[^[:alnum:][:punct:]\n\s]/', '', $message_body);

  
  
  $form['body'] = array(
    '#type' => 'textarea',
    '#title' => t('Message'),
    '#default_value' => $message_body,
    '#id' => 'email_body'
  );
  
  

  /*
  _file_attach_form_fieldset($form, NULL);
  $form['attachments']['#collapsed']=false;
  $form['attachments']['wrapper'] += _file_attach_form(NULL);
  */
  
  //wpd($form);
  // attach a signature if it's configured
  //webmail_plus_dump($_webmail_plus_config);

  

  $form['attachments'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('File attachments'),
    '#collapsible' => TRUE,
    '#collapsed'   => empty($object->files),
    '#description' => t('Changes made to the attachments are not permanent until you save this post. "Delete" only removes attachment from the node, but does not delete the attached file. The first "listed" file will be included in RSS feeds.'),
    '#prefix'      => '<div class="attachments">',
    '#suffix'      => '</div>',
    '#weight'      => 1,
  );

  $form['attachments']['wrapper'] = array(
    '#prefix' => '<div id="file-attach-wrapper">',
    '#suffix' => '</div>',
  );
  $form['attachments']['wrapper'] += _file_attach_form($object);

  

  if(arg(2)=='forward' || arg(2)=='draft') {
    $attachments = mail_api_get_attachment_files($message_info->message_uid);

    //wpd($attachments);
    if(is_array($attachments)) {
      $files_obj = new stdClass();
      $files_obj -> files = array();
      
        foreach($attachments as $attachment_id=>$attachment_info) {
          //wpd($attachment_info);
        
          //echo file_directory_temp();
          
          // fetch the attachment
          $part_body = mail_api_get_part($message_info->message_uid, $attachment_info['part_number']);
          // and store it
     
          if(!file_put_contents(file_directory_temp().'/'.$attachment_info['file'], $part_body)) {
             drupal_set_message(t("Error saving file %file. Please, contact site administrator.", array('%file' => $attachment_info['file'])), 'error');
          }
          /*
          $file_bitcache_id = bitcache_put_file(NULL, file_directory_temp().'/'.$attachment_info['file']);
          $this_file_obj = new stdClass();
          $this_file_obj->list = 1;
          $this_file_obj -> sid = 's_'. (!empty($_SESSION['file_attach_files']) ? max(array_map(create_function('$a', 'return substr($a, 2);'), array_keys($_SESSION['file_attach_files']))) + 1 : '0');
          $this_file_obj -> nid = NULL;
          $this_file_obj -> vid = NULL;
          $this_file_obj -> uri = 'bitcache://'.$file_bitcache_id;
          $this_file_obj -> size = $attachment_info['size'];
          $this_file_obj -> name = $attachment_info['file'];
          $this_file_obj -> weight = 0;
          //$this_file_obj -> nosave = 1;
          
          _file_attach_file($form_state, $node->file);
          
          $files_obj -> files['n_'.$attachment_id] = $this_file_obj;
          $_SESSION['file_attach_files']['s_'.$attachment_id] = $this_file_obj;    
          */
          
          $node = (object)array('nosave' => TRUE);
          $file = (object)array('filepath' => file_directory_temp().'/'.$attachment_info['file'], 'filesize' => $attachment_info['size']);
          file_node_save($node, $file);
          $file->size = $file->filesize;
          $file->type = $file->filemime;
          $file->name = $attachment_info['file'];
          $file->list = 1;
          $file->sid = 's_'. (!empty($_SESSION['file_attach_files']) ? max(array_map(create_function('$a', 'return substr($a, 2);'), array_keys($_SESSION['file_attach_files']))) + 1 : '0');
          
          $files_obj -> files[$file->sid] = $file;
          $_SESSION['file_attach_files'][$file->sid] = $file;
        }
        
        //wpd($files_obj);
        
        $form_attach = _file_attach_form($files_obj);
        _file_attach_form_fieldset($form, $form_attach);
        $form['attachments']['wrapper']['files'] = array_merge($form['attachments']['wrapper']['files'] , $form_attach['files']);
 
    }    
    
  }




  $form['bottom_buttons'] = array(
    '#type' => 'fieldset',
    '#weight' => 2
  );

  $form['bottom_buttons']['send'] = array(
    '#type' => 'submit',
    '#title' => t('Send'),
    '#value' => t('Send'),
    '#id' => 'send_bottom',
    '#attributes' => array(
      'onClick' => 'return webmail_plus_validate_compose()'
    ),
  );

  $form['bottom_buttons']['discard'] = array(
    '#type' => 'submit',
    '#title' => t('Discard'),
    '#value' => t('Discard'),
    '#id' => 'discard',

  );



  $form['#attributes']['enctype'] = 'multipart/form-data';
  //$form['#submit'][] = 'webmail_plus_file_attach_form_submit';
  
  unset($form['#submit']);
  //wpd($form);
  return $form;

}


/**
 * this handles attachment uploading
 *
 * @param unknown_type $form
 * @param unknown_type $form_state
 * @return unknown
 */

/*
function webmail_plus_save_attachment($form, &$form_state) {
  global $user;

  $clicked_button = $form_state['clicked_button']['#id'];

  if($clicked_button!='upload') return FALSE;

  $_SESSION['webmail_plus']['this_message']['to'] = $form_state['values']['to'];
  $_SESSION['webmail_plus']['this_message']['cc'] = $form_state['values']['cc'];
  $_SESSION['webmail_plus']['this_message']['bcc'] = $form_state['values']['bcc'];
  $_SESSION['webmail_plus']['this_message']['subject'] = $form_state['values']['subject'];
  $_SESSION['webmail_plus']['this_message']['body'] = $form_state['values']['body'];


  // make sure the directory exists
  $this_message_files = file_directory_path().'/webmail_plus/'.$user->uid.'/this_message';
  file_check_directory($this_message_files, 1);

  move_uploaded_file($_FILES['files']['tmp_name']['upload_file'], $this_message_files.'/'.$_FILES['files']['name']['upload_file']);

  $file_size = filesize($this_message_files.'/'.$_FILES['files']['name']['upload_file']);
  $_SESSION['webmail_plus']['this_message']['attachments'][] = array(
  	'file' => $_FILES['files']['name']['upload_file'],
    'size' => $file_size
  );

}
*/

function webmail_plus_compose_form_validate($form, &$form_state) {

  // skip validation
  $clicked_button = $form_state['clicked_button']['#id'];
  if($clicked_button!='send_top' && $clicked_button!='send_bottom') return;

  if(empty($form_state['values']['to'])) {
    form_set_error('', t('The To field can not be empty.'));
  }
  
  //wpd($form_state);
  
  require_once 'webmail_plus.functions.inc';
  
  $invalid_to_emails = _webmail_plus_validate_email_list($form_state['values']['to']);
  if(is_array($invalid_to_emails)) {
  	form_set_error('to', t('The following recepient addresses are invalid: %invalid_list.', array('%invalid_list' => implode(", ", $invalid_to_emails))), 'error', FALSE);
  }
  
  if(!empty($form_state['values']['cc'])) {
	  $invalid_cc_emails = _webmail_plus_validate_email_list($form_state['values']['cc']);
	  if(is_array($invalid_cc_emails)) {
	    form_set_error('cc', t('The following recepient addresses are invalid: %invalid_list.', array('%invalid_list' => implode(", ", $invalid_cc_emails))), 'error', FALSE);
	  }
  }

  if(!empty($form_state['values']['bcc'])) {
    $invalid_bcc_emails = _webmail_plus_validate_email_list($form_state['values']['bcc']);
    if(is_array($invalid_bcc_emails)) {
      form_set_error('bcc', t('The following recepient addresses are invalid: %invalid_list.', array('%invalid_list' => implode(", ", $invalid_bcc_emails))), 'error', FALSE);
    }
  }
  
  
}


/**
 * hook_submit
 *
 * @param unknown_type $form
 * @param unknown_type $form_state
 */
function webmail_plus_compose_form_submit($form, &$form_state) {

//  wpd($_SESSION);
//  wpd($form_state);
//  die ();
  

  

  global $user, $_webmail_plus_config, $_webmail_plus_user_preferences;


  // only authenticated users can send email
  if($user->uid<=0) return FALSE;

  
  
    
  $clicked_button = $form_state['clicked_button']['#id'];

  

  switch($clicked_button) {
    case 'discard':
      unset($_SESSION['webmail_plus']['this_message']);
      $form_state['redirect']='webmail_plus/gateway';
      break;

    case 'send_top':
    case 'send_bottom':
    case 'save_draft':


      
      require_once "vendor/PHPMailer/class.phpmailer.php";
      $mail = new PHPMailer();
      $mail->SMTPDebug = 0;

      if($_webmail_plus_config->send_method=='smtp') {

        if($_webmail_plus_config->debug) watchdog('webmail_plus', 'Sending via SMTP to %smtp_host:%smtp_port', array('%smtp_host'=>$_webmail_plus_config->smtp_host, '%smtp_port'=>$_webmail_plus_config->smtp_port));

        $mail->IsSMTP();

        if($_webmail_plus_config -> send_encryption == 'ssl') {
          $mail->SMTPSecure = "ssl";
        }

        //webmail_plus_dump($_webmail_plus_config);
        $mail->Host = $_webmail_plus_config->smtp_host;
        $mail->Port = $_webmail_plus_config->smtp_port;


        // use SMTP authentication
        if($_webmail_plus_config->smtp_username && $_webmail_plus_config->smtp_password) {

          //drupal_set_message('user and password are set');
          $mail->SMTPAuth = true;
          $mail->Username   = $_webmail_plus_config->smtp_username;
    	    $mail->Password   = $_webmail_plus_config->smtp_password;

	    	  if($_webmail_plus_config -> smtp_encryption != 'none') {
	    	  	$mail->SMTPSecure = "ssl";
	    	  }
        }

      }
      else {
        if($_webmail_plus_config->debug) drupal_set_message('sending via Sendmail');

        $mail->IsSendmail();
      }

      $sender = __webmail_plus_get_sender();

      
      //drupal_set_message('the body is '.$form_state['values']['body']);
      
      $mail->From       = $sender['email'];
      $mail->Sender     = $sender['email'];
      $mail->FromName   = $sender['name'];
      $mail->Subject    = $form_state['values']['subject'];

      
      // process TO addresses
      $email_to_list = _webmail_plus_cleanup_email_list($form_state['values']['to'], TRUE);
      foreach($email_to_list as $index=>$email) {
      	$mail->AddAddress($email);
      }
      
      //$mail->AddAddress($form_state['values']['to']);

      
      // process CC addresses
      if($form_state['values']['cc']) {
      	$email_cc_list = _webmail_plus_cleanup_email_list($form_state['values']['cc'], TRUE);
        foreach($email_cc_list as $index=>$email) {
          $mail->AddCC($email);
        }      	
      }

      // process BCC addresses
      if($form_state['values']['bcc']) {
        $email_bcc_list = _webmail_plus_cleanup_email_list($form_state['values']['bcc'], TRUE);
        foreach($email_bcc_list as $index=>$email) {
          $mail->AddBCC($email);
        }       
      }
      
      // figure out the email format
      if($_webmail_plus_config->email_format_override) {

        if(!$_webmail_plus_user_preferences->email_format) {
          $email_format = $_webmail_plus_config->email_format;
        }
        else {
          $email_format = $_webmail_plus_user_preferences->email_format;
        }

      }
      else {
        $email_format = $_webmail_plus_config->email_format;
      }

      
      //drupal_set_message('email format is '.$email_format);

    
      if($email_format == 'html') {
        //drupal_set_message('message is html');
        $mail->MsgHTML($form_state['values']['body']);
        $mail->IsHTML(true); // send as HTML
        $mail->AltBody    = $form_state['values']['body'];
        $mail->Body       = preg_replace("/\n/", "<br>\n", $form_state['values']['body']);
        $mail->WordWrap   = 50; // set word wrap
        
      }
      else {
        //drupal_set_message('message is plain text');
        $mail->IsHTML(false);
        $mail->Body       = $form_state['values']['body'];
 
      }

      // process the form
      webmail_plus_file_attach_form_submit($form, $form_state);
      
      
      // handle attachments

      //wpd($_SESSION['webmail_plus']['this_message']['attachments']);
      
      if(sizeof($_SESSION['webmail_plus']['this_message']['attachments'])>0) {
        $this_message_files = file_directory_path().'/webmail_plus/'.$user->uid.'/this_message';

        foreach($_SESSION['webmail_plus']['this_message']['attachments'] as $attachment_id=>$attachment_info) {
          $mail->AddAttachment($this_message_files.'/'.$attachment_info['file']);
        }
      }


      
      if($clicked_button=='send_top' || $clicked_button=='send_bottom') {
	      // send the message
	      if(!$mail->Send()) {
	        drupal_set_message(t('Your message %subject was not sent due to the following error: %error', array('%subject' => $form_state['values']['subject'], '%error'=>$mail->ErrorInfo)), 'error', FALSE);
	      } else {
	        drupal_set_message(t('Your message %subject was sent', array('%subject' => $form_state['values']['subject'])), 'warning', FALSE);
	      }
	      
	      
	      // connect to mail storage Sent folder
          _webmail_plus_connect(mail_api_sent_path($_webmail_plus_config->domain));
          mail_api_append($mail -> ExportHeader.$mail->ExportBody);
      
          // mark the message as sent
          $overview = mail_api_overview();

          
          if($overview) {
            $max_message = sizeof($overview)-1;
            //wpd($overview[$max_message]);
            //drupal_set_message('marking '.$overview[$max_message]->uid);
            $mark_read = mail_api_flag_read($overview[$max_message]->uid);
          }
          
    
          // clear the session only if the send button was clicked
          if($form_state['clicked_button']['#id']=='send') {
            unset($_SESSION['webmail_plus']['this_message']);
          }
    
          // delete the previous draft if there was one
          _webmail_plus_delete_draft($form_state);         
          
          	     
          // clear the cache
          __webmail_plus_clear_this_message();

         // send the user back to where he came from
         drupal_goto('webmail_plus/gateway');
      
      }
      elseif($clicked_button=='save_draft') {
      	// save the draft
      	
        require_once 'webmail_plus.functions.inc';
        
        
      	$mail->Simulate=TRUE;
      	$mail->Send();
      	
      	_webmail_plus_connect(mail_api_drafts_path($_webmail_plus_config->domain));
      	//wpd($mail -> ExportHeader.$mail->ExportBody);
      	
      	// delete the previous draft if there was one
      	_webmail_plus_delete_draft($form_state);
      	
      	// append the message to the folder
      	mail_api_append($mail -> ExportHeader.$mail->ExportBody);
        
      	// mark the message as sent
        $overview = mail_api_overview();

          
        if($overview) {
          $max_message = sizeof($overview)-1;
          $mark_read = mail_api_flag_read($overview[$max_message]->uid);
          $message_key = $user->uid."!".mail_api_drafts_path($_webmail_plus_config->domain)."@".$overview[$max_message]->uid;
        }     	

        $form_state['redirect']='webmail_plus/compose/draft/'.$message_key;
        //drupal_goto("webmail_plus/compose/draft/".$message_key);
      	
      }

      



      break;

    default:
      watchdog('webmail_plus', 'i dunno what is going on');
      break;
  }


}

/**
 * takes multi-line text and prepends a prefix to each line
 *
 * @param unknown_type $text
 * @param unknown_type $prefix
 * @return unknown
 */
function webmail_plus_prepend_text($text, $prefix="> ") {
  return $prefix.preg_replace("/\n/", "\n".$prefix, $text);
}



/*
 * returns an array with sender name and email
 */
function __webmail_plus_get_sender() {
  global $user, $_webmail_plus_config, $_webmail_plus_user_preferences;


  // pull in values from profile module if it's active
  if(module_exists('profile') && $_webmail_plus_config->user_from!=NULL) {
    profile_load_profile($user);

    // create a variable map
    foreach($user as $key=>$value) {
      if(is_array($value)) continue;
      if(preg_match('/^profile\_/', $key)) {
        $profile_var_map['@'.$key]=$value;
      }
    }

    $sender['email'] = $user->name.'@'.$_webmail_plus_config -> domain;
    $sender['name'] = t($_webmail_plus_config->user_from, $profile_var_map);
  }
  elseif (module_exists('content_profile') && $_webmail_plus_config->user_from!=NULL) {

      $profile = webmail_plus_get_profile_node_for($user);

      foreach (content_profile_get_types('names') as $type => $type_name) {
        $node = content_profile_load($type, $user->uid);
      }

      if(is_array($node)) {
        // first flatten the array
        foreach($node as $key=>$value) {
          if(is_array($value) && sizeof($value)==1 && (is_string($value['0']['value']) || is_null($value['0']['value']))) {
            $new[$key]=$value['0']['value'];
          }
          else {
            $new[$key]=$value;
          }
        }

        foreach($new as $key=>$value) {
          if(is_array($value)) continue;
          $profile_var_map['@'.$key]=$value;
        }
        $sender['email'] = $user->name.'@'.$_webmail_plus_config -> domain;
        $sender['name'] = t($_webmail_plus_config->user_from, $profile_var_map);
      }
      else {
        $sender['email'] = $user->name.'@'.$_webmail_plus_config->domain;
        $sender['name'] = NULL;
      }



    }
    else {
      $sender['email'] = $user->name.'@'.$_webmail_plus_config->domain;
      $sender['name'] = NULL;

    }

  return $sender;
}


function webmail_plus_file_attach_form_submit($form, &$form_state) {
  global $user;
  
  
  if(sizeof($form_state['values']['files'])<=0) return;
 
  foreach($form_state['values']['files'] as $file_id=>$file_info) {

    if($file_info['remove']==1) continue;

    //wpd($file_info['uri']);
 
    
    preg_match('/bitcache:\/\/([a-z0-9]{40})/', $file_info['uri'], $matches);

    //wpd($matches);
    $bitcache_id = $matches[1];
    //wpd($bitcache_id);
    $bitcache_path =  bitcache_get($bitcache_id)->path;
    //wpd($bitcache_path);
    
    //die();
    
    _webmail_plus_create_directory(file_directory_path().'/webmail_plus/'.$user->uid.'/this_message/');
    
    //drupal_set_message('copying '.$bitcache_path.' to '.file_directory_path().'/webmail_plus/'.$user->uid.'/this_message/'.$file_info['name']);
    copy($bitcache_path, file_directory_path().'/webmail_plus/'.$user->uid.'/this_message/'.$file_info['name']);
    //drupal_set_message('adding '.$file_info['name'].' to attachments');
    
    // prevent duplicate attachments from being included
    $attachment_sha1=sha1_file(file_directory_path().'/webmail_plus/'.$user->uid.'/this_message/'.$file_info['name']);
    if($_SESSION['webmail_plus']['this_message']['attachments']) {
      foreach($_SESSION['webmail_plus']['this_message']['attachments'] as $attachment_id=>$attachment_info) {
        if($attachment_info['file']==$file_info['name']) {
          if($attachment_info['sha1']==$attachment_sha1) continue(2);
        }
      }
    }
    
    
    // add it to the session 
    $_SESSION['webmail_plus']['this_message']['attachments'][] = array
    (
       'file' => $file_info['name'],
       'size' => filesize(file_directory_path().'/webmail_plus/'.$user->uid.'/this_message/'.$file_info['name']),
       'sha1' => $attachment_sha1
    );
    
    // remove only temporary files, do not remove nodes
    if(!$file_info['nid'] && !$file_info['vid']) bitcache_delete($bitcache_id);
    
  }

}

function __webmail_plus_clear_this_message() {
  if(sizeof($_SESSION['webmail_plus']['this_message']['attachments'])>0) {
    $this_message_files = file_directory_path().'/webmail_plus/'.$user->uid.'/this_message';

    foreach($_SESSION['webmail_plus']['this_message']['attachments'] as $attachment_id=>$attachment_info) {
      @unlink($this_message_files.'/'.$attachment_info['file']);
    }
  }

  unset($_SESSION['webmail_plus']['this_message']);

  return TRUE;
}

function _webmail_plus_delete_draft($form_state) {
  global $_webmail_plus_config;
  require_once 'webmail_plus.functions.inc';
  
  if($form_state['values']['message_key']) {
     $old_message_key = $form_state['values']['message_key'];
     $old_message_parts = webmail_plus_parse_message_key($old_message_key);
      	  
     // if it's a draft, delete it
     if($old_message_parts->folder==mail_api_drafts_path($_webmail_plus_config->domain)) {
      _webmail_plus_delete_message($old_message_key);
    }
  }
}
