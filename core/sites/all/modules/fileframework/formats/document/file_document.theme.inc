<?php
// $Id$

/**
 * @file
 * Themes for document file formats.
 */

//////////////////////////////////////////////////////////////////////////////
// Theme callbacks

/**
 * Theme for Mind map document rendering.
 */
function theme_file_document_mm_render($options = array()) {
  extract($options, EXTR_SKIP);

  $player_path = url() . drupal_get_path('module', 'file') .'/vendor/freemind';

  $output = '<applet code="freemind.main.FreeMindApplet.class" archive="'. $player_path .'/freemind.jar" width="100%" height="500">'."\n";
  $output .= '<param name="type" value="application/x-java-applet;version=1.4" />'."\n";
  $output .= '<param name="scriptable" value="false" />'."\n";
  $output .= '<param name="modes" value="freemind.modes.browsemode.BrowseMode" />'."\n";
  $output .= '<param name="browsemode_initial_map" value="'. $src .'" />'."\n";
  $output .= '<param name="initial_mode" value="Browse" />'."\n";
  $output .= '<param name="selection_method" value="selection_method_direct" />'."\n";
  $output .= '</applet>'."\n";

  return $output;
}

