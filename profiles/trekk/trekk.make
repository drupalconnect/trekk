core = 7.x

api = 2
projects[drupal][version] = "7.x"

;projects[trekk][download][type] = ""
;projects[trekk][download][url] = ""
;projects[trekk][type] = "module"


; Modules
projects[admin_menu][version] = "3.0-rc3"
projects[admin_menu][subdir] = contrib
projects[ctools][version] = "1.0"
projects[ctools][subdir] = contrib
projects[devel][version] = "1.3"
projects[devel][subdir] = contrib
projects[ds][version] = "1.5"
projects[ds][subdir] = contrib
projects[entity][version] = "1.0-rc3"
projects[entity][subdir] = contrib
projects[features][version] = 1.0-rc2
projects[features][subdir] = contrib
projects[libraries][version] = "1.0"
projects[libraries][subdir] = contrib
projects[migrate][version] = "2.3"
projects[migrate][subdir] = contrib
projects[migrate_extras][version] = "2.3"
projects[migrate_extras][subdir] = contrib
projects[rules][version] = "2.1"
projects[rules][subdir] = contrib
projects[services][version] = 3.1
projects[services][subdir] = contrib
projects[strongarm][version] = 2.0-rc1
projects[strongarm][subdir] = contrib
projects[views][version] = "3.3"
projects[views][subdir] = contrib
projects[views_bulk_operations][version] = "3.0-rc1"
projects[views_bulk_operations][subdir] = contrib
projects[wysiwyg][version] = "2.1"
projects[wysiwyg][subdir] = contrib


; Themes
projects[omega][version] = "3.0"
projects[rubik][version] = "4.0-beta8"
projects[tao][version] = "3.0-beta4"


; Libraries
libraries[htmlpurifier][download][type] = "get"
libraries[htmlpurifier][download][url] = "http://htmlpurifier.org/releases/htmlpurifier-4.4.0.tar.gz"
libraries[htmlpurifier][directory_name] = "htmlpurifier"
libraries[htmlpurifier][type] = "library"

libraries[spyc][download][type] = "git"
libraries[spyc][download][url] = "git://github.com/mustangostang/spyc.git"
libraries[spyc][directory_name] = "spyc"
libraries[spyc][type] = "library"

libraries[querypath][download][type] = "git"
libraries[querypath][download][url] = "git://github.com/technosophos/querypath.git"
libraries[querypath][directory_name] = "querypath"
libraries[querypath][type] = "library"
