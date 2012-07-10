<?php
/**
 *
 */
class trekker_config extends ctools_export_ui {
  function list_form(&$form, &$form_state) {
    parent::list_form($form, $form_state);
    // Register migrations for each enabled configs
    foreach ($form_state['object']->items as $name => $config) {
      if (isset($config->disabled)) {
        continue;
      }
      foreach ($config->config['components'] as $component => $def) {
        if (!strcasecmp('media', $component)) { $class = 'MediaMigration';
        }
        else {
          $class = 'NodeMigration';
        }
        $args = array('machine_name' => $config->machine_name, 'component' => $component);
        Migration::registerMigration($class, strtolower($config->machine_name . '_' . $component), $args);
      }
    }
  }
}
