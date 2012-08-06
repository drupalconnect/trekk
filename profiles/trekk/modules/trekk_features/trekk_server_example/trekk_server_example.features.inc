<?php
/**
 * @file
 * trekk_server_example.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function trekk_server_example_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "services" && $api == "services") {
    return array("version" => "3");
  }
}

/**
 * Implements hook_views_api().
 */
function trekk_server_example_views_api() {
  return array("version" => "3.0");
}
