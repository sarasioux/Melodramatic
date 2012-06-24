
API Module
Generates and displays API documentation pages.

GENERAL INFORMATION

This is an implementation of a subset of the Doxygen documentation generator
specification, tuned to produce output that best benefits the Drupal code base.
It is designed to assume the code it documents follows Drupal coding
conventions, and supports the following Doxygen constructs:
  @mainpage
  @file
  @defgroup
  @ingroup
  @addtogroup (as a synonym of @ ingroup)
  @param
  @return
  @link
  @see
  @{
  @}


In addition to Doxygen syntax requirements, the following restrictions are made
on the code format. These are all Drupal coding conventions (see
http://drupal.org/node/1354 for more details and suggestions).

All documentation blocks must use the syntax:

/**
 * Documentation here.
 */

The leading spaces are required.

When documenting a function, the documentation block must immediately precede
the function it documents, with no intervening blank lines.

There may be no intervening spaces between a function name and the left
parenthesis that follows it.

Functions must be closed by a right curly bracket in the first column. No lines
inside a function may have a right curly bracket in the first column.


Besides the Doxygen features that are supported, this module also provides the
following features:

CVS version numbers and dates are parsed from
tags and reported.

Functions may be in multiple groups (Doxygen ignores all but the first
group). This allows, for example, theme_menu_tree() to be marked as both
"themeable" and part of the "menu system".

Function calls to PHP library functions are linked to the PHP manual.

Function calls have tooltips briefly describing the called function.

Documentation pages have non-volatile, predictable URLs, so links to individual
functions will not be invalidated when the number of functions in a document
changes.

INSTALLATION AND SETUP

See http://drupal.org/node/425944 for information on how to install and set up
this module.
