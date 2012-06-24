<?php
// $Id$

/**
 * @file
 * Themes for text file formats.
 */

//////////////////////////////////////////////////////////////////////////////
// Theme callbacks

/**
 * Theme for text rendering.
 */
function theme_file_text_text_render($options = array()) {
  extract($options, EXTR_SKIP);

  $form['preview'] = array(
    '#id' => 'file_text_text',
    '#type' => 'textarea',
    '#name' => 'file_preview',
    '#rows' => 15,
    '#cols' => 80,
    '#attributes' => array('disabled' => 'disabled'),
    '#value' => utf8_encode($text) . (isset($append) ? $append : ''),
  );
  return drupal_render($form);
}

/**
 * Theme for code text rendering.
 */
function theme_file_text_code_render($options = array()) {
  extract($options, EXTR_SKIP);

  $src = bitcache_resolve_uri($uri, array('absolute' => FALSE, 'query' => array_merge(!empty($vid) ? array('vid' => $vid) : array(), array('disposition' => 'inline'))));
  return '<iframe src="'. $src .'" width="580px" height="350px"></iframe>';
}

