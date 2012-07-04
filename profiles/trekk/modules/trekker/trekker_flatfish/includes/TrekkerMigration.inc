<?php
abstract class TrekkerMigration extends DynamicMigration {
	public function __construct(Array $arguments) {
		$this->arguments = $arguments;

    $config = $this->getConfig();

    parent::__construct(MigrateGroup::getInstance($arguments['machine_name']));
    
    $this->initMap();
    $this->initSource();
    $this->initDestination();
    
    // There are more items in the source than have been mapped.
    if ($this->processedCount() < ($this->sourceCount() - $this->errorCount())) {
    	drupal_set_message(t('Migration !name NOT ready for updates (system of record set to SOURCE)', array('!name' => "<em>" . $this->machineName . "</em>")));
    	$this->systemOfRecord = Migration::SOURCE;     
    }
    // There are as many items in the source as have been mapped
    else if ($this->processedCount() >= ($this->sourceCount() - $this->errorCount())) {
    	drupal_set_message(t('Migration !name ready for updates (system of record set to DESTINATION)', array('!name' => "<em>" . $this->machineName . "</em>")));
    	$this->systemOfRecord = Migration::DESTINATION;
    }
    
    $this->buildFieldMappings();
	}
	
	abstract public function initSource();
	abstract public function initDestination();
	
	public function initMap() {
    $this->map = new MigrateSQLMap($this->machineName, $this->getSourceKeySchema(), MigrateDestinationNode::getKeySchema());    
	}
	
	/**
   * Construct the machine name from the source file name.
   */
  protected function generateMachineName($class_name = NULL) {
    return strtolower(pathinfo($this->arguments['machine_name'] . '_' . $this->arguments['component'], PATHINFO_FILENAME));
  }
  
  /**
   * Load the confuration object (ctools exportable)
   */
  public function getConfig() {
  	ctools_include('export');
  	
  	$configs = ctools_export_load_object('trekker_config', 'names', array($this->arguments['machine_name']));

  	$config = isset($configs[$this->arguments['machine_name']]) ? $configs[$this->arguments['machine_name']]->config[$this->arguments['component']] : NULL;
    module_load_include('php', 'trekker', 'lib/spyc');

  	return $this->config = (array)$config;
  }
  
  public function getBaseName() {
  	return $this->arguments['machine_name'];
  }
  
  public function getComponentName() {
  	return $this->arguments['component'];
  }
  
  /**
   * Build field mappings. This comes from config, where the mapping (source_field, dest_field) together with callbacks,
   * arguments, etc., are declared
   */
  public function buildFieldMappings() {
  	$config = $this->getConfig();
  	
  	foreach ($config['mapping'] as $def) {
      if (!isset($def['allow_update']) && $this->systemOfRecord == Migration::DESTINATION) {
      	drupal_set_message(t('Destination field !destination_field will not be updated for !name', array('!destination_field' => $def['destination_field'], '!name' => "<em>" . $this->machineName . "</em>")), 'warning');
      	continue;
      }  		
  		
    	$mapping = $this->addFieldMapping($def['destination_field'], $def['source_field']);
    	
    	if (isset($def['arguments']) && !empty($def['arguments'])) {
    		$mapping->arguments($def['arguments']);
    	}
    	
    	$callbacks = isset($def['callbacks']) && is_array($def['callbacks']) ? $def['callbacks'] : array();
    	
    	foreach ($callbacks as $callback) {
    	  $mapping->addCallback($callback);
    	}
    	
    	// Add the scraper if it's asked for
    	if ($def['filter_html']) {
    		$mapping->addCallback(array($this, 'filterHTML'));
    	}
    }
  }
  
  public function getSourceKeySchema() {
  	$source_key_schema = array();
  	
  	$config = $this->getConfig();

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
   * Execute HTML filters on source content
   */
  public function filterHTML($html) {
  	$scraper = HTMLScraper::init($this, $html);
    $scraper->addFilter('MediaFilter', $options = array());
    return $scraper->execute();
  }
  
  /**
   * Seriously. We have to override an entire method just to be able to use our own Mapper class, which
   * we wouldn't need to do if the provided mapper were decently designed. Seems like the class of the 
   * mapper we want to use should be in a SETTING somewhere, or at least passed into this method as an 
   * argument.
   */
  public function addFieldMapping($destination_field, $source_field = NULL) {
    // Warn of duplicate mappings
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

  public function prepareRow($row) {
  	return;
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
