<?php
function webmail_plus_site_users_autocomplete($string="") {
  if($string=="") return;
  

  $rs = webmail_plus_site_users_matches($string);
}
?>