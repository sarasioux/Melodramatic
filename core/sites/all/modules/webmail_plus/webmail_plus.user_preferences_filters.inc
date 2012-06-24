<?php

function webmail_plus_preferences_filters() {
  global $user;
  
  $form['actions'] = array(
   '#type' => 'fieldset'
  );
  

  $form['actions']['new_filter'] = array(
   '#type' => 'submit',
   '#id' => 'new_filter',
   '#title' => t('New Filter'),
   '#value' => t('New Filter'),
   '#prefix' => '<div class="from container-inline">'
  
  );
  
  $form['actions']['action'] = array(
    '#type' => 'select',
    '#title' => t('Action'),
    '#options' => array(
      'suspend' => t('Suspend'),
      'unsuspend' => t('Unsuspend'),
      'delete' => t('Delete')
    ),
    
  );

  $form['actions']['apply_action'] = array(
    '#type' => 'submit',
    '#id' => 'apply_action',
    '#title' => t('Apply'),
    '#value' => t('Apply'),
    '#suffix' => '</div>'
  );
  
  
   
   $header = array(
    array('data' => t('Id'), 'field' => "`id`"),
    array('data' => t('Description'), 'field' => '`id`'),
    array('data' => t('active'), 'field' => '`active`'),
   );
   
   
   $form['header'] = array(
    '#type' => 'value', 
    '#value' => array(
      theme('table_select_header_cell'),
      array('data' => t('Description'), 'field' => '`id`'),
      array('data' => t('Status'), 'field'=>'`active`')
    )
   );   
    
  $sql = "SELECT * FROM {webmail_plus_user_filters} WHERE uid=".$user->uid." ORDER BY weight DESC";
  

  $results = pager_query($sql, 50);

  while($filter = db_fetch_object($results)) {
    //wpd($filter);
    $selected[$filter->id] = '';
   
    $form['id'][$filter->id] = array('#value' => $filter ->id, '#type' => 'hidden');
    $form['description'][$filter->id] = array('#value' =>  _webmail_plus_filter_description($filter).'<br>'.l('Edit', 'user/'.$user->uid.'/webmail_plus/filters/manage/'.$filter->id));
    $form['status'][$filter->id] = array('#value' => _webmail_plus_filter_status($filter));
    
    /*
    $form['to'][$filter->id] = array('#value' => $filter -> to);
    $form['cc'][$filter->id] = array('#value' => $filter -> cc);
    */
  }
  
  $form['selected'] = array('#type' => 'checkboxes', '#options' => $selected);
  $form['pager'] = array('#value' => theme('pager', NULL, 50, 0));
  
  
  //wpd($form);
  
  return $form;
}
	
function theme_webmail_plus_preferences_filters($form) {
	
  $output = drupal_render($form['actions']);
	
  if (isset($form['id'])) {
     $row = array();
  	 
     foreach (element_children($form['id']) as $key) {
       $rows[] = array(
        array('data'=>drupal_render($form['selected'][$key]),'class'=>$class),
        array('data'=>drupal_render($form['description'][$key]),'class'=>$class),
        array('data'=>drupal_render($form['status'][$key]),'class'=>$class)
        /*
        array('data'=>drupal_render($form['from'][$key]),'class'=>$class),
        array('data'=>drupal_render($form['to'][$key]),'class'=>$class),
        array('data'=>drupal_render($form['cc'][$key]),'class'=>$class)
        */
       );
     }
  }
  
  $output .= theme('table', $form['header']['#value'], $rows);

  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);  

  return $output;
}


function webmail_plus_preferences_filters_submit($form, &$form_state) {
  global $user;
 
  
  // handle a clicked new filter button
  if($form_state['clicked_button']['#id']=='new_filter') {
    $form_state['redirect']='user/'.$user->uid.'/webmail_plus/filters/add';
  }
  
  
  // handle an action from a drop-down
  if($form_state['clicked_button']['#id']=='apply_action') {
  	
  	foreach($form_state['values']['selected'] as $filter_id=>$checked) {
  		if($checked==0) continue;
  		
  		drupal_set_message($form_state['values']['action'].' '.$filter_id);
  		//echo "processing $filter_id<br>\n";
  		
  		if($form_state['values']['action']=='suspend') {
  			_webmail_plus_suspend_filter($filter_id);
  		}
  		
  		if($form_state['values']['action']=='unsuspend') {
  			_webmail_plus_unsuspend_filter($filter_id);
  		}
  		
  		if($form_state['values']['action']=='delete') {
  			//echo 'deleting '.$filter_id;
  			_webmail_plus_delete_filter($filter_id);
  		}
  		
  	}
  }
  
  $form_state['#redirect'] = 'user/'.$user->uid.'/webmail_plus/filters';
}

/**
 * suspends a filter
 *
 * @param unknown_type $id
 * @param unknown_type $user
 */
function _webmail_plus_suspend_filter($id, $user=NULL) {
	if(!$user) global $user;
	db_query("UPDATE {webmail_plus_user_filters} SET active=0 WHERE id=%d AND uid=%d", $id, $user->uid);
}

function _webmail_plus_unsuspend_filter($id, $user=NULL) {
  if(!$user) global $user;
  db_query("UPDATE {webmail_plus_user_filters} SET active=1 WHERE id=%d AND uid=%d", $id, $user->uid);
}


function _webmail_plus_delete_filter($id, $user=NULL) {
  if(!$user) global $user;
  db_query("DELETE FROM {webmail_plus_user_filters} WHERE id=%d AND uid=%d", $id, $user->uid);
}

function _webmail_plus_filter_description($filter) {
  $string=t('Matches:').' ';
  if($filter->match_from) $matches['From']=$filter->match_from;
  if($filter->match_to) $matches['To']=$filter->match_to;
  if($filter->match_cc) $matches['Cc']=$filter->match_cc;
  if($filter->match_subject) $matches['Subject']=$filter->match_subject;
  if($filter->match_has_words) $matches['Has words']=$filter->match_has_words;
  if($filter->match_has_attachments) $matches['Has attachments']=t('yes');
  
  foreach($matches as $field=>$value) {
  	$matches_array[]='<strong>'.t($field).':</strong> '.$value;
  }
  

  
  $matches_string = implode(', ', $matches_array);
  
  if($filter->action_mark_as_read) $actions[] = t('mark as read');
  if($filter->action_apply_tags && $filter->action_apply_tags_list) $actions[] = t('tag').' '.$filter->action_apply_tags_list;
  if($filter->action_forward && $filter->action_forward_list) $actions[] = t('forward to ').' '.$filter->action_forward_list;
  if($filter->action_move_to_junk) $actions[] = t('move to Junk');
  if($filter->action_move_to_trash) $actions[] = t('move to Trash');
  
  if(is_array($actions)) $actions_string = implode(', ',$actions);
  
  $result = "";
  if($matches_string) $result.= $matches_string;
  if($actions_string) $result.= '<br><strong>'.t('Do this').':</strong> '.$actions_string;

  return $result;
}

function _webmail_plus_filter_status($filter) {
	if($filter->active==1) return t('Active');
	if($filter->active==0) return t('Disabled');
}
?>