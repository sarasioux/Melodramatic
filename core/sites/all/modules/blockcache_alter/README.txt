Block cache alter
------------------------
Requires Drupal 6

Author: Kristof De Jaeger - http://drupal.org/user/107403
Sponsored by One Agency - http://www.one-agency.be

Overview:
--------
Alter cache settings per block. Cache settings per block are now set in code,
but if you don't like the default - usually none - you can now easily change this
per block on the block configuration page in a fieldset called 'Caching settings'.
Install this to speed up block rendering for authenticated users.

The module also comes with 2 core patches you can apply to the block module
which will make block caching much smarter. You'll be able to set expire times
for block based and cache clearing on actions (nodeapi, comment and user).
For those who don't like applying patches, you can also use a complete
patched version of block.module that comes with this module. 
All patches and module files can be found in the patches directory.

1: blockcache_alter_with_node_grants.patch / block_with_node_grants.module
   Most users should use this one.

2: blockcache_alter_no_node_grants.patch / block_no_node_grants.module
   Experienced users can use this one if one of your installed modules is implementing
   a node_grants hook. Drupal checks on this and whenever 1 or more hooks are found
   block caching is disabled completely. Handle with care though, this might
   cause problems, be sure to test your site completely if you apply this patch.

Note: you can run this module *without* applying a patch, you simply don't get
that much options for refreshing a block.

Installation:
-------------
1. Place this module directory in your modules folder (this will
   usually be "sites/all/modules/").
2. Go to "administer -> build -> modules" and enable the module.

3. Either copy the right patched block.module file or you can
   apply one of the 2 core patches if you like. If you patch, copy
   the patchfile to modules/block and run following command:
   
   patch -p0 < filename.
   
   To reverse the patch, simple run following command:
   
   patch -R -p0 < filename

Configuration:
--------------
Go to "administer -> settings -> blockcache_alter" where you have 2 options

- core patch: toggle this checkbox if you have applied one of the 2 core 
  patches. Additional options for refreshing the block will appear in the caching
  fieldset on the block configuration page. Note: if you didn't apply a core patch,
  these additional settings simply won't have any effect.
- Debug: apply this only during testing and development. It will show you
  messages when a block is refreshed.

Calling a block from code
-------------------------
Sometimes, developers do not enable the block itself, but call it with the 
module_invoke() function to put it somewhere they need. In that case blocks don't 
get cached even if all cache settings are set, because the block does not belong to 
any region. To work around that problem:

1. set the cache settings for the block as you desire, leave it disabled
2. call the block with the following piece of code:

<?php
$block = module_invoke('blockcache_alter', 'block', 'view', 'block,14');
print $block['content'];
?>

The last parameter should consist of the name of original block and its original delta 
seperated by comma. All blocks are cached that way.

Support:
--------
Please use the issue queue available at http://drupal.org/project/blockcache_alter to
file support and feature requests, bugs etc. Be as descriptive as you can.

Last updated:
------------
; $Id: README.txt,v 1.1.2.6 2009/06/02 18:35:58 swentel Exp $
