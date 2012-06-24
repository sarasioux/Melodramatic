<?php

/**
 * @file
 * Views functions.
 */

/**
 * Implementation of hook_views_query_alter().
 *
 * Prevent views from loading the node containing the view.
 */
function viewfield_views_query_alter(&$view, &$query) {
  global $_viewfield_stack;

  if (!empty($_viewfield_stack) && in_array('node', $query->table_queue)) {
    $placeholders = db_placeholders($_viewfield_stack);
    $query->add_where(0, "node.nid NOT IN ($placeholders)", $_viewfield_stack);
  }
}
