<?php
// $Id$

/**
 * @file
 * Themes for image file formats.
 */

//////////////////////////////////////////////////////////////////////////////
// Theme callbacks

/**
 * Theme for admin EXIF checkboxes.
 */
function theme_file_image_admin_settings($form) {
  $rows = array();
  foreach ($form['exif'] as $name => $element) {
    if (preg_match('/exif_/', $name)) {
      $rows[] = array(
        drupal_render($form['exif'][$name][0]),
        drupal_render($form['exif'][$name][1]),
        drupal_render($form['exif'][$name][2])
      );
      unset($form['exif'][$name]);
    }
  }
  $form['exif']['exif'] = array('#type' => 'markup', '#value' => theme('table', NULL, $rows));
  return drupal_render($form);
}

/**
 * Theme for image rendering.
 */
function theme_file_image_image_render($options = array()) {
  extract($options, EXTR_SKIP);

  $output = '';
  if (!isset($container) || !empty($container)) {
    $output .= '<span class="file-image" style="display: block; margin: 0px; padding: 0px;';
    $output .= isset($width) ? ' width: '. min($width, $max_width) .'px;' : '';
    $output .= isset($height) ? ' height: '. min($height, $max_height) .'px;' : '';
    $output .= isset($width) || isset($height) ? ' overflow: hidden;' : '';
    $output .= '">';
  }
  $output .= '<img ';
  $allowed_keys = _file_image_allowed_attributes();
  foreach ($options as $key => $value) {
    if (in_array($key, $allowed_keys) && !empty($value)) {
      // Restrict HTML attribute names to safe values
      $key = preg_replace('/[^a-zA-Z0-9\-\._:]+/', '', strip_tags($key));
      $output .= $key .'="'. check_plain($value) .'" ';
    }
  }
  $output .= '/>';
  if (!isset($container) || !empty($container)) {
    $output .= '</span>';
  }

  return $output;
}

