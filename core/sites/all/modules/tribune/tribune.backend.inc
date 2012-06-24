<?php
// vim:filetype=php expandtab tabstop=2 softtabstop=2 shiftwidth=2 autoindent smartindent
// $Id: tribune.backend.inc,v 1.6.2.3 2008/12/17 13:49:26 seeschloss Exp $


/**
  * Print the XML backend
  */
function tribune_backend($node, $format) {
  switch ($format) {
    case 'xml':
      header("Content-Type: text/xml");
      $posts = tribune_get_last_posts($node, $node->tribune_settings['xml_size']);
      print theme("tribune_xml", $posts, $node);
      break;
    case 'rss':
      header("Content-Type: application/rss+xml");
      $posts = tribune_get_last_posts($node, $node->tribune_settings['rss_size']);
      print theme("tribune_rss", $posts, $node);
      break;
  }
  exit();
}

/**
  * Returns the $nb last posts, in XML format
  * @param nb Number of posts to return (tribune_history_size by default)
  * @return String containing the posts in an HTML unordered list
  */
function theme_tribune_xml($posts, $node) {
  $contents  = "<?xml version=\"1.0\" encoding=\"UTF-8\""."?".">\n";
  $contents .= "<!DOCTYPE board PUBLIC \"C01N C01N !\" \"http://phplop.org/dtd/tribune-1.0.dtd\">\n";

  $timezone = variable_get('date_default_timezone', 0);
  $tz = "UTC". ($timezone < 0 ? "-" : "+") . round($timezone/3600);
  $contents .= "<board site=\"". url('node/'. $node->nid, array('absolute' => TRUE)) ."\" timezone=\"". $tz ."\">\n";

  $posts = array_reverse($posts);

  foreach ($posts as $id => $post) {
    if ($post['moderated'] == 0) {
      $contents .= theme("tribune_post_xml", $post, $node);
    }
  }

  $contents .= "</board>\n";

  return $contents;
}

/**
  * Returns the $nb last posts, in RSS format
  * @param nb Number of posts to return (tribune_history_size by default)
  * @return String containing the posts in an HTML unordered list
  */
function theme_tribune_rss($posts, $node) {
  $contents  = "<?xml version=\"1.0\"?>\n";
  $contents .= "<rss version=\"2.0\">\n";
  $contents .= "\t<channel>\n";
  $contents .= "\t\t<title>".variable_get("site_name", "")."</title>\n";
  $contents .= "\t\t<link>".url('node'. $node->nid, array('absolute' => TRUE))."</link>\n";
  $contents .= "\t\t<description>".variable_get("site_slogan", "")."</description>\n";

  $posts = array_reverse($posts, TRUE);

  foreach ($posts as $id => $post) {
    if ($post['moderated'] == 0) {
      $contents .= theme("tribune_post_rss", $post, $node);
    }
  }

  $contents .= "\t</channel>\n";
  $contents .= "</rss>\n";

  return $contents;
}

/**
  * Returns a single post, in XML format
  * @param post Array with the following keys : id, clock, login, info, message
  */
function theme_tribune_post_xml($post, $node) {
  $contents  = "    <post time=\"". $post['clock']  ."\" id=\"". $post['tribune_post_id'] ."\">\n";
  $contents .= "        <info>"   . $post['info']   ."</info>\n";
  $contents .= "        <message>". tribune_xml_slip($post['message'], $node) ."</message>\n";
  $contents .= "        <login>"  . $post['login']  ."</login>\n";
  $contents .= "    </post>\n";

  return $contents;
}

/**
  * Returns a single post, in RSS format
  * @param post Array with the following keys : id, clock, login, info, message
  */
function theme_tribune_post_rss($post, $node) {
  $contents  = "\t\t<item>\n";
  $contents .= "\t\t\t<description>". tribune_rss_slip($post['message'], $node) ."</description>\n";
  $contents .= "\t\t\t<pubDate>". date('r', tribune_date_to_timestamp($post['post_time'])) ."</pubDate>\n";
  $contents .= "\t\t\t<author>". tribune_rss_slip($post['login'], $node) ." &lt;". tribune_rss_slip($post['login'], $node) ."@null&gt;</author>\n";
  $contents .= "\t\t</item>\n";

  return $contents;
}

function tribune_rss_slip($string, $node) {
  require_once "tribune.sleep.inc";

  $string = tribune_sleep($string, $node->tribune_settings['max_message_size']);
  $string = preg_replace('#((https?|ftp|gopher|file|mms|rtsp|rtmp)://.*?)((,|\.|\)|\])?(<| | |"|$))#', '<a href="\1">[url]</a>\3', $string);
  $replacements = array(
    '<b>' => '<strong>',
    '</b>' => '</strong>',
    '<i>' => '<em>',
    '</i>' => '</em>',
    '<s>' => '<span style="text-decoration: underline">',
    '</s>' => '</span>',
    '<tt>' => '<code>',
    '</tt>' => '</code>',
  );
  $string = str_replace(array_keys($replacements), array_values($replacements), $string);
  $string = htmlspecialchars($string, ENT_QUOTES, 'UTF-8');

  return $string;
}
