<?php

/**
 * @file
 * Scheduled Melo functions.
 */

/**
 * @defgroup melodev_crons Melodev Crons
 *
 * Functions that are run on cron.
 *
 * Functionality that needs to be run regularly is wrapped in a function
 * and scheduled in hook_cron.  These are the functions that run.
 * @{
 */
 
/**
 * How long is the time period for stats recent.
 */
define('STATS_RECENT_THRESHOLD', 60*60*24*3); // 3 days

/**
 * Rotate the Front Page Art.
 */
function melodev_cron_fpart_rotate($force=false) {
  $thresh = 60*60*24;

  if(melodev_cron_log('melodev_cron_fpart_rotate', $thresh) || $force) {
    $time = mktime();
    $nodes = nodequeue_load_nodes(1, FALSE, 0, 1, FALSE);
    if(count($nodes) == 1) {
    
      // Publish the node
      $node = array_pop($nodes);
      $node->status = 1;
      node_save($node);
      
      // Remove it from the upcoming subqueue
      nodequeue_subqueue_remove_node(1, $node->nid);
      
      // Add it to the posted subqueue
      $queue = nodequeue_load(2);
      $subqueue = nodequeue_load_subqueue(2);
      nodequeue_subqueue_add($queue, $subqueue, $node->nid);
      
      melodev_cron_log('melodev_cron_fpart_rotate', 24*60*60, true, '('.(mktime() - $time).')');
      
      watchdog('melodev.cron', 'Posted front page art ['.$node->nid.'].');
      
      return true;
    } else {
      watchdog('melodev.cron', 'Nothing to update the frontpage art to. ('.(mktime() - $time).')');
    }

  }
}

/**
 * Rotate the Front Page Poll.
 */
function melodev_cron_fppoll_rotate($force=false) {
  $thresh = 60*60*24;

  if(melodev_cron_log('melodev_cron_fppoll_rotate', $thresh) || $force) {
    $time = mktime();
    $nodes = nodequeue_load_nodes(3, FALSE, 0, 1, FALSE);
    if(count($nodes) == 1) {
    
      // Grab the node
      $node = array_pop($nodes);
      
      // Remove it from the upcoming subqueue
      nodequeue_subqueue_remove_node(3, $node->nid);
      
      // Add it to the posted subqueue
      $queue = nodequeue_load(4);
      $subqueue = nodequeue_load_subqueue(4);
      nodequeue_subqueue_add($queue, $subqueue, $node->nid);
      
      melodev_cron_log('melodev_cron_fppoll_rotate', $thresh, true, '('.(mktime() - $time).')');
      
      watchdog('melodev.cron', 'Posted front page poll ['.$node->nid.'].');
      
      return true;
    } else {
      watchdog('melodev.cron', 'Nothing to update the frontpage poll to. ('.(mktime() - $time).')');
    }

  }
}

/**
 * Reset user touches.
 */
function melodev_cron_reset_touches($force=false) {

  // Every 24 hours
  $thresh = 24*60*60;
  
  if(melodev_cron_log('melodev_cron_touches_reset', $thresh) || $force) {
    $time  = mktime();
    db_query('truncate touch_counts');
    watchdog('melodev.cron', 'User touch counts have been reset.');  
    melodev_cron_log('melodev_cron_touches_reset', $thresh, true, '('.(mktime() - $time).')');
  }
  
}

/**
 * Calculate user karma for today.
 */
function melodev_cron_today_karma($force=false) {

  // Every 15 minutes
  $thresh = 60*15;
  
  // Grant rewards to anyone with stats today
  if(melodev_cron_log('melodev_cron_today_karma', $thresh) || $force) {
    if(!variable_get('melodev_cron_today_karma_lock', false)) {
      variable_set('melodev_cron_today_karma_lock', mktime());
      $time = mktime();
      $res = db_query('select distinct uid from {stats_today}');
      while($row = db_fetch_object($res)) {
        $account = melodev_user_load($row->uid);
        melodev_karma($account, true, true);
      }
      melodev_cron_log('melodev_cron_today_karma', $thresh, true, '('.(mktime() - $time).')');
      variable_set('melodev_cron_today_karma_lock', false);
    }
  }
}

/**
 * Calculate user karma overall.
 */
function melodev_cron_karma($force=false) {
  
  $thresh = 60;
  
  // Calculate everyone's karma
  if(melodev_cron_log('melodev_cron_karma', $thresh) || $force) {
    $limit = 500;
    $offset = variable_get('melodev_cron_karma_offset', 0);
    $time = mktime();
    $res = db_query('select distinct uid from {stats_alltime} where uid in (select uid from stats_today) order by uid asc limit '.$limit.' offset '.($limit * $offset));
    $i=0;
    while($row = db_fetch_object($res)) {
      $start = mktime() + microtime();
      $account = melodev_user_load($row->uid);
      melodev_karma($account, true, false);
      $i++;
    }
    $offset++;
    variable_set('melodev_cron_karma_offset', $offset);
    if($i == 0) {
      variable_set('melodev_cron_karma_offset', 0);
      melodev_cron_log('melodev_cron_karma', $thresh, true, '('.(mktime() - $time).')');
    }
  }
}

/**
 * Rotate stats daily.
 */
function melodev_cron_stats_daily($force=false) {

  $thresh = 60*60*24;

  // Once a day
  if(melodev_cron_log('melodev_cron_stats_daily', $thresh) || $force) {
  
    $time = mktime();
    watchdog('melodev.cron', 'Starting to run stats_daily.');
    
    // Copy stats_today into stats_temp & truncate
    db_query('truncate stats_temp');
    db_query('insert into stats_temp select * from stats_today');
    db_query('truncate stats_today');
    
    // Get today's date
    $date = mktime();
    
    // Get all of the stats_today and copy them over
    $result = db_query('select * from stats_temp');
    while($row = db_fetch_object($result)) {
    
      // Save a copy in stats_recent_breakdown
      db_query('insert into stats_recent_breakdown (date, uid, name, type, total) values(%d, %d, "%s", "%s", %d)', $date, $row->uid, $row->name, $row->type, $row->total);
      
      // Update stats_alltime
      if($res = db_fetch_object(db_query('select * from stats_alltime where uid = %d and type = "%s"', $row->uid, $row->type))) {
        db_query('update stats_alltime set total = total + %d where uid = %d and type = "%s"', $row->total, $row->uid, $row->type);
      } else {
        db_query('insert into stats_alltime (uid, name, type, total) values(%d, "%s", "%s", %d)', $row->uid, $row->name, $row->type, $row->total);
      }
  
    }
    
    // Delete old records from stats_recent_breakdown
    db_query('delete from stats_recent_breakdown where date < %d', $date-STATS_RECENT_THRESHOLD);
    
    // Truncate records from stats_recent
    db_query('truncate table stats_recent');
    
    // And rebuild it
    $result = db_query('select distinct type from stats_recent_breakdown');
    while($row = db_fetch_array($result)) {
      $type = $row['type'];
      $res = db_query('select uid, name, type, sum(total) as total from stats_recent_breakdown where type = "%s" group by uid', $type);
      while($row = db_fetch_object($res)) {
        db_query('insert into stats_recent (uid, name, type, total) values(%d, "%s", "%s", %d)', $row->uid, $row->name, $row->type, $row->total);
      }
    }
    
    melodev_cron_log('melodev_cron_stats_daily', $thresh, true, '('.(mktime() - $time).')');
  }
    
}

function melodev_cron_process_logs($force=false) {

  $limit = 10000;
  
  $start = mktime() + microtime();

  // Find all unprocessed logs
  $result = db_query('select distinct aid, title, path, uid from accesslog where aid not in (select aid from accesslog_owner) limit '.$limit);
  $logs = 0;
  $unknown = array();
  while($log = db_fetch_object($result)) {
    if(melodev_access_savelog($log)) {
      $logs++;
    } else {
      $unknown[] = $log;
    }
  }
//  watchdog('melodev.cron', 'Wrote '.$logs.' log ownership records.');
  
  if(count($unknown) > 500) {
    $log = '<pre>' .
      "<b>Unknown logs:\n</b>" .
      print_r($unknown, true) .
      '</pre>';
    watchdog('melodev.cron', count($unknown).' unknown logs encountered. [logs]', array('[logs]'=>$log));
  }
  
  variable_set('melodev_cron_process_logs', mktime());
//  watchdog('melodev.cron', 'Log processing took '.round((mktime() + microtime() - $start)).' seconds to complete.');
  
}

function melodev_cron_tagclouds($force=false) {
  $start = mktime() + microtime();
  $karma = 1000;

  db_query('truncate {melo_personal_tagcloud}');

  // Find all appropriate users
  $users = db_query('select uid from {karma} where uid > 0 and karma >= %d order by uid asc', $karma);
  $l = 0;
  while($row = db_fetch_object($users)) {
    // Regenerate this user's tag cloud
    $result = db_query('select nid from {node} where uid = %d', $row->uid);
    $nids = array();
    while($nid = db_result($result)) {
      $nids[] = $nid;
    }
    if(count($nids) > 0) {
      $nid_restrictor = '('. implode(',', $nids) .')';
      $sql = 'SELECT COUNT(*) AS count, d.tid, d.name, d.vid FROM {term_data} d INNER JOIN {term_node} n ON d.tid = n.tid WHERE n.nid IN '. $nid_restrictor . ' and d.vid = 2 GROUP BY d.tid, d.name, d.vid ORDER BY count DESC';
      $result = db_query($sql);
      $tags = tagadelic_build_weighted_tags($result, 6);
      if(count($tags) > 0) {
        $edit = (object)array(
          'type' => 'user',
          'id' => $row->uid,
          'tags' => serialize($tags),
          );
        drupal_write_record('melo_personal_tagcloud', $edit);
        $l++;
      }
    }
  }

  watchdog('melodev.cron', $l.' personal tagclouds written.');  
  variable_set('melodev_cron_tagclouds', mktime());
}

function melodev_cron_reimport_users() {

  // Check if we're already processing a user
  if($uid = variable_get('melodev_reimport_queue_uid', false)) {
    // UID set
  } else {
    // STart on a new user
    $uid = db_result(db_query('select uid from melodev_reimport_user where status = 0 order by uid asc limit 1'));
    variable_set('melodev_reimport_queue_uid', $uid);
    variable_set('melodev_reimport_queue_offset', 0);
  }
  if(!is_numeric($uid) || $uid < 1) {
//    watchdog('melodev.cron', 'Unable to reimport user with id '.$uid.'.');
    variable_set('melodev_reimport_queue_uid', 0);
    return false;
  }
  
  if(!variable_get('melodev_cron_reimport_lock', false)) {
    // Lock processing
    variable_set('melodev_cron_reimport_lock', mktime());
    
    // Begin processing this user
    $limit = 100;
    $offset = variable_get('melodev_reimport_queue_offset', 0);
    $start = mktime() + microtime();
  
    // Find nodes by this user
    $sql = 'select nid from node where uid = %d and type = "%s" and nid < %d order by nid asc limit '.$limit.' offset '.($limit * $offset);
    $result = db_query($sql, $uid, 'blog', MAX_OLD_NODE_ID);
    $nodes = 0;
    while($row = db_fetch_object($result)) {
      $nid = $row->nid;
      $node = node_load($nid);
      $oldid = db_result(db_query('SELECT oldid FROM {melodev_node_map} WHERE nodetype = "blog" and newid = %d', $nid));
      if(!$oldid || !is_numeric($oldid)) {
        watchdog('melodev.cron', 'Reimport Queue: Unable to find old id for node id '.$nid.'.');
        continue;
      }
      // Reimport this node
      melodev_reimport_node($oldid, $node);
      $nodes++;
    }
  
    // Finished with this user/queue
    if($nodes == 0) {
      db_query('update {melodev_reimport_user} set status = 1 where uid = %d', $uid);
      variable_set('melodev_reimport_queue_uid', 0);
      variable_set('melodev_reimport_queue_offset', 0);
    } else {
      $offset++;
      variable_set('melodev_reimport_queue_offset', $offset);
    }
    
    variable_set('melodev_cron_reimport_queue', mktime());
    watchdog('melodev.cron', 'Reimport Queue took '.round((mktime() + microtime() - $start)).' seconds to complete (uid: '.$uid.' nodes: '.$nodes.' offset: '.$offset.').');
    
    variable_set('melodev_cron_reimport_lock', false);
  }  
}

/**
* @} End of "melodev_crons"
*/

/**
 * Control the cron run by logging last timestamps.
 */
function melodev_cron_log($name, $threshold=0, $set=false, $txt='') {
  if($set == true) {
    watchdog('melodev.cron', 'Finished running '.$name.'. '.$txt);
    variable_set($name, mktime());
  }
  if((mktime() - variable_get($name, 0)) > $threshold) {
    return true;
  }
  return false;
}

function melodev_cron_timer() {
  $hook = 'cron';
  $args = array();
  $return = array();
  $modules = module_implements($hook);
  echo 'Found modules:'."\n";
  print_r($modules);
  echo "\n";
  $i=0;
  foreach ($modules as $module) {
    $start = mktime() + microtime();
    $function = $module .'_'. $hook;
    echo 'Calling function '.$function."\n";
    
    // Test search specially
    if($module == 'search') {
      echo 'Testing search...'."\n";
      
      echo 'Testing node...'."\n";
      echo 'Invoking update_index for module node'."\n";
//      module_invoke('node', 'update_index'); instead of this do this:
      $limit = (int)variable_get('search_cron_limit', 100);
      // Store the maximum possible comments per thread (used for ranking by reply count)
      echo "setting comments_scale...\n";
      variable_set('node_cron_comments_scale', 1.0 / max(1, db_result(db_query('SELECT MAX(comment_count) FROM {node_comment_statistics}'))));
      echo "setting views_scale...\n";
      variable_set('node_cron_views_scale', 1.0 / max(1, db_result(db_query('SELECT MAX(totalcount) FROM {node_counter}'))));
      echo "querying for nodes...\n";
      $result = db_query_range("SELECT n.nid FROM {node} n LEFT JOIN {search_dataset} d ON d.type = 'node' AND d.sid = n.nid WHERE d.sid IS NULL OR d.reindex <> 0 ORDER BY d.reindex ASC, n.nid ASC", 0, $limit);
      echo "beginning loop...\n";
      while ($node = db_fetch_object($result)) {
        echo 'Testing indexing a single node... '."INDEXING: ".$node->type.' '.$node->nid."\n";
        
        echo "Loading node ".$node->nid."\n";
        $node = node_load($node->nid);
      
        // save the changed time of the most recent indexed node, for the search results half-life calculation
        echo "Setting variable... \n";
        variable_set('node_cron_last', $node->changed);
      
        // Build the node body.
        echo "Building node ".$node->nid."\n";
        $node->build_mode = NODE_BUILD_SEARCH_INDEX;
        $node = node_build_content($node, FALSE, FALSE);
        $node->body = drupal_render($node->content);
      
        $text = '<h1>'. check_plain($node->title) .'</h1>'. $node->body;
      
        echo "Invoking nodeapi 'update index'... ".$node->nid."\n";
        
        // Fetch extra data normally not visible
        $extra = node_invoke_nodeapi($node, 'update index');
        foreach ($extra as $t) {
          $text .= $t;
        }

        echo "Running search_index... ".$node->nid."\n";
      
        // Update index
        melodev_hijack_search_index($node->nid, 'node', $text);
      }


      
      /*
      // Update word index
      $smodules = module_list();
      echo 'Module List:'."\n";
      print_r($smodules);
      foreach ($smodules as $smodule) {
        echo 'Invoking update_index for module '.$smodule."\n";
        module_invoke($smodule, 'update_index');
      }
      */
    }
    
    $result = call_user_func_array($function, $args);
    if (isset($result) && is_array($result)) {
      $return = array_merge_recursive($return, $result);
    }
    else if (isset($result)) {
      $return[] = $result;
    }
    echo 'Function '.$function.' took '.round((mktime() + microtime() - $start), 2).' seconds to run.'."\n";
    $i++;
  }

  return $return;
}

function melodev_hijack_search_index($sid, $type, $text) {
  $minimum_word_size = variable_get('minimum_word_size', 3);

  // Link matching
  global $base_url;
  $node_regexp = '@href=[\'"]?(?:'. preg_quote($base_url, '@') .'/|'. preg_quote(base_path(), '@') .')(?:\?q=)?/?((?![a-z]+:)[^\'">]+)[\'">]@i';

  // Multipliers for scores of words inside certain HTML tags.
  // Note: 'a' must be included for link ranking to work.
  $tags = array('h1' => 25,
                'h2' => 18,
                'h3' => 15,
                'h4' => 12,
                'h5' => 9,
                'h6' => 6,
                'u' => 3,
                'b' => 3,
                'i' => 3,
                'strong' => 3,
                'em' => 3,
                'a' => 10);

  // Strip off all ignored tags to speed up processing, but insert space before/after
  // them to keep word boundaries.
  $text = str_replace(array('<', '>'), array(' <', '> '), $text);
  $text = strip_tags($text, '<'. implode('><', array_keys($tags)) .'>');

  // Split HTML tags from plain text.
  $split = preg_split('/\s*<([^>]+?)>\s*/', $text, -1, PREG_SPLIT_DELIM_CAPTURE);
  // Note: PHP ensures the array consists of alternating delimiters and literals
  // and begins and ends with a literal (inserting $null as required).

  $tag = FALSE; // Odd/even counter. Tag or no tag.
  $link = FALSE; // State variable for link analyser
  $score = 1; // Starting score per word
  $accum = ' '; // Accumulator for cleaned up data
  $tagstack = array(); // Stack with open tags
  $tagwords = 0; // Counter for consecutive words
  $focus = 1; // Focus state

  $results = array(0 => array()); // Accumulator for words for index

  echo "Beginning split loop...\n";

  foreach ($split as $value) {
    $spstart = mktime() + microtime();
    if ($tag) {
      echo "tag...\n";
      // Increase or decrease score per word based on tag
      list($tagname) = explode(' ', $value, 2);
      $tagname = drupal_strtolower($tagname);
      // Closing or opening tag?
      if ($tagname[0] == '/') {
        $tagname = substr($tagname, 1);
        // If we encounter unexpected tags, reset score to avoid incorrect boosting.
        if (!count($tagstack) || $tagstack[0] != $tagname) {
          $tagstack = array();
          $score = 1;
        }
        else {
          // Remove from tag stack and decrement score
          $score = max(1, $score - $tags[array_shift($tagstack)]);
        }
        if ($tagname == 'a') {
          $link = FALSE;
        }
      }
      else {
        if (isset($tagstack[0]) && $tagstack[0] == $tagname) {
          // None of the tags we look for make sense when nested identically.
          // If they are, it's probably broken HTML.
          $tagstack = array();
          $score = 1;
        }
        else {
          // Add to open tag stack and increment score
          array_unshift($tagstack, $tagname);
          $score += $tags[$tagname];
        }
        if ($tagname == 'a') {
          // Check if link points to a node on this site
          if (preg_match($node_regexp, $value, $match)) {
            echo "get path...\n";
            $path = drupal_get_normal_path($match[1]);
            if (preg_match('!(?:node|book)/(?:view/)?([0-9]+)!i', $path, $match)) {
              $linknid = $match[1];
              if ($linknid > 0) {
                echo "ignore links...\n";
                // Note: ignore links to uncachable nodes to avoid redirect bugs.
                $node = db_fetch_object(db_query('SELECT n.title, n.nid, n.vid, r.format FROM {node} n INNER JOIN {node_revisions} r ON n.vid = r.vid WHERE n.nid = %d', $linknid));
                if (filter_format_allowcache($node->format)) {
                  $link = TRUE;
                  $linktitle = $node->title;
                }
              }
            }
          }
        }
      }
      // A tag change occurred, reset counter.
      $tagwords = 0;
    }
    else {
      echo "notag...\n";
      // Note: use of PREG_SPLIT_DELIM_CAPTURE above will introduce empty values
      if ($value != '') {
        if ($link) {
          // Check to see if the node link text is its URL. If so, we use the target node title instead.
          if (preg_match('!^https?://!i', $value)) {
            $value = $linktitle;
          }
        }
        echo "search_index_split...\n";
        $words = melodev_hijack_search_index_split($value);
        echo "words loop...\n";
        foreach ($words as $word) {
          // Add word to accumulator
          $accum .= $word .' ';
          $num = is_numeric($word);
          // Check wordlength
          if ($num || drupal_strlen($word) >= $minimum_word_size) {
            // Normalize numbers
            if ($num) {
              $word = (int)ltrim($word, '-0');
            }

            // Links score mainly for the target.
            if ($link) {
              if (!isset($results[$linknid])) {
                $results[$linknid] = array();
              }
              $results[$linknid][] = $word;
              // Reduce score of the link caption in the source.
              $focus *= 0.2;
            }
            // Fall-through
            if (!isset($results[0][$word])) {
              $results[0][$word] = 0;
            }
            $results[0][$word] += $score * $focus;

            // Focus is a decaying value in terms of the amount of unique words up to this point.
            // From 100 words and more, it decays, to e.g. 0.5 at 500 words and 0.3 at 1000 words.
            $focus = min(1, .01 + 3.5 / (2 + count($results[0]) * .015));
          }
          $tagwords++;
          // Too many words inside a single tag probably mean a tag was accidentally left open.
          if (count($tagstack) && $tagwords >= 15) {
            $tagstack = array();
            $score = 1;
          }
        }
      }
    }
    $tag = !$tag;
    $secs = round((mktime() + microtime() - $spstart), 2);
    echo "$secs seconds to run...\n";
  }

  echo "Beginning search wipe...\n";
  search_wipe($sid, $type, TRUE);
  
  echo "Beginning insert...\n";

  // Insert cleaned up data into dataset
  db_query("INSERT INTO {search_dataset} (sid, type, data, reindex) VALUES (%d, '%s', '%s', %d)", $sid, $type, $accum, 0);

  echo "Beginning results loop...\n";
  // Insert results into search index
  foreach ($results[0] as $word => $score) {
    // Try inserting first because this will succeed most times, but because
    // the database collates similar words (accented and non-accented), the
    // insert can fail, in which case we need to add the word scores together.
    @db_query("INSERT INTO {search_index} (word, sid, type, score) VALUES ('%s', %d, '%s', %f)", $word, $sid, $type, $score);
    if (!db_affected_rows()) {
      db_query("UPDATE {search_index} SET score = score + %f WHERE word = '%s' AND sid = %d AND type = '%s'", $score, $word, $sid, $type);
    }
    search_dirty($word);
  }
  unset($results[0]);

  echo "Begin node links search...\n";
  
  // Get all previous links from this item.
  $result = db_query("SELECT nid, caption FROM {search_node_links} WHERE sid = %d AND type = '%s'", $sid, $type);
  $links = array();
  while ($link = db_fetch_object($result)) {
    $links[$link->nid] = $link->caption;
  }

  echo "Beginning node links storage...\n";
  // Now store links to nodes.
  foreach ($results as $nid => $words) {
    $caption = implode(' ', $words);
    if (isset($links[$nid])) {
      if ($links[$nid] != $caption) {
        // Update the existing link and mark the node for reindexing.
        db_query("UPDATE {search_node_links} SET caption = '%s' WHERE sid = %d AND type = '%s' AND nid = %d", $caption, $sid, $type, $nid);
        search_touch_node($nid);
      }
      // Unset the link to mark it as processed.
      unset($links[$nid]);
    }
    else {
      // Insert the existing link and mark the node for reindexing.
      db_query("INSERT INTO {search_node_links} (caption, sid, type, nid) VALUES ('%s', %d, '%s', %d)", $caption, $sid, $type, $nid);
      search_touch_node($nid);
    }
  }
  
  echo "Beginning delete leftovers...\n";
  // Any left-over links in $links no longer exist. Delete them and mark the nodes for reindexing.
  foreach ($links as $nid => $caption) {
    db_query("DELETE FROM {search_node_links} WHERE sid = %d AND type = '%s' AND nid = %d", $sid, $type, $nid);
    search_touch_node($nid);
  }
}


function melodev_hijack_search_index_split($text) {
  static $last = NULL;
  static $lastsplit = NULL;

  if ($last == $text) {
    return $lastsplit;
  }
  // Process words
  echo "Begin search_simplify...\n";
  $text = melodev_hijack_search_simplify($text);
  $words = explode(' ', $text);
  
  echo "Beginning array_walk...\n";
  array_walk($words, '_search_index_truncate');

  // Save last keyword result
  $last = $text;
  $lastsplit = $words;

  echo "Returning words...\n";
  return $words;
}

function melodev_hijack_search_simplify($text) {
  // Decode entities to UTF-8
  $text = decode_entities($text);

  // Lowercase
  $text = drupal_strtolower($text);

  // Call an external processor for word handling.
  echo "Invoke preprocess...\n";
  search_invoke_preprocess($text);
  echo "End Invoke preprocess...\n";
  
  echo "Simple CJK...\n";
  // Simple CJK handling
  if (variable_get('overlap_cjk', TRUE)) {
    $text = preg_replace_callback('/['. PREG_CLASS_CJK .']+/u', 'search_expand_cjk', $text);
  }

  // To improve searching for numerical data such as dates, IP addresses
  // or version numbers, we consider a group of numerical characters
  // separated only by punctuation characters to be one piece.
  // This also means that searching for e.g. '20/03/1984' also returns
  // results with '20-03-1984' in them.
  // Readable regexp: ([number]+)[punctuation]+(?=[number])
  
  echo "------------------------------------------------------\n$text\n------------------------------------------------------\n";
  
//  echo "preg1...\n";
//  $text = preg_replace('/(['. PREG_CLASS_NUMBERS .']+)['. PREG_CLASS_PUNCTUATION .']+(?=['. PREG_CLASS_NUMBERS .'])/u', '\1', $text);

  // The dot, underscore and dash are simply removed. This allows meaningful
  // search behavior with acronyms and URLs.
  echo "preg2...\n";
  $text = preg_replace('/[._-]+/', '', $text);

  // With the exception of the rules above, we consider all punctuation,
  // marks, spacers, etc, to be a word boundary.
  echo "preg3...\n";
  $text = preg_replace('/['. PREG_CLASS_SEARCH_EXCLUDE .']+/u', ' ', $text);

  echo "Returning from search_simplify...\n";
  return $text;
}

function melodev_test_badcontent() {
  $text = '1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111';
  
  echo 'running preg on text...'."\n\n";
  echo $text."\n\n\n\n";
  
  echo 'PREG_CLASS_NUMBERS: '.PREG_CLASS_NUMBERS."\n\n";
  echo 'PREG_CLASS_PUNCTUATION: '.PREG_CLASS_PUNCTUATION."\n\n";
  
  $count = 0;
  $text = preg_replace('/(['. PREG_CLASS_NUMBERS .']+)['. PREG_CLASS_PUNCTUATION .']+(?=['. PREG_CLASS_NUMBERS .'])/u', '\1', $text, 100, $count);
  
  echo $count."\n";
  
  echo "\n\n".'pregged!@!!!!'."\n\n";
}