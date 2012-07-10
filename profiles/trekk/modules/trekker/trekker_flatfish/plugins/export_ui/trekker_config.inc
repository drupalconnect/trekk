<?php
$plugin = array(
  'schema' => 'trekker_config',
  'access' => 'access content', //TODO UPDATE
  'menu' => array(
    'menu item' => 'trekker-flatfish',
    'menu title' => 'Trekker Flatfish',
    'menu description' => 'Administer Trekker Flatfish configurations.',
  ),

  'title singular' => t('configuration'),
  'title plural' => t('configurations'),
  'title singular proper' => t('Trekker Flatfish configuration'),
  'title plural proper' => t('Trekker Flatfish configurations'),

  'form' => array(
    'settings' => 'trekker_flatfish_ctools_export_ui_form',
    'validate' => 'trekker_flatfish_ctools_export_ui_form_validate',
    'submit' => 'trekker_flatfish_ctools_export_ui_form_submit',
  ),

  'handler' => 'trekker_config',
);

function trekker_flatfish_ctools_export_ui_form(&$form, &$form_state) {
  $config = $form_state['item'];

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
      '#description' => 'Type of Migration to run',
      '#tree' => TRUE,
      'components' => array(
        '#type' => 'radios',
        '#title' => t('Enable'),
        '#options' => drupal_map_assoc(array(t('Node'), t('Media'))),
        '#default_value' => ($config->config)? array_filter($config->config['components']): array(),
      ),
    ),
  );
  dpm($form);
}

// FIX ERROR HERE (array_filter).  ONE MIGRATION PER CONFIG OR MULTIPLE???
function trekker_flatfish_ctools_export_ui_form_validate(&$form, &$form_state) {
	foreach ($form_state['values']['options'] as $optgroup => $options) {
		$form_state['values']['options'][$optgroup] = array_filter($options);
	}
}

function trekker_flatfish_ctools_export_ui_form_submit(&$form, &$form_state) {
	$form_state['item']->config = $form_state['values']['options'];
}
