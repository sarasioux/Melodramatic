<?php
// $Id: comment-wrapper.tpl.php,v 1.2 2008/09/14 12:12:38 johnalbin Exp $

/**
 * @file comment-wrapper.tpl.php
 * Default theme implementation to wrap comments.
 *
 * Available variables:
 * - $content: All comments for a given page. Also contains sorting controls
 *   and comment forms if the site is configured for it.
 *
 * The following variables are provided for contextual information.
 * - $node: Node object the comments are attached to.
 * The constants below the variables show the possible values and should be
 * used for comparison.
 * - $display_mode
 *   - COMMENT_MODE_FLAT_COLLAPSED
 *   - COMMENT_MODE_FLAT_EXPANDED
 *   - COMMENT_MODE_THREADED_COLLAPSED
 *   - COMMENT_MODE_THREADED_EXPANDED
 * - $display_order
 *   - COMMENT_ORDER_NEWEST_FIRST
 *   - COMMENT_ORDER_OLDEST_FIRST
 * - $comment_controls_state
 *   - COMMENT_CONTROLS_ABOVE
 *   - COMMENT_CONTROLS_BELOW
 *   - COMMENT_CONTROLS_ABOVE_BELOW
 *   - COMMENT_CONTROLS_HIDDEN
 *
 * @see template_preprocess_comment_wrapper()
 * @see theme_comment_wrapper()
 */
 
 // Folders get extra-special comment treatment
?>
<?php
 if($node->type != 'folder') {
?>
<div id="comment-input">
<?php
if (user_access('post comments')) {
  print comment_form_box(array('nid' => $node->nid), t('leave a gspot'));
}
?>
</div>
<?php if ($content): ?>
  <div id="comments">
    <?php if ($node->type != 'forum'): ?>
      <h2 id="comments-title"><?php print t('guestbook'); ?></h2>
    <?php endif; ?>
    <?php print $content; ?>
  </div>
<?php endif; ?>
<?
  }
?>