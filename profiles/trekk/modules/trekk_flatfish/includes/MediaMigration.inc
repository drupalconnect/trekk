<?php
/**
 * MediaMigration class, extends TrekkFlatfishMigration
 * nearly all work is done by the base class
 */
class MediaMigration extends TrekkFlatfishMigration {
  public function __construct(Array $arguments) {
    parent::__construct($arguments);
  }

  public function initSource() {
    parent::initSource(); // DB conn
    $query = Database::getConnection('default', 'flatfish')
      ->select('media', 'm')
      ->fields('m', $this->getSourceFields());

    $this->source = new MigrateSourceSQL($query, array(), NULL, array('map_joinable' => FALSE));
  }

  public function initDestination() {
    $this->destination = new MigrateDestinationMedia('default', $options = array('copy_blob' => TRUE));
  }

  public function prepareRow($row) {
    if (!isset($row->filename) && isset($row->url)) {
      $row->filename = end(explode('/', $row->url));
    }
  }
  public function getSourceFields() {
    $fields = array('url', 'id', 'contents');
    return $fields;
  }
}
