<?php
// $Id$

/**
 * @file
 * User page callbacks.
 */

//////////////////////////////////////////////////////////////////////////////
// Mind Map file

/**
 * Outputs a Mind Map file.
 *
 * @param $nid
 *   A node ID.
 * @param $vid
 *   A revision ID.
 * @param $hash
 *   A bitstram hash.
 */
function file_document_mm($nid, $vid, $hash) {
  if (is_numeric($nid) && is_numeric($vid)) {
    $node = node_load($nid, $vid);
    if (!node_access('view', $node))
      exit;
  }
  else {
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

  drupal_set_header('Content-Type: application/x-freemind');
  drupal_set_header('Content-Length: '. $file->size);
  $fp = bitcache_get_stream($hash);
  fpassthru($fp);
  exit;
}

