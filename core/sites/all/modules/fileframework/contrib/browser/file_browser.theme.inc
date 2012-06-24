<?php
// $Id$

/**
 * @file_browser
 * Themes for file browser.
 */

//////////////////////////////////////////////////////////////////////////////
// Theme callbacks

/**
 * Renders a browser page.
 */
function theme_file_browser_page($data) {
  extract($data, EXTR_SKIP);

  drupal_add_css(drupal_get_path('module', 'file') .'/file.css');
  drupal_add_js(drupal_get_path('module', 'file') .'/file.js');

  $output = '<div class="file-system" id="file-system-'. $vid .'">';
  $output .= $terms;
  $output .= '</div>';
  $output .= '<script type="text/javascript">$(document).ready(function() { Drupal.file_browserInit(\'page\'); });</script>';
  $output .= '<form id="file_browser-ajax-urlholder-form"><input id="file-ajax-url" type="hidden" value="'. url('file_browser/ajax') .'" /></form>';
  return $output;
}

/**
 * Renders a term line.
 */
function theme_file_browser_term($data) {
  extract($data, EXTR_SKIP);

  $count = $count_terms + $count_files;
  $path = ($gid > 0) ? 'node/'. $gid .'/gallery/' : 'file_gallery/';
  $output = '<div class="'. $xhtml_class . ($hierarchy ? ' hierarchy' : '') . (!$count ? ' empty' : '') .'" id="'. $xhtml_id .'" style="display: block;">';
  $output .= '<div class="file-cells" onClick="Drupal.file_browserFolderClick(this);">';
  $output .= '<div class="file-toggle"></div>';
  $output .= FILE_BROWSER_FOLDER_LINKS ? '' : '<div class="file-icon"></div>';
  $output .= '<div class="file-title">'. (FILE_BROWSER_FOLDER_LINKS ? theme('file_browser_term_render', $term, $path) : check_plain($term->name)) .'</div>';
  $output .= '<div id ="'. $xhtml_id .'-spinner" class="file-spinner" style="display: none;"></div>';
  if ($columns) {
    $output .= '<div class="file-date">'. $count_files .'</div>';
    $output .= '<div class="file-size">'. $size .'</div>';
  }
  $output .= '</div></div>';
  if ($expanded) {
    $output .= '<script type="text/javascript">$(document).ready(function() { $(\'#'. $xhtml_id .'\').children().each(function() { Drupal.file_browserFolderClick(this) }); });</script>';
  }
  return $output;
}

/**
 * Renders a file line.
 */
function theme_file_browser_file($data) {
  extract($data, EXTR_SKIP);

  $title_length = $columns ? FILE_BROWSER_TITLE_LENGTH : FILE_BROWSER_TITLE_LENGTH_SHORT;
  $title = drupal_strlen($title) > $title_length ? drupal_substr($title, 0, $title_length - 3) .'...' : $title;
  $file->name = $title;

  $output = '<div class="file-node" id="file-node-'. $id .'" style="display: block;">';
  $output .= '<div class="file-cells" onClick="Drupal.file_browserFileClick(this);">';
  $output .= '<div class="file-icon"></div>';

  $output .= '<div class="file-title">'. theme('file_render', $file, array('info' => array()), $iframe) .'</div>';
  if ($columns) {
    $output .= '<div class="file-date">'. $date .'</div>';
    $output .= '<div class="file-size">'. $size .'</div>';
  }
  $output .= '</div>';
  $output .= '</div>';
  $output .= '<script type=\'text/javascript\'>$(\'a.file-tip\').cluetip();</script>';
  return $output;
}

/**
 * Outputs a file preview block.
 */
function theme_file_browser_preview($data) {
  extract($data, EXTR_SKIP);

  $output = '<input id="file-preview-url" type="hidden" value="'. url('file_browser/preview') .'" />';
  $output .= '<div id="file-preview-spinner" style="display: none;"></div>';
  $output .= '<div id="file-preview">';

  if (isset($node)) {
    $file = $node->file;
    $file->name = isset($file->name) ? $file->name : $node->title;
    $file->name = drupal_strlen($file->name) > FILE_BROWSER_TITLE_LENGTH_BLOCK ? drupal_substr($file->name, 0, FILE_BROWSER_TITLE_LENGTH_BLOCK - 3) .'...' : $file->name;
    $output .= '<div id="file-preview-info">';
    $output .= '<table>';
    $output .= '<tr><th>'. t('File:') .'</th><td>'. theme('file_render', $file, array('info' => array())) .'</td></tr>';
    $output .= '<tr><th>'. t('Author:') .'</th><td>'. theme('username', user_load($node->uid)) .'</td></tr>';
    $output .= '<tr><th>'. t('Created:') .'</th><td>'. format_date($node->created, 'small') .'</td></tr>';
    $output .= '<tr><th>'. t('Modified:') .'</th><td>'. format_date($node->changed, 'small') .'</td></tr>';
    $output .= '<tr><th>'. t('Type:') .'</th><td>'. file_mime_description_for($file->type) .'</td></tr>';
    $output .= '<tr><th>'. t('Size:') .'</th><td>'. format_size($file->size) .'</td></tr>';
    if (FILE_VIEW_LINK)
      $output .= '<tr><th>'. t('Views:') .'</th><td>'. $file->views .'</td></tr>';
    $output .= '<tr><th>'. t('Downloads:') .'</th><td>'. $file->downloads .'</td></tr>';

    // Integration with og.module
    if (!empty($groups)) {
      $output .= '<tr><th>'. t('Groups:') .'</th><td>'. implode(', ', $groups) .'</td></tr>';
    }

    // Integration with location.module
    if (!empty($location)) {
      $output .= '<tr><th>'. t('Latitude:') .'</th><td>'. $location['latitude'] .'</td></tr>';
      $output .= '<tr><th>'. t('Longitude:') .'</th><td>'. $location['longitude'] .'</td></tr>';
    }
    $output .= '</table>';
    $output .= '<script type=\'text/javascript\'>$(\'#file-preview-info\').find(\'span.file.with-menu\').click(function(event) { $(this).toggleClass(\'active\'); $(this).find(\'ul\').toggle(); });</script>';
    $output .= isset($url_preview) ? l($thumbnail, $url_preview, array('html' => TRUE, 'attributes' => array('title' => check_plain($node->title)))) : $thumbnail;
    $output .= '<script type=\'text/javascript\'>$(\'a.file-tip\').cluetip();</script>';
    $output .= '</div>';

    // Backreferences
    if (!empty($nodes)) {
      $output .= '<div id="file-references">';
      $output .= '<span>'. t('This file is attached to') .':</span>';
      $output .= '<div class="item-list">';
      $output .= '<ul>';
      foreach ($nodes as $n) {
        $output .= '<li>'. l($n->title, 'node/'. $n->nid, isset($n->cid) ? array('fragment' => 'comment-'. $n->cid) : array()) .'</li>';
      }
      $output .= '</ul>';
      $output .= '</div>';
    }
  }

  $output .= '</div>'; // id=file-preview

  // File move form.
  if (isset($form)) {
    $output .= '<div id="file-move">';
    $output .= $form;
    $output .= '</div>';
    $output .= '<iframe src="javascript:;" id="file_browser-move-submission-frame" name="file_browser-move-submission-frame" border="0" frameborder="0" width="0" height="0"></iframe>';
  }

  return $output;
}

/**
 * Outputs a browser block.
 */
function theme_file_browser_block($data) {
  extract($data, EXTR_SKIP);

  $output = '<div class="file-system" id="file-system-'. $vid .'">';
  $output .= $terms;
  $output .= '</div>';
  $output .= '<div class="more-link">'. l(t('more'), 'file_browser/'. $vid, array('title' => t('View file browser'))) .'</div>';
  $output .= '<script type="text/javascript">Drupal.file_browserInit(\''. $vid .'\');</script>';
  $output .= '<form id="file-node-url-form"><input id="file-node-url" type="hidden" value="'. url('node/') .'" /></form>';
  $output .= '<form id="file_browser-ajax-urlholder-form"><input id="file-ajax-url" type="hidden" value="'. url('file_browser/ajax') .'" /></form>';
  return $output;
}

/**
 * Outputs a create new term block.
 */
function theme_file_browser_newterm($form) {
  $output = '<div id="file_browser-newterm">';
  $output .= $form;
  $output .= '</div>';
  $output .= '<iframe src="javascript:;" id="file_browser-newterm-submission-frame" name="file_browser-newterm-submission-frame" border="0" frameborder="0" width="0" height="0"></iframe>';
  return $output;
}

/**
 * Renders a file upload block
 */
function theme_file_browser_upload($form) {
  $output = '<div id="file-upload-spinner" style="display: none;"></div>';
  $output .= '<div id="file-upload">';
  $output .= $form;
  $output .= '</div>';
  $output .= '<iframe src="javascript:;" id="file_browser-upload-submission-frame" name="file_browser-upload-submission-frame" border="0" frameborder="0" width="0" height="0"></iframe>';
  return $output;
}

/**
 * Theme for showing a term with a links.
 */
function theme_file_browser_term_render($term, $path, $links_add = array()) {
  $links['show'] = array('title' => t('Show'), 'href' => $path . $term->vid . (isset($term->tid) ? '/'. $term->tid : ''), 'attributes' => array('title' => t('Show files in the gallery.')));
  $links = theme('links', array_merge($links, $links_add), array('class' => 'file-links'));
  $output = '<span class="file with-menu"><span class="label"><span class="highlight"></span><span title="'. $term->name .'" class="name">'. theme('image', drupal_get_path('module', 'file_browser') .'/icons/folder.png') .' <span class="title" style="vertical-align: top;">'. check_plain($term->name) .'</span></span>'. $links .'</span></span>';
  $output = preg_replace("/\n/", '', $output);
  return $output;
}

