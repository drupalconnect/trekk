<?php
/**
 * @file
 * trekk_client_example.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function trekk_client_example_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "trekk_client" && $api == "default_trekk_client_configs") {
    return array("version" => "1");
  }
}
