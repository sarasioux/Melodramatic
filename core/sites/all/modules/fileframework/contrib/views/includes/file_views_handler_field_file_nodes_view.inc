<?php
// $Id$

/**
 * @file
 * Integrates file operations with the Views module.
 */

/**
 * Field handler to present a file view link.
 */
class file_views_handler_field_file_nodes_view extends views_handler_field_node_link {
  function construct() {
    parent::construct();
    $this->additional_fields['vid'] = 'vid';
    $this->additional_fields['type'] = array('table' => 'node', 'field' => 'type');
  }

  function render($values) {
    $nid = $values->{$this->aliases['nid']};
    $vid = $values->{$this->aliases['vid']};
    $type = $values->{$this->aliases['type']};
    $text = !empty($this->options['text']) ? $this->options['text'] : t('view');
    return $type == 'file' ? l($text, 'file/'. $nid .'/view/'. $vid) : '';
  }
}

