<?php
// $Id$

/**
 * @file
 * Integrates file operations with the Views module.
 */

/**
 * Field handler to present a file type.
 */
class file_views_handler_field_file_nodes_type extends views_handler_field {
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['show_name'] = array(
      '#title' => t('Use type name'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['show_name']),
    );
    $form['show_icon'] = array(
      '#title' => t('Show type icon'),
      '#type' => 'checkbox',
      '#default_value' => !empty($this->options['show_icon']),
    );
  }

  function render($values) {
    $type = $values->{$this->field_alias};
    return (!empty($this->options['show_icon']) ? file_mime_icon_for($type) .'&nbsp;': '') . (!empty($this->options['show_name']) ? file_mime_description_for($type) : $type);
  }
}

