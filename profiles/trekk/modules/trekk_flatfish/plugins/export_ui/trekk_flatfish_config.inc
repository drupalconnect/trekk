<?php
$plugin = array(
  'schema' => 'trekk_flatfish_config',
  'access' => 'access content', //TODO UPDATE
  'menu' => array(
    'menu item' => 'trekk-flatfish',
    'menu title' => 'Trekk Flatfish',
    'menu description' => 'Administer Trekk Flatfish configurations.',
  ),

  'title singular' => t('configuration'),
  'title plural' => t('configurations'),
  'title singular proper' => t('Trekk Flatfish configuration'),
  'title plural proper' => t('Trekk Flatfish configurations'),

  'form' => array(
    'settings' => 'trekk_flatfish_ctools_export_ui_form',
    'validate' => 'trekk_flatfish_ctools_export_ui_form_validate',
    'submit' => 'trekk_flatfish_ctools_export_ui_form_submit',
  ),

  'handler' => 'trekk_flatfish_config',
);

function trekk_flatfish_ctools_export_ui_form(&$form, &$form_state) {
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
      '#default_value' => (isset($config->group_name))? $config->group_name: '',
    ),
    'options' => array(
      '#type' => 'fieldset',
      '#title' => t('Components'),
      '#description' => 'Type of Migration to run',
      '#tree' => TRUE,
      'components' => array(
        '#type' => 'checkboxes',
        '#title' => t('Enable'),
        '#options' => drupal_map_assoc(array(t('Node'), t('Media'))),
        '#default_value' => (isset($config->config['components']))? array_filter($config->config['components']): array(),
      ),
    ),
  );
}

function trekk_flatfish_ctools_export_ui_form_validate(&$form, &$form_state) {
  foreach ($form_state['values']['options'] as $optgroup => $options) {
    $form_state['values']['options'][$optgroup] = array_filter($options);
  }
}

function trekk_flatfish_ctools_export_ui_form_submit(&$form, &$form_state) {
  $form_state['item']->config['components'] = $form_state['values']['options']['components'];
}
