<?php
// $Id: cache_browser.pages.inc,v 1.1.2.5 2009/02/23 12:18:24 markuspetrux Exp $

/**
 * @file
 * Implements the cache browser page and related actions.
 */

/**
 * Menu callback; Cache browser page.
 *
 * @param string $cache_table
 * @param string $action
 */
function cache_browser_page($cache_table = NULL, $action = NULL) {
  // List all cache tables when no one has been selected.
  if (!isset($cache_table)) {
    return drupal_get_form('cache_browser_page_list_tables');
  }

  // Validate user supplied cache table name.
  $cache_tables = cache_browser_get_tables();
  if (!isset($cache_tables[$cache_table])) {
    drupal_not_found();
    return;
  }

  // List all cache entries for specified table when no entry has been selected.
  if (!isset($_GET['cid'])) {
    return drupal_get_form('cache_browser_page_list_entries', $cache_table);
  }

  // Display specified cache entry.
  return cache_browser_page_display_entry($cache_table, $_GET['cid']);
}

/**
 * List cache tables form.
 */
function cache_browser_page_list_tables() {
  $form = array();
  $form['keys'] = array(
    '#type' => 'checkboxes',
    '#options' => array(),
  );
  $form['tables'] = array(
    '#type' => 'value',
    '#value' => cache_browser_get_tables(),
  );
  foreach ($form['tables']['#value'] as $table => $table_info) {
    if ($table_info->entries > 0) {
      $form['keys']['#options'][$table] = '';
    }
  }
  if (!empty($form['keys']['#options'])) {
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Flush selected tables'),
    );
  }
  return $form;
}

/**
 * Theme list cache tables form.
 *
 * @ingroup themeable
 */
function theme_cache_browser_page_list_tables($form) {
  drupal_add_css(drupal_get_path('module', 'cache_browser') .'/cache_browser.css', 'module', 'all', FALSE);
  drupal_add_js('misc/tableselect.js');
  $report_header = array(
    array('data' => t('Table'), 'class' => 'cache-browser-cell-text'),
    array('data' => t('Description'), 'class' => 'cache-browser-cell-text'),
    array('data' => t('Entries'), 'class' => 'cache-browser-cell-numeric'),
    array('data' => t('Size (KB)'), 'class' => 'cache-browser-cell-numeric'),
    array('data' => t('Last updated'), 'class' => 'cache-browser-cell-date'),
    array('data' => '', 'class' => 'select-all cache-browser-cell-action'),
  );
  $report_rows = array();
  $cache_size = 0;
  foreach ($form['tables']['#value'] as $table => $table_info) {
    $cache_size += $table_info->size;
    $report_rows[] = array(
      array(
        'data' => l($table, CACHE_BROWSER_PAGE_PATH .'/'. $table),
        'class' => 'cache-browser-cell-text',
      ),
      array(
        'data' => check_plain($table_info->description),
        'class' => 'cache-browser-cell-text',
      ),
      array(
        'data' => format_number($table_info->entries),
        'class' => 'cache-browser-cell-numeric',
      ),
      array(
        'data' => format_number($table_info->size / 1024, 2),
        'title' => format_number($table_info->size) .' '. t('bytes'),
        'class' => 'cache-browser-cell-numeric',
      ),
      array(
        'data' => (!empty($table_info->max_created) ? format_date($table_info->max_created, 'custom', 'Y-m-d H:i:s') : t('N/A')),
        'class' => 'cache-browser-cell-date',
      ),
      array(
        'data' => (isset($form['keys'][$table]) ? drupal_render($form['keys'][$table]) : t('N/A')),
        'class' => 'cache-browser-cell-action',
      ),
    );
  }
  $report_rows_count = count($report_rows);
  if ($report_rows_count <= 0) {
    $report_rows[] = array(array('colspan' => count($report_header), 'align' => 'center', 'data' => t('No cache tables found.')));
  }
  $output = theme('cache_browser_page_header', array(
    'size' => $cache_size,
    'items' => $report_rows_count,
  ));
  $output .= theme('table', $report_header, $report_rows);
  if ($report_rows_count > 0) {
    $output .= '<div class="cache-browser-submit">'. drupal_render($form['submit']) .'</div>';
  }
  $output .= theme('pager', NULL, CACHE_BROWSER_ITEMS_PER_PAGE);
  $output .= drupal_render($form);
  return $output;
}

/**
 * Validate handler for the list cache tables form.
 */
function cache_browser_page_list_tables_validate($form, &$form_state) {
  $form_state['values']['keys'] = array_filter($form_state['values']['keys']);
  if (count($form_state['values']['keys']) == 0) {
    form_set_error('', t('No tables selected.'));
  }
}

/**
 * Submit handler for the list cache tables form.
 */
function cache_browser_page_list_tables_submit($form, &$form_state) {
  foreach (array_keys($form_state['values']['keys']) as $cache_table) {
    cache_clear_all('*', $cache_table, TRUE);
  }
  drupal_set_message(t('The selected cache tables have been cleared.'));
}

/**
 * List cache entries form.
 */
function cache_browser_page_list_entries(&$form_state, $cache_table) {
  $form = array();
  $form['keys'] = array(
    '#type' => 'checkboxes',
    '#options' => array(),
  );
  $form['entries'] = array(
    '#type' => 'value',
    '#value' => array(),
  );
  $form['cache_table'] = array(
    '#type' => 'value',
    '#value' => $cache_table,
  );
  $result = pager_query(
    'SELECT cid, created, expire, LENGTH(data) AS size FROM {'. $cache_table .'} ORDER BY cid',
    CACHE_BROWSER_ITEMS_PER_PAGE, 0,
    'SELECT COUNT(*) FROM {'. $cache_table .'}'
  );
  while ($entry = db_fetch_object($result)) {
    $form['keys']['#options'][$entry->cid] = '';
    $form['entries']['#value'][] = $entry;
  }
  if (!empty($form['keys']['#options'])) {
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Delete selected entries'),
    );
  }
  return $form;
}

/**
 * Theme list cache entries form.
 *
 * @ingroup themeable
 */
function theme_cache_browser_page_list_entries($form) {
  drupal_add_css(drupal_get_path('module', 'cache_browser') .'/cache_browser.css', 'module', 'all', FALSE);
  drupal_add_js('misc/tableselect.js');
  $cache_table = $form['cache_table']['#value'];
  $report_header = array(
    array('data' => t('Entry ID'), 'class' => 'cache-browser-cell-text'),
    array('data' => t('Size (KB)'), 'class' => 'cache-browser-cell-numeric'),
    array('data' => t('Created'), 'class' => 'cache-browser-cell-date'),
    array('data' => t('Expires'), 'class' => 'cache-browser-cell-date'),
    array('data' => '', 'class' => 'select-all cache-browser-cell-action'),
  );
  $report_rows = array();
  $cache_size = 0;
  foreach ($form['entries']['#value'] as $entry) {
    $cache_size += $entry->size;
    $report_rows[] = array(
      array(
        'data' => l(cache_browser_trim_cid($cache_table, $entry->cid), CACHE_BROWSER_PAGE_PATH .'/'. $cache_table .'/view', array('query' => 'cid='. $entry->cid)),
        'title' => $entry->cid,
        'class' => 'cache-browser-cell-text',
      ),
      array(
        'data' => format_number($entry->size / 1024, 2),
        'title' => format_number($entry->size) .' '. t('bytes'),
        'class' => 'cache-browser-cell-numeric',
      ),
      array(
        'data' => format_date($entry->created, 'custom', 'Y-m-d H:i:s'),
        'class' => 'cache-browser-cell-date',
      ),
      array(
        'data' => cache_browser_format_expire_date($entry->expire),
        'class' => 'cache-browser-cell-date',
      ),
      array(
        'data' => drupal_render($form['keys'][$entry->cid]),
        'class' => 'cache-browser-cell-action',
      ),
    );
  }
  $report_rows_count = count($report_rows);
  if ($report_rows_count <= 0) {
    $report_rows[] = array(array('colspan' => count($report_header), 'align' => 'center', 'data' => t('No cache entries found.')));
  }
  $output = theme('cache_browser_page_header', array(
    'size' => $cache_size,
    'items' => $report_rows_count,
    'table' => $cache_table,
  ));
  $output .= theme('table', $report_header, $report_rows);
  if ($report_rows_count > 0) {
    $output .= '<div class="cache-browser-submit">'. drupal_render($form['submit']) .'</div>';
  }
  $output .= theme('pager', NULL, CACHE_BROWSER_ITEMS_PER_PAGE);
  $output .= drupal_render($form);
  return $output;
}

/**
 * Validate handler for the list cache entries form.
 */
function cache_browser_page_list_entries_validate($form, &$form_state) {
  $form_state['values']['keys'] = array_filter($form_state['values']['keys']);
  if (count($form_state['values']['keys']) == 0) {
    form_set_error('', t('No entries selected.'));
  }
}

/**
 * Submit handler for the list cache entries form.
 */
function cache_browser_page_list_entries_submit($form, &$form_state) {
  $cache_table = $form_state['values']['cache_table'];
  foreach (array_keys($form_state['values']['keys']) as $cid) {
    cache_clear_all($cid, $cache_table);
  }
  drupal_set_message(t('The selected cache entries have been deleted.'));
}

/**
 * View contents of the given cache entry.
 *
 * @param string $cache_table
 * @param string $cid
 */
function cache_browser_page_display_entry($cache_table, $cid) {
  drupal_add_css(drupal_get_path('module', 'cache_browser') .'/cache_browser.css', 'module', 'all', FALSE);

  $cache = cache_browser_cache_get($cid, $cache_table);
  if (!isset($cache->data)) {
    drupal_not_found();
    return;
  }

  $page_header_info = array(
    'table' => $cache_table,
    'cid' => $cid,
  );
  $output = theme('cache_browser_page_header', $page_header_info);

  // Uncompress data?
  if ($cache_table == 'cache_page' && !empty($cache->data) && variable_get('page_compression', TRUE) && function_exists('gzencode')) {
    // Assume data is compressed when first bytes are Gzip ID1 + ID2.
    // See http://www.gzip.org/zlib/rfc-gzip.html
    if ($cache->data[0] == "\x1f" && $cache->data[1] == "\x8b") {
      $cache->data = gzinflate(substr(substr($cache->data, 10), 0, -8));
    }
  }

  // Display cache entry information.
  $output .= cache_browser_dump_cache_entry($cache);

  // Dump cached headers.
  if (!empty($cache->headers)) {
    $output .= cache_browser_dump_headers($cache->headers);
  }

  // Dump cached data.
  $output .= cache_browser_dump_data($cache->data, $cid);

  return $output;
}

/**
 * Obtain list of cache tables.
 *
 * @return array
 *   Hash of cache tables where key is cache table name and elements are
 *   objects with the following attributes:
 *
 *     description   string  Table description obtained from database schema.
 *     max_created   int     Timestamp of the most recently created cache entry.
 *     size          int     Sum of the size in bytes of all cached data.
 *     entries       int     Number of cache entries in the table.
 *
 *   The hash is ordered by cache table name.
 */
function cache_browser_get_tables() {
  global $db_type;
  static $cache_tables;
  if (!isset($cache_tables)) {
    // Obtain cache tables from system schema.
    $schema_tables = array();
    if ($schema = drupal_get_schema()) {
      foreach ($schema as $table => $table_info) {
        if (strpos($table, 'cache') === 0) {
          $schema_tables[$table] = $table_info['description'];
        }
      }
    }
    ksort($schema_tables);

    // Load table statistics from database.
    $cache_tables = array();
    foreach ($schema_tables as $table => $description) {
      $cache_tables[$table] = db_fetch_object(db_query('SELECT MAX(created) AS max_created, SUM(LENGTH(data)) AS size, COUNT(*) AS entries FROM {'. $table .'}'));
      $cache_tables[$table]->description = $description;
    }
  }
  return $cache_tables;
}

/**
 * Shorten cid when it is too long.
 *
 * @param string $cache_table
 * @param string $cid
 * @param int $max_length
 */
function cache_browser_trim_cid($cache_table, $cid, $max_length = NULL) {
  global $base_url;
  if (!isset($max_length)) {
    $max_length = CACHE_BROWSER_MAX_CID_LENGTH;
  }
  if (drupal_strlen($cid) > $max_length) {
    if ($cache_table == 'cache_page' && strpos($cid, $base_url) === 0) {
      $cid = (isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] == 'on' ? 'https' : 'http') .'://...'. drupal_substr($cid, drupal_strlen($base_url));
      if (drupal_strlen($cid) > $max_length) {
        $cid = drupal_substr($cid, 0, $max_length-3) .'...';
      }
    }
    else {
      $cid = drupal_substr($cid, 0, $max_length-3) .'...';
    }
  }
  return $cid;
}

/**
 * Format a cache expiration date.
 *
 * @param int $expire
 * @return string
 */
function cache_browser_format_expire_date($expire) {
  if ($expire == CACHE_PERMANENT) {
    return t('Permanent');
  }
  if ($expire == CACHE_TEMPORARY) {
    return t('Temporary');
  }
  $output = format_date($expire, 'custom', 'Y-m-d H:i:s');
  if ($expire <= time()) {
    $output = '<span class="cache-browser-expired">'. $output .'</span>';
  }
  return $output;
}

/**
 * Obtain data from the persistent cache.
 *
 * This function is a safe wrapper around Drupal's cache_get().
 * We wish to obtain data even if it is expired, and we don't wish to alter
 * normal cache expiration schema removing stale data as cache_get() does.
 */
function cache_browser_cache_get($cid, $table) {
  // Views classes support. Any better wat to achieve this?
  if ($table == 'cache_views' && function_exists('views_get_all_views')) {
    views_get_all_views();
  }

  $cache = db_fetch_object(db_query("SELECT data, created, headers, expire, serialized FROM {". $table ."} WHERE cid = '%s'", $cid));
  if (isset($cache->data)) {
    $cache->data = db_decode_blob($cache->data);
    if ($cache->serialized) {
      $cache->data = unserialize($cache->data);
    }
    return $cache;
  }
  return 0;
}

/**
 * Dump cache headers.
 */
function cache_browser_dump_cache_entry($cache) {
  $info = array();
  $info[] = '<strong>'. t('Created') .'</strong>: '. format_date($cache->created, 'custom', 'Y-m-d H:i:s');
  $info[] = '<strong>'. t('Expires') .'</strong>: '. cache_browser_format_expire_date($cache->expire);
  $info[] = '<strong>'. t('Size') .'</strong>: '. format_number(strlen($cache->serialized ? serialize($cache->data) : $cache->data)) .' '. t('bytes');
  return '<h3 class="cb-dump-title">'. t('Cache entry') .':</h3><pre class="cb-dump">'. implode("\n", $info) .'</pre>';
}

/**
 * Dump cache headers.
 */
function cache_browser_dump_headers($headers) {
  $headers = '<em class="cb-dump-headers">'. check_plain($headers) .'</em>';
  return '<h3 class="cb-dump-title">'. t('Cached headers') .':</h3><pre class="cb-dump">'. $headers .'</pre>';
}

/**
 * Dump data.
 */
function cache_browser_dump_data(&$data, $name = NULL, $parent_type = '', $level = 0) {
  static $instance_hotspot_count, $global_dumps_count, $global_hotspot_count, $js_ready, $icon_minus, $icon_plus;
  if (!isset($global_hotspot_count)) {
    $global_dumps_count = 0;
    $global_hotspot_count = 0;
    $js_ready = FALSE;
    $module_path = drupal_get_path('module', 'cache_browser');
    $icon_minus = url($module_path .'/images/icon_minus.gif');
    $icon_plus = url($module_path .'/images/icon_plus.gif');
  }
  if ($level == 0) {
    $instance_hotspot_count = 0;
  }

  $output = str_repeat(' ', $level * 2);

  if (isset($name)) {
    $formatted_name = $name;
    if ($parent_type == 'object') {
      $formatted_name = 'var <em class="cb-dump-varname">$'. htmlspecialchars($formatted_name) .'</em> =';
    }
    else {
      if ($parent_type == 'array' && is_string($formatted_name)) {
        $formatted_name = '\''. str_replace('\'', '\\\'', htmlspecialchars($formatted_name)) .'\'';
      }
      $formatted_name = '<em class="cb-dump-hash">'. htmlspecialchars($formatted_name) .'</em> =&gt;';
    }
  }
  else {
    $formatted_name = '';
  }

  if (is_array($data)) {
    $items_count = count($data);
    if (!isset($data['__cache_browser_usage_counter'])) {
      $data['__cache_browser_usage_counter'] = 0;
    }
    $formatted_type = ($data['__cache_browser_usage_counter'] > 0 ? '<em class="cb-dump-constant">&amp;</em>' : '') .'array('. $items_count .')';
    if ($data['__cache_browser_usage_counter'] > 1) {
      $output .= $formatted_name .' '. $formatted_type .' (<em class="cb-dump-constant">*'. t('RECURSION') .'*</em>)'."\n";
      return $output;
    }
    $data['__cache_browser_usage_counter']++;
    $output .= $formatted_name .' '. $formatted_type .' (';
    if ($items_count > 0) {
      $next_level = $level + 1;
      if ($next_level > CACHE_BROWSER_MAX_NESTING_LEVEL) {
        $output .= '<em class="cb-dump-constant">*'. t('NESTING-TOO-DEEP') .'*</em>';
      }
      else {
        if ($level > 0) {
          $output .= '<img src="'. $icon_plus .'" class="cb-dump-hotspot-'. $global_dumps_count .'" id="cb-dump-hotspot-'. $global_hotspot_count .'" alt="" />';
          $output .= '<span class="cb-dump-block-'. $global_dumps_count .'" id="cb-dump-block-'. $global_hotspot_count .'">';
          $global_hotspot_count++;
          $instance_hotspot_count++;
        }
        $output .= "\n";
        foreach ($data as $key => $value) {
          if ($key !== '__cache_browser_usage_counter') {
            $output .= cache_browser_dump_data($data[$key], $key, 'array', $next_level);
          }
        }
        unset($key, $value);
        $output .= str_repeat(' ', $level * 2);
        if ($level > 0) {
          $output .= '</span>';
        }
      }
    }
    $output .= ')'. ($level > 0 ? ',' : ';') ."\n";
    unset($data['__cache_browser_usage_counter']);
  }
  else if (is_object($data)) {
    $array = (array)$data;
    $items_count = count($array);
    unset($array);
    if (!isset($data->__cache_browser_usage_counter)) {
      $data->__cache_browser_usage_counter = 0;
    }
    $formatted_type = ($data->__cache_browser_usage_counter > 0 ? '<em class="cb-dump-constant">&amp;</em>' : '') .'class';
    $formatted_type .= ' <em class="cb-dump-varname">'. htmlspecialchars(get_class($data)) .'</em>('. $items_count .')';
    if ($data->__cache_browser_usage_counter > 1) {
      $output .= $formatted_name .' '. $formatted_type .' {<em class="cb-dump-constant">*'. t('RECURSION') .'*</em>}'."\n";
      return $output;
    }
    $data->__cache_browser_usage_counter++;
    $output .= $formatted_name .' '. $formatted_type .' {';
    if ($items_count > 0) {
      $next_level = $level + 1;
      if ($next_level > CACHE_BROWSER_MAX_NESTING_LEVEL) {
        $output .= '<em class="cb-dump-constant">*'. t('NESTING-TOO-DEEP') .'*</em>';
      }
      else {
        if ($level > 0) {
          $output .= '<img src="'. $icon_plus .'" class="cb-dump-hotspot-'. $global_dumps_count .'" id="cb-dump-hotspot-'. $global_hotspot_count .'" alt="" />';
          $output .= '<span class="cb-dump-block-'. $global_dumps_count .'" id="cb-dump-block-'. $global_hotspot_count .'">';
          $global_hotspot_count++;
          $instance_hotspot_count++;
        }
        $output .= "\n";
        foreach ($data as $key => $value) {
          if ($key !== '__cache_browser_usage_counter') {
            $output .= cache_browser_dump_data($value, $key, 'object', $next_level);
          }
        }
        unset($key, $value);
        $output .= str_repeat(' ', $level * 2);
        if ($level > 0) {
          $output .= '</span>';
        }
      }
    }
    $output .= '}'. ($level > 0 ? ',' : ';') ."\n";
    unset($data->__cache_browser_usage_counter);
  }
  else {
    if (is_string($data)) {
      $formatted_data = '<em class="cb-dump-string">\''. str_replace('\'', '\\\'', htmlspecialchars($data)) .'\'</em>';
    }
    else if (is_null($data) || is_bool($data)) {
      $formatted_data = '<em class="cb-dump-constant">'. (is_null($data) ? 'NULL' : ($data ? 'TRUE' : 'FALSE')) .'</em>';
    }
    else if (is_numeric($data)) {
      $formatted_data = '<em class="cb-dump-numeric">'. htmlspecialchars((string)$data) .'</em>';
    }
    else if (get_class($data) !== FALSE) {
      // Catch up __PHP_Incomplete_Class issues. ie. we got an object of an still undefined class.
      $formatted_data = 'class <em class="cb-dump-constant">'. htmlspecialchars(get_class($data)) .'</em> {}';
    }
    else {
      $formatted_data = htmlspecialchars($data);
    }
    $output .= $formatted_name .' '. $formatted_data . (!empty($parent_type) ? ',' : '') ."\n";
  }

  if ($level == 0) {
    if ($instance_hotspot_count > 0) {
      $header_icons = '<div class="cb-dump-title-right">'
          .'<a href="javascript:void(0)" class="cb-dump-hotspot-expand" id="cb-dump-hotspot-expand-'. $global_dumps_count .'"><img src="'. $icon_plus .'" alt="" /> '. t('Expand all') .'</a>'
          .'&nbsp;&nbsp;'
          .'<a href="javascript:void(0)" class="cb-dump-hotspot-collapse" id="cb-dump-hotspot-collapse-'. $global_dumps_count .'"><img src="'. $icon_minus .'" alt="" /> '. t('Collapse all') .'</a>'
        .'</div>';
      if (!$js_ready) {
        $js_ready = TRUE;
        drupal_add_js('cache_browser = '. drupal_to_js(array('icon_plus' => $icon_plus, 'icon_minus' => $icon_minus)) .';', 'inline');
        // Add our custom JS script (do not defer, can be cached, do not preprocess).
        drupal_add_js(drupal_get_path('module', 'cache_browser') .'/cache_browser.js', 'module', 'header', FALSE, TRUE, FALSE);
      }
      $global_dumps_count++;
    }
    else {
      $header_icons = '';
    }
    return '<div class="clear-block"><h3 class="cb-dump-title-left">'. t('Data dump') .':</h3>'. $header_icons .'</div><pre class="cb-dump">'. $output .'</pre>';
  }

  return $output;
}

/**
 * Render cache browser header.
 *
 * @ingroup themeable
 */
function theme_cache_browser_page_header($page_header_info) {
  $header_left_pane = '<a href="'. url(CACHE_BROWSER_PAGE_PATH) .'">'. t('Cache') .'</a>';
  if (isset($page_header_info['table'])) {
    $header_left_pane .= ' -&gt; <a href="'. url(CACHE_BROWSER_PAGE_PATH .'/'. $page_header_info['table']) .'">'. check_plain($page_header_info['table']) .'</a>';
    if (isset($page_header_info['cid'])) {
      $header_left_pane .= ' -&gt; <a href="'. url(CACHE_BROWSER_PAGE_PATH .'/'. $page_header_info['table'] .'/view', array('query' => 'cid='. $page_header_info['cid'], 'attributes' => array('title' => $page_header_info['cid']))) .'">'. check_plain(cache_browser_trim_cid($page_header_info['table'], $page_header_info['cid'])) .'</a>';
    }
  }

  $extra = array();
  if (isset($page_header_info['items'])) {
    $extra[] = (!isset($page_header_info['table']) ? t('Tables') : t('Items')) .': '. format_number($page_header_info['items']);
  }
  if (isset($page_header_info['size'])) {
    $cache_size = $page_header_info['size'];
    $cache_size_units = t('bytes');
    if ($cache_size > 1024) {
      $cache_size = $cache_size / 1024;
      $cache_size_units = 'KB';
    }
    if ($cache_size > 1024) {
      $cache_size = $cache_size / 1024;
      $cache_size_units = 'MB';
    }
    $extra[] = '<span title="'. format_number($page_header_info['size']) .' '. t('bytes') .'">'. t('Size') .': '. format_number($cache_size, 2) .' '. $cache_size_units .'</span>';
  }
  if (!empty($extra)) {
    $header_left_pane .= ' ('. implode(', ', $extra) .')';
  }

  $header_right_pane = t('Current time') .': '. format_date(time(), 'custom', 'Y-m-d H:i:s');

  $output = '<div class="clear-block">';
  $output .= '<div class="cache-browser-header-left-pane">'. $header_left_pane .'</div>';
  $output .= '<div class="cache-browser-header-right-pane">'. $header_right_pane .'</div>';
  $output .= '</div>';
  return $output;
}
