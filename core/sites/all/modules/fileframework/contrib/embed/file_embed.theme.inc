<?php
// $Id$

/**
 * @file_embed
 * Themes for file embed.
 */

//////////////////////////////////////////////////////////////////////////////
// Theme callbacks

/**
 * Outputs textarea links.
 */
function theme_file_embed_textarea_link($data) {
  extract($data, EXTR_SKIP);

  static $initialized = FALSE;
  if (!$initialized) {
    $initialized = TRUE;

    // Add stylesheets and javascript. Thickbox must come before the module's stylesheet:
    drupal_add_css(drupal_get_path('module', 'file') .'/jquery/thickbox/thickbox.css', 'module');
    drupal_add_js(drupal_get_path('module', 'file') .'/jquery/thickbox/thickbox.js', 'module');

    // Cluetip for metadata.
    drupal_add_css(drupal_get_path('module', 'file') .'/jquery/cluetip/jquery.cluetip.css', 'module');
    drupal_add_js(drupal_get_path('module', 'file') .'/jquery/cluetip/jquery.dimensions-1.2.js', 'module');
    drupal_add_js(drupal_get_path('module', 'file') .'/jquery/cluetip/jquery.cluetip.js', 'module');

    drupal_add_js('tb_pathToImage='. drupal_to_js('/'. drupal_get_path('module', 'file') .'/jquery/thickbox/loadingAnimation.gif') .';', 'inline');
  }

  $output = '<div class="file-embed-actions" id="file-embed-'. $id .'">';
  $output .= l($link != 'image' ? t('embed file') : theme('image', drupal_get_path('module', 'file_embed') .'/images/add-image.jpg', t('embed an existing file'), t('Click here to add files from the database'), array('border' => 0)), $url['url'], array('html' => TRUE, 'query' => $url['query'], 'attributes' => array('class' => 'file-embed-select')));
  $output .= '</div>';
  return $output;
}

/**
 * Creates an embed option page.
 */
function theme_file_embed_options($data) {
  extract($data, EXTR_SKIP);

  $output = $script;
  $output .= '<div id="file-gallery-head">';
  $output .= theme('breadcrumb', array('Embed options'));
  $output .= '</div>';
  $output .= '<div id="file-gallery-body">';
  $output .= '<div style="text-align: center; padding-bottom: 10px;"><h4>'. $title .'</h4></div>';
  $output .= '<div style="float: right;"><br />'. (isset($preview) ? $preview : '<br /><span class="no-thumbnail">'. t('No preview') .'</span><br />') .'</div>';
  $output .= $form;
  $output .= '</div>';
  return $output;
}

/**
 * Outputs embeded file.
 */
function theme_file_embed_file($data) {
  extract($data, EXTR_SKIP);

  $output = '<div class="embed'. (isset($align) ? '-'. $align : '') .'">';
  $output .= '<div class="embed-file">'. $file_html .'</div>';
  $output .= isset($caption) ? '<div class="embed-caption">'. $caption .'</div>' : '';
  $output .= '</div>';
  return $output;
}

