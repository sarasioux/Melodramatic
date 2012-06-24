Drupal.tribune.totoz = {};

Drupal.tribune.totoz.addHandlers = function(nodes) {
	nodes.find('.tribune-totoz').hover(
		function() {$(this).addClass('tribune-totoz-visible');},
		function() {$(this).removeClass('tribune-totoz-visible');}
	);
}

if (Drupal.jsEnabled) {
    $(document).ready(function() {
			$('#edit-tribune-totoz-autocomplete').keydown(function (e) {
				if (e.which == 13) {
					Drupal.tribune.insertIntoPalmipede($('#edit-tribune-totoz-autocomplete').val() + " ", 'tribune-page');
					$('#edit-tribune-totoz-autocomplete').val("");
					return false;
				}
			});


			for (tribune_id in Drupal.tribune.tribunes) {
				Drupal.settings.tribune.tribunes[tribune_id].after_reload.push(Drupal.tribune.totoz.addHandlers);
				Drupal.tribune.totoz.addHandlers(Drupal.tribune.tribunes[tribune_id].pinnipede);
			}
		}
	);
}
