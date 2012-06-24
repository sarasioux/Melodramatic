<?php
// $Id$

/**
 * @file
 * Implements an extended procedural File API for file management.
 */

//////////////////////////////////////////////////////////////////////////////
// File node API

/**
 * Saves file blob in the bitchache and saves file uri to the db.
 *
 * @param $node
 *   A populated node object.
 * @param $file
 *   A popolated file object.
 *
 * @return
 *   SAVED_NEW or SAVED_UPDATED on success or FALSE on failure.
 */
function file_node_save($node, $file) {
  if (!is_object($file)) {
    watchdog('file', 'The file object is not passed to a file_node_save() function for the node with nid=%nid.', array('%nid' => $node->nid), WATCHDOG_ERROR, isset($node->nid) ? l(t('view'), 'node/'. $node->nid) : NULL);
    return FALSE;
  }

  if (isset($file->uri)) {
    // Sanity check if we are saving a file which is already put in bitcche.

    $file->filename = isset($file->filename) ? $file->filename : $node->title;

    // Check if file exists in the bitcache.
    if (!bitcache_exists(file_get_hash($file->uri))) {
      watchdog('file', 'The blob was not found in the bitcache for the %name, uri=%uri.', array('%name' => $file->filename, '%uri' => $file->uri), WATCHDOG_ERROR, isset($file->nid) ? l(t('view'), 'node/'. $file->nid) : NULL);
      return FALSE;
    }

    $file->size = isset($file->size) ? $file->size : bitcache_get_size(file_get_hash($file->uri));
    $file->type = isset($file->type) ? $file->type : bitcache_get_type(file_get_hash($file->uri));
  }

  if (!isset($file->uri)) {
    // Note that at this point, we have a transient file object with a path,
    // MIME type and file size. The file is still located in the temporary
    // directory.

    // If we didn't get a proper MIME type from the client, attempt to guess
    // it from the uploaded file name
    if (empty($file->filemime) || $file->filemime == 'application/octet-stream')
      $file->filemime = file_mime_guess($file->filepath);

    // If specified by the administrator, we'll always try and autodetect the
    // MIME type on the server-side for any uploaded files
    if (FILE_MIME_AUTODETECTION > 0 && ($mime = file_mime_detect($file)))
      $file->filemime = $mime;

    // Assign nid and vid to the file object for the logging.
    if (isset($node->nid)) {
      $file->nid = $node->nid;
      $file->vid = $node->vid;
    }

    // Move the file to permanent storage, deleting the temporary file.
    // Save metadata information to the RDF.
    if (!file_data_save($file))
      return FALSE;

    // Remove the entry from the {files} table about the temporary file, which was
    // added by the file_save_upload().
    if (isset($file->fid))
      db_query('DELETE FROM {files} WHERE fid = %d', $file->fid);

    // Generate and save the preview of the file.
    if (!isset($file->noconvert) && !file_generate_previews($file))
      watchdog('file', 'At least one preview generation faild for the file %name.', array('%name' => $file->filename), WATCHDOG_ERROR, isset($node->nid) ? l(t('view'), 'node/'. $node->nid) : '');

    // Delete uploaded file.
    if (isset($file->filepath))
      file_delete($file->filepath);
  }

  // Create a file object for the node and saving it to the db.
  $node->file = (object)array('nid' => isset($node->nid) ? $node->nid : NULL, 'vid' => isset($node->vid) ? $node->vid : NULL, 'uri' => $file->uri, 'size' => isset($file->filesize) ? $file->filesize : $file->size, 'type' => isset($file->filemime) ? $file->filemime : $file->type, 'name' => isset($file->filename) ? $file->filename : (isset($file->name) ? $file->name : NULL));
  if (isset($node->nid) && !isset($node->nosave)) {
    if ($no_revision = (empty($node->is_new) && empty($node->revision)))
      $uri_old = db_result(db_query('SELECT fn.uri FROM {file_nodes} fn JOIN {node} n ON fn.nid = n.nid AND fn.vid = n.vid WHERE fn.nid = %d', $node->nid));
    $return = drupal_write_record('file_nodes', $node->file, $no_revision ? array('nid', 'vid') : array());
  }
  else {
    global $user;
    $file_tmp = (object)array('uid' => $user->uid, 'uri' => $file->uri, 'created' => time());
    $return = drupal_write_record('file_tmp', $file_tmp);
  }

  // Delete old file blob.
  if (isset($no_revision) && $no_revision)
    file_node_delete_node((object)array('uri' => $uri_old));

  return $return;
}

/**
 * Saves file blob in the bitchache and metadata to RDF repository.
 *
 * @param $file
 *   A popolated file object.
 *
 * @return
 *   Bitcache resource uri.
 */
function file_data_save($file, $tier = NULL) {
  // File itself or derivative to be saved.
  $file_save = isset($tier) ? $file->converted[$tier] : $file;
  $file_save->filemime = isset($tier) ? $file->mimes[$tier] : $file->filemime;

  // Is the file already in bitcache?
  if (isset($file_save->uri))
    return TRUE;

  // Check the file size.
  if (isset($file_save->filepath) && file_exists($file_save->filepath) && !filesize($file_save->filepath)) {
    watchdog('file', 'The file %name or it\'s derivative to %mime was not saved because it was empty.', array('%name' => $file->filename, '%mime' => $file_save->filemime), WATCHDOG_ERROR, isset($file->nid) ? l(t('view'), 'node/'. $file->nid) : NULL);
    return FALSE;
  }

  // Move the file to permanent storage, deleting the temporary file.
  bitcache_use_repository(FILE_BITCACHE_REPOSITORY);
  if (!($hash = bitcache_put_file(NULL, $file_save->filepath, isset($file->noconvert) ? TRUE : FALSE))) {
    watchdog('file', 'The bitcache_put_file() function failed for the file %name, uri=%uri or it\'s derivative in %mime.', array('%name' => $file->filename, '%uri' => $file->uri, '%mime' => $file_save->filemime), WATCHDOG_ERROR, isset($file->nid) ? l(t('view'), 'node/'. $file->nid) : NULL);
    return FALSE;
  }

  if (($uri = bitcache_uri($hash)) && isset($tier))
    $file->converted[$tier]->uri = $uri;
  else
    $file->uri = $uri;

  // Hook with a parent file.
  $parent = isset($tier) ? array('dc:source' => array($file->uri)) : array();

  // Save metadata information to the RDF.
  $metadata = isset($file_save->metadata) ? array($uri => array_merge($parent, $file_save->metadata)) : array_merge_recursive(array($uri => $parent), file_get_metadata($file_save));

  // A generated file can have several parent filess, for instance the same
  // video file with different audio codecs will have the same image preview.
  foreach (file_metadata_normalize(rdf_normalize(rdf_query($file_save->uri, NULL, NULL, array('repository' => FILE_RDF_REPOSITORY)))) as $p => $o) {
    if (in_array($p, array('dc:source', 'dc:creator'))) {
      $metadata[$uri][$p] = array_unique(array_merge($o, isset($metadata[$uri][$p]) ? $metadata[$uri][$p] : array()));
    }
  }
  if (!rdf_delete($uri, NULL, NULL, array('repository' => FILE_RDF_REPOSITORY))) {
    watchdog('file', 'The RDF data was not deleted for the subject s=%sub.', array('%sub' => $key), WATCHDOG_ERROR);
    return FALSE;
  }
  if (!rdf_insert_all(rdf_denormalize($metadata), array('repository' => FILE_RDF_REPOSITORY))) {
    watchdog('file', 'The RDF data was not saved for the file %name, uri=%uri or it\'s derivative in %mime.', array('%name' => $file->filename, '%uri' => $file->uri, '%mime' => $file_save->filemime), WATCHDOG_ERROR, isset($file->nid) ? l(t('view'), 'node/'. $file->nid) : NULL);
    return FALSE;
  }
  return TRUE;
}

/**
 * Programmatically creates a new file container node.
 *
 * @param $node
 *   A pre-populated node array.
 *
 * @return
 *   A fully populated node object.
 */
function file_node_create($node = array()) {
  global $user;
  $node = (object)$node;

  if (empty($node->file))
    return FALSE;

  // Set default attributes
  $node->type = 'file';
  $node_type_default = variable_get('node_options_'. $node->type, array('status', 'promote'));
  $node->title = isset($node->title) ? $node->title : (isset($node->file->name) ? $node->file->name : $node->file->filename);
  $node->uid = isset($node->uid) ? $node->uid : $user->uid;
  $node->status = isset($node->status) ? $node->status : in_array('status', $node_type_default);
  $node->comment = isset($node->comment) ? $node->comment : variable_get('comment_'. $node->type, 2);
  $node->promote = isset($node->promote) ? $node->promote : in_array('promote', $node_type_default);
  $node->moderate = isset($node->moderate) ? $node->moderate : in_array('moderate', $node_type_default);
  $node->revision = isset($node->revision) ? $node->revision : in_array('revision', $node_type_default);
  $node->sticky = isset($node->sticky) ? $node->sticky : in_array('sticky', $node_type_default);
  $node->format = FILTER_FORMAT_DEFAULT;
  $node->body = '';

  // Save and load the node to get a proper $node->file.
  node_save($node);
  return node_load($node->nid);
}

/**
 * Deletes a file and all previews.
 *
 * @param $node
 *   A pre-populated node array.
 *
 */
function file_node_delete_node($file) {
  if (!empty($file->uri) && db_result(db_query("SELECT COUNT(f.nid) FROM {file_nodes} f WHERE f.uri = '%s'", $file->uri)) == 0) {
    bitcache_use_repository(FILE_BITCACHE_REPOSITORY);

    // The last reference to the blob. Search for a previews.
    if ($derived = rdf_normalize(rdf_query(NULL, NULL, $file->uri, array('repository' => FILE_RDF_REPOSITORY)))) {
      // Delete all previews.
      foreach ($derived as $uri => $data) {
        $item = rdf_normalize(rdf_query($uri, NULL, NULL, array('repository' => FILE_RDF_REPOSITORY)));

        $db_cont = db_result(db_query("SELECT COUNT(f.nid) FROM {file_nodes} f WHERE f.uri = '%s'", $uri));
        $parent_count = count($item[$uri][rdf_qname_to_uri('dc:source')]);

        // Delete if derived is not referenced in the {file_nodes} table
        // or it is not also a preview for other uploaded file.
        if ($db_cont > 0 || $parent_count > 1) {
          if (!rdf_delete($uri, rdf_qname_to_uri('dc:source'), $file->uri, array('repository' => FILE_RDF_REPOSITORY)))
            watchdog('file', 'The RDF data was not deleted for the subject s=%sub.', array('%sub' => $uri), WATCHDOG_ERROR);
          if ($parent_count == 1 && !rdf_delete($uri, rdf_qname_to_uri('dc:creator'), NULL, array('repository' => FILE_RDF_REPOSITORY)))
            watchdog('file', 'The RDF data was not deleted for the subject s=%sub.', array('%sub' => $uri), WATCHDOG_ERROR);
        }
        else {
          if (!rdf_delete($uri, NULL, NULL, array('repository' => FILE_RDF_REPOSITORY)))
            watchdog('file', 'The RDF data was not deleted for the subject s=%sub.', array('%sub' => $uri), WATCHDOG_ERROR);

          // Delete from a bitcache.
          if (!bitcache_delete(file_get_hash($uri)))
            watchdog('file', 'The %uri blob was not deleted from the bitcache.', array('%uri' => $uri), WATCHDOG_ERROR);
        }
      }
    }

    // Delete from the RDF.
    if (!rdf_delete($file->uri, NULL, NULL, array('repository' => FILE_RDF_REPOSITORY)))
      watchdog('file', 'The RDF data was not deleted for the subject s=%sub.', array('%sub' => $file->uri), WATCHDOG_ERROR);

    // Delete from a bitcache.
    if (!bitcache_delete(file_get_hash($file)))
      watchdog('file', 'The %uri blob was not deleted from the bitcache.', array('%uri' => $file->uri), WATCHDOG_ERROR);
  }
}

//////////////////////////////////////////////////////////////////////////////
// File MIME type detection

/**
 * This function executes all hook_mime_types() hooks in the format modules
 * and creates an array of all supported MIME types pointing tho the array of
 * module name, description and icon file.
 *
 * @return
 *   A structured array of the MIME types pointing to the arary of the MIME details.
 */
function file_get_mime_types() {
  static $handlers = array();
  if (empty($handlers)) {
    foreach (module_implements('mime_types') as $module) {
      if (($types = module_invoke($module, 'mime_types')) && is_array($types)) {
        foreach ($types as $mime => $data) {
          $types[$mime]['module'] = $module;
          if (!isset($data['handlers']))
            $types[$mime]['handlers'] = array();
          if (!isset($data['extensions']))
            $types[$mime]['extensions'] = array();
        }
        $handlers = array_merge_recursive($handlers, $types);
      }
    }
  }
  return $handlers;
}

/**
 * Implementation of hook_mime_types().
 */
function file_mime_types() {
  return array(
    'application/octet-stream' => array(
      'name' => t('Unrecognized file type'),
      'icon' => 'binary.gif',
    ),
  );
}

/**
 * Returns a description of the MIME type. If a particular format module is not enabled
 * the default description is returned.
 *
 * @param $type
 *   A MIME type.
 * @param $default
 *   Default description for the MIME type to be return if no modules are implementing the MIME type.
 *
 * @return
 *   The description for the MIME type.
 */
function file_mime_description_for($type, $default = NULL) {
  $handlers = file_get_mime_types();
  $description = isset($handlers[$type]) && isset($handlers[$type]['name']) ? $handlers[$type]['name'] : NULL;
  $default_mime_types = file_default_mime_types();
  return isset($description) ? $description :
    // If the exact mime type is not defined, we give a default description.
    (in_array(preg_replace('/([^\/]+).*/', '$1/*', $type), array_keys($default_mime_types)) ? $default_mime_types[preg_replace('/([^\/]+).*/', '$1/*', $type)]['name'] : $default);
}

/**
 * Returns an image HTML block with the MIME type icon.
 *
 * @param $type
 *   A MIME type.
 *
 * @return
 *   The HTML image block for the MIME type.
 */
function file_mime_icon_for($type, $description = NULL) {
  $handlers = file_get_mime_types();
  $default_mime_types = file_default_mime_types();
  $icon = isset($handlers[$type]['icon']) ? $handlers[$type]['icon'] : NULL;
  $icon = isset($icon) ? $icon : (in_array(preg_replace('/([^\/]+).*/', '$1/*', $type), array_keys($default_mime_types)) ? $default_mime_types[preg_replace('/([^\/]+).*/', '$1/*', $type)]['icon'] : '');
  if (!empty($icon) && file_exists(drupal_get_path('module', 'file') .'/icons/'. $icon))
    return theme('image', drupal_get_path('module', 'file') .'/icons/'. $icon, $description, $description, NULL, TRUE);

  return NULL;
}

/**
 * Returns a MIME extensions registered with the systen.
 *
 * $custom
 *   Should a custom MIME additions be included?
 *
 * @return
 *   A structured array of the MIME types pointing to the array of extensions.
 */
function file_mime_extensions($custom = 1) {
  static $registry = NULL;
  if (!$registry) {
    $registry = array();
    module_load_include('inc', 'file', 'file.mime');
    $registry = array_map(create_function('$a', 'return explode(\'|\', $a);'), array_flip(file_drupal_mime_types()));

    foreach (file_get_mime_types() as $type => $data) {
      if (!empty($data['extensions']))
        $registry[$type] = isset($registry[$type]) ? array_unique(array_merge($data['extensions'], $registry[$type])) : $data['extensions'];
    }

    // Placing custom variables at the begining of the array.
    $registry = array_merge(variable_get('file_mime_types', array()), array_merge($registry, variable_get('file_mime_types', array())));
    //foreach (variable_get('file_mime_types', array()) as $type => $extensions)
      //$registry[$type] = $extensions;
  }
  return $registry;
}

/**
 * Returns supported extensions for the MIME type.
 *
 * @param $type
 *   A MIME type.
 *
 * @return
 *   An array of the extensions.
 */
function file_mime_extensions_for($type) {
  $extensions = file_mime_extensions();
  return isset($extensions[$type]) ? $extensions[$type] : array();
}

/**
 * Finds the first matching MIME content type based on a file's extension.
 *
 * @param $filename
 *   Full filename of the file.
 *
 * @return
 *   A MIME type of the file.
 */
function file_mime_guess($filename) {
  $registry = file_mime_extensions();
  $pathinfo = pathinfo($filename);
  $extension = $pathinfo['extension'];
  foreach ($registry as $type => $extensions) {
    if (in_array($extension, $extensions))
      return $type;
  }

  // Default MIME type if the file extension is not registerd.
  return 'application/octet-stream';
}

/**
 * Tries to detect the file MIME type using other utilities.
 *
 * @param $filename
 *   Full filename of the file.
 *
 * @return
 *   A MIME type of the file.
 */
function file_mime_detect($file) {
  if (FILE_MIME_AUTODETECTION == 2) {
    $mimes = $exts = array();
    foreach (explode("\n", FILE_MIME_AUTODETECTION_CONDITIONS) as $line) {
      $line = trim($line);
      if (!empty($line)) {
        if (preg_match('/\//', $line)) {
          $mimes[] = $line;
        }
        else {
          $exts[] = $line;
        }
      }
    }
    $ext = preg_replace('/^.*\.([^\.]+)$/', '$1', drupal_strtolower($file->filename));
    if (!in_array($ext, $exts) && !in_array($file->filemime, $mimes))
      return FALSE;
  }

  if (extension_loaded('fileinfo') && ($finfo = finfo_open(FILEINFO_MIME))) {
    // Use 'fileinfo' php extension
    $type = finfo_file($finfo, $file->filepath);
    finfo_close($finfo);
    return $type;
  }
  else if (file_exists($file->filepath)) {
    // Attempts to detect a file's MIME type using the Unix `file' utility.
    // MIME content type format defined in http://www.ietf.org/rfc/rfc1521.txt
    $filename = realpath($file->filepath);
    $output = exec('file -bi '. escapeshellarg($file->filepath), $ignore, $status);
    if ($status == 0 && preg_match('!([\w-]+/[\w\d_+-]+)!', $output, $matches))
      return $matches[1];
  }

  // Default MIME type if the file extension is not registerd.
  return $file->filemime;
}

/**
 * Default MIME types and their descriptions.
 * This is needed if corresponding format modules are not enabled.
 * object.
 *
 * @return
 *   Structured array of default MIME types.
 */
function file_default_mime_types() {
  return array(
    'text/*' => array(
      'name' => t('Text file'),
      'icon' => 'text.gif',
    ),
    'image/*' => array(
      'name' => t('Image file'),
      'icon' => 'image.gif',
    ),
    'audio/*' => array(
      'name' => t('Audio file'),
      'icon' => 'audio.gif',
    ),
    'video/*' => array(
      'name' => t('Video file'),
      'icon' => 'video.gif',
    ),
    'application/*' => array(
      'name' => t('Unrecognized file type'),
      'icon' => 'binary.gif',
    ),
  );
}

/**
 * This function executes all hook_mime_handlers() hooks in the format modules
 * and creates an array of all supported MIME handlers pointing tho the array of
 * handler details.
 *
 * @return
 *   A structured array of the MIME type handlers pointing to the handler detailsi.
 */
function file_get_mime_handlers() {
  static $mime_handlers = array();
  if (empty($mime_handlers)) {
    $handlers = array();
    foreach (module_implements('mime_handlers') as $module) {
      if (($types = module_invoke($module, 'mime_handlers')) && is_array($types)) {
        foreach ($types as $handler => $data) {
          $types[$handler]['module'] = $module;
          if (!isset($data['weight']))
            $types[$handler]['weight'] = 0;
        }
        $handlers = array_merge_recursive($handlers, $types);
      }
    }
    $handlers_sort = array();
    foreach ($handlers as $handler => $data) {
      $handlers_sort[$handler] = $data['weight'];
    }
    $default_handlers = variable_get('file_handlers', array());
    $default_handlers_sort = array();
    if (is_array($default_handlers)) {
      foreach ($default_handlers as $handler => $data) {
        $default_handlers_sort[$handler] = $data['weight'];
      }
    }
    $handlers_sorted = array_merge($handlers_sort, $default_handlers_sort);
    asort($handlers_sorted);
    foreach ($handlers_sorted as $handler => $weight) {
      if (isset($handlers[$handler])) {
        $mime_handlers[$handler] = array(
          'name' => $handlers[$handler]['name'],
          'dimensions' => isset($handlers[$handler]['dimensions']) ? $handlers[$handler]['dimensions'] : '0x0',
          'parent' => isset($handlers[$handler]['parent']) ? $handlers[$handler]['parent'] : NULL,
          'weight' => $weight,
          'enabled' => isset($default_handlers[$handler]['enabled']) ? $default_handlers[$handler]['enabled'] : (isset($handlers[$handler]['enabled']) ? $handlers[$handler]['enabled'] : 1),
          'module' => $handlers[$handler]['module'],
        );
      }
    }
  }
  return $mime_handlers;
}

//////////////////////////////////////////////////////////////////////////////
// File metadata extraction

/**
 * This function executes all hook_metadata_info() hooks in the format modules
 * and creates an array of the metadata info.
 *
 * @return
 *   A structured array of the metadata information.
 */
function file_get_metadata_info() {
  static $result = array();
  if (empty($result)) {
    $result['dc:format'] = array('name' => t('MIME type'));
    $result['dc:extent'] = array('name' => t('Size'), 'theme' => 'file_metadata_size');
    foreach (module_implements('metadata_info') as $module) {
      if (($info = module_invoke($module, 'metadata_info')) && is_array($info))
        $result = array_merge($result, $info);
    }
  }
  return $result;
}

/**
 * This function executes all hook_metadata_parse() hooks in the format modules
 * and creates an array of the file's metadata.
 *
 * @param $filename
 *   A name of the file.
 * @param $mimetype
 *   A file's MIME type.
 *
 * @return
 *   A structured array of the metadata information.
 */
function file_metadata_extract($filename, $mimetype) {
  $result = array(
    'dc:format' => array($mimetype),
    'dc:extent' => array(filesize($filename)),
  );
  $parsed = FALSE;
  foreach (module_implements('metadata_parse') as $module) {
    if (($metadata = module_invoke($module, 'metadata_parse', $filename, $mimetype)) && is_array($metadata)) {
      $result = array_merge($result, $metadata, array('wordnet:metadata' => array('parsed')));
      $parsed = TRUE;
    }
  }
  if (!$parsed)
    $result = array_merge($result, array('wordnet:failed' => array('metadata')));

  return $result;
}

/**
 * Extracts file's metadata information.
 *
 * @param $file
 *   A file object as returned from file_node_load().
 *
 * @return
 *   Structured array pointing to the structored array of metadata information.
 */
function file_get_metadata($file) {
  return array($file->uri => file_metadata_extract(bitcache_get_path(file_get_hash($file), TRUE), $file->filemime));
}

/**
 * Generates and saves the preview for the file based on the MIME type.
 *
 * @param $file
 *   A file object as returned from file_node_load().
 * @param $cron
 *   A flag to show that function is called from the cron.
 *
 * @return
 *   TRUE if the preview was generated, FALSE otherwise.
 */
function file_generate_previews($file, $cron = FALSE) {
  $result = TRUE;
  $previews = array();

  // Check if file exists in the bitcache.
  if (!bitcache_exists(file_get_hash($file->uri))) {
    watchdog('file', 'The blob was not found in the bitcache for the %name, uri=%uri.', array('%name' => $file->filename, '%uri' => $file->uri), WATCHDOG_ERROR, isset($file->nid) ? l(t('view'), 'node/'. $file->nid) : NULL);
    return FALSE;
  }

  //Extract metadata if it is missing and save to RDF.
  if (!rdf_exists($file->uri, 'wordnet:failed', 'metadata', array('repository' => FILE_RDF_REPOSITORY)) && !rdf_exists($file->uri, 'wordnet:metadata', 'parsed', array('repository' => FILE_RDF_REPOSITORY))) {
    if (!rdf_delete($file->uri, NULL, NULL, array('repository' => FILE_RDF_REPOSITORY)))
      watchdog('file', 'The RDF data was not deleted for the %name, uri=%uri.', array('%name' => $file->filename, '%uri' => $file->uri), WATCHDOG_ERROR, isset($file->nid) ? l(t('view'), 'node/'. $file->nid) : NULL);
    if (!rdf_insert_all(rdf_denormalize(file_get_metadata($file)), array('repository' => FILE_RDF_REPOSITORY))) {
      watchdog('file', 'The RDF data was not saved for the the %name, uri=%uri.', array('%name' => $file->filename, '%uri' => $file->uri), WATCHDOG_ERROR, isset($file->nid) ? l(t('view'), 'node/'. $file->nid) : NULL);
      return FALSE;
    }
  }
  $file->metadata = file_metadata_normalize(rdf_normalize(rdf_query($file->uri, NULL, NULL, array('repository' => FILE_RDF_REPOSITORY))));

  // Find generated previews for the file.
  $file->previews = array();
  if ($generated = rdf_normalize(rdf_query(NULL, rdf_qname_to_uri('dc:source'), $file->uri, array('repository' => FILE_RDF_REPOSITORY)))) {
    foreach ($generated as $uri => $data) {
      $item = rdf_normalize(rdf_query($uri, NULL, NULL, array('repository' => FILE_RDF_REPOSITORY)));
      if (!bitcache_exists(file_get_hash($uri)) || !isset($item[$uri][rdf_qname_to_uri('dc:extent')][0]->value)) {
        // The blob does not exist in the bitcache or the size is zero.
        // We delete the generated entry reference in the RDF
        // and the file will be converted again.
        if (!rdf_delete($uri, NULL, NULL, array('repository' => FILE_RDF_REPOSITORY)))
          watchdog('file', 'The RDF data was not deleted for the %name missing derivative  uri=%uri.', array('%name' => $file->filename, '%uri' => $uri), WATCHDOG_ERROR, isset($file->nid) ? l(t('view'), 'node/'. $file->nid) : NULL);
      }
      if ($mime = $item[$uri][rdf_qname_to_uri('dc:format')][0]) {
        // Preview is already generated.
        // All derived files should have a MIME type set.
        $file->previews[$mime][] = (isset($item[$uri][rdf_qname_to_uri('dc:creator')][0]) && ($creator = $item[$uri][rdf_qname_to_uri('dc:creator')][0])) ? $creator : 'no_handler';
      }
    }
  }

  // Create array of enabled handlers.
  $file->handlers_enabled = array_keys(array_filter(file_get_mime_handlers(), create_function('$a', 'return $a[\'enabled\'];')));

  // Find all preview generation handlers for the MIME type and generate previews if they are not already generated.
  $mime_types = file_get_mime_types();
  if (($mime_types = file_get_mime_types()) && isset($mime_types[$file->filemime]) && is_array($mime_types[$file->filemime]['handlers'])) {
    foreach (array_diff($mime_types[$file->filemime]['handlers'], isset($file->previews[$file->filemime]) ? $file->previews[$file->filemime] : array()) as $handler) {
      if (in_array($handler, $file->handlers_enabled) && function_exists($full_handler = $handler .'_generate'))
        $result = $full_handler($file) && $result;
    }
  }

  // We are done if the convert module is disabled.
  if (!module_exists('file_convert'))
    return $result;

  // Check size limit for the conversion.
  // There is no restrictions for the cron run.
  if (!isset($cron) && FILE_CONVERT_LIMIT_SIZE > 0 && $file->filesize > FILE_CONVERT_LIMIT_SIZE * 1024 * 1024)
    return $result;

  // Check if any of the converters is defined and enabled.
  $file->converters_enabled = array();
  foreach (file_get_mime_converters() as $from => $data) {
    $data = array_filter($data, create_function('$a', 'return $a[\'enabled\'];'));
    if (!empty($data))
      $file->converters_enabled[$from] = $data;
  }

  // Loop over the enebled converters for the MIME types. The conversion is implemented in analogy to streams,
  // i.e. the actual file conversion is done only on the first time it is needed.
  if (!empty($file->converters_enabled[$file->filemime])) {
    foreach ($file->converters_enabled[$file->filemime] as $to => $converter) {
      $file->converters = array($converter);
      $file->mimes = array($to);

      // Executes generation handlers.
      foreach (array_intersect($file->handlers_enabled, $converter['handlers']) as $handler) {
        if (function_exists($handler_generate = $handler .'_generate')) {
          $file->handlers = array($handler);
          $result = $handler_generate($file, 0) && $result;
        }
      }
      unset($file->handlers);

      // Save the file if there is no convertion handlers defined.
      if (empty($converter['handlers']) && !in_array('no_handler', isset($file->previews[$to]) ? $file->previews[$to]: array())) {
        if (file_get_converted($file, 0)) {
          if (!file_data_save($file, 0))
            $result = FALSE;
          watchdog('file', 'The %mime preview was generated for the file %name, uri=%uri.', array('%name' => $file->filename, '%mime' => $to, '%uri' => $file->uri), WATCHDOG_NOTICE, isset($file->nid) ? l(t('view'), 'node/'. $file->nid) : NULL);
        }
      }

      // Delete converted fie.
      if (isset($file->converted[0]) && isset($file->converted[0]->filepath))
        file_delete($file->converted[0]->filepath);
      unset($file->converted);
    }
  }
  return $result;
}

/**
 * Converts and saves a secondary file derivative.
 *
 * @param
 *   A file object.
 * @param
 *   A tier level.
 *
 * @return
 *   TRUE if the preview was generated, FALSE otherwise.
 */
function file_generate_secondary_previews($file, $tier) {
  if ($tier < 1 || $tier >= FILE_CONVERT_TIER_LIMIT)
    return FALSE;

  $result = FALSE;
  if (($handler_generate = $file->handlers[$tier] .'_generate') && function_exists($handler_generate) && ($converter = $file->converters_enabled[$file->mimes[$tier - 1]][$file->mimes[$tier]])) {
    $file->converters[$tier] = $converter;
    $result = $handler_generate($file, $tier);
    if (isset($file->converted[$tier]->filepath))
      file_delete($file->converted[$tier]->filepath);
    unset($file->converted[$tier]);
  }
  return $result;
}

/**
 * Recursively find or generate missing converted file.
 *
 * @param
 *   A file object.
 * @param
 *   A tier level.
 */
function file_get_converted($file, $tier) {
  // When $tier is negative we have reached the original file.
  if ($tier < 0 || isset($file->converted[$tier]))
    return TRUE;

  // Do we have converted file in the bitcache?
  if ($generated = rdf_normalize(rdf_query(NULL, rdf_qname_to_uri('dc:source'), $file->uri, array('repository' => FILE_RDF_REPOSITORY)))) {
    foreach ($generated as $uri => $data) {
      if (isset($file->handlers[$tier]) && rdf_normalize(rdf_query($uri, rdf_qname_to_uri('dc:creator'), $file->handlers[$tier], array('repository' => FILE_RDF_REPOSITORY)))) {
        $file->converted[$tier] = (object)array('uri' => $uri);
        $file->converted[$tier]->metadata = file_metadata_normalize(rdf_normalize(rdf_query($uri, NULL, NULL, array('repository' => FILE_RDF_REPOSITORY))));
        return TRUE;
      }
    }
  }

  // We cannot do anything more if file_convert is disabled.
  if (!module_exists('file_convert'))
    return FALSE;

  // Let's go one level up then and bring in the file to convert from.
  if (!file_get_converted($file, $tier - 1))
    return FALSE;

  // We have to convert the file.
  if ($converted = file_convert($file, $file->converters[$tier]['pipeline'], $tier, isset($file->handlers[$tier]) ? array($file->handlers[$tier]) : array(), isset($file->converters[$tier]['options']) ? $file->converters[$tier]['options'] : array())) {
    $file->converted[$tier] = (object)array('filepath' => $converted);
    $file->converted[$tier]->metadata = array_merge(file_metadata_extract($converted, $file->mimes[$tier]), isset($file->handlers[$tier]) ? array('dc:creator' => array($file->handlers[$tier])) : array());
    $file->converts++;
    return TRUE;
  }

  // Log that conversion has failed.
  if ($converted === FALSE) {
    $file->errors++;
    watchdog('file', 'File %name, uri=%uri was not converted to %mime by executing a pipeline %pipeline.', array('%name' => $file->filename, '%uri' => $file->uri, '%mime' => $file->mimes[$tier], '%pipeline' => $file->converters[$tier]['pipeline']), WATCHDOG_ERROR, isset($file->nid) ? l(t('view'), 'node/'. $file->nid) : NULL);
  }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
// File rendering & theming

/**
 * Generates an array of all  preview handlers for the file pointing.
 * to the previews' bitcache URIs.
 *
 * @param $file
 *   A file object as returned from file_node_load().
 *
 * @return
 *   An array of the preview handlers pointing to the bitcache URIs.
 */
function file_handlers_for($file) {
  $mime_types = file_get_mime_types();
  $result = array();

  // Find generated previews for the file.
  $previews = array();
  if (!empty($file->uri) && ($generated = rdf_normalize(rdf_query(NULL, NULL, $file->uri, array('repository' => FILE_RDF_REPOSITORY))))) {
    foreach ($generated as $uri => $data) {
      $item = rdf_normalize(rdf_query($uri, NULL, NULL, array('repository' => FILE_RDF_REPOSITORY)));
      if (isset($item[$uri][rdf_qname_to_uri('dc:creator')]) && ($handler = $item[$uri][rdf_qname_to_uri('dc:creator')][0]))
        $result[$handler] = array(
          'uri' => $uri,
          'type' => $item[$uri][rdf_qname_to_uri('dc:format')][0],
          'size' => $item[$uri][rdf_qname_to_uri('dc:extent')][0]->value,
        );
    }
  }

  // Display a file when preview is absent.
  if (isset($file->type) && isset($mime_types[$file->type]) && isset($mime_types[$file->type]['handlers'])) {
    foreach (array_diff($mime_types[$file->type]['handlers'], array_keys($result)) as $handler) {
      $result[$handler] = array(
        'uri' => $file->uri,
        'type' => $file->type,
        'size' => $file->size,
      );
    }
  }

  return $result;
}

/**
 * Generates an array of all file's generated derivatives URIs pointing
 * to the data of that derivative.
 *
 * @param $file
 *   A file object as returned from file_node_load().
 *
 * @return
 *   An array of the generated derivatives URIs pointing to derivatives' data.
 */
function file_generated_for($file) {
  $result = array();
  // Find generated previews for the file.
  if (!empty($file->uri) && ($generated = rdf_normalize(rdf_query(NULL, NULL, $file->uri, array('repository' => FILE_RDF_REPOSITORY))))) {
    foreach ($generated as $uri => $data) {
      $item = rdf_normalize(rdf_query($uri, NULL, NULL, array('repository' => FILE_RDF_REPOSITORY)));
      $result[$uri] = array(
        'type' => $item[$uri][rdf_qname_to_uri('dc:format')][0],
        'size' => $item[$uri][rdf_qname_to_uri('dc:extent')][0]->value,
        'handler' => isset($item[$uri][rdf_qname_to_uri('dc:creator')]) ? $item[$uri][rdf_qname_to_uri('dc:creator')][0] : NULL,
      );
    }
  }
  return $result;
}

/**
 * Returns the other file formats section.
 *
 * @param $file
 *   A file object as returned from file_node_load().
 * @param $id
 *   ID of the preview in case there are several previews on the same page.
 *
 * @return
 *   The HTML section of the file formats.
 */
function file_render_generated($file, $id = 0) {
  $formats = array();
  $file_uris = file_generated_for($file);
  foreach ($file_uris as $uri => $data) {
    if ($file->uri != $uri)
      $formats[] = array_merge($data, array(
        'uri' => $uri,
        'description' => file_mime_description_for($data['type']),
      ));
  }

  return theme('file_generated', array('formats' => $formats, 'nid' => $file->nid, 'vid' => $file->vid, 'name' => check_plain($file->name), 'id' => $id));
}

/**
 * Returns the controls section of the all file previews. The actual
 * preview is rendered later via the AHAH call.
 *
 * @param $file
 *   A file object as returned from file_node_load().
 * @param $id
 *   ID of the preview in case there are several previews on the same page.
 *
 * @return
 *   The HTML section of the file preview controls.
 */
function file_render_previews($file, $id = 0) {
  $previews = array();
  $file_handlers = file_handlers_for($file);
  foreach (file_get_mime_handlers() as $handler => $data) {
    if (array_key_exists($handler, $file_handlers) && $data['enabled'] == 1 && function_exists($handler .'_render')) {
      $form = file_render_preview_for($file, $handler, $id);
      $previews[] = array('handler' => $handler, 'name' => $data['name']);
    }
  }

  return theme('file_previews', array('previews' => $previews, 'nid' => $file->nid, 'name' => check_plain($file->name), 'id' => $id));
}

/**
 * Returns the form element which triggers the AHAH call to download
 * the file preview for particular handler.
 * This function ensures that AHAH extentions are added to the page.
 *
 * @param $file
 *   A file object as returned from file_node_load().
 * @param $handler
 *   The file preview handler.
 * @param $id
 *   ID of the preview in case there are several previews on the same page.
 *
 * @return
 *   The form element.
 */
function file_render_preview_for($file, $handler, $id) {
  $form['preview'] = array(
    '#type' => 'button',
    '#id' => $handler .'-'. $id,
    '#ahah' => array(
      'path' => 'file_preview/'. $handler .'/'. file_get_hash($file->uri) .'/'. $file->vid,
      'wrapper' => 'file-preview-container-'. $id,
      'method' => 'replace',
      'effect' => 'slide',
    ),
  );

  form_expand_ahah($form['preview']);
  return $form;
}

/**
 * Returns the HTML or JSON data of the rendered HTML preview of the file.
 * This functions is called from the file view page via AHAH call.
 *
 * @param $handler
 *   A preview handler associated with the file.
 * @param $uri
 *   A file uri.
 * @param $node
 *   A populated node object.
 * @param $options
 *   An array of additional options.
 * @param $type
 *   A return type. 'html' will output the html page, other values will result in javascript.
 *
 * @return
 *   A  or HTML or JSON data of the rendered HTML of the file preview.
 */
function file_wrapper($handler, $uri, $node = NULL, $options = array(), $type = NULL) {
  $output = '';
  if (array_key_exists($handler, file_get_mime_handlers()) && function_exists($handler .'_render')) {
    $handler_full = $handler .'_render';
    $output = $handler_full($uri, is_object($node) ? array_merge(array('nid' => isset($node->nid) ? $node->nid : NULL, 'vid' => isset($node->vid) ? $node->vid : NULL), $options) : $options);
  }
  if ($type == 'html')
    return $output;

  drupal_json(array('status' => TRUE, 'data' => theme('status_messages') . $output));
}

/**
 * Returns the HTML data of the rendered HTML preview of the file.
 *
 * @param $node
 *   A node object.
 * @param $handler
 *   A file handler to use for file rendering.
 * @param $options
 *   An array of additional options.
 * @param $cached
 *   A flag if the HTML will be displayed cached..
 *
 * @return
 *   A HTML data of the rendered HTML of the file preview.
 */
function file_wrapper_html($node, $handler, $options = array(), $cached = FALSE) {
  // Access control is done at the menu layer.
  if (!isset($handler) || $handler == 'file') {
    $node->file->name = $node->title;
    return theme('file_render', $node->file, $cached ? array('show' => array(), 'info' => array(), 'metadata' => array()) : array());
  }

  $handlers = file_handlers_for($node->file);
  return file_wrapper($handler, $handlers[$handler]['uri'], $node, $options, 'html');
}

/**
 * Returns the Javascript data of the rendered HTML preview of the file.
 *
 * @param $handler
 *   A preview handler associated with the file.
 * @param $hash
 *   A blob hash.
 *
 * @return
 *   A JAvascript data of the rendered HTML of the file preview.
 */
function file_wrapper_ahah($handler, $hash, $vid) {
  // Access control is not done at menu layer.
  if ($node = file_bitcache_access($hash, $vid)) {
    $handlers = file_handlers_for((object)array('uri' => bitcache_uri($hash)));
    if (empty($handlers[$handler]))
      $handlers[$handler]['uri'] = bitcache_uri($hash);

    return file_wrapper($handler, $handlers[$handler]['uri'], is_object($node) ? $node : '', array(), 'js');
  }
  //header("HTTP/1.1 403 Forbidden");
  drupal_access_denied();
}

/*
 * Finds a file first preview handler with parameters.
 *
 * @param $file
 *   A file object.
 *
 * @return
 *   An array with a preview parameters.
 */
function file_first_preview($file) {
  $previews = array();
  $file_handlers = file_handlers_for($file);
  foreach (file_get_mime_handlers() as $handler => $data) {
    if (array_key_exists($handler, $file_handlers) && $data['enabled'] == 1 && function_exists($handler .'_render'))
      $previews[] = $handler;
  }
  $handler = !empty($previews) ? reset($previews) : NULL;
  if (!isset($handler))
    return;

  $width = $height = 0;
  $handlers = file_get_mime_handlers();
  list($width, $height) = explode('x', $handlers[$handler]['dimensions']);
  $generated_found = FALSE;
  $generated = rdf_normalize(rdf_query(NULL, rdf_qname_to_uri('dc:source'), $file->uri, array('repository' => FILE_RDF_REPOSITORY)));
  $generated_found = FALSE;
  foreach ($generated as $uri => $data) {
    if (rdf_value($uri, rdf_qname_to_uri('dc:creator'), NULL, array('repository' => FILE_RDF_REPOSITORY)) == $handler) {
      $generated_found = TRUE;
      break;
    }
  }
  if (!$generated_found) {
    // We're displaying the file itself.
    $uri = $file->uri;
  }
  foreach (array('exif', 'wordnet') as $name) {
    $width += ($w = rdf_value($uri, rdf_qname_to_uri($name .':width'), NULL, array('repository' => FILE_RDF_REPOSITORY))) ? $w->value : 0;
    $height += ($h = rdf_value($uri, rdf_qname_to_uri($name .':height'), NULL, array('repository' => FILE_RDF_REPOSITORY))) ? $h->value : 0;
  }
  list($width_max, $height_max) = explode('x', FILE_POPUP_SIZE);
  $width = min($width, $width_max);
  $height = min($height, $height_max);

  return array('handler' => $handler, 'width' => $width, 'height' => $height);
}

//////////////////////////////////////////////////////////////////////////////
// Bitcache

/**
 * Implementation of hook_bitcache().
 */
function file_bitcache($op, $id, $stream = NULL) {
  $node = file_bitcache_access($id, isset($_GET['vid']) ? $_GET['vid'] : NULL);

  switch ($op) {
    case 'access':
      return is_object($node) ? TRUE : $node;
    case 'download':
      // Determine additional HTTP headers for the bitstream download
      $disposition = isset($_GET['disposition']) ? $_GET['disposition'] : 'attachment';
      if (isset($node->nid) && isset($_GET['op']) && in_array($_GET['op'], array('view', 'download'))) {
        $counter = $_GET['op'] .'s';
        db_query("UPDATE {file_nodes} f SET $counter = $counter + 1 WHERE f.vid = %d", $node->vid);
      }
      $extensions = file_mime_extensions_for(rdf_value(bitcache_uri($id), rdf_qname_to_uri('dc:format'), NULL, array('repository' => FILE_RDF_REPOSITORY)));
      foreach ($extensions as $extension) {
        if (preg_match('/\.'. $extension .'$/', drupal_strtolower($node->title))) {
          $extension_correct = TRUE;
          break;
        }
      }
      $ext = !empty($extensions) && !isset($extension_correct) ? '.'. reset($extensions) : '';
      $title = isset($node->title) ? $node->title : 'file';
      $mime = rdf_value(bitcache_uri($id), rdf_qname_to_uri('dc:format'), NULL, array('repository' => FILE_RDF_REPOSITORY));
      // http://www.adobe.com/devnet/flashplayer/articles/fplayer10_security_changes_02.html#head32
      $headers = ($mime == 'application/x-shockwave-flash') ? array() : array('Content-Disposition' => $disposition .'; filename="'. $title . $ext .'"');
      return array_merge($headers, array(
        'Content-Type' => $mime,
        'Content-Length' => rdf_value(bitcache_uri($id), rdf_qname_to_uri('dc:extent'), NULL, array('repository' => FILE_RDF_REPOSITORY))->value,
        'Last-Modified' => gmdate('D, d M Y H:i:s', isset($node->changed) ? $node->changed : time()) .' GMT',
      ));
      break;
  }
}

/**
 * Returns the hash of the file stored in the bitcache.
 *
 * @param $file
 *   A file object as returned from file_node_load().
 *
 * @return
 *   A file hash.
 */
function file_get_hash($file) {
  return is_object($file) ? bitcache_uri_to_id($file->uri) : bitcache_uri_to_id($file);
}

/**
 * Checks if the user has rights to access bitsream.
 *
 * @param $id
 *   Bitstream hash.
 * @param $vid
 *   A node version ID.
 *
 * @return
 *   A node object or FALSE if access is not granted. NULL means that file framework
 *     does not make the access decision and passes it back to the bitcache.
 */
function file_bitcache_access($id, $vid = NULL) {
  $uri = bitcache_uri($id);
  $parent_uri = rdf_value($uri, rdf_qname_to_uri('dc:source'), NULL, array('repository' => FILE_RDF_REPOSITORY));

  if (!empty($vid)) {
    $row = db_fetch_object(db_query("SELECT f.nid, f.vid FROM {file_nodes} f WHERE f.uri = '%s' AND f.vid = %d", $uri, $vid));
    if (!is_object($row))
      $row = db_fetch_object(db_query("SELECT f.nid, f.vid FROM {file_nodes} f WHERE f.uri = '%s' AND f.vid = %d", $parent_uri, $vid));
    if (is_object($row)) {
      $node = node_load($row->nid, $row->vid);
      return node_access('view', $node) ? $node : FALSE;
    }
  }

  $files = array_merge(
    isset($_SESSION['file_preview_file']) && is_object($_SESSION['file_preview_file']) ? array($_SESSION['file_preview_file']) : array(),
    isset($_SESSION['file_attach_files']) && is_array($_SESSION['file_attach_files']) ? $_SESSION['file_attach_files'] : array(),
    isset($_SESSION['file_cck_files']) && is_array($_SESSION['file_cck_files']) ? $_SESSION['file_cck_files'] : array()
  );

  foreach ($files as $file) {
    if (is_object($file) && $file->uri == $parent_uri || $file->uri == $uri)
      return (object)array('title' => $file->name, 'file' => $file);
  }

  return NULL;
}

//////////////////////////////////////////////////////////////////////////////
// File upload validators

/**
 * Gets all file upload validators.
 *
 * @return
 *   An array of validate functions.
 */
function file_get_validators() {
  static $validators = array();
  if (empty($validators)) {
    foreach (module_implements('file_validate') as $module) {
      $validators[$module .'_file_validate'] = array();
    }
    $validators = is_array($validators) ? $validators : array();
  }
  return $validators;
}

//////////////////////////////////////////////////////////////////////////////
// File quotas

/**
 * Calculates total size of all user's files.
 *
 * @param
 *   A user ID.
 * @param
 *   A flag which shows if converted files should be included in the calculations.
 *
 * @return
 *   Size in bytes.
 */
function file_get_files_size($uid, $converted = 0) {
  if (!$converted) {
    $row = db_fetch_object(db_query('SELECT SUM(fn.size) AS size FROM {file_nodes} fn INNER JOIN {node} n ON fn.nid = n.nid WHERE n.uid = %d', $uid));
    return !empty($row->size) ? $row->size : 0;
  }

  $size = 0;
  $result = db_query('SELECT fn.size, fn.uri FROM {file_nodes} fn INNER JOIN {node} n ON fn.nid = n.nid WHERE n.uid = %d', $uid);
  while ($row = db_fetch_object($result)) {
    $size += $row->size;

    if ($generated = rdf_normalize(rdf_query(NULL, rdf_qname_to_uri('dc:source'), $row->uri, array('repository' => FILE_RDF_REPOSITORY)))) {
      foreach ($generated as $uri => $data) {
        $size += rdf_value($uri, rdf_qname_to_uri('dc:extent'), NULL, array('repository' => FILE_RDF_REPOSITORY))->value;
      }
    }
  }
  return $size;
}

//////////////////////////////////////////////////////////////////////////////
// Auxiliary functions

/**
 * Normalizes metadata array returned by rdf_normalize().
 *
 * @param $dataa
 *   An array returned by rdf_normalize()..
 *
 * @return
 *   An array of normalized metadata.
 */
function file_metadata_normalize($data) {
  $metadata = array();
  if (!is_array($data))
    return $metadata;

  $namespaces = rdf_get_namespaces();
  foreach ($data as $subject => $predicates) {
    foreach ($predicates as $predicate => $objects) {
      $predicate_normalized = rdf_uri_to_qname($predicate, $namespaces, FALSE);
      foreach ($objects as $object) {
        $metadata[$subject][$predicate_normalized][] = is_object($object) ? $object->value : $object;
      }
    }
  }
  return isset($subject) && isset($metadata[$subject]) ? $metadata[$subject] : array();
}

/**
 * Compares data with the template array and gets the matching entry.
 * Is needed by format modules.
 *
 * @param $data
 *   Data array.
 * @param $template
 *   Template array.
 *
 * @return
 *   A data which was matched in the template.
 */
function file_get_recursive($data, $template) {
  while (is_array($data) && is_array($template)) {
    $key = key($template);
    $template = $template[$key];
    $data = isset($data[$key]) ? $data[$key] : NULL;
  }
  return isset($data) && !is_array($data) ? $data : NULL;
}

/**
 * Finds an image of a given resolution of the file if it is available.
 *
 * @param $file
 *   A file object.
 * @param $handler
 *   An image handler.
 * @param $dimensions
 *   An array with max width and height of the image.
 *
 * @return
 *   A HTML output of the image or NULL if it is not available.
 */
function file_get_image($file, $handler, $dimensions = array(0, 0)) {
  if ($generated = rdf_normalize(rdf_query(NULL, rdf_qname_to_uri('dc:source'), $file->uri, array('repository' => FILE_RDF_REPOSITORY)))) {
    foreach ($generated as $uri => $data) {
      if (rdf_value($uri, rdf_qname_to_uri('dc:creator'), NULL, array('repository' => FILE_RDF_REPOSITORY)) == $handler) {
        // We gave a generated image.
        $image = file_get_hash($uri);
        $width = ($w = rdf_value($uri, rdf_qname_to_uri('exif:width'), NULL, array('repository' => FILE_RDF_REPOSITORY))) ? $w->value : 0;
        $height = ($h = rdf_value($uri, rdf_qname_to_uri('exif:height'), NULL, array('repository' => FILE_RDF_REPOSITORY))) ? $h->value : 0;
        break;
      }
    }
  }
  if (!isset($image) && preg_match('/^image\//', $file->type)) {
    $width = ($w = rdf_value($file->uri, rdf_qname_to_uri('exif:width'), NULL, array('repository' => FILE_RDF_REPOSITORY))) ? $w->value : 0;
    $height = ($h = rdf_value($file->uri, rdf_qname_to_uri('exif:height'), NULL, array('repository' => FILE_RDF_REPOSITORY))) ? $h->value : 0;
    if ($width > 0 && $width < $dimensions[0] && $height > 0 && $height < $dimensions[1])
      $image = file_get_hash($file->uri);
  }
  return isset($image) ? '<img width="'. $width .'" height="'. $height .'" src="'. bitcache_resolve_id($image, array('absolute' => FALSE, 'query' => array('vid' => $file->vid, 'disposition' => 'inline'))) .'">' : NULL;
}

/**
 * Propagates og groups to the file node.
 *
 * @param $node
 *   A node object.
 * @param $file_node
 *   A file node object.
 *
 * @return
 */
function _file_propagate_og($node, $file_node) {
  if (!module_exists('og'))
    return FALSE;

  global $user;
  $file_node->og_groups = array_unique(array_merge(isset($file_node->og_groups) ? $file_node->og_groups : array(), isset($node->og_groups) && !empty($node->og_groups) ? array_intersect($node->og_groups, array_keys($user->og_groups)) : array()));
  if (isset($node->og_public) || isset($file_node->og_public))
    $file_node->og_public = isset($node->og_public) ? $node->og_public : $file_node->og_public;
  og_save_ancestry($file_node);
}

/**
 * Propagates taxonomy terms to the file node.
 *
 * @param $node
 *   A node object.
 * @param $file_node
 *   A file node object.
 * @param $settings
 *   An array of settings defining which vocabs should be propagated.
 *
 * @return
 */
function _file_propagate_terms($node, $file_node, $settings = array()) {
  if (!empty($node->taxonomy)) {
    global $user;

    // Retrieve the list of vocabulary IDs from which we'll propagate
    // taxonomy terms associated with the containing parent node to any
    // contained file attachment nodes:

    $vocabularies = taxonomy_get_vocabularies('file');
    $og_vocabs = $og_vocabs_hidden = array();
    if (module_exists('og_vocab')) {
      $result = db_query('SELECT nid, vid FROM {og_vocab}');
      while ($row = db_fetch_object($result)) {
        if (in_array($row->nid, array_keys($user->og_groups)))
          $og_vocabs[] = $row->vid;
        else
          $og_vocabs_hidden[] = $row->vid;
      }
    }

    $vocabs = array_diff($settings['all'] ? array_keys($vocabularies) : array_unique(array_merge(array_keys($settings['vocabs']), $settings['og'] ? $og_vocabs : array())), $og_vocabs_hidden);

    if (!empty($vocabs)) {

      // Prepare the taxonomy to propagate.
      $taxonomy = is_array($node->taxonomy) ? $node->taxonomy : array();
      if (is_array($taxonomy['tags'])) {
        foreach ($taxonomy['tags'] as $vid => $data) {
          if (!in_array($vid, $vocabs))
            unset($taxonomy['tags'][$vid]);
        }
      }
      foreach ($taxonomy as $vid => $data) {
        if (is_numeric($vid) && !in_array($vid, $vocabs))
          unset($taxonomy[$vid]);
      }

      if (!empty($taxonomy)) {
        // We have to save the taxonomy first to get rid of the dublicates and get tids
        // for a free tags. taxonomy_node_save() deletes old terms, so we have to make
        // a copy first.
        $terms = taxonomy_node_get_terms($file_node);
        taxonomy_node_save($file_node, $taxonomy);

        // Creates a union of the old and propagated tags. Can't use taxonomy_node_get_terms()
        // any more because it has already cached the terms for this node ID.
        $result = db_query('SELECT tid FROM {term_node} WHERE nid = %d AND vid = %d', $file_node->nid, $file_node->vid);
        while ($row = db_fetch_object($result)) {
          $terms[$row->tid] = $row->tid;
        }
        taxonomy_node_save($file_node, $terms);
      }
    }
  }
}

