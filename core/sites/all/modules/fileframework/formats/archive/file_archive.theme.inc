<?php
// $Id$

/**
 * @file
 * Themes for archive file formats.
 */
//////////////////////////////////////////////////////////////////////////////
// Theme callbacks

/**
 * Theme for archive rendering.
 */
function theme_file_archive_archive_render($options = array()) {
  extract($options, EXTR_SKIP);

  return '<code>'. preg_replace(array('/ /', "/\n/"), array('&nbsp;', '<br />'),  $text) .'</code>';
}

