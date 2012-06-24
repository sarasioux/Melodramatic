<?php
// vim:filetype=php expandtab tabstop=2 softtabstop=2 shiftwidth=2 autoindent smartindent
// $Id: censor.inc,v 1.6.2.1 2008/12/01 08:21:53 seeschloss Exp $

function tribune_censor_info() {
  return t('Censors messages.');
}

function tribune_censor_filter(&$post) {
  foreach (explode("\n", tribune_variable_get('tribune_censor_words', "")) as $line) {
    if (trim($line)) {
      $line = str_replace('/', '\/', trim($line));
      if (@preg_match('/'. $line .'/', $post['message'])) {
        if (tribune_variable_get('tribune_censor_alert', TRUE)) {
          $new_message = array(
            'info' => tribune_variable_get('tribune_censor_name', "Big brother"),
            'message' => "A post by \"". $post['login'] ."\" was censored.",
          );

          if (tribune_variable_get('tribune_censor_authentified', FALSE)) {
            $new_message['login'] = $new_message['info'];
          }

          $post = NULL;
          return array($new_message);
        }

        $post = NULL;
      }
    }
  }
}

function tribune_censor_settings() {
  $form = array();

  $form['tribune_censor_alert'] = array(
    '#type' => "checkbox",
    '#title' => t("Alert when a post is censored"),
    '#default_value' => tribune_variable_get('tribune_censor_alert', TRUE),
    '#description' => t("Whether this filter will post an explanation message when it censors a post."),
  );

  $form['tribune_censor_name'] = array(
    '#type' => "textfield",
    '#title' => t("Display name"),
    '#default_value' => tribune_variable_get('tribune_censor_name', "Big brother"),
  );

  $form['tribune_censor_authentified'] = array(
    '#type' => "checkbox",
    '#title' => t("Appear to be authentified"),
    '#default_value' => tribune_variable_get('tribune_censor_authentified', FALSE),
    '#description' => t("Whether this filter will appear to be 'anonymous' or 'authentified'. Since the filter does not correspond to a real user, posting as authentified may result in 'nickname collisions'."),
  );

  $form['tribune_censor_words'] = array(
    '#type' => "textarea",
    '#title' => t("censored expressions"),
    '#default_value' => tribune_variable_get('tribune_censor_words', ""),
    '#description' => t("One regular expression per line. If any message matches any of these expressions (according to <code>!preg_match_url</code>), it will be censored.", array('!preg_match_url' => l('preg_match', 'http://php.net/preg_match'))),
  );

  return tribune_settings_form($form);
}
