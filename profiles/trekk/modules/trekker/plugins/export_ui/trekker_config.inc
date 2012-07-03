<?php
$plugin = array(
  'schema' => 'trekker_config',
  'access' => 'access content',
  'menu' => array(
    'menu item' => 'trekker',
    'menu title' => 'Trekker',
    'menu description' => 'Administer Trekker configurations.',
  ),
  
  //'default hook' => 'default_trekker_trekker_config',

  'title singular' => t('configuration'),
  'title plural' => t('configurations'),
  'title singular proper' => t('Trekker configuration'),
  'title plural proper' => t('Trekker configurations'),

  'form' => array(
    'settings' => 'trekker_ctools_export_ui_form',
    'validate' => 'trekker_ctools_export_ui_form_validate',
    'submit' => 'trekker_ctools_export_ui_form_submit',
  ),
  
  'handler' => 'trekker_config',
);

function trekker_ctools_export_ui_form(&$form, &$form_state) {
  $config = $form_state['item'];
  $form['schema'] = array(
    '#type' => 'fieldset',
    '#title' => t('Config'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    'config' => array(
      '#type' => 'markup',
      '#markup' => dprint_r($config->config, TRUE),
    ),
  );
  
  $form += array(
    'description' => array(
      '#type' => 'textfield',
      '#title' => t('Description'),
      '#default_value' => $config->description,
    ),
    'group_name' => array(
      '#type' => 'textfield',
      '#title' => t('Group'),
      '#default_value' => $config->group_name,
    ),
    'options' => array(
      '#type' => 'fieldset',
      '#title' => t('Components'),
      '#tree' => TRUE,
      'components' => array(
        '#type' => 'checkboxes',
        '#title' => t('Enable'),
        '#options' => drupal_map_assoc(array(t('HTML'), t('Files'))),
        '#default_value' => array_filter($config->config['components']),
        ),
      ),

  );
  dpm($form);
}

function trekker_ctools_export_ui_form_validate() {
	foreach ($form_state['values']['options'] as $optgroup => $options) {
		$form_state['values']['options'][$optgroup] = array_filter($options);
	}
}

function trekker_ctools_export_ui_form_submit(&$form, &$form_state) {
	$form_state['item']->config = $form_state['values']['options'];

}
