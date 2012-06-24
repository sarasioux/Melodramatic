<?php

/**
 * @file api-constant-page.tpl.php
 * Theme implementation to display a constant overview.
 *
 * Available variables:
 * - $documentation: Documentation from the comment header of the constant.
 * - $branch: Object with information about the branch.
 * - $object: Object with information about the constant.
 * - $defined: HTML reference to file that defines this class.
 * - $is_admin: True or false.
 * - $logged_in: True or false.
 *
 * Available variables in the $branch object:
 * - $branch->project: The machine name of the branch.
 * - $branch->title: A proper title for the branch.
 * - $branch->directories: The local included directories.
 * - $branch->excluded_directories: The local excluded directories.
 *
 * Available variables in the $object object.
 * - $object->title: Display name.
 * - $object->object_type: For this template it will be 'constant'.
 * - $object->branch_id: An identifier for the branch.
 * - $object->file_name: The path to the file in the source.
 * - $object->summary: A one-line summary of the object.
 * - $object->code: Escaped source code.
 * - $object->see: HTML index of additional references.
 *
 * @see api_preprocess_api_object_page().
 */
?>
<?php print $alternatives; ?>

<?php print $documentation ?>

<?php if (!empty($see)) { ?>
<h3><?php print t('See also') ?></h3>
<?php print $see ?>
<?php } ?>

<h3><?php print t('File'); ?></h3>
<?php print $defined; ?>

<h3><?php print t('Code'); ?></h3>
<?php print $code; ?>

<?php if (!empty($related_topics)) { ?>
<h3><?php print t('Related topics') ?></h3>
<?php print $related_topics ?>
<?php } ?>
