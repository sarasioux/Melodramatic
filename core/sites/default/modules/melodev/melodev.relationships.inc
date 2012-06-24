<?php

/**
 * @file
 * Helper functions for friends/enemies/tabs.
 */


/*  Enemy System  */

/**
 * Load all enemies for a UID.
 */
function melodev_load_enemy_bans($uid) {
  $enemies = array();
  
  // Get i'm enemied
  $flags = flag_get_content_flags('user', $uid);
  if(is_array($flags)) {
    foreach($flags as $tuid => $theirflags) {
      foreach($theirflags as $type => $flag) {
        if($type == 'enemy') {
          $enemies[] = $flag->uid;
        }
      }
    }
  }
  // Get my enemies
  $flags = flag_get_user_flags('user', NULL, $uid);
  if(is_array($flags)) {
    foreach($flags as $type => $myflags) {
      if($type == 'enemy') {
        foreach($myflags as $tuid => $flag) {
          $enemies[] = $flag->content_id;
        }
      }
    }
  }
  return $enemies;  
}

/**
 * Check if users are enemies of eachother.
 */
function melodev_is_enemy($uid1, $uid2) {
  // They enemied me
  $flags = flag_get_content_flags('user', $uid2, $uid1);
  if(isset($flags[$uid1]['enemy'])) {
    return true;
  }
  // I enemied them
  $flags = flag_get_user_flags('user', $uid1, $uid2);
  if(isset($flags['enemy'])) {
    return true;
  }
  return false;
}

/**
 * Perform an enemy ban if necessary.
 */
function melodev_check_enemy_ban($uid1, $uid2, $name=null) {
  // They enemied me
  $flags = flag_get_content_flags('user', $uid2, $uid1);
  if(isset($flags[$uid1]['enemy'])) {
    header('Location: /enemy/'.$name);
    exit();
  }
  // I enemied them
  $flags = flag_get_user_flags('user', $uid1, $uid2);
  if(isset($flags['enemy'])) {
    drupal_set_message('That user considers you an enemy.');
    header('Location: /enemy/'.$name);
    exit();
  }
}

/**
 * Load enemy reasons.  @TODO:  make this functional!
 */
function melodev_get_enemy_reasons($uid1, $uid2) {
  $reasons = array();
  
  $reasons[$uid1] = 'i hate your guts';
  $reasons[$uid2] = 'fuck the horse you road in on';
  
  return $reasons;
}

/*  Friend System  */

/**
 * Load friends for supplied user id.
 */
function melodev_load_friends($uid) {
  $flags = flag_get_user_flags('user', NULL, $uid);
  $friends = array();
  if(is_array($flags) && is_array($flags['friend'])) {
    foreach($flags['friend'] as $uid => $flag) {
      $friends[] = $uid;
    }
  }
  return $friends;
}

/**
 * Load friended for supplied user id.
 */
function melodev_load_friended($uid) {
  $flags = flag_get_content_flags('user', $uid);
  $friends = array();
  if(is_array($flags)) {
    foreach($flags as $uid => $flag) {
      if(isset($flag['friend'])) {
        $friends[$uid] = $uid;
      }
    }
  }
  return $friends;
}
