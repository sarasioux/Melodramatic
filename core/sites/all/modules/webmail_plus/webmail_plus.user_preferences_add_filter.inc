<?php
function webmail_plus_preferences_add_filter() {

	$form['matches'] = array(
	 '#type' => 'fieldset',
	 '#title' => t('Matches'),
	 '#collapsible' => TRUE,
	 '#collapsed' => FALSE
	);
	
	$form['matches']['from'] = array(
	 '#type' => 'textfield',
	 '#title' => t('From'),
	);
	
  $form['matches']['to'] = array(
   '#type' => 'textfield',
   '#title' => t('To'),
  );
  
  $form['matches']['subject'] = array(
   '#type' => 'textfield',
   '#title' => t('Subject'),
  );
  
  $form['matches']['has_words'] = array(
   '#type' => 'textfield',
   '#title' => t('Has Words'),
  );

  $form['matches']['doesnt_have'] = array(
   '#type' => 'textfield',
   '#title' => t('Doesn\'t have'),
  );  
  
  $form['matches']['has_attachment'] = array(
   '#type' => 'checkbox',
   '#title' => t('Has Attachment'),
  );  
  

  $form['action'] = array(
   '#type' => 'fieldset',
   '#title' => t('Action'),
   '#collapsible' => TRUE,
   '#collapsed' => FALSE
  );
  
  
  $form['action']['mark_as_read'] = array(
    '#type' => 'checkbox',
    '#title' => t('Mark as Read')
  );

  $form['action']['apply_tags'] = array(
    '#type' => 'checkbox',
    '#title' => t('Apply Tags'),
    '#prefix' => '<div class="from container-inline">'
  );
  
  $form['action']['apply_tags_list'] = array(
    '#type' => 'textfield',
    '#size' => 10,
    '#suffix' => '</div>'
  );
  


  $form['action']['move_to_junk'] = array(
    '#type' => 'checkbox',
    '#title' => t('Move to Junk'),
  );

  $form['action']['forward'] = array(
    '#type' => 'checkbox',
    '#title' => t('Forward to'),
    '#prefix' => '<div class="from container-inline">'  
  );

  $form['action']['forward_email'] = array(
    '#type' => 'textfield',
    '#size' => 10,
    '#suffix' => '</div>'
  );    
  
  $form['action']['delete'] = array(
    '#type' => 'checkbox',
    '#title' => t('Delete it'),
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#name' => t('Add'),
    '#value' => t('Add'),
    '#id' => 'add'
  );
  
	return $form;
}


function webmail_plus_preferences_add_filter_validate($form, &$form_state) {
  
  $matches=0;
  if($form_state['values']['from']) $matches++;
  if($form_state['values']['to']) $matches++;
  if($form_state['values']['subject']) $matches++;
  if($form_state['values']['had_words']) $matches++;
  if($form_state['values']['has_attachments']) $matches++;
  
  
  $actions=0;
  if($form_state['values']['mark_as_read']) $actions++;
  if($form_state['values']['apply_tags']) $actions++;
  if($form_state['values']['move_to_junk']) $actions++;
  if($form_state['values']['forward']) $actions++;
  if($form_state['values']['delete']) $actions++;
  
  if($matches==0) form_set_error('', t('The filter does not specify any search criteria.'));  	
  if($actions==0) form_set_error('', t('The filter does not specify any actions.'));   
    
}

function webmail_plus_preferences_add_filter_submit($form, &$form_state) {
	
	global $user;
	// create a new filter
	
	db_query("INSERT INTO {webmail_plus_user_filters}
	(
	`uid`,
	`weight`,
	`active`,
	`from`,
	`to`,
	`cc`,
	`subject`,
	`has_words`,
	`doesnt_have_words`,
	`has_attachments`,
	`mark_as_read`,
	`apply_tags`,
	`apply_tags_list`,
	`forward`,
	`forward_list`,
	`delete`)
	
	VALUES(
	%d,
	%d,
	%d,
	'%s',
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
	'%s',
	%d)",
	
  $user->uid,
  0,
  1,
  $form_state['values']['from'],
  $form_state['values']['to'],
  $form_state['values']['cc'],
  $form_state['values']['subject'],
  $form_state['values']['has_words'],
  $form_state['values']['doesnt_have_words'],
  $form_state['values']['has_attachments'],
  $form_state['values']['mark_as_read'],
  $form_state['values']['apply_tags'],
  $form_state['values']['apply_tags_list'],
  $form_state['values']['forward'],
  $form_state['values']['forward_list'],
  $form_state['values']['delete']
	);
	
	drupal_goto('user/'.$user->uid.'/webmail_plus/filters');
}
?>