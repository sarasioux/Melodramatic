<?php

/**
 * @file api-related-topics.tpl.php
 * Theme implementation to provide related topics
 *
 * Available variables:
 * - $topics: Array containing topic descriptions keyed on topic.
 */
?>
<dl class="api-related-topics">
<?php foreach ($topics as $topic => $description) { ?>
  <dt><?php print $topic ?></dt>
  <dd><?php print $description ?></dd>
<?php } ?>
</dl>
