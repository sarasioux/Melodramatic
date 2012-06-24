<?php

define('MAX_OLD_NODE_ID', 5436530);

// More advanced example: multi-step operation - load all nodes, five by five
function melodev_rebuild_user_access_batch($uid, &$context) {
  if(empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_node'] = 0;
    $context['sandbox']['max'] = db_result(db_query('SELECT COUNT(DISTINCT nid) FROM {node} where uid = %d and nid < %d', $uid, MAX_OLD_NODE_ID));
    watchdog('melodev.batches', 'max nodes '.$context['sandbox']['max']);
    $context['sandbox']['uid'] = $uid;
  }
  $uid = $context['sandbox']['uid'];
  $limit = 100;
  $result = db_query_range("SELECT nid FROM {node} WHERE uid = %d and nid > %d ORDER BY nid ASC", $uid, $context['sandbox']['current_node'], 0, $limit);
  $i=0;
  while ($row = db_fetch_array($result)) {  
    $node = node_load($row['nid'], NULL, TRUE);
    node_access_acquire_grants($node);
    $context['results'][] = $node->nid .' : '. $node->title;
    $context['sandbox']['progress']++;
    $context['sandbox']['current_node'] = $node->nid;
    $context['message'] = $node->title;
    $i++;
  }
  if($i == 0) {
    // No more rows
    $context['sandbox']['progress'] = $context['sandbox']['max'];
  }
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

function melodev_rebuild_user_access_batch_finished($success, $results, $operations) {
  if ($success) {
    $message = format_plural(count($results), 'One post processed.', '@count posts processed.');
  } else {
    $message = t('Finished with an error.');
  }
  drupal_set_message($message);
  // Providing data for the redirected page is done through $_SESSION.
  foreach ($results as $result) {
    $items[] = t('Rebuilt access for node %title.', array('%title' => $result));
  }
}

// More advanced example: multi-step operation - load all nodes, five by five
function melodev_reimport_user_entries_batch($uid, &$context) {
  if(empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_node'] = 0;
    $context['sandbox']['max'] = db_result(db_query('SELECT COUNT(DISTINCT nid) FROM {node} where uid = %d', $uid));
    watchdog('melodev.batches', 'Reimporting '.$context['sandbox']['max'].' nodes for user id '.$uid.'.');
    $context['sandbox']['uid'] = $uid;
  }
  $uid = $context['sandbox']['uid'];
  $limit = 1;
  $result = db_query_range('select nid from node where type = "blog" and uid = %d and nid < %d and nid > %d ORDER BY nid ASC', $uid, MAX_OLD_NODE_ID, $context['sandbox']['current_node'], 0, $limit);
  $i=0;
  while ($row = db_fetch_array($result)) {
    $nid = $row['nid'];
    $res = db_fetch_array(db_query('SELECT oldid, newid FROM {melodev_node_map} WHERE nodetype = "blog" and newid = %d', $nid));
  
    // Load the map, find the original body, compare
    $node = node_load($res['newid'], NULL, TRUE);
    if(melodev_reimport_node($res['oldid'], $node)) {
      $context['message'] = 'REIMPORTED: '.$node->title;
      watchdog('melodev.batches', 'Reimported nid '.$node->nid.'.');
    } else {
      $context['message'] = 'IDENTICAL: '.$node->title;
    }
    
    $context['results'][] = $node->nid .' : '. $node->title;
    $context['sandbox']['progress']++;
    $context['sandbox']['current_node'] = $node->nid;

    $i++;
  }
  if($i == 0) {
    // No more rows
    $context['sandbox']['progress'] = $context['sandbox']['max'];
  }
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

function melodev_reimport_user_entries_batch_finished($success, $results, $operations) {
  if ($success) {
    $message = format_plural(count($results), 'One post processed.', '@count posts processed.');
  } else {
    $message = t('Finished with an error.');
  }
  drupal_set_message($message);
  // Providing data for the redirected page is done through $_SESSION.
  foreach ($results as $result) {
    $items[] = t('Reimported node %title.', array('%title' => $result));
  }
}

function melodev_reimport_node($oldid, $node, $output=false) {

  if(!is_numeric($oldid) || $oldid < 1) {
    watchdog('melodev.batches', 'Invalid oldid '.$oldid.' for node id '.$node->nid.'.');
    return false;
  }

  $sql = 'select text_body from journal_entries_clean where container_item_id = '.$oldid.' limit 1';
  $res = melodev_pg_good_query($sql);
  $oldbody = $res[0]['text_body'];
  
  if($oldbody == $node->body || strlen($oldbody) == strlen($node->body)) {
//    watchdog('melodev.batches', 'Identical node found on reimport.');
    if($output) { drupal_set_message('Identical node found on reimport.'); }
    return false;
  }
  
  if(strlen($oldbody) < 1) {
    watchdog('melodev.batches', 'No text_body for oldid '.$oldid.' and node id '.$node->nid.'. sql='.$sql.' return: '.print_r($res, true));
    return false;
  }
  
  // FIX BAD CHARACTERS
  $chrmap = array(
  
    145 => "'",
    146 => "'",
    147 => '"',
    148 => '"',
    150 => '--',
    133 => "...",
    233 => "é",
    226 => "â",
    
  );
  
  $change = false;
  $body = $oldbody;
  $pbody = $body;
  while($str = substr($pbody, 0, 1)) {
    $pbody = substr($pbody, 1);
    if(ord($str) > 127) {
      $val = ord($str);
      // Check for mapping
      if(isset($chrmap[$val])) {
        $body = str_replace($str, $chrmap[$val], $body);
        $change = true;
      } else {
        watchdog('melodev.batches', 'ASCII NOT FOUND: oldid: '.$oldid.' newid: '.$node->nid.' ord: '.ord($str).' str='.$str.' val='.$val);
        if($output) { drupal_set_message('ASCII NOT FOUND: '.$str.' = '.$val); }
        return false;
      }
    }
  }
  
  // Resave the node
  if($change && strlen($body) > 0) {
    $node->body = $body;
    node_save($node);
  }
  
  return true;
  
}

function melodev_reset_elite_expires_batch($uid, &$context) {
  if(empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_node'] = 0;
    $context['sandbox']['max'] = db_result(db_query('SELECT COUNT(DISTINCT uid) FROM {users}'));
  }
  $limit = 1000;
  $result = db_query_range("SELECT uid, name FROM {users} WHERE uid > %d ORDER BY uid ASC", $context['sandbox']['current_node'], 0, $limit);
  $i=0;
  while ($row = db_fetch_array($result)) {  
    $arr = melodev_estimate_current_expiration($row['uid']);
    if($arr['expiration'] > 0) {
//      db_query('update uc_roles_expirations set expiration = %d where uid = %d and rid = %d', $arr['expiration'], $row['uid'], MELO_ELITE_RID);
    }
    $context['results'][] = $row['uid'];
    $context['sandbox']['progress']++;
    $context['sandbox']['current_node'] = $row['uid'];
    $context['message'] = 'User: '.$row['name'].' ID: '.$row['uid'];
    $i++;
  }
  if($i == 0) {
    // No more rows
    $context['sandbox']['progress'] = $context['sandbox']['max'];
  }
  if ($context['sandbox']['progress'] != $context['sandbox']['max']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

function melodev_reset_elite_expires_batch_finished($success, $results, $operations) {
  if ($success) {
    $message = format_plural(count($results), 'One elite processed.', '@count elites processed.');
  } else {
    $message = t('Finished with an error.');
  }
  drupal_set_message($message);
}