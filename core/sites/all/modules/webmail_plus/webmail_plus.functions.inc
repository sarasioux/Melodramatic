<?php

function webmail_plus_get_profile_node_for($user=NULL) {
  if(!$user) global $user;
  
  if (module_exists('content_profile')) {
    foreach (content_profile_get_types('names') as $type => $type_name) {
      $profile = content_profile_load($type, $user->uid);
    }
    return $profile;
  }
}

/**
message subject should always be passed thru this function
since it ensures that if it's empty it still has some text
and thus will be clickable when turned into a link
*/
function _webmail_plus_render_from($from="") {
   $from=html_entity_decode(trim($from));
   if(empty($from)) return '('.t('No sender').')';
   

   return _webmail_plus_extract_name($from);


}

function _webmail_plus_render_recepient($to="") {
   require_once 'webmail_plus.charset_functions.inc';
  
   $to=html_entity_decode(trim($to));
   

   
   if(empty($to)) return '('.t('No recipient').')';
   return _webmail_plus_extract_name($to);
   


}



/**
message subject should always be passed thru this function
since it ensures that if it's empty it still has some text
and thus will be clickable when turned into a link
*/
function _webmail_plus_render_subject($subject="") {
  require_once 'webmail_plus.charset_functions.inc';
  
   $subject=html_entity_decode(trim($subject));
   if(empty($subject)) return t('(@nosubject)', array('@nosubject' => t('No subject')));

   //echo (decodeHeader($subject));
   
   $subject = html_entity_decode(decodeHeader($subject));
   return $subject;

}

/**
date should always be passed thru this function
since it ensures that if it's empty it still has some text
and thus will be clickable when turned into a link

additionally it makes the date more presentable
*/
function _webmail_plus_render_date($date="") {

   $date=trim($date);
   if(empty($date)) return t('(No date)');

   
   return format_date($date, 'custom', 'M j, Y g:i A');
}



/*
 * returns the profile stored in content_profile module
 */
function webmail_plus_profile_node_for($uid=NULL) {

  if(is_null($uid)) {
    global $user;
    $uid=$user->uid;
  }

  $object = user_load($uid);

  if (module_exists('content_profile')) {
    foreach (content_profile_get_types('names') as $type => $type_name) {
      $profile = content_profile_load($type, $user->uid);
    }
    return $profile;
  }
}

/**
 * takes an array of availlable folders and an optional array of hidden folders. compares the two and only returns the available ones
 *
 * @param unknown_type $available
 * @param unknown_type $hidden
 * @return unknown
 */
function __webmail_plus_resolve_folder_list($available, $hidden=NULL) {
  global $_webmail_plus_config;

  
  if(!$hidden) $hidden = $_webmail_plus_config -> hidden_folders;

  // if hidden is empty no reason to bother
  if(!$hidden) return $available;
  
  
  $hidden_folders_array_raw = explode(",", $hidden);
  foreach($hidden_folders_array_raw as $key=>$value) {
  	$hidden_folders_array[trim($value)]=trim($value);
  }
  
  foreach($available as $key=>$value) {
    $key=trim($key);
    $value=trim($value);
    if($hidden_folders_array[$value]) continue;
    
    $rs[$value]=$value;
  }
  
  return $rs;
}

/**
 * returns a human-readable string of weeks, days, hours, minutes
 *
 * @param unknown_type $seconds
 * @return unknown
 */
function webmail_plus_seconds_to_human($seconds, $complete=FALSE) {
	if($seconds>604800) {
		$weeks = floor($seconds/604800);
		$seconds_left = $seconds - $weeks*604800;
	}
	else {
		$seconds_left = $seconds;
	}
	
	if($seconds_left>86400) {
		$days = floor($seconds_left/86400);
		$seconds_left = $seconds_left - $days*86400;
	}
	
	if($seconds_left>3600) {
      $hours = floor($seconds_left/3600);
      $seconds_left = $seconds_left - $hours*3600;
	}
	
	if($seconds_left>60) {
	  $minutes = floor($seconds_left/60);
	  $seconds_left = $seconds_left - $minutes*60;
	}
	
	
	if($complete) {
	  $arr[]=$weeks." ".t('weeks');
	  $arr[]=$days." ".t('days');
	  $arr[]=$hours." ".t('hours');
	  $arr[]=$minutes." ".t('minutes');
	  return implode(' ', $arr);
	}
	
	//$string = "";
	if($weeks) {
	  $arr[]=$weeks." ".t('weeks');
	  $arr[]=$days." ".t('days');
	  return implode(' ', $arr);
	}
	
	if($days) {
	  $arr[]=$days." ".t('days');
	  $arr[]=$hours." ".t('hours');
	  return implode(' ', $arr);
	}
	
	
	if($hours) {
	  $arr[]=$hours." ".t('hours');
	  $arr[]=$minutes." ".t('minutes');
	  return implode(' ', $arr);
	}
	
	if($minutes) {
	  return $minutes." ".t('minutes');
	}


}

/**
 * takes a message_key as an argument and deletes that message
 *
 * @param unknown_type $message_key
 * @param unknown_type $user
 */
function _webmail_plus_move_message($message_key, $destination, $user=NULL) {
  if(!$user) global $user;
  global $_webmail_plus_config;
  
  $message_parts = webmail_plus_parse_message_key($message_key);
  _webmail_plus_connect($message_parts->folder);
  
  $message_header = webmail_plus_parse_header(mail_api_header($message_parts->message_uid), TRUE);
  $message_header = (object)$message_header;
  
  mail_api_move_message($message_parts->message_uid, $destination);
  
  // now lets track that message and tag it
  _webmail_plus_connect($destination);     
  $overview = mail_api_overview();
  


  if(sizeof($overview)<=0) return; // something probably went wrong but it's not critical, we just lose a tag
  // since the last message will be closer to the top, reverse the array
  $overview=array_reverse($overview);

  // match headers
  foreach($overview as $id=>$message_info) {
    if(_webmail_plus_compare_headers($message_header, $message_info)) {
      //die("headers matched, remapping tags<br>\n");
      _webmail_plus_remap_tags($message_key, $user->uid.'!'.$destination.'@'.$message_info->uid);
      break;
    }

  }
  
}

function _webmail_plus_compare_headers($header1,$header2,$fields='from,to,subject,date') {

  
  $fields=trim($fields);
  if($fields=='') return FALSE;

  $fields_array=preg_split("/,/", $fields);
  $fields_proper = array_flip($fields_array);

  foreach($fields_array as $field_id=>$field) {
    $field=strtolower($field);
    //echo "checking field $field: ".html_entity_decode($header1->$field)." vs ".html_entity_decode($header2->$field)."<br>\n";

    if(html_entity_decode($header1->$field)!=html_entity_decode($header2->$field)) return FALSE;
  }


  return TRUE;
  
}



/**
 * takes a message_key as an argument and deletes that message
 *
 * @param unknown_type $message_key
 * @param unknown_type $user
 */
function _webmail_plus_delete_message($message_key, $user=NULL) {
  if(!$user) global $user;
  global $_webmail_plus_config;
  
  $message_parts = webmail_plus_parse_message_key($message_key);
  _webmail_plus_connect($message_parts->folder);
  mail_api_delete_message($message_parts->message_uid);
}

function _webmail_plus_get_user_folders($cache=TRUE, $user=NULL) {
  if(!$user) global $user;
  global $_webmail_plus_config;

  // if the flag isn't set we load the folders from the mail api even if the user specified cache
  if($_SESSION['webmail_plus']['connection_disabled']) return FALSE;
  if(!$_SESSION['webmail_plus']['folders_loaded']) $cache=FALSE;
  
  // create a list of required folders
  $required_folders_array = explode(",", $_webmail_plus_config -> required_folders);
  if(is_array($required_folders_array)) {
	  foreach($required_folders_array as $index=>$folder_name) {
	  	$required_folders[trim($folder_name)]=trim($folder_name);
	  }
  }
  else {
  	$required_folders = array();
  }
  
  
  // get them from cache
  if($cache) {
  	$results = db_query("SELECT `folder` FROM {webmail_plus_user_folders} WHERE uid=%d", $user->uid);
  	while($folder = db_fetch_object($results)) {
  		$output[$folder->folder]=$folder->folder;
  	}
  }
  else {
  	//echo "loading from mailapi";
  	
  	// fetch folders from mail_api and store them in cache
  	$mail_api_connection = _webmail_plus_connect();
  	$mailboxes = __webmail_plus_resolve_folder_list(mail_api_mailboxes());
    //wpd($mailboxes);
  	
  	// delete all folders
  	db_query("DELETE FROM {webmail_plus_user_folders} WHERE uid=%d", $user->uid);
  	
    	
    if (is_array($mailboxes)) {
      foreach($mailboxes as $mailbox_id => $mailbox_name) {
        if ($mailbox_name=="") continue;
        db_query("INSERT INTO {webmail_plus_user_folders}(uid,folder) VALUES(%d,'%s')", $user->uid, $mailbox_name);
        
        $output[$mailbox_name]=$mailbox_name;
     }
    }  	
    
    // set the session to true
    $_SESSION['webmail_plus']['folders_loaded']=TRUE;
  }
  
  
  // now translate folders if needed
  if(is_array($output)) {
    foreach($output as $folder_name=>$folder_junk) {
    	if($required_folders[$folder_name]) $output[$folder_name]=t($folder_name);
    }
  }
  
  return $output;
}

function _webmail_plus_get_newer_message($timestamp, $folder=NULL, $user=NULL) {
	if(!$user) global $user;
	if(!$folder) return FALSE;
	
	$message_key=db_result(db_query("SELECT `message_key` FROM {webmail_plus_user_headers} WHERE `date`>%d AND uid=%d AND `folder`='%s' ORDER BY `date` ASC LIMIT 1", $timestamp, $user->uid, $folder));
	if(!$message_key) {
  	return FALSE;
  }
  else {
  	return $message_key;
  }
}

function _webmail_plus_get_older_message($timestamp, $folder=NULL, $user=NULL) {
  if(!$user) global $user;
  if(!$folder) return FALSE;
  
  $message_key=db_result(db_query("SELECT `message_key` FROM {webmail_plus_user_headers} WHERE `date`<%d AND uid=%d AND `folder`='%s' ORDER BY `date` DESC LIMIT 1", $timestamp, $user->uid, $folder));
  if(!$message_key) {
    return FALSE;
  }
  else {
    return $message_key;
  }	
}

function _webmail_plus_parse_email($email) {
  if(_webmail_plus_extract_email($email)) $rs['email']=_webmail_plus_extract_email($email);
  
  list($first_name, $last_name) = split(" ", _webmail_plus_extract_name($email));
  if(trim($first_name)) $rs['first_name']=trim($first_name);
  if(trim($last_name)) $rs['last_name']=trim($last_name);
    
    
  $rs['user']=_webmail_plus_extract_user($rs['email']);
  $rs['domain']=_webmail_plus_extract_domain($rs['email']);
    
 
  return $rs;
  
}

function _webmail_plus_extract_email($email) {
	$email=trim($email);
	if(empty($email)) return FALSE;

	if(strpos($email, "&lt;") || strpos($email, "&gt;")) {
		$email = preg_replace("/\&lt\;/", "<", $email);
		$email = preg_replace("/\&gt\;/", ">", $email);
	}


	preg_match("/[_a-z0-9-]+(\.[_a-z0-9-]+)*@[a-z0-9-]+(\.[a-z0-9-]+)*(\.[a-z]{2,3})/", $email, $matches);
	
    $email = $matches[0];
    return $email;
}

/*
takes an email as an input and returns just the user part, removing the domain
*/
function _webmail_plus_extract_user($email) {

	if(!_webmail_plus_email_valid($email)) {
		return FALSE;
	}

	list($username,$domain) = split("@", $email);

	return $username;


}

function _webmail_plus_extract_domain($email) {

	if(!_webmail_plus_email_valid($email)) {
		return FALSE;
	}

	list($username,$domain) = split("@", $email);

	return $domain;


}
/*
tries to extract the name from the email
if fails returns the email
*/
function _webmail_plus_extract_name($email, $use_me=TRUE) {
  global $user, $_webmail_plus_config;
  
  
  
  $email = _webmail_plus_present_email($email);


  
  if(_webmail_plus_extract_email($email) == $user->name."@".$_webmail_plus_config -> domain  && $use_me) return "me";
  
  
  
	    
	if(preg_match("/</", $email) && preg_match("/>/", $email)) {
		$name = substr($email,0,strpos($email,"<"));

		if(preg_match("/\"/", $name)) {
			$name = preg_replace("/\"/", "", $name);
			$name = trim($name);
		}

		
		
		if($name[0]=="\"") $name=substr($name,1, strlen($name)-1);
		if($name[strlen($name)-1]=="\"") $name=substr($name,0, strlen($name)-1);
		
		
		return $name;

	} 
	else {
	  list($username, $domain) = split("@", $email);
	  return $username;
	}
	
	return;
}

function _webmail_plus_present_email($email) {
	$email = preg_replace("/\&lt\;/", "<", $email);
	$email = preg_replace("/\&gt\;/", ">", $email);
	$email = preg_replace("/&quot;/", "\"", $email);
	return $email;
}

function _webmail_plus_clean_email($email) {
  if(preg_match("/[_a-z0-9-]+(\.[_a-z0-9-]+)*@[a-z0-9-]+(\.[a-z0-9-]+)*(\.[a-z]{2,3})/", $email)) return TRUE;
  return FALSE;
}


function _webmail_plus_email_valid($email) {

	if(preg_match("/(.*)\<(.*)\>/", $email)) {
		preg_match("/[_a-z0-9-]+(\.[_a-z0-9-]+)*@[a-z0-9-]+(\.[a-z0-9-]+)*(\.[a-z]{2,3})/", $email, $matches);
		$email = $matches[0];
	}

  //echo "email $email<br>\n";

	if(!eregi("^[_a-z0-9-]+(\.[_a-z0-9-]+)*@[a-z0-9-]+(\.[a-z0-9-]+)*(\.[a-z]{2,3})$",$email)) {
		//echo "email $email is invalid<br>\n";
		return FALSE;
	} else {
		//echo "email $email is valid<br>\n";
		return TRUE;
	}

}

function _webmail_plus_check_email_address($email) {
  // First, we check that there's one @ symbol, and that the lengths are right
  if (!ereg("^[^@]{1,64}@[^@]{1,255}$", $email)) {
    // Email invalid because wrong number of characters in one section, or wrong number of @ symbols.
    return false;
  }
  
  // Split it into sections to make life easier
  $email_array = explode("@", $email);
  $local_array = explode(".", $email_array[0]);
  for ($i = 0; $i < sizeof($local_array); $i++) {
    if (!ereg("^(([A-Za-z0-9!#$%&'*+/=?^_`{|}~-][A-Za-z0-9!#$%&'*+/=?^_`{|}~\.-]{0,63})|(\"[^(\\|\")]{0,62}\"))$", $local_array[$i])) {
    return false;
    }
  }
  
  if (!ereg("^\[?[0-9\.]+\]?$", $email_array[1])) { // Check if domain is IP. If not, it should be valid domain name
    $domain_array = explode(".", $email_array[1]);
    if (sizeof($domain_array) < 2) {
      return false; // Not enough parts to domain
    }
  
    for ($i = 0; $i < sizeof($domain_array); $i++) {
      if (!ereg("^(([A-Za-z0-9][A-Za-z0-9-]{0,61}[A-Za-z0-9])|([A-Za-z0-9]+))$", $domain_array[$i])) {
        return false;
      }
    }
  }
  
  return true;
}

/**
 * generates a link that will be used to add an email address to the contact manager from the header
 *
 * @param unknown_type $email
 */
function _webmail_plus_add_to_address_book($email) {
  global $user;

  $just_email = _webmail_plus_extract_email($email);
  
  // if the email already exists, skip it

  if(module_exists('contact_manager')) {
    if(contact_manager_contact_exists($user, 'field_email', $just_email)) return;
  }

  
  $name = _webmail_plus_extract_name($email);


  if(strpos($name, " ")>0) {
    list($first_name, $last_name) = split(' ', $name);
  }
  else {
    $first_name = $name;
  }
  
  if(!$first_name && !$last_name) {
    return "<a href='".base_path()."node/add/contact-manager?field_email=$just_email'>".t('(!add)', array('!add'=>t('Add')))."</a>";
  }
  
  if(!$last_name) {
    return "<a href='".base_path()."node/add/contact-manager?field_email=$just_email&field_first_name=$first_name'>".t('(!add)', array('!add'=>t('Add')))."</a>";
  }
  
  if($first_name && $last_name) {
    return "<a href='".base_path()."node/add/contact-manager?field_email=$just_email&field_first_name=$first_name&field_last_name=$last_name'>".t('(!add)', array('!add'=>t('Add')))."</a>";
  }
  
}

function _webmail_plus_present_email_list($list) {
  global $user;
  
  if($list=="") return;
  
  $emails = explode(", ", $list);
  
  foreach($emails as $index=>$email) {
    if(_webmail_plus_add_to_address_book($email)) {
      $pieces[]=$email.' '._webmail_plus_add_to_address_book($email);
      
    }
    else {
      $pieces[]=$email;
    }
  }
  
  return implode(", ", $pieces);
}

function _webmail_plus_get_header_count($folder="INBOX", $field, $value, $user=NULL) {
  if(!$user) global $user;
  if((string)$field=='' || (string)$value=="") return;

  
  //echo $field." ".$value."<br>\n";
  
  $count=db_result(db_query("SELECT COUNT(*) FROM {webmail_plus_user_headers} WHERE uid=%d AND folder='%s' AND %s='%s'", $user->uid,$folder, $field, $value));
  
  return $count;
}

/**
 * returns a list of invalid addresses
 *
 * @param unknown_type $list
 * @return unknown
 */
function _webmail_plus_validate_email_list($list, $array=TRUE) {
	$emails = explode(",", $list);
	if(!is_array($emails)) return FALSE;
	
	// cleanup emails
	foreach($emails as $index=>$email) {
	  $email = trim($email);
	  if($email=="") continue;
	  $emails[$index]=$email;
	}
	
	// validate emails
	foreach($emails as $index=>$email) {
	  
	  if(trim($email)=="") continue;
	  if(!_webmail_plus_email_valid($email)) $invalid[]=$email;
	}
	
	

	
	if($array) {
	 return $invalid;
	}
	else {
		return implode(", ", $invalid); 
	}
}

function _webmail_plus_cleanup_email_list($list, $array=TRUE) {
  $emails = explode(",", $list);
  if(!is_array($emails)) return FALSE;
  
  foreach($emails as $index=>$email) {
    $email = trim($email);
	if($email=="") continue;
	$rs[]=$email;
  }
  
  $rs = array_unique($rs);
  
  

  
  if($array) {
    return $rs;
  }
  else {
    return implode(", ", $rs); 
  }
  
}