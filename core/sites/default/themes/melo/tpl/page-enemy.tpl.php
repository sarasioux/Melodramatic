<?php
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="<?php print $language->language; ?>" lang="<?php print $language->language; ?>" dir="<?php print $language->dir; ?>">

<head>
  <title><?php print $head_title; ?></title>
  <?php print $head; ?>
  <?php print $styles; ?>
  <?php print $scripts; ?>
</head>
<body class="<?php print $body_classes; ?> page-enemy">
  <div id="page"><div id="page-inner">
  <div id="content"><div id="content-inner">
  
    <?php if ($title): ?>
      <h1 class="title"><?php print $title; ?></h1>
    <?php endif; ?>
    
    <div id="content-area">
      <?php print $content; ?>
    </div>
  </div></div> <!-- /#content-inner, /#content -->
  </div></div> <!-- /#page-inner, /#page -->

  <?php if ($closure_region): ?>
    <div id="closure-blocks" class="region region-closure"><?php print $closure_region; ?></div>
  <?php endif; ?>

  <?php print $closure; ?>

</body>
</html>
