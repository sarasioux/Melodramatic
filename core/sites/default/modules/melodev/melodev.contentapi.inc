<?php

/**
 * @file
 * Functions required for the automatic creation & manipulation of content.
 *
 * Creating nodes with CCK fields requires a little extra handling.
 * This file contains functions to make that work easier.
 */
 
module_load_include('inc', 'node', 'node.pages');  // new for Drupal 6

/**
 *  Programmatically create a node.
 */
function melodev_create_node($edit, $debug=false, $cli=false) {
  static $upload_cache;
  static $content_types;

	// Get the fields for this node type
	$type = $edit['type'];
	if(!$type) {
		drupal_set_message('Please provide a node type before generating content.');
		drupal_goto();
	}
	
	// Load content type
	if(!is_array($content_types)) {
	  $content_types = array();
	}
	if(isset($content_types[$type])) {
    $contenttype = $content_types[$type];
  } else {
    $contenttype = content_types($type);
  }
  
  // Set the UID
  if(isset($edit['name']) && isset($edit['uid'])) {
  	$uid = $edit['uid'];
  	$name = $edit['name'];
  	if($cli == true) {
  	  $user = drupal_anonymous_user();
  	} else {
  	  global $user;
  	}
    // Fake log the user in so we can do this
    if($user->uid == 0) {
      $fakelogin = true;
      $user = user_load(array('uid' => $edit['uid']));
    }
  } else {
	  global $user;
  	$uid = $user->uid;
		$name = $user->name;
  }
  
	$form_state = array();
	$nodeTmp = array('type' => $type); // a variable holding the content type
	
	$form_state['uid'] = $uid;
	
	$form_state['values']['type'] 		= $type; // the type of the node to be created
	$form_state['values']['status'] 	= 1; // set the node's status to Published, or set to 0 for unpublished
	$form_state['values']['title'] 		= $edit['title'];   // the node's title
	$form_state['values']['body'] 		= $edit['body']; // the body, not required
	if (isset($edit['path'])) {
		$form_state['values']['path'] 		= $edit['path']; // the body, not required
	}
	
	// settings
	$form_state['values']['comment'] = $edit['comment'];
	
	// the username of the node's author, must be a valid user with access to create this node.
	// Otherwise, see (http://drupal.org/node/178506#comment-726479) above about using user_load() to create nodes as any user, even when not logged in
	$form_state['values']['uid'] = $uid;
	$form_state['values']['name'] = $name;
	$form_state['values']['op'] = t('Save');  // this seems to be a required value
	
	if(isset($edit['taxonomy'])) {
		$form_state['values']['taxonomy'] = $edit['taxonomy'];
	}
	if(isset($edit['locations'])) {
		$form_state['values']['locations'] = $edit['locations'];
	}
	
	// Process CCK fields
	if(is_array($contenttype['fields'])) {
		foreach($contenttype['fields'] as $fieldname => $field) {
			if(isset($edit[$fieldname])) {
				$value = $edit[$fieldname];
				$form_state['values'][$fieldname] = melodev_prepare_cck_field($field, $edit[$fieldname], $uid);
			}
		}
	}
	
	if($debug) {
		$form_state['debug'] = 1;
		echo '<pre>';
		echo "<b>form_state:</b>\n";
		print_r($form_state);
		echo "\n";
		echo '</pre>';
	}
	
	$form_state['programmed'] = true;
	
	// call the function to create the node, node_revisions, and CCK values.
	// Replace "story" with the name of your form / content type
	
	$errs = drupal_execute($type.'_node_form', $form_state, (object) $nodeTmp);
		
	if($debug) {
		echo '<pre>';
		echo '<b>errs:</b>'."\n";
		print_r($errs);
		exit();
		echo '<b>form_state:</b>'."\n";
		print_r($form_state);
		echo '<b>node:</b>'."\n";
		print_r($nodeTmp);
		echo '</pre>';
		exit();
	}

	// if there were any validation errors, drupal_execute will return them, then you have a chance to do something
	if (count($errs)) {
		// optional code here
		global $contentapi_errs;
		$contentapi_errs = $errs;
	}
	
	// Log out of our fake login
	if($fakelogin) {
	  $user = drupal_anonymous_user();
	}
	
	return $form_state['nid'];
}

/**
 * Process a CCK field and store it appropriately.
 */
function melodev_prepare_cck_field($field, $value, $uid=null) {
	global $user;
	if(is_null($uid)) {
		$uid = $user->uid;
	}
	$type = $field['type'];
	switch($type) {
	
	
		// DATE CCK
		case 'date':
			$time = strtotime($value);
			$date = date('m/d/Y', $time);
			$vals = array(0=>array(
				'value'=>array('date'=>$date)
				));
		break;
		
		// TEXT CCK
		case 'text':
			$vals = array(0=>array('value'=>$value));
		break;
		
		// NODE REFERENCE CCK
		case 'nodereference':
			$vals = array('nid'=>array('nid'=>$value));
			switch($field['widget']['type']) {
				case 'noderefcreate_autocomplete':
					$vals = array(0=>$vals);
				break;
			}
		break;
		
		// NUMBER CCK
		case 'number_integer':
			$vals = array('value'=>$value);
			if($field['widget']['type'] != 'optionwidgets_select') {
				$vals = array(0=>$vals);
			}
		break;
		
		case 'taxonomy':
			continue;
		break;
		
		case 'location':
			$vals = array(0=>$value);
		break;
		
		case 'emvideo':
			$vals = array(0=>array('embed'=>$value));
		break;
		
		case 'link':
			$vals = array(0=>$value);
		break;
		
		case 'filefield':
		  if($field['widget']['type'] == 'imagefield_widget') {
        // IMAGE CCK
        if(file_exists($value)) {
          // Build the list of non-munged extensions.
          // @todo: this should not be here. we need to figure out the right place.
          $extensions = '';
          foreach ($user->roles as $rid => $name) {
            $extensions .= ' '. variable_get("upload_extensions_$rid",
            variable_get('upload_extensions_default', 'jpg jpeg gif png txt html doc xls pdf ppt pps odt ods odp'));
          }
          
          // Create the dir if it doesn't exist
          $dir = file_directory_path() . '/' . $field['widget']['file_path'];
          file_check_directory($dir, 1);
  
          $file_name = basename($value);
          $file_dest = file_directory_path() . '/' . $field['widget']['file_path'] . '/' . $file_name;
          $file_temp = file_get_contents($value);						
          $file_temp = file_save_data($file_temp, $file_dest, FILE_EXISTS_RENAME);
          
          $file = new stdClass();
          $file->filename = file_munge_filename(trim(basename($file_temp), '.'), $extensions);
          $file->filepath = $file_temp;
          $file->filemime = file_get_mimetype($file_temp);
          $file->source = $file_temp;
          $file->destination = $file_dest;
          $file->filesize = filesize($file_temp);
          $file->uid =  $uid;
          $file->status = FILE_STATUS_TEMPORARY;
          $file->timestamp = time();
          drupal_write_record('files', $file);
  
          // Add file to the cache.
          $upload_cache[$source] = $file;
          
          $vals = array(0=>array(
            'fid' => $file->fid,
            'list' => 0,
            'data' => array('description'=>null)
            ));
        }
        break;
      }
		
		default:
			echo '<pre>';
			echo 'DEFAULT CCK:';
			print_r($field);
			exit();
		break;
	}
	
	return $vals;
}