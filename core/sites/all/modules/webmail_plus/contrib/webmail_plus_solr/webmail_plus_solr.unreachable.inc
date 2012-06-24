<?php
function webmail_plus_solr_unreachable() {
  drupal_set_title(t('Search failed'));
  $content.=t('Solr search engine is unreachable. Please report this error to the site administrator.');
  return $content;
}
?>