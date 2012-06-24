<?php
function webmail_plus_preferences_manage_filter() {
  global $user;
  
  $filter_id=arg(5);
  
  if($filter_id) {
    $filter = db_fetch_object(db_query("SELECT * FROM {webmail_plus_user_filters} WHERE uid=%d AND id=%d", $user->uid, $filter_id));
    
  
    
	$form['filter_id'] = array(
	 '#type' => 'hidden',
	 '#value' => $filter_id
	);   
	
    $form['form_action'] = array(
	 '#type' => 'hidden',
	 '#value' => 'update'
	);  
  }
  else {
    $form['form_action'] = array(
	 '#type' => 'hidden',
	 '#value' => 'add'
	);      
  }
  
  $form['matches'] = array(
	 '#type' => 'fieldset',
	 '#title' => t('Matches'),
	 '#collapsible' => TRUE,
	 '#collapsed' => FALSE
  );
	
  $form['matches']['description'] = array(
    '#type' => 'item',
    '#value' => t('Note: multiple matching conditions are treated as logical "AND" which means a message would have to satisfy every specified condition for the filter to be triggered.')
  );
  
  $form['matches']['match_from'] = array(
    '#type' => 'textfield',
	'#title' => t('From'),
  );
	
  if($filter) $form['matches']['match_from']['#default_value'] = $filter->match_from;
	
  $form['matches']['match_to'] = array(
   '#type' => 'textfield',
   '#title' => t('To'),
  );
  
  if($filter) $form['matches']['match_to']['#default_value'] = $filter->match_to;
  
  $form['matches']['match_cc'] = array(
   '#type' => 'textfield',
   '#title' => t('Cc'),
  );
  
  if($filter) $form['matches']['match_cc']['#default_value'] = $filter->match_cc;
    
  $form['matches']['match_subject'] = array(
   '#type' => 'textfield',
   '#title' => t('Subject'),
  );
  
  if($filter) $form['matches']['match_subject']['#default_value'] = $filter->match_subject;
  
  
  $form['matches']['match_has_words'] = array(
   '#type' => 'textfield',
   '#title' => t('Has Words'),
  );

  if($filter && $filter->has_words) $form['matches']['match_has_words']['#default_value'] = $filter->match_has_words;
  
  /*
  $form['matches']['match_doesnt_have_words'] = array(
   '#type' => 'textfield',
   '#title' => t('Doesn\'t have'),
  );  
  
  if($filter && $filter->doesnt_have_words) $form['matches']['match_doesnt_have_words']['#default_value'] = $filter->doesnt_have_words;
  */
  
  $form['matches']['match_has_attachment'] = array(
   '#type' => 'checkbox',
   '#title' => t('Has Attachment'),
  );  

  if($filter) $form['matches']['match_has_attachment']['#default_value'] = $filter->match_has_attachment;
  

  $form['action'] = array(
   '#type' => 'fieldset',
   '#title' => t('Action'),
   '#collapsible' => TRUE,
   '#collapsed' => FALSE
  );
  
  
  $form['action']['action_mark_as_read'] = array(
    '#type' => 'checkbox',
    '#title' => t('Mark as Read')
  );
  
  if($filter) $form['action']['action_mark_as_read']['#default_value'] = $filter->action_mark_as_read;
  

  $form['action']['action_apply_tags'] = array(
    '#type' => 'checkbox',
    '#title' => t('Apply Tags'),
    '#prefix' => '<div class="from container-inline">'
  );

  if($filter) $form['action']['action_apply_tags']['#default_value'] = $filter->action_apply_tags;
  
  $form['action']['action_apply_tags_list'] = array(
    '#type' => 'textfield',
    '#size' => 30,
    '#suffix' => '</div>',
    '#autocomplete_path' => 'webmail_plus/tag_list_autocomplete'  
  );
  
  if($filter) $form['action']['action_apply_tags_list']['#default_value'] = $filter->action_apply_tags_list;
  

  $form['action']['action_move_to_junk'] = array(
    '#type' => 'checkbox',
    '#title' => t('Move to Junk'),
  );
  
  if($filter) $form['action']['action_move_to_junk']['#default_value'] = $filter->action_move_to_junk;
  
  /*
  $form['action']['action_forward'] = array(
    '#type' => 'checkbox',
    '#title' => t('Forward to'),
    '#prefix' => '<div class="from container-inline">'  
  );


  if($filter) $form['action']['action_forward']['#default_value'] = $filter->action_forward;
  
  
  $form['action']['action_forward_list'] = array(
    '#type' => 'textfield',
    '#size' => 10,
    '#suffix' => '</div>'
  );    
  
  if($filter) $form['action']['action_forward_list']['#default_value'] = $filter->action_forward_list;
  */
  
  $form['action']['action_move_to_trash'] = array(
    '#type' => 'checkbox',
    '#title' => t('Move to Trash'),
  );
  
  if($filter) $form['action']['action_move_to_trash']['#default_value'] = $filter->action_move_to_trash;
  
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#name' => t('Add'),
    '#id' => 'submit'
  );
  
  if($filter_id) {
    $form['submit']['#value']=t('Update');
  }
  else {
    $form['submit']['#value']=t('Add'); 
  }
  
  return $form;
}


function webmail_plus_preferences_manage_filter_validate($form, &$form_state) {
  
  $matches=0;
  if($form_state['values']['match_from']) $matches++;
  if($form_state['values']['match_to']) $matches++;
  if($form_state['values']['match_cc']) $matches++;
  if($form_state['values']['match_subject']) $matches++;
  if($form_state['values']['match_had_words']) $matches++;
  if($form_state['values']['match_has_attachments']) $matches++;
  
  
  $actions=0;
  if($form_state['values']['action_mark_as_read']) $actions++;
  if($form_state['values']['action_apply_tags']) $actions++;
  if($form_state['values']['action_move_to_junk']) $actions++;
  if($form_state['values']['action_forward']) $actions++;
  if($form_state['values']['action_move_to_trash']) $actions++;
  
  if($form_state['values']['action_move_to_junk'] && $form_state['values']['action_move_to_trash']) {
    form_set_error('', t('You can not move a message to Junk and Trash. Please select one or the other.'));
  }
  
  if($matches==0) form_set_error('', t('The filter does not specify any search criteria.'));  	
  if($actions==0) form_set_error('', t('The filter does not specify any actions.'));   
    
}

function webmail_plus_preferences_manage_filter_submit($form, &$form_state) {
	

  
  global $user;
  
  if($form_state['values']['form_action']=='update') {

    db_query("UPDATE {webmail_plus_user_filters} SET
    `weight`=0,
    `active`=1,
    `match_from`='%s',
    `match_to`='%s',
    `match_cc`='%s',
    `match_subject`='%s',
    `match_has_words`='%s',
    `match_has_attachments`=%d,
    `action_mark_as_read`=%d,
    `action_apply_tags`=%d,
    `action_apply_tags_list`='%s',
    `action_move_to_junk`=%d,
    `action_forward`=%d,
    `action_forward_list`='%s',
    `action_move_to_trash`=%d
    WHERE `id`=%d AND `uid`=%d",
    $form_state['values']['match_from'],
    $form_state['values']['match_to'],
    $form_state['values']['match_cc'],
    $form_state['values']['match_subject'],
    $form_state['values']['match_has_words'],
    $form_state['values']['match_has_attachments'],
    $form_state['values']['action_mark_as_read'],
    $form_state['values']['action_apply_tags'],
    $form_state['values']['action_apply_tags_list'],
    $form_state['values']['action_move_to_junk'],
    $form_state['values']['action_forward'],
    $form_state['values']['action_forward_list'],
    $form_state['values']['action_move_to_trash'],
    $form_state['values']['filter_id'],
    $user->uid
    );
  }
  elseif($form_state['values']['form_action']=='add') {
  // create a new filter
	db_query("INSERT INTO {webmail_plus_user_filters}
	(
	`uid`,
	`weight`,
	`active`,
	`match_from`,
	`match_to`,
	`match_cc`,
	`match_subject`,
	`match_has_words`,
	`match_has_attachments`,
	`action_mark_as_read`,
	`action_apply_tags`,
	`action_apply_tags_list`,
	`action_move_to_junk`,
	`action_forward`,
	`action_forward_list`,
	`action_move_to_trash`)
	VALUES(
	%d,
	%d,
	%d,
	'%s',
	'%s',
	'%s',
	'%s',
	'%s',
	%d,
	%d,
	%d,
	'%s',
	%d,
	%d,
	'%s',
	%d)",
	
    $user->uid,
    0,
    1,
    $form_state['values']['match_from'],
    $form_state['values']['match_to'],
    $form_state['values']['match_cc'],
    $form_state['values']['match_subject'],
    $form_state['values']['match_has_words'],
    $form_state['values']['match_has_attachments'],
    $form_state['values']['action_mark_as_read'],
    $form_state['values']['action_apply_tags'],
    $form_state['values']['action_apply_tags_list'],
    $form_state['values']['action_move_to_junk'],
    $form_state['values']['action_forward'],
    $form_state['values']['action_forward_list'],
    $form_state['values']['action_move_to_trash']
	);
  }
  
  $form_state['redirect']='user/'.$user->uid.'/webmail_plus/filters';
}
?>