<?php
// $Id$

/**
 * @file
 * Themes for slideshow file formats.
 */

//////////////////////////////////////////////////////////////////////////////
// Theme callbacks

/**
 * Theme for admin PDF checkboxes.
 */
function theme_file_slideshow_admin_settings($form) {
  $rows = array();
  foreach ($form['pdf'] as $name => $element) {
    if (preg_match('/pdf_/', $name)) {
      $rows[] = array(
        drupal_render($form['pdf'][$name][0]),
        drupal_render($form['pdf'][$name][1]),
        drupal_render($form['pdf'][$name][2])
      );
      unset($form['pdf'][$name]);
    }
  }
  $form['pdf']['pdf'] = array('#type' => 'markup', '#value' => theme('table', NULL, $rows));
  return drupal_render($form);
}

