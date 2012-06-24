<?php
function webmail_plus_tag_list_autocomplete($string = '') {

  if($string=="") return false;

  require_once 'webmail_plus.tag_functions.inc';

  $items = preg_split("/(\s|,)/", $string);

  foreach($items as $key=>$value) {
    $existing[$value]=$value;
  }



  $search = trim($items[count($items) - 1]);
  unset($items[count($items) - 1]);
  $base_string = implode(' ' ,$items);



  if($search == '') {
      return drupal_json(array());
  }

  if(trim($base_string) != '') {
    $base_string .= ' ';
  }


  $tags = webmail_plus_get_user_tags();


  if(sizeof($tags)<=0) {
    return drupal_json(array());
  }


  $matches = array();



  foreach($tags as $index=>$tag) {


      if(strstr($tag, $search)) {
        if(!$existing[$tag]) {
          $matches[$base_string.$tag] = check_plain($base_string.$tag);
        }
        else {
          $matches[$base_string] = $base_string;
        }
      }

  }

  drupal_json($matches);
}

?>