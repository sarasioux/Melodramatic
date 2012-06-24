<?php
function webmail_plus_solr_autocomplete($string=NULL) {
  header("Cache-Control: no-cache, must-revalidate");
  header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");


	$string=trim($string);
  if($string=="") exit();
  
  global $user;
  $results = db_query("SELECT keyword FROM {webmail_plus_solr_history} WHERE uid=%d AND keyword LIKE '%s' LIMIT 10", $user->uid, $keyword);  

  while($match = db_fetch_object($results)) {
  	$matches[$match]=$match;
  }
  print drupal_to_js($matches);

  exit();
}
?>