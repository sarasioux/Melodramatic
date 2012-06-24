<?php
/**
 * @file api-defined.tpl.php
 * Theme implementation to show which object an api object is defined in.
 *
 * Available variables:
 * - $documentation: Documentation from the comment header of the constant.
 * - $branch: Object with information about the branch.
 * - $object: Object with information about the constant.
 * - $is_admin: True or false.
 * - $logged_in: True or false.
 *
 * Available variables in the $branch object:
 * - $branch->project: The machine name of the branch.
 * - $branch->title: A proper title for the branch.
 * - $branch->directories: The local included directories.
 * - $branch->excluded_directories: The local excluded directories.
 *
 * Available variables in the definition $object.
 * - $object->title: Display name.
 * - $object->summary: Short summary.
 * - $object->documentation: HTML formatted comments.
 * - $object->code: HTML formatted source code.
 */
?>
<p class="api-defined"><?php print t('!file, line @start_line', array('!file' => api_file_link($object), '@start_line' => $object->start_line)); ?></p>
