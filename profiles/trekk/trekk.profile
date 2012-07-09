<?php
/**
 * @file
 *
 * Enable selection of site type (client|server) and similar options.
 *
 */

/**
 * Implements hook_form_FORM_ID_alter() for install_configure_form().
 *
 * Allows the profile to alter the site configuration form.
 */
function trekk_form_install_configure_form_alter(&$form, $form_state) {
  // Pre-populate the site name with the server name.
  $form['site_information']['site_name']['#default_value'] = 'TREKK TESTER';
}

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
    '#default_value' => 1
  );

  $form['add_demo_content'] = array(
    '#type' => 'checkbox',
    '#title' => "Add Demo Content",
    '#description' => 'Only applicable for Servers.',
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
    module_enable(array('migrate', 'migrate_ui', 'trekker_flatfish'));
  }
  if ('client' == $form_state['values']['site_type']) {
    module_enable(array('migrate', 'migrate_ui', 'trekker_client'));
  }
  else {
    module_enable(array('services', 'trekker_server'));
  }

  // only do this for servers
  if ('server' == $form_state['values']['site_type'] && $form_state['values']['add_demo_content']) {
    $bodies = array(
      'Something pithy!',
      'I <3 Drupal!',
      'Happy 4th of July!',
      'Yay!',
      'These are random statements...'
    );

    for ($i = 0; $i < 5; $i++) {
      // Create the new node.
      $node = (object) array('type' => 'pages');
      node_object_prepare($node);
      $node->title = "Page $i"; 
      $node->body[LANGUAGE_NONE]['value'] = $bodies[rand(0,4)]; 
      $node->uid = 1;
      // Make sure we set the default language
      $node->language = LANGUAGE_NONE;
      node_save($node);
    }
  }

}
