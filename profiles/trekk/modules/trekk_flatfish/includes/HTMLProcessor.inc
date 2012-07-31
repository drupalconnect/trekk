<?php
/**
 * Processes HTML--cleans it and updates hrefs and img tokens
 *
 * Depends on QueryPath
 *
 */

class HTMLProcessor {

  protected $qp; // QueryPath

  protected $migration;

  /**
   * constructor
   *
   * set migration and load QueryPath
   */
  public function __construct($migration, $html = NULL) {
    $this->migration = $migration;

    $directory = variable_get('querypath_directory', 'sites/all/libraries/querypath');
    require_once $directory . '/src/qp.php';

    $this->qp = htmlqp($html);
  }

  /**
   * Update Media and clean HTML via check_markup().
   */
  public function execute() {
    $this->updateMediaTokens();

    // snag the HTML
    $html = $this->qp->top('body')->innerHTML();

    // load htmlpurifier and clean it
    $directory = variable_get('htmlpurifier_directory', 'sites/all/libraries/htmlpurifier');
    require_once $directory . '/library/HTMLPurifier.auto.php';
    $config = HTMLPurifier_Config::createDefault();
    $allowed_tags = array(
      'p',
      'br',
      'b',
      'a[href]',
      'li',
      'ul',
      'ol',
      'h1',
      'h2',
      'h3',
      'i',
      'em',
      'strong',
      'img[src]',
    );
    $config->set('HTML.Allowed', implode(',', $allowed_tags));
    $purifier = new HTMLPurifier($config);
    // rm extra whitespace
    return preg_replace('/\s\s+/', ' ', $purifier->purify($html));
  }

  /**
   * Update media tokens in anchor and img tags
   *
   * Replace with file_create_url or json_encoded array respectively.
   */
  public function updateMediaTokens() {
    $migration = $this->migration;
    $migration_base_name = strtolower($migration->getBaseName());

    $qp = $this->qp;

    // Replace media tokens in anchor tags (ie, downloadable files--pdfs, docs, xls, etc)
    foreach ($qp->top()->find('a') as $a) {
      $href = $a->attr('href');
      $id = trim(array_pop(explode(':', $href)), ']');
      $key = trim(array_shift(explode(':', $href)), '[');

      if (!is_numeric($id)) {
        continue;
      }
      $item = $migration->getMappedEntity(array('flatfish_media'), array(array('id' => $id)));

      if ($item) {
        $a->attr('href', file_create_url($item->uri));
      }
    }

    // Replace media tokens in img tags
    foreach ($qp->top()->find('img') as $img) {
      $src = $img->attr('src');
      $id = trim(array_pop(explode(':', $src)), ']');
      $key = trim(array_shift(explode(':', $src)), '[');
      if (!is_numeric($id)) {
        continue;
      }
      $item = $migration->getMappedEntity(array('flatfish_media'), array(array('id' => $id)));

      if ($item) {
        // Filter syntax is just a json_encode()ed array
        $attrs = getimagesize($item->uri);
        $height = ((int)$img->attr('height'))? (int)$img->attr('height'): $attrs[0];
        $width = ((int)$img->attr('width'))? (int)$img->attr('width'): $attrs[1];
        $repl = json_encode(array(
          'type'       => 'media',
          // TODO find out if this view mode is cool
          'view_mode'  => 'media_large',
          'fid'        => (int)$item->fid,
          'attributes' => array(
            'alt'      => check_plain($img->attr('alt')),
            'title'    => check_plain($img->attr('title')),
            'height'   => $height,
            'width'    => $width,
          ),
        ));

        $img->replaceWith('[[' . $repl . ']]');
      }
    }
  }

}
