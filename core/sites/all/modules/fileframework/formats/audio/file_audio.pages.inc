<?php
// $Id$

/**
 * @file
 * User page callbacks.
 */

//////////////////////////////////////////////////////////////////////////////
// Xspf playlist

/**
 * Outputs a xspf playlist xml.
 *
 * @param $nid
 *   A node ID.
 * @param $vid
 *   A revision ID.
 * @param $hash
 *   A bitstram hash.
 */
function file_audio_xspf($hash, $nid, $vid) {
  if (is_numeric($nid) && is_numeric($vid)) {
    $node = node_load($nid, $vid);
    if (!node_access('view', $node))
      exit;
  }
  else {
    $node = file_bitcache_access($hash);
    if (!is_object($node))
      exit;
  }

  $file = $node->file;
  $uri = bitcache_uri($hash);

  // Check if the requested uri is a derivatie of the original one.
  if ($file->uri != $uri && !rdf_exists($uri, rdf_qname_to_uri('dc:source'), $file->uri, array('repository' => FILE_RDF_REPOSITORY))) {
    // The URI is not the node iteslf, nor the derivative.
    drupal_access_denied();
    exit;
  }

  $item = rdf_normalize(rdf_query($uri, NULL, NULL, array('repository' => FILE_RDF_REPOSITORY)));
  $src = bitcache_resolve_id($hash, array('absolute' => TRUE, 'query' => !empty($vid) ? array('vid' => $vid) : array()));

  $output = '<?xml version="1.0" encoding="UTF-8"?>'."\n";
  $output .= '<playlist version="1" xmlns="http://xspf.org/ns/0/">'."\n";
  $output .= '  <trackList>'."\n";
  $output .= '    <track>'."\n";
  $output .= '      <location>'. $src .'</location>'."\n";
  if (isset($item[$uri][rdf_qname_to_uri('foaf:made')]) && $artist = $item[$uri][rdf_qname_to_uri('foaf:made')][0])
    $output .= '      <creator>'. $artist .'</creator>'."\n";
  if (isset($item[$uri][rdf_qname_to_uri('wordnet:album')]) && $album = $item[$uri][rdf_qname_to_uri('wordnet:album')][0])
    $output .= '      <album>'. $album .'</album>'."\n";
  if (isset($item[$uri][rdf_qname_to_uri('dc:title')]) && $title = $item[$uri][rdf_qname_to_uri('dc:title')][0])
    $output .= '      <title>'. $title .'</title>'."\n";
  if (isset($item[$uri][rdf_qname_to_uri('wordnet:length')]) && $length = $item[$uri][rdf_qname_to_uri('wordnet:length')][0]) {
    list($m, $s) = split(':', $length);
    $length = (int)(($m * 60 + $s) * 1000);
    $output .= '      <duration>'. $length .'</duration>'."\n";
  }
  $output .= '      <annotation>'. $node->title .'</annotation>'."\n";
  $output .= '    </track>'."\n";
  $output .= '  </trackList>'."\n";
  $output .= '</playlist>'."\n";

  drupal_set_header('Content-Type: application/xspf+xml; charset=utf-8');
  print($output);
  exit;
}

