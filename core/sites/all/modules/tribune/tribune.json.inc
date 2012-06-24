<?php
// vim:filetype=php expandtab tabstop=2 softtabstop=2 shiftwidth=2 autoindent smartindent
// $Id: tribune.json.inc,v 1.6.2.1 2008/12/01 08:21:52 seeschloss Exp $

/**
  * Prints one post in JSON format, does not print moderated posts
  */
function tribune_print_json_post($id, $node) {
  if ($id and $post = tribune_get_post($node, $id) and !$post['moderated']) {
    $user  = user_load_self(array());
    $login = $user[1]->name;
    $self = ($post['login'] && $post['login'] == $login);

    print tribune_get_json_post($post, $self);
  }
  exit();
}

/**
  * Prints the new posts since $last_time in JSON format
  * @params $last_id The post ID from which to display posts (exclusively)
  */
function tribune_print_json_newposts($last_time, $node) {
  $posts = tribune_get_new_posts($node, $last_time);

  $user  = user_load_self(array());
  $login = $user[1]->name;

  $contents = "{ 'time': ". time() .",\n".
              "  'help': [\n";
  if (is_array($_SESSION['tribune_filters_help'])) foreach ($_SESSION['tribune_filters_help'] as $filter_name => $filter_help) {
    if ($filter_help['changed']) {
      $contents .= "{ 'filter': \"". str_replace(array("\\", "\n", "\""), array("\\\\", " ", "\\\""), $filter_name) ."\",\n".
             "  'string': \"". str_replace(array("\\", "\n", "\""), array("\\\\", " ", "\\\""), t($filter_help['string'])) ."\"},\n";
      $_SESSION['tribune_filters_help'][$filter_name]['changed'] = FALSE;
    }
  }
  $contents .= "\n],".
               "  'posts': [\n";

  if (sizeof($posts) > 0) foreach ($posts as $post) {
    $self = ($post['login'] && $post['login'] == $login);

    $contents .= tribune_get_json_post($post, $node, $self) .",";

    $_tribune_current_post_id = -1;
  }

  $contents .= "\n]}";

  print $contents;
  exit();
}

/**
  * Returns a JSON string with the contents of a post
  */
function tribune_get_json_post($post, $node, $self = FALSE) {
  $contents = "";

  $class = 'tribune-post';

  if ($self) {
    $class .= ' tribune-self-post';
  }

  if ($post['moderated'] and !tribune_access("moderate tribune", $node)) {
    $contents .= "{ 'moderated': ". $post['moderated'] .",\n".
           "  'id': ". $post['post_id'] ." }\n";
  }
  else {
    $contents .= "{ 'text': \"". str_replace(array("\\", "\n", "\""), array("\\\\", " ", "\\\""), theme('tribune_post', $post, $node)) ."\",\n".
                 "  'classes': \"". $class ."\",\n".
                 "  'ref': \"". tribune_post_title($post['referenced_by']) ."\",\n".
                 "  'login': \"". $post['login'] ."\",\n".
                 "  'moderated': ". $post['moderated'] .",\n".
                 "  'id': ". $post['post_id'] ." }\n";
  }

  return $contents;
}

