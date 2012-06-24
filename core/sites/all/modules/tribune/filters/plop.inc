<?php
// vim:filetype=php expandtab tabstop=2 softtabstop=2 shiftwidth=2 autoindent smartindent
// $Id: plop.inc,v 1.5 2008/08/30 22:51:38 seeschloss Exp $

function tribune_plop_info() {
  return t('Adds "plop !" at the end of every message.');
}

function tribune_plop_filter(&$post, &$help) {
  $post['message'] .= " plop !";
}
