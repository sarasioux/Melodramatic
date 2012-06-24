<?php
function webmail_plus_address_autocomplete($string=''){
  if($string=="") return;
  

  
  global $_webmail_plus_config;

  // grab the incoming string, it might already contain something
  $pieces = preg_split("/(,)/", $string);
  
  $last = trim($pieces[sizeof($pieces)-1]);
  

    
  // take out the last record since it's incomplete and we're trying to match it
  unset($pieces[count($pieces) - 1]);
  $base_string = implode(', ', $pieces);
  if (drupal_strlen(trim($base_string))) {
    $base_string .= ', ';
  }




  
  
  if($_webmail_plus_config->address_book['contact_manager']) {


    $rs = module_invoke('contact_manager', 'matches', $last);
    
    
    
    if(sizeof($rs)>0) {
      foreach($rs as $index=>$value) {
        $results[$base_string.$value]=htmlentities($value);
      }
    }    
    
  }

  


  if($_webmail_plus_config->address_book['webmail_plus_site_users']) {
    

    
    
    $rs = module_invoke('webmail_plus_site_users', 'matches', $last);

    
    // add it to results
    if(sizeof($rs)>0) {
      foreach($rs as $index=>$value) {
        $results[$base_string.$value]=htmlentities($value);
      }
    }

  }
  



  
  print drupal_to_js($results);
  
  //wpd($results);

}
?>