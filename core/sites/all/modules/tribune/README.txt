$Id: README.txt,v 1.2 2008/04/06 13:48:55 seeschloss Exp $

Drupal tribune module
----------------------
Author - Matthieu Valleton valleton.matthieu@ssz.fr
Requires - Drupal 6.x
License - GPLv2 (see LICENSE.txt)

Description
-----------
This module provides a tribune, a space for users to discuss the contents of
the website (or have any other kind of discussion).

Highlighting and clickable clocks make it easier to follow discussions: click
on the clock of the message you want to answer and write your answer after it,
when your mouse hovers clock references on tha page, the referenced post and
the other answers are automatically highlighted.

Features
--------
* Messages highlighting.
* Light javascript, most is done with CSS.
* Support for smileys ([:smiley], see http://totoz.eu for a list).
* User agents used in place of login for anonymous posts are shortened to
  enhance readability

Installation
------------
1. Copy the tribune module to the Drupal modules/ directory.
2. Go to "administer" -> "modules" and enable the module.
3. Check the "administer" -> "access control" page to enable use of
   this module to different roles.
4. Make sure the menu item is enabled in "administer" -> "content management".

Setup
-----
1. Go to "admin/settings/tribune"
2. Adjust the settings as you wish (the defaults are fine if you don't know
   what yo use) and save the configuration.
3. Go to "/tribune" to see the tribune, and start posting messages.

Thanks
------
Thanks to LiNuCe< for providing his titanium and adamantium slipounet() (which
sanitizes the posts) and to finss< for all the bugs he found [:kiki], for his
howto video (http://tout.essaye.sauf.ca/tribune/howto) and for generally
helping [:users] _o/
