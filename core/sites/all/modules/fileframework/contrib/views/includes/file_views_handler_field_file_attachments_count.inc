<?php
// $Id$

/**
 * @file
 * Integrates file operations with the Views module.
 */

/**
 * Field handler to present a node attachment count.
 */
class file_views_handler_field_file_attachments_count extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['nid'] = array('table' => 'node', 'field' => 'nid');
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['only_listed'] = array(
      '#title' => t('Only count "listed" file attachments'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['only_listed']),
    );
  }

  function query() {
    $this->add_additional_fields();
    $this->field_alias = $this->aliases['nid'];
  }

  function pre_render($values) {
    $nids = array();
    $this->items = array();

    $data = array();
    foreach ($values as $result) {
      $nids[] = $result->{$this->aliases['nid']};
    }
    if (!empty($nids)) {
      // Support "only listed files" option.
      $where = '';
      if (!empty($this->options['only_listed'])) {
        $where = " AND fa.list <> 0";
      }
      $result = db_query("SELECT fa.nid AS pnid, fn.*, nf.title AS name FROM {file_attachments} fa INNER JOIN {file_nodes} fn ON fa.fnid = fn.nid INNER JOIN {node} na ON fa.vid = na.vid INNER JOIN {node} nf ON fn.vid = nf.vid WHERE fa.nid IN (". implode(', ', $nids) .")". $where ." ORDER BY fa.weight");
      while ($file = db_fetch_object($result)) {
        $this->items[$file->pnid] = isset($this->items[$file->pnid]) ? $this->items[$file->pnid] + 1 : 1;
      }
    }
  }

  function render($values) {
    $nid = $values->{$this->field_alias};
    return isset($this->items[$nid]) ? $this->items[$nid] : 0;
  }
}

