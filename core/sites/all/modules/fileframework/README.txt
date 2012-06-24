// $Id$

NOTE: This is experimental software meant for advanced users; assume nothing
works, and you may be pleasantly surprised. And when it breaks, you get to
keep both pieces.

File Management for Drupal
==========================
This module allows uploading files as a standalone content type and provides
a comprehensive file management framework.


FILE UTILITY MODULES
--------------------

  * file:
    Provides files as a standalone content type and includes an extended
    file API and a comprehensive file operations framework.
    NOTE: This module is required by all the following modules.

  * file_attach:
    Allows attaching files to nodes and comments (replaces upload.module).

  * file_convert:
    Allows files to be converted from one MIME content type into another as needed.

  * file_antivirus:
    Allows files to be scaned on the upload. Uses ClamAV in command line or 
    daemon mode.

  * file_restriction:
    Controls which restrictions should be applied on the uploaded files.

  * file_gallery:
    Provides a taxonomy-based gallery view of various file types.

  * file_browser:
    Provides a file browser for file nodes organized in a hierarchical taxonomy tree.

  * file_embed:
    Provides an input filter for embedding files into other content.

  * file_views:
    Integrates file operations with the Views module.


FILE FORMAT MODULES
-------------------
These bundled lightweight modules each support a specific class of file
formats and media types. Each module registers the set of MIME content types
it is capable of understanding, and then provides support for extracting
metadata from those formats, as well as handles displaying file previews for
them.

  * file_image:
    Supports image and photo file formats, extracting metadata such as EXIF
    information, and provides image thumbnails and previews.

  * file_audio:
    Supports audio file formats, extracting ID3 metadata, and bundles a
    Flash-based MP3 player.

  * file_video:
    Supports video file formats, extracting ID3 metadata, and bundles a
    Flash-based video player.

  * file_text:
    Supports plain text file formats.

  * file_archive:
    Supports compressed archive file formats. Provides listing of commpresed
    files.

  * file_slideshow:
    Supports slideshow and presentation file formats. Provides a Flash-based
    preview.

  * file_spreadsheet:
    Supports spreadsheet file formats.

  * file_document:
    Supports document file formats.


BUG REPORTS
-----------
Post bug reports and feature requests to the issue tracking system at:

  <http://drupal.org/node/add/project_issue/fileframework>


CREDITS
-------
Developed and maintained by Arto Bendiken <http://bendiken.net/> and
  Miglius Alaburda <miglius at gmail dot com>

