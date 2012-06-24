// $Id: README.txt,v 1.6 2009/10/29 01:51:48 davereid Exp $

CONTENTS OF THIS FILE
---------------------

 * Description and Benefits
 * Upgrading from 1.x
 * Installation and Usage
 * More Information


DESCRIPTION AND BENEFITS
------------------------

The SEO Checklist module provides a list of good SEO actions that you should
take to maximize the presence of your website in the major search engines. It
provides little functionality itself but rather it helps you keep track of what
needs to be done and what has been completed already.

Search Engines Drive 90% of the traffic on the web. The more "findable" you are,
the easier it is for you to get customers. This module helps you with on-page
SEO - a necessary component of a good online marketing campaign.


UPGRADING FROM 1.x
------------------

If you are upgrading from the previous SEO Checklist 1.x versions, you will
need to follow a couple important steps:

1. Make sure you remove the entire SEO Checklist module's folder before copying
   in the new files.
2. Execute the following SQL in your site's database, making sure to prefix
   the 'system' table name if your site uses a table prefix:
   UPDATE system SET name = 'seochecklist', filename = REPLACE(filename, 'SEOChecklist', 'seochecklist'), status = 1 WHERE type = 'module' AND name = 'SEOChecklist'

3. Make sure you run update.php immediately afterwards.


INSTALLATION AND USAGE
----------------------

See http://drupal.org/getting-started/5/install-contrib for instructions on
how to install or update Drupal modules.

Summary:
1. Download and extract the module package into your sites/all/modules directory.
2. Go to admin/build/modules and enable the "SEO Checklist" module which should
   be in the 'Other' category.
3. To start using the SEO Checklist, go to admin/settings/seochecklist. The
   module should automatically check if you have already installed any of the
   modules required for a task.
4. Start checking off some tasks!

Optional:
- Install the Vertical Tabs module (http://drupal.org/project/vertical_tabs) to
  help improve your SEO Checklist interface. It helps collapse the interface
  into vertical tabs instead of one huge long list of fieldsets. This module
  also works on the add or edit content forms, which is helpful for your site's
  content creators and editors!


MORE INFORMATION
----------------

- A very handy companion for this module is the Drupal 6 Search Engine
  Optimization book by Ben Finklea. For more information and to purchase, go to
  http://www.drupalseobook.com/.

- To issue any bug reports, feature or support requests, see the module issue
  queue at http://drupal.org/project/issues/seo_checklist.

- This module is potentially controversial as many people have ideas about good
  and bad SEO. If you have an idea of a module or task that should be included,
  please file an issue with the above link to the module's issue queue.

- Volacci spent numerous hours in research and development on this module. We
  want to maintain it and keep good SEO advice available to the entire
  community. Instead of asking for donations or bounties for this module, we
  request that you include a simple link back to us somewhere on your website.
  When you're done (or before) go down to the "Link to Volacci" task and check
  it! That will automatically add a link in your website's footer to Volacci.
  If you want to link to use, but not in your footer, uncheck the box and put
  the link to http://www.volacci.com/ where you want to. And thanks for the
  link! If you e-mail us (seochecklist [at] volacci.com) and tell us where your
  link is, then we'll link back to you! And as you may know, links help move
  your site up in the search engines.

- Enjoy using the module!
