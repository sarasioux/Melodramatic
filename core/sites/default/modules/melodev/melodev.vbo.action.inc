<?php

/**
 * @file
 * Mess with Views Bulk Operations.
 *
 * VBO might not even be required anymore, I did so much
 * hacking with it.  All of that work is stored here.
 */

/**
 * Implementation of hook_vbo_action_info().
 */
function melodev_vbo_action_info() {
  if (!module_exists('content')) return array();
  
  // Folder Update
  $array = array();
  $array['melodev_vbo_folders_action'] = array(
    'type' => 'node',
    'description' => t('Update Folders'),
    'configurable' => TRUE,
    'behavior' => array('changes_node_property'),
  );
  
  // Favorites Update
  $array['melodev_vbo_faves_action'] = array(
    'type' => 'node',
    'description' => t('Update Favorites'),
    'configurable' => TRUE,
    'behavior' => array('changes_node_property'),
  );
  
  // Move Posts to Folders
  $array['melodev_vbo_entries_folders_action'] = array(
    'type' => 'node',
    'description' => t('Move Entries'),
    'configurable' => TRUE,
    'behavior' => array('changes_node_property'),
    );
    
  return $array;
}

/**
 * Modify VBO forms.
 */
function melodev_vbo_form_alter(&$form, &$form_state, $form_id) {
  $view = $form['objects']['#view'];
  switch($view->name) {
    case 'Folders':
      // Add Weights
      $form['objects']['weights'] = array();
      foreach($form['objects']['#sets'] as $set) {
        foreach($set as $obj) {
          $weight = $obj->node_data_field_weight_field_weight_value;
          $form['objects']['weights']['weights_'.$obj->nid] = array(
            '#type' => 'textfield',
            '#title' => $obj->nid,
            '#value' => $weight,
            '#attributes' => array('class' => 'weight')
          );
        }
      }
      
      // Add Permissions
      $form['objects']['perms'] = array();
      module_load_include('inc', 'content', 'includes/content.node_form');
      $field = content_fields('field_folder_permission');
      $options = optionwidgets_options($field);
      foreach($form['objects']['#sets'] as $set) {
        foreach($set as $obj) {
          $perm = $obj->node_data_field_folder_permission_field_folder_permission_value;
          $elem = array(
            '#type' => 'select',
            '#default_value' => $perm,
            '#options' => $options,
            '#attributes' => array(
              'onChange' => 'javascript:meloShowFolderChanged(this);'
              )
            );
          $form['objects']['perms']['perms_'.$obj->nid] = $elem;
    
        }
      }
    break;
    
    case 'Favorites':
      // Add Weights
      $form['objects']['weights'] = array();
      foreach($form['objects']['#sets'] as $set) {
        foreach($set as $obj) {
          $weight = $obj->node_data_field_weight_field_weight_value;
          $form['objects']['weights']['weights_'.$obj->nid] = array(
            '#type' => 'textfield',
            '#title' => $obj->nid,
            '#value' => $weight,
            '#attributes' => array('class' => 'weight')
          );
        }
      }
    
    break;
    
    case 'Blogs':
    
      // Add Folder pulldown
      $folders = array('#' => '- '.t('Move to Folder').' - ');
      $arr = melodev_get_my_folders();
      foreach($arr as $key => $val) {
        $folders[$key] = $val;
      }
      $form['select']['folder_selection'] = array(
        '#type' => 'select',
        '#options' => $folders,
        '#default_value' => '#',
        '#weight' => -2,
      );
      $plugin = $form['plugin']['#value'];
      
      // Adjust the buttons
      foreach($plugin->get_selected_operations() as $key => $val) {
        $info = $plugin->get_operation_info($key);
        switch($info['callback']) {
          case 'melodev_vbo_entries_folders_action':
            $form['select'][$key]['#weight'] = -1;
            $form['select'][$key]['#value'] = t('Move');
          break;
          
          case 'views_bulk_operations_delete_node_action':
            $form['select'][$key]['#value'] = t('Delete');
          break;
        }
      }
      
      // Move the operations to the bottom
      unset($form['select']['#title']);
      $form['select']['#weight'] = 10;
      
      // Add a submit handler
      $form['select']['melodev_vbo_entries_folders_action']['#submit'][] = 'melodev_vbo_entries_folders_action_submit';
      /*
      echo '<pre>';
      print_r($form);
      exit();
      */
    break;
    
    case 'Guestbook':
    
      // Move the operations to the bottom
      unset($form['select']['#title']);
      $form['select']['#weight'] = 10;
      
      // Adjust the buttons
      $plugin = $form['plugin']['#value'];
      foreach($plugin->get_selected_operations() as $key => $val) {
        $info = $plugin->get_operation_info($key);

        switch($info['callback']) {
          case 'views_bulk_operations_delete_comment_action':
            $form['select'][$key]['#value'] = t('Delete Gspots');
          break;
          
          case 'comment_unpublish_action':
            $form['select'][$key]['#weight'] = -1;
            $form['select'][$key]['#value'] = t('Hide Gspots');
          break;
        }
      }
      
    break;
    
    case 'Photos':
      // Move the operations to the bottom
      unset($form['select']['#title']);
      $form['select']['#weight'] = 10;
      
      // Add Folder pulldown
      $folders = array('#' => '- '.t('Move to Folder').' - ');
      $arr = melodev_get_my_folders();
      foreach($arr as $key => $val) {
        $folders[$key] = $val;
      }
      
      $form['select']['folder_selection'] = array(
        '#type' => 'select',
        '#options' => $folders,
        '#default_value' => '#',
        '#weight' => -2,
      );
      // Add a submit handler
      $form['select']['melodev_vbo_entries_folders_action']['#submit'][] = 'melodev_vbo_entries_folders_action_submit';
      
      // Adjust the buttons
      $plugin = $form['plugin']['#value'];
      foreach($plugin->get_selected_operations() as $key => $val) {
        $info = $plugin->get_operation_info($key);
        switch($info['callback']) {
          case 'views_bulk_operations_delete_node_action':
            $form['select'][$key]['#value'] = t('Delete Photo');
          break;
          case 'melodev_vbo_entries_folders_action':
            $form['select'][$key]['#weight'] = -1;
            $form['select'][$key]['#value'] = t('Move Photos');
          break;
        }
      }
    break;
    /*
      echo '<pre>';
      print_r($form);
      exit();
    */
  }
}

/*  FOLDERS UPDATE ACTION  */

/**
 * Update folder action form.
 */
function melodev_vbo_folders_action_form($context) {
  $form = array();
  return $form;
}
/**
 * Update folder action submit.
 */
function melodev_vbo_folders_action_submit($form, $form_state) {
  $k=0;
  // Save each node
  foreach($form_state['values']['objects'] as $id => $val) {
    if(is_numeric($id) && $id == $val) {
      $changed = false;
      $node = node_load($id);
      
      // Check for weight
      if($weight = $form['#post']['objects']['weights']['weights_'.$id]) {
        if($node->field_weight[0]['value'] != $weight) {
          $node->field_weight[0]['value'] = $weight;
          $changed = true;
        }
      }
      // Check for perm
      if($perm = $form_state['values']['objects']['perms']['perms_'.$id]) {
        if($node->field_folder_permission[0]['value'] != $perm) {
          $node->field_folder_permission[0]['value'] = $perm;
          $changed = true;
        }
      }
      if($changed == true) {
        node_save($node);
        $k++;
      }
    }
  }
  // Okay we're done!  Redirect now
  drupal_set_message('Successfully updated ('.$k.') of your folders.');
  drupal_goto('mymelo/content/folders');
}

/*  FAVES UPDATE ACTION  */

/**
 * Update favorites form.
 */
function melodev_vbo_faves_action_form($context) {
  $form = array();
  if(isset($_GET['folder'])) {
    $form['#redirect'] = $_GET['q'].'?folder='.$_GET['folder'];
  } else {
    $form['#redirect'] = $_GET['q'];
  }
  return $form;
}
/**
 * Update favorites submit.
 */
function melodev_vbo_faves_action_submit($form, $form_state) {
  // Save each node
  foreach($form_state['values']['objects'] as $id => $val) {
    if(is_numeric($id) && $id == $val) {
      $changed = false;
      $node = node_load($id);
      
      // Check for weight
      if($weight = $form['#post']['objects']['weights']['weights_'.$id]) {
        if($node->field_weight[0]['value'] != $weight) {
          $node->field_weight[0]['value'] = $weight;
          $changed = true;
        }
      }

      if($changed == true) {
        node_save($node);
      }
    }
  }
  // Okay we're done!  Redirect now
  drupal_set_message('Your favorites were updated.');

  $redir = explode('?', $form['#redirect']);
  if(isset($redir[0])) { $path = $redir[0]; } else { $path = 'mymelo/content/favorites'; }
  if(isset($redir[1])) { $qury = $redir[1]; } else { $qury = NULL; }
  drupal_goto($path, $qury);
}

/*  ENTRIES FOLDERS UPDATE ACTION  */

/**
 * Update entries action form.
 */
function melodev_vbo_entries_folders_action_form($context) {
  $form = array();
  return $form;
}
/**
 * Update entries action submit.
 */
function melodev_vbo_entries_folders_action_submit(&$form, &$form_state) {
  $fid = $form_state['values']['folder_selection'];
  $arg = $form['plugin']['#value']->view->args[0];
  
  // Save each node
  foreach($form_state['values']['objects'] as $id => $val) {
    if(is_numeric($id) && $id == $val) {
      $changed = false;
      $node = node_load($id);
      
      $node->field_folder_reference[0]['nid'] = $fid;
      node_save($node);
    }
  }
  // Okay we're done!  Redirect now
  drupal_set_message('Your posts were successfully updated.');  
  $redirect = substr($form['#action'], 1);
  if(!$redirect) {
    $redirect = 'mymelo/content/entries';
  }
  drupal_goto($redirect);
}
/**
 * Update entries action placeholder
 */
function melodev_vbo_entries_folders_action(&$node, $context) {
}


/**
 * Provide the ability to select items in a view using checkboxes.
 */
function phptemplate_views_node_selector($element) {
  require_once(drupal_get_path('module', 'views').'/theme/theme.inc');
  
  drupal_add_js('misc/tableselect.js');
  drupal_add_js(drupal_get_path('module', 'views_bulk_operations').'/views_bulk_operations.js');
  drupal_add_css(drupal_get_path('module', 'views_bulk_operations') . '/views_bulk_operations.css', 'module');

  // Don't add the vbo setting twice.
  $javascript = drupal_add_js();
  $found = FALSE;
  if (!empty($javascript['setting'])) foreach ($javascript['setting'] as $setting) {
    if (is_array($setting) && isset($setting['vbo']) && isset($setting['vbo']['url'])) {
      $found = TRUE;
      break;
    }
  }
  if (!$found) {
    drupal_add_js(array('vbo' => array('url' => url($_GET['q']))), 'setting');
  }

  $output = '';
  $view = $element['#view'];
  $sets = $element['#sets'];
  $vars = array(
    'view' => $view,
  );

  // Give each group its own headers row.
  foreach ($sets as $records) {
    // template_preprocess_views_view_table() expects the raw data in 'rows'.
    $vars['rows'] = $records;
      
    // Render the view as table. Function from views/theme/theme.inc
    template_preprocess_views_view_table($vars);
    
    // Add checkboxes to the header and the rows.
    $headers[] = theme('table_select_header_cell');
    foreach ($vars['header'] as $field => $label) {
      $headers[] = array('data' => $label, 'class' => "views-field views-field-{$vars['fields'][$field]}");
    }

    // Check if this is a draggable table
    if(isset($vars['rows'][0]['field_weight_value'])) {
      $draggable = true;
    }
    
    $rows = array();
    foreach ($records as $num => $object) {
      $row = array('class' => 'rowclick', 'data' => array());
      // Make checkboxes default to checked and hidden
      if($draggable) {
        $element[$object->{$view->base_field}]['#type'] = 'hidden';
        $element[$object->{$view->base_field}]['#value'] = 1;
        $element[$object->{$view->base_field}]['#default_value'] = 1;
        $row['data'][] = theme('hidden', $element[$object->{$view->base_field}]);
      } else {
        $row['data'][] =  theme('checkbox', $element[$object->{$view->base_field}]);
      }
      foreach ($vars['rows'][$num] as $field => $content) {
        // Treat weight fields special
        if($field == 'field_weight_value') {
          $elem = $element['weights']['weights_'.$object->nid];
          $content = theme('textfield', $elem);
        }
        // Treat perm fields special
        global $user;
        if($field == 'field_folder_permission_value') {
          if($object->nid != $user->profile_public_folder_id) {
            $elem = $element['perms']['perms_'.$object->nid];
            $content = '<div class="mymelo-perm-'.$elem['#default_value'].' mymelo-perm-folder">'.theme('select', $elem).'</div>';
          } else {
            $content = '<div class="mymelo-perm-public mymelo-perm-folder">'.$content.'</div>';
          }
        }
        $row['data'][] = array('data' => $content, 'class' => "views-field views-field-{$vars['fields'][$field]}");
      }
      if($draggable) {
        $row['class'] .= ' draggable';
      }
      $rows[] = $row;
    }
  
    // Add the first row as option to select all records across all pages.
    if ($view->pager['use_pager'] && $view->total_rows > $view->pager['items_per_page']) {
      $row = array(
        array(
          'data' => '<span id="vbo-this-page">'. t('All <strong>!objects</strong> rows on this page are selected.', array('!objects' => count($records)))
            .'&nbsp;<input type="button" id="vbo-select-all-pages" value="'. t('Select all !objects rows in this view.', array('!objects' => $view->total_rows)) .'" /></span>'
            .'<span id="vbo-all-pages" style="display: none">'. t('All <strong>!objects</strong> rows in this view are selected.', array('!objects' => $view->total_rows))
            .'&nbsp;<input type="button" id="vbo-select-this-page" value="'. t('Select only !objects rows on this page.', array('!objects' => count($records))) .'" /></span>',
          'class' => 'view-field view-field-select-all',
          'colspan' => count($headers) + 1,
        ),
      );
      array_unshift($rows, $row);
    }
    
    $table_id = 'vbo-table-view-'.$view->name;
    
    $output .= theme('table', $headers, $rows, array('class' => $vars['class'], 'id' => $table_id));
    $output .= theme('hidden', $element['select_all']);

    if($draggable) {
      // Call add_tabledrag to add and setup the JS for us
      // The key thing here is the first param - the table ID
      // and the 4th param, the class of the form item which holds the weight
      drupal_add_tabledrag($table_id, 'order', 'sibling', 'weight');
    }
  }
  return theme('form_element', $element, $output);
}
