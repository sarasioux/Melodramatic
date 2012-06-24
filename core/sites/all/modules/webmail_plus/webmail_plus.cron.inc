<?php
  global $_webmail_plus_config;
    

  
  // clear the session since we won't be needing it
    
  unset($_SESSION['webmail_plus']);
  if($_webmail_plus_config->debug) echo 'webmail_plus_cron<br>';
    
    
  if($_webmail_plus_config->use_cron!=1) return;

  // see how often we can fetch
  $cron_fetch_frequency = variable_get('webmail_plus_cron_frequency', 60);
  $cron_batch_size =  variable_get('webmail_plus_cron_batch_size', 0);
  
  // figure out which batch to run
  if($cron_batch_size>0) {
    $last_batch = variable_get('webmail_plus_cron_last_batch', 0);
    $unprocessed = db_result(db_query("SELECT COUNT(*) FROM {webmail_plus_user_cron} WHERE batch!=%d", $last_batch));
    if($unprocessed<=0) {
      $this_batch = $last_batch+1;
    }
    else {
      $this_batch = $last_batch;
    }
  }
  
  // only fetch users that have a password
  if($cron_batch_size>0) {
    
    if($_webmail_plus_config->debug) echo "running in batch mode for batch $this_batch<br>\n";
    
    $users = db_query("SELECT p.uid,p.password
      FROM webmail_plus_user_passwords AS p
      LEFT JOIN webmail_plus_user_cron AS c
      ON p.uid=c.uid
      WHERE c.batch!=%d OR c.batch IS NULL 
      LIMIT %d", $this_batch, $cron_batch_size);  
      
      // update the batch id
      variable_set('webmail_plus_cron_last_batch', $this_batch);
  }
  else {
      $users = db_query("SELECT uid,password FROM {webmail_plus_user_passwords} WHERE password!=''");
  }

 
  
  // go thru all users and fetch their messages
  $iterration=1;
  while($user_info = db_fetch_object($users)) {
    // see when we checked mail last time
    $user_last_checked = db_result(db_query("SELECT `checked` FROM {webmail_plus_user_cron} WHERE uid=%d", $user_info->uid));
    
    // if this was recently checked then skip it
    $difference = time()-strtotime($user_last_checked);
    
    if($_webmail_plus_config->debug) echo "difference: $difference<br>\n";
    
    
    if($difference<=$cron_fetch_frequency) {
      if($_webmail_plus_config->debug) echo "skipping user ".$user_info->uid." since he was recently checked<br>\n";
      continue;
    } else {
      if($_webmail_plus_config->debug) echo "checking user $user_info->uid...<br.\n";
    }
    
    $user = user_load($user_info->uid);
    if($_webmail_plus_config->debug) echo "fetching messages for ".$user->name."<br>\n";
    $password = _webmail_plus_get_user_password($user);
    if($_webmail_plus_config->debug) echo "password is $password<br>\n";
      
    if($_webmail_plus_config->debug) wpd($user);
    _webmail_plus_update_messages("INBOX", $user);
    
    // update the user 
    if($cron_batch_size>0) {
      _webmail_plus_cron_update($user, $this_batch);
      
    }
    else {
      _webmail_plus_cron_update($user);
    }


    // if we're in batch mode make sure we quit after n number of checks
    if($cron_batch_size>0) {
        $iterration++;
        if($iterration>=$cron_batch_size) break;
    }
  }



/*
function _webmail_plus_cron_update($user=NULL, $batch=NULL) {
  if(!$user) global $user;
  if(!$user->uid) return;
  global $_webmail_plus_config;
 
  $timestamp = date("Y-m-d H:i:s");
  
  if(_webmail_plus_cron_get($user)) {
    if($_webmail_plus_config->debug) echo "updating existent<br>\n";
    
    if($batch>0) {
      db_query("UPDATE {webmail_plus_user_cron} SET checked='%s', batch=%d WHERE uid=%d", $timestamp, $batch, $user->uid);
      
    }
    else {
      db_query("UPDATE {webmail_plus_user_cron} SET checked='%s' WHERE uid=%d", $timestamp,  $user->uid);
    }
  }
  else {
    if($_webmail_plus_config->debug) echo "creating new<br>\n";
    if($batch>0) {
      db_query("INSERT INTO {webmail_plus_user_cron}(uid, checked, batch) VALUES(%d, '%s', %d)", $user->uid, $timestamp, $batch);      
    }
    else {
      db_query("INSERT INTO {webmail_plus_user_cron}(uid, checked) VALUES(%d, '%s')", $user->uid, $timestamp);
    }
  }
}

function _webmail_plus_cron_get($user=NULL) {
  if(!$user) global $user;
  if(!$user->uid) return;
  
  $timestamp = db_result(db_query("SELECT `checked` FROM {webmail_plus_user_cron} WHERE uid=%d", $user->uid));
 
  return $timestamp;
}
*/

?>