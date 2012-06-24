<?php
// $Id$

/**
 * @file
 * Integrates file operations with the Views module.
 */

/**
 * Field handler to present a list of nodes the file is attached to.
 */
class file_views_handler_field_file_attachments_parents extends views_handler_field_prerender_list {
  function construct() {
    parent::construct();
    $this->additional_fields['nid'] = array('table' => 'node', 'field' => 'nid');
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['only_listed'] = array(
      '#title' => t('Only show "listed" file attachments'),
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
      $result = db_query("SELECT fa.fnid, fa.nid, n.title FROM {file_attachments} fa INNER JOIN {node} n ON fa.vid = n.vid WHERE fa.nid IN (". implode(', ', $nids) .")". $where ." ORDER BY fa.weight");
      while ($row = db_fetch_object($result)) {
        $this->items[$row->fnid][$row->nid] = $this->render_link(check_plain($row->title), $row->nid);
      }
    }
  }

  /**
   * Render title as a link to the node or file.
   *
   * Data should be made XSS safe prior to calling this function.
   */
  function render_link($title, $nid) {
    return l($title, 'node/'. $nid);
  }
}

