<?php
class HTMLMigration extends TrekkerMigration {
	public function __construct(Array $arguments) {
		parent::__construct($arguments);
  }
  
  public function initSource() {  
    
    $name = $this->generateMachineName();
    
    $config = $this->getConfig();

    Database::addConnectionInfo($name, 'default', array(
      'driver'   => $config['source']['connection']['info']['driver'],
      'database' => $config['source']['connection']['info']['database'],
      'username' => $config['source']['connection']['info']['username'],
      'password' => $config['source']['connection']['info']['password'],
      'host'     => $config['source']['connection']['hostname'],
      'prefix'   => $config['source']['connection']['prefix'],
    ));
    
    $query = Database::getConnection('default', $name)
      ->select('pages', 'p')
      ->fields('p', $this->getSourceFields());

    $this->source = new MigrateSourceSQL($query, array(), NULL, array('map_joinable' => FALSE));
  }
  
  public function initDestination() {
    $this->destination = new MigrateDestinationNode('page'); 
  }
}


