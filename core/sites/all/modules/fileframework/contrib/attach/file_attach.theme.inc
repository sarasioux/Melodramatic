<?php
// $Id$

/**
 * @file_attach
 * Themes for file attachments.
 */

//////////////////////////////////////////////////////////////////////////////
// Theme callbacks

/**
 * Renders a node's or comment's file attachments in a table layout.
 */
function theme_file_attachments($files, $pid) {
  $header = array(
    t('Attachment'),
    t('Kind'),
    t('Size')
  );
  $mime_types = file_get_mime_types();
  $rows = array();
  $id = 0;
  foreach ($files as $file) {
    $file->name = drupal_strlen($file->name) > FILE_ATTACH_TITLE_LENGTH ? drupal_substr($file->name, 0, FILE_ATTACH_TITLE_LENGTH - 3) .'...' : $file->name;
    $rows[] = array(
      l(t('Download'), 'file/'. $file->nid .'/download/'. $file->vid, array('attributes' => array('class' => 'file-attach-download-button'))) . (FILE_ATTACH_SHOW_PREVIEWS ? l(t('Preview'), '', array('attributes' => array('id' => 'file-attach-preview-button-'. $pid .'-'. $id, 'class' => 'file-attach-preview-button'))) : '') . theme('file_render', $file, array('download' => array(), 'show' => array())),
      file_mime_description_for($file->type),
      format_size($file->size),
    );
    $id++;
  }

  $output = '<div id="file-attachments-'. $pid .'" class="file-attachments">';
  if (!empty($rows)) {
    $output .= theme('table', $header, $rows, array('id' => 'attachments'));
    $js = "$(document).ready(function() {
      $('#file-container-". $pid ."').find('.file-attach-container').hide();
      $('#file-attachments-". $pid ."').find('a.file-attach-preview-button').click(function() {
        if($(this).hasClass('active')) {
          $('#file-attachments-". $pid ."').find('.file-attach-preview-button').removeClass('active').text('". t('Preview') ."');
          $('#file-container-". $pid ."').find('.file-attach-container').hide();
        }
        else {
          var id = $(this).attr('id').substr(27);
          $('#file-attachments-". $pid ."').find('.file-attach-preview-button').removeClass('active').text('". t('Preview') ."');
          $(this).addClass('active').text('". t('Hide') ."');
          $('#file-container-". $pid ."').find('.file-attach-container').hide();
          $('#file-attach-' + id).show();
        }
        return false;
      });
    });";
    drupal_add_js($js, 'inline');
  }
  $output .= '</div>';
  return $output;
}

/**
 * Themes the attachments list.
 */
function theme_file_attach_form_current(&$form) {
  $mime_types = file_get_mime_types();
  $header = array('', t('Delete'), t('List'), t('Description'), t('Type'), t('Size'), t('Weight'));
  drupal_add_tabledrag('file-attachments', 'order', 'sibling', 'attach-weight');
  foreach (element_children($form) as $key) {
    // Add class to group weight fields for drag and drop.
    $form[$key]['weight']['#attributes']['class'] = 'attach-weight';

    $type = $form[$key]['type']['#value'];
    $row = array('');
    $row[] = drupal_render($form[$key]['remove']);
    $row[] = drupal_render($form[$key]['list']);
    $row[] = drupal_render($form[$key]['name']);
    $row[] = file_mime_icon_for($type) .' '. file_mime_description_for($type);
    $row[] = format_size($form[$key]['size']['#value']);
    $row[] = drupal_render($form[$key]['weight']);
    $rows[] = array('data' => $row, 'class' => 'draggable'. (isset($form[$key]['list']['#value']) ? ' menu-enabled' : ' menu-disabled'));
  }
  return !empty($rows) ? theme('table', $header, $rows, array('id' => 'file-attachments')) : '';
}

/**
 * Themes the attachment form.
 * Note: required to output prefix/suffix.
 */
function theme_file_attach_form_new($form) {
  return drupal_render_form('file_attach_new', $form);
}

/**
 * Renders a block containing previews and generated files for one file.
 */
function theme_file_attach_show($id, $data) {
  return !empty($data) ? '<div id="file-attach-'. $id .'" class="file-attach-container"><div class="file-attach-show">'. $data .'</div></div>' : '';
}

/**
 * Renders a block containing a list of backreferences to a file node.
 */
function theme_file_attach_backreferences($nodes) {
  if (!empty($nodes)) {
    $output = '<span>'. t('This file is attached to') .':</span>';
    $output .= '<div class="item-list">';
    $output .= '<ul>';
    foreach ($nodes as $node) {
      $output .= '<li>'. l($node->title, 'node/'. $node->nid, isset($node->cid) ? array('fragment' => 'comment-'. $node->cid) : array()) .'</li>';
    }
    $output .= '</ul>';
    $output .= '</div>';
    return $output;
  }
}

/**
 * Renders an autocomplete block.
 */
function theme_file_attach_autocomplete($matches) {
  $output = array();
  foreach ($matches as $match) {
    $name = db_result(db_query('SELECT u.name FROM {users} u WHERE u.uid = %d', $match['uid']));
    $title = check_plain($match['title']);
    $title = drupal_strlen($title) > 50 ? drupal_substr($title, 0, 50) .'...' : $title;
    $output['['. $match['nid'] .'] '. $match['title']] = file_mime_icon_for($match['type']) .' '. $title .' <span style="color: grey;">('.  format_size($match['size']) .') '. format_date($match['created'], 'small') .' - '. $name .'</span>';
  }

  return $output;
}

