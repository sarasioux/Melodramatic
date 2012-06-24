<?php
// $Id$

/**
 * @file
 * Themes for file formats.
 */

//////////////////////////////////////////////////////////////////////////////
// Theme callbacks

/**
 * Theme for file handlers.
 */
function theme_file_admin_handler_settings($form) {
  drupal_add_tabledrag('file-table', 'order', 'sibling', 'handler-weight');

  $header[] = '';
  $header[] = t('Handler');
  $header[] = t('MIME types');
  if (function_exists('file_get_mime_converters')) {
    $header[] = t('Converters');
  }
  $header[] = t('Parent');
  $header[] = t('Module');
  $header[] = array('data' => t('Enabled'), 'class' => 'checkbox');
  $header[] = t('Weight');

  $rows = array();
  foreach ($form['handler'] as $handler => $element) {
    if (preg_match('/^file_/', $handler)) {
      $row = array('');
      $row[] = drupal_render($element['title']);
      $row[] = drupal_render($element['mime']);
      if (function_exists('file_get_mime_converters')) {
        $row[] = drupal_render($element['convert']);
      }
      $row[] = drupal_render($element['parent']);
      $row[] = drupal_render($element['module']);
      $element['weight']['#attributes']['class'] = "handler-weight";
      $row[] = array('data' => drupal_render($element['enabled']), 'class' => 'checkbox');
      $row[] = drupal_render($element['weight']);
      $rows[] = array('data' => $row, 'class' => 'draggable'. ($element['enabled']['#default_value'] ? ' menu-enabled' : ' menu-disabled'));
      unset($form['handler'][$handler]);
    }
  }

  if (!empty($rows)) {
    $form['handler']['table'] = array(
      '#value' => theme('table', $header, $rows, array('id' => 'file-table')),
    );
  }
  $output = drupal_render($form);
  return $output;
}

/**
 * Theme for showing a file with a links.
 */
function theme_file_render($file, $links_add = array(), $iframe = FALSE) {
  static $initialized = FALSE;
  $is_node = isset($file->nid) && !isset($file->derived);
  // If the same file is displayed several times, the js bindes incorrectly.
  // Have to ensure unique IDs.
  $id = '';
  for ($i = 0; $i < 8; $i++)
    $id .= mt_rand(0, 9);

  if (!$initialized) {
    $initialized = TRUE;
    drupal_add_css(drupal_get_path('module', 'file') .'/file.css');
    drupal_add_js(drupal_get_path('module', 'file') .'/file.js');

    // Add stylesheets and javascript. Thickbox must come before the module's stylesheet:
    drupal_add_css(drupal_get_path('module', 'file') .'/jquery/thickbox/thickbox.css', 'module');
    drupal_add_js(drupal_get_path('module', 'file') .'/jquery/thickbox/thickbox.js', 'module');

    // Cluetip for metadata.
    drupal_add_css(drupal_get_path('module', 'file') .'/jquery/cluetip/jquery.cluetip.css', 'module');
    drupal_add_js(drupal_get_path('module', 'file') .'/jquery/cluetip/jquery.dimensions-1.2.js', 'module');
    drupal_add_js(drupal_get_path('module', 'file') .'/jquery/cluetip/jquery.cluetip.js', 'module');
    drupal_add_js('$(document).ready(function() { $(\'a.file-tip\').cluetip(); });', 'inline');

    drupal_add_js('tb_pathToImage='. drupal_to_js('/'. drupal_get_path('module', 'file') .'/jquery/thickbox/loadingAnimation.gif') .';', 'inline');
  }

  // We don't need open link on the node view page.
  if ($is_node && !(arg(0) == 'node' && arg(1) == $file->nid))
    $links['open'] = array('title' => t('Open'), 'href' => 'node/'. $file->nid, 'attributes' => array('title' => t('Open file.')));

  // Find a first enabled handler for the file.
  if ($is_node && ($preview = file_first_preview($file)))
    $links['show'] = array('title' => t('Preview'), 'href' => 'node/'. $file->nid .'/embed/'. $preview['handler'], 'query' => array('width' => $preview['width'], 'height' => $preview['height']), 'attributes' => array('title' => t('Show file in the thickbox.'), 'class' => 'file-thickbox', 'id' => 'file-'. $file->nid .'-'. $id));
  if (FILE_VIEW_LINK) {
    $links['view'] = array('title' => t('View'), 'attributes' => array('title' => t('View file inline in the browser.')));
    $links['view']['href'] = $is_node ? 'file/'. $file->nid .'/view/'. $file->vid : bitcache_resolve_uri($file->uri, array('absolute' => TRUE));
    $links['view']['query'] = $is_node ? '' : array('vid' => $file->vid, 'disposition' => 'inline', 'op' => 'view');
  }
  $links['download'] = array('title' => t('Download'), 'attributes' => array('title' => t('Download file.')));
  $links['download']['href'] = $is_node ? 'file/'. $file->nid .'/download/'. $file->vid : bitcache_resolve_uri($file->uri, array('absolute' => TRUE));
  $links['download']['query'] = $is_node ? '' : array('vid' => $file->vid, 'disposition' => 'attachment', 'op' => 'download');
  if (FILE_INFO_LINK && $is_node)
    $links['info'] = array('title' => t('Info'), 'href' => '', 'attributes' => array('title' => t('File\'s information.'), 'class' => 'file-tip', 'rel' => base_path() .'node/'. $file->nid .'/info', 'onclick' => 'return false;'));
  if (FILE_PROPERTIES_LINK)
    $links['metadata'] = array('title' => t('Properties'), 'href' => '', 'attributes' => array('title' => t('File\'s properties.'), 'class' => 'file-tip', 'rel' => base_path() .'file_metadata/'. file_get_hash($file) .'/'. $file->nid, 'onclick' => 'return false;'));
  $links = theme('links', array_merge($links, $links_add), array('class' => 'file-links'));
  $output = '<span class="file with-menu"><span class="label"><span class="highlight"></span><span title="'. file_mime_description_for($file->type) .' - '. format_size($file->size) . (isset($file->desc) ? ' - '. $file->desc : '') .'" class="name">'. file_mime_icon_for($file->type) .' <span class="title">'. (isset($file->name) ? check_plain($file->name) : '') . (isset($file->name) && isset($file->desc) ? ' - ' : '') . (isset($file->desc) ? $file->desc : '') .'</span></span>'. $links .'</span></span>';
  if ($is_node)
    $output .= '<script type=\'text/javascript\'>'. (isset($iframe) ? 'parent.' : '') .'tb_init(\'#file-'. $file->nid .'-'. $id .'\');</script>';
  $output = preg_replace("/\n/", '', $output);
  return $output;
}

/**
 * Theme for showing previews and generated files.
 */
function theme_file_show($data) {
  $output = implode('<br />', array_filter($data));
  return !empty($output) ? '<div class="file-show">'. $output .'</div>'."\n" : '';
}

/**
 * Theme for previews.
 */
function theme_file_previews($data) {
  extract($data, EXTR_SKIP);

  $output = '';
  $prevs = array();
  foreach ($previews as $preview) {
    if (!isset($handler_first)) {
      $handler_first = $preview['handler'];
    }
    $prevs[] = '<span class="file-preview-toggle" id="'. $preview['handler'] .'-'. $id .'">'. l($preview['name'], !empty($nid) ? 'node/'. $nid .'/file_preview/'. $preview['handler'] : '') .'</span>';
  }
  if (!empty($prevs)) {
    $output .= '<div class="file-generated" id="file-preview-'. $id .'">';
    // If there is no string before the first previre, the first preview does not work.
    $output .= t('Preview') .': '. implode(' | ', $prevs);
    $output .= !isset($nid) ? '<div id="file-preview-notice-'. $id .'">'. t(' (works only with a Javascript enabled)') .'</div>' : '';
    $output .= '</div>';

    $output .= '<div id="file-preview-container-'. $id .'"></div>';
    if (!isset($nid)) {
      drupal_add_js('$(document).ready(function() { $(\'#file-preview-notice-'. $id .'\').hide(); });', 'inline');
    }
    drupal_add_js('$(document).ready(function() { $(\'#file-preview-'. $id .' .file-preview-toggle\').each(function() { $(this).mousedown(function() { $(\'#file-preview-'. $id .' .file-preview-toggle\').each(function() { $(this).removeClass("active"); }); $(this).addClass("active"); }); }); });', 'inline');
    drupal_add_js('$(document).ready(function() { $(\'#file-preview-'. $id .'\').append(\' | <a href="'. base_path() .'" id="file-preview-hide-'. $id .'" >'. t('hide') .'</a>\'); });', 'inline');
    drupal_add_js('$(document).ready(function() { $(\'#file-preview-hide-'. $id .'\').click(function() { $(\'#file-preview-'. $id .' .file-preview-toggle\').each(function() { $(this).removeClass("active"); }); $(\'#file-preview-container-'. $id .'\').html(\'\'); return false; }); });', 'inline');
    if (FILE_HANDLER_AUTOLOAD) {
      drupal_add_js('$(document).ready(function() { $(\'#'. $handler_first .'-'. $id .'\').trigger(\'mousedown\'); });', 'inline');
    }
  }
  return $output;
}

/**
 * Theme for generated formats.
 */
function theme_file_generated($data) {
  extract($data, EXTR_SKIP);

  $output = '';
  $entries = array();
  $handlers = file_get_mime_handlers();
  foreach ($formats as $gid => $format) {
    $description = preg_match('/^file_image_/', $format['handler']) ? $handlers[$format['handler']]['name'] : '';
    $entries[] = theme('file_render', (object)array('nid' => $nid, 'vid' => $vid, 'type' => $format['type'], 'size' => $format['size'], 'uri' => $format['uri'], 'derived' => TRUE, 'desc' => $description));
  }
  if (!empty($entries)) {
    $output .= '<div class="file-formats" id="file-formats-'. $id .'">';
    $output .= t('Other available formats') .': ';
    $output .= implode('&nbsp;&nbsp;', $entries);
    $output .= '</div>';
  }
  return $output;
}

/**
 * Theme for ajax metadata.
 */
function theme_file_metadata($data) {
  if (!empty($data)) {
    foreach ($data as $d) {
      $output[] = '<strong>'. $d[0] .':</strong><br />&nbsp;&nbsp;'. $d[1];
    }
  }
  print implode('<br />', $output);
  exit;
}

/**
 * Theme for ajax file info.
 */
function theme_file_info($node) {
  if (!is_object($node->file))
    exit;

  $data = array(
    array('name' => t('Author'), 'data' => array(theme('username', user_load($node->uid)))),
    array('name' => t('Title'), 'data' => array(check_plain($node->title))),
    array('name' => t('Created'), 'data' => array(format_date($node->created, 'small'))),
    array('name' => t('Modified'), 'data' => array(format_date($node->changed, 'small'))),
    array('name' => t('Type'), 'data' => array(file_mime_description_for($node->file->type))),
    array('name' => t('Size'), 'data' => array(format_size($node->file->size))),
  );
  if (FILE_VIEW_LINK)
    $data[] = array('name' => t('Views'), 'data' => array($node->file->views));
  $data[] = array('name' => t('Downloads'), 'data' => array($node->file->downloads));

  // Integration with og.module
  if (module_exists('og') && ($groups = og_get_node_groups($node)) && !empty($groups)) {
    foreach ($groups as $gid => $group) {
      $groups[$gid] = l($group, 'node/'. $gid);
    }
    $data[] = array('name' => t('Groups'), 'data' => $groups);
  }

  foreach ($data as $d) {
    $output[] = '<strong>'. $d['name'] .':</strong><br />&nbsp;&nbsp;'. implode('<br />&nbsp;&nbsp;', $d['data']);
  }
  print implode('<br />', $output);
  exit;
}

/**
 * Renders a block containing file's metadata.
 */
function theme_file_block_properties($metadata) {
  if (!empty($metadata)) {
    foreach ($metadata as $data) {
      $fields[] = '<tr><td><span class="metadata-name"><strong>'. $data[0] .':</strong></span></td><td><span class="metadata-value">'. $data[1] .'</span></td></tr>';
    }
    if (!empty($fields)) {
      return '<div id="file-block-properties"><table><tbody>'. implode('', $fields) .'</tbody></table></div>';
    }
  }
}

/**
 * Renders a block containing file's derivatives.
 */
function theme_file_block_formats($data) {
  extract($data, EXTR_SKIP);

  $entries = array();
  $handlers = file_get_mime_handlers();
  foreach ($formats as $id => $format) {
    $description = preg_match('/^file_image_/', $format['handler']) ? $handlers[$format['handler']]['name'] : NULL;
    $entries[] = theme('file_render', (object)array('nid' => $nid, 'vid' => $vid, 'name' => file_mime_description_for($format['type']), 'type' => $format['type'], 'size' => $format['size'], 'uri' => $format['uri'], 'derived' => TRUE, 'desc' => $description));
  }
  if (!empty($entries)) {
    $output = '<div id="file-block-formats">';
    $output .= implode('<br />', $entries);
    $output .= '</div>';
    return $output;
  }
}

/**
 * Renders a metadata size.
 */
function theme_file_metadata_size($data) {
  return format_size($data);
}

