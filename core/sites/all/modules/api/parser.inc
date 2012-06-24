<?php

/**
 * @file
 * The PHP documentation parser that generates content for api.module.
 */

module_load_include('inc', 'grammar_parser', 'engine/parser');
module_load_include('inc', 'grammar_parser', 'engine/reader');
module_load_include('inc', 'grammar_parser', 'engine/writer');
module_load_include('inc', 'grammar_parser', 'engine/editor');
module_load_include('inc', 'grammar_parser', 'engine/list');
module_load_include('inc', 'grammar_parser', 'engine/object');

/**
 * Reads in a file and calls a callback function to parse and save it.
 *
 * @param $callback
 *   Name of the function to call to parse and save the file contents, such as
 *   api_parse_php_file(), api_parse_text_file(), or api_parse_html_file()
 *   (pass the function name as a string).
 * @param $file_path
 *   Full path to the file to read in.
 * @param $branch
 *   Branch to assign the file contents to.
 * @param $file_name
 *   File name to store in the database for this file.
 */
function api_parse_file($callback, $file_path, $branch, $file_name) {
  $docblock = array(
    'object_name' => $file_name,
    'branch' => $branch,
    'object_type' => 'file',
    'file_name' => $file_name,
    'title' => strpos($file_name, '/') ? substr($file_name, strrpos($file_name, '/') + 1) : $file_name,
    'documentation' => '',
    'version' => '',
    'modified' => filemtime($file_path),
    'source' => str_replace(array("\r\n", "\r"), array("\n", "\n"), file_get_contents($file_path)),
    'content' => '',
    'class' => '',
  );

  $match = array();
  if (preg_match('!\$'.'Id: .*?,v (.*?) (.*?) (.*?) (.*?) Exp \$!', $docblock['source'], $match)) {
    $docblock['version'] = $match[1] .' (checked in on '. $match[2] .' at '. $match[3] .' by '. $match[4] .')';
  }

  $callback($docblock);
}

/**
 * Saves contents of a file as a single piece of text documentation.
 *
 * Callback for api_parse_file().
 *
 * @param $docblock
 *   Array from api_parse_file() containing the file contents and information
 *   about the file, branch, etc.
 */
Function api_parse_text_file($docblock) {
  $docblock['documentation'] = api_format_documentation($docblock['source']);
  $docblock['code'] = api_format_php($docblock['source']);

  api_save_documentation(array($docblock));
}

/**
 * Saves contents of a file as a single piece of HTML documentation.
 *
 * Escapes any HTML special characters in the text, so that it can be
 * displayed to show the HTML tags.
 *
 * Callback for api_parse_file().
 *
 * @param $docblock
 *   Array from api_parse_file() containing the file contents and information
 *   about the file, branch, etc.
 */
function api_parse_html_file($docblock) {
  $docblock['code'] = '<pre>' . htmlspecialchars($docblock['source']) . '</pre>';
  $title_match = array();
  if (preg_match('!<title>([^<]+)</title>!is', $docblock['source'], $title_match)) {
    $docblock['title'] = trim($title_match[1]);
  }
  $documentation_match = array();
  if (preg_match('!<body>(.*?</h1>)?(.*)</body>!is', $docblock['source'], $documentation_match)) {
    $docblock['documentation'] = $documentation_match[2];
  }

  api_save_documentation(array($docblock));
}

/**
 * Parses a PHP file and saves the file and its contents as documentation.
 *
 * PHP functions, classes, global variables, constants, etc. in the file
 * are saved as documentation, if they have docblocks.
 *
 * Callback for api_parse_file().
 *
 * @param $docblock
 *   Array from api_parse_file() containing the file contents and information
 *   about the file, branch, etc.
 */
function api_parse_php_file($docblock) {
  // Build grammar statements.
  $editor = PGPEditor::getInstance();
  $reader = $editor->getReader();
  $reader->setSnippet($docblock['source']);
  $reader->addTokenNames();
  $reader->buildGrammar();

  // Retrieve items of interest.
  $statements = $reader->getStatements();
  if (!$statements) {
    // This is a text file or template file with no functions, constants, etc.
    $docblock['code'] = api_format_php($docblock['source']);
    api_save_documentation(array($docblock));
    // Free up memory.
    $reader->reset();
    return;
  }

  // Reserve the first array slot for the file documentation block.
  $docblock['code'] = api_format_php($docblock['source']);
  $docblocks = array($docblock);

  // Set default documenation block array for items other than the file.
  $default_block = array(
    'branch' => $docblock['branch'],
    'file_name' => $docblock['file_name'],
    'class' => '',
    'object_type' => '',
    'documentation' => '',
    'see' => '',
  );

  api_documentation_loop($statements, $default_block, $docblocks);

  // Free up memory.
  $reader->reset();

  api_save_documentation($docblocks);
}

/**
 * Builds a list of documentation items.
 *
 * @param PGPBody $statements
 *   A PGPBody object containing body statements.
 * @param array $default_block
 *   The default documentation block to use to build documentation items.
 * @param array $docblocks
 *   The array of documentation block items. Documentation items
 *   are added to the end of this array.
 */
function api_documentation_loop($statements, $default_block, &$docblocks) {
  static $object_types = array(
    T_ASSIGNMENT => 'property',
    T_CONST => 'constant',
  );
  $editor = PGPEditor::getInstance();

  // Traverse statement list to gather documentation items.
  $current = $statements->first();
  while ($current != NULL) {
    $statement = $current->data;
    $type = is_object($statement) ? $statement->type : $statement['type'];
    // Common processing.
    switch ($type) {
      case T_ASSIGNMENT: // Class property
        if (empty($default_block['class'])) {
          break;
        } // Explicit fallthrough.
      case T_INTERFACE:
      case T_CLASS:
      case T_FUNCTION:
      case T_DEFINE:
      case T_CONST:
      case T_GLOBAL:
        $docblock = $default_block;
        $docblock['object_type'] = isset($object_types[(int) $type]) ? $object_types[(int) $type] : $editor->statementTypeToString($statement);
        $class_prefix = empty($default_block['class']) ? '' : $default_block['class'] . '::';
        if ($type == T_GLOBAL || $type == T_ASSIGNMENT) {
          $docblock['member_name'] = preg_replace('/^\$/', '', $editor->statementOperandToText($statement));
        }
        else {
          $docblock['member_name'] = $editor->statementOperandToText($statement);
        }
        $docblock['object_name'] = $class_prefix . $docblock['member_name'];
        $docblock['title'] = $class_prefix . $editor->statementOperandToText($statement);
        $docblock['start_line'] = $current->line;
        $docblock['content'] = $editor->commentToString($statement->comment);
        $statement->comment = '';
        $docblock['code'] = api_format_php("<?php\n". $statement->toString() ."\n?".">");
        if ($statement->type == T_FUNCTION) {
          $docblock['signature'] = $editor->functionGetSignature($statement);
        }
        elseif ($statement->type == T_CLASS || $statement->type == T_INTERFACE) {
          $docblock['extends'] = $statement->extends;
          $docblock['implements'] = $statement->implements;
        }
        $docblocks[] = $docblock;
        break;

      case T_DOC_COMMENT:
        $docblock = $default_block;
        $docblock['content'] = $editor->commentToString($statement);
        $docblock['start_line'] = $current->line;
        $docblocks[] = $docblock;
        break;
    }

    // Additional recursive processing on statements with bodies.
    switch ($type) {
      case T_INTERFACE:
      case T_CLASS:
        api_documentation_loop($statement->body, array_merge($default_block, array('class' => $docblock['object_name'])), $docblocks);
        break;
    }
    $current = $current->next;
  }
}

/**
 * Finds functions called in a formatted block of code.
 *
 * @param $code
 *   Formatted block of code.
 *
 * @return
 *   Array of functions that are called in the code.
 */
function api_parse_function_calls($code) {
  $function_calls = array();
  preg_match_all('!<span class="php-function-or-constant">([a-zA-Z0-9_]+)</span>\(!', $code, $function_call_matches, PREG_SET_ORDER);
  array_shift($function_call_matches); // Remove the first match, the function declaration itself.
  foreach ($function_call_matches as $function_call_match) {
    $function_calls[$function_call_match[1]] = $function_call_match[1];
  }

  return $function_calls;
}

/**
 * Saves a documentation block to the database.
 *
 * @param $docblocks
 *   An array containing information about the documentation block.
 *
 * @return
 *   The documentation ID of the inserted/updated construct.
 */
function api_save_documentation($docblocks) {
  $old_dids = array();
  $nested_groups = array();
  $result = db_query("SELECT did FROM {api_documentation} WHERE branch_id = %d AND file_name = '%s'", $docblocks[0]['branch']->branch_id, $docblocks[0]['file_name']);
  while ($object = db_fetch_object($result)) {
    $old_dids[] = $object->did;
  }

  $dids = array();
  $class_dids = array();
  // Look for @file block first so $docblocks[0] gets filled in before it is processed.
  foreach ($docblocks as $docblock) {
    if (preg_match('/' . API_RE_TAG_START . 'file/', $docblock['content'])) {
      $docblocks[0]['content'] = str_replace('@file', '', $docblock['content']);
      break;
    }
  }
  foreach ($docblocks as $docblock) {
    if (preg_match('/' . API_RE_TAG_START . 'mainpage/', $docblock['content'])) {
      preg_match('/' . API_RE_TAG_START . 'mainpage (.*?)\n/', $docblock['content'], $mainpage_matches);
      $docblock['title'] = $mainpage_matches[1];
      $docblock['content'] = preg_replace('/' . API_RE_TAG_START . 'mainpage.*?\n/', '', $docblock['content']);
      $docblock['object_type'] = 'mainpage';
      $docblock['object_name'] = $docblocks[0]['branch']->branch_name;
    }
    elseif (preg_match('/' . API_RE_TAG_START . 'defgroup/', $docblock['content'])) {
      if (preg_match('/' . API_RE_TAG_START . 'defgroup ([a-zA-Z0-9_.-]+) +(.*?)\n/', $docblock['content'], $group_matches)) {
        $docblock['object_name'] = $group_matches[1];
        $docblock['title'] = $group_matches[2];
        $docblock['content'] = preg_replace('/' . API_RE_TAG_START . 'defgroup.*?\n/', '', $docblock['content']);
        $docblock['object_type'] = 'group';
      }
      else {
        watchdog('api', 'Malformed @defgroup in %file at line %line.', array('%file' => $docblocks[0]['file_path'], '%line' => $docblock['start_line']), WATCHDOG_NOTICE);
      }
    }

    // Determine group membership.
    $match = array();
    if (preg_match_all('/' . API_RE_TAG_START . '(ingroup|addtogroup) ([a-zA-Z0-9_]+)/', $docblock['content'], $match)) {
      $docblock['groups'] = $match[2];
      $docblock['content'] = preg_replace('/' . API_RE_TAG_START . '(ingroup|addtogroup).*?\n/', '', $docblock['content']);
    }

    // Handle nested function groups.
    if (!isset($nested_groups[$docblock['class']])) {
      $nested_groups[$docblock['class']] = array();
    }
    foreach ($nested_groups[$docblock['class']] as $group_id) {
      if (!empty($group_id)) {
        $docblock['groups'][] = $group_id;
      }
    }
    if (preg_match('/' . API_RE_TAG_START . '{/', $docblock['content'])) {
      if ($docblock['object_type'] === 'group') {
        array_push($nested_groups[$docblock['class']], $docblock['object_name']);
      }
      elseif (isset($docblock['groups'])) {
        array_push($nested_groups[$docblock['class']], reset($docblock['groups']));
      }
      else {
        array_push($nested_groups[$docblock['class']], '');
      }
    }
    if (preg_match('/' . API_RE_TAG_START . '}/', $docblock['content'])) {
      array_pop($nested_groups[$docblock['class']]);
    }

    if (empty($docblock['object_type'])) {
      continue;
    }

    if (!empty($docblock['content'])) {
      // Find parameter definitions.
      $offset = 0;
      $docblock['parameters'] = '';
      while (preg_match('/' . API_RE_TAG_START . 'param\s(.*?)(?=\n' . API_RE_TAG_START . '|$)/s', substr($docblock['content'], $offset), $param_match, PREG_OFFSET_CAPTURE)) {
        $docblock['content'] = str_replace($param_match[0][0], '', $docblock['content']);
        $docblock['parameters'] .= "\n\n". $param_match[1][0];
        $offset = $param_match[0][1];
      }
      $docblock['parameters'] = api_format_documentation($docblock['parameters']);

      // Find return value definitions.
      $docblock['return_value'] = '';
      preg_match_all('/' . API_RE_TAG_START . 'return\s(.*?)(?=\n' . API_RE_TAG_START . '|$)/s', $docblock['content'], $return_matches, PREG_SET_ORDER);
      foreach ($return_matches as $return_match) {
        $docblock['content'] = str_replace($return_match[0], '', $docblock['content']);
        $docblock['return_value'] .= "\n\n". $return_match[1];
      }
      $docblock['return_value'] = api_format_documentation($docblock['return_value']);

      // Find @see lines.
      $offset = 0;
      $docblock['see'] = '';
      while (preg_match('/' . API_RE_TAG_START . 'see\s(.*?)\n/s', substr($docblock['content'], $offset), $match, PREG_OFFSET_CAPTURE)) {
        $docblock['content'] = str_replace($match[0][0], '', $docblock['content']);
        $docblock['see'] .= "\n\n". $match[1][0];
        $offset = $match[0][1];
      }
      $docblock['see'] = api_format_documentation($docblock['see']);

      // Find @var, a class name
      $docblock['var'] = '';
      if (preg_match('/' . API_RE_TAG_START . 'var\s(.*?)(?=\n' . API_RE_TAG_START . '|$)/s', $docblock['content'], $match)) {
        $docblock['content'] = str_replace($match[0], '', $docblock['content']);
        $docblock['var'] = trim($match[1]);
      }

      // Find @throws, a paragraph
      $docblock['throws'] = '';
      if (preg_match('/' . API_RE_TAG_START . 'throws\s(.*?)(?=\n' . API_RE_TAG_START . '|$)/s', $docblock['content'], $match)) {
        $docblock['content'] = str_replace($match[0], '', $docblock['content']);
        $docblock['throws'] = api_format_documentation($match[1]);
      }

      $docblock['documentation'] = api_format_documentation($docblock['content']);
    }
    $docblock['summary'] = api_documentation_summary($docblock['documentation']);

    if (!empty($docblock['class'])) {
      $docblock['class_did'] = $class_dids[$docblock['class']];
    }

    $docblock['did'] = db_result(db_query("SELECT did FROM {api_documentation} WHERE object_name = '%s' AND branch_id = %d AND object_type = '%s' AND file_name = '%s'", $docblock['object_name'], $docblocks[0]['branch']->branch_id, $docblock['object_type'], $docblock['file_name']));
    if ($docblock['did'] > 0) {
      drupal_write_record('api_documentation', $docblock, 'did');
    }
    else {
      $node = new stdClass();
      $node->type = 'api';
      $node->uid = 0;
      if (module_exists('comment')) {
        $node->comment = COMMENT_NODE_READ_WRITE;
      }
      node_save($node);
      $docblock['did'] = $node->nid;
      $docblock['branch_id'] = $docblocks[0]['branch']->branch_id;
      drupal_write_record('api_documentation', $docblock);
    }

    // Keep track of class membership
    if ($docblock['object_type'] === 'class' || $docblock['object_type'] === 'interface') {
      $class_dids[$docblock['object_name']] = $docblock['did'];
    }

    db_query('DELETE FROM {api_overrides} WHERE did = %d OR overrides_did = %d OR root_did = %d', $docblock['did'], $docblock['did'], $docblock['did']);

    switch ($docblock['object_type']) {
      case 'function':
        db_query('DELETE FROM {api_function} WHERE did = %d', $docblock['did']);
        drupal_write_record('api_function', $docblock);

        if (!empty($docblock['code'])) {
          db_query("DELETE FROM {api_reference_storage} WHERE branch_id = %d AND object_type = 'function' AND from_did = %d", $docblocks[0]['branch']->branch_id, $docblock['did']);
          foreach (api_parse_function_calls($docblock['code']) as $function_name) {
            api_reference($docblocks[0]['branch'], 'function', $function_name, $docblock['did']);
          }
        }
        break;

      case 'file':
        db_query('DELETE FROM {api_file} WHERE did = %d', $docblock['did']);
        drupal_write_record('api_file', $docblock);
        break;

      case 'interface':
      case 'class':
        db_query("DELETE FROM {api_reference_storage} WHERE branch_id = '%s' AND object_type IN ('class', 'interface') AND from_did = %d", $docblocks[0]['branch']->branch_id, $docblock['did']);
        foreach ($docblock['extends'] as $extend) {
          api_reference($docblocks[0]['branch'], 'class', $extend, $docblock['did']);
        }
        foreach ($docblock['implements'] as $implement) {
          api_reference($docblocks[0]['branch'], 'interface', $implement, $docblock['did']);
        }
        break;
    }

    if (isset($docblock['groups'])) {
      db_query("DELETE FROM {api_reference_storage} WHERE branch_id = '%s' AND object_type = 'group' AND from_did = %d", $docblocks[0]['branch']->branch_id, $docblock['did']);
      foreach ($docblock['groups'] as $group_name) {
        api_reference($docblocks[0]['branch'], 'group', $group_name, $docblock['did']);
      }
    }

    $dids[] = $docblock['did'];
  }

  $old_dids = array_diff($old_dids, $dids);
  if (count($old_dids) > 0) {
    $old_dids = implode(',', $old_dids);
    db_query('DELETE FROM {api_documentation} WHERE did IN (%s)', $old_dids);
    db_query('DELETE FROM {api_file} WHERE did IN (%s)', $old_dids);
    db_query('DELETE FROM {api_function} WHERE did IN (%s)', $old_dids);
    db_query('DELETE FROM {api_reference_storage} WHERE from_did IN (%s) OR to_did IN (%s)', $old_dids, $old_dids);
    db_query('DELETE FROM {api_overrides} WHERE did IN (%s) OR overrides_did IN (%s) OR root_did IN (%s)', $old_dids, $old_dids, $old_dids);
  }

  api_schedule_shutdown();
}

/**
 * Formats a documentation block as HTML.
 *
 * First escapes all HTML tags. Then processes links and code blocks, and
 * converts newlines into paragraphs.
 */
function api_format_documentation($documentation) {
  // Don't do processing on empty text (so we don't end up with empty paragraphs).
  if (empty($documentation)) {
    return '';
  }

  $documentation = check_plain($documentation);

  // @link full URLs.
  $documentation = preg_replace('/' . API_RE_TAG_START . 'link ((http:\/\/|https:\/\/|ftp:\/\/|mailto:|smb:\/\/|afp:\/\/|file:\/\/|gopher:\/\/|news:\/\/|ssl:\/\/|sslv2:\/\/|sslv3:\/\/|tls:\/\/|tcp:\/\/|udp:\/\/)([a-zA-Z0-9@:%_+*~#?&=.,\/;-]*[a-zA-Z0-9@:%_+*~#&=\/;-])) (.*?) ' . API_RE_TAG_START . 'endlink/', '<a href="$1">$4</a>', $documentation);
  // Site URLs.
  $documentation = preg_replace('/' . API_RE_TAG_START . 'link \/([a-zA-Z0-9_\/-]+) (.*?) ' . API_RE_TAG_START . 'endlink/', str_replace('%24', '$', l('$2', '$1')), $documentation);

  // Replace left over curly braces
  $documentation = preg_replace('/' . API_RE_TAG_START . '[{}]/', '', $documentation);

  // Process the @code @endcode tags.
  $documentation = preg_replace_callback('/' . API_RE_TAG_START . 'code(.+?)' . API_RE_TAG_START . 'endcode/s', 'api_format_embedded_php', $documentation);

  // Convert newlines into paragraphs.
  $documentation = api_autop($documentation);

  return $documentation;
}

/**
 * Converts newlines into paragraphs.
 *
 * Like _filter_autop(), but does not add <br /> tags.
 */
function api_autop($text) {
  // All block level tags
  $block = '(?:table|thead|tfoot|caption|colgroup|tbody|tr|td|th|div|dl|dd|dt|ul|ol|li|pre|select|form|blockquote|address|p|h[1-6]|hr)';

  // Split at <pre>, <script>, <style> and </pre>, </script>, </style> tags.
  // We don't apply any processing to the contents of these tags to avoid messing
  // up code. We look for matched pairs and allow basic nesting. For example:
  // "processed <pre> ignored <script> ignored </script> ignored </pre> processed"
  $chunks = preg_split('@(</?(?:pre|script|style|object)[^>]*>)@i', $text, -1, PREG_SPLIT_DELIM_CAPTURE);
  // Note: PHP ensures the array consists of alternating delimiters and literals
  // and begins and ends with a literal (inserting NULL as required).
  $ignore = FALSE;
  $ignoretag = '';
  $output = '';
  foreach ($chunks as $i => $chunk) {
    if ($i % 2) {
      // Opening or closing tag?
      $open = ($chunk[1] != '/');
      list($tag) = preg_split('/[ >]/', substr($chunk, 2 - $open), 2);
      if (!$ignore) {
        if ($open) {
          $ignore = TRUE;
          $ignoretag = $tag;
        }
      }
      // Only allow a matching tag to close it.
      elseif (!$open && $ignoretag == $tag) {
        $ignore = FALSE;
        $ignoretag = '';
      }
    }
    elseif (!$ignore) {
      $chunk = api_format_documentation_lists($chunk);
      $chunk = preg_replace('|\n*$|', '', $chunk) ."\n\n"; // just to make things a little easier, pad the end
      $chunk = preg_replace('|<br />\s*<br />|', "\n\n", $chunk);
      $chunk = preg_replace('!(<'. $block .'[^>]*>)!', "\n$1", $chunk); // Space things out a little
      $chunk = preg_replace('!(</'. $block .'>)!', "$1\n\n", $chunk); // Space things out a little
      $chunk = preg_replace("/\n\n+/", "\n\n", $chunk); // take care of duplicates
      $chunk = preg_replace('/\n?(.+?)(?:\n\s*\n|\z)/s', "<p>$1</p>\n", $chunk); // make paragraphs, including one at the end
      $chunk = preg_replace('|<p>\s*</p>\n|', '', $chunk); // under certain strange conditions it could create a P of entirely whitespace
      $chunk = preg_replace("|<p>(<li.+?)</p>|", "$1", $chunk); // problem with nested lists
      $chunk = preg_replace('|<p><blockquote([^>]*)>|i', "<blockquote$1><p>", $chunk);
      $chunk = str_replace('</blockquote></p>', '</p></blockquote>', $chunk);
      $chunk = preg_replace('!<p>\s*(</?'. $block .'[^>]*>)!', "$1", $chunk);
      $chunk = preg_replace('!(</?'. $block .'[^>]*>)\s*</p>!', "$1", $chunk);
      $chunk = preg_replace('/&([^#])(?![A-Za-z0-9]{1,8};)/', '&amp;$1', $chunk);
    }
    $output .= $chunk;
  }
  return $output;
}

/**
 * Regular expression callback for @code in api_format_documentation().
 */
function api_format_embedded_php($matches) {
  return "\n\n". api_format_php("<?php". decode_entities($matches[1]) ."?".">") ."\n\n";
}

/**
 * Formats documentation lists as HTML lists.
 *
 * Parses a block of text for lists that uses hyphens or asterisks as bullets,
 * and format the lists as proper HTML lists.
 */
function api_format_documentation_lists($documentation) {
  $lines = explode("\n", $documentation);
  $output = '';
  $bullet_indents = array(-1);

  foreach ($lines as $line) {
    preg_match('!^( *)([*-] )?(.*)$!', $line, $matches);
    $indent = strlen($matches[1]);
    $bullet_exists = $matches[2];

    if ($indent < $bullet_indents[0]) {
      // First close off any lists that have completed.
      while ($indent < $bullet_indents[0]) {
        array_shift($bullet_indents);
        $output .= '</li></ul>';
      }
    }

    if ($indent == $bullet_indents[0]) {
      if ($bullet_exists) {
        // A new bullet at the same indent means a new list item.
        $output .= '</li><li>';
      }
      else {
        // If the indent is the same, but there is no bullet, that also
        // signifies the end of the list.
        array_shift($bullet_indents);
        $output .= '</li></ul>';
      }
    }

    if ($indent > $bullet_indents[0] && $bullet_exists) {
      // A new list at a lower level.
      array_unshift($bullet_indents, $indent);
      $output .= '<ul><li>';
    }

    $output .= $matches[3] ."\n";
  }

  // Clean up any unclosed lists.
  array_pop($bullet_indents);
  foreach ($bullet_indents as $indent) {
    $output .= '</li></ul>';
  }

  return $output;
}

/**
 * Retrieves a summary from a documentation block.
 */
function api_documentation_summary($documentation) {
  $pos = strpos($documentation, '</p>');
  if ($pos !== FALSE) {
    $documentation = substr($documentation, 0, $pos);
  }
  $documentation = trim(strip_tags($documentation));

  if (strlen($documentation) > 255) {
    return substr($documentation, 0, strrpos(substr($documentation, 0, 252), ' ')) . '…';
  }
  else {
    return $documentation;
  }
}

/**
 * Colorizes and formats PHP code.
 *
 * @param $code
 *   PHP code to format.
 *
 * @return
 *   HTML-formatted code, with spans enclosing various PHP elements.
 */
function api_format_php($code) {
  $output = '';

  if (!defined('T_ML_COMMENT')) {
    define('T_ML_COMMENT', T_COMMENT);
  }
  if (!defined('T_DOC_COMMENT')) {
    define('T_DOC_COMMENT', T_COMMENT);
  }

  $tokens = token_get_all($code);

  $in_string = FALSE;

  foreach ($tokens as $token) {
    if ($in_string) {
      if ($token == '"') {
        $output .= '"</span>';
        $in_string = FALSE;
      }
      else {
        $output .= is_array($token) ? htmlspecialchars($token[1]) : htmlspecialchars($token);
      }
      continue;
    }
    elseif ($token == '"') {
      $output .= '<span class="php-string">"';
      $in_string = TRUE;
      continue;
    }

    if (is_array($token)) {
      $type = $token[0];
      $value = htmlspecialchars($token[1]);

      switch ($type) {
        case T_OPEN_TAG:
        case T_CLOSE_TAG:
          $output .= '<span class="php-boundry">'. $value .'</span>';
          break;

        case T_COMMENT:
        case T_ML_COMMENT:
        case T_DOC_COMMENT:
          $output .= '<span class="php-comment">'. $value .'</span>';
          break;

        case T_VARIABLE:
          $output .= '<span class="php-variable">'. $value .'</span>';
          break;

        case T_CONSTANT_ENCAPSED_STRING:
        case T_INLINE_HTML:
          $output .= '<span class="php-string">'. $value .'</span>';
          break;

        case T_STRING:
          $output .= '<span class="php-function-or-constant">'. $value .'</span>';
          break;

        case T_LNUMBER:
        case T_DNUMBER:
          $output .= '<span class="php-constant">'. $value .'</span>';
          break;

        case T_ARRAY_CAST: case T_ARRAY: case T_AS: case T_BOOL_CAST:
        case T_BREAK: case T_CASE: case T_CLASS: case T_CONST:
        case T_CONTINUE: case T_DECLARE: case T_DEFAULT: case T_DO:
        case T_DOUBLE_CAST: case T_ECHO: case T_ELSE: case T_ELSEIF:
        case T_EMPTY: case T_ENDDECLARE: case T_ENDFOR: case T_ENDFOREACH:
        case T_ENDIF: case T_ENDSWITCH: case T_ENDWHILE: case T_EVAL:
        case T_EXIT: case T_EXTENDS: case T_FOR: case T_FOREACH:
        case T_FUNCTION: case T_GLOBAL: case T_IF: case T_INCLUDE_ONCE:
        case T_INCLUDE: case T_INT_CAST: case T_ISSET: case T_LIST:
        case T_NEW: case T_OBJECT_CAST: case T_PRINT:
        case T_REQUIRE_ONCE: case T_REQUIRE: case T_RETURN: case T_STATIC:
        case T_STRING_CAST: case T_SWITCH: case T_UNSET_CAST: case T_UNSET:
        case T_USE: case T_VAR: case T_WHILE:
          $output .= '<span class="php-keyword">'. $value .'</span>';
          break;

        default:
          $output .= $value;
      }
    }
    else {
      $output .= $token;
    }
  }

  // Manage whitespace:
  return '<pre class="php"><code>'. trim($output) .'</code></pre>';
}

/**
 * Keeps track of references while parsing API files.
 *
 * Since we may parse a file containing a reference before we have parsed the
 * file containing the referenced object, we keep track of the references
 * using a scratch table and save the references to the database table after the
 * referenced object has been parsed.
 *
 * @param $branch
 *   Branch the reference is in.
 * @param $to_type
 *   Type of object being referenced.
 * @param $to_name
 *   Name of object being referenced.
 * @param $from_did
 *   Documentation ID of the object that references this object.
 */
function api_reference($branch, $to_type, $to_name, $from_did) {
  static $is_php_function = array();

  if ($to_type == 'function' && !isset($is_php_function[$to_name])) {
    $is_php_function[$to_name] = (db_result(db_query_range("SELECT 1 FROM {api_documentation} d INNER JOIN {api_branch} b ON b.branch_id = d.branch_id AND b.type = 'php' WHERE d.object_name = '%s'", $to_name, 1, 1)));
  }

  if ($to_type != 'function' || !$is_php_function[$to_name]) {
    db_query("INSERT INTO {api_reference_storage} (object_name, branch_id, object_type, from_did) VALUES ('%s', '%s', '%s', %d)", $to_name, $branch->branch_id, $to_type, $from_did);
  }
}

/**
 * Registers a shutdown function for cron, making sure to do it just once.
 *
 * @see api_shutdown()
 */
function api_schedule_shutdown() {
  static $scheduled = FALSE;

  if (!$scheduled) {
    register_shutdown_function('api_shutdown');
    $scheduled = TRUE;
  }
}

/**
 * Cleans up at the end of the cron job.
 *
 * Updates the collected references, updates the JSON object list, and clears
 * the cache.
 */
function api_shutdown() {
  // Figure out all the dids of the object/branch/types.
  db_query('UPDATE {api_reference_storage} r INNER JOIN {api_documentation} d ON r.object_name = d.object_name AND r.branch_id = d.branch_id AND r.object_type = d.object_type SET r.to_did = d.did');

  // Save overrides
  $changed_classes = array();
  $result = db_query("SELECT ad.did, ad.object_type, ad.member_name, ad.class_did, ad.documentation, af.parameters, af.return_value, ad.see, ad.throws, ad.var FROM {api_documentation} ad LEFT JOIN {api_overrides} ao ON ao.did = ad.did LEFT JOIN {api_function} af ON af.did = ad.did WHERE ad.class_did <> 0 AND ao.did IS NULL");
  while ($object = db_fetch_object($result)) {
    $changed_classes[$object->class_did] = TRUE;
    $override = array(
      'did' => $object->did,
      'overrides_did' => 0,
      'documented_did' => api_has_documentation($object) ? $object->did : 0,
      'root_did' => $object->did,
    );
    $overrides_did = 0;
    $parents = array($object->class_did);
    while ($parent = array_shift($parents)) {
      $result_parents = db_query("SELECT ad.did, ars.to_did, ad.summary, ad.documentation, af.parameters, af.return_value, ad.see, ad.throws, ad.var, ad.class_did FROM {api_reference_storage} ars LEFT JOIN {api_documentation} ad ON ad.class_did = ars.to_did AND ad.object_type = '%s' AND ad.member_name = '%s' LEFT JOIN {api_function} af ON af.did = ad.did WHERE ars.from_did = %d AND ars.object_type IN ('class', 'interface')", $object->object_type, $object->member_name, $parent);
      while ($parent_class = db_fetch_object($result_parents)) {
        $parents[] = $parent_class->to_did;
        if (!empty($parent_class->class_did)) {
          $changed_classes[$parent_class->class_did] = TRUE;
        }
        if (!is_null($parent_class->did)) { // If documented
          if ($override['overrides_did'] === 0) {
            $override['overrides_did'] = $parent_class->did;
          }
          if ($override['documented_did'] === 0 && api_has_documentation($parent_class)) {
            $override['documented_did'] = $parent_class->did;
            // Save the inherited summary
            $inherited_summary = array(
              'summary' => $parent_class->summary,
              'did' => $object->did,
            );
            drupal_write_record('api_documentation', $inherited_summary, 'did');
          }
          $override['root_did'] = $parent_class->did;
        }
      }
    }
    drupal_write_record('api_overrides', $override);
  }

  // Save members
  while (list($class_did,) = each($changed_classes)) {
    // Add child classes to $changed_classes
    $result_chidren = db_query("SELECT ars.from_did FROM {api_reference_storage} ars WHERE ars.to_did = %d AND ars.object_type IN ('class', 'interface') AND ars.from_did <> 0", $class_did);
    while ($child = db_fetch_object($result_chidren)) {
      $changed_classes[$child->from_did] = TRUE;
    }
    // Walk up the tree to find members
    $members = array('');
    $parents = array($class_did);
    while ($parent = array_shift($parents)) {
      $result_parents = db_query("SELECT ars.to_did FROM {api_reference_storage} ars WHERE ars.from_did = %d AND ars.object_type IN ('class', 'interface') AND ars.to_did <> 0", $parent);
      while ($object = db_fetch_object($result_parents)) {
        $parents[] = $object->to_did;
      }
      $result_members = db_query("SELECT did, member_name FROM {api_documentation} WHERE class_did = %d AND member_name NOT IN (" . db_placeholders($members, 'text') . ")", array_merge(array($parent), array_keys($members)));
      while ($member = db_fetch_object($result_members)) {
        $members[$member->member_name] = $member->did;
      }
    }
    // Save them
    array_shift($members);
    db_query("DELETE FROM {api_members} WHERE class_did = %d", $class_did);
    foreach ($members as $did) {
      $api_member = array(
        'class_did' => $class_did,
        'did' => $did,
      );
      drupal_write_record('api_members', $api_member);
    }
  }

  // Save JSON autocomplete cache
  $directory = file_directory_path();
  if (is_dir($directory) && is_writable($directory) && (variable_get('file_downloads', FILE_DOWNLOADS_PUBLIC) == FILE_DOWNLOADS_PUBLIC)) {
    $path = file_create_path('api');
    file_check_directory($path, FILE_CREATE_DIRECTORY);
    $date = gmdate('U');
    foreach (api_get_branch_names() as $branch_name) {
      $new_json = api_autocomplete($branch_name, FALSE);

      $old_file_path = variable_get('api_autocomplete_path_' . $branch_name, FALSE);
      if ($old_file_path !== FALSE) {
        if (md5($new_json) === md5(file_get_contents($old_file_path))) {
          continue; // No changes, no file write.
        }
        // Delete in the future, help avoid race conditions.
        job_queue_add('file_delete', t('Remove expired API JSON, %path.'), array('%path' => $old_file_path));
      }

      $file_name = $path . '/api-' . $branch_name . '-' . $date . '.json';
      file_save_data($new_json, $file_name, FILE_EXISTS_REPLACE);
      variable_set('api_autocomplete_path_' . $branch_name, $file_name);
    }
  }

  cache_clear_all();
}

function api_has_documentation($object) {
  foreach (array('documentation', 'parameters', 'return_value', 'see', 'throws', 'var') as $member) {
    if (!empty($object->$member)) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Updates all branches, by calling their update functions.
 *
 * @see api_update_branch_php()
 * @see api_update_branch_files()
 */
function api_update_all_branches() {
  foreach (api_get_branches() as $branch) {
    $function = 'api_update_branch_' . $branch->type;
    $function($branch);
    watchdog('api', 'Updated %project branch %branch_name.', array('%branch_name' => $branch->branch_name, '%project' => $branch->project));
  }
}

/**
 * Updates a PHP branch.
 *
 * Queries the branch URL to get an updated list of functions, and saves each
 * function in the database.
 *
 * @see api_update_all_branches()
 */
function api_update_branch_php($branch) {
  $response = drupal_http_request($branch->summary);
  if ($response->code === '200') {
    $docblocks = array();
    preg_match_all('!^[a-zA-Z0-9_]+ ([a-zA-Z0-9_]+)\(.*\n.*$!m', $response->data, $function_matches, PREG_SET_ORDER);
    foreach ($function_matches as $function_match) {
      $docblocks[] = array(
        'branch' => $branch,
        'file_name' => $branch->summary,
        'object_type' => 'function',
        'object_name' => $function_match[1],
        'title' => $function_match[1],
        'documentation' => $function_match[0],
        'content' => '',
        'class' => '',
      );
    }
    api_save_documentation($docblocks);
  }
}

/**
 * Updates a files branch.
 *
 * Checks the current directories included in the branch to make an updated
 * list of files. Removes documentation from files that no longer exist, adds
 * documentation from new files, and updates documentation for any files that
 * have changed.
 *
 * @see api_update_all_branches()
 */
function api_update_branch_files($branch) {
  static $parse_functions = array(
    'php' => 'api_parse_php_file',
    'module' => 'api_parse_php_file',
    'inc' => 'api_parse_php_file',
    'install' => 'api_parse_php_file',
    'engine' => 'api_parse_php_file',
    'theme' => 'api_parse_php_file',
    'profile' => 'api_parse_php_file',
    'test' => 'api_parse_php_file',

    'txt' => 'api_parse_text_file',

    'htm' => 'api_parse_html_file',
    'html' => 'api_parse_html_file',
  );

  // List all documented files for the branch.
  $files = array();
  $result = db_query("SELECT f.did, f.modified, d.object_name FROM {api_documentation} d INNER JOIN {api_file} f ON d.did = f.did WHERE d.branch_id = %d AND d.object_type = 'file'", $branch->branch_id);
  while ($file = db_fetch_object($result)) {
    $files[$file->object_name] = $file;
  }

  foreach (api_scan_directories($branch->directories, $branch->excluded_directories) as $path => $file_name) {
    preg_match('!\.([a-z]*)$!', $file_name, $matches);
    if (isset($matches[1]) && isset($parse_functions[$matches[1]])) {
      if (isset($files[$file_name])) {
        $parse = (filemtime($path) > $files[$file_name]->modified);
        unset($files[$file_name]); // All remaining files will be removed.
      }
      else { // New file.
        $parse = TRUE;
      }
      if ($parse) {
        job_queue_add('api_parse_file', t('API parse %branch %file'), array($parse_functions[$matches[1]], $path, $branch, '%file' => $file_name, '%branch' => $branch->branch_name), drupal_get_path('module', 'api') .'/parser.inc', TRUE);
      }
    }
  }

  // Remove outdated files.
  foreach (array_keys($files) as $file_name) {
    watchdog('api', 'Removing %file.', array('%file' => $file_name));
    $result = db_query("SELECT ad.did FROM {api_documentation} ad WHERE ad.file_name = '%s' AND ad.branch_id = %d", $file_name, $branch->branch_id);
    while ($doc = db_fetch_object($result)) {
      db_query("DELETE FROM {api_documentation} WHERE did = %d", $doc->did);
      db_query("DELETE FROM {api_file} WHERE did = %d", $doc->did);
      db_query("DELETE FROM {api_function} WHERE did = %d", $doc->did);
      db_query("DELETE FROM {api_reference_storage} WHERE from_did = %d OR to_did = %d", $doc->did, $doc->did);
    }
    api_schedule_shutdown();
  }
}

/**
 * Finds all the files in the directories specified for a branch.
 *
 * @param $directories
 *   List of directories to scan, as text (separated by newlines).
 * @param $excluded_directories
 *   List of directories to exclude, as text (separated by newlines).
 *
 * @return
 *   Associative array of files, where the keys are the full paths to the
 *   files and the values are the file names.
 */
function api_scan_directories($directories, $excluded_directories) {
  $directory_array = explode("\n", $directories);
  $excluded_array = explode("\n", $excluded_directories);

  if (count($directory_array) > 1) {
    $directories_components = array();
    foreach ($directory_array as $directory) {
      $directory_components = array();
      $parts = explode(DIRECTORY_SEPARATOR, $directory);
      foreach ($parts as $part) {
        if (strlen($part)) {
          array_unshift($directory_components, reset($directory_components) . DIRECTORY_SEPARATOR . $part);
        }
      }
      $directories_components[] = $directory_components;
    }

    $common_ancestor_components = call_user_func_array('array_intersect', $directories_components);
    $common_ancestor = reset($common_ancestor_components);
  }
  else {
    $common_ancestor = $directories;
  }

  $source_files = array();
  foreach ($directory_array as $directory) {
    $files = file_scan_directory($directory, '.*');
    foreach ($files as $path => $file) {
      if (strpos($path, '/.') !== FALSE) {
        continue;
      }
      $excluded = FALSE;
      // If the file is in an excluded path, ignore it
      foreach ($excluded_array as $excluded_path) {
        if (!empty($excluded_path) && (strpos($path, $excluded_path) === 0)) {
          $excluded = TRUE;
        }
      }
      if (!$excluded) {
        $file_name = substr($path, strlen($common_ancestor) + 1);
        $source_files[$path] = $file_name;
      }
    }
  }
  return $source_files;
}
