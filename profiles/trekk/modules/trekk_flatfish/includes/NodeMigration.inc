<?php
/**
 * NodeMigration class, extends TrekkFlatfishMigration
 * a lot of the works is done by the base class
 */
class NodeMigration extends TrekkFlatfishMigration {
  public function __construct(Array $arguments) {
    parent::__construct($arguments);
  }

  /**
   * run during constructor
   */
  public function initSource() {
    parent::initSource(); // DB conn

    $name = $this->config[$this->getBaseName()]->config[$this->getBaseName()]['machine_name'];
    $query = Database::getConnection('default', 'flatfish')
      ->select($name, 'type')
      ->fields('type', $this->getSourceFields());
    $this->source = new MigrateSourceSQL($query, array(), NULL, array('map_joinable' => FALSE));
  }

  /**
   * run during constructor
   */
  public function initDestination() {
    $name = $this->config[$this->getBaseName()]->config[$this->getBaseName()]['machine_name'];
    $this->destination = new MigrateDestinationNode($name);
  }
}
