<?php

/**
 * @file
 */

/**
 * Menu callback for dashboard page
 */
function migrate_ui_dashboard() {
  return drupal_get_form('_migrate_ui_dashboard_form');
}

/**
 * Form definition for dashboard page
 */
function _migrate_ui_dashboard_form() {
  drupal_add_css(drupal_get_path('module', 'migrate_ui') . '/migrate_ui.css');
  $build = array();

  $build['overview'] = array(
    '#prefix' => '<div>',
    '#value' => migrate_overview(),
    '#suffix' => '</div>',
  );

  $header = array(
    'status' => array('data' => t('Status')),
    'machinename' => array('data' => t('Migration')),
    'importrows' => array('data' => t('Total rows')),
    'imported' => array('data' => t('Imported')),
    'unimported' => array('data' => t('Unimported')),
    'messages' => array('data' => t('Messages')),
    'lastthroughput' => array('data' => t('Throughput')),
    'lastimported' => array('data' => t('Last imported')),
  );

  $migrations = migrate_migrations();

  $rows = array();
  foreach ($migrations as $migration) {
    $row = array();
    $has_counts = TRUE;
    if (method_exists($migration, 'sourceCount')) {
      $total = $migration->sourceCount();
    }
    else {
      $has_counts = FALSE;
      $total = t('N/A');
    }
    if (method_exists($migration, 'importedCount')) {
      $imported = $migration->importedCount();
    }
    else {
      $has_counts = FALSE;
      $imported = t('N/A');
    }
    if ($has_counts) {
      $unimported = $total - $imported;
    }
    else {
      $unimported = t('N/A');
    }
    $status = $migration->getStatus();
    switch ($status) {
      case MigrationBase::STATUS_IDLE:
        $status = t('Idle');
        break;
      case MigrationBase::STATUS_IMPORTING:
        $status = t('Importing');
        break;
      case MigrationBase::STATUS_ROLLING_BACK:
        $status = t('Rolling back');
        break;
      case MigrationBase::STATUS_STOPPING:
        $status = t('Stopping');
        break;
      case MigrationBase::STATUS_DISABLED:
        $status = t('Disabled');
        break;
      default:
        $status = t('Unknown');
        break;
    }

    $row['status'] = $status;
    $machine_name = $migration->getMachineName();
    $row['machinename'] =
      l($machine_name, 'admin/content/migrate/' . $machine_name);
    $row['importrows'] = $total;
    $row['imported'] = $imported;
    $row['unimported'] = $unimported;

    if (is_subclass_of($migration, 'Migration')) {
      $num_messages = $migration->messageCount();
      $row['messages'] = $num_messages ? l($num_messages, 'admin/content/migrate/messages/' . $machine_name) : 0;
    }
    else {
      $row['messages'] = t('N/A');
    }
    if (method_exists($migration, 'getLastThroughput')) {
      $rate = $migration->getLastThroughput();
      if ($rate == '') {
        $row['lastthroughput'] = t('Unknown');
      }
      elseif ($status == MigrationBase::STATUS_IDLE) {
        $row['lastthroughput'] = t('!rate/min', array('!rate' => $rate));
      }
      else {
        if ($rate > 0) {
          $row['lastthroughput'] = t('!rate/min, !time remaining', array('!rate' => $rate, '!time' => format_interval((60*$unimported) / $rate)));
        }
        else {
          $row['lastthroughput'] = t('!rate/min, unknown time remaining', array('!rate' => $rate));
        }
      }
    }
    else {
      $row['lastthroughput'] = t('N/A');
    }
    $row['lastimported'] = $migration->getLastImported();
    $rows[$machine_name] = $row;
  }

  $build['dashboard'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#empty' => t('No migrations'),
  );

  // Build the 'Update options' form.
  $build['operations'] = array(
    '#type' => 'fieldset',
    '#title' => t('Operations'),
  );
  $options = array(
    'import' => t('Import'),
    'rollback' => t('Rollback'),
    'rollback_and_import' => t('Rollback and import'),
    'stop' => t('Stop'),
    'reset' => t('Reset'),
  );
  $build['operations']['operation'] = array(
    '#type' => 'select',
    '#title' => t('Operation'),
    '#title_display' => 'invisible',
    '#options' => $options,
  );
  $build['operations']['limit'] = array(
    '#tree' => TRUE,
    'value' => array(
      '#type' => 'textfield',
      '#title' => t('Limit to'),
      '#size' => 10,
    ),
    'unit' => array(
      '#type' => 'select',
      '#options' => array(
        'items' => t('items'),
        'seconds' => t('seconds'),
      ),
    ),
  );
  $build['operations']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Execute'),
    '#validate' => array('migrate_ui_dashboard_validate'),
    '#submit' => array('migrate_ui_dashboard_submit'),
  );
  $build['operations']['description'] = array(
    '#prefix' => '<p>',
    '#value' => t(
      'Choose an operation to run on all migrations selected above:
       <ul>
         <li>Import - Imports all previously unimported records from the source, plus
             any records marked for update, into destination Drupal objects.</li>
         <li>Rollback - Deletes all Drupal objects created by the migration.</li>
         <li>Rollback and import - Performs the Rollback operation, immediately
             followed by the Import operation.</li>
         <li>Stop - Cleanly interrupts any import or rollback processes that may
             currently be running.</li>
         <li>Reset - Sometimes a migration process may fail to stop cleanly, and be
             left stuck in an Importing or Rolling Back status. Choose Reset to clear
             the status and permit other operations to proceed.</li>
       </ul>'
    ),
    '#postfix' => '</p>',
  );

  return $build;
}

/**
 * Validate callback for the dashboard form.
 */
function migrate_ui_dashboard_validate($form, &$form_state) {
  // Error if there are no items to select.
  if (!is_array($form_state['values']['dashboard']) || !count(array_filter($form_state['values']['dashboard']))) {
    form_set_error('', t('No items selected.'));
  }
}

/**
 * Submit callback for the dashboard form.
 */
function migrate_ui_dashboard_submit($form, &$form_state) {
  $operation = $form_state['values']['operation'];
  $limit = $form_state['values']['limit'];
  $machine_names = array_filter($form_state['values']['dashboard']);
  $operations = array();

  // Rollback in reverse order.
  if (in_array($operation, array('rollback', 'rollback_and_import'))) {
    $machine_names = array_reverse($machine_names);
    foreach ($machine_names as $machine_name) {
      $operations[] = array('migrate_ui_batch', array('rollback', $machine_name, $limit));
    }
    // Reset order of machines names in preparation for final operation.
    $machine_names = array_reverse($machine_names);
    $operation = $operation == 'rollback_and_import' ? 'import' : NULL;
  }

  // Perform non-rollback operation, if one exists.
  if ($operation) {
    foreach ($machine_names as $machine_name) {
      $operations[] = array('migrate_ui_batch', array($operation, $machine_name, $limit));
    }
  }

  $batch = array(
    'operations' => $operations,
    'title' => t('Migration processing'),
    'file' => drupal_get_path('module', 'migrate_ui') . '/migrate_ui.pages.inc',
    'init_message' => t('Starting migration process'),
    'progress_message' => t(''),
    'error_message' => t('An error occurred. Some or all of the migrate processing has failed.'),
    'finished' => 'migrate_ui_batch_finish',
  );
  batch_set($batch);
}

/**
 * Process all enabled migration processes in a browser, using the Batch API
 * to break it into manageable chunks.
 *
 * @param $operation
 *  Operation to perform - 'import', 'rollback', 'stop', or 'reset'.
 * @param $machine_name
 *  Machine name of migration to process.
 * @param $limit
 *  An array indicating the number of items to import or rollback, or the
 *  number of seconds to process. Should include 'unit' (either 'items' or
 *  'seconds') and 'value'.
 * @param $context
 *  Batch API context structure
 */
function migrate_ui_batch($operation, $machine_name, $limit, &$context) {
  // If we got a stop message, skip everything else
  if (isset($context['results']['stopped'])) {
    $context['finished'] = 1;
    return;
  }

  $migration = Migration::getInstance($machine_name);

  // Messages generated by migration processes will be captured in this global
  global $_migrate_messages;
  $_migrate_messages = array();
  $migration->setOutputFunction('migrate_ui_capture_message');

  // Perform the requested operation
  switch ($operation) {
    case 'import':
      $result = $migration->processImport(array('limit' => $limit));
      break;
    case 'rollback':
      $result = $migration->processRollback(array('limit' => $limit));
      break;
    case 'stop':
      $migration->stopProcess();
      $result = Migration::RESULT_COMPLETED;
      break;
    case 'reset':
      $migration->resetStatus();
      $result = Migration::RESULT_COMPLETED;
      break;
  }

  switch ($result) {
    case Migration::RESULT_INCOMPLETE:
      // Default to half-done, in case we can't get a more precise fix
      $context['finished'] = .5;
      if (method_exists($migration, 'sourceCount')) {
        $total = $migration->sourceCount();
        if ($total && method_exists($migration, 'importedCount')) {
          $imported = $migration->importedCount();
          switch ($operation) {
            case 'import':
              $context['finished'] = $imported/$total;
              break;
            case 'rollback':
              $context['finished'] = ($total-$imported)/$total;
              break;
          }
        }
      }
      break;
    case MigrationBase::RESULT_SKIPPED:
      $_migrate_messages[] = t('Skipped !name due to unfulfilled dependencies',
        array('!name' => $machine_name));
      $context['finished'] = 1;
      break;
    case MigrationBase::RESULT_STOPPED:
      $context['finished'] = 1;
      // Skip any further operations
      $context['results']['stopped'] = TRUE;
      break;
    default:
      $context['finished'] = 1;
      break;
  }

  // Add any messages generated in this batch to the cumulative list
  foreach ($_migrate_messages as $message) {
    $context['results'][] = $message;
  }

  // While in progress, show the cumulative list of messages
  $full_message = '';
  foreach ($context['results'] as $message) {
    $full_message .= $message . '<br />';
  }
  $context['message'] = $full_message;
}

/**
 * Batch API finished callback - report results
 *
 * @param $success
 *  Ignored
 * @param $results
 *  List of results from batch processing
 * @param $operations
 *  Ignored
 */
function migrate_ui_batch_finish($success, $results, $operations) {
  unset($results['stopped']);
  foreach ($results as $result) {
    drupal_set_message($result);
  }
}

function migrate_ui_capture_message($message, $level) {
  if ($level != 'debug') {
    global $_migrate_messages;
    $_migrate_messages[] = $message;
  }
}

/**
 * Menu callback for messages page
 */
function migrate_ui_messages($migration) {
  drupal_add_css(drupal_get_path('module', 'migrate_ui') . '/migrate_ui.css');
  $build = $rows = array();

  $header = array(t('Source ID'), t('Level'), t('Message'));

  if (is_string($migration)) {
    $migration = migration_load($migration);
  }

  // TODO: need a general MigrateMap API
  $messages = db_select($migration->getMap()->getMessageTable(), 'msg')
              ->extend('PagerDefault')
              ->limit(500)
              ->fields('msg')
              ->execute();

  foreach ($messages as $message) {
    $class = $message->level <= MigrationBase::MESSAGE_WARNING ? 'migrate-error' : '';
    $rows[] = array(
      array('data' => $message->sourceid1, 'class' => $class), // TODO: deal with compound keys
      array('data' => $migration->getMessageLevelName($message->level), 'class' => $class),
      array('data' => $message->message, 'class' => $class),
    );
    unset($classes);
  }

  $build['messages'] = array(
    '#prefix' => '<div>',
    '#value' => theme_table($header, $rows),
    '#suffix' => '</div>',
  );

  $build['migrate_ui_pager'] = array('#theme' => 'pager');
  return drupal_render($build);
}

/**
 * Menu callback function for migration view page.
 */
function migrate_migration_info($form_state, $migration) {
  drupal_add_css(drupal_get_path('module', 'migrate_ui') . '/migrate_ui.css');

  if (is_string($migration)) {
    $migration = migration_load($migration);
  }

  $has_mappings = method_exists($migration, 'getFieldMappings');

  $form = array();
  $form['detail'] = array(
    '#type' => 'fieldset',
  );

  // Add vertical tabs display if available.
  $form['detail']['#pre_render'][] = 'vertical_tabs_form_pre_render';
  // Despite docs (http://drupal.org/handbook/modules/vertical-tabs), line below isn't working.
  // $form['detail']['#attached']['js']['vertical-tabs'] = drupal_get_path('module', 'migrate_ui') . '/migrate_ui.js';
  // Here is easy workarund.
  drupal_add_js(drupal_get_path('module', 'migrate_ui') . '/migrate_ui.js');

  if ($has_mappings) {
    $field_mappings = $migration->getFieldMappings();
    // Identify what destination and source fields are mapped
    foreach ($field_mappings as $mapping) {
      $source_field = $mapping->getSourceField();
      $destination_field = $mapping->getDestinationField();
      $sourceFields[$source_field] = $source_field;
      $destinationFields[$destination_field] = $destination_field;
    }
  }

  $form['detail']['overview'] = array(
    '#type' => 'fieldset',
    '#title' => t('Overview'),
    '#group' => 'detail',
  );

  $team = array();
  foreach ($migration->getTeam() as $member) {
    $email_address = $member->getEmailAddress();
    $team[$member->getGroup()][] =
      $member->getName() . ' <' . l($email_address, 'mailto:' .$email_address) . '>';
  }

  foreach ($team as $group => $list) {
    $form['detail']['overview'][$group] = array(
      '#prefix' => '<div>',
      '#value' => theme_item_list($list, $group, 'ul', array('id' => 'team')),
      '#suffix' => '</div>',
    );
  }

  $dependencies = $migration->getHardDependencies();
  if (count($dependencies) > 0) {
    $form['detail']['overview']['dependencies'] = array(
      '#prefix' => '<div>',
      '#value' => '<strong>' . t('Dependencies: ') . '</strong>' .
         implode(', ', $dependencies),
      '#suffix' => '</div>',
    );
  }
  $soft_dependencies = $migration->getSoftDependencies();
  if (count($soft_dependencies) > 0) {
    $form['detail']['overview']['soft_dependencies'] = array(
      '#prefix' => '<div>',
      '#value' => '<strong>' . t('Soft Dependencies: ') . '</strong>' .
         implode(', ', $soft_dependencies),
      '#suffix' => '</div>',
    );
  }

  if ($has_mappings) {
    switch ($migration->getSystemOfRecord()) {
      case Migration::SOURCE:
        $system_of_record = t('Source data');
        break;
      case Migration::DESTINATION:
        $system_of_record = t('Destination data');
        break;
      default:
        $system_of_record = t('Unknown');
        break;
    }
    $form['detail']['overview']['system_of_record'] = array(
      '#prefix' => '<div>',
      '#value' => '<strong>' . t('System of record: ') . '</strong>' . $system_of_record,
      '#suffix' => '</div>',
    );
  }

  $form['detail']['overview']['description'] = array(
    '#prefix' => '<div>',
    '#value' => $migration->getDescription(),
    '#suffix' => '</div>',
  );

  if ($has_mappings) {
    // Destination field information
    $form['detail']['destination'] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#title' => t('Destination'),
      '#group' => 'detail',
      '#description' =>
        t('<p>These are the fields available in the destination of this
           migration. The machine names listed here are those available to be used
           as the first parameter to $this->addFieldMapping() in your Migration
           class constructor. <span class="error">Unmapped fields are red</span>.</p>'),
    );
    $destination = $migration->getDestination();
    $form['detail']['destination']['type'] = array(
      '#type' => 'item',
      '#title' => t('Type'),
      '#value' => (string)$destination,
    );
    $destKey = $destination->getKeySchema();
    $header = array(t('Machine name'), t('Description'));
    $rows = array();
    foreach ($destination->fields() as $machine_name => $description) {
      $class = '';
      if (isset($destKey[$machine_name])) {
        // Identify primary key
        $machine_name .= t(' (PK)');
      }
      else {
        // Add class for mapped/unmapped. Used in summary.
        if (!isset($destinationFields[$machine_name])) {
          $class = 'migrate-error';
        }
      }
      $rows[] = array(array('data' => $machine_name, 'class' => $class), array('data' => $description, 'class' => $class));
    }

    $form['detail']['destination']['fields'] = array(
      '#prefix' => '<div>',
      '#value' => theme_table($header, $rows),
      '#suffix' => '</div>',
    );

    // TODO: Get source_fields from arguments
    $form['detail']['source'] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#title' => t('Source'),
      '#group' => 'detail',
      '#description' =>
        t('<p>These are the fields available from the source of this
           migration. The machine names listed here are those available to be used
           as the second parameter to $this->addFieldMapping() in your Migration
           class constructor. <span class="error">Unmapped fields are red</span>.</p>'),
    );
    $source = $migration->getSource();
    $form['detail']['source']['query'] = array(
      '#type' => 'item',
      '#title' => t('Query'),
      '#value' => '<pre>' . $source . '</pre>',
    );
    $sourceKey = $migration->getMap()->getSourceKey();
    $header = array(t('Machine name'), t('Description'));
    $rows = array();
    foreach ($source->fields() as $machine_name => $description) {
      $class = '';
      if (isset($sourceKey[$machine_name])) {
        // Identify primary key
        $machine_name .= t(' (PK)');
      }
      else {
        // Add class for mapped/unmapped. Used in summary.
        if (!isset($sourceFields[$machine_name])) {
          $class =  'migrate-error';
        }
      }
      $rows[] = array(array('data' => $machine_name, 'class' => $class), array('data' => $description, 'class' => $class));
    }

    $form['detail']['source']['fields'] = array(
      '#prefix' => '<div>',
      '#value' => theme_table($header, $rows),
      '#suffix' => '</div>',
    );

    $header = array(t('Destination'), t('Source'), t('Default'), t('Description'), t('Priority'));

    // First group the mappings
    $descriptions = array();
    $source_fields = $source->fields();
    $destination_fields = $destination->fields();

    foreach ($field_mappings as $mapping) {
      // Validate source and destination fields actually exist
      $source_field = $mapping->getSourceField();
      $destination_field = $mapping->getDestinationField();
      if (!is_null($source_field) && !isset($source_fields[$source_field])) {
        drupal_set_message(t('!source used as source field in mapping but not in
          list of source fields', array('!source' => $source_field)),
        'warning');
      }
      if (!is_null($destination_field) && !isset($destination_fields[$destination_field])) {
        drupal_set_message(t('!destination used as destination field in mapping but not in
          list of destination fields', array('!destination' => $destination_field)),
        'warning');
      }
      $descriptions[$mapping->getIssueGroup()][] = $mapping;
    }

    // Put out each group header
    $rows = array();
    $count = 0;
    foreach ($descriptions as $group => $mappings) {
      $form['detail'][$group] = array(
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
        '#title' => t('Mapping: ') . $group,
        '#group' => 'detail',
        '#attributes' => array('class' => 'migrate-mapping'),
      );
      $rows = array();
      foreach ($mappings as $mapping) {
        $default = $mapping->getDefaultValue();
        if (is_array($default)) {
          $default = implode(',', $default);
        }
        $issue_priority = $mapping->getIssuePriority();
        if (!is_null($issue_priority)) {
          $class = 'migrate-priority-' . $issue_priority;
          $priority = MigrateFieldMapping::$priorities[$issue_priority];
          $issue_pattern = $migration->getIssuePattern();
          $issue_number = $mapping->getIssueNumber();
          if (!is_null($issue_pattern) && !is_null($issue_number)) {
            $priority .= ' (' . l('#' . $issue_number, str_replace(':id:', $issue_number,
              $issue_pattern)) . ')';
          }
          if ($issue_priority != MigrateFieldMapping::ISSUE_PRIORITY_OK) {
            $class .= ' migrate-error';
          }
        }
        else {
          $priority = t('OK');
          $class .= ' migrate-priority-' . 1;
        }
        $row = array(
          array('data' => $mapping->getDestinationField(), 'class' => $class),
          array('data' => $mapping->getSourceField(), 'class' => $class),
          array('data' => $default, 'class' => $class),
          array('data' => $mapping->getDescription(),  'class' => $class),
          array('data' => $priority, 'class' => $class),
        );
        $rows[] = $row;
        $class = '';
      }
      $form['detail'][$group]['table'] = array(
        '#prefix' => '<div>',
        '#value' => theme_table($header, $rows),
        '#suffix' => '</div>',
      );
    }
  }

  return $form;
}

/**
 * Menu callback
 */
function migrate_ui_configure() {
  return drupal_get_form('_migrate_ui_configure_form');
}

/**
 * Form for reviewing migrations.
 */
function _migrate_ui_configure_form() {
  $build = array();
  $build['description'] = array(
    '#prefix' => '<div>',
    '#value' => t('In some cases, such as when a handler for a contributed module is
      implemented in both migrate_extras and the module itself, you may need to disable
      a particular handler. In this case, you may uncheck the undesired handler below.'),
    '#suffix' => '</div>',
  );

  $build['destination'] = array(
    '#type' => 'fieldset',
    '#title' => t('Destination handlers'),
    '#collapsible' => TRUE,
  );

  $header = array(
    'module' => array('data' => t('Module')),
    'class' => array('data' => t('Class')),
    'types' => array('data' => t('Destination types handled')),
  );

  $disabled = unserialize(variable_get('migrate_disabled_handlers', serialize(array())));
  $class_list = _migrate_class_list('MigrateDestinationHandler');
  $rows = array();
  $default_values = array();
  foreach ($class_list as $class_name => $handler) {
    $row = array();
    $module = db_select('autoload_registry', 'r')
              ->fields('r', array('module'))
              ->condition('name', $class_name)
              ->condition('type', 'class')
              ->execute()
              ->fetchField();
    $row['module'] = $module;
    $row['class'] = $class_name;
    $row['types'] = implode(', ', $handler->getTypesHandled());
    if (!in_array($class_name, $disabled)) {
      $default_values[$class_name] = TRUE;
    }
    $rows[$class_name] = $row;
  }
  $build['destination']['destination_handlers'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#default_value' => $default_values,
    '#empty' => t('No destination handlers found'),
  );

  $build['field'] = array(
    '#type' => 'fieldset',
    '#title' => t('Field handlers'),
    '#collapsible' => TRUE,
  );

  $header = array(
    'module' => array('data' => t('Module')),
    'class' => array('data' => t('Class')),
    'types' => array('data' => t('Field types handled')),
  );

  $class_list = _migrate_class_list('MigrateFieldHandler');
  $rows = array();
  $default_values = array();
  foreach ($class_list as $class_name => $handler) {
    $row = array();
    $module = db_select('autoload_registry', 'r')
              ->fields('r', array('module'))
              ->condition('name', $class_name)
              ->condition('type', 'class')
              ->execute()
              ->fetchField();
    $row['module'] = $module;
    $row['class'] = $class_name;
    $row['types'] = implode(', ', $handler->getTypesHandled());
    if (!in_array($class_name, $disabled)) {
      $default_values[$class_name] = TRUE;
    }
    $rows[$class_name] = $row;
  }
  $build['field']['field_handlers'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#default_value' => $default_values,
    '#empty' => t('No field handlers found'),
  );

  $build['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save handler statuses'),
    '#submit' => array('migrate_ui_configure_submit'),
  );

  return $build;
}

/**
 * Submit callback for the dashboard form.
 */
function migrate_ui_configure_submit($form, &$form_state) {
  $disabled = array();
  foreach ($form_state['values']['destination_handlers'] as $class => $value) {
    if (!$value) {
      $disabled[] = $class;
    }
  }
  foreach ($form_state['values']['field_handlers'] as $class => $value) {
    if (!$value) {
      $disabled[] = $class;
    }
  }
  variable_set('migrate_disabled_handlers', serialize($disabled));
  if (!empty($disabled)) {
    drupal_set_message(t('The following handler classes are disabled: !classes',
      array('!classes' => implode(', ', $disabled))));
  }
  else {
    drupal_set_message(t('No handler classes are currently disabled.'));
  }
}
