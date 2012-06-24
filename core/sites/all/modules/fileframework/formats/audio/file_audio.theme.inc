<?php
// $Id$

/**
 * @file
 * Themes for audio file formats.
 */

//////////////////////////////////////////////////////////////////////////////
// Theme callbacks

/**
 * Theme for admin getID checkboxes.
 */
function theme_file_audio_admin_settings($form) {
  $rows = array();
  foreach ($form['getid3'] as $name => $element) {
    if (preg_match('/getid3_/', $name)) {
      $rows[] = array(
        drupal_render($form['getid3'][$name][0]),
        drupal_render($form['getid3'][$name][1]),
        drupal_render($form['getid3'][$name][2])
      );
      unset($form['getid3'][$name]);
    }
  }
  $form['getid3']['getid3'] = array('#type' => 'markup', '#value' => theme('table', NULL, $rows));
  return drupal_render($form);
}

/**
 * Renders a Flash-based MP3 player for audio previews.
 */
function theme_file_audio_mp3_render($options = array()) {
  extract($options, EXTR_SKIP);

  $output = "\n".'<div'. drupal_attributes(array('class' => $class)) .'>';
  unset($options['src'], $options['nid'], $options['vid'], $options['title'], $options['class'], $options['continer']);
  $options = array_merge((array) $options, array(
    'playlist_url' => url('file_audio/xspf/'. $src),
    'player_title' => $title,
  ));
  $player_path = url() . drupal_get_path('module', 'file') .'/vendor/xspf_player';

  $url = $player_path .'/xspf_player_slim.swf?'. drupal_query_string_encode($options);
  $output .= '<object type="application/x-shockwave-flash" data="'. $url .'" width="400" height="15">';
  $output .= '<param name="movie" value="'. $url .'" />';
  $output .= '</object>';
  if (!isset($container) || !empty($container)) {
    $output .= '<p><small>'. t('Problems with audio?') .' ';
    $output .= str_replace('&gt;&gt;', '&raquo;', l(t('Download the latest Adobe Flash Player') .' >>', FILE_AUDIO_FLASH_LINK, array('title' => $title), NULL, NULL, TRUE));
    $output .= '</small></p>';
  }
  $output .= '</div>'."\n";
  return $output;
}

