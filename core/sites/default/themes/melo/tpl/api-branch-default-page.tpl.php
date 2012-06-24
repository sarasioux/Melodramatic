<?php if (!empty($branch)) { ?>
  <h3><?php print l('Topics', 'api/groups/' . $branch); ?></h3>
  <?php print api_page_listing($branch, 'group'); ?>
  <h3><?php print l('Files', 'api/files/' . $branch); ?></h3>
  <?php print api_page_listing($branch, 'file'); ?>
  <h3><?php print l('Globals', 'api/globals/' . $branch); ?></h3>
  <h3><?php print l('Constants', 'api/constants/' . $branch); ?></h3>
  <h3><?php print l('Functions', 'api/functions/' . $branch); ?></h3>
<?php } ?>