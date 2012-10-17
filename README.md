# TREKK
Drupal for the university...evolved

## DESCRIPTION
Trekk allows you to share content across sites and provides built-in integration with [Flatfish](https://github.com/drupalconnect/flatfish.git) for migrating legacy HTML content into Drupal.
See the homepage, [trekk.drupalconnect.com](http://trekk.drupalconnect.com) for more information, including a FAQ.
File Trekk specific issues [here](/drupalconnect/trekk/issues/)

## NOTE
This software is **BETA**

## TREKK INSTALLATION INSTRUCTIONS
1. Download and gunzip the tarball
2. `cd` into the directory and run `drush make -y --no-core profiles/trekk/trekk.make` to download all of the contrib modules
3. Run the installer--the primary choice is whether the server is a 'client' or 'server'. Clients download content from a centralized server. You can also optionally enable support for flatfish.
4. Follow regular Drupal implementation practices

## TREKK CONTENT SHARING
* The Trekk server code is generic enough to be largely complete
* Content is made accessible via Services and Views
* Clients need custom Migrate classes to consume the Server content
* See the Trekk Client module for example code

## FLATFISH SUPPORT
* Flatfish is a custom Drupal Connect ruby gem that provides jQuery-style CSS selection for HTML web scraping. Flatfish is an easy, powerful, and flexbile way to migrate content into Drupal
* See a demo of flatfish @ [http://drupalconnect.com/flatfish-migration-walkthrough](http://drupalconnect.com/flatfish-migration-walkthrough)
* You'll need to install Ruby 1.9.3 and install the gem to run it
* The gem saves the HTML and media assets to MySQL database tables
* The config.yml file that provides the database connection, the outputted schema.yml, and the database need to be available to the flatfish module. NOTE: Trekk does NOT use Ruby or the flatfish gem directly
* Flatfish creates dynamic Migrate classes and imports/updates the content
* More detailed instructions on the use of the flatfish gem and an example can be found in the GitHub repo
* The flatfish module has more information in its directory
