<?php

/**
 * Define this Export UI plugin.
 */
$plugin = array(
  'schema' => 'trekk_client_aggregator',  // As defined in hook_schema().
  'access' => 'administer trekk_client',  // Define a permission users must have to access these pages.

  // Define the menu item.
  'menu' => array(
    'menu prefix' => 'admin/config/services',
    'menu item' => 'trekk_client',
    'menu title' => 'Trekk Client',
    'menu description' => 'Administer Trekk Client configs.',
  ),

  // Define user interface texts.
  'title singular' => t('config'),
  'title plural' => t('configs'),
  'title singular proper' => t('Trekk client preset'),
  'title plural proper' => t('Trekk client presets'),

  // Define the names of the functions that provide the add/edit forms.
  'form' => array(
    'settings' => 'trekk_client_ctools_export_ui_form',
    'validate' => 'trekk_client_ctools_export_ui_form_validate',
    // 'submit' and 'validate' are also valid callbacks.
  ),
);

/**
 * Principal aggregator config form builder. This form is set up to fetch configuration
 * for the chosen components. Any class recognized to implement the necessary interfaces
 * will be available for selection. When a class is selected for a given component its
 * configuration form is pulled in here.
 */
function trekk_client_ctools_export_ui_form(&$form, &$form_state) {
  $config =& $form_state['item'];
    	
  // Classes that implement the following interfaces are available for selection
  $interfaces = array(
    'source' => 'TrekkClientAggregatorSourceInterface',
    'destination' => 'TrekkClientAggregatorDestinationInterface',
    'mapping' => 'TrekkClientAggregatorMappingInterface',
    );

  $query = db_select('registry', 'r');
  $result = $query
    ->fields('r', array('name', 'filename', 'module'))
    ->condition('type', 'class')
    ->condition('filename', '%.test', 'NOT LIKE')
    ->execute();

  // Collate our classes by the components they implement
  foreach ($result as $record) {
  	$class = $record->name;
  	
  	try { $reflector = new ReflectionClass($class); } catch (Exception $e) { continue; }

    foreach ($interfaces as $component => $interface) {
    	// Build the options
      if ($reflector && $reflector->implementsInterface($interface)) {
      	$info = call_user_func(array($class, 'info'));
      	$options[$component . '_options'][$class] = $info['name'];
      }
    }

    $interface = 'TrekkClientAggregatorSourceInterface';
  }

  extract($options);

  $form['name'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $config->name,
  );

  foreach (array('source', 'destination', 'mapping') as $section) {

	  // Pull in the section configuration form
	  $form[$section] = array(
	    '#type' => 'fieldset',
	    '#title' => t(ucwords($section)),
	    '#collapsible' => TRUE,
	    '#collapsed' => FALSE,
	    '#prefix' => '<div id="' . $section . '">',
	    '#suffix' => '</div>',
	    );

    // Present available classes in a select
	  $form[$section][$section . '_class'] = array(
	    '#type' => 'select',
	    '#title' => t(ucwords($section) . ' handler'),
	    '#default_value' => isset($config->$section) ? get_class($config->$section) : '',
	    '#options' => isset(${$section . '_options'}) ? ${$section . '_options'} : array(),
	    '#empty_option' => '--' . t('Select one') . '--',
	    '#empty_value' => '',
	    '#ajax' => array(
	      'callback' => 'trekk_client_ctools_export_ui_form_' . $section,
	      'wrapper' => $section,
	      'method' => 'replace',
	      'effect' => 'fade',
	      ),
	    );
	  
	  // If a class has been selected then pull in its config form  
	  if (is_object($form_state['item']->$section)) {
	  	$form[$section][$section . '_options'] = $form_state['item']->{$section}->configForm($form, $form_state, $form_state['item']->{$section}, $op = 'edit');
	  }

  } // end foreach
  
  return $form;
}

/**
 * Config form validation handler
 */
function trekk_client_ctools_export_ui_form_validate($form, &$form_state) {
  foreach (array('source', 'destination', 'mapping') as $section) {
  	if (!is_object($form_state['item']->$section) && !empty($form_state['values'][$section . '_class'])) {
  		$class = $form_state['values'][$section . '_class'];
  		$form_state['item']->$section = new $class();
  	}
  	if (is_callable(array($form_state['item']->$section, 'configFormValidate'))) {
  	  $form_state['item']->$section->configFormValidate($form, $form_state);
  	}
  }
}

/**
 * Ajax callback. Updates the source section of the admin form
 */
function trekk_client_ctools_export_ui_form_source($form, &$form_state) {
	return $form['source'];
}

/**
 * Ajax callback. Updates the destination section of the admin form
 */
function trekk_client_ctools_export_ui_form_destination($form, &$form_state) {
	return $form['destination'];
}

/**
 * Ajax callback. Updates the mapping section of the admin form
 */
function trekk_client_ctools_export_ui_form_mapping($form, &$form_state) {
	return $form['mapping'];
}