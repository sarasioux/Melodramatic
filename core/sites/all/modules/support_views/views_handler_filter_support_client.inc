<?php

class views_handler_filter_support_client extends views_handler_filter_in_operator {
  function get_value_options() {
    if (isset($this->value_options)) {
      return;
    }

    $this->value_options = array();
    $result=db_query("SELECT clid, name FROM {support_client}");
    
    while ($row = db_fetch_array($result)) {
      $this->value_options[$row['name']] = t(ucfirst($row['name']));
    }
  }
}
