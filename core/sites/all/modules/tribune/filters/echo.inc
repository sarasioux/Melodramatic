<?php
// vim:filetype=php expandtab tabstop=2 softtabstop=2 shiftwidth=2 autoindent smartindent
// $Id: echo.inc,v 1.3 2008/08/30 22:51:38 seeschloss Exp $

function tribune_echo_info() {
  return t('Echoes every message.');
}

function tribune_echo_filter(&$post, &$help) {
  $new_message = array(
    'info'    => $post['info'],
    'message' => $post['message'],
    'login'   => 'Echo',
  );

  return array($new_message);
}
