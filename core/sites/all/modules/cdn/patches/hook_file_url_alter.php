<?php
// $Id: hook_file_url_alter.php,v 1.2 2010/04/20 10:30:44 wimleers Exp $

/**
 * @file
 * Documentation for the hook_file_url_alter() hook.
 */


/**
 * Alter the URL to a file.
 *
 * This hook is called from file_create_url(), and  is called fairly frequently
 * (10+ times per page), depending on how many files there are in a given page.
 * If CSS and JS aggregation are disabled, this can become very frequently (50+
 * times per page) so performance is critical.
 *
 * This function should alter the path, if it wants to rewrite the file URL.
 * If it does so, no other hook_file_url_alter() implementation will be
 * allowed to further alter the path.
 *
 * @param $path
 *   A string containing the Drupal path (i.e. path relative to the Drupal
 *   root directory) of the file to generate the URL for.
 */
function hook_file_url_alter(&$path) {
  global $user;

  // User 1 will always see the local file in this example.
  if ($user->uid == 1) {
    return;
  }

  $cdn1 = 'http://cdn1.example.com';
  $cdn2 = 'http://cdn2.example.com';
  $cdn_extensions = array('css', 'js', 'gif', 'jpg', 'jpeg', 'png');

  // Most CDN's don't support private file transfers without a lot of hassle,
  // so don't support this in the common case.
  if (variable_get('file_downloads', FILE_DOWNLOADS_PUBLIC) == FILE_DOWNLOADS_PRIVATE) {
    return;
  }

  // Serve files without extension and files with one of the CDN extensions
  // from CDN 1, all others from CDN 2.
  $pathinfo = pathinfo($path);
  if (!array_key_exists('extension', $pathinfo) || in_array($pathinfo['extension'], $cdn_extensions)) {
    $path = $cdn1 . '/' . $path;
  }
  else {
    $path = $cdn2 . '/' . $path;
  }
}
