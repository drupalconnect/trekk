<?php
/**
 * how does this get picked up? automatically via ctools???
 */
class trekker_flatfish_config extends ctools_export_ui {

  function list_form(&$form, &$form_state) {
    parent::list_form($form, $form_state);

    // Register migrations for enabled configs
    foreach ($form_state['object']->items as $name => $config) {

      if (!$config->disabled) {
        foreach ($config->config as $component => $def) {
          if (!strcmp('media', $component)) { $class = 'MediaMigration';
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
}
