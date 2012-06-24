<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="<?php print $language->language; ?>" lang="<?php print $language->language; ?>" dir="<?php print $language->dir; ?>">

<head>
  <title><?php print $head_title; ?></title>
  <?php print $head; ?>
  <?php print $styles; ?>
  <?php print $scripts; ?>
</head>
<body class="<?php print $body_classes; ?>">
  <div id="page"><div id="page-inner">
    <div id="active-users-container"></div>
    <div id="active-users-loading"></div>
    <div id="header"><div id="header-inner" class="clear-block">
    
      <div id="header-blocks" class="region region-header">
        <?php print $header; ?>
      </div> <!-- /#header-blocks -->

      <?php if ($site_name): ?>
          <h1 id="site-name">
            <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home">
            <?php print $site_name; ?>
            </a>
          </h1>
      <?php endif; ?>
      
      <?php if($navbar || $primary_links || $secondary_links) { ?>
        <div id="navbar"><div id="navbar-inner" class="clear-block region region-navbar">

          <?php if ($primary_links): ?>
            <div id="primary">
              <?php print theme('links', $primary_links); ?>
            </div> <!-- /#primary -->
          <?php endif; ?>

          <?php if ($secondary_links): ?>
            <div id="secondary">
              <?php print theme('links', $secondary_links); ?>
            </div> <!-- /#secondary -->
          <?php endif; ?>

          <?php print $navbar; ?>

        </div></div> <!-- /#navbar-inner, /#navbar -->
      <?php } ?>
      
    </div></div> <!-- /#header-inner, /#header -->

    <?php if($messages): ?>
       <?php print $messages; ?>
    <?php endif; ?>
    
    <div id="main"><div id="main-inner" class="clear-block<?php if ($search_box || $primary_links || $secondary_links || $navbar) { print ' with-navbar'; } ?>">

      <div id="content"><div id="content-inner">

        <?php if ($content_top): ?>
          <div id="content-top" class="region region-content_top">
            <?php print $content_top; ?>
          </div> <!-- /#content-top -->
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
          </div> <!-- /#content-header -->
        <?php endif; ?>
        
        <?php if ($content_area_top): ?>
          <div id="content-area-top" class="region region-content_area_top">
            <?php print $content_area_top; ?>
          </div> <!-- /#content-top -->
        <?php endif; ?>
        
        <div id="fp-left-col" class="fp-col">
          <?php print $fp_left_col; ?>
        </div>

        <div id="fp-right-col" class="fp-col">
          <?php print $fp_right_col; ?>
        </div>
        
        <div id="content-area">
          <?php print $content; ?>
        </div>

        <?php if ($feed_icons): ?>
          <div class="feed-icons"><?php print $feed_icons; ?></div>
        <?php endif; ?>

        <?php if ($content_bottom): ?>
          <div id="content-bottom" class="region region-content_bottom">
            <?php print $content_bottom; ?>
          </div> <!-- /#content-bottom -->
        <?php endif; ?>

      </div></div> <!-- /#content-inner, /#content -->
      
      <?php if ($left): ?>
        <div id="sidebar-left"><div id="sidebar-left-inner" class="region region-left">
          <?php print $left; ?>
        </div></div> <!-- /#sidebar-left-inner, /#sidebar-left -->
      <?php endif; ?>

      <?php if ($right): ?>
        <div id="sidebar-right"><div id="sidebar-right-inner" class="region region-right">
          <?php print $right; ?>
        </div></div> <!-- /#sidebar-right-inner, /#sidebar-right -->
      <?php endif; ?>            

    </div></div> <!-- /#main-inner, /#main -->
    
    <?php if ($outer_left): ?>
      <div id="outer-left"><div id="outer-left-inner" class="region region-left">
        <?php print $outer_left; ?>
      </div></div> <!-- /#sidebar-left-inner, /#sidebar-left -->
    <?php endif; ?>
    
    <?php if ($outer_right): ?>
      <div id="outer-right"><div id="outer-right-inner" class="region region-left">
        <?php print $outer_right; ?>
      </div></div> <!-- /#sidebar-right-inner, /#sidebar-right -->
    <?php endif; ?>


    <?php if ($footer || $footer_message): ?>
      <div id="footer"><div id="footer-inner" class="region region-footer">

        <?php if ($footer_message): ?>
          <div id="footer-message"><?php print $footer_message; ?></div>
        <?php endif; ?>

        <?php print $footer; ?>

      </div></div> <!-- /#footer-inner, /#footer -->
    <?php endif; ?>

  </div></div> <!-- /#page-inner, /#page -->

  <?php if ($closure_region): ?>
    <div id="closure-blocks" class="region region-closure"><?php print $closure_region; ?></div>
  <?php endif; ?>

  <?php print $closure; ?>

</body>
</html>
