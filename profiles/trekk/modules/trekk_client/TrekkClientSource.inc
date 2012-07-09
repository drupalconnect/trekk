<?php
/**
 * The source class is a narrow interface to all operations on the source, including
 * fetching, processing, and iterating over source data. A Source is an iterator, so
 * its principal duty is to control the iteration and communicate with the migration
 * about its status. Most of the business of actually fetching and hydrating data is
 * delegated to the "itemsManager". This is a lot like MigrateList and friends, but
 * our implementation is way more flexible. We can deal in one class with cases that
 * used to be handled by some combination of three or four different ones, e.g.,
 * MigrateSourceList, MigrateMultiItems, MigrateItems, etc.
 */
interface TrekkClientAggregatorSourceInterface {
  public static function info();
  public function __construct(Array $config = array(), $options = array());

  /**
   * Implementors must provide a getItemsManager() method. This is where the itemsManager
   * is composed. Without it, nothing happens.
   */
  public function getItemsManager();
}

abstract class TrekkClientSource extends MigrateSource {
  /**
   * This is an iterator, so keeping track of these is essential
   */
  protected $currentRow = NULL;
  protected $currentKey = NULL;

  /**
   * And these let the migration know all about our status, so we
   * need to keep track of these, too
   */
  protected $numProcessed = 0;
  protected $numIgnored = 0;

  /**
   * This is the configuration we need to save. Subclasses may define more, and if so then
   * they should remember to pull these in with parent::__sleep()
   */
  public function __sleep() {
    return array('schema');
  }

  /**
   * Name of the schema to be loaded. Modules can provide one of these by implementing
   * trekk_client_SCHEMA_schema()
   */
  public $schema;

  /**
  * Drupal hook_schema() style schema definition pulled in from modules implementing
  * hook_trekk_client_source_schema()
   */
  protected $_schemaDefinition;

  /**
   * This flag is used by the itemsManager to know when to reset idList and items
   * caches. This is a hack. In a future iteration proper caching policies will be
   * set here and allowed to propagate down the line
   */
  protected $cachePolicy = TRUE;

  public function setCachePolicy($flag = TRUE) {
    $this->cachePolicy = $flag;
  }
  public function getCachePolicy() {
    return $this->cachePolicy;
  }

  public function __construct(Array $config = array(), $options = array()) {
    $this->schema = $config['schema'];

    if (!empty($options['cache_counts'])) {
      $this->cacheCounts = TRUE;
    }
  }

  public function getSchema() {
    if (is_array($this->_schemaDefinition)) {
      return $this->_schemaDefinition;
    }

    $schemas = module_invoke_all('trekk_client_source_schema');
    // @todo: this is stupid. Find a better way.
    $schema = isset($schemas[$this->schema]) ? $schemas[$this->schema] : array(
      'description' => t('Stub schema'),
      'fields' => array(
        'id' => array('type' => 'varchar', 'length' => 255, 'not null' => TRUE),
      ),
      'primary key' => array('id'),
      );
      
    return $this->_schemaDefinition = $schema;
  }

  /**
   * Return those fields from the schema definition that are defined as keys.
   */
  public function getKeySchema() {
    $schemaDefinition = $this->getSchema();

    // @todo: bare bones, here. No provision for unique keys and other nice stuff
    $keys = $schemaDefinition['primary key'];

    $keySchema = array();

    foreach ($keys as $key) {
      $keySchema[$key] = $schemaDefinition['fields'][$key];
    }

    return $keySchema;
  }

  /**
   * Return an array of the fields defined for this source.
   * @todo: implement this
   */
  public function fields() {
    $schemaDefinition = $this->getSchema();
    foreach ($schemaDefinition as $dest => $mapping) {
      $fields[$mapping['sourceField']] = isset($mapping['description']) ? $mapping['description'] : '';
    }
    //dpm($fields);
    return $fields;
  }

  /**
   * Implementation of Iterator::rewind() - called before beginning a foreach loop.
   */
  public function rewind() {
    $migration = Migration::currentMigration();
    migrate_instrument_start('MigrateSourceList rewind');
    $this->numProcessed = 0;
    $this->numIgnored = 0;
    $idlist = $migration->getOption('idlist');
    if ($idlist) {
      $this->idList = explode(',', $idlist);
    }
    else {
      $this->idList = $this->getIdList();
    }
    migrate_instrument_stop('MigrateSourceList rewind');
    // Load up the first row
    $this->next();
  }

  /**
   * Implementation of Iterator::next() - called at the bottom of the loop implicitly,
   * as well as explicitly from rewind(). Boiler plate of MigrateList's version, with some
   * adjustments appropriate to how we do things in our neighborhood.
   */
  public function next() {
    $migration = Migration::currentMigration();

    migrate_instrument_start('MigrateSourceList next');

    $this->currentRow = NULL;
    $this->currentKey = NULL;

    // Enforce the itemlimit
    $itemlimit = $migration->getItemLimit();
    
    // Get next item (next ID not already in the map, unless needs_update=1)
    while ($this->id = array_shift($this->idList)) {
      // Skip empty IDs
      if (empty($this->id)) {
        continue;
      }
      // Enforce the itemlimit
      if ($itemlimit && $this->numProcessed >= $itemlimit) {
        return;
      }

      if (!is_array($this->id)) {
        $this->id = array($this->id);
      }

      // Check the map - if it's already mapped, and not marked for update, skip it
      $map_row = $migration->getMap()->getRowBySource($this->id);

      //watchdog('client update', $message = $map_row['needs_update'] == 0 ? 'skipping ' . implode($this->id) : 'updating ' . implode($this->id));      
      
      if ($map_row && $map_row['needs_update'] == 0) {
        continue;
      }
      
      $message = t("Processing @sourceid", array('@sourceid' => implode(', ', $this->id)));
      $migration->showMessage($message, 'ok');
      // TODO: Also check message table (non-informational entries here indicate failed items, we don't
      // want to run through them again)
      $this->currentRow = $this->getItemsManager()->getItem($this->id);

      //$info = array(
      //  'item' => $this->currentRow,
      //  'map' => $map_row,
      //  );
      //watchdog('client update', dprint_r($info, TRUE));

      if ($this->currentRow) {

        foreach ($migration->getMap()->getSourceKey() as $field_name => $field_schema) {
          $this->currentKey[$field_name] = $this->currentRow->$field_name;
        }

        // Add map info to the row, if present
        if ($map_row) {
          foreach ($map_row as $field => $value) {
            $field = 'migrate_map_' . $field;
            $this->currentRow->$field = $value;
          }
        }
        // Allow the Migration to prepare this row. prepareRow() can return boolean
        // FALSE to stop processing this row. To add/modify fields on the
        // result, modify $row by reference.
        $return = TRUE;
        if (method_exists($migration, 'prepareRow')) {
          $return = $migration->prepareRow($this->currentRow);
        }

        if ($return !== FALSE) {
          // OK, got a valid one, break out and return
          $this->numProcessed++;
          break;
        }
        else {
          $this->numIgnored++;
        }
      }
      else {
        $this->currentKey = NULL;
      }
    }
    //$info = array(
    //  'currentkey' => $this->currentKey,
    //);
    //watchdog('client', dprint_r($info, TRUE));
    migrate_instrument_stop('MigrateSourceList next');
  }

  /**
   * Get a count of the records available for migration
   */
  public function computeCount() {
    $idList = $this->getIdList();
    return count($idList);
  }

  public function __toString() {
    // @todo: fix this
    return (string)'some string';
  }
}
