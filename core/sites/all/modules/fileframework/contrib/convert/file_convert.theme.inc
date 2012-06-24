<?php
// $Id$

/**
 * @file
 * Themes for file convert.
 */

//////////////////////////////////////////////////////////////////////////////
// Theme callbacks

/**
 * Theme for file convert admin converters settings.
 */
function theme_file_convert_admin_converter_settings($form) {
  $rows = array();
  if (!empty($form['convert'])) {
    foreach ($form['convert'] as $from => $tos) {
      if (preg_match('/^file_/', $from)) {
        list($module, $from_mime) = explode('__', $from);
        foreach ($tos as $to => $pipeline) {
          $name = preg_replace('/\./', '\\', $from) .'_'. preg_replace('/\./', '\\', $to);
          if (!preg_match('/^#/', $to)) {
            $row = array(
              $from_mime .'<br />&nbsp;&nbsp;'. $to,
              drupal_render($pipeline[$name .'_pipeline']),
              $pipeline['#check'],
              array('data' => drupal_render($pipeline[$name .'_enabled']), 'class' => 'checkbox'),
              $module,
            );
            $rows[] = array('data' => $row, 'class' => !empty($pipeline[$name .'_enabled']['#default_value']) ? ' menu-enabled' : ' menu-disabled');
          }
          unset($form['convert'][$from][$to]);
        }
      }
    }
  }
  else {
    $rows[] = array(array('data' => t('No converters defined. Enable file format modules to get them.'), 'colspan' => 5));
  }
  $header = array(t('From/To'), t('Conversion utilities'), t('Installed'), t('Enabled'), t('Module'));
  $form['convert']['convert'] = array('#type' => 'markup', '#value' => theme('table', $header, $rows));
  return drupal_render($form);
}

