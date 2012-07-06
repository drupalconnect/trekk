<?php
/**
 * NodeMigration class, extends TrekkerMigration
 * a lot of the works is done by the base class
 */

class NodeMigration extends TrekkerMigration {
	public function __construct(Array $arguments) {
		parent::__construct($arguments);
  }
  
  /**
   * run during constructor
   */
  public function initSource() {  
    parent::initSource(); // DB conn
    
    $name = $this->generateMachineName(); //WAT
    
    $query = Database::getConnection('default', $name)
      ->select($name, 'type')
      ->fields('type', $this->getSourceFields());

    $this->source = new MigrateSourceSQL($query, array(), NULL, array('map_joinable' => FALSE));
  }
  
  /**
   * run during constructor
   */
  public function initDestination() {
    // need to update this
    $this->destination = new MigrateDestinationNode($name); 
  }
}
