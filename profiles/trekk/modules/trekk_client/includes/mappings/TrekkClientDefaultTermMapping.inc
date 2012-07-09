<?php
class TrekkClientDefaultTermMapping extends TrekkClientMapping implements TrekkClientAggregatorMappingInterface {

  public function __sleep() {
  	return parent::__sleep();
  }	
	
	public static function info() {
		return $info = array(
		  'name' => t('DefaultTermMapping'),
		  'description' => t('Maps taxonomy terms to taxonomy terms'),
		  );
  }  
  
  public function getMappings() {
  	$mappings['name'] = array(
  	  'destinationField' => 'name',
  	  'sourceField' => 'name',
  	  );
  	$mappings['parent'] = array(
  	  'destinationField' => 'parent',
  	  'sourceField' => 'parents',
  	  'sourceMigration' => TRUE,
  	  'arguments' => array('source_type' => 'tid'),
  	  );
  	$mappings['uuid'] = array(
  	  'destinationField' => 'uuid',
  	  'sourceField' => 'uuid',
  	  );
  	  
  	return $mappings;  
  }  
  
	public function __construct($config) {
		parent::__construct($config);
  }
}

