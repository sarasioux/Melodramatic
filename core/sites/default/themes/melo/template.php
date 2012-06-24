<?php

// $Id: template.php,v 1.17.2.1 2009/02/13 06:47:44 johnalbin Exp $

/**
 * @file
 * Contains theme override functions and preprocess functions for the theme.
 *
 * ABOUT THE TEMPLATE.PHP FILE
 *
 *   The template.php file is one of the most useful files when creating or
 *   modifying Drupal themes. You can add new regions for block content, modify
 *   or override Drupal's theme functions, intercept or make additional
 *   variables available to your theme, and create custom PHP logic. For more
 *   information, please visit the Theme Developer's Guide on Drupal.org:
 *   http://drupal.org/theme-guide
 *
 * OVERRIDING THEME FUNCTIONS
 *
 *   The Drupal theme system uses special theme functions to generate HTML
 *   output automatically. Often we wish to customize this HTML output. To do
 *   this, we have to override the theme function. You have to first find the
 *   theme function that generates the output, and then "catch" it and modify it
 *   here. The easiest way to do it is to copy the original function in its
 *   entirety and paste it here, changing the prefix from theme_ to melo_.
 *   For example:
 *
 *     original: theme_breadcrumb()
 *     theme override: melo_breadcrumb()
 *
 *   where melo is the name of your sub-theme. For example, the
 *   zen_classic theme would define a zen_classic_breadcrumb() function.
 *
 *   If you would like to override any of the theme functions used in Zen core,
 *   you should first look at how Zen core implements those functions:
 *     theme_breadcrumbs()      in zen/template.php
 *     theme_menu_item_link()   in zen/template.php
 *     theme_menu_local_tasks() in zen/template.php
 *
 *   For more information, please visit the Theme Developer's Guide on
 *   Drupal.org: http://drupal.org/node/173880
 *
 * CREATE OR MODIFY VARIABLES FOR YOUR THEME
 *
 *   Each tpl.php template file has several variables which hold various pieces
 *   of content. You can modify those variables (or add new ones) before they
 *   are used in the template files by using preprocess functions.
 *
 *   This makes THEME_preprocess_HOOK() functions the most powerful functions
 *   available to themers.
 *
 *   It works by having one preprocess function for each template file or its
 *   derivatives (called template suggestions). For example:
 *     THEME_preprocess_page    alters the variables for page.tpl.php
 *     THEME_preprocess_node    alters the variables for node.tpl.php or
 *                              for node-forum.tpl.php
 *     THEME_preprocess_comment alters the variables for comment.tpl.php
 *     THEME_preprocess_block   alters the variables for block.tpl.php
 *
 *   For more information on preprocess functions and template suggestions,
 *   please visit the Theme Developer's Guide on Drupal.org:
 *   http://drupal.org/node/223440
 *   and http://drupal.org/node/190815#template-suggestions
 */


/*
 * Add any conditional stylesheets you will need for this sub-theme.
 *
 * To add stylesheets that ALWAYS need to be included, you should add them to
 * your .info file instead. Only use this section if you are including
 * stylesheets based on certain conditions.
 */
/* -- Delete this line if you want to use and modify this code
// Example: optionally add a fixed width CSS file.
if (theme_get_setting('melo_fixed')) {
  drupal_add_css(path_to_theme() . '/layout-fixed.css', 'theme', 'all');
}
// */

// Add enemy stylesheet
if(arg(0) == 'enemy') {
  drupal_add_css(path_to_theme().'/css/enemy.css');
}
if(arg(0) == 'home') {
  drupal_add_css(path_to_theme().'/css/frontpage.css');
}

drupal_add_js(path_to_theme().'/js/melo.js');

/**
 * Implementation of HOOK_theme().
 */
function melo_theme(&$existing, $type, $theme, $path) {
  $hooks = zen_theme($existing, $type, $theme, $path);
  // Add your theme hooks like this:
  $hooks['preprocess_user_profile'] = array('variables');
  /*
  $hooks['hook_name_here'] = array( // Details go here );
  */
  // @TODO: Needs detailed comments. Patches welcome!
  return $hooks;
}

/**
 * Override or insert variables into all templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered (name of the .tpl.php file.)
 */
/* -- Delete this line if you want to use this function
function melo_preprocess(&$vars, $hook) {
  $vars['sample_variable'] = t('Lorem ipsum.');
}
// */

/**
 * Override or insert variables into the page templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("page" in this case.)
 */
function melo_preprocess_page(&$vars, $hook) {
  if((arg(0) == 'user' || arg(0) == 'node') && is_numeric(arg(1)) && is_null(arg(2))) {
    $vars['body_classes'] .= ' node-comments';
  }
  
  // Add comment box
  if(isset($vars['node']) && $vars['node']->comment == 2 && $vars['node']->comment_count == 0 && !arg(2)) {
    if (user_access('post comments')) {
     $vars['content'] .= '<div id="comment-input">'.comment_form_box(array('nid' => $vars['node']->nid), t('leave a gspot')).'</div>';
    }
  }
    
  // Check for colors.css
  $colors = false;
  foreach($vars['css']['all']['module'] as $css => $val) {
    if(strpos($css, 'colors')) {
      $colors = true;
    }
  }
  if(!$colors) {
    $vars['styles'] .= "\n".'<link type="text/css" rel="stylesheet" media="all" href="/sites/melodramatic/themes/melo/css/colors.css?" />'."\n";  
  }
  
  // Add create links to the page title
  $arg0 = arg(0);
  $arg1 = arg(1);
  $arg2 = arg(2);
  switch($arg0) {
    case 'mymelo':
      switch($arg1) {
        case 'content':
          switch($arg2) {
            case 'polls':
              $vars['title'] .= '<span class="title-right new-link">'.l('new poll', 'node/add/poll').'</span>';
            break;
            case 'favorites':
              $vars['title'] .= '<span class="title-right new-link">'.l('new favorite', 'node/add/favorite').'</span>';
            break;
            case 'entries':
            case '':
              $vars['title'] .= '<span class="title-right new-link">'.l('new entry', 'node/add/blog').'</span>';
            break;
            case 'folders':
              $vars['title'] .= '<span class="title-right new-link">'.l('new folder', 'node/add/folder').'</span>';
            break;
            case 'photos':
              $vars['title'] .= '<span class="title-right new-link">'.l('new photo', 'node/add/photo').'</span>';
            break;
            case 'colors':
              $vars['title'] .= '<span class="title-right new-link">'.l('new color scheme', 'node/add/color-scheme').'</span>';
            break;
          }
        break;
      }
     break;
    case 'mycliques':
      if(($arg1 == 'all') || ($arg1 == 'mine' && $arg2 == 'memberships')) {
        $vars['title'] .= '<span class="title-right new-link">'.l('start a new clique', 'node/add/clique').'</span>';
      }
    break;
    case 'admin':
      if($arg1 == 'reports' && $arg2 == 'feedback') {
        $vars['title'] .= '<span class="title-right new-link">'.l('submit support ticket', 'node/add/devticket', array('query'=>'destination=community/devcenter/feedback')).'</span>';
      }
    break;
  }
  
  // Replace user profile tabs
  if(arg(0) == 'user' && is_numeric(arg(1)) && !arg(2)) {
    unset($vars['tabs']);
  }
}
// */

/**
 * Override or insert variables into the node templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("node" in this case.)
 */
function melo_preprocess_node(&$vars, $hook) {

  // Format terms
  $terms = array();
  $links = array();
  if(is_array($vars['tags'])) {
    foreach ($vars['tags'] as $vid => $tags) {
      foreach($tags as $term) {
        $links[] = l($term->name, taxonomy_term_path($term), array('rel' => 'tag', 'title' => strip_tags($term->description)));
      }
    }
    $vars['terms'] = implode(', ', $links);
  }

  // Remove "username's blog" link
  if($vars['type'] == 'blog') {
    unset($vars['node']->links['blog_usernames_blog']);
    $vars['links'] = theme('links', $vars['node']->links, array('class' => 'links inline'));
  }

}

/**
 * Override or insert variables into the comment templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("comment" in this case.)
 */
/* -- Delete this line if you want to use this function
function melo_preprocess_comment(&$vars, $hook) {
  $vars['sample_variable'] = t('Lorem ipsum.');
}
// */

/**
 * Override or insert variables into the block templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("block" in this case.)
 */
/* -- Delete this line if you want to use this function
function melo_preprocess_block(&$vars, $hook) {
  $vars['sample_variable'] = t('Lorem ipsum.');
}
// */

function melo_preprocess_user_profile(&$variables) {
  $variables['profile'] = array();
  // Sort sections by weight
  uasort($variables['account']->content, 'element_sort');
  // Provide keyed variables so themers can print each section independantly.
  foreach (element_children($variables['account']->content) as $key) {
    $variables['profile'][$key] = drupal_render($variables['account']->content[$key]);
  }
  // Collect all profiles to make it easier to print all items at once.
  $variables['user_profile'] = implode($variables['profile']);
  
  $variables['body_classes'] .= ' node-comments';
}

function melo_preprocess_user_picture(&$variables) {
  $variables['picture'] = '';
  if (variable_get('user_pictures', 0)) {
    $account = $variables['account'];
    if(!empty($account->picture) && file_exists($account->picture)) {
      $picture = $account->picture;
    }
    else if (variable_get('user_picture_default', '')) {
      $picture = variable_get('user_picture_default', '');
    }
    $variables['full_picture'] = file_create_path($picture);
    
    // Check if we have a custom thumbnail @TODO check if we're allowed to have a custom thumbnails
    $profile = node_load(array('type' => 'profile', 'uid' => $account->uid));
    if($profile) {
      $file = $profile->field_user_thumbnail[0]['filepath'];
      if(file_exists($file)) {
        $picture = $file;
      }
    }

    if (isset($picture)) {
      $alt = t("@user's picture", array('@user' => $account->name ? $account->name : variable_get('anonymous', t('Anonymous'))));
      $variables['picture'] = theme('imagecache', 'user_thumb', $picture, $alt, $alt, '', FALSE);
      if (!empty($account->uid) && user_access('access user profiles')) {
        $attributes = array('attributes' => array('title' => t('View user profile.')), 'html' => TRUE);
        $variables['picture'] = l($variables['picture'], "user/$account->uid", $attributes);
      }
    }
  }
}

function melo_theme_permvalue($value) {
  return '<div class="mymelo-perm-'.$value.'">'.$value.'</div>';
  $perm = array(
    'public' => '/'.path_to_theme().'/images/icons/folder.png', 
    'private' => '/'.path_to_theme().'/images/icons/folder_key.png', 
    'friends' => '/'.path_to_theme().'/images/icons/folder_heart.png'
    );
    
  $img = theme('image', $perm[$value]);
  echo $img;
  exit();
  return theme('image', $perm[$value]);
}

function melo_user_list($users, $title = NULL) {
  if (!empty($users)) {
    foreach ($users as $user) {
      $items[] = theme('username', $user);
    }
  }
  return implode(', ', $items);
}

// Remove filter tips
function melo_filter_tips_more_info() { return ''; }

/**
 * Displays a table for users to administer their recurring fees.
 */
function melo_uc_recurring_user_table($uid) {
  $rows = array();
  $output = '';

  // Set up a header array for the table.
  $header = array(t('Amount'), t('Interval'), t('Next charge'), t('Actions'));

  $context = array(
    'revision' => 'themed-original',
    'location' => 'recurring-user-table',
  );

  $fees = uc_recurring_get_user_fees($uid);
  foreach ($fees as $fee) {
    $ops = uc_recurring_get_fee_ops('user', $fee);

    // Add the row to the table for display.
    $rows[] = array(
      l(strip_tags(uc_price($fee->fee_amount, $context)), 'user/'. $uid .'/order/'. $fee->order_id),
      array('data' => check_plain($fee->regular_interval), 'nowrap' => 'nowrap'),
      format_date($fee->next_charge, 'small'),
      array('data' => implode(' | ', $ops), 'nowrap' => 'nowrap'),
    );
  }

  // Only display the table if fees were found.
  if (!empty($rows)) {
    $output = theme('table', $header, $rows);
  }
  return $output;
}

function melo_menu_item_link($link) {
  // prep our array
  if (empty($link['localized_options'])) {
    $link['localized_options'] = array();
    $link['localized_options']['attributes'] = array();
  }
  
  // Keep main menu items highlighted
  if($link['menu_name'] == 'menu-mainmenu' && $link['link_title'] != 'home' && $link['depth'] == 1) {
    // check the requested url
    $urlarr = explode('/', $_REQUEST['q']);
    // check the url alias of this path
    $newurl = drupal_get_path_alias($link['link_path']);
    $newarr = explode('/', $newurl);
    // take care of god
    if($link['router_path'] == 'admin/god') {
      if($urlarr[0] == 'admin') {
        $link['localized_options']['attributes']['class'] .= ' active';
      }
    } else if($urlarr[0] == $newarr[0]) {
      // take care of user
      if($urlarr[0] == 'user') {
        if($urlarr[1] == $newarr[1]) {
          $link['localized_options']['attributes']['class'] .= ' active';
        }
      } else {
        $link['localized_options']['attributes']['class'] .= ' active';
      }
      
    // Handle API
    } else if($urlarr[0] == 'api' && $newarr[0] == 'community') {
      $link['localized_options']['attributes']['class'] .= ' active';
    }
  }

  // Only add names to tabs & mymelo for now
  if($link['type'] == MENU_DEFAULT_LOCAL_TASK || $link['type'] == MENU_LOCAL_TASK) {
    $link['localized_options']['attributes']['class'] .= ' menu-tab-'.$link['title'];
  }
  if($link['menu_name'] == 'menu-mymelo') {
    $class = 'menu-tab-'.str_replace('/', '_', $link['link_path']);
    $link['localized_options']['attributes']['class'] .= ' '.$class;
  }
  return l($link['title'], $link['href'], $link['localized_options']);
}

function melo_imagefield_admin_thumbnail($item = NULL) {
  if (is_null($item) || empty($item['filepath'])) {
    return '<!-- link to default admin thumb -->';
  }
  $thumb_path = imagefield_file_admin_thumb_path($item);
  return '<img src="'. file_create_url($thumb_path) .'" title="' . check_plain($item['filename']) . '" />';
}

function melo_status_messages($display = NULL) {
  $output = '';
  foreach (drupal_get_messages($display) as $type => $messages) {
    $output .= "<div class=\"messages $type\">\n";
    if (count($messages) > 1) {
      $output .= " <ul>\n";
      foreach ($messages as $message) {
        $output .= '  <li>'. $message ."</li>\n";
      }
      $output .= " </ul>\n";
    }
    else {
      $output .= '<ul><li>'.$messages[0].'</li></ul>'."\n";
    }
    $output .= "</div>\n";
  }
  return $output;
}

function melo_form_element($element, $value) {
  // This is also used in the installer, pre-database setup.
  $t = get_t();

  $output = '<div class="form-item';
  if($element['#type'] == 'select') {
    $output .= ' form-item-select';
  }
  $output .= '" ';
  if (!empty($element['#id'])) {
    $output .= ' id="'. $element['#id'] .'-wrapper"';
  }
  $output .= ">\n";
  $required = !empty($element['#required']) ? '<span class="form-required" title="'. $t('This field is required.') .'">*</span>' : '';

  if (!empty($element['#title'])) {
    $title = $element['#title'];
    if (!empty($element['#id'])) {
      $output .= ' <label for="'. $element['#id'] .'">'. $t('!title: !required', array('!title' => filter_xss_admin($title), '!required' => $required)) ."</label>\n";
    }
    else {
      $output .= ' <label>'. $t('!title: !required', array('!title' => filter_xss_admin($title), '!required' => $required)) ."</label>\n";
    }
  }

  $output .= " $value\n";

  if (!empty($element['#description'])) {
    $output .= ' <div class="description">'. $element['#description'] ."</div>\n";
  }

  $output .= "</div>\n";

  return $output;
}

/**
 * Output a sortable table containing all feedback entries.
 */
function melo_feedback_admin_view_form(&$form) {
  $output = '';
  $status = 0;
  foreach (element_children($form) as $element) {
    $item = &$form[$element];
    if (!isset($item['#type']) || $item['#type'] != 'fieldset') {
      continue;
    }
    // Build the table.
    $rows = array();
    $children = element_children($item);
    foreach ($children as $element_entry) {
      $entry = &$item[$element_entry];
      
      // Check for a ticket
      if($node = db_fetch_object(db_query('select nid from {content_type_devticket} where field_feedback_reference_value = %d', $element_entry))) {
        $nid = $node->nid;
        $link = l('view ticket', 'node/'.$nid);
      } else {
        $link = '<strong>'.l('create ticket', 'node/add/devticket', array('query'=>'field_feedback_reference='.$element_entry.'&destination=community/devcenter/feedback')).'</strong>';
      }
      
      $field = array(
        '#type' => 'textarea',
        '#title' => 'Admin Note',
        '#rows' => 3,
        '#name' => 'edit-feedback-messages-'.$element_entry.'-admin-note',
        '#prefix' => '<div id="edit-feedback-messages-0-'.$element_entry.'-note" class="feedback-admin-note">',
        '#suffix' => '</div>'
        );
      
      // Render the data first.
      $rows[] = array(
        0,
        drupal_render($entry['location']),
        drupal_render($entry['date']),
        drupal_render($entry['user']),
        drupal_render($entry['message']) . drupal_render($field),
        $link,
      );
      // Render the checkbox.
      $rows[count($rows) - 1][0] = drupal_render($entry);
    }
    if (empty($rows)) {
      $rows[] = array(array('data' => t('No feedback entries available.'), 'colspan' => 5));
    }
    // Inject the table.
    $form['#feedback_header'][] = 'Edit';
    $item['messages'] = array(
      '#value' => theme('table', $form['#feedback_header'], $rows) . theme('pager', array(), 50, $status),
      '#weight' => -1,
    );
    // Render the fieldset.
    $output .= drupal_render($item);

    $status++;
  }
  // Render internal FAPI and potential extra form elements.
  $output .= drupal_render($form);
  return $output;
}

function melo_tribune_post($post, $node) {
  unset($post['info']);
  $post['clock'] = tribune_format_clock($post['clock'], $post['post_rank']);
  $post['message'] = $post['parsed_message'];

  $contents = "";

  $class = 'tribune-post';

  if ($post['is_self']) {
    $class .= " tribune-self-post";
  }
  if ($post['is_answer']) {
    $class .= " tribune-answer";
  }
  if ($post['moderated']) {
    $class .= " tribune-moderated";
  }

  $title = tribune_post_title($post['referenced_by']);

  $contents = "<li class='". $class ."' ref='". $title ."' id='post-". $post['post_id'] ."' login='". $post['login'] ."'>\n";

  $contents .= theme("post_username", $post) .' ';
  if (tribune_access("moderate tribune", $node)) {
    $contents .= theme("post_moderation", $post);
  }
  $contents .= theme("post_time",     $post) .' ';
  
  $contents .= theme("post_message",  $post);

  $contents .= "</li>\n";
  
  return $contents;
}

function melo_post_moderation($post) {
  $contents = "";

  if ($post['moderated']) {
    $contents .= "<span class='tribune-moderate-post' title='". t("Undelete this post") ."'>";
    $contents .= l('<img alt="'. t("Undelete") .'" src="'. base_path() . drupal_get_path('module', 'tribune') .'/images/tick.gif" />',
      'node/'. $post['tribune_id'] .'/moderation/undelete/'. $post['post_id'],
      array('html' => TRUE, 'attributes' => array('target' => '_blank'))
    );
    $contents .= "</span>\n";
  }
  else {
    $contents .= "<span class='tribune-moderate-post' title='". t("Delete this post") ."'>";
    $contents .= l('<img alt="'. t("Delete") .'" src="'. base_path() . drupal_get_path('module', 'tribune') .'/images/cross.gif" />',
      'node/'. $post['tribune_id'] .'/moderation/delete/'. $post['post_id'],
      array('html' => TRUE, 'attributes' => array('target' => '_blank'))
    );
    $contents .= "</span>\n";
  }

  return $contents;
}

function melo_post_time($post) {
  $contents  = "<span class='tribune-first-clock' title='id=". $post['tribune_post_id'] ."'>";
  $contents .= $post['clock'];
  $contents .= "</span>\n";

  return $contents;
}

function melo_post_username($post) {
  if (strlen ($post['login']) > 0) {
    $contents  = "<span class='tribune-login' title='". htmlspecialchars($post['info'], ENT_QUOTES) ."'>";
    $contents .= $post['login'];
    $contents .= "</span>\n";
  }
  else {
    $contents  = "<span class='tribune-info' title='". htmlspecialchars($post['info'], ENT_QUOTES) ."'>";
    $contents .= $post['info_small'];
    $contents .= '</span> ';
  }

  return $contents;
}

function melo_post_message($post) {
  $contents  = "<div class='tribune-message' dir='ltr'>";
  $contents .= $post['message'];
  $contents .= "</div>";

  return $contents;
}

function melo_imagecache_formatter_default($element) {
   echo 'HEREIAM!';
   exit();
  // Inside a view $element may contain NULL data. In that case, just return.
  if (empty($element['#item']['fid'])) {
    return '';
  }

  // Extract the preset name from the formatter name.
  $presetname = substr($element['#formatter'], 0, strrpos($element['#formatter'], '_'));
  $style = 'linked';
  $style = 'default';

  $item = $element['#item'];
  $item['data']['alt'] = isset($item['data']['alt']) ? $item['data']['alt'] : '';
  $item['data']['title'] = isset($item['data']['title']) ? $item['data']['title'] : NULL;
  
   echo $element['#field_name'].' ';
   if($element['#field_name'] == 'field_fp_image') {
     echo '<pre>';
     print_r($element);
     exit();
   }

  $class = "imagecache imagecache-$presetname imagecache-$style imagecache-{$element['#formatter']}";
  return theme('imagecache', $presetname, $item['filepath'], $item['data']['alt'], $item['data']['title'], array('class' => $class));
}