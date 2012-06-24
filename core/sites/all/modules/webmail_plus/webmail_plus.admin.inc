<?php
function webmail_plus_admin_settings() {
  $form = array();

	// notify the user that he needs to select a connection
  $existing_connection = variable_get('webmail_plus_connection', NULL);
	$servers = mail_api_get_servers();

	// flatten the server list
	if(sizeof($servers)>0) {
	
      foreach($servers as $key=>$value) {
        $servers_flat[$value['domain']]=$value['domain'];
        if(!$default_connection) $default_connection = $value['domain'];
      }
	
	}
	else {
      drupal_set_message(t('No email servers are configured. You need to configure at least one mail server before Webmail Plus can be configured. To configure a server click !link.', array('!link' => l(t('here'),'admin/settings/mail_api'))), 'error', FALSE);
	}	
	

	//echo "default connection: $default_connection<br>\n";
	
	// this solves a problem that occurs in updates
	// when a different connection that no longer exists is still recorded
	// in drupal variables, it causes quite a headache
	if(sizeof($servers)>0) {
		if($servers_flat[$existing_connection]) {
			$default_connection = $existing_connection;
		}
		else {
			variable_set('webmail_plus_connection', $default_connection);
			unset($_SESSION['webmail_plus']['connection_disabled']);
			drupal_set_message(t('The previously configured connection %existing_connection was not found. A new connection %default_connection was configured as default.', array('%existing_connection' => $existing_connection, '%default_connection' => $default_connection)),'error', FALSE);
		}
	}

	if(sizeof($servers)>0) {
      // Server settings
      $form['storage'] = array(
      	'#type' => 'fieldset',
        '#title' => t('Mail Storage'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE
      );

       $form['storage']['webmail_plus_connection'] = array(
      	'#type' => 'radios',
        '#title' => t('Mail Connection'),
        '#default_value' => $default_connection,
        '#options' => $servers_flat,
        '#description' => t('Select the connection which will be used by Webmail Plus to provide email storage.')
      );
      
      $form['storage']['webmail_plus_required_folders'] = array(
        '#type' => 'textfield',
        '#title' => t('Required Folders'),
        '#default_value' => variable_get('webmail_plus_required_folders', 'INBOX, Trash, Junk, Sent'),
        '#description' => t('A comma-separated list of required folders. These can not be removed or renamed.')
      );

      
      $form['storage']['webmail_plus_use_cron'] = array(
        '#type' => 'checkbox',
        '#title' => t('Use CRON to fetch mail.'),
        '#default_value' => variable_get('webmail_plus_use_cron', NULL),
        '#description' => t('To prevent your IMAP server from being overloaded you can use CRON to fetch messages. This might cause a delay in new message delivery but will lower the load on the server.')
      );
      
      $form['storage']['webmail_plus_cron_frequency'] = array(
        '#type' => 'textfield',
        '#size' => 3,
        '#title' => t('How often to fetch email with cron'),
        '#default_value' => variable_get('webmail_plus_cron_frequency', 60),
        '#description' => t('If you use CRON to check messages and it runs frequenty, set this number higher to lower the server load.')
      );      
      
      $form['storage']['webmail_plus_cron_batch_size'] = array(
        '#type' => 'textfield',
        '#size' => 3,
        '#title' => t('Cron batch size'),
        '#default_value' => variable_get('webmail_plus_cron_batch_size', 0),
        '#description' => t('Large sites can configure the maximum number of mailboxes to check per one cron run.')
      );        
	}
      $form['outbound'] = array(
      	'#type' => 'fieldset',
          '#title' => t('Mail Delivery'),
          '#collapsible' => TRUE,
          '#collapsed' => TRUE
      );      
      
      $send_methods = array(
        'sendmail' => t('Sendmail'),
        'smtp' => t('SMTP')
      );
      
      $form['outbound']['webmail_plus_send_method'] = array(
      	'#type' => 'radios',
        '#title' => t('Mail Connection'),
        '#default_value' => variable_get('webmail_plus_send_method', 'sendmail'),
        '#options' => $send_methods,
        '#description' => t('Select the method for outbound mail delivery.')
      );      


      $form['outbound']['webmail_plus_send_smtp_host'] = array(
      	'#type' => 'textfield',
        '#title' => t('SMTP host'),
        '#default_value' => variable_get('webmail_plus_send_smtp_host', 'localhost'),
        '#description' => t('Host name or IP address of the SMTP server.')
      );      
      
      $form['outbound']['webmail_plus_send_smtp_port'] = array(
      	'#type' => 'textfield',
        '#title' => t('SMTP port'),
        '#default_value' => variable_get('webmail_plus_send_smtp_port', 25),
        '#description' => t('Port number, e.g. 25 for SMTP, 465 for SSL.')
      );     

      $encryption_options = array(
        'none' => t('None'),
        'ssl' => t('SSL')
      );
      
      $form['outbound']['webmail_plus_smtp_encryption'] = array(
      	'#type' => 'radios',
        '#title' => t('Encryption'),
        '#default_value' => variable_get('webmail_plus_smtp_encryption', 'none'),
        '#options' => $encryption_options,
        '#description' => t('Select the connection encryption method.')
      );
      
      $form['outbound']['webmail_plus_smtp_username'] = array(
      	'#type' => 'textfield',
        '#title' => t('Username'),
        '#default_value' => variable_get('webmail_plus_smtp_username', NULL),
        '#description' => t('The username, if any, that is used to send mail to the SMTP server.')
      );

      $form['outbound']['webmail_plus_smtp_password'] = array(
      	'#type' => 'password',
        '#title' => t('Password'),
        '#default_value' => variable_get('webmail_plus_smtp_password', NULL),
        '#description' => t('The password associated with the user used to send mail to the SMTP server.')
      );      
      
      // authentication
      $form['authentication'] = array(
        '#type' => 'fieldset',
          '#title' => t('Authentication'),
          '#collapsible' => TRUE,
          '#collapsed' => TRUE
      );

      // this is always avaiable

      $authentication['user_defined']=t('User defined email password');


      $authentication['internal']=t('Internal');

      if(module_exists('ldapprov')) {
        $authentication['ldapprov']=t('LDAP Provisioning');
      }

      $form['authentication']['webmail_plus_authentication'] = array(
        '#type' => 'radios',
          '#title' => t('Authentication Mechanism'),
          '#default_value' => variable_get('webmail_plus_authentication', NULL),
          '#options' => $authentication,
          '#description' => t('Select the mechanism you want to use for password storage.')
      );


      // presentation
      $form['presentation'] = array(
      	'#type' => 'fieldset',
        '#title' => t('Presentation'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE
      );
  
      $form['presentation']['webmail_plus_messages_per_page'] = array(
        '#type' => 'select',
        '#title' => t('Number of messages per page'),
        '#options' => array('25'=>'25', '50'=>'50', '75'=>'75', '100'=>'100'),
        '#default_value' => variable_get('webmail_plus_messages_per_page', 25),
        '#description' => t('A number of messages to show per page before paginating.'),
      );      
      
      $form['presentation']['webmail_plus_notification_block_records'] = array(
        '#type' => 'select',
        '#title' => t('Number of messages in the notification block'),
        '#options'=> array('0' => 'unlimited', '3' => '3', '5'=>'5', '10'=>'10'),
        '#default_value' => variable_get('webmail_plus_notification_block_records', 3),
        '#description' => t('A number of messages you want to see in the notification block.'),
      );

      $form['presentation']['webmail_plus_max_tags_in_dropdown'] = array(
        '#type' => 'select',
        '#title' => t('Number of tags in the drop down on message or thread view'),
        '#options' => array('0'=>'unlimited', '5'=>'5', '10'=>'10', '15'=>'15', '20' => '20', '25'=>'25'),
        '#default_value' => variable_get('webmail_plus_max_tags_in_dropdown', 0),
        '#description' => t('Specify the maximum number of popular tags to show in the drop-downs on message or thread view.'),
      );      
      
      $form['presentation']['webmail_plus_thread_messages'] = array(
      	'#type' => 'checkbox',
        '#title' => t('Group similar messages into threads'),
        '#default_value' => variable_get('webmail_plus_thread_messages', TRUE),
        '#description' => t('Message replies will be organized into a coversation.')
      );


      $form['presentation']['webmail_plus_hidden_folders'] = array(
        '#type' => 'textfield',
        '#title' => t('Hide these folders'),
        '#default_value' => variable_get('webmail_plus_hidden_folders', NULL),
        '#description' => t('Supply the list of folders that are hidden from users.')
      );
	  
      $form['presentation']['webmail_plus_folder_order'] = array(
        '#type' => 'textfield',
        '#title' => t('Folder order'),
        '#default_value' => variable_get('webmail_plus_folder_order', 'INBOX, Sent, Drafts, Junk, Trash'),
        '#description' => t('Comma-separated list of folder names in the order they will appear.')
      );
	  
      $form['presentation']['webmail_plus_presenatation_override'] = array(
        '#type' => 'checkbox',
        '#title' => t('Allow users to override default presentation setting.'),
        '#default_value' => variable_get('webmail_plus_presenatation_override', TRUE),
        '#description' => t('Check this if you want users to be able to override the above settings.')
      );      
      
      
      // address-book related stuff
      
      $form['address_book'] = array(
      	'#type' => 'fieldset',
          '#title' => t('Address Book'),
          '#collapsible' => TRUE,
          '#collapsed' => TRUE
      );
      

      if(module_exists('webmail_plus_site_users'))  $address_book_options['webmail_plus_site_users']='Site Users';
      if(module_exists('contact_manager'))  $address_book_options['contact_manager']='Contact Manager';
      
      $form['address_book']['webmail_plus_address_book'] = array(
        '#title' => t('Select the preferred address book module'),
        '#type' => 'checkboxes',
        '#options' => $address_book_options,
        '#default_value' => variable_get('webmail_plus_address_book', 'none')
      ); 
        
      // user settings
      $form['user'] = array(
      	'#type' => 'fieldset',
          '#title' => t('User Settings'),
          '#collapsible' => TRUE,
          '#collapsed' => TRUE
      );

          
      
      $email_format_options = array(
        'plaintext' => t('Plain Text'),
        'html' => t('HTML')
      );
      
      $form['user']['webmail_plus_email_format'] = array(
      	'#type' => 'radios',
        '#title' => t('Preferred email format'),
        '#default_value' => variable_get('webmail_plus_email_format', 'plaintext'),
        '#options' => $email_format_options,      
        '#description' => t('Defines the format used for outbound email.')
      );      
      
      $form['user']['webmail_plus_email_format_override'] = array(
      	'#type' => 'checkbox',
        '#title' => t('Allow users to override the preferred email format.'),
        '#default_value' => variable_get('webmail_plus_email_format_override', NULL),
        '#description' => t('Check this box if you want users to select their own preferred email format.')
      );          
      
      $form['user']['webmail_plus_allow_signature'] = array(
      	'#type' => 'checkbox',
          '#title' => t('Allow user signatures'),
          '#default_value' => variable_get('webmail_plus_allow_signature', TRUE),
          '#description' => t('Check this if you want users to use their signatures in outbound emails.')
      );

      $form['user']['webmail_plus_user_out_of_office'] = array(
      	'#type' => 'checkbox',
          '#title' => t('Allow out of office response'),
          '#default_value' => variable_get('webmail_plus_user_out_of_office', NULL),
          '#description' => t('Check this to allow users to configure out of office responses.')
      );

      
      // a setting for profile fields
      if(module_exists('profile')) {
        $form['user']['webmail_plus_user_from'] = array(
          '#type' => 'textfield',
            '#title' => t('Custom From field'),
            '#default_value' => variable_get('webmail_plus_user_from', NULL),
            '#description' => t('Configure a custom from field using profile variables, e.g. @profile_first_name @profile_last_name')
        );
      }
      
      
      // a setting for content_profile fields
	  if(module_exists('content_profile')) {
        $form['user']['webmail_plus_user_from'] = array(
          '#type' => 'textfield',
            '#title' => t('Custom From field'),
            '#default_value' => variable_get('webmail_plus_user_from', NULL),
            '#description' => t('Configure a custom from field using profile variables, e.g. @profile_first_name @profile_last_name')
        );
      }

      // external mailboxes
      /*
      $form['external_mailboxes'] = array(
      	'#type' => 'fieldset',
          '#title' => t('External Mailboxes'),
          '#collapsible' => TRUE,
          '#collapsed' => TRUE
      );

      $form['external_mailboxes']['webmail_plus_allow_external_mailboxes'] = array(
      	'#type' => 'checkbox',
          '#title' => t('Allow user-configured external mailboxes'),
          '#default_value' => variable_get('webmail_plus_allow_external_mailboxes', TRUE),
          '#description' => t('Check this to allow users to configure external mailboxes.')
      );

      $form['external_mailboxes']['webmail_plus_external_mailboxes_check_interval'] = array(
      	'#type' => 'textfield',
          '#title' => t('Check interval'),
          '#default_value' => variable_get('webmail_plus_external_mailboxes_check_interval', 30),
          '#description' => t('Set the frequency (in minutes) between mailboxes checks (depends on cron).')
      );

      $form['external_mailboxes']['webmail_plus_external_mailboxes_max_failures'] = array(
      	'#type' => 'textfield',
          '#title' => t('Maximum connection failures'),
          '#default_value' => variable_get('webmail_plus_external_mailboxes_max_failures', 5),
          '#description' => t('If the connection to the mailbox fails, it will be disabled and user notified.')
      );
      */

      // publishing settings
      $form['publishing'] = array(
        '#type' => 'fieldset',
        '#title' => t('Publishing Settings'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE
      );


      $available_nodes_raw = node_get_types();

      //wpd($available_nodes_raw);
      
      if(sizeof($available_nodes_raw)>0) {
        foreach($available_nodes_raw as $available_node_id=>$available_node_info) {
          $allowed_publish_nodes[$available_node_info->type]=$available_node_info->name;
        }

        $form['publishing']['webmail_plus_allowed_publish_nodes'] = array(
        	'#type' => 'checkboxes',
        	'#title' => t('Allow publishing to these nodes'),
         	'#default_value' => variable_get('webmail_plus_allowed_publish_nodes', array()),
        	'#options' => $allowed_publish_nodes,
        	'#description' => t('Select nodes which a user can publish an email as.')
        );
      }
      
      
      if(module_exists('webmail_plus_crypt')) {
        $form['security'] = array(
          '#type' => 'fieldset',
          '#title' => t('Security Settings'),
          '#collapsible' => TRUE,
          '#collapsed' => TRUE
        );
  
        
  
        $form['security']['webmail_plus_cypher'] = array(
          '#title' => 'Password Cypher',  
          '#type' => 'select',
          '#options' => webmail_plus_crypt_cyphers(),
          '#default_value' => variable_get("webmail_plus_cypher", "rijndael-128"),
        );
        
        
    	$form['security']['webmail_plus_key'] = array(
        	'#type' => 'textfield',
        	'#title' => t('Key'),
        	'#description' => t("The key for your encryption system. You normally don't need to worry about this since this module will generate a key for you if none is specified. However you have the option of using your own custom key here."),
    	);
    
  
      }
      
      // publishing settings
      $form['debugging'] = array(
        '#type' => 'fieldset',
        '#title' => t('Debugging Settings'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE
      );

      $form['debugging']['webmail_plus_debug'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable debugging'),
        '#default_value' => variable_get('webmail_plus_debug', FALSE),
        '#description' => t('Writes debugging information into the watchdog log.')
      );


      return system_settings_form($form);
	
}

?>