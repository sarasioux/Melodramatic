<?php
// $Id: theme.inc,v 1.8 2010/10/11 21:47:28 wimleers Exp $

/**
 * @file
 * Theme functions.
 */

/**
 * Render the CDN integration page statistics.
 *
 * @param $file_count
 *   The number of files detected on this page.
 * @param $cdn_file_count
 *   The number of files on this page that are served from a CDN.
 * @param $synced_files_per_server_count
 *   The number of files synced to each server.
 * @param $total_time
 *   The total time it took to get all current CDN URLs.
 * @param $synced_files
 *   Array of synchronized files.
 * @param $unsynced_files
 *   Array of unsynchronized files.
 * @return
 *   The rendered HTML.
 */
function theme_cdn_page_stats($file_count, $cdn_file_count, $synced_files_per_server_count, $total_time, $synced_files, $unsynced_files) {
  $output = '';
  $items = array();
  $mode = variable_get(CDN_MODE_VARIABLE, CDN_MODE_BASIC);

  $output .= '<div id="cdn-integration-page-stats">';
  $items[] = 'Total number of files on this page: <strong>'. $file_count .'</strong>.';
  $percentage = ($file_count == 0) ? '100' : ($file_count > 0) ? number_format($cdn_file_count / $file_count * 100) : 0;
  $items[] = 'Number of files available on CDNs: <strong>'. $cdn_file_count .'</strong> ('. $percentage . '% coverage).';
  foreach (array_keys($synced_files_per_server_count) as $server) {
    $items[] = t('Number of files served from the server %server: !count', array('%server' => $server, '!count' => $synced_files_per_server_count[$server]));
  }
  $items[] = t('Total time it took to look up the CDN URLs for these files:
                !total-time ms, or !avg-time ms on average per file.',
                array(
                  '!total-time' => round($total_time * 1000, 3),
                  '!avg-time'   => ($file_count > 0) ? round($total_time * 1000 / $file_count, 3) : 0,
                )
  );


  // Nested list of unsynced files.
  if ($mode != CDN_MODE_BASIC) {
    if (count($unsynced_files)) {
      $unsynced_items = array();
      foreach ($unsynced_files as $file) {
        $unsynced_items[] = theme('cdn_page_stats_file_link', $file, file_create_url($file), FALSE, $file, NULL);
      }
      $items[] = t('The files that are not (yet?) synchronized to the CDN:') . theme('item_list', $unsynced_items);
    }
  }

  // Nested list of synced files.
  if (count($synced_files)) {
    $synced_items = array();
    foreach ($synced_files as $synced_file) {
      $file    = $synced_file['file'];
      $cdn_url = $synced_file['cdn_url'];
      $server  = $synced_file['server'];

      $synced_items[] = theme('cdn_page_stats_file_link', $file, $synced_file['absolute path'], TRUE, $cdn_url, $server);
    }
    $items[] = t('The files that are synchronized to the CDN:') . theme('item_list', $synced_items);
  }

  $output .= theme('item_list', $items, '<strong>'. t('CDN integration statistics for %drupal_path', array('%drupal_path' => $_GET['q'])) .'</strong>');
  $output .= '</div>';
  return $output;
}

/**
 * Render a file link in the CDN integration page statistics.
 *
 * @param $file
 *   A string containing the Drupal path (i.e. path relative to the Drupal
 *   root directory) of the file to generate the URL for.
 * @param $absolute_path
 *   The absolute path (on the filesystem) to the file.
 * @param $synced
 *   Whether this file has been synced to the CDN or not.
 * @param $cdn_url
 *   The CDN URL of the file, or the normal URL when the file is not on a CDN.
 * @param $server
 *   The server on which the file resides.
 * @return
 *   The rendered HTML.
 */
function theme_cdn_page_stats_file_link($file, $absolute_path, $synced, $cdn_url, $server) {
  $file_link  = l(t(($server == NULL) ? '!file' : '!file (server: !server)', array('!file' => $file, '!server' => $server)), $cdn_url, array('attributes' => array('title' => $absolute_path)));
  $touch_link = l(t('touch'), 'admin/cdn/touch/' . $file);

  $output = '';
  $output .= '<span class="file-link">' . $file_link;
  if ($synced) {
    $output .= '<span class="touch-link">';
    $output .=   '<span class="arrow">' . t('→') . '</span>';
    $output .=    $touch_link;
    $output .=   '<span class="touch-help">' . t('Touching this file will trigger a resync to the CDN.') . '</span>';
    $output .= '</span>';
  }
  $output .= '</span>';
  return $output;
}
