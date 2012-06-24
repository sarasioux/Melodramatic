*****************************************************
Consistent Language Interface module
Author: Aaron Hawkins

All items in this module are GPL including the flag icons which were borrowed from the old i18n module.
*****************************************************




The Unified Language Interface module is built with the goal of creating a consistent language interface for translated content on your Drupal site.
It provides a block similar to that provided by the core local module, however differs from the local block in several subtle ways:

1. The visibility of language links in the block is consistent and independent from the existence of translations for the page that you are viewing. The logic here is that the user should be able to change the language of the interface at all times. This is important because often there are menus and blocks that are language specific and the user should always be able to find the link back to their language. *** For those who would like to have it so that languages which are not translated also do not have language links you should use the Language Icons module in cunjuction with the core Locale module. ***

2. Pages that branch from node pages such as node/2/render are translated in interface only (rather than switching to a different node) which means you can switch to es/node/2/render to see the rendered page with all other text in that language rather than being sent to es/node/3/render which was the actual translated version of the page. Again this comes back to the concept that the user should be able to view any page with the interface in their native language. In the case of es/node/2/render if you want to see the translation go to the node view of the page and switch languages from there. 

3. The block is designed to be easily themed. Each language adds a class to its <li> tag its <a> tag and its <img> tag. All classes are assigned independent of the Drupal core assigned attributes at this time as a workaround for a bug in core which causes active tags to be assigned to every language on the locale module version. 

To change the configuration settings of the module such as flag and text orientation go to the admin page at: 'admin/settings/language-interface'.

***IMPORTANT NOTE***
Language negotiantion has to be set to "Prefix" or "Prefix with language fallback" in order for this module to work. This setting can be changed at: 'admin/settings/language/configure'