// $Id: usercolor.js

Drupal.behaviors.color = function (context) {
  // This behavior attaches by ID, so is only valid once on a page.
  if ($('#color_scheme_form .color-form.color-processed').size()) {
    return;
  }
  var form = $('#color_scheme_form .color-form', context);
  var inputs = [];
  var input_refs = [];
  var hooks = [];
  var locks = [];
  var focused = null;

  // Add Farbtastic
//  $(form).prepend('<div id="placeholder"></div>').addClass('color-processed');
  $('.group-scheme-colors').prepend('<div id="placeholder"></div>').addClass('color-processed');
  var farb = $.farbtastic('#placeholder');

  // Decode reference colors to HSL
  var reference = Drupal.settings.color.reference;
  for (i in reference) {
    reference[i] = farb.RGBToHSL(farb.unpack(reference[i]));
  }
  
  // Get our input references
  input_refs = Drupal.settings.color.input_refs;

  // Build preview
  $('#preview:not(.color-processed)')
    .addClass('color-processed');
  // Fix preview background in IE6
  if (navigator.appVersion.match(/MSIE [0-6]\./)) {
    var e = $('#preview #img')[0];
    var image = e.currentStyle.backgroundImage;
    e.style.backgroundImage = 'none';
    e.style.filter = "progid:DXImageTransform.Microsoft.AlphaImageLoader(enabled=true, sizingMethod=crop, src='" + image.substring(5, image.length - 2) + "')";
  }
  
  /**
   * Render the preview.
   */
  function preview() {
    // Solid background
    $('#preview', form).css('backgroundColor', inputs[input_refs['scheme_bg_color']].value);
    $('#preview', form).css('color', inputs[input_refs['scheme_body_text']].value);
    $('#preview a', form).css('color', inputs[input_refs['scheme_body_link']].value);
    
    // Header
    $('#preview #img', form).css('backgroundColor', inputs[input_refs['scheme_header_color']].value);

    // Page preview
    $('#preview-body', form).css('backgroundColor', inputs[input_refs['scheme_body_color']].value);
    $('#preview-body', form).css('color', inputs[input_refs['scheme_page_text']].value);
    $('#preview-body a', form).css('color', inputs[input_refs['scheme_page_link']].value);

    // Text preview
    $('#text', form).css('backgroundColor', inputs[input_refs['scheme_block_body']].value);
    $('#text', form).css('color', inputs[input_refs['scheme_block_text']].value);
    $('#text a', form).css('color', inputs[input_refs['scheme_link_color']].value);
    $('#text h2', form).css('backgroundColor', inputs[input_refs['scheme_block_header']].value);
    $('#text p', form).css('borderColor', inputs[input_refs['scheme_block_header']].value);
    $('#text', form).css('border', 'solid '+inputs[input_refs['scheme_block_header']].value+' 1px');
    $('#text h2', form).css('color', inputs[input_refs['scheme_block_header_text']].value);

  }

  /**
   * Shift a given color, using a reference pair (ref in HSL).
   *
   * This algorithm ensures relative ordering on the saturation and luminance
   * axes is preserved, and performs a simple hue shift.
   *
   * It is also symmetrical. If: shift_color(c, a, b) == d,
   *                        then shift_color(d, b, a) == c.
   */
  function shift_color(given, ref1, ref2) {
    // Convert to HSL
    given = farb.RGBToHSL(farb.unpack(given));

    // Hue: apply delta
    given[0] += ref2[0] - ref1[0];

    // Saturation: interpolate
    if (ref1[1] == 0 || ref2[1] == 0) {
      given[1] = ref2[1];
    }
    else {
      var d = ref1[1] / ref2[1];
      if (d > 1) {
        given[1] /= d;
      }
      else {
        given[1] = 1 - (1 - given[1]) * d;
      }
    }

    // Luminance: interpolate
    if (ref1[2] == 0 || ref2[2] == 0) {
      given[2] = ref2[2];
    }
    else {
      var d = ref1[2] / ref2[2];
      if (d > 1) {
        given[2] /= d;
      }
      else {
        given[2] = 1 - (1 - given[2]) * d;
      }
    }

    return farb.pack(farb.HSLToRGB(given));
  }

  /**
   * Callback for Farbtastic when a new color is chosen.
   */
  function callback(input, color, propagate, colorscheme) {
    // Set background/foreground color
    $(input).css({
      backgroundColor: color,
      'color': farb.RGBToHSL(farb.unpack(color))[2] > 0.5 ? '#000' : '#fff'
    });

    // Change input value
    if (input.value && input.value != color) {
      input.value = color;

      // Update locked values
      if (propagate) {
        var i = input.i;
        for (j = i + 1; ; ++j) {
          if (!locks[j - 1] || $(locks[j - 1]).is('.unlocked')) break;
          var matched = shift_color(color, reference[input.key], reference[inputs[j].key]);
          callback(inputs[j], matched, false);
        }
        for (j = i - 1; ; --j) {
          if (!locks[j] || $(locks[j]).is('.unlocked')) break;
          var matched = shift_color(color, reference[input.key], reference[inputs[j].key]);
          callback(inputs[j], matched, false);
        }

        // Update preview
        preview();
      }

      // Reset colorscheme selector
      if (!colorscheme) {
        resetScheme();
      }
    }

  }

  /**
   * Reset the color scheme selector.
   */
  function resetScheme() {
    $('#edit-scheme', form).each(function () {
      this.selectedIndex = this.options.length - 1;
    });
  }

  // Focus the Farbtastic on a particular field.
  function focus() {
    var input = this;
    // Remove old bindings
    focused && $(focused).unbind('keyup', farb.updateValue)
        .unbind('keyup', preview).unbind('keyup', resetScheme)
        .parent().removeClass('item-selected');

    // Add new bindings
    focused = this;
    farb.linkTo(function (color) { callback(input, color, true, false); });
    farb.setColor(this.value);
    $(focused).keyup(farb.updateValue).keyup(preview).keyup(resetScheme)
      .parent().addClass('item-selected');
  }

  // Initialize color fields
  $('input.text-color', form)
  .each(function () {
    // Extract palette field name
    this.key = this.id.substring(13);

    // Link to color picker temporarily to initialize.
    farb.linkTo(function () {}).setColor('#000').linkTo(this);

    // Add lock
    var i = inputs.length;
    if (inputs.length) {
      var lock = $('<div class="lock"></div>').toggle(
        function () {
          $(this).addClass('unlocked');
          $(hooks[i - 1]).attr('class',
            locks[i - 2] && $(locks[i - 2]).is(':not(.unlocked)') ? 'hook up' : 'hook'
          );
          $(hooks[i]).attr('class',
            locks[i] && $(locks[i]).is(':not(.unlocked)') ? 'hook down' : 'hook'
          );
        },
        function () {
          $(this).removeClass('unlocked');
          $(hooks[i - 1]).attr('class',
            locks[i - 2] && $(locks[i - 2]).is(':not(.unlocked)') ? 'hook both' : 'hook down'
          );
          $(hooks[i]).attr('class',
            locks[i] && $(locks[i]).is(':not(.unlocked)') ? 'hook both' : 'hook up'
          );
        }
      );
      $(this).after(lock);
      locks.push(lock);
    };

    // Add hook
    var hook = $('<div class="hook"></div>');
    $(this).after(hook);
    hooks.push(hook);

    $(this).parent().find('.lock').click();
    this.i = i;
    inputs.push(this);
  })
  .focus(focus);

  $('#palette label', form);

  // Focus first color
  focus.call(inputs[0]);

  // Render preview
  preview();
};
