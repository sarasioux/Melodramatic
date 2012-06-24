<?php

/**
 * @file api-expandable.tpl.php
 * Theme implementation to provide an expandable content wrapper.
 *
 * Available variables:
 * - $prompt: The prompt to view the content.
 * - $content: Content within.
 */
?>
<div class="api-expandable<?php print (is_null($class) ? '' : ' '. $class) ?>">
  <div class="prompt"><?php print $prompt ?></div>
  <div class="content"><?php print $content ?></div>
</div>
