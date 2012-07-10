<?php
/**
 * TrekkFlatfishMigration abstract base class, extends DynamicMigration
 *
 * Does a lot of the work for the Node and Media subclasses
 * 
 */

abstract class TrekkFlatfishMigration extends DynamicMigration {

  /**
   * Constructor
   *
   * Get Config, Mapping, Source, and Desitination
   * Check for mismatches between source and mappings
   */
  public function __construct(Array $arguments) {
    $this->arguments = $arguments;

    parent::__construct(MigrateGroup::getInstance($arguments['machine_name'] . '_' . $arguments['component']));
    $config = $this->getConfig();
    if (empty($config) || empty($arguments)) {
      dpm("Could not find your configuration file, no migrations were performed", null, 'error');
      $this->systemOfRecord = Migration::SOURCE;
    }


    // key is always URL
    $this->schemaKey = array(
      'url' => array(
        'type' => 'varchar',
        'length' => 255,
      )
    );

    $this->initMap();
    $this->initSource();
    $this->initDestination();

/*
    // ERROR IF:
    // There are more items in the source than have been mapped.
    if ($this->processedCount() < ($this->sourceCount() - $this->errorCount())) {
      drupal_set_message(t('Migration !name NOT ready for updates (system of record set to SOURCE)'
        , array('!name' => "<em>" . $this->getMachineName() . "</em>")));
      $this->systemOfRecord = Migration::SOURCE;
    }
    // There are as many items in the source as have been mapped
    else if ($this->processedCount() >= ($this->sourceCount() - $this->errorCount())) {
      drupal_set_message(t('Migration !name ready for updates (system of record set to DESTINATION)'
        , array('!name' => "<em>" . $this->getMachineName() . "</em>")));
      $this->systemOfRecord = Migration::DESTINATION;
    }
*/

    $this->buildFieldMappings();
  }

  /**
   * Migrate needs this
   */
  public function getMachineName() {
    return strtolower(pathinfo($this->arguments['machine_name'] . '_' . $this->arguments['component'], PATHINFO_FILENAME));
  }

  /**
   * Migrate needs this
   */
  protected function generateMachineName($class_name = NULL) {
    return strtolower(pathinfo($this->arguments['machine_name'] . '_' . $this->arguments['component'], PATHINFO_FILENAME));
  }


  /**
   * Load the confuration object (ctools exportable)
   */
  public function getConfig() {
    ctools_include('export');
    $configs = ctools_export_load_object('trekk_flatfish_config', 'names', array($this->arguments['machine_name']));
    return $this->config = (array)$configs;
  }

  /**
   * Create a new MigrateSQLMap
   */
  public function initMap() {
    $this->map = new MigrateSQLMap($this->machineName, $this->schemaKey, MigrateDestinationNode::getKeySchema());
  }

  public function initSource() {
    // setup the db conn
    $config = $this->getConfig();
    $conn = $config[$this->getBaseName()]->config;

    Database::addConnectionInfo('flatfish', 'default', array(
      'driver'   => $conn['driver'],
      'database' => $conn['database'],
      'username' => $conn['username'],
      'password' => $conn['password'],
      'host'     => $conn['host'],
      'prefix'   => $conn['prefix'],
    ));
  }

  abstract public function initDestination();

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

    $config = $this->getConfig();
    if ('media' == strtolower($this->getComponentName())) {
      // MEDIA
      $mapping = $this->addFieldMapping('id','id'); // necessary???
      $mapping = $this->addFieldMapping('contents','contents');
      $mapping->arguments(array('copy_blob' => TRUE));
      $mapping = $this->addFieldMapping('filename','filename'); //necessary???
    }
    else {
      foreach ($this->getSourceFields() as $def) {
        if ('id' == $def) {
          continue; // skip our key
        }
        $mapping = $this->addFieldMapping($def, $def);
        // don't processHTML for media fields or the path and title
        if (!in_array($def, array('path','title'))) {
          $mapping->arguments(array('format' => 'full_html'));
          $mapping->addCallback(array($this, 'processHTML'));
        }
      }
    }
  }

  public function getSourceFields() {
    $config = $this->getConfig();
    $keys = array_keys($config[$this->getBaseName()]->config[$this->getBaseName()]['fields']);
    return $keys;
  }

  /**
   * Load an entity from a previous migration.
   */
  public function getMappedEntity($source_migrations, $source_keys, $default = NULL) {
    foreach ($source_migrations as $source_migration) {
      //handleSourceMigration('article_media', array('id' => #));
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
    $processor = new HTMLProcessor($this, $html);
    return $processor->execute();
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
