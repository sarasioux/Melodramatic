<?php

/**
 * @file api-group-page.tpl.php
 * Theme implementation for summarizing code that in a group.
 *
 * Available variables:
 * - $classes: List of classes, if any.
 * - $constants: List of constants, if any.
 * - $globals: List of globals, if any.
 * - $functions: List of functions, if any.
 * - $files: List of functions, if any.
 * - $defined: HTML reference to file that defines this group.
 * - $see: Related api objects.
 */
?>

<?php print $alternatives; ?>

<?php print $documentation ?>

<?php if (!empty($see)) { ?>
<h3><?php print t('See also') ?></h3>
<?php print $see ?>
<?php } ?>

<?php print $objects; ?>

<?php print $defined; ?>
