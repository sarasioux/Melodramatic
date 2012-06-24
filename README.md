Melodramatic
============

Welcome to Melo Development!

Here is the quickest path toward getting setup:

1.  Unzip the file db/dev_melo_db.sql.zip and import it into your MySQL database.
2.  Create a virtual host on your webserver called "melodramatic" and point it at the core/ folder.
3.  Copy the file core/sites/default/default.settings.php to settings.php and edit the database settings.
4.  Visit http://melodramatic/ in a web browser.

Notes:
* You can use any vhost you want (such as "localhost") so long as you have a working sites/melodramatic => sites/default symbolic link.
* If you get a WSOD (White screen of death) check your webserver error logs or edit the index.php file and insert a "ini_set('display_errors', 1);" up at the top.

Good Luck!