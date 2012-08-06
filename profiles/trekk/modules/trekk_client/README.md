The client component does most of the work in a Trekk deployment. The core of the system is a custom MigrateSource class for handling HTTP requests. Once a client is set up it can be run via migrate's "control panel" or via migrate's drush commands.

An aggregator consists of three components: a Source (with schema), a destination, and a mapping. The Source defines a method for querying data from a remote server. Trekk Client implements only one source, for fetching JSON via HTTP. It is necessary to define the source data's key field for aggregation. This is done by implementing hook_trekk_client_source_schema(). See trekk_client_example.module for an example of how this is done.

Destinations define how the source data will be handled on the client side. Two destinations are provided, for aggregating nodes and for aggregating terms. Additional destinations may be implemented by defining a class that implements the appropriate interface. See the existing Node and Term destinations for examples of how this is done.

Once source and destination are configured, the final step is to define a mapping from fields in the source to fields in the destination. Mappings can be defined by providing a class that implements TrekkClientAggregatorMappingInterface. See trekk_client_example.module for an example of how this is done.

Note that we will be providing more complete examples in the near future, demonstrating the full extent of these classes' capabilities.

