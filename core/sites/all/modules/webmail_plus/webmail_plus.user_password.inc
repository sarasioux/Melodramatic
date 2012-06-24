<?php

function webmail_plus_user_password() {
   global $user;

   $content .= "Currently we're relying on AES to get interal Drupal passwords.";


   return $content;
   //return drupal_get_form('webmail_plus_user_password', $form);
}