// $Id$

/**
 * Attach handlers to evaluate if the file extension is supported by the web server.
 */
Drupal.file_restrictionExtensions = function(selector, extensions) {
  var translate = Drupal.settings.file_restriction;
  extensions = typeof(extensions) == 'undefined' ? translate.extensions_allowed : extensions;

  $(selector).each(function() {
    $(this).parent().after('<div class="file-restriction error" style="display: block;">'+ translate.description +'<br />'+ extensions +'</div>');
    $(this).parent().parent().find("div.file-restriction").hide();

    $(this).change(function() {
      var file = $(this).val();
      var ext = '';
      if(file.length > 0) {
        var dot = file.lastIndexOf(".");
	if(dot != -1) {
	  ext = file.substr(dot+1,file.length);
	}
      }
      var exts = extensions.split(" ");
      var found = 0;
      if(ext.length > 0) {
        for(i=0;i<exts.length;i++) {
	  if(exts[i] == ext.toLowerCase()) { found = 1; }
        }
      }
      if(found == 1) {
        $(this).parent().parent().find("div.file-restriction").hide();
      }
      else {
        $(this).parent().parent().find("div.file-restriction").show();
      }
    }); 
  });
};

