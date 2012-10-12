<?php
/**
 * The mapping base class provides for building field mappings apart from the main migration / aggregator
 * class. In a typical migration, one would build the mappings using $this->addFieldMapping() and friends
 * in the constructor of the migration class. Not so, here. Mappings are entirely declarative. They come
 * in as configuration and get built into the main migration class when buildMapping() gets called.
 */
interface TrekkClientAggregatorMappingInterface {
  /**
   * All implementing classes need to provide info for use on configuration forms.
   */
  public static function info();

  /**
   * All implementing classes need to call this class's __sleep() to make sure options get serialized. For
   * example:
   * public function __sleep() {
   *   return parent::__sleep()
   * }
   */
	public function __sleep();
}

abstract class TrekkClientMapping {
  public $sourceMigrations;

	public function __sleep() {
		return array('sourceMigrations');
	}

	public function __construct($config) {
		$this->sourceMigrations = $config['sourceMigrations'];
	}

  /**
   * The aggregator that composed this object, passed via buildMapping().
   */
  protected $_aggregator;

  public static function info() {
  }

	/**
 	 * Builder for our options form.
 	 */
	public function configForm($form, &$form_state, $entity, $op = 'edit') {
  	$form = array();

    $mappings = $this->getMappings();

    //$aggregators = entity_load('aggregator_config');
    $aggregators = (ctools_export_crud_load_all('trekk_client_aggregator'));

    $options = array();

    foreach ($aggregators as $aggregator) {
    	$options[$aggregator->name] = $aggregator->name;
    }

    // Build a form element for selecting the source migration on fields that want one
    foreach ($mappings as $mapping) {
    	if (!empty($mapping['sourceMigration'])) {
    		$form['source_migrations']['source_migration_' . $mapping['destinationField']] = array(
    		  '#type' => 'select',
    		  '#title' => t('Source for %dest (from source field(s) %source)', array('%dest' => $mapping['destinationField'], '%source' => implode(' and ' , $mapping['sourceField']))),
    		  '#default_value' => !empty($this->sourceMigrations['source_migration_' . $mapping['destinationField']]) ? $this->sourceMigrations['source_migration_' . $mapping['destinationField']] : '',
    		  '#options' => $options,
    		  '#empty_option' => '--' . t('Select one') . '--',
    		  '#empty_value' => '',
    		  );
    	}
    }

  	return $form;
  }

  /**
   * Validator for our options form
   */
  public function configFormValidate($form, &$form_state) {
  	// Save source migrations assigned to field mappings. These are named
  	// in $form_state as 'source_migration_foo'
  	foreach ($form_state['values'] as $key => $value) {
  		if (0 === strpos($key, 'source_migration_')) {
  			$field = substr($key, strlen('source_migration_') - 1);
  			$this->sourceMigrations[$key] = $form_state['values'][$key];
  		}
  	}
  }

  /**
   * Implementors are expected to return an array of mappings. The structure of the array is
   * pretty much identical to how it ends up in the Migration class itself.
   */
  abstract public function getMappings();

  /**
   * Default implementation of buildMapping(). Here we read through the mappings declared in the mapping class
   * and execute addFieldMapping() on the composing migration / aggregator class. Note that our mappings allow
   * greater flexibility in handling multiple incoming values. See class TrekkClientAggregator's
   * reimplementation of addFieldMapping().
   */
  public function buildMapping($aggregator) {
  	$this->_aggregator = $aggregator;
  	
  	foreach ($this->getMappings() as $mapping) {
  		$mapping = (object)$mapping;
  		
		  $handler = $this->_aggregator->addFieldMapping($mapping->destinationField, $mapping->sourceField);
		  
		  // Apply declared callbacks
		  if (isset($mapping->callbacks)) {
		  	foreach ($mapping->callbacks as $callback) {
		  		$handler->callbacks($callback);
		  	}
		  }

      // Set up source migrations for this field
		  if (isset($mapping->sourceMigration)) {
		  	$source_migration = $this->sourceMigrations['source_migration_' . $mapping->destinationField];
		  	$handler->sourceMigration(!empty($source_migration) ? $source_migration : NULL);
		  }

		  if (isset($mapping->defaultValue)) {
		  	$handler->defaultValue($mapping->defaultValue);
		  }

		  if (isset($mapping->arguments)) {
		  	if (!isset($mapping->arguments['create_stubs'])) {
		  		$mapping->arguments['create_stubs'] = FALSE;
		  	}
		  	$handler->arguments($mapping->arguments);
		  }

		  if (isset($mapping->separator)) {
		  	$handler->separator($mapping->separator);
		  }
    }
  }
}
