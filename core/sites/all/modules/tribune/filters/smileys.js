Drupal.tribune.smileys = {};

Drupal.tribune.smileys.addHandlers = function() {
	$(".tribune-description-smileys img.smiley-class").click(function() {
			Drupal.tribune.insertIntoPalmipede($(this).attr("alt") + " ", "tribune-page");
		});
}

if (Drupal.jsEnabled) {
    $(document).ready(function() {
			Drupal.tribune.smileys.addHandlers();
		}
	);
}
