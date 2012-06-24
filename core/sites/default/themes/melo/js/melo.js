// $Id: melodev.js

/**
 * Melo custom js
 */

function meloShowFolderChanged() {
  var table = document.getElementById('vbo-table-view-Folders');
  var msg = $('.view-Folders #edit-object-wrapper .warning');
  $(Drupal.theme('tableDragChangedWarning')).insertAfter(table).hide().fadeIn('slow');
//  table.changed = true;
}



/*
Drupal.tableDrag.updateChanged = function() {
  this.changed = true;
}
Drupal.tableDrag.getChanged = function() {
  alert('1='+this.changed);
  return this.changed;
}
*/


/**
 * Attach behaviors to see-more-melo links.
 */
Drupal.behaviors.seeMeloAttach = function () {
  var seemelolinks = [];
  $('a.see-more-melo').each(function () {
    var tabid = $(this).attr('href');
    $(this).attr('href', '#seemore');
    
    // Create an object with this tabid, so that we can attach events to it.
    if (!seemelolinks[tabid]) {
      seemelolinks[tabid] = new Drupal.SeeMeloLink(this, tabid);
    }
  });
}

// html .js > body .not-front logged-in two-sidebars page-mymelo section-mymelo lightbox-processed admin-menu > div #page > div #page-inner > div #main > div #main-inner .clear-block with-navbar > div #content > div #content-inner > div #content-area > div .view view-MyMelo view-id-MyMelo view-display-id-page_1 view-dom-id-1 > div .view-content > div .mymelo-table-scroll > span .seemore > a

/**
 * The SeeMelo Link object
 */
Drupal.SeeMeloLink = function (elem, tabid) {
  var tl = this;
  this.elem = elem;
  this.tabid = tabid;
  this.table = $('table#'+tabid);
//  this.table.hide(); //
  var table = this.table;
  
  $(elem).click(function () {
    tl.table.fadeIn('slow');
    $(tl.elem).hide();
  });
}

function dump_props(dumparr) {
	var debugDestID = 'debug_place';
	if(!document.getElementById(debugDestID)) {
		alert('Error:  Could not find div tag with id '+debugDestID+'.');
		return false;
	}
	var txt = '';
	for(var i in dumparr) {
		if(i != 'domConfig') { if(i != 'channel') {
//			txt += i + ' = ' + dumparr[i] + "\n";
			txt += i + ' = ' + "\n";
		} }
	}
	document.getElementById(debugDestID).innerHTML = txt;
}

Drupal.behaviors.actuserAttach = function () {
  $('#active-users a').each(function () {
    var uri = $(this).attr('href');
    $(this).attr('href', '#actusers');
    var actlink = new Drupal.ActuserLink(this, uri);
  });
  $('#active-users-container').fadeOut('fast');
}

/**
 * The Touch Link object
 */
Drupal.ActuserLink = function (elem, uri) {
  var tl = this;
  this.elem = elem;
  this.uri = uri;
  
  $(elem).click(function () {
    // Add the loading graphic
//    $("#active-users-container").append('<div id="active-users-loading"><img src="/sites/melodramatic/themes/melo/images/ajax-loader.gif" /></div>');
//    $("#header").prepend('<div id="active-users-loading"></div>');
    $("#active-users-loading").html('<img src="/sites/melodramatic/themes/melo/images/ajax-loader-black.gif">');
    
    // Ajax GET request for active users
    $.ajax({
      type: 'GET',
      url: tl.uri+'/load',
      success: function (data) {
      
        var html_content = $("html_content", data).text();
        $("#active-users-container").html(html_content);
        $("#active-users-container").slideToggle('fast');
        $("#active-users-loading").fadeOut('fast');
//        var debugDestID = 'debug_place';
//        document.getElementById(debugDestID).innerHTML = html_content;
        
    /*
        // Extract the nid
        var nid = tl.id.match(/[0-9]+$/);
        var v_success = $("success",data).text();
        
        if(v_success == 'TRUE') {        
          var countelem = document.getElementById(type+'-count-'+nid);
          if(countelem) {
            var countNum = parseInt(countelem.innerHTML);
            countelem.innerHTML = countNum + 1;
          }
          // De-increment my touches
          if(type == 'bang') {
            var subNum = 10;
          } else {
            var subNum = 1;
          }
          var touchelem = document.getElementById('my-touches');
          if(touchelem) {
            var touchNum = parseInt(touchelem.innerHTML);
            if(touchNum - subNum >= 0) {
              touchelem.innerHTML = touchNum - subNum;
            }
          }
        } else {
          var v_errmsg = $("error_message",data).text();
          if(touch_warning == false) {
            alert(v_errmsg);
            touch_warning = true;
          }
        }
        */
        return true;
      },
      error: function (xmlhttp) {
//        alert('An HTTP '+ xmlhttp.status +' error occured. Your '+type+' was not submitted!\n');
      }
    });
  });
}
