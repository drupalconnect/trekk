# Design

Uses Migrate under the hood, so all data imports are techincally Migration subclasses.

There are two basic types of Flatfish Migrations:

1. Content Types (Pages, Articles, News, etc) -- NodeMigration
  The Content Type of Migration primarily serves two functions:

    1. Update Media to replace the tokens (this includes anchor tag hrefs and img tag src attributes).
    2. Clean HTML

2. Media/Files (which handles the physical images, docs, pdfs, xls, etc) -- MediaMigration
