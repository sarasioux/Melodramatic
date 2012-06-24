<?php
// vim:filetype=php expandtab tabstop=2 softtabstop=2 shiftwidth=2 autoindent smartindent
// $Id: smileys.inc,v 1.4 2008/08/30 22:51:38 seeschloss Exp $

function tribune_smileys_info() {
  $status = function_exists("smileys_filter_process") ? t("enabled") : t("<strong>not</strong> enabled");
  $info = array(
    'description' => t("Uses smileys from the smileys module (which is !status).", array('!status' => $status)),
    'weight' => 9,
  );

  return $info;
}

function tribune_smileys_filter_after(&$post, &$help) {
  if (function_exists("smileys_filter_process")) {
    $post['parsed'] = smileys_filter_process($post['parsed']);
  }
}

function tribune_smileys_help() {
  if (function_exists("_smileys_list")) {
    drupal_add_css(drupal_get_path('module', 'tribune') .'/filters/smileys.css');
    drupal_add_js(drupal_get_path('module', 'tribune') .'/filters/smileys.js');

    $form = array();
    $output = '';
    $header = array(t('Smiley'), t('Acronyms'));
    $rows = array();
    $list = _smileys_list(1);
    foreach ($list as $smiley) {
    $acronyms = explode(' ', $smiley->acronyms);
      $rows[] = array(
        '<img src="'. check_url($GLOBALS['base_url'] .'/'. $smiley->image) .'" alt="'. $acronyms[0] .'" title="'. check_plain($smiley->description) .'" class="smiley-class" />',
        check_plain($smiley->acronyms)
      );
    }
    $form['smileys'] = array(
      '#type' => 'fieldset',
      '#title' => t('Textual smileys will be replaced with graphical ones (<em>see list</em>).'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['smileys']['smileys_box'] = array(
      '#type' => 'markup',
      '#value' => theme('table', $header, $rows),
    );
    $output .= drupal_render($form);
    return t(' ') . $output;
  }
  else {
    return "";
  }
}

