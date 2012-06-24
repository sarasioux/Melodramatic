<?php

class ApiTestCase extends DrupalWebTestCase {
  private $default_branch;

  function setUp() {
    $this->default_branch = variable_get('api_default_branch', NULL);
    variable_del('api_default_branch');
    parent::setUp('job_queue', 'grammar_parser', 'api');

    include drupal_get_path('module', 'api') .'/api.admin.inc';
    include drupal_get_path('module', 'api') .'/parser.inc';

    // Make a branch for sample code.
    $branch = new stdClass();
    $branch->type = 'files';
    $branch->project = 'test';
    $branch->branch_name = '6';
    $branch->title = 'Testing 6';
    $branch->status = 1;
    $branch->data = array(
      'directories' => drupal_get_path('module', 'api') .'/tests/sample',
      'excluded_directories' => '',
    );
    api_save_branch($branch);
    variable_set('api_default_branch', $branch->branch_id);

    // Parse the code.
    api_update_all_branches();
    while (job_queue_dequeue()) { }
    api_shutdown();

    api_get_branches(TRUE);
  }

  function tearDown() {
    parent::tearDown();
    // Aparently SimpleTest is leaky sometimes.
    variable_set('api_default_branch', $this->default_branch);
  }

  function getBranch() {
    $branches = api_get_branches();
    return reset($branches);
  }
}
