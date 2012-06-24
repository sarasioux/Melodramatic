<?php
// $Id: node_limitnumber.rules_defaults.inc,v 1.1.2.6 2010/04/06 02:10:55 jdwfly Exp $
function node_limitnumber_rules_defaults() {
$config = 
array (
  'rules' => 
  array (
    'node_limitnumber_example' => 
    array (
      '#type' => 'rule',
      '#set' => 'event_node_limitnumber_check_limit',
      '#label' => 'Example: Authenticated Users create only 1 page',
      '#active' => 0,
      '#weight' => '0',
      '#categories' => 
      array (
      ),
      '#status' => 'custom',
      '#conditions' => 
      array (
        0 => 
        array (
          '#weight' => 0,
          '#info' => 
          array (
            'label' => 'User has Authenticated role',
            'label callback' => false,
            'arguments' => 
            array (
              'user' => 
              array (
                'type' => 'user',
                'label' => 'User',
              ),
            ),
            'module' => 'User',
          ),
          '#name' => 'rules_condition_user_hasrole',
          '#settings' => 
          array (
            'roles' => 
            array (
              0 => 2,
            ),
            'operation' => 'OR',
            '#argument map' => 
            array (
              'user' => 'user',
            ),
          ),
          '#type' => 'condition',
        ),
        1 => 
        array (
          '#type' => 'condition',
          '#settings' => 
          array (
            'type' => 
            array (
              'page' => 'page',
            ),
            '#argument map' => 
            array (
              'node' => 'node',
            ),
          ),
          '#name' => 'rules_condition_content_is_type',
          '#info' => 
          array (
            'label' => 'Node is Page',
            'arguments' => 
            array (
              'node' => 
              array (
                'type' => 'node',
                'label' => 'Content',
              ),
            ),
            'module' => 'Node',
          ),
          '#weight' => 0,
        ),
        2 => 
        array (
          '#weight' => 0,
          '#info' => 
          array (
            'label' => 'Reached limit for the node type',
            'module' => 'Node Limit Number',
            'arguments' => 
            array (
              'node' => 
              array (
                'label' => 'Node',
                'type' => 'node',
              ),
              'user' => 
              array (
                'label' => 'User',
                'type' => 'user',
              ),
            ),
          ),
          '#name' => 'node_limitnumber_rules_condition_limit',
          '#settings' => 
          array (
            'limit' => '1',
            'time' => 'none',
            'og' => 'none',
            '#argument map' => 
            array (
              'node' => 'node',
              'user' => 'user',
            ),
          ),
          '#type' => 'condition',
        ),
      ),
      '#actions' => 
      array (
        1 => 
        array (
          '#info' => 
          array (
            'label' => 'Show a configurable message on the site',
            'module' => 'System',
            'eval input' => 
            array (
              0 => 'message',
            ),
          ),
          '#name' => 'rules_action_drupal_message',
          '#settings' => 
          array (
            'message' => 'You cannot add any more pages',
            'error' => 0,
          ),
          '#type' => 'action',
          '#weight' => -1,
        ),
        0 => 
        array (
          '#type' => 'action',
          '#settings' => 
          array (
            'path' => 'node',
            'query' => '',
            'fragment' => '',
            'force' => 0,
            'immediate' => 1,
          ),
          '#name' => 'rules_action_drupal_goto',
          '#info' => 
          array (
            'label' => 'Page redirect to front page',
            'label callback' => false,
            'module' => 'System',
            'eval input' => 
            array (
              0 => 'path',
              1 => 'query',
              2 => 'fragment',
            ),
          ),
          '#weight' => 0,
        ),
      ),
      '#version' => 6003,
    ),
  ),
);
return $config;
}