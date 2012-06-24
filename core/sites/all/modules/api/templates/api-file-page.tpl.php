<?php

/**
 * @file api-file-page.tpl.php
 * Theme implementation for the summary page of a file.
 *
 * Available variables:
 * - $documentation: Documentation from the comment header of the file.
 * - $object: Object with information about the file.
 * - $is_admin: True or false.
 * - $logged_in: True or false.
 * - $user: user object.
 *
 * Available variables in the $object object:
 * - $object->title: Display name.
 * - $object->summary: Short summary.
 * - $object->documentation: HTML formatted comments.
 * - $object->code: HTML formatted source code.
 * - $object->objects: Documented objects HTML.
 * - $object->see: Related api objects.
 */
?>

<?php print $alternatives; ?>

<?php print $documentation ?>

<?php if (!empty($see)) { ?>
<h3><?php print t('See also') ?></h3>
<?php print $see ?>
<?php } ?>

<?php print $objects; ?>

<?php print theme('ctools_collapsible', t('View source'), $code, $collapsed = TRUE) ?>

<?php if (!empty($related_topics)) { ?>
  <h3><?php print t('Related topics') ?></h3>
  <?php print $related_topics ?>
<?php } ?>
