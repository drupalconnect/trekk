## process == ctools MAGICK!!!

1. *Make sure your Drupal content types exist AND match your Flatfish CSV(s).*
2. Run Flatfish! 
3. Make sure your database, config.yml, and schema.yml are all accessible to Drupal--the config.yml and schema.yml should be in the sites/all/flatfish\_migrations directory.
4. We're using ctools' exportables for the config.  Enabled Flatfish Configurations get registered as Migrations.
5. Update your Flatfish Configuration (admin/structure/trekk-flatfish) to Migrate Nodes (HTML) and/or Media (physical files).
6. Run the Migration(s).

# IMPORTANT NOTES

* If you are not familiar with ctools exportables and Migrate, you are advised to read up on them prior to digging into the code.
* Migrate keeps tabs on all of your partial Migrations in migrate\_\* tables--remember this if you need to clean house.
* Watch out, not all special variables, classes, functions are clearly marked at this time.
