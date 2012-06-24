// $Id$

/**
 * Setting initial variables.
 */
Drupal.file_browserVars = {
  'selected':null, // selected object
  'currentItem':'v', // might be vocabulary 'v', term 't' or file 'f'
  'pageX':0, // where the mouseX pointer is
  'pageY':0, // where the mouseY pointer is
  'ftDropDownTimeout':0, // timeout for hiding action div
  'filecount':1,
  'FTMSGCLEARTIMEOUT':5000
};

/**
 * Behaviours are bound to the Drupal namespace.
 */
Drupal.behaviors.file_browser = function(context) {  
  $('.file-dropdown').mouseout(function() {
    Drupal.file_browserVars.ftDropDownTimeout = setTimeout(function() { $('.file-dropdown').hide(); }, 1000); // hide action div
  }).mouseover(function() {
    clearTimeout(Drupal.file_browserVars.ftDropDownTimeout); // remove timeout for hiding the action div
  });

  $(document).mousemove(function(e) {
    // msie does not support pageX so handle that incase needed
    Drupal.file_browserVars.pageX = e.pageX ? e.pageX : e.clientX;
    Drupal.file_browserVars.pageY = e.pageY ? e.pageY : e.clientY;
    if ($.browser['msie'] == true && $.browser.version < 7.0) { Drupal.file_browserVars.pageY += 10; };
  });
};
 
/**
 * Set the height of the file system holder based off the window height.
 */
Drupal.file_browserInit = function(block) {
  var ratio = block == 'page' ? 0.75 : 0.3;
  $('div.file-system').height('' + (Drupal.file_browserGetWindowHeight() * ratio) + 'px');
}

/**
 * Set the vocabulary and term IDs on the folder folder.
 */
Drupal.file_browserUploadCheck = function(id) {
  var translate = Drupal.settings.file_browser;
  if (matches = id.match(/-([vt])([\d]+)/)) {

    // setting the term id that the file will be uploaded too
    $('#file-upload-tid').val(matches[2]);

    // retrieve vid from the file system for use in the create term form
    var filesysvid = $('#' + id).parents().find('.file-system').attr('id');
    var vid = filesysvid.match(/file-system-([\d]+)/);
    // if vid not in the file system folder get it from the file folder
    if (!vid) {
      while (!id.match(/-v([\d]+)/))
        id = $('#' + id).parent().attr('id');
      vid = id.match(/-v([\d]+)/)[1];
    }
    // if vocabulary then set term parent to 0 otherwise set to the currently selected term
    Drupal.file_browserVars.currentItem != 'v' ? $('#newterm-parent').val(matches[2]) : $('#newterm-parent').val('0');
    // setting the vocabulary id that the term will be created under
    $('#newterm-vid').val(vid);

    Drupal.file_browserToggleUpload();
    $('#block-file_browser-upload').show();
    // select and disable current group checkbox
    $('#file-browser-upload-form').find('.form-checkbox').each(function() { this.checked = false; }).removeAttr('disabled');
    if (id.match(/-g([\d]+)/)) {
      var gid = id.match(/-g([\d]+)/)[1];
      $('#edit-og-groups-' + gid).each(function() { this.checked = true; }).attr('disabled', 'disabled');
    }
  }
  $('#block-file_browser-preview').show();
  $('#file-preview').show().html(translate.no_file_selected)
  // show a new term block if the browser is within a page
  var block = id.match(/-b([^-]+)/)[1];
  if (block == 'page') 
    $('#block-file_browser-newterm').show();
}

/**
 * Toggle upload button.
 */
Drupal.file_browserToggleUpload = function() {
  var button = $('input#edit-upload-submit');
  Drupal.file_browserVars.currentItem == 't' ? button.removeAttr('disabled') : button.attr('disabled', 'disabled');
}

/**
 * Toggle newterm  button.
 */
Drupal.file_browserToggleNewterm = function(id) {
  var button = $('input#edit-newterm-submit');
  Drupal.file_browserVars.currentItem != 'f' && $('#' + id).hasClass('hierarchy') ? button.removeAttr('disabled') : button.attr('disabled', 'disabled');
}

/**
 * Gets window height.
 */
Drupal.file_browserGetWindowHeight = function() {
  if (typeof(window.innerHeight) == 'number')
    return window.innerHeight; // !MSIE
  if (document.documentElement && document.documentElement.clientHeight)
    return document.documentElement.clientHeight; // MSIE6
  if (document.body && document.body.clientHeight)
    return document.body.clientHeight; // MSIE4
  return 600; // reasonable default
}

/**
 * Dynamically add the new Term to the DOM in the correct location
 * @param block {String} browser id
 * @param tid {Integer} term which we add
 * @param ptid {Integer} parent term to which we are appending the new node under
 * @param vid {Integer} if parent term is 0 then insert node under the correct vocabulary
 * @param gid {Integer} a og_vocab group id or 0
 * @param node {String} html representation of the node
 * @param title {String} message to display on screen to the user
 * @param nodename {String} name of the new node being created
 */
Drupal.file_browserDisplayTerm = function(block, tid, ptid, vid, gid, node, msg, nodename) {
  var folder = 'file-folder-' + (ptid == 0 ? 'v' + vid : 't' + ptid) + '-g' + gid + '-b' + block;
  var term = 'file-folder-t' + tid + '-g' + gid + '-b' + block;
  var check = 0;
  var child = 0;
  $('#' + folder).children().each(function() {
    if (this.id.match(/file-folder-/)) {
      var name = $(this).find('.file-title').text();
      if (name.toLowerCase() > nodename.toLowerCase() && check == 0) {
        check = 1;
        $(node).insertBefore($(this));
      };
      child = 1;
    };
  });
  // the node might be the last one or there is no child nodes
  if (check == 0) {
    // if there were no child nodes
    if (child == 0)
      $('#' + folder + ' div.file-cells:first').after(node);
    else
      $('#' + folder + ' div.file-folder:last').after(node);
  };
  // remove empty class
  if ($('#' + folder).hasClass('empty')) {
    $('#' + folder).removeClass('empty')
  }
  // remove the term if the folder is not expanded
  if (!$('#' + folder).is('.expanded')) {
    $('#' + term).remove();
    // now expand the parent shelf
    $('#' + folder + ' div.file-cells:first').each(function() { Drupal.file_browserFolderClick(this, term) });
  }
  // select the newly created folder
  $('#' + term + ' div.file-cells:first').each(function() { Drupal.file_browserFolderClick(this) });
  Drupal.file_browserSetMsg(msg);
  $('#file-browser-newterm-form input#newterm-name').val(''); // resetting the value in the text field
}

/**
 * Dynamically add the new node to the DOM in the correct location
 * @param block {String} browser id
 * @param nid {Integer} term which we add
 * @param tid {Integer} parent term where the new node is being inserted into
 * @param node {String} html representation of the node
 * @param title {String} message to display on screen to the user
 */
Drupal.file_browserDisplayNode = function(block, nid, ptid, gid, node, msg, nodename) {
  var folder = 'file-folder-t' + ptid + '-g' + gid + '-b' + block;
  var file = 'file-node-t' + ptid + '-n' + nid + '-b' + block;
  var check = 0;
  $('#' + folder).children().each(function() {
    if (this.id.match(/file-node-/)) {
      var name = $(this).find('.title').text();
      if (name.toLowerCase() > nodename.toLowerCase() && check == 0) {
	check = 1;
        $(node).insertBefore($(this));
      };
    };
  });
  // the node might be the last one or there is no child nodes
  if (check == 0) {
      $('#' + folder).append(node);
  };
  // remove empty class
  if ($('#' + folder).hasClass('empty')) {
    $('#' + folder).removeClass('empty')
  }
  // hide the term if the folder is not expanded
  if (!$('#' + folder).is('.expanded')) {
    $('#' + file).remove();
    // now expand the parent shelf
    $('#' + folder + ' div.file-cells:first').each(function() { Drupal.file_browserFolderClick(this, file) });
  }
  // select the newly created folder
  $('#' + file + ' div.file-cells:first').each(function() { Drupal.file_browserFileClick(this) });
  Drupal.file_browserSetMsg(msg);
  // resetting the form for file upload back to its original state
  if ($('#file_browser').find('input:file').size() > 1) {
    $('#file_browser').find('input:file').each(function() {
      if (this.id != 'edit-upload') {
        $(this).parent().remove(); // remove it from the DOM we do not need it, parent is the <p>
      }
    });
  }
}

/**
 * Adds informational messages to the screen for the user to see
 * @param msg {String} message to be displayed on the screen to the user
 */
Drupal.file_browserSetMsg = function(msg) {
  if (!$('#file-upload-messages').size())
    $('<div id="file-upload-messages" class="messages status"></div>').insertBefore('.file-system');
  $('#file-upload-messages').append(msg);
  setTimeout(function() { $('#file-upload-messages').remove(); }, Drupal.file_browserVars.FTMSGCLEARTIMEOUT); // clear the messages after period of time
}

/**
 * Display any error messages on the screen since we do not do full page refreshes for actions
 * @param msg {String} error message to be displayed to the user
 */
Drupal.file_browserErrorMsg = function(msg) {
  if (!$('#file-upload-errors').size())
    $('<div id="file-upload-errors" class="messages error"></div>').insertBefore('.file-system');
  $('#file-upload-errors').append(msg);
  setTimeout(function() { $('#file-upload-errors').remove(); }, Drupal.file_browserVars.FTMSGCLEARTIMEOUT); // clear the messages after period of time
}

/**
 * Display any notice messages on the screen since we do not do full page refreshes for actions
 * @param msg {String} error message to be displayed to the user
 */
Drupal.file_browserNoticeMsg = function(msg) {
  if (!$('#file-upload-notices').size())
    $('<div id="file-upload-notices" class="messages notice"></div>').insertBefore('.file-system');
  $('#file-upload-notices').append(msg);
  setTimeout(function() { $('#file-upload-notices').remove(); }, Drupal.file_browserVars.FTMSGCLEARTIMEOUT); // clear the messages after period of time
}

/**
 * Highlights the selected row and adds necessary css classes to the pertinent rows
 * @param obj {Object} DOM object that was clicked on
 * @param id {String} Parent id of the selected DOM object
 * @param folder {Boolean} Is it a folder {true = folder, false = not a folder}
 * @param link {Boolean} Is it a link {true = link, false = not a link}
 */
Drupal.file_browserSelectRow = function(obj, id, folder, link) {
  if (id != Drupal.file_browserVars.selected)
    if (Drupal.file_browserVars.selected && ($('#' + Drupal.file_browserVars.selected)))
      $('#' + Drupal.file_browserVars.selected).children('.file-cells').removeClass('selected');
  Drupal.file_browserVars.selected = id;
  !link ? $(obj).addClass('selected') : $(obj.parentNode.parentNode).addClass('selected');

  if (folder)
    $(obj).parent().toggleClass('expanded');
}

/*
 * Expands or collapses the folder that was selected based on the current DOM information
 * @param obj {Object} The folder object that was clicked on
 * @param elm {String} The id of the element to click on the folder expand
 */
Drupal.file_browserFolderClick = function(obj, elm) {
  var parent = obj.parentNode;
  var id = parent.id;
  var block = id.match(/-b([^-]+)/)[1];
  if (id.match(/-t([\d]+)/)) {
    tid = 'term/' + id.match(/-t([\d]+)/)[1];
    Drupal.file_browserVars.currentItem = 't';
  }
  else {
    tid = 'voc/' + id.match(/-v([\d]+)/)[1];
    Drupal.file_browserVars.currentItem = 'v';
  }
  // hiding the block
  $('#block-file_browser-preview').hide();
  // highlighting the row that was selected
  Drupal.file_browserSelectRow(obj, id, true, false);
  $('#file-preview').hide();
  // upload and newterm are only on the main page
  if (block == 'page') {
    Drupal.file_browserUploadCheck(id);
    Drupal.file_browserToggleNewterm(id);
  }
  // if the shelf currently is closed then open the shelf
  // the class 'expanded' is already toggled in SelectRow.
  if ($(parent).hasClass('expanded')) {
    if (!$(parent).hasClass('empty')) {
      $('#' + id + '-spinner').show();
    }
    $.get($('#file-ajax-url').val() + '/' + tid + '/' + block, function(result) {
      $(obj).parent().addClass('expanded');
      // to stop duplicate info from double clicks
      if (parent.childNodes.length == 1) {
        $('#' + id).append(result);
        $('#' + id).find('span.file.with-menu').click(function(event) {
          $(this).toggleClass('active');
          $(this).find('ul').toggle();
        });
	$('a.file-metadata').cluetip({arrows: true});
      }
      $('#' + id + '-spinner').hide();
      if (typeof(elm) != 'undefined') {
        // click the element
        if (elm.match(/file-folder/))
	  $('#' + elm + ' div.file-cells:first').each(function() { Drupal.file_browserFolderClick(this) });
        else
	  $('#' + elm + ' div.file-cells:first').each(function() { Drupal.file_browserFileClick(this) });
      }
    });
  } else {
    // shelf is open hence remove all the children except the shelf name
    $(obj.parentNode).children().each(function() {
      if (this.nodeName.toLowerCase() == 'div' && this.id)
        parent.removeChild(this);
      else if (this.nodeName.toLowerCase() == 'script')
        parent.removeChild(this);
    });
  }
  return false;
}

/**
 * Handle operations when one of the file rows is clicked
 * @param obj {Object} File Object that holds a file which was clicked
 */
Drupal.file_browserFileClick = function(obj) {
  Drupal.file_browserVars.currentItem = 'f';
  var id = obj.parentNode.id;
  var nid = id.match(/-n([\d]+)/)[1]; // node id
  var block = id.match(/-b([^-]+)/)[1];
  var tid = id.match(/-t([\d]+)/)[1]; // term id
  // highlighting the row that was selected
  Drupal.file_browserSelectRow(obj, id, false, false);
  // hide the create term block since they have clicked on a file
  Drupal.file_browserToggleNewterm(id);
  Drupal.file_browserToggleUpload();
  // show preview
  $('#file-preview').hide();
  $('#file-preview-spinner').show();
  $.get($('#file-preview-url').val() + '/' + nid + '/' + tid, function(result) {
    $('#file-preview-spinner').hide();
    $('#file-preview').html(result).show();
    if (typeof collapseAutoAttach != 'undefined') {
      collapseAutoAttach();
    };
  });
  return false;
}

/**
 * Adds upload widget.
 */
Drupal.file_browserAddFileWidget = function() {
  // creating the new file widget that will be added to the form
  var widget = document.createElement("input");
  widget.setAttribute("type", "file");
  widget.setAttribute("name", "files[upload_" + Drupal.file_browserVars.filecount + "]");
  widget.setAttribute("id", "edit-upload-" + Drupal.file_browserVars.filecount);
  widget.setAttribute("class", "form-file");
  widget.setAttribute("size", "10");
  // creating widget wrapper
  var wrapper = document.createElement("div");
  wrapper.setAttribute('id', 'edit-upload-' + Drupal.file_browserVars.filecount + '-wrapper');
  wrapper.setAttribute('class', 'form-item');
  wrapper.appendChild(widget);
  var div = document.createElement("div");
  div.appendChild(wrapper);
  // adding the new widget to the container widget
  $('#file-upload').find('input:file').each(function() {
    if (this.id == 'edit-upload-' + (Drupal.file_browserVars.filecount - 1)) {
      $(div).insertAfter($(this).parent().parent());
    }
  });
  // integration with file restriction module
  if (typeof(Drupal.file_restrictionExtensions) == 'function') {
    Drupal.file_restrictionExtensions('#edit-upload-' + Drupal.file_browserVars.filecount);
  }
  Drupal.file_browserVars.filecount++;
}

/**
 * Removes upload widgets.
 */
Drupal.file_browserDelFileWidget = function() {
  for (var i=1;i<Drupal.file_browserVars.filecount;i++) {
    $('#edit-upload-' + i).parent().remove();
  }
  Drupal.file_browserVars.filecount = 1;
  $('#file-browser-upload-form input#edit-upload-0').val(''); // resetting the value in the text field
  $('.file-restriction').hide();
  $('#file-upload-spinner').hide();
}

/**
 * Sets parent term values on the file upload.
 */
Drupal.file_browserUpdateTerm = function(id, size, files) {
  $('#' + id).children().each(function() {
    if (this.id == '') {
      $(this).find('.file-size').text(size);
      $(this).find('.file-date').text(files);
    }
  })
}

/**
 * Handle operations when file upload is clicked
 * @param obj {Object} File Upload Object <a> tag
 */
Drupal.file_browserFileUploadClick = function(obj) {
  $('#file-upload-spinner').show();
}

