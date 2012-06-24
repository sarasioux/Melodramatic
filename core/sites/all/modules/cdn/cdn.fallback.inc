<?php
// $Id: cdn.fallback.inc,v 1.4 2010/11/18 21:50:17 wimleers Exp $

/**
 * @file
 * Fallback when hook_file_url_alter() is not available (i.e. when the core
 * patch is not installed or when not using Pressflow): use the Parallel
 * module's logic (with some adaptations to be able to use the CDN module's
 * logic (in particular: cdn_file_url_alter()) to alter file URLs via the
 * theme layer.
 * Some of the functions are also used when altering rendered nodes so that
 * their images are served from the CDN.
 */


/**
 * Implementation of hook_theme_registry_alter().
 *
 * Make CDN's page preprocess function run *after* everything else's, so that
 * a theme can't call drupal_get_js() and mess everything up.
 *
 * @param &$theme_registry
 *   The entire cache of theme registry information, post-processing.
 */
function cdn_theme_registry_alter(&$theme_registry) {
  if (isset($theme_registry['page'])) {
    // If CDN's preprocess function is there already, remove it.
    if ($key = array_search('cdn_preprocess_page', $theme_registry['page']['preprocess functions'])) {
      unset($theme_registry['page']['preprocess functions'][$key]);
    }
    // Now tack it on at the end so it runs after everything else.
    $theme_registry['page']['preprocess functions'][] = 'cdn_preprocess_page';
  }
}

/**
 * Implementation of template_preprocess_page().
 */
function cdn_preprocess_page(&$variables) {
  // CSS.
  cdn_html_alter_css_urls($variables['styles']);

  // JS.
  cdn_html_alter_js_urls($variables['scripts']);
  cdn_html_alter_js_urls($variables['closure']);

  // Images.
  $skip_keys = array('styles', 'scripts', 'zebra', 'id', 'directory', 'layout', 'head_title', 'base_path', 'front_page', 'head', 'body_classes');
  foreach ($variables as $key => $value) {
    if (!in_array($key, $skip_keys) && is_string($value) && !empty($value)) {
      cdn_html_alter_image_urls($variables[$key]);
    }
  }
}

/**
 * Alter CSS file URLs in a piece of HTML.
 */
function cdn_html_alter_css_urls(&$html) {
  $url_prefix_regex = _cdn_generate_url_prefix_regex();
  $pattern = "#href=\"(($url_prefix_regex)(.*?\.css)(\?.*)?)\"#";
  _cdn_html_alter_file_url($html, $pattern, 0, 3, 4, 'href="', '"');  
}

/**
 * Alter JS file URLs in a piece of HTML.
 */
function cdn_html_alter_js_urls(&$html) {
  $url_prefix_regex = _cdn_generate_url_prefix_regex();
  $pattern = "#src=\"(($url_prefix_regex)(.*?\.js)(\?.*)?)\"#";
  _cdn_html_alter_file_url($html, $pattern, 0, 3, 4, 'src="', '"');
}

/**
 * Alter image file URLs in a piece of HTML.
 */
function cdn_html_alter_image_urls(&$html) {
  $url_prefix_regex = _cdn_generate_url_prefix_regex();
  $pattern = "#((<img\s+|<img\s+[^>]*\s+)src\s*=\s*[\"|\'])($url_prefix_regex)([^\"|^\']*)([\"|\'])#i";
  _cdn_html_alter_file_url($html, $pattern, 0, 4, 5, 1, '');
}

/**
 * Generate the URL prefix regular expression, that supports all possible
 * types of file URLs: root-relative, protocol-relative and absolute URLs.
 */
function _cdn_generate_url_prefix_regex() {
  global $base_url;
  static $url_prefix_regex;

  if (!isset($url_prefix_regex)) {
    $url_prefix_regex = implode('|', array_map('preg_quote', array(
      base_path(),                                // Root-relative URL.
      '//' . $_SERVER['HTTP_HOST'] . base_path(), // Protocol-relative URL.
      $base_url . '/',                            // Absolute URL.
    )));
  }

  return $url_prefix_regex;
}

/**
 * Alter the file URLs in a piece of HTML given a regexp pattern and some
 * additional parameters.
 *
 * @param &$html
 *   The HTML in which file URLs will be altered.
 * @param $pattern
 *   A regular expression pattern to apply to the subject.
 * @param $search_index
 *   The index of the search string in the array of regexp matches.
 * @param $path_index
 *   The index of the file path in the array of regexp matches.
 * @param $querystring_index
 *   The index of (an optional) query string in the array of regexp matches.
 * @param $prefix
 *   $search_index will be replaced by $prefix, plus the altered file URL,
 *   plus the @suffix. If numeric, then it is assumed to be the index of the
 *   prefix in the array of regexp matches.
 * @param $suffix
 *   See $prefix.
 */
function _cdn_html_alter_file_url(&$html, $pattern, $search_index, $path_index, $querystring_index, $prefix, $suffix) {
  // Find a match against the given pattern.
  preg_match_all($pattern, $html, $matches);

  // Generate replacements to alter file URLs.
  $searches = array();
  $replacements = array();
  for ($i = 0; $i < count($matches[0]); $i++) {
    $search = $matches[$search_index][$i];
    $path = $matches[$path_index][$i];

    $prefix_string = (is_numeric($prefix)) ? $matches[$prefix][$i] : $prefix;
    $suffix_string = (is_numeric($suffix)) ? $matches[$suffix][$i] : $suffix;

    // Store the current path as the old path, then let cdn_file_url_alter()
    // do its magic by invoking all file_url_alter hooks. When the path hasn't
    // changed and is not already root-relative or protocol-relative, then
    // generate a file URL as Drupal core would: prepend the base path.
    $old_path = $path;
    drupal_alter('file_url', $path);
    if ($path == $old_path && drupal_substr($path, 0, 1) != '/' && drupal_substr($path, 0, 2) != '//') {
      $path = base_path() . $path;
    }

    $searches[]     = $search;
    $replacements[] = $prefix_string . $path . $matches[$querystring_index][$i] . $suffix_string;
  }

  // Apply the generated replacements ton the subject.
  $html = str_replace($searches, $replacements, $html);
}
