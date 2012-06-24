// $Id$

Drupal.behaviors.file_embed = function(context) {
  // Body field text links
  $('a.file-embed-select').click(function() {
    Drupal.file_embedPopup(this.href, 'Embed an existing file');
    return false;
  });
};

Drupal.file_embedPopup = function(url, title) {
  // Thickbox.js is pretty buggy; removing all its DIV elements before
  // attempting to show a thickbox overlay will ensure that the overlay
  // doesn't duplicate after the user has cancelled an earlier overlay.
  $("#TB_load").remove();
  $("#TB_overlay").remove();
  $("#TB_window").remove();
  tb_show(title || 'Add files', url, false);
}

Drupal.file_embedInsert = function(options) {
  var textarea_id = options['textarea'];
  var nid = options['nid'];

  var content = '[file:' + nid +
    (options['handler'] ? ' handler=' + options['handler'] : '') +
    (options['align'] ? ' align=' + options['align'] : '') +
    //(Number(options['padding']) > 0 ? ' padding=' + options['padding'] : '') +
    (options['link'] ? ' link=1' : '') +
    (options['caption'] ? ' caption=' + options['caption'] : '') +
    ']';

  // Special case for the TinyMCE WYSIWYG editor provided by tinymce.module
  if (typeof tinyMCE != 'undefined') {
    // TinyMCE can be tricky; we'll attempt inserting the snippet into the
    // WYSIWYG editor, but will also silently prepare to eat up any
    // curveball TinyMCE may throw at us. In case of trouble, we'll simply
    // fall back to attempting to use the standard textarea. One of the two
    // methods better work.
    try {
     if (tinyMCE.selectedInstance) {
        tinyMCE.execCommand('mceInsertContent', true, content);
        return;
     }
    } catch(e) {}
  }

  var textarea = $('textarea#' + textarea_id);
  var text = textarea.val();

  // here we fetch our text range object using jquery-fieldselection
  var range = $(textarea).getSelection();
  
  textarea.val(text.substring(0, range.start) + content + text.substring(range.end, text.length));
}

