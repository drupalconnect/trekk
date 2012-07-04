<?php

class MediaFilter implements HTMLScraperFilterInterface {

  /**
   * The calling scraper
   */
  protected $scraper;
  
  public function getScraper() {
    return $this->scraper;
  }

  /**
   * Array of parameters
   */
  protected $params;
  
  public function getParams() {
    return $this->params;
  }
  
  public function __construct($scraper, $params = array()) {
    $this->scraper = $scraper;
    $this->params = $params;
  }
  
  public function execute() {
    $migration = $this->getScraper()->getMigration();
    
    $migration_base_name = $migration->getBaseName();
    
    $qp = $this->getScraper()->getQp();
    
    // Munge HREFs
    foreach ($qp->top()->find('a') as $a) {
      $href = $a->attr('href');
      $id = trim(array_pop(explode(':', $href)), ']');
      $key = trim(array_shift(explode(':', $href)), '[');
      
      // @todo: DANGER! Hard coded dependency on migration name!!!
      
      $item = $migration->getMappedEntity(array($migration_base_name . '_media'), array(array('id' => $id)));
      
      if ($item) {
        $a->attr('href', file_create_url($item->uri));
      }
    }
    
    // Munge IMGs
    foreach ($qp->top()->find('img') as $img) {
      $src = $img->attr('src');
      
      $id = trim(array_pop(explode(':', $src)), ']');
      
      $key = trim(array_shift(explode(':', $src)), '[');
      
      // @todo: DANGER! Hard coded dependency on migration name!!!
      $item = $migration->getMappedEntity(array($migration_base_name . '_media'), array(array('id' => $id)));

      if ($item) {
        // Hey! Filter syntax is just a json_encode()ed array!
        $repl = json_encode(array(
          'type'       => 'media',
          'view_mode'  => 'media_large',
          'fid'        => (int)$item->fid,
          'attributes' => array(
            'alt'      => check_plain($img->attr('alt')),
            'title'    => check_plain($img->attr('title')),
            'height'   => (int)$img->attr('height'),
            'width'    => (int)$img->attr('width'),
          ),
        ));

        $img->replaceWith('[[' . $repl . ']]');
      }
    }
  }
}