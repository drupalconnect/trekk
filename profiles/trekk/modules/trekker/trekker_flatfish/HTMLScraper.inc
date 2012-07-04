<?php
interface HTMLScraperFilterInterface {
  public function __construct($scraper, $params = array());
  public function execute();
}

class HTMLScraper {
	
	/**
 	 * QueryPath object
 	 */
	protected $qp;
  
  /**
   * Filters to use in this scrape.
   */
  protected $filters = array();

  protected $migration;
  
  public function getMigration() {
	  return $this->migration;
	}

  public static function init($migration, $html = NULL) {
    return new HTMLScraper($migration, $html);
  }

  public function __construct($migration, $html = NULL) {
  	$this->migration = $migration;
  	
  	$this->getQp($html);
  }

  /**
   * Setter to add a filter. Filters must implement
   * HTMLScraperFilter
   */
  public function addFilter($class, $params = array()) {
  	if (!module_load_include('inc', 'trekker', 'lib/scraper/' . $class)) {
  		return $this;
  	}
  		
    if (class_exists($class)) {
      $this->filters[] = new $class($this, $params);
    }

    return $this;
  }
  public function getFilters() {
    return $this->filters;
  }

  /**
   * Execute the scraper
   */
  public function execute() {
    foreach ($this->getFilters() as $filter) {
      $filter->execute();
    }

    $html = $this->getQp()->top('body')->innerHTML();
    return check_markup($html, 'purified_html');
  }
  
  public function getQp($html = NULL) {
  	if (!is_null($this->qp)) {
  		return $this->qp;
  	}
  	
    $directory = variable_get('querypath_directory', 'sites/all/libraries/QueryPath');
    require_once $directory . '/QueryPath.php';
    return $this->qp = htmlqp($html);
  }
}
