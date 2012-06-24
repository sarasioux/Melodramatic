// $Id: tribune.ajax.js,v 1.20.2.8 2009/04/11 20:22:25 seeschloss Exp $

if (typeof(Drupal) == "undefined" || !Drupal.tribune) {    
   Drupal.tribune = {notification: false, notification_original_title: document.title};
   Drupal.tribune.tribunes = {};
}

Drupal.behaviors.tribune = function() {
	for (var tribune_id in Drupal.settings.tribune.tribunes) (function (tribune_id) {
		Drupal.tribune.tribunes[tribune_id] = {};
		Drupal.tribune.tribunes[tribune_id].highlightedElements = new Array();
		Drupal.tribune.tribunes[tribune_id].message_preview_element = $("<div id='tribune-message-preview-" + tribune_id + "' class='tribune-message-preview'></div>");
		Drupal.tribune.tribunes[tribune_id].palmipede = $("div#form-" + tribune_id + " #tribune-" + tribune_id + "-palmipede");
		Drupal.tribune.tribunes[tribune_id].pinnipede = $("ul#ul-" + tribune_id);

		Drupal.tribune.tribunes[tribune_id].postElements = {};

		if (Drupal.tribune.tribunes[tribune_id].palmipede.length == 1) {
			Drupal.tribune.ajaxifyForm(tribune_id);
		}

		Drupal.tribune.setOnHovers(Drupal.tribune.tribunes[tribune_id].pinnipede, tribune_id);

		Drupal.tribune.tribunes[tribune_id].pinnipede.find('li').each(function() {
			var id_array = $(this).attr("id").split("-");
			if (id_array.length == 2) {
				Drupal.tribune.tribunes[tribune_id].postElements[id_array[1]] = $(this);
			}
		});

		Drupal.tribune.tribunes[tribune_id].palmipede.keypress(function(e) {Drupal.tribune.keyboardShortcuts(e, tribune_id)});

		if (Drupal.settings.tribune.tribunes[tribune_id].reload_delay > 0) {
			Drupal.tribune.startTimer(tribune_id, Drupal.settings.tribune.tribunes[tribune_id].reload_delay);
		}	

		if (!Drupal.settings.tribune.tribunes[tribune_id].block && Drupal.tribune.tribunes[tribune_id].palmipede.length == 1) {
			Drupal.tribune.tribunes[tribune_id].palmipede.get(0).focus();
		}

		$(document).mousemove(Drupal.tribune.resetNotification);
		$(window).focus(Drupal.tribune.resetNotification);

		Drupal.settings.tribune.tribunes[tribune_id].after_reload = [function(nodes) {Drupal.tribune.setOnHovers(nodes, tribune_id)}, function(nodes) {Drupal.tribune.setNotification(nodes, tribune_id)}];
	})(tribune_id);

	tribune_id = null;
}

Drupal.tribune.resetNotification = function() {
	if (Drupal.tribune.notification) {
		document.title = Drupal.tribune.notification_original_title;
		Drupal.tribune.notification = false;
	}
}

Drupal.tribune.setNotification = function(nodes, tribune_id) {
	nodes.each(function() {
		if (!$(this).hasClass("tribune-self-post")) {
			if ($(this).hasClass("tribune-answer")) {
				document.title = "# " + Drupal.tribune.notification_original_title;
				Drupal.tribune.notification = true;
				return;
			} else if (!Drupal.tribune.notification) {
				document.title = "* " + document.title;
				Drupal.tribune.notification = true;
			}
		}
	});
}

Drupal.tribune.isElementVisible = function (jq) {
    element = jq.get(0);

    var scrollY = 0;
    if (window.pageYOffset) {
        scrollY = window.pageYOffset;
    } else if (document.body && document.body.scrollTop) {
        scrollY = document.body.scrollTop;
    } else if (document.documentElement && document.documentElement.scrollTop) {
        scrollY = document.documentElement.scrollTop;
    }

    var posY = 0;
    if (element.offsetParent) {
        for (var posY = 0 ; element.offsetParent ; element = element.offsetParent) {
            posY += element.offsetTop;
        }
    } else {
        posY = element.y;
    }

    return posY < scrollY;
}

/*
 * Submit comment with javascript.
 *
 */

Drupal.tribune.ajaxifyForm = function (tribune_id) {
	if (Drupal.settings.tribune.tribunes[tribune_id].path) {
		var options = {
			url: Drupal.settings.tribune.tribunes[tribune_id].path.post,
			resetForm: true,
			beforeSubmit: function(formData, jqForm, options) {Drupal.tribune.validate(formData, jqForm, options, tribune_id)},
			success: function(responseText) {Drupal.tribune.formSuccess(responseText, tribune_id)},
			error: function(responseText) {Drupal.tribune.formError(responseText, tribune_id)}
		};

		$("div#form-" + tribune_id + " form").ajaxForm(options);
	}
}

/**
  * Display response text and update the color
  * field. Remove top message if we are over 
  * the max count. 
  */

Drupal.tribune.formSuccess = function (responseText, tribune_id) {
	$("div#form-" + tribune_id + " .form-submit").removeAttr("disabled");
    Drupal.tribune.tribunes[tribune_id].palmipede.removeAttr("disabled").removeAttr("readonly");
    Drupal.tribune.tribunes[tribune_id].palmipede.removeClass("tribune-error");
	
    Drupal.tribune.loadPosts(tribune_id);

	Drupal.tribune.tribunes[tribune_id].palmipede.get(0).focus();
}

Drupal.tribune.formError = function (responseText, tribune_id) {
	$("div#form-" + tribune_id + " .form-submit").removeAttr("disabled");
    Drupal.tribune.tribunes[tribune_id].palmipede.removeAttr("disabled").removeAttr("readonly");
    Drupal.tribune.tribunes[tribune_id].palmipede.addClass("tribune-error");

	Drupal.tribune.tribunes[tribune_id].palmipede.get(0).focus();
}


/**
 * Creates a timer that triggers every delay seconds.
 */
Drupal.tribune.startTimer = function(tribune_id, delay) {
	Drupal.tribune.tribunes[tribune_id].reload_interval = setInterval("Drupal.tribune.loadPosts('" + tribune_id + "')", delay);	
}

Drupal.tribune.startReloading = function(tribune_id) {
	Drupal.tribune.tribunes[tribune_id].palmipede.attr("style", "background: white url(" + $('#' + tribune_id + '-reload-img').attr('src') + ") no-repeat right");
}

Drupal.tribune.endReloading = function(tribune_id) {
	Drupal.tribune.tribunes[tribune_id].palmipede.removeAttr("style");
}

/**
 * Reloads all posts from the server.
 */
Drupal.tribune.loadPosts = function(tribune_id) {
    Drupal.tribune.startReloading(tribune_id);

    Drupal.tribune.loadJSONPosts(tribune_id);
}

Drupal.tribune.loadJSONPosts = function(tribune_id) {
    res = $.ajax({url: Drupal.settings.tribune.tribunes[tribune_id].path.json_posts + "/" + Drupal.settings.tribune.tribunes[tribune_id].last_load_time + "/" + Math.random() + "/json-reload",
			     dataType: "json",
			     success: function(json) {Drupal.tribune.JSONSuccess(json, tribune_id)},
			     error: function(json) {Drupal.tribune.JSONError(json, tribune_id)},
			     complete: function(json) {Drupal.tribune.JSONComplete(json, tribune_id)}
	});
}

Drupal.tribune.JSONSuccess = function (json, tribune_id) {
    Drupal.tribune.tribunes[tribune_id].palmipede.removeClass("tribune-error");

	if (json.help.length > 0) {
		for (i = 0 ; i < json.help.length ; i++) {
			if (json.help[i] && json.help[i].filter) {
				$('div#form-' + tribune_id + ' .tribune-description-' + json.help[i].filter).html (json.help[i].string);
			}
		}
	}

	var new_posts = [];

	if (json.posts.length > 0) {
		Drupal.settings.tribune.tribunes[tribune_id].last_load_time = json.time;

		for (i = 0 ; i < json.posts.length ; i++) {
			if (json.posts[i]) {
				post = $(json.posts[i].text);

				existing_post = Drupal.tribune.tribunes[tribune_id].pinnipede.find('li#post-' + json.posts[i].id);
				if (json.posts[i].text) {
					if (json.posts[i].moderated) {
						post.addClass("tribune-moderated");
					}

					if (existing_post.length == 1) {
						existing_post.replaceWith(post);

						new_posts.push(post);
						Drupal.tribune.tribunes[tribune_id].postElements[json.posts[i].id] = post;
					} else {
						var first_id = 0;
						if (Drupal.settings.tribune.tribunes[tribune_id].message_order == "top_to_bottom") {
							var first = Drupal.tribune.tribunes[tribune_id].pinnipede.find('li:first');
						} else {
							var first = Drupal.tribune.tribunes[tribune_id].pinnipede.find('li:last');
						}

						if (first.length > 0) {
							var first_id_array = first.attr("id").split("-");
							if (first_id_array.length == 2) {
								first_id = first_id_array[1];
							}
						}

						if (json.posts[i].id >= first_id && Drupal.settings.tribune.tribunes[tribune_id].history_size) {
							while (Drupal.tribune.tribunes[tribune_id].pinnipede.find('li').length >= Drupal.settings.tribune.tribunes[tribune_id].history_size) {
								if (Drupal.settings.tribune.tribunes[tribune_id].message_order == "top_to_bottom") {
									var first = Drupal.tribune.tribunes[tribune_id].pinnipede.find('li:first');
								} else {
									var first = Drupal.tribune.tribunes[tribune_id].pinnipede.find('li:last');
								}
								first.each(function () {
									var id_array = $(this).attr("id").split("-");
									if (id_array.length == 2) {
										delete Drupal.tribune.tribunes[tribune_id].postElements[id_array[1]];
									}
									$(this).remove();
								});
							}

							if (Drupal.settings.tribune.tribunes[tribune_id].message_order == "top_to_bottom") {
								post.appendTo(Drupal.tribune.tribunes[tribune_id].pinnipede);
							} else {
								post.prependTo(Drupal.tribune.tribunes[tribune_id].pinnipede);
							}
							new_posts.push(post);
							Drupal.tribune.tribunes[tribune_id].postElements[json.posts[i].id] = post;
						}
					}
				} else if (json.posts[i].ref && existing_post) {
					existing_post.attr("ref", json.posts[i].ref);
				} else {
					if (existing_post.length == 1) {
						existing_post.remove();
						delete Drupal.tribune.tribunes[tribune_id].postElements.remove[json.posts[i].id];
					}
				}
			}
		}

		for (i in Drupal.settings.tribune.tribunes[tribune_id].after_reload) {
			for (j in new_posts) {
				Drupal.settings.tribune.tribunes[tribune_id].after_reload[i](new_posts[j]);
			}
		}
	}
}

Drupal.tribune.JSONError = function (json, tribune_id) {
	$("div#form-" + tribune_id + " .form-submit").removeAttr("disabled");
    Drupal.tribune.tribunes[tribune_id].palmipede.removeAttr("disabled").removeAttr("readonly");
    Drupal.tribune.tribunes[tribune_id].palmipede.addClass("tribune-error");
}

Drupal.tribune.JSONComplete = function (json, tribune_id) {
    Drupal.tribune.endReloading(tribune_id);
}

/**
 * Validate input before submitting.
 * Don't accept empty strings.
 */

Drupal.tribune.validate = function (formData, jqForm, options, tribune_id) {
    var form = jqForm[0];		
    if (!form.message.value) {
        return true;
    }

	$("div#form-" + tribune_id + " .form-submit").attr("disabled", "disabled");
    Drupal.tribune.tribunes[tribune_id].palmipede.attr("disabled", "disabled").attr("readonly", "readonly");

	if (Drupal.settings.tribune.tribunes[tribune_id].idle_delay) {
		clearInterval(Drupal.tribune.tribunes[tribune_id].idle_timeout);
		clearInterval(Drupal.tribune.tribunes[tribune_id].reload_interval);
		Drupal.tribune.tribunes[tribune_id].idle_timeout = setTimeout("Drupal.tribune.timed_out('" + tribune_id + "')", Drupal.settings.tribune.tribunes[tribune_id].idle_delay);
		Drupal.tribune.tribunes[tribune_id].reload_interval = setInterval("Drupal.tribune.loadPosts('" + tribune_id + "')", Drupal.settings.tribune.tribunes[tribune_id].reload_delay);	

		Drupal.tribune.tribunes[tribune_id].pinnipede.removeClass("tribune-idle");
		$("div#form-" + tribune_id).removeClass("tribune-idle");
	}
    return true;	
}

Drupal.tribune.timed_out = function (tribune_id) {
	clearInterval(Drupal.tribune.tribunes[tribune_id].reload_interval);
	Drupal.tribune.tribunes[tribune_id].pinnipede.addClass("tribune-idle");
	$("div#form-" + tribune_id).addClass("tribune-idle");
}

Drupal.tribune.setOnHovers = function(nodes, tribune_id) {
	nodes.find(".tribune-clock").hover(function() {
			$(this).append(Drupal.tribune.tribunes[tribune_id].message_preview_element);
			var attr = $(this).attr('ref');
			if (attr) {
				var refs = attr.split('|');
				var found = false;
				for (var i = 0 ; i < refs.length ; i++) {
					var node = Drupal.tribune.tribunes[tribune_id].postElements[refs[i]];
					if (node) {
						Drupal.tribune.showPreview(node, tribune_id);
						Drupal.tribune.highlightMainPost(node, tribune_id);
						found = true;
					}
				}
				if (!found) {
					Drupal.tribune.highlightElement($(this), tribune_id);
					Drupal.tribune.showAjaxPreview(refs[0], tribune_id);
				}
			}
		},
		function() {Drupal.tribune.unHighlightAllPosts(tribune_id)}
	);

	nodes.find(".tribune-first-clock").click(function() {
			Drupal.tribune.insertIntoPalmipede($(this).text() + " ", tribune_id);
		});
	
	nodes.find(".tribune-first-clock").hover(
			function() {Drupal.tribune.highlightMainPost($(this).parent(), tribune_id);},
			function() {Drupal.tribune.unHighlightAllPosts(tribune_id);}
		);

	nodes.find(".tribune-login").click(function() {
			Drupal.tribune.insertIntoPalmipede($(this).text() + "< ", tribune_id);
		});

	nodes.find(".tribune-info").click(function() {
			Drupal.tribune.insertIntoPalmipede($(this).text() + "< ", tribune_id);
		});

	nodes.find(".tribune-moderate-post a").click(function() {
			$.ajax({
				url: $(this).attr('href'),
			    success: function() {Drupal.tribune.loadPosts(tribune_id)}
			});

			return false;
		});
}

Drupal.tribune.insertIntoPalmipede = function(text, tribune_id) {
	var palmipede = Drupal.tribune.tribunes[tribune_id].palmipede;
	var message = palmipede.val();
	var range = Drupal.tribune.getSelectionRange(palmipede.get(0));
	palmipede.get(0).focus();
	palmipede.val(message.substring(0, range[0]) + text + message.substring(range[1], message.length));
    Drupal.tribune.setSelectionRange(palmipede.get(0), range[0] + text.length, range[0] + text.length);
}

Drupal.tribune.insertTagIntoPalmipede = function(tag, tribune_id) {
	Drupal.tribune.insertIntoPalmipedeAroundText("<" + tag + ">", "</" + tag + ">", tribune_id);
}

Drupal.tribune.insertIntoPalmipedeAroundText = function(before, after, tribune_id) {
	var palmipede = Drupal.tribune.tribunes[tribune_id].palmipede;
	var message = palmipede.val();
	var range = Drupal.tribune.getSelectionRange(palmipede.get(0));
	palmipede.get(0).focus();
	palmipede.val(message.substring(0, range[0]) + before + message.substring(range[0], range[1]) + after + message.substring(range[1], message.length));
    Drupal.tribune.setSelectionRange(palmipede.get(0), range[1] + before.length, range[1] + before.length);
}

Drupal.tribune.getSelectionRange = function(field) {
    if (field.setSelectionRange) {
        return [field.selectionStart, field.selectionEnd];
	} else if (field.createTextRange) {
        var range = document.selection.createRange();

		if (range.parentElement() == field) {
			var range2 = field.createTextRange();
			range2.collapse(true);
			range2.setEndPoint('EndToEnd', range);
			return [range2.text.length - range.text.length, range2.text.length];
		}
    }
    return [field.value.length, field.value.length];
}

Drupal.tribune.setSelectionRange = function(field, start, end) {
    if (field.setSelectionRange) {
        field.setSelectionRange(start, end);
	} else if (field.createTextRange) {
		var range = field.createTextRange();
		range.collapse(true);
		range.moveStart('character', start);
		range.moveEnd('character', end - start);
		range.select();
    }
}

Drupal.tribune.showPreview = function(node, tribune_id) {
	if (node.length && Drupal.tribune.isElementVisible(node)) {
		Drupal.tribune.tribunes[tribune_id].message_preview_element.html(Drupal.tribune.tribunes[tribune_id].message_preview_element.html() + node.html());
		Drupal.tribune.tribunes[tribune_id].message_preview_element.show();
	} else {
		Drupal.tribune.tribunes[tribune_id].message_preview_element.hide();
		Drupal.tribune.tribunes[tribune_id].message_preview_element.html("");
	}
}

Drupal.tribune.showAjaxPreview = function(node_id, tribune_id) {
	Drupal.tribune.tribunes[tribune_id].message_preview_element.show();
	Drupal.tribune.tribunes[tribune_id].message_preview_element.html("...");

    res = $.getJSON(Drupal.settings.tribune.tribunes[tribune_id].path.json_post + "/" + node_id, function(json) {

        if (json.text) {
			var node = Drupal.tribune.tribunes[tribune_id].message_preview_element.html(json.text);
			node.attr('ref', json.ref);
			Drupal.tribune.highlightMainPost(node, tribune_id);
		} else {
			Drupal.tribune.tribunes[tribune_id].message_preview_element.hide();
		}
    });
}

Drupal.tribune.highlightMainPost = function(node, tribune_id) {
	node.addClass('tribune-highlighted-main-post');

	Drupal.tribune.tribunes[tribune_id].highlightedElements.push(node);
	var ref = node.attr('ref');

	if (ref) {
		var referencesTo = ref.split('|');

		for (var k = 0 ; k < referencesTo.length ; k++) {
			var ref = referencesTo[k].split('#');
			
			if (ref.length == 2) {
				var refPostId  = ref[0];
				var refClockId = ref[1];

				Drupal.tribune.highlightClock(refPostId, refClockId, tribune_id);
			}
		}
	}
}

Drupal.tribune.highlightClock = function(postid, clockid, tribune_id) {
	var node = Drupal.tribune.tribunes[tribune_id].postElements[postid];
	if (node) {
		clock = node.find('#post-' + postid + '-clock-' + clockid);
		Drupal.tribune.highlightElement(clock, tribune_id);
	}
}

Drupal.tribune.highlightElement = function(node, tribune_id) {
	node.addClass('tribune-highlighted-answer-clock');
	Drupal.tribune.tribunes[tribune_id].highlightedElements.push(node);
}

Drupal.tribune.unHighlightAllPosts = function(tribune_id) {
	Drupal.tribune.tribunes[tribune_id].message_preview_element.remove();
    Drupal.tribune.tribunes[tribune_id].message_preview_element.html("");

	while (Drupal.tribune.tribunes[tribune_id].highlightedElements.length) {
		Drupal.tribune.tribunes[tribune_id].highlightedElements.pop().removeClass('tribune-highlighted-main-post').removeClass('tribune-highlighted-answer-clock');
	}
}

Drupal.tribune.keyboardShortcuts = function(e, tribune_id) {
	var code = String.fromCharCode(e.which ? e.which : e.keyCode);

	for (id in Drupal.settings.tribune.tribunes[tribune_id].shortcuts) {
		var shortcut = Drupal.settings.tribune.tribunes[tribune_id].shortcuts[id];

		if (code == shortcut.key) {
			if ((shortcut.modifier == 'ctrl' && e.ctrlKey) || (shortcut.modifier == 'alt' && e.altKey) || (shortcut.modifier == 'alt' && e.originalEvent && e.originalEvent.altKey)) {
				if (shortcut.text) {
					Drupal.tribune.insertIntoPalmipede(shortcut.text, tribune_id);
				} else if (shortcut.before && shortcut.after) {
					Drupal.tribune.insertIntoPalmipedeAroundText(shortcut.before, shortcut.after, tribune_id);
				}

				if (e.preventDefault) {
					e.preventDefault();
				}

				if (e.stopPropagation) {
					e.stopPropagation();
				}

				return false;
			}
		}
	}
}
