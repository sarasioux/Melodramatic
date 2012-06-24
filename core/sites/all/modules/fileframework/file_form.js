// $Id$

Drupal.behaviors.file = function() {
  $('input[@type=file]').change(function() {
    if (this.value != '' && $('#edit-title').val() == '') {
      var title = this.value.match(/([^\/\\]+)$/)[1];
      $('#edit-title').val(title);
    }
  });
};

