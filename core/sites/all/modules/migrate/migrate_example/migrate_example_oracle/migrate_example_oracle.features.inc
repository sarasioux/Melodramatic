<?php

/**
 * Implementation of hook_node_info().
 */
function migrate_example_oracle_node_info() {
  $items = array(
    'migrate_example_oracle' => array(
      'name' => t('Migrate example - oracle'),
      'module' => 'features',
      'description' => t('Example and test fodder for migration directly from an Oracle database.'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'has_body' => '1',
      'body_label' => t('Body'),
      'min_word_count' => '0',
      'help' => '',
    ),
  );
  return $items;
}
