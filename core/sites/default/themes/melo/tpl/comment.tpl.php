<?php
// $Id: comment.tpl.php,v 1.3 2008/09/14 12:09:37 johnalbin Exp $

/**
 * @file comment.tpl.php
 * Default theme implementation for comments.
 *
 * Available variables:
 * - $author: Comment author. Can be link or plain text.
 * - $classes: A set of CSS classes for the DIV wrapping the comment.
     Possible values are: comment, comment-new, comment-preview,
     comment-unpublished, comment-published, odd, even, first, last,
     comment-by-anon, comment-by-author, or comment-mine.
 * - $content: Body of the post.
 * - $date: Date and time of posting.
 * - $links: Various operational links.
 * - $new: New comment marker.
 * - $picture: Authors picture.
 * - $signature: Authors signature.
 * - $status: Comment status. Possible values are:
 *   comment-unpublished, comment-published or comment-preview.
 * - $submitted: By line with date and time.
 * - $title: Linked title.
 * - $unpublished: Is the comment unpublished?
 *
 * These two variables are provided for context.
 * - $comment: Full comment object.
 * - $node: Node object the comments are attached to.
 *
 * @see template_preprocess_comment()
 * @see theme_comment()
 */
?>
<div class="<?php print $classes; ?> <?php if($comment->new) { print ' comment-new'; }?>"><div class="comment-inner clear-block">

  <?php if ($unpublished): ?>
    <div class="unpublished"><?php print t('Unpublished'); ?></div>
  <?php endif; ?>

  <?php if ($picture) print $picture; ?>
  
  <div class="submitted">
    <span class="comment-author"><?php print $author; ?></span>
    <?php if ($links): ?>
      <span class="links">
        <?php print $links; ?>
      </span>
    <?php endif; ?>
    <span class="comment-date"><?php print $date; ?></span>
  </div>
  
  <div class="content">
    <?php 
    // Show nodes if it's not a folder gspot
    // TODO do this in the preprocess instead
    if($node->nid != arg(1)) {
      // If we're viewing outgoing, show to whom it was sent
      /*
      if(arg(2) == 'outgoing') {
        $out = '';
        // Process replies
        if($comment->pid > 0) {
          // Load parent comment
          if($pcomment = comment_load($comment->pid)) {
            $out .= l('To: '.$pcomment->name, 'user/'.$pcomment->uid);
          }
        } else {
          $out .= l('To: '.$node->name, 'user/'.$node->uid);
        }
        $out .= '<br />';
      } else {
        $out = '';
      }
      */
      print '<span class="gspot-re">'.$out.l('Re: ' .$node->title, 'node/'.$node->nid, array('fragment'=>'comment-'.$comment->cid, 'html'=>true)).'</span>';
    }
    ?>
    <?php print $content; ?>
    <?php if ($signature): ?>
    <div class="user-signature clear-block">
      <?php print $signature; ?>
    </div>
    <?php endif; ?>
  </div>

</div></div> <!-- /comment-inner, /comment -->
