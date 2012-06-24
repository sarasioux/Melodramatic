<?php
// vim:filetype=php expandtab tabstop=2 softtabstop=2 shiftwidth=2 autoindent smartindent
// $Id: tribune.info.inc,v 1.1 2008/11/28 13:00:02 seeschloss Exp $

/**
  * Show cookie information for Coincoin users
  */
function tribune_show_info($node) {
  global $user;

  $cookie  = "";
  $content = "";

  if ($user->uid) {

    if (isset($_SERVER['HTTP_COOKIE'])) {
      //$cookie = explode(';',  $_SERVER['HTTP_COOKIE']);
      //$cookie = $cookie[0];

      $cookie = $_SERVER['HTTP_COOKIE'];
    }

    $content = t("<p><strong>Your cookie</strong>: @cookie</p>", array('@cookie' => $cookie));
  }

  $site_root = url(NULL, array('absolute' => TRUE));

  $url_backend = url('node/'. $node->nid .'/tribune.xml', array('absolute' => TRUE));
  $url_post  = url('node/'. $node->nid .'/post', array('absolute' => TRUE));
  $url_b2b   = url('node/'. $node->nid .'/info/b2b.xml', array('absolute' => TRUE));

  $content .= "<p><strong>Backend</strong>: ". $url_backend ."</p>";
  $content .= "<p><strong>Post URL</strong>: ". $url_post ."</p>";
  $content .= "<p><strong>B2B discover URL</strong> (for ". l("JCoinCoin", "http://fredm00.free.fr/jcoincoin/") ."): ". $url_b2b ."</p>";

  $login = $user->uid ? $user->name : t("YOUR_LOGIN");

  $wmcc_conf .= "board_site:                \"". $node->title ."\"\n";
  $wmcc_conf .= ".backend_flavour:          2\n";
  $wmcc_conf .= ".palmipede.userlogin:      ". $login ."\n";
  $wmcc_conf .= ".backend.url:              ". $url_backend ."\n";
  $wmcc_conf .= ".post.url:                 ". $url_post ."\n";
  $wmcc_conf .= ".tribune.delay:            ". $node->tribune_settings['reload_delay'] ."\n";
  $wmcc_conf .= ".palmipede.msg_max_length: ". $node->tribune_settings['max_message_size'] ."\n";
  if ($user->uid) {
    $wmcc_conf .= "\n";
    $wmcc_conf .= "\"". $node->title ."\" cookie: \"". $cookie ."\"\n";
  }

  $content .= t("<div>For <strong><a href='http://hules.free.fr/wmcoincoin'>wmcoincoin</a> users, wmc&sup2; configuration</strong> (place it in <code>~/.wmcoincoin/options</code>):");
  $content .= "<pre>". $wmcc_conf ."</pre>";
  $content .= "</div>";

  return $content;
}

function tribune_b2b_xml($node) {
  $site_root = url(NULL, array('absolute' => TRUE)) . (variable_get('clean_url', 0) ? '' : '?q=');

  $url_backend = url('node/'. $node->nid .'/tribune.xml');
  $url_post    = url('node/'. $node->nid .'/tribune/post');

  $name = check_plain($node->title);
  $maxlength = $node->tribune_settings['max_message_size'];

  header("Content-Type: application/x-coincoin");
  print <<<EOT
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE bouchot SYSTEM "http://www.nraynaud.org/kilombo/b2b-config.dtd">

<bouchot name="$name" title="$name">
   <site>$site_root</site>

   <module name="tribune" type="tribune">

     <backend url="$url_backend">
       <slip type="tags_not_encoded" />
     </backend>

     <post url="$url_post">
       <data>message=%m</data>
       <maxlength>$maxlength</maxlength>
       <anonym>true</anonym>
     </post>
   </module>

</bouchot>
EOT;
  exit();
}
