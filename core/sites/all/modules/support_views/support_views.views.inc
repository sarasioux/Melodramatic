<?php

/**
 * Implementation of hook_views_data()
 */
function support_views_views_data() {
  // Support table
  $data['support_ticket']['table']['group']  = t('Support Tickets');
  $data['support_ticket']['table']['base'] = array(
    'field' => 'nid',
    'title' => t('Support Tickets'),
    'help' => t("Support tickets contain ticketing info and can be related to nodes."),
    'weight' => -10,
  );

  $data['support_ticket']['table']['join'] = array(
    'node' => array(
      'left_field' => 'nid',
      'field' => 'nid',
    ),
  );

  // Node ID field.
  $data['support_ticket']['nid'] = array(
    'title' => t('Support ticket'),
    'help' => t('A Support ticket references a node.'),
    'relationship' => array(
      'base' => 'node',
      'field' => 'nid',
      'handler' => 'views_handler_relationship',
      'label' => t('Support node'),
    ),
  );

  // State
  $data['support_ticket']['state'] = array(
    'title' => t('State ID'),
    'help' => t('The State ID of the ticket.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );

  // Priority
  $data['support_ticket']['priority'] = array(
    'title' => t('Priority ID'),
    'help' => t('The Priority ID of the ticket.'),
    'field' => array(
    'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );

  // Client ID
  $data['support_ticket']['client'] = array(
    'title' => t('Client ID'),
    'help' => t('The Client ID of the ticket.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );

  // Assigned
  $data['support_ticket']['assigned'] = array(
    'title' => t('Assigned ID'),
    'help' => t('The Assigned ID of the ticket.'),
    'relationship' => array(
       'base' => 'users',
       'field' => 'uid',
       'handler' => 'views_handler_relationship',
       'label' => t('Assigned User'),
    ),
    'field' => array(
    'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );

  // Last state change
  $data['support_ticket']['last_state_change'] = array(
    'title' => t('Last state change'),
    'help' => t('Date of last state change.'),
    'field' => array(
    'handler' => 'support_views_handler_field_last_state_change',
      'click sortable' => TRUE,
      'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    ),
  );

  // Support State Table
  $data['support_states']['table']['group'] = t('Support Tickets');
  $data['support_states']['table']['join']['node'] = array(
    'left_table' => 'support_ticket',
    'left_field' => 'state',
    'field' => 'sid',
  );

  // sid
  $data['support_states']['sid'] = array(
    'title' => t('States ID'),
    'help' => t('The state ID of the ticket.'),
  );

  // State
  $data['support_states']['state'] = array(
    'title' => t('State'),
    'help' => t('The state of the ticket, e.g. open, closed etc...'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_support_state',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // Support Client Table
  $data['support_client']['table']['group'] = t('Support Tickets');
  $data['support_client']['table']['join']['node'] = array(
    'left_table' => 'support_ticket',
    'left_field' => 'client',
    'field' => 'clid',
  );

  // clid
  $data['support_client']['clid'] = array(
   'title' => t('Client ID'),
   'help' => t('The Client ID of the ticket.'),
  );
  
  // Name
  $data['support_client']['name'] = array(
    'title' => t('Client Name'),
    'help' => t('The Client Name for the ticket.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_support_client',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // Support Priority Table
  $data['support_priority']['table']['group'] = t('Support Tickets');
  $data['support_priority']['table']['join']['node'] = array(
    'left_table' => 'support_ticket',
    'left_field' => 'priority',
    'field' => 'pid',
  );

  // Priority ID
  $data['support_priority']['pid'] = array(
    'title' => t('Priority ID'),
    'help' => t('The Priority ID of the ticket.'),
  );
  
  // State
  $data['support_priority']['priority'] = array(
    'title' => t('Priority'),
    'help' => t('The Priority for the ticket.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_support_priority',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // Support Client Table
  $data['support_ticket_comment']['table']['group'] = t('Support Comments');
  $data['support_ticket_comment']['table']['join'] = array(
    'comments' => array(
      'left_table' => 'comments',
      'left_field' => 'cid',
      'field' => 'cid',
    ),
  );

  return $data;
}

/**
 * Implementation ofr hook_views_handlers().
 */
function support_views_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'support_views'),
    ),
    'handlers' => array(
      // filters
      'views_handler_filter_support_state' => array(
        'parent' => 'views_handler_filter_in_operator',
      ),
      'views_handler_filter_support_client' => array(
        'parent' => 'views_handler_filter_in_operator',
      ),
      'views_handler_filter_support_priority' => array(
        'parent' => 'views_handler_filter_in_operator',
      ),
      'support_views_handler_field_last_state_change' => array(
        'parent' => 'views_handler_field_date',
      ),
    ),
  );
}
