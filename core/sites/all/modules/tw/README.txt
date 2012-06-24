$Id: README.txt,v 1.1.2.12 2010/05/17 14:55:47 mikeryan Exp $

OVERVIEW

The Table Wizard facilitates dealing with database tables:

* It allows surfacing any table in the Drupal default database through Views 2.
* Relationships between the tables it manages can be defined, so views
  combining data in the tables can be constructed.
* It performs analysis of the tables it manages, reporting on empty fields,
  data ranges, ranges of string lengths, etc.
* It provides an API for other modules to views-enable their tables.
* It provides an API for importing data into tables in the Drupal default
  database (automatically doing the views integration above).
* It is bundled with an implementation of this API, for importing comma- and
  tab-delimited files.

Applications of Table Wizard include:

* Module developers can use Table Wizard to views-enable their data tables
  without writing their own Views hook.
* Developers and administrators can use Table Wizard to create views of all
  tables in the database, whether or not their creators provided Views integration.
* Users could upload spreadsheets and apply Views to them.

INSTALLATION

Table Wizard requires the Schema and Views modules. In addition, you may need to
install the following patches to support external tables (check to see if
these patches are in the Schema and Views versions you have installed):

Schema 1.3 or earlier: http://drupal.org/node/411538
Views 2.3 or earlier:  http://drupal.org/node/380560

For versions of Views up through 2.5, you need the patch in this issue
to be able to create custom views for tables with more than 32 characters in their
name:

http://drupal.org/node/437070

For versions of Views up through at least 2.8, you need the patch in this issue
to be able to create views joining data between your default database and a
separate database on the same server (MySQL only):

http://drupal.org/node/576694

CONFIGURATION

If you are going to be importing large delimited files, you should set your
PHP upload_max_filesize and post_max_size settings to be larger than the largest
file you need to upload. The simplest way to do that is to add the settings to
the .htaccess file in your Drupal root:

  php_value upload_max_filesize             64M
  php_value post_max_size                   64M

USAGE

Please see the detailed documentation at http://drupal.org/node/452374

CREDITS

Table Wizard was developed by Cyrve (http://cyrve.com/).

Much of the initial development was sponsored by GenomeWeb (http://www.genomeweb.com/)
and The Economist (http://www.economist.com/).

The basis of the tw_import_delimited module was Node Import
(http://drupal.org/project/node_import).
