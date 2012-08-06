<?php
/**
 * @file
 * trekk_client_example.default_trekk_client_configs.inc
 */

/**
 * Implements hook_default_trekk_client_config().
 */
function trekk_client_example_default_trekk_client_config() {
  $export = array();

  $config = new stdClass();
  $config->disabled = FALSE; /* Edit this to true to make a default config disabled initially */
  $config->api_version = 1;
  $config->aid = '1';
  $config->machine_name = '';
  $config->name = 'Example';
  $config->source = TrekkClientSourceHTTP::__set_state(array(
   'method' => 'GET',
   'endpoint' => 'http://trekk.partialflow.com/api/v1/content_export/default',
   'params' => 
  array (
  ),
   'username' => NULL,
   'password' => NULL,
   'currentRow' => NULL,
   'currentKey' => NULL,
   'numProcessed' => 0,
   'numIgnored' => 0,
   'schema' => 'trekk_server_example',
   '_schemaDefinition' => NULL,
   'cachePolicy' => true,
   'activeMigration' => NULL,
   'activeMap' => NULL,
   'highwaterField' => NULL,
   'idList' => 
  array (
  ),
   'cacheCounts' => false,
   'cacheKey' => NULL,
   'skipCount' => false,
   'mapRowAdded' => false,
));
  $config->destination = TrekkClientDestinationNode::__set_state(array(
   'bundle' => 'pages',
   'entityType' => NULL,
   'language' => NULL,
   'textFormat' => NULL,
   'numCreated' => 0,
   'numUpdated' => 0,
));
  $config->mapping = '';
  $config->refresh_interval = 0;
  $export['Example'] = $config;

  return $export;
}
