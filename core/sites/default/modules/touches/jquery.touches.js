/**
 * Attach behaviors to touch links.
 */
 
var touch_warning = false;

Drupal.behaviors.touchesAutoAttach = function () {
  var touchlinks = [];
  $('a.touchlink').each(function () {
    var uri = $(this).attr('href');
    $(this).attr('href', '#touch');
    
    // Create an object with this uri, so that we can attach events to it.
    if (!touchlinks[uri]) {
      touchlinks[uri] = new Drupal.TouchLink(this, uri);
    }
  });
}

/**
 * The Touch Link object
 */
Drupal.TouchLink = function (elem, uri) {
  var tl = this;
  this.elem = elem;
  this.uri = uri;
  this.id = $(elem).attr('id');
  var type = tl.uri.match(/[a-z]+/);
  
  $(elem).click(function () {
    // Ajax POST request for the voting data
    $.ajax({
      type: 'GET',
      url: tl.uri+'/ajax',
      success: function (data) {
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
        return true;
      },
      error: function (xmlhttp) {
        alert('An HTTP '+ xmlhttp.status +' error occured. Your '+type+' was not submitted!\n');
      }
    });
  });
}
