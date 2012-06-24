<?php
// $Id$

/**
 * @file_gallery
 * Themes for file gallery.
 */

//////////////////////////////////////////////////////////////////////////////
// Theme callbacks

/*
 * Theme for the entire page.
 */
function theme_file_gallery_page($content, $options = array()) {
  extract($options, EXTR_SKIP);

  $output = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n";
  $output .= '<html xmlns="http://www.w3.org/1999/xhtml">';
  $output .= '<head>';
  $output .= ' <title>'. (drupal_get_title() ? strip_tags(drupal_get_title()) : variable_get('site_name', 'drupal')) .'</title>';
  $output .= drupal_get_html_head();
  $output .= drupal_get_css();
  $output .= drupal_get_js();
  $output .= '</head>';
  $output .= '<body style="min-width: 0; background-color: #fff; color: #000; margin: 10px;">';
  if (isset($breadcrumb) && $breadcrumb) {
    $output .= '<div id="wrapper" class="gallery"><div id="container"><div class="breadcrumb gallery">'. implode(' > ', drupal_get_breadcrumb()) .'</div></div></div>';
  }
  $output .= "\n<!-- begin content -->\n";
  $output .= $content;
  $output .= "\n<!-- end content -->\n";
  $output .= "</body>\n</html>";
  return $output;
}

/*
 * Theme for the taxonomy term.
 */
function theme_file_gallery_term($data, $options) {
  extract($data, EXTR_SKIP);
  extract($options, EXTR_SKIP);

  $image = theme('image', drupal_get_path('module', 'file_gallery') .'/images/folder.gif', NULL, NULL, array('width' => 50, 'height' => 50), FALSE);
  $output = '<li class="file-gallery" style="width: '. ((int)$max_width) .'px; height: '. ((int)$max_height) .'px;">';
  $output .= l($image, $url, array('html' => TRUE, 'query' => $query, 'attributes' => array('class' => 'folder', 'title' => check_plain($term->name))));
  //$title_short = drupal_strlen($term->name) > FILE_GALLERY_TITLE_LENGTH_SHORT ? drupal_substr($term->name, 0, FILE_GALLERY_TITLE_LENGTH_SHORT - 3) .'...' : $term->name;
  $title_short = $term->name;
  $output .= '<br /><span class="term">'. l($title_short, $url, array('query' => $query, 'attributes' => array('title' => check_plain($term->name)))) .'</span> <span class="term-files">('. $count_files .')</span>';
  $output .= '</li>';
  return $output;
}

/*
 * Theme for the file node.
 */
function theme_file_gallery_node($data, $options) {
  extract($data, EXTR_SKIP);
  extract($options, EXTR_SKIP);

  // If the same file is displayed several times, the js bindes incorrectly.
  // Have to ensure unique IDs.
  $id = '';
  for ($i = 0; $i < 8; $i++)
    $id .= mt_rand(0, 9);

  $output = '<li class="file-gallery" id="file-'. $file->nid .'" style="width: '. ((int)$max_width) .'px; height: '. ((int)$max_height) .'px;">';
  $output .= !empty($url_preview) ? l(isset($thumbnail) ? $thumbnail : t('No thumbnail'), $url_preview['link'], array('html' => isset($thumbnail) ? TRUE : FALSE, 'query' => isset($url_preview['query']) ? $url_preview['query'] : NULL, 'attributes' => array_merge(array('title' => $title, 'id' => 'file-'. $file->nid .'-'. $id), isset($url_preview['attributes']) ? $url_preview['attributes'] : array()))) : (isset($thumbnail) ? $thumbnail : t('No thumbnail'));
  $output .= $thickbox ? '<script type=\'text/javascript\'>tb_init(\'#file-'. $file->nid .'-'. $id .'\');</script>' : '';
  $file->name = $title_short;
  $output .= '<br />'. theme('file_render', $file, $file_links);
  $output .= '</li>';
  return $output;
}

/*
 * Theme for the empty page.
 */
function theme_file_gallery_empty() {
  $output = '<span class="file-gallery-empty">';
  $output .= t('The folder is empty.');
  $output .= '</span>';
  return $output;
}

/**
 * Outputs a gallery filter block.
 */
function theme_file_gallery_filter($form) {
  $output = '<div id="file_gallery-filter">';
  $output .= $form;
  $output .= '</div>';
  return $output;
}

/**
 * Outputs a gallery navigation.
 */
function theme_file_gallery_navigation($navigation) {
  $output = '<div class="file-gallery-nav">';
  $output .= implode(' > ', $navigation);
  $output .= '</div>';
  return $output;
}

/**
 * Outputs a gallery tabs.
 */
function theme_file_gallery_tabs($tabs) {
  $tabs = array_map(create_function('$a', 'return \'<li\'. (isset($a[\'active\']) ? \' class="active"\' : \' class="ff"\') .\'>\'. l($a[\'title\'], $a[\'link\'], array(\'query\' => $a[\'query\'], \'attributes\' => isset($a[\'active\']) ? array(\'class\' => \'active\') : array())) .\'</li>\';'), $tabs);
  $output = '<div id="tabs-wrapper" class="clear-block file-gallery"><ul class="tabs primary">';
  $output .= implode('', $tabs);
  $output .= '</ul></div>';
  return $output;
}

