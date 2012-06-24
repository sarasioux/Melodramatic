Drupal.behaviors.apiExpandable = function (context) {
  $('div.api-expandable:not(.apiExpandableProcessed)', context).each(function () {
    $(this).addClass('apiExpandableProcessed')
      .children('div.prompt').find('a.show-content').click(function () {
        $(this).parents('div.api-expandable').eq(0)
          .children('.prompt').hide().end()
          .children('.content').show();
        return false;
      }).end().end()
      .children('div.content').find('a.hide-content').click(function () {
        $(this).parents('div.api-expandable').eq(0)
          .children('.prompt').show().end()
          .children('.content').hide();
        return false;
      });
  });
};

Drupal.behaviors.apiAutoComplete = function (context) {
  $('#api-search-form:not(.apiAutoCompleteProcessed)', context).addClass('apiAutoCompleteProcessed').each(function () {
    // On the first focus.
    $('#edit-search', this).attr('autocomplete', 'off').one('focus', function () {
      var $this = $(this);
      // Prefetch list of objects for this branch.
      $.getJSON(Drupal.settings.apiAutoCompletePath, function (data) {
        // Attach to autocomplete.
        $this.autocomplete(data, {
          sort: function (a, b) {
            return a.value.length - b.value.length;
          },
          matchContains: true,
          max: 200,
          scroll: true,
          scrollHeight: 360,
          width: 300
        }).result(function () {
          $this.get(0).form.submit();
        }).focus();
      });
    });
  });
};
