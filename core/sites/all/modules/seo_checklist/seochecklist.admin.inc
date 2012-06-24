<?php
// $Id: seochecklist.admin.inc,v 1.11 2010/05/25 13:13:44 eclipsegc Exp $

/**
 * @file
 * Administrative page callbacks for the seochecklist module.
 */

define('SEOCHECKLIST_BOOK_PURCHASE', 'http://www.packtpub.com/drupal-6-search-engine-optimization-seo/mid/170909568gh3?utm_source=volacci.com%2F&utm_medium=affiliate&utm_content=authorsite&utm_campaign=mdb_000690');

/**
 * Define the settings form.
 */
function seochecklist_admin_settings() {
  global $user;

  $form['save_above'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  // Fetch modules and groups from database.
  $query = "SELECT id, name, description FROM {seo_group} ORDER BY id";

  $result = db_query($query);
  $group_id = 0;

  // Every group is a fieldset.
  while ($data = db_fetch_object($result)) {
    $group_id = intval($data->id);

    $form['group_' . $group_id] = array(
      '#type' => 'fieldset',
      '#title' => t($data->name),
      '#collapsible' => TRUE,
      '#description' => t($data->description),
      '#group' => 'seochecklist',
    );

    $res = db_query("SELECT download, enable, configure, module, completed, id, name, uid FROM {seo_checklist} WHERE group_id = %d ORDER BY order_id", $group_id);
    while ($row = db_fetch_object($res)) {
      $row->links = array();

      if ($row->completed) {
        // Show when and who completed this task.
        $row->links[] = t('Completed on %date by !username', array('%date' => format_date($row->completed, 'small'), '!username' => theme('username', user_load($row->uid))));
      }
      else {
        // Show non-completed sub-tasks.
        if ($row->download) {
          $row->links[] = l(t('Download'), $row->download, array('attributes' => array('target' => '_blank')));
        }
        if ($row->enable) {
          $row->links[] = l(t('Enable'), $row->enable);
        }
      }

      if ($row->configure && (!$row->module || module_exists($row->module))) {
        // Show the link to configure if this isn't a module or module is enabled.
        $row->links[] = l(t('Configure'), $row->configure);
      }

      if (variable_get('seo_checklist_book_references', 1) && $page = seochecklist_get_book_references($row->id)) {
        $row->links[] = t('See <a href="@book-purchase">Drupal 6 SEO</a> p. @page', array('@page' => $page, '@book-purchase' => SEOCHECKLIST_BOOK_PURCHASE));
      }

      $task = $form['group_' . $group_id]['seochecklist_task_' . $row->id] = array(
        '#type' => 'checkbox',
        '#title' => t($row->name),
        '#default_value' => $row->completed || ($row->module && module_exists($row->module)),
        '#description' => join(' | ', $row->links),
      );
      if (strpos($row->name, 'Clean URLs') === 0) {
        $task['#disabled'] = !variable_get('clean_url', 0);
        $task['#default_value'] |= variable_get('clean_url', 0);
      }
    }
  }

  $form['extras'] = array(
    '#type' => 'fieldset',
    '#title' => t('Extras'),
    '#collapsible' => TRUE,
    '#group' => 'seochecklist',
  );
  $form['extras']['seo_checklist_link'] = array(
    '#type' => 'checkbox',
    '#title' => t('Link to Volacci to thank them for this awesome module.'),
    '#default_value' => variable_get('seo_checklist_link', 0),
    '#description' => t('A small link will appear at the very bottom of your website. You can disable it at any time by un-checking this box. We really appreciate it!'),
  );
  $form['extras']['seo_checklist_thanks'] = array(
    '#type' => 'checkbox',
    '#title' => t('Send us feedback on the Drupal 6 SEO Checklist module or just say <em>Thanks!</em> and we will link to you from our website. Send your feedback and link information to <a href="mailto:@email">@email</a>.', array('@email' => 'seochecklist@volacci.com')),
    '#default_value' => variable_get('seo_checklist_thanks', 0),
    '#description' => t('If you do not know why you should link with other websites, read the following article: <a href="@link">Why links help SEO</a>.', array('@link' => 'http://www.volacci.com/why-links-help-seo')),
  );
  $form['extras']['seo_checklist_podcast'] = array(
    '#type' => 'checkbox',
    '#title' => t('Listen to the <a href="@podcast">Volacci Drupal SEO Podcast</a> for more tips and tricks about Drupal SEO.', array('@podcast' => 'http://www.volacci.com/podcast')),
    '#default_value' => variable_get('seo_checklist_podcast', 0),
  );
  $form['extras']['seo_checklist_book_references'] = array(
    '#type' => 'checkbox',
    '#title' => t('Include page number references from the <a href="@book">Drupal 6 SEO Book</a> by Ben Finklea.', array('@book' => 'http://www.drupalseobook.com/')),
    '#default_value' => variable_get('seo_checklist_book_references', 1),
    '#description' => t('<a href="@book-purchase">Purchase from Packt Publishing</a>', array('@book-purchase' => SEOCHECKLIST_BOOK_PURCHASE)),
  );
  $form['extras']['buy_seo_book'] = array(
    '#type' => 'checkbox',
    '#title' => t('Buy Drupal 6 Search Engine Optimization by Ben Finklea from <a href="@amazon">Amazon</a> or <a href="@packt">Packt</a>.', array('@amazon' => 'http://www.amazon.com/gp/product/1847198228?ie=UTF8&tag=dvdcentral02&linkCode=as2&camp=1789&creative=390957&creativeASIN=1847198228', '@packt' => 'https://www.packtpub.com/drupal-6-search-engine-optimization-seo/book?mid/170909568gh3')),
    '#default_value' => variable_get('seo_checklist_buy_book', 1),
  );

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 100,
  );

  if (module_exists('vertical_tabs')) {
    $form['#pre_render'][] = 'vertical_tabs_form_pre_render';
    $form['save_above']['#attributes']['class'] = 'js-hide';
  }
  else {
    drupal_set_message(t('Your SEO Checklist interface (in addition to your content and content type edit pages) will be greatly enhanced by installing the <a href="@vertical-tabs">Vertical Tabs module</a>.', array('@vertical-tabs' => 'http://drupal.org/project/vertical_tabs')), 'status', FALSE);
  }

  return $form;
}

/**
 * Submit callback for seochecklist_admin_settings().
 */
function seochecklist_admin_settings_submit($form, &$form_state) {
  global $user;

  $count = 0;
  foreach ($form_state['values'] as $key => $value) {
    if (preg_match('/seochecklist_task_/', $key)) {
      $key = explode('_', $key);
      $key = $key[2];
      $current = (bool) db_result(db_query("SELECT completed FROM {seo_checklist} WHERE id = %d", $key));
      if ($current != $value) {
        // If the checkbox changed states, update the record.
        db_query("UPDATE {seo_checklist} SET completed = %d, uid = %d WHERE id = %d", ($value ? time() : 0), $user->uid, $key);
        $count++;
      }
    }
  }

  // Special values not in database.
  variable_set('seo_checklist_link', $form_state['values']['seo_checklist_link']);
  variable_set('seo_checklist_thanks', $form_state['values']['seo_checklist_thanks']);
  variable_set('seo_checklist_podcast', $form_state['values']['seo_checklist_podcast']);
  variable_set('seo_checklist_book_references', $form_state['values']['seo_checklist_book_references']);

  drupal_set_message(format_plural($count, 'Updated @count task successfully.', 'Updated @count tasks successfully.'));
}
