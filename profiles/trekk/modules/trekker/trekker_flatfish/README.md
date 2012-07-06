## process

1. Module picks up YAML. It's actually ctools exportables that drive that. The exportable code is able to read the YAML and derive itself from that. YAML includes all the infos necessary for a migration, including source and destination defs and mappings.
2. Each exportable registers a migration and passes itself to it as config.
3. When migrations are instantiated they load their exportable config and set themselves up to run, dynamically creating and configuring their source and destination classes and their mappings.
