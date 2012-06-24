Webmail Plus Solr

Implements Apache Solr search for email. This module requires apachesolr
(http://drupal.org/project/apachesolr) to function properly. The requirement
is unfortunate since apachesolr integrates the SolrPhpClient and I didn't
want to create a naming collision. It is not necessary to configure Apache
Solr search on your Drupal site to use the email search.

Email search requires a custom schema.xml which you will find with this
project. You need to replace the stock Solr schema file with this one as it
has the correct email fields. If your site already uses Solr search you will
need to create a separate index for email. This can be achieved in two ways:

1. Upgrading to Solr 1.3 and running it in MultiCore
(http://wiki.apache.org/solr/CoreAdmin)

2. Using Solr under Apache Tomcat and running multiple copies of it.
(http://wiki.apache.org/solr/SolrTomcat)