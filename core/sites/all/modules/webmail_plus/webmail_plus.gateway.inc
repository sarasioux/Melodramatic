<?php
function webmail_plus_gateway() {
  global $_webmail_plus_config;

  
  // see if we need to set a new active folder
  if(arg(2)) {
  	require_once 'webmail_plus.functions.inc';
  	$user_folders = _webmail_plus_get_user_folders();
  	if($user_folders[arg(2)]) $_SESSION['webmail_plus']['active_folder']=arg(2);
  }

  
  if($_webmail_plus_config->thread_messages) {
    if($_SESSION['webmail_plus']['active_folder']) {
      drupal_goto('webmail_plus/view_threads/'.$_SESSION['webmail_plus']['active_folder']); 
    }
    else {
      drupal_goto('webmail_plus/view_threads/'.$_webmail_plus_config->inbox_folder);
    }

  }
  else {
    
    if($_SESSION['webmail_plus']['active_folder']) {
      drupal_goto('webmail_plus/view_folder/'.$_SESSION['webmail_plus']['active_folder']);
      
    }
    else {
      drupal_goto('webmail_plus/view_folder/'.$_webmail_plus_config->inbox_folder);
    }
  }


}
?>