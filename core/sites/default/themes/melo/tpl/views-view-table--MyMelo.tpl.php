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
 
define('MYMELO_UPDATE_LIMIT', 5);
?>
<?php 
  print $title;
  
  if(count($rows) > MYMELO_UPDATE_LIMIT) {
    $class .= ' melo-pre-hidden-table';
  }
?>
<table class="<?php print $class; ?>">
  <tbody>
    <?php 
      $i=0;
      $hiderows = array();
      foreach ($rows as $count => $row) {
        $txt = '<tr>'."\n";
        foreach ($row as $field => $content) {
          $txt .= '<td class="views-field views-field-'.$fields[$field].'">'.$content.'</td>'."\n";
        }
        $txt .= '</tr>';
        // Hide rows if there are more than the limit
        if($i >= MYMELO_UPDATE_LIMIT) {
          $hiderows[] = $txt;
        } else {
          print $txt;
        }
        $i++;
      }
  ?>
  </tbody>
</table>
<?php
  if(count($hiderows) > 0) {
    $printrows = implode("\n", $hiderows);
    $id = md5($printrows);?>
<table class="<?php print $class; ?> melo-hidden-table" style="display:none" id="<?php print $id; ?>">
  <tbody>
    <?php print $printrows; ?>
  </tbody>
</table>
<?php
    $txt = t('See (!posts) more posts', array('!posts'=>count($hiderows)));
    print '<p class="seemore"><a href="'.$id.'" class="see-more-melo">'.$txt.'</a></p>';
  }
?>
