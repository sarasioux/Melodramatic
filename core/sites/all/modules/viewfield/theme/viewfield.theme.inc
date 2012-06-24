<?php

/**
 * @file
 * Theme functions.
 */

/**
 * Return a themed view avoiding viewfield recursion.
 */
function theme_viewfield_formatter_default($element) {
  global $_viewfield_stack;

  $node = $element['#node'];
  // For safety's sake, we can only display 2 levels of viewfields.
  if (!isset($_viewfield_stack[$node->nid]) && count($_viewfield_stack) <= 2) {
    list($view_name, $display) = explode('|', $element['#item']['vname'], 2);
    $view_args = _viewfield_get_view_args($element['#item']['token_enabled'], $element['#item']['vargs'], $element['#node']);

    // Prevent infinite recursion.
    if ($node->nid) {
      $_viewfield_stack[$node->nid] = $node->nid;
    }

    // Render the view like Views would do.
    // @see views_embed_view()
    $view = views_get_view($view_name);
    if ($view && $view->access($display)) {
      // Override the view's path to the current path. Otherwise, exposed views
      // filters would submit to the front page.
      $view->override_path = $_GET['q'];

      $output = $view->preview($display, $view_args);
    }

    // This node is "safe" again.
    if ($node->nid) {
      unset($_viewfield_stack[$node->nid]);
    }

    // Only return an actual view result to not break empty value behavior.
    if (isset($output)) {
      return $output;
    }
  }
}

/**
 * Perform argument replacement
 */
function _viewfield_get_view_args($token_enabled, $vargs, $node) {
  $args = array();
  // Prevent token_replace() from running this function a second time
  // before it completes the first time.
  static $tokens = TRUE;
  if ($tokens && !empty($vargs)) {
    $pos = 0;
    while ($pos < strlen($vargs)) {
      $found = FALSE;
      // If string starts with a quote, start after quote and get everything
      // before next quote.
      if (strpos($vargs, '"', $pos) === $pos) {
        if (($quote = strpos($vargs, '"', ++$pos)) !== FALSE) {
          // Skip pairs of quotes.
          while (!(($ql = strspn($vargs, '"', $quote)) & 1)) {
            $quote = strpos($vargs, '"', $quote + $ql);
          }
          $args[] = str_replace('""', '"', substr($vargs, $pos, $quote + $ql - $pos - 1));
          $pos = $quote + $ql + 1;
          $found = TRUE;
        }
      }
      elseif (($comma = strpos($vargs, ',', $pos)) !== FALSE) {
        // Otherwise, get everything before next comma.
        $args[] = substr($vargs, $pos, $comma - $pos);
        // Skip to after comma and repeat
        $pos = $comma + 1;
        $found = TRUE;
      }
      if (!$found) {
        $args[] = substr($vargs, $pos);
        $pos = strlen($vargs);
      }
    }
    if ($token_enabled) {
      $tokens = FALSE;
      // If the view field is being loaded as a "view field" of "view row",
      // instead of a simple "node field", the node object is not fully populated:
      // we need a full node to perform a correct replacement.
      $node_values = node_load($node->nid);
      $args = token_replace($args, 'node', $node_values);
      $tokens = TRUE;
    }
    // For backwards compatibility, we scan for %nid, etc.
    foreach ($args as $key => $value) {
      $args[$key] = strtr($value, array(
        '%nid' => $node->nid,
        '%author' => isset($node->uid) ? $node->uid : (isset($node_values->uid) ? $node_values->uid : NULL),
        '%viewer' => $GLOBALS['user']->uid,
      ));
    }
  }
  return $args;
}

function theme_viewfield_select($element) {
  if (!empty($element['#children'])) {
    $field = $element['#field_info'][$element['#field_name']];
    if ($field['multiple'] && $element['#delta'] == 0) {
      // This is needed only for multiple viewfields.
      drupal_add_css(drupal_get_path('module', 'viewfield') .'/theme/viewfield.css');
    }
    return '<div class="viewfield-select">'. $element['#children'] .'</div>';
  }
}
