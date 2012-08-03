The client component does most of the work in a Trekk deployment. The core of the system is a custom MigrateSource class for handling HTTP requests. Once a client is set up it can be run via migrate's "control panel" or via migrate's drush commands.

To set up aggregation, one visits the admin UI to configure source, source schema, destination, and mapping. Once done, a dynamic migration is instantiated and the configuration is passed in. Running an aggregation simply means running the migration corresponding to your configuration.

Todos
1. Implement a control point to initiate aggregation through a trekk client aggregator
2. Revise how mappings and source schemas are stored
