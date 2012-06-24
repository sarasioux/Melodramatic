<?php
// $Id: views-view-table.tpl.php,v 1.6 2008/06/25 22:05:11 merlinofchaos Exp $
/**
 * @file views-view-table.tpl.php
 * Template to display a view as a table.
 *
 * - $title : The title of this group of rows.  May be empty.
 * - $header: An array of header labels keyed by field id.
 * - $fields: An array of CSS IDs to use for each field id.
 * - $class: A class or classes to apply to the table, based on settings.
 * - $rows: An array of row items. Each row is an array of content
 *   keyed by field ID.
 * @ingroup views_templates
 */
 
 // Theme this table for bulk updating
 $form = array(
  '#id' => 'melodev_vbo_folders_action_form',
  '#method' => 'post',
  'weights' => array(),
  'perms' => array(),
  'submit' => array(
    '#type' => 'submit',
    '#value' => 'Update Folders',
  ),
  '#submit' => array('melodev_vbo_folders_action_form_submit')
 );

 foreach($rows as $key => $row) {

    
    // Add weight field
    $form['weights'][$weightkey] = array(
      '#type' => 'textfield',
      '#value' => $row['field_weight_value'],
      '#size' => '5',
      '#attributes' => array(
        'class' => 'weight'
        ),
   );
   unset($rows[$key]['nid']);
   $rows[$key]['field_weight_value'] = drupal_render($form['weights'][$weightkey]);
 }
 unset($header['nid']);
 
 /*
 echo '<pre>';
 print_r($rows);
 print_r($header);
 exit();
 */
 
 $submit = drupal_render($form['submit']);
 print drupal_render($form);
 
?>
<table class="<?php print $class; ?>">
  <thead>
    <tr>
      <?php foreach ($header as $field => $label): ?>
        <th class="views-field views-field-<?php print $fields[$field]; ?>">
          <?php print $label; ?>
        </th>
      <?php endforeach; ?>
    </tr>
  </thead>
  <tbody>
    <?php foreach ($rows as $count => $row): ?>
      <tr class="<?php print ($count % 2 == 0) ? 'even' : 'odd';?> draggable">
        <?php foreach ($row as $field => $content): ?>
          <td class="views-field views-field-<?php print $fields[$field]; ?>">
            <?php print $content; ?>
          </td>
        <?php endforeach; ?>
      </tr>
    <?php endforeach; ?>
  </tbody>
</table>

<?php
print $submit;

print drupal_render($form);
?>