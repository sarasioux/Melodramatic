// $Id: cache_browser.js,v 1.1.2.2 2009/02/19 16:06:29 markuspetrux Exp $

Drupal.behaviors.cacheBrowser = function(context) {
  // Preload icons.
  var cache_browser_icon_plus = new Image();
  cache_browser_icon_plus.src = cache_browser.icon_plus;
  var cache_browser_icon_minus = new Image();
  cache_browser_icon_minus.src = cache_browser.icon_minus;

  // Attach click events.
  $('.cb-dump-hotspot-expand').not('.cache-browser-processed').addClass('cache-browser-processed').bind('click', function() {
    var id = $(this).attr('id').replace(/cb-dump-hotspot-expand-/, '');
    $('.cb-dump-block-'+ id).css('display', 'inline');
    $('.cb-dump-hotspot-'+ id).attr('src', cache_browser_icon_minus.src);
    this.blur();
    return false;
  });
  $('.cb-dump-hotspot-collapse').not('.cache-browser-processed').addClass('cache-browser-processed').bind('click', function() {
    var id = $(this).attr('id').replace(/cb-dump-hotspot-collapse-/, '');
    $('.cb-dump-block-'+ id).css('display', 'none');
    $('.cb-dump-hotspot-'+ id).attr('src', cache_browser_icon_plus.src);
    this.blur();
    return false;
  });
  $('pre.cb-dump img').not('.cache-browser-processed').addClass('cache-browser-processed').bind('click', function() {
    var id = $(this).attr('id').replace(/cb-dump-hotspot-/, '');
    if ($('#cb-dump-block-'+ id).css('display') != 'none') {
      $('#cb-dump-block-'+ id).hide('fast');
      $('#cb-dump-hotspot-'+ id).attr('src', cache_browser_icon_plus.src);
    }
    else {
      $('#cb-dump-block-'+ id).show('fast', function() { $(this).css('display', 'inline'); });
      $('#cb-dump-hotspot-'+ id).attr('src', cache_browser_icon_minus.src);
    }
  });
};
