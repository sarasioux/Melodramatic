<?php

/**
 * @file
 * Redirect deprecated API links.
 */

/**
 * Legacy function for file redirect.
 *
 * api/file/{directory}/{file.php}[/{branch}] -> api/{project}/{directory}--{file.php}/{branch}
 *
 * @param menu_tail
 *   One or more menu parts that follows the old style api/file/ path.
 */
function api_file_redirect() {
  $branches = api_get_branches();

  $args = func_get_args();
  $suffix = '';
  $file = new stdClass();
  $file->object_type = 'file';
  $file->branch_id = variable_get('api_default_branch', NULL);
  $default_project = $branches[variable_get('api_default_branch', NULL)]->project;

  $end = count($args) - 1;
  if (in_array($args[$end], array('source', 'documentation'))) {
    $suffix = '/' . $args[$end];
    $end -= 1;
  }
  foreach ($branches as $b) {
    if ($b->branch_name === $args[$end] && $b->project === $default_project) {
      $file->branch_id = $b->branch_id;
      $end -= 1;
      break;
    }
  }
  $file->object_name = implode('/', array_slice($args, 0, $end + 1));

  drupal_goto(api_url($file) . $suffix);
}
