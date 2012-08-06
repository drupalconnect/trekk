<?php
interface TrekkClientAggregatorInterface {
  public function __construct($config, $options = array());
}

class TrekkClientAggregatorConfig extends Entity {
  public $name;
  public $source;
  public $destination;
  public $mapping;

  function label() {
    return $this->name;
  }

  /**
   * Returns the full url() for the aggregator.
   */
  public function url() {
    $uri = $this->uri();
    return url($uri['path'], $uri);
  }

  /**
   * Returns the drupal path to this aggregator.
   */
  public function path() {
    $uri = $this->uri();
    return $uri['path'];
  }

  public function defaultUri() {
    return array(
      'path' => 'admin/config/services/aggregators/' . $this->aid . '/view',
      );
  }

  function isLocked() {
    return isset($this->status) && empty($this->is_new) && (($this->status & ENTITY_IN_CODE) || ($this->status & ENTITY_FIXED));
  }

  public function save() {
    parent::save();
    TrekkClientAggregator::reRegisterMigration('TrekkClientAggregator', $this->machine_name, array(
      'machine_name' => $this->machine_name,
      'source' => $this->source,
      'destination' => $this->destination,
      'mapping' => $this->mapping));
  }

  public function delete() {
    parent::delete();
    MigrationBase::deregisterMigration($this->machine_name);
  }


  public function buildContent($view_mode, $langcode = 'und', $content = array()) {
    module_load_include('inc', 'trekk_client', 'trekk_client.admin.aggregator');
    $config = (array)$this;
    $aggregator = MigrationBase::getInstance($this->machine_name, $class = NULL, $config);

    //$result = $aggregator->processRollback();
    //$result = $aggregator->processImport(array('limit' => 5));

    // Ops
    $machine_name = $aggregator->getMachineName();
    $content['dashboard'] = drupal_get_form('trekk_client_aggregator_dashboard', $machine_name);

    // General statistics

    $has_counts = TRUE;
    if (is_callable(array($aggregator, 'sourceCount'))) {
      $total = $aggregator->sourceCount();
    }
    else {
      $has_counts = FALSE;
      $total = t('N/A');
    }
    if (is_callable(array($aggregator, 'importedCount'))) {
      $imported = $aggregator->importedCount();
    }
    else {
      $has_counts = FALSE;
      $imported = t('N/A');
    }
    if ($has_counts) {
      $unimported = $total - $imported;
    }
    else {
      $unimported = t('N/A');
    }
    $status = $aggregator->getStatus();
    switch ($status) {
      case MigrationBase::STATUS_IDLE:
        $status = t('Idle');
        break;
      case MigrationBase::STATUS_IMPORTING:
        $status = t('Importing');
        break;
      case MigrationBase::STATUS_ROLLING_BACK:
        $status = t('Rolling back');
        break;
      case MigrationBase::STATUS_STOPPING:
        $status = t('Stopping');
        break;
      case MigrationBase::STATUS_DISABLED:
        $status = t('Disabled');
        break;
      default:
        $status = t('Unknown');
        break;
    }

    $header = array(
      'status' => array('data' => t('Status')),
      'importrows' => array('data' => t('Total rows')),
      'imported' => array('data' => t('Imported')),
      'unimported' => array('data' => t('Unimported')),
      'lastimported' => array('data' => t('Last imported')),
    );

    $row['status'] = $status;
    $row['importrows'] = $total;
    $row['imported'] = $imported;
    $row['unimported'] = $unimported;
    $row['lastimported'] = $aggregator->getLastImported();
    $rows[] = $row;

    $content['summary'] = array(
      '#type' => 'fieldset',
      '#title' => t('Summary'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      );
    $content['summary']['info'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      );

    // Map table details
    $map = $aggregator->getMap();
    $source = $aggregator->getSource();

    if (is_callable(array($map, 'getRowBySource')) && is_callable(array($source, 'getIdList'))) {
      $idList = array();
      $destIds = array();

      $source->setCachePolicy(FALSE);
      $idList = $source->getIdList($reset = TRUE);

      $update_count = 0;
      foreach ($idList as $sourceId) {
        $lookupId = !is_array($sourceId) ? array($sourceId) : $sourceId;
        $destId = $map->getRowBySource($lookupId);

        $needs_update = $destId['needs_update'];
        unset($destId['needs_update']);

        $last_imported = $destId['last_imported'];
        unset($destId['last_imported']);

        $destIds[] = array(
          'sourceid' => implode(', ', $lookupId),
          // The $destId may not be an array, so ignore values that will break implode():
          'destid' => (is_array($destId)) ? implode(', ', $destId) : '',
          'last_imported' => $last_imported,
          'needs_update' => $needs_update,
        );
        if ($needs_update) {
          $update_count++;
        }
      }

      $header = array(
        'sourceids' => array('data' => t('Source ID')),
        'destids' => array('data' => t('Destination ID')),
        'last_imported' => array('data' => t('Last imported')),
        'needs_update' => array('data' => t('Needs update (@count records)', array('@count' => $update_count))),
        );

      $content['map_details'] = array(
        '#type' => 'fieldset',
        '#title' => t('Map details'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        );

      $content['map_details']['map_table'] = array(
        '#theme' => 'table',
        '#header' => $header,
        '#rows' => $destIds,
        );
    }

    return $content;

  }
}

class TrekkClientAggregatorConfigUIController extends EntityDefaultUIController {
}


/**
 * Dynamic migration class for handling all aggregations. Everything we need is passed through config.
 * @todo: get rid of $options, since it's pretty much useless.
 */
class TrekkClientAggregator extends DynamicMigration implements TrekkClientAggregatorInterface {
  protected $config = NULL;
  
  public function __construct($args, $options = array()) {
    $this->args = $args;
    $this->options = $options;

    parent::__construct();

    $source_config = $this->getConfig('source');
    $source_class = get_class($source_config);
    $this->source = !empty($source_class) ? new $source_class((array)$source_config) : NULL;

    $destination_config = $this->getConfig('destination');
    $destination_class = get_class($destination_config);
    $this->destination = !empty($destination_class) ? new $destination_class((array)$destination_config) : NULL;

    $mapping_config = $this->getConfig('mapping');
    $mapping_class = get_class($mapping_config);
    $this->mapping = !empty($mapping_class) ? new $mapping_class((array)$mapping_config) : NULL;

    // This should be coming from config, but let's not get too crazy. We don't need it.
    $map_class = !empty($options['map_class']) ? $options['map_class'] : 'MigrateSQLMap';

    // Set up the map class for tracking the migration
    //$this->map = new $map_class();
    $this->map = new $map_class(
      $this->machineName,
      $this->source->getKeySchema(),
      $this->destination->getKeySchema()
    );

    // This is what we do in place of setting up field mappings, which in our case are
    // passed in through config and applied here.
    if ($this->mapping) {
      $this->mapping->buildMapping($this);
    }

  }

  /**
   * Generate a machine name for this migration. This comes from config ("arguments" in
   * migration speak) and is set in the constructor for this class.
   */
  protected function generateMachineName() {
    return $this->getConfig('name');
  }
  
  public function getConfig($key = NULL) {
    if (is_null($this->config)) {
      ctools_include('export');
      $name = $this->args['config'];
      $config = array_shift(ctools_export_load_object('trekk_client_aggregator', 'names', $args = array($name)));
      $this->config = (array)$config;  
    }
    
    if (!is_null($key) && isset($this->config[$key])) {
      return $this->config[$key];
    }
    
    return $this->config;
  }

  /**
   * Generic create stub. Unlike standard migration fare, we refer stub business
   * to the destination, which is where it should be (because it's the destination
   * that encapsulates the creation of destination entities).
   */
  public function createStub($aggregator) {
    if (is_callable(array($aggregator->destination, 'createStub'))) {
      return $aggregator->destination->createStub($aggregator);
    }
  }

  /**
   * Overrides Migration's default implementation. The only reason we have to do this is because
   * Migration's implementation is protected, but we need it to be public.
   */
  public function addFieldMapping($destination_field, $source_field = NULL) {
    // Warn of duplicate mappings
    if (!is_null($destination_field) && isset($this->fieldMappings[$destination_field])) {
      $this->showMessage(
        t('!name addFieldMapping: !dest was previously mapped, overridden',
          array('!name' => $this->machineName, '!dest' => $destination_field)),
        'warning');
    }

    $mapping = new MigrateFieldMapping($destination_field, $source_field);

    if (is_null($destination_field)) {
      $this->fieldMappings[] = $mapping;
    }
    else {
      $this->fieldMappings[$destination_field] = $mapping;
    }

    return $mapping;
  }

  /**
   * Apply field mappings to a data row received from the source, returning
   * a populated destination object. This is a boiler plate of the method
   * we're overriding, with the exception that we are capable of handling
   * multiple source keys in a mapping, e.g., for passing to a source migration
   * for lookup. This lets us avoid having to do dances in prepareRow().
   *
   * @todo: since we're overriding it, we may as well clean it up some. This is a butt
   * ugly mess of a method, here.
   */
  protected function applyMappings() {
    // Apply mappings.
    $this->destinationValues = new stdClass;

    foreach ($this->fieldMappings as $mapping) {
      $destination = $mapping->getDestinationField();

      // Skip mappings with no destination (source fields marked DNM)
      if (!$destination) {
        continue;
      }

      $source = $mapping->getSourceField();
      $default = $mapping->getDefaultValue();

      // If there's a source mapping, and a source value in the data row, copy
      // to the destination
      if (!empty($source) && !empty($this->sourceValues->$source)) {
        $this->destinationValues->$destination = $this->sourceValues->$source;
      }
      // Handle arrays of source values
      elseif ($source && (is_array($source) || is_object($source))) {
        // source like array('prop1', 'prop2') means fields directly on the incoming row.
        // source like array('prop1' => array('prop2', 'prop3') means fields on items nested into the row, under prop1

        // Determine whether we have an nested array
        if (count($source) != count($source, 1)) {

          // Deal with source like array('prop' => array('prop1', 'prop2')) where source keys are on an array of embedded
          // objects
          foreach ($source as $key => $keyvalues) {

            $destinationValues->{$key} = array();

            foreach ($this->sourceValues->{$key} as $i => $source_item) {
              $source_item = (array)$source_item;

              foreach ($keyvalues as $keyvalue) {
                $destinationValue[$keyvalue] = $source_item[$keyvalue];
              }

              if (!in_array(NULL, $destinationValue)) {
                $this->destinationValues->{$destination}[] = $destinationValue;
              }
            }
            $source = $key;
          }
          //dsm($this, 'this');
        }

        else {
          // Deal with source like array('prop1', 'prop2'), where the source is composed of multiple properties taken
          // directly from the incoming row
          $source_values = array();
          foreach ($source as $i => $key) {
            //dsm($key);
            //dsm($this->sourceValues);
            $keys[] = $this->sourceValues->{$key};
          }

          if (!in_array(NULL, $keys)) {
            $this->destinationValues->$destination = array($keys);
          }
        }
      }
      // Otherwise, apply the default value (if any)
      elseif (!is_null($default)) {
        $this->destinationValues->$destination = $default;
      }

      // If there's a separator specified for this destination, then it
      // will be populated as an array exploded from the source value
      $separator = $mapping->getSeparator();
      if ($separator && isset($this->destinationValues->$destination)) {
        $this->destinationValues->$destination = explode($separator, $this->destinationValues->$destination);
      }

      // If a source migration is supplied, use the current value for this field
      // to look up a destination ID from the provided migration
      $source_migration = $mapping->getSourceMigration();

      if ($source_migration && isset($this->destinationValues->$destination)) {
      	//dpm($this->destinationValues->$destination, 'looking up');
      	$arguments = $mapping->getArguments();
      	$allowStub = isset($arguments['create_stubs']) ? $arguments['create_stubs'] : FALSE;
        $this->destinationValues->$destination = $this->handleSourceMigration($source_migration, $this->destinationValues->$destination, $default, $this, $allowStub);
      }
      //dsm($source_migration, 'source_migration');

      // Call any designated callbacks
      $callbacks = $mapping->getCallbacks();
      foreach ($callbacks as $callback) {
        if (property_exists($this->destinationValues, $destination)) {
          $this->destinationValues->$destination = call_user_func($callback, $this->destinationValues->$destination);
        }
      }

      // If specified, assure a unique value for this property.
      $dedupe = $mapping->getDedupe();
      if ($dedupe && isset($this->destinationValues->$destination)) {
        $this->destinationValues->$destination = $this->handleDedupe($dedupe, $this->destinationValues->$destination, $mapping);
      }

      // Assign any arguments
      if (isset($this->destinationValues->$destination)) {
        $arguments = $mapping->getArguments();
        if ($arguments) {
          if (!is_array($this->destinationValues->$destination)) {
            $this->destinationValues->$destination = array($this->destinationValues->$destination);
          }
          // TODO: Stuffing arguments into the destination field is gross - can
          // we come up with a better way to communicate them to the field
          // handlers?
          $this->destinationValues->{$destination}['arguments'] = array();
          foreach ($arguments as $argname => $destarg) {
            if (is_array($destarg) && isset($destarg['source_field']) && property_exists($this->sourceValues, $destarg['source_field'])) {
              $this->destinationValues->{$destination}['arguments'][$argname] = $this->sourceValues->$destarg['source_field'];
            }
            elseif (is_array($destarg) && isset($destarg['default_value'])) {
              $this->destinationValues->{$destination}['arguments'][$argname] = $destarg['default_value'];
            }
            else {
              $this->destinationValues->{$destination}['arguments'][$argname] = $destarg;
            }
          }
        }
      }

      // When we're updating existing nodes, if there is a source mapping but there
      // was no value for this row, add a null destination value so it gets removed
      // from the node
      if ($this->systemOfRecord == Migration::DESTINATION && $source && !isset($this->destinationValues->$destination)) {
        $this->destinationValues->$destination = NULL;
      }

    }
  }

  /**
   * Look up a value migrated in another migration.
   *
   * @param mixed $source_migrations
   *   An array of source migrations, or string for a single migration.
   * @param mixed $source_keys
   *   Key(s) to be looked up against the source migration(s). This may be a simple
   *   value (one single-field key), an array of values (multiple single-field keys
   *   to each be looked up), or an array of arrays (multiple multi-field keys to
   *   each be looked up).
   * @param mixed $default
   *   The default value, if no ID was found.
   * @param $migration
   *   The implementing migration.
   * @return
   *   Destination value(s) from the source migration(s), as a single value if
   *   a single key was passed in, or an array of values if there were multiple
   *   keys to look up.
   */
  protected function handleSourceMigration($source_migrations, $source_keys, $default = NULL, $migration = NULL, $createStubs = FALSE) {
    // Handle the source migration(s) as an array.
    $source_migrations = (array) $source_migrations;

    // We want to treat source keys consistently as an array of arrays (each
    // representing one key).
    if (is_array($source_keys)) {
      if (is_array(reset($source_keys))) {
        // Already an array of key arrays, fall through
      }
      else {
        // An array of single-key values - make each one an array
        $new_source_keys = array();
        foreach ($source_keys as $source_key) {
          $new_source_keys[] = array($source_key);
        }
        $source_keys = $new_source_keys;
      }
    }
    else {
      // A simple value - make it an array within an array
      $source_keys = array(array($source_keys));
    }

    // Instantiate each migration, and store back in the array.
    foreach ($source_migrations as $key => $source_migration) {
      $source_migrations[$key] = Migration::getInstance($source_migration);
    }

    $results = array();
    // Each $source_key will be an array of key values
    foreach ($source_keys as $source_key) {
      if (empty($source_key)) {
        continue;
      }
      // Loop through each source migration, checking for an existing dest ID.
      foreach ($source_migrations as $source_migration) {
        // Break out of the loop as soon as a destination ID is found.
        if ($destids = $source_migration->getMap()->lookupDestinationID($source_key)) {
          break;
        }
      }
      // If no destination ID was found, give each source migration a chance to
      // create a stub.
      if (!$destids && $createStubs) {
        foreach ($source_migrations as $source_migration) {
          // Break out of the loop if a stub was successfully created.
          if ($destids = $source_migration->createStubWrapper($source_key, $migration)) {
            break;
          }
        }
      }
      if ($destids) {
        // Assume that if the destination key is a single value, it
        // should be passed as such
        if (count($destids) == 1) {
          $results[] = reset($destids);
        }
        else {
          $results[] = $destids;
        }
      }
      // If no match found, apply the default value (if any)
      elseif (!is_null($default)) {
        $results[] = $default;
      }
    }
    // Return a single result if we had a single key
    if (count($source_keys) > 1) {
      return $results;
    }
    else {
      return reset($results);
    }
  }
  /**
   * MigrateBase doesn't handle updating the arguments and class name on an existing migration. Not sure I understand
   * all the implications of doing that, but I've added it in here since all our configuration, which is volatile,
   * is stored in arguments.
   */
  public static function reRegisterMigration($class_name, $machine_name = NULL, array $arguments = array()) {
    if (!$machine_name) {
      $machine_name = self::machineFromClass($class_name);
    }

    // See if this migration is already registered
    $migration_row = db_select('migrate_status', 'ms')
                     ->fields('ms', array('class_name', 'arguments'))
                     ->condition('machine_name', $machine_name)
                     ->execute()
                     ->fetchObject();
    if (!$migration_row) {
      db_insert('migrate_status')
        ->fields(array(
          'machine_name' => $machine_name,
          'class_name' => $class_name,
          'arguments' => serialize($arguments),
        ))
        ->execute();
    }
    else {
      db_update('migrate_status')
        ->fields(array(
          'class_name' => $class_name,
          'arguments' => serialize($arguments),
          ))
          ->condition('machine_name', $machine_name, '=')
          ->execute();
    }
  }
}
