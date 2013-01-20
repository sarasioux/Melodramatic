<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="<?php print $language->language; ?>" lang="<?php print $language->language; ?>" dir="<?php print $language->dir; ?>">
<head>
  <title><?php print $head_title; ?></title>
  <?php print $head; ?>
  <?php print $styles; ?>
  <?php print $scripts; ?>
</head>
<body class="<?php print $body_classes; ?>">
	<!-- mr_coffe's test comment -->
  <div id="page"><div id="page-inner">
    <div id="active-users-container"></div>
    <div id="active-users-loading"></div>
    <div id="header"><div id="header-inner" class="clear-block">
      <div id="header-blocks" class="region region-header">
        <?php print $header; ?>
      </div>
      <h1 id="site-name">
        <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home">
        <?php print $site_name; ?>
        </a>
      </h1>
      <div id="navbar"><div id="navbar-inner" class="clear-block region region-navbar">
        <?php print $navbar; ?>
      </div></div>
    </div></div>
    <div id="main"><div id="main-inner" class="clear-block with-navbar">
      <div id="content"><div id="content-inner">
        <?php if ($content_top): ?>
          <div id="content-top" class="region region-content_top">
            <?php print $content_top; ?>
          </div>
        <?php endif; ?>
        <?php if ($breadcrumb || $title || $tabs || $help): ?>
          <div id="content-header">
            <?php if ($title): ?>
              <h1 class="title"><?php print $title; ?></h1>
            <?php endif; ?>
            <?php if ($tabs): ?>
              <div class="tabs"><?php print $tabs; ?></div>
            <?php endif; ?>
            <?php print $breadcrumb; ?>
            <?php print $help; ?>
          </div>
        <?php endif; ?>
        <?php if ($content_area_top): ?>
          <div id="content-area-top" class="region region-content_area_top">
            <?php print $content_area_top; ?>
          </div>
        <?php endif; ?>
        <div id="content-area">
          <?php if($messages): ?>
             <?php print $messages; ?>
          <?php endif; ?>

          <?php print $content; ?>
        </div>
        <?php if ($feed_icons): ?>
          <div class="feed-icons"><?php print $feed_icons; ?></div>
        <?php endif; ?>
        <?php if ($content_bottom): ?>
          <div id="content-bottom" class="region region-content_bottom">
            <?php print $content_bottom; ?>
          </div>
        <?php endif; ?>
      </div></div>
      <?php if ($left): ?>
        <div id="sidebar-left"><div id="sidebar-left-inner" class="region region-left">
          <?php print $left; ?>
        </div></div>
      <?php endif; ?>
      <?php if ($right): ?>
        <div id="sidebar-right"><div id="sidebar-right-inner" class="region region-right">
          <?php print $right; ?>
        </div></div>
      <?php endif; ?>
    </div></div>
    <?php if ($footer || $footer_message): ?>
      <div id="footer"><div id="footer-inner" class="region region-footer">
        <?php if ($footer_message): ?>
          <div id="footer-message"><?php print $footer_message; ?></div>
        <?php endif; ?>
        <?php print $footer; ?>
      </div></div>
    <?php endif; ?>
  </div></div>
  <?php if ($closure_region): ?>
    <div id="closure-blocks" class="region region-closure"><?php print $closure_region; ?></div>
  <?php endif; ?>
  <?php print $closure; ?>
</body>
</html>