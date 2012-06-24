<?php
// $Id$

/**
 * @file
 * User page callbacks.
 */

//////////////////////////////////////////////////////////////////////////////
// File metadata tab

/**
 * Outputs a themed metadata HTML page.
 *
 * @param $node
 *   A node object.
 *
 * @return
 *   Themed HTML metadata table.
 */
function file_page_metadata($node) {
  $namespaces = rdf_get_namespaces();
  $header = array(
    array('data' => t('Name'), 'field' => 'name'),
    array('data' => t('Value'), 'field' => 'value'),
  );

  db_query("CREATE TEMPORARY TABLE file_properties (name VARCHAR(255), value VARCHAR(255))");

  $info = file_get_metadata_info();
  $data = rdf_normalize(rdf_query($node->file->uri, NULL, NULL, array('repository' => FILE_RDF_REPOSITORY)));
  foreach ($data as $subject => $predicates) {
    foreach ($predicates as $predicate => $objects) {
      foreach ($objects as $object) {
        $rdf = rdf_uri_to_qname($predicate, $namespaces, FALSE);
        if (isset($info[$rdf]) && ($name = $info[$rdf]['name'])) {
          db_query("INSERT INTO file_properties (name, value) VALUES ('%s', '%s')", $name, isset($info[$rdf]['theme']) ? theme($info[$rdf]['theme'], is_object($object) ? $object->value : $object) : theme('rdf_value', $object));
        }
      }
    }
  }

  $result = db_query('SELECT * FROM file_properties'. tablesort_sql($header));
  while ($row = db_fetch_object($result)) {
    $rows[] = array(
      //array('data' => $row->name .':', 'align' => 'right'),
      $row->name,
      $row->value,
    );
  }

  if (empty($rows)) {
    $rows[] = array(array('data' => t('No data.'), 'colspan' => 2));
  }

  return theme('table', $header, $rows, array('class' => 'file-properties'));
}

//////////////////////////////////////////////////////////////////////////////
// File previews

/**
 * Outputs an embeddable XHTML snippet containing a viewer for a file, and
 * terminates the page request.
 *
 * This is suitable for use in Ajax-based file previews, for instance.
 *
 * @param $node
 *   A node object as returned from node_load().
 * @param $handler
 *   A handler to display object.
 */
function file_embed($node, $handler) {
  if ($output = file_wrapper_html($node, $handler)) {
    print $output;
  }
  else {
    drupal_set_header('HTTP/1.0 404 Not Found');
    drupal_set_header('Content-Type: text/plain; charset=utf-8');
    print '404 Not Found';
  }

  exit();
}

//////////////////////////////////////////////////////////////////////////////
// File serve

/**
 * Redirects to the file view or download path.
 *
 * @param $node
 *   A node object as returned from node_load().
 * @param $vid
 *   A node version ID.
 * @param $disposition
 *   Disposition of the file. Might be 'download' or 'inline'.
 */
function file_serve($node, $vid, $disposition = 'attachment') {
  if (isset($vid) && $node->vid != $vid) {
    $node = node_load($node->nid, $vid);
  }
  if ($node->type != 'file' || !is_object($node->file) || (!empty($vid) && !is_numeric($vid))) {
    drupal_not_found();
    return;
  }
  $href = bitcache_resolve_uri($node->file->uri);
  $query = array('vid' => isset($vid) ? $vid : $node->vid, 'disposition' => $disposition, 'op' => ($disposition == 'attachment') ? 'download' : 'view');
  drupal_goto($href, $query);
}

//////////////////////////////////////////////////////////////////////////////
// File metadata

/**
 * Prints file metadata.
 *
 * @param $uri
 *   A blob uri.
 * @param $nid
 *   A node ID.
 *
 * @return
 *   Metadata html page
 */
function file_metadata($uri, $nid) {

  $access = FALSE;
  $uri = bitcache_uri($uri);
  $parent_uri = rdf_value($uri, rdf_qname_to_uri('dc:source'));

  $files = array_merge(
    isset($_SESSION['file_preview_file']) && is_object($_SESSION['file_preview_file']) ? array($_SESSION['file_preview_file']) : array(),
    isset($_SESSION['file_attach_files']) && is_array($_SESSION['file_attach_files']) ? $_SESSION['file_attach_files'] : array(),
    isset($_SESSION['file_cck_files']) && is_array($_SESSION['file_cck_files']) ? $_SESSION['file_cck_files'] : array()
  );

  foreach ($files as $file) {
    if (is_object($file) && $file->uri == $parent_uri || $file->uri == $uri) {
      $access = TRUE;
    }
  }

  if (is_numeric($nid)) {
    $node = node_load($nid);
    if (node_access('view', $node) && $node->type == 'file' && db_result(db_query("SELECT f.* FROM {file_nodes} f WHERE f.nid = %d AND (f.uri = '%s' OR f.uri = '%s')", $node->nid, $uri, $parent_uri))) {
      $access = TRUE;
    }
  }

  if ($access && $rdf = rdf_normalize(rdf_query($uri, NULL, NULL, array('repository' => FILE_RDF_REPOSITORY)))) {
    $namespaces = rdf_get_namespaces();
    $info = file_get_metadata_info();
    foreach ($rdf as $subject => $predicates) {
      foreach ($predicates as $predicate => $objects) {
        foreach ($objects as $object) {
          $rdf = rdf_uri_to_qname($predicate, $namespaces, FALSE);
          if (isset($info[$rdf]) && $name = $info[$rdf]['name']) {
            $data[] = array($name, isset($info[$rdf]['theme']) ? theme($info[$rdf]['theme'], is_object($object) ? $object->value : $object) : theme('rdf_value', $object));
          }
        }
      }
    }
    return theme('file_metadata', $data);
  }

  drupal_access_denied();
  exit;
}

//////////////////////////////////////////////////////////////////////////////
// OG integration

/**
 * Prints og file browser.
 *
 * @param $node
 *   A group node object.
 *
 * @return
 *   Browser html page
 */
function file_og_browser($node, $vid) {
  drupal_set_title(check_plain($node->title));
  if (isset($node->og_vocabularies) && is_array($vocabs = $node->og_vocabularies) && !empty($vocabs)) {
    if (isset($vid) && isset($vocabs[$vid])) {
      return file_browser_page($vid);
    }
    else {
      foreach ($vocabs as $vid => $v) {
        $vids[] = $vid;
      }
      $script = (count($vids) == 1) ? '<script type=\'text/javascript\'>$(document).ready(function() { $("#file-folder-v'. reset($vids) .'-bpage > div.file-cells").click(); });</script>' : '';
      return file_browser_page($vids, NULL, $script);
    }
  }
  else {
    return t('No group vocabularies defined.');
  }
}

/**
 * Prints og file gallery.
 *
 * @param $node
 *   A group node object.
 *
 * @return
 *   Browser html page
 */
function file_og_gallery($node, $vid, $tid, $filter) {
  drupal_set_title(check_plain($node->title));
  if (isset($node->og_vocabularies) && is_array($vocabs = $node->og_vocabularies) && !empty($vocabs)) {
    if (isset($vid) && isset($vocabs[$vid])) {
      return file_gallery_page($vid, $tid, $filter, 'file');
    }
    else {
      foreach ($vocabs as $vid => $v) {
        $vids[] = $vid;
      }
      return (count($vids) == 1) ? file_gallery_page(reset($vids), NULL, NULL, 'file') : file_gallery_page($vids, NULL, NULL, 'file');
    }
  }
  else {
    return t('No group vocabularies defined.');
  }
}

