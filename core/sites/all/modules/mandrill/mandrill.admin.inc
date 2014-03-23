<?php
/**
 * @file
 * Administrative forms for Mandrill module.
 */


/**
 * Administrative settings.
 *
 * @return
 *   An array containing form items to place on the module settings page.
 */
function mandrill_admin_settings(&$form_state) {
  $key = variable_get('mandrill_api_key', '');
  $form['mandrill_api_key'] = array(
    '#title' => t('Mandrill API Key'),
    '#type' => 'textfield',
    '#description' => t('Create or grab your API key from the !link.',
      array('!link' => l('Mandrill settings', 'https://mandrillapp.com/settings/index'))),
    '#default_value' => $key
  );

  if ($key) {
    $form['mandrill_status'] = array(
      '#type' => 'radios',
      '#title' => t('Mandrill Mail interface status'),
      '#default_value' => variable_get('mandrill_status', MANDRILL_STATUS_OFF),
      '#options' => array(
        MANDRILL_STATUS_ON => t('On'),
        MANDRILL_STATUS_TEST => t('Test'),
        MANDRILL_STATUS_OFF => t('Off')
      ),
      '#description' => t('Setting to on routes all site emails through the Mandrill
        API. Test mode implements an alternate mail interface,
        TestingMailChimpMandrillMailSystem, that does not send any emails, it just
        displays a message and logs the event.'),
    );

    $form['email_options'] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#title' => t('Email options'),
    );
    $form['email_options']['mandrill_from'] = array(
      '#title' => t('From address'),
      '#type' => 'textfield',
      '#description' => t('The sender email address. If this address has not been verified, messages will be queued and not sent until it is.'),
      '#default_value' => variable_get('mandrill_from', variable_get('site_mail', ''))
    );
    $form['email_options']['mandrill_from_name'] = array(
      '#type' => 'textfield',
      '#title' => t('From name'),
      '#default_value' => variable_get('mandrill_from_name', ''),
      '#description' => t('Optionally enter a from name to be used.'),
    );

    $formats = filter_formats();
    $options = array('' => t('-- Select --'));
    foreach ($formats AS $v => $format) {
      $options[$v] = $format->name;
    }
    $form['email_options']['mandrill_filter_format'] = array(
      '#type' => 'select',
      '#title' => t('Input format'),
      '#description' => t('If selected, the input format to apply to the message body before sending to the Mandrill API.'),
      '#options' => $options,
      '#default_value' => array(variable_get('mandrill_filter_format', '')),
    );

    $form['send_options'] = array(
      '#title' => t('Send options'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
    );
    $form['send_options']['mandrill_track_opens'] = array(
      '#title' => t('Track opens'),
      '#type' => 'checkbox',
      '#description' => t('Whether or not to turn on open tracking for messages.'),
      '#default_value' => variable_get('mandrill_track_opens', TRUE)
    );
    $form['send_options']['mandrill_track_clicks'] = array(
      '#title' => t('Track clicks'),
      '#type' => 'checkbox',
      '#description' => t('Whether or not to turn on click tracking for messages.'),
      '#default_value' => variable_get('mandrill_track_clicks', TRUE)
    );
    $form['send_options']['mandrill_url_strip_qs'] = array(
      '#title' => t('Strip query string'),
      '#type' => 'checkbox',
      '#description' => t('Whether or not to strip the query string from URLs when aggregating tracked URL data.'),
      '#default_value' => variable_get('mandrill_url_strip_qs', FALSE)
    );
    $form['send_options']['mandrill_mail_key_blacklist'] = array(
      '#title' => t('Content logging blacklist'),
      '#type' => 'textarea',
      '#description' => t('Comma delimited list of Drupal mail keys to exclude content logging for. CAUTION: Removing the default password reset key may expose a security risk.'),
      '#default_value' => mandrill_mail_key_blacklist()
    );

    $form['analytics'] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#title' => t('Google analytics'),
    );
    $form['analytics']['mandrill_analytics_domains'] = array(
      '#title' => t('Google analytics domains'),
      '#type' => 'textfield',
      '#description' => t('One or more domains for which any matching URLs will automatically have Google Analytics parameters appended to their query string. Separate each domain with a comma.'),
      '#default_value' => variable_get('mandrill_analytics_domains', '')
    );
    $form['analytics']['mandrill_analytics_campaign'] = array(
      '#title' => t('Google analytics campaign'),
      '#type' => 'textfield',
      '#description' => t('The value to set for the utm_campaign tracking parameter. If this isn\'t provided the messages from address will be used instead.'),
      '#default_value' => variable_get('mandrill_analytics_campaign', '')
    );

    $form['#submit'][] = 'mandrill_admin_settings_submit';
  }

  return system_settings_form($form);
}

/**
 * Submission for the administrative settings form.
 *
 * @param form
 *   An associative array containing the structure of the form.
 * @param form_state
 *   A keyed array containing the current state of the form.
 */
function mandrill_admin_settings_submit($form, &$form_state) {
  $values = $form_state['values'];

  $mail_inc = drupal_get_path('module', 'mandrill') . '/mandrill.mail.inc';

  switch ($form_state['values']['mandrill_status']) {
    case MANDRILL_STATUS_ON:
      variable_set('smtp_library', $mail_inc);
      drupal_set_message(t('Mandrill will be used to deliver all site emails.'));
      watchdog('mandrill', 'Mandrill has been enabled.');
      break;
    case MANDRILL_STATUS_TEST:
      variable_set('smtp_library', $mail_inc);
      drupal_set_message(t('Mandrill will be used in test mode. Emails will not actually be sent, just logged.'));
      watchdog('mandrill', 'Mandrill has been placed in test mode.');
      break;
    case MANDRILL_STATUS_OFF:
      if (variable_get('smtp_library', '') == $mail_inc) {
        variable_del('smtp_library');
      }
      drupal_set_message(t('The default mail system will be used to deliver all site emails.'));
      watchdog('mandrill', 'Mandrill has been disabled.');
      break;
  }
}

/**
 * Return a form for sending a test email.
 *
 * @param string $form
 * @param string $form_state
 *
 * @return array $form
 */
function mandrill_test_form(&$form_state) {
  drupal_set_title(t('Send test email'));

  $form['mandrill_test_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Email address to send a test email to'),
    '#default_value' => variable_get('site_mail', ''),
    '#description' => t('Type in an address to have a test email sent there.'),
    '#required' => TRUE
  );
  $form['mandrill_test_body'] = array(
    '#type' => 'textarea',
    '#title' => t('Test body contents'),
    '#default_value' => t('If you receive this message it means your site is capable of using Mandrill to send email. This <a href="http://drupal.org">link</a> is here to test click tracking.'),
  );
  $form['include_attachment'] = array(
    '#title' => t('Include attachment'),
    '#type' => 'checkbox',
    '#description' => t('If checked, the Drupal icon will be included as an attachment with the test email.'),
    '#default_value' => TRUE
  );

  $form['test_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Send test email'),
  );
  $form['test_cancel'] = array(
    '#type' => 'link',
    '#href' => 'admin/settings/mandrill',
    '#title' => t('Cancel'),
  );

  return $form;
}

/**
 * Submit handler for mandrill_test_form(), sends the test email.
 *
 * @param string $form
 * @param string $form_state
 *
 * @return void
 */
function mandrill_test_form_submit($form, &$form_state) {
  // If an address was given, send a test email message.
  $test_address = $form_state['values']['mandrill_test_address'];
  global $language;
  $params['subject'] = t('Drupal Mandrill test email');
  $params['body'] = $form_state['values']['mandrill_test_body'];
  $params['include_attachment'] = $form_state['values']['include_attachment'];
  $result = drupal_mail('mandrill', 'mandrill-test', $test_address, $language, $params, variable_get('mandrill_from', ''));
  if (isset($result['result']) && $result['result'] === true) {
    drupal_set_message(t('A test email has been sent to @email.', array('@email' => $test_address)));
  }
}
