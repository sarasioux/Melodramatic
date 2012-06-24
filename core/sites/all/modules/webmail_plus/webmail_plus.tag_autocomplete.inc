<?php
function webmail_plus_tag_autocomplete($string = '') {

  if($string=="") return false;

  require_once 'webmail_plus.tag_functions.inc';
  $matches = array();



  $tags = webmail_plus_get_user_tags();


  if(sizeof($tags)<=0) {
    $matches=array();
  }
  else {



    foreach($tags as $index=>$tag) {
      if(strstr($tag, $string)) {
         $matches[$tag] = check_plain($tag);
      }

    }
  }

  drupal_json($matches);
}

?>