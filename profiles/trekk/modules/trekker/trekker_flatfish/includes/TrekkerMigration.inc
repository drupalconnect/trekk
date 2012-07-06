<?php
/**
 * TrekkerMigration abstract base class, extends DynamicMigration
 *
 * Does a lot of the work for the Node and Media subclasses
 * 
 */

abstract class TrekkerMigration extends DynamicMigration {

  /**
   * Constructor
   *
   * Get Config, Mapping, Source, and Desitination
   * Check for mismatches between source and mappings
   */
	public function __construct(Array $arguments) {
		$this->arguments = $arguments;

    $config = $this->getConfig();

    parent::__construct(MigrateGroup::getInstance($arguments['machine_name']));
    
    $this->initMap();
    $this->initSource();
    $this->initDestination();
    
    // BAIL IF:
    // There are more items in the source than have been mapped.
    if ($this->processedCount() < ($this->sourceCount() - $this->errorCount())) {
      drupal_set_message(t('Migration !name NOT ready for updates (system of record set to SOURCE)'
        , array('!name' => "<em>" . $this->machineName . "</em>")));
    	$this->systemOfRecord = Migration::SOURCE;     
    }
    // There are as many items in the source as have been mapped
    else if ($this->processedCount() >= ($this->sourceCount() - $this->errorCount())) {
      drupal_set_message(t('Migration !name ready for updates (system of record set to DESTINATION)'
        , array('!name' => "<em>" . $this->machineName . "</em>")));
    	$this->systemOfRecord = Migration::DESTINATION;
    }
    
    $this->buildFieldMappings();
	}

  /**
   * Load the confuration object (ctools exportable)
   */
  public function getConfig() {
  	ctools_include('export');
  	
  	$configs = ctools_export_load_object('trekker_config', 'names', array($this->arguments['machine_name']));

  	$config = isset($configs[$this->arguments['machine_name']]) ? $configs[$this->arguments['machine_name']]->config[$this->arguments['component']] : NULL;

    //module_load_include('php', 'trekker', 'lib/spyc');

  	return $this->config = (array)$config;
  }
 
  /**
   * Create a new MigrateSQLMap
   */
	public function initMap() {
    $this->map = new MigrateSQLMap($this->machineName, $this->getSourceKeySchema(), MigrateDestinationNode::getKeySchema());    
	}

  public function initSource() {
    // setup the db conn
    $config = $this->getConfig();
    Database::addConnectionInfo($name, 'default', array(
      'driver'   => $config['driver'],
      'database' => $config['database'],
      'username' => $config['username'],
      'password' => $config['password'],
      'host'     => $config['host'],
      'prefix'   => $config['prefix'],
    ));
  }

	abstract public function initDestination();


	/**
   * Construct the machine name from the source file name.
   */
  protected function generateMachineName($class_name = NULL) {
    return strtolower(pathinfo($this->arguments['machine_name'] . '_' . $this->arguments['component'], PATHINFO_FILENAME));
  }
  
 
  /**
   * Boilerplate getter
   */
  public function getBaseName() {
  	return $this->arguments['machine_name'];
  }
  
  /**
   * Boilerplate getter
   */
  public function getComponentName() {
  	return $this->arguments['component'];
  }
  
  /**
   * Build field mappings. This comes from config, where the mapping (source_field, dest_field) together with callbacks,
   * arguments, etc., are declared
   */
  public function buildFieldMappings() {

    // build standard url, title, path, body for content types
    // mappings need to assume defaults!
    // source name == destination name
    // updatable == true
    // format == full_html
    // filter_html == true
    //
    // media doesn't need a mapping b/c it's always the same


  	$config = $this->getConfig();
  	
    // grab from schema 
  	foreach ($config['mapping'] as $def) {
      if (!isset($def['allow_update']) && $this->systemOfRecord == Migration::DESTINATION) {
        drupal_set_message(t('Destination field !destination_field will not be updated for !name'
          , array('!destination_field' => $def['destination_field'], '!name' => "<em>" . $this->machineName . "</em>")), 'warning');
      	continue;
      }  		
  		
      // NOTE: source == destination
    	$mapping = $this->addFieldMapping($def['source_field'], $def['source_field']);
    	
    	if (isset($def['arguments']) && !empty($def['arguments'])) {
    		$mapping->arguments($def['arguments']);
    	}
    	
    	$callbacks = isset($def['callbacks']) && is_array($def['callbacks']) ? $def['callbacks'] : array();
    	
    	foreach ($callbacks as $callback) {
    	  $mapping->addCallback($callback);
    	}
    	
    	// Add the processor if requested
    	if ($def['process_html']) {
    		$mapping->addCallback(array($this, 'processHTML'));
    	}
    }
  }
  
  public function getSourceKeySchema() {
  	$config = $this->getConfig();
  	$source_key_schema = array();
    foreach ($config['source']['primary key'] as $keyfield) {
      $source_key_schema[$keyfield] = $config['source']['fields'][$keyfield];
    }
    
    return $source_key_schema;
  }
  
  public function getSourceFields() {
  	$config = $this->getConfig();
  	return array_keys($config['source']['fields']);
  }
  
  /**
   * Load an entity from a previous migration.
   */
  public function getMappedEntity($source_migrations, $source_keys, $default = NULL) {
    foreach ($source_migrations as $source_migration) {
    	$result = $this->handleSourceMigration($source_migrations, $source_keys, $default);
    	if ($result) {
    	  $sourceMigration = Migration::getInstance($source_migration);
    	  if (!is_callable(array($sourceMigration->destination, 'getEntityType'))) {
    	    continue;
    	  }
		    $entityType = $sourceMigration->destination->getEntityType();
		    return array_shift(entity_load($entityType, array($result)));
		  }
    }
  	
  	return FALSE;
  }
  
  /**
   * Process the Source's HTML
   *
   * @param $html
   *   The HTML to be processed 
   */
  public function processHTML($html) {
  	return new HTMLProcessor($this, $html)->execute;
  }
  
  /**
   * We have to override an entire method just to be able to use our own Mapper class.
   */
  public function addFieldMapping($destination_field, $source_field = NULL) {
    // Warn if duplicate mappings
    if (!is_null($destination_field) && isset($this->fieldMappings[$destination_field])) {
      self::displayMessage(
        t('!name addFieldMapping: !dest was previously mapped, overridden',
          array('!name' => $this->machineName, '!dest' => $destination_field)),
        'warning');
    }
    $mapping = new MigrateFieldMappingAlt($destination_field, $source_field);
    if (is_null($destination_field)) {
      $this->fieldMappings[] = $mapping;
    }
    else {
      $this->fieldMappings[$destination_field] = $mapping;
    }
    return $mapping;
  }

  /**
   *
  public function prepareRow($row) {
  	return; //<---------------------------- DEPRECATED???
  	// Skip a row if skip condition is met.
  	if (!$this->systemOfRecord == Migration::DESTINATION) {
  		return;
    }
    
    // Get the destination row that corresponds to this source row    
	  $keys = $this->source->getCurrentKey();
	  $destIds = (array)$this->map->lookupDestinationID($keys);

    // Don't proceed unless an entity was loaded
    if (!$destIds) {
    	return;
    }
    
    // This check works only on entities.
	  if (!is_callable(array($this->destination, 'getEntityType'))) {
	  	return;
	  }
	  
	  $entityType = $this->destination->getEntityType();
	  
	  // Load the destination entity
	  $ids = array_filter(array_values($destIds));
	  if (!count($ids)) {
	  	return;
	  }
	  
    $entities = entity_load($entityType, $ids);
    if (!$entities) {
    	return;
    }
    $entity = array_shift($entities);

    // Check the skip condition
    $field_name = 'field_qa_status';
    $skip_value = 209;
    $values = field_get_items($entityType, $entity, $field_name, $entity->language);
    if ($values) {
    	// shouldn't there be a decent way to get a field's value?
      foreach ($values as $value) {
        if (in_array($skip_value, $value)) {
        	drupal_set_message('Skipped ' . $ids);
      	  return FALSE;
        }
      }
    }
    
  }
   */
}

/**
 * 
 */
class MigrateFieldMappingAlt extends MigrateFieldMapping {
  protected $callbacks = array();
  
  public function __construct($destination_field, $source_field) {
    parent::__construct($destination_field, $source_field);
  }
  
  public function addCallback($callback) {
    $this->callbacks += array($callback);
  }
}
