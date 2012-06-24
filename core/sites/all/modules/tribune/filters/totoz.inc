<?php
// vim:filetype=php expandtab tabstop=2 softtabstop=2 shiftwidth=2 autoindent smartindent
// $Id: totoz.inc,v 1.9.2.3 2009/03/31 22:50:22 seeschloss Exp $

function tribune_totoz_info() {
  $info = array(
    'description' => t("Allows users to post [:totoz]s"),
    'weight' => 10,
  );

  return $info;
}

function tribune_totoz_filter_after(&$post, &$help) {
  // if using the filter_after hook, the filter will be executed after the
  // message is parsed, and will be able to add arbitrary HTML. The filter
  // will also be executed when the admin hits the "Parse visible messages
  // again" button and will not be able to post new messages.

  $image_url = tribune_variable_get("tribune_totoz_server", "http://sfw.totoz.eu") ."/$2.gif";
  $replacement_string  = "<a href='". $image_url ."' class='tribune-totoz'>$1";
  $replacement_string .= "<img src='". $image_url ."' alt='$2' />";
  $replacement_string .= "</a>";

  // since this filter executes after a message is parsed, it has to use
  // the 'parsed' index instead of the 'message' one.
  $post['parsed'] = preg_replace("/(\[:((\w| |_|-)+)\])/",
    $replacement_string,
    $post['parsed']
  );
}

function tribune_totoz_help() {
  // this function is called everytime the tribune is loaded, either the
  // full page or the block, it is a good place to add javascript or css

  drupal_add_js(array(
    "tribune" => array(
      "totoz" => array(
        'server' => tribune_variable_get("tribune_totoz_server", "http://sfw.totoz.eu"),
        ),
      ),
    ), "setting"
  );

  drupal_add_js(drupal_get_path('module', 'tribune') .'/filters/totoz.js');
  drupal_add_css(drupal_get_path('module', 'tribune') .'/filters/totoz.css');


  // hardware.fr's list isn't at the same address
  switch (tribune_variable_get("tribune_totoz_server", "http://sfw.totoz.eu")) {
    case "http://forum.hardware.fr/images/perso":
      $totoz_list .= "http://forum.hardware.fr/smilieperso.php";
      break;
    default:
      $totoz_list .= tribune_variable_get("tribune_totoz_server", "http://sfw.totoz.eu");
      break;
  }

  $description .= t('You can insert smileys in your post this way: [:smiley]. See all the available smileys or add a new one at <a href="@list">this address</a>',
    array('@list' => $totoz_list)
  );

  if (preg_match("/totoz.eu/", tribune_variable_get("tribune_totoz_server", "http://sfw.totoz.eu"))) {
    $description .= drupal_get_form("tribune_totoz_autocomplete_form");
  }

  return $description;
}

function tribune_totoz_autocomplete_form() {

  $form = array();
  $form['tribune-totoz-autocomplete'] = array(
    '#prefix' => "<div class='container-inline'>",
    '#suffix' => "</div>",
    '#type' => "textfield",
    '#title' => t("Search totoz"),
    '#autocomplete_path' => 'node/'. arg(1) .'/filter/totoz/search',
    '#size' => 30,
  );

  return $form;
}

function tribune_totoz_settings() {
  $servers_list = t('<ul><li>http://forum.hardware.fr/images/perso (!link1)</li><li>http://sfw.totoz.eu (!link2)</li><li>http://totoz.eu (!link3)</li></ul>',
    array(
      '!link1' => l(t('safe'), 'http://forum.hardware.fr/smilieperso.php'),
      '!link2' => l(t('safe'), 'http://sfw.totoz.eu'),
      '!link3' => l(t('not safe for all audiences'), 'http://totoz.eu'),
    )
  );
  $servers_help = t('The server from which to retrieve the smileys. Known servers are:!list', array('!list' => $servers_list));

  $form = array();
  $form['tribune_totoz_server'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Totoz server'),
    '#default_value'  => tribune_variable_get('tribune_totoz_server', 'http://sfw.totoz.eu'),
    '#description'    => $servers_help,
  );

  return tribune_settings_form($form);
}

function tribune_totoz_page() {
  if (arg(4)) {
    switch (arg(4)) {
      case 'search':
        if (arg(5)) {
          return tribune_search_totoz(arg(5));
        }
        break;
    }
  }
}

function tribune_search_totoz($tag) {
  $url = tribune_variable_get('tribune_totoz_server', 'http://sfw.totoz.eu') ."/totoz/search";
  $result = drupal_http_request($url ."?xml=drupal-tribune&q=". urlencode($tag));

  if ($result->code == 200) {
    $GLOBALS['element'] = NULL;
    $GLOBALS['totozid'] = NULL;
    $GLOBALS['totozes'] = array();
    $xml_parser = drupal_xml_parser_create($result->data);
    xml_set_element_handler($xml_parser, 'tribune_search_totoz_element_start', 'tribune_search_totoz_element_end');
    xml_set_character_data_handler($xml_parser, 'tribune_search_totoz_data');
    if (!xml_parse($xml_parser, $result->data, 1)) {
      exit();
    }
    xml_parser_free($xml_parser);

    $totoz_list = array();
    foreach ($GLOBALS['totozes'] as $id => $totoz) {
      $filename = check_plain($totoz['filename']);
      $image_url = tribune_variable_get("tribune_totoz_server", "http://sfw.totoz.eu") ."/". $filename;
      $img = "<img src='". $image_url .".gif' alt='' />";
      $totoz_list["[:". $filename ."]"] = $img ." [:". $filename ."]";
    }

    print drupal_to_js($totoz_list);
  } else {
    print drupal_to_js(array('' => t("Cannot retrieve list of totoz.")));
  }
  exit();
}

function tribune_search_totoz_element_start($parser, $name, $attributes) {
  $GLOBALS['element'] = $name;

  switch ($name) {
    case "TOTOZ":
      $GLOBALS['totozid'] = $attributes['ID'];
      $GLOBALS['totozes'][$attributes['ID']] = array('id' => $attributes['ID']);
      break;
  }
}

function tribune_search_totoz_data($parser, $data) {
  if ($GLOBALS['totozid']) {
    switch ($GLOBALS['element']) {
      case "FILENAME":
          $GLOBALS['totozes'][$GLOBALS['totozid']]['filename'] = substr($data, 0, strrpos($data, '.'));
        break;
    }
  }
}

function tribune_search_totoz_element_end() {
  $GLOBALS['element'] = NULL;
  $GLOBALS['totozid'] = NULL;
}
