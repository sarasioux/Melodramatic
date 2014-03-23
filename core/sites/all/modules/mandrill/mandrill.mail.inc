<?php

/**
 * @file
 * Class for sending mails via Mandrill.
 */

function _mandrill_remove_newlines($matches) {
  return str_replace(array("\n", "\r"), ' ', $matches[0]);
}

/**
 * Modify the drupal mail system to use Mandrill when sending emails.
 */
class MandrillMailSystem {

  /**
   * Concatenate and wrap the email body for either
   * plain-text or HTML emails.
   *
   * @param $message
   *   A message array, as described in hook_mail_alter().
   *
   * @return
   *   The formatted $message.
   */
  public function format(array $message) {
    // Join the body array into one string.
    if (is_array($message['body'])) {
      $message['body'] = implode("\n\n", $message['body']);
    }

    // Undoes damage to the HTML e-mail done by drupal_wrap_mail() because this can break
    // check_markup() depending on the filters you're using -- particularly the newline filter.
    // Ideally, we'd run check_markup() before the message is passed to drupal_wrap_mail(), but
    // I don't see a good way to do that!
    $message['body'] = preg_replace_callback('/<[^<>]*>/s', '_mandrill_remove_newlines', $message['body']);

    return $message;
  }

  /**
   * Send the email message.
   *
   * @see drupal_mail()
   *
   * @param $message
   *   A message array, as described in hook_mail_alter().
   *
   * @return
   *   TRUE if the mail was successfully accepted, otherwise FALSE.
   */
  public function mail(array $message) {
    if (!$from = variable_get('mandrill_from', '')) {
      drupal_set_message(t('Mandrill can\'t send email. Please !link.',
        array(
          '!link' => l('add a verified from address',
            'admin/settings/mandrill'
          )
        )
      ), 'error');
      return FALSE;
    }

    // send the email passing the message id as the tag for use in reporting
    $mailer = mandrill_get_api_object();

    // apply input format to body
    $html = $message['body'];
    $format = variable_get('mandrill_filter_format', '');
    if (!empty($format)) {
      $html = check_markup($message['body'], $format, FALSE);
    }

    $to = mandrill_get_to($message['to']);

    $attachments = array();
    if (isset($message['attachments']) && !empty($message['attachments'])) {
      foreach ($message['attachments'] as $attachment) {
        if (is_file($attachment)) {
          $attachments[] = $mailer->getAttachmentStruct($attachment);
        }
      }
    }

    // determine if content should be available for this message
    $blacklisted_keys = explode(',', mandrill_mail_key_blacklist());
    $view_content = TRUE;
    foreach ($blacklisted_keys as $key) {
      if  ($message['id'] == drupal_strtolower(trim($key))) {
        $view_content = FALSE;
        break;
      }
    }

    $mandrill_message = array(
      'html' => $html,
      'text' => drupal_html_to_text($message['body']),
      'subject' => $message['subject'],
      'from_name' => variable_get('mandrill_from_name', ''),
      'from_email' => $from,
      'to' => $to,
      // optional extra headers to add to the message (currently only Reply-To and X-* headers are allowed)
      'headers' => $message['headers'],
      'track_opens' => variable_get('mandrill_track_opens', TRUE),
      'track_clicks' => variable_get('mandrill_track_clicks', TRUE),
      // we're handling this with drupal_html_to_text().
      'auto_text' => FALSE,
      'url_strip_qs' => variable_get('mandrill_url_strip_qs', FALSE),
      'bcc_address' => isset($message['bcc_email']) ? $message['bcc_email'] : NULL,
      'tags' => array($message['id']),
      'google_analytics_domains' => (variable_get('mandrill_analytics_domains', NULL)) ? explode(',', variable_get('mandrill_analytics_domains')) : array(),
      'google_analytics_campaign' => variable_get('mandrill_analytics_campaign', ''),
      'attachments' => $attachments,
      'view_content_link' => $view_content
    );

    drupal_alter('mandrill_mail', $mandrill_message, $message);

    try {
      $result = $mailer->messages_send($mandrill_message);
      // @todo: look for rejected messages and log
      return TRUE;
    }
    catch (Mandrill_Exception $e) {
      watchdog('mandrill', 'Error sending email from %from to %to. @code: @message',
        array(
          '%from' => $from,
          '%to' => $message['to'],
          '@code' => $e->getCode(),
          '@message' => $e->getMessage(),
        ),
        WATCHDOG_ERROR
      );

      return FALSE;
    }
  }

}

/**
 * A mail sending implementation that captures sent messages to watchdog.
 *
 * This class is for running tests or for development.
 */
class TestingMandrillMailSystem extends MandrillMailSystem {

  /**
   * Accept an e-mail message and store it in a variable.
   *
   * @param $message
   *   An e-mail message.
   */
  public function mail(array $message) {
    if (!$from = variable_get('mandrill_from', '')) {
      drupal_set_message(t('Mandrill can\'t send email. Please !link.',
        array(
          '!link' => l('add a verified from address',
            'admin/settings/mandrill'
          )
        )
      ), 'error');
      return FALSE;
    }
    $to = mandrill_get_to($message['to']);

    watchdog('mandrill', 'Mandrill test email sent from %from to %to. Message: <pre>@message</pre>',
      array(
        '%from' => $from, '%to' => $to[0]['email'],
	      '@message' => print_r($message, TRUE),
      ),
      WATCHDOG_NOTICE
    );

    drupal_set_message(t('Mandrill test email sent from %from to %to.',
      array('%from' => $from, '%to' => $to[0]['email'])
    ), 'notice');
    return TRUE;
  }
}

/**
 * Helper to generate an array of recipients.
 *
 * @param mixed $to
 *
 * @return array
 */
function mandrill_get_to($to) {
  //@todo: extract name and add to associative array.

  $recipients = array();
  if ($to_array = explode(',', $to)) {
    foreach ($to_array as $email) {
      $recipients[] = array('email' => $email);
    }
  }
  else {
    $recipients[] = array('email' => $to);
  }
  return $recipients;
}

/**
 * Implements drupal_mail_wrapper().
 */
function drupal_mail_wrapper($message) {
  $mandrill_status = variable_get('mandrill_status', MANDRILL_STATUS_ON);
  $mandrill = ($mandrill_status == MANDRILL_STATUS_TEST) ?
    new TestingMandrillMailSystem() : new MandrillMailSystem();

  $message = $mandrill->format($message);
  return $mandrill->mail($message);
}

