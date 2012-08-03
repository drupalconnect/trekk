<?php
/**
 * @file
 *
 * Enable selection of site type (client|server) and similar options.
 *
 */

/**
 * Implements hook_install_tasks().
 */
function trekk_install_tasks() {
  $tasks = array();

  $tasks['trekk_site_type_form'] = array(
    'display_name' => st('Set up Trekk Specifics'),
    'type' => 'form',
  );

  return $tasks;
}

/**
 * Trekk Installation Specifics Form
 */
function trekk_site_type_form() {
  drupal_set_title(st('Setting Up Your Trekk Site'));

  $options = array(
    'client' => st('Client'),
    'server' => st('Server'),
  );

  $form['site_type'] = array(
    '#type' => 'radios',
    '#title' => st('Site Type'),
    '#description' => st("You can choose either a <em>Client</em> or a <em>Server</em>.  Clients receive content from the Server(s), it's a one-way flow but allows for incremental updates and Client overrides."),
    '#options' => $options,
    '#default_value' => 'client'
  );

  $form['enable_flatfish'] = array(
    '#type' => 'checkbox',
    '#title' => "Enable Flatfish Support",
    '#description' => st('<a href="https://github.com/drupalstaffing/flatfish">Flatfish</a> is a library for scraping content from legacy HTML and importing into Drupal.'),
    '#default_value' => 1
  );

  $form['add_roles_workflow'] = array(
    '#type' => 'checkbox',
    '#title' => "Add Roles & Workflow",
    '#description' => 'Add basic set of Roles and simple, initial Workflow.',
    '#default_value' => 0
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => st('Create and continue'),
    '#weight' => 15,
  );

  return $form;
}

/**
 * Trekk Specifics form submit handler 
 */
function trekk_site_type_form_submit(&$form, &$form_state) {
  // enable requested modules
  if ($form_state['values']['enable_flatfish']) {
    module_enable(array('migrate', 'migrate_ui', 'migrate_extras', 'trekk_flatfish'));
  }
  if ('client' == $form_state['values']['site_type']) {
    module_enable(array('migrate', 'migrate_ui', 'trekk_client'));
  }
  else {
    module_enable(array('services', 'trekker_server'));
  }

  if ($form_state['values']['add_roles_workflow']) {
    module_enable(array('trekk_roles')); // add trekk_workflow feature here
  }

}
