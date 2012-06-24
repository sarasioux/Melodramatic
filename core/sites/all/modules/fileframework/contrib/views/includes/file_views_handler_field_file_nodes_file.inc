<?php
// $Id$

/**
 * @file
 * Integrates file operations with the Views module.
 */

/**
 * Field handler to present a file or a thumbnail.
 */
class file_views_handler_field_file_nodes_file extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['nid'] = 'nid';
    $this->additional_fields['vid'] = 'vid';
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['display'] = array(
      '#title' => t('Display'),
      '#type' => 'radios',
      '#default_value' => isset($this->options['display']) ? $this->options['display'] : 0,
      '#options' => array(0 => t('File'), 1 => t('Thumbnail')),
    );
    $form['link_to'] = array(
      '#title' => t('Link this field'),
      '#type' => 'select',
      '#default_value' => isset($this->options['link_to']) ? $this->options['link_to'] : 0,
      '#options' => array(t('No link'), t('Node'), t('Download')),
      '#description' => t('A link for the thumbnail display.'),
    );
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }

  function render($values) {
    $nid = $values->{$this->aliases['nid']};
    $vid = $values->{$this->aliases['vid']};
    $node = node_load(array('nid' => $nid, 'vid' => $vid));
    if (isset($node->file) && ($file = $node->file) && is_object($file)) {
      $file->name = drupal_strlen($node->title) > FILE_VIEWS_TITLE_LENGTH_SHORT ? drupal_substr($node->title, 0, FILE_VIEWS_TITLE_LENGTH_SHORT - 3) .'...' : $node->title;
      switch ($this->options['display']) {
        case 0:
          return theme('file_render', $file) .'<script type=\'text/javascript\'>$(\'a.file-metadata\').cluetip();</script>';
        case 1:
          $thumbnail = file_get_image($file, 'file_image_thumbnail');
          $thumbnail = $thumbnail ? $thumbnail : t('no thumbnail');
          return !empty($this->options['link_to']) ? l($thumbnail, $this->options['link_to'] == 1 ? 'node/'. $nid : 'file/'. $nid .'/download', array('html' => TRUE)) : $thumbnail;
      }
    }
  }
}

