;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Cache browser module for Drupal
;; $Id: README.txt,v 1.2.2.4 2008/11/02 16:16:02 markuspetrux Exp $
;;
;; Original author: markus_petrux at drupal.org (September 2008)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

OVERVIEW
========

The Cache browser module provides a page (protected by 'administer cache tables'
permission) from where you can browse all Drupal cache tables, list their
contents, dump cached entries. You can also reset the contents of any cache
table or delete single entries.

Data dumps use tipical PHP var_dump format. Arrays and classes are collapsed by
default. You can expand/collapse them one by one or all at once.
The expand/collapse feature requires a javascript enabled browser and uses
Drupal/jQuery compatible methods.

For developers, function <code>cbdump($mixed [,$varname])</code> is provided as
an alternative to <code>var_dump()</code> for debugging purposes.


REQUIREMENTS
============

- This module requires the format_number module that can be found here:
  http://drupal.org/project/format_number


INSTALLATION
============

- Copy all contents of this package to your modules directory preserving
  subdirectory structure.
- Goto admin/build/modules to install the module.
- Goto admin/user/access to adjust permissions (administer cache tables).


USAGE
=====

- Goto admin/build/cache_browser

  From here you can browser all Drupal cache tables, list their contents, dump
  the contents of cached entries.
  You can also reset the contents of any cache table or delete single entries.

  Data dumps use PHP tipical var_export format. Arrays and classes are
  collapsed by default. You can expand/collapse them one by one or all at once.
  The expand/collapse feature requires a javascript enabled browser and uses
  Drupal/jQuery compatible methods.


SUPPORT
=======

- Please use the issue tracker of the project at drupal.org:

  http://drupal.org/project/cache_browser


CREDITS
=======

- Original versions of the plus/minus icons can be found free from here:

  http://www.famfamfam.com/
