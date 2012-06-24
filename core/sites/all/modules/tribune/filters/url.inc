<?php
// vim:filetype=php expandtab tabstop=2 softtabstop=2 shiftwidth=2 autoindent smartindent
// $Id: url.inc,v 1.5.2.2 2009/04/12 16:59:29 seeschloss Exp $

function tribune_url_info() {
  $info = array(
    'description' => t("Turns 'http://...' into clickable links."),
    'weight'      => 0,
  );

  return $info;
}

function tribune_url_filter_after(&$post, &$help) {
  $post['parsed'] = preg_replace_callback('#'.
      '('.                                        // 1
        '(https?|ftp|gopher|file|mms|rtsp|rtmp)'. // 2
        '://.*?'.
      ')'.
      '('.                                        // 3
        '(,|\.|\)|\]|\})?'.                       // 4
        '(<| | |"|$)'.                            // 5
      ')'.
      '#', 'tribune_url_replace', $post['parsed']);
}

function tribune_url_replace($matches) {
  $url = $matches[1];
  $text = 'url';

  $before = "";
  $after  = "";

  if ($matches[4]) {
    if (substr_count($url, "(") == substr_count($url, ")") + 1) {
      $url .= $matches[4];
    } else {
      $after = $matches[4];
    }
  }

  $after .= $matches[5];

  foreach (explode("\n", tribune_variable_get('tribune_url_replacement', "")) as $line) {
    preg_match(':^/(.*[^\\\\])/(.*)$:U', trim($line), $result);

    if (count($result) == 3) {
      $rmatches = array();
      if (@preg_match('/'. $result[1] .'/', $url, $rmatches)) {
        $text = trim($result[2]);
        foreach ($rmatches as $key => $value) {
          $text = str_replace('$'.$key, $value, $text);
        }
      }
    }
  }

  return $before .'<a href="'. $url .'">['. $text .']</a>'. $after;
}

function tribune_url_settings() {
  $form = array();

  $form['tribune_url_replacement'] = array(
    '#type'           => "textarea",
    '#title'          => t("URL replacement expressions"),
    '#default_value'  => tribune_variable_get('tribune_url_replacement', ""),
    '#description'    => t("Use this format: <code>/regexp/ replacement string</code><br />You can escape /'s in the regexp with a \.<br />If any url matches an expression (according to <code>!preg_match_url</code>), it will be replaced with <em>[replacement string]</em> instead of the default <em>[url]</em>. You can use backreferences: '$1' will be replaced with the first expression between parenthesis.", array('!preg_match_url' => l('preg_match', 'http://php.net/preg_match'))),
  );

  return tribune_settings_form($form);
}
