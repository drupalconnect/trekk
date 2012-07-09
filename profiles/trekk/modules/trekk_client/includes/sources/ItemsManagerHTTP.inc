<?php
/**
 * Do both getItem and getIdList
 */
class ItemsManagerHTTP extends MigrateItems {

  /**
   * Items fetched from a response.
   */
  protected $_items = array();

  /**
   * The list request gets the items index (list of items to migrate)
   */
  protected $_listRequest;

  public function setListRequest($request) {
    $this->_listRequest = $request;
    return $this;
  }
  public function getListRequest() {
    return $this->_listRequest ? $this->_listRequest : NULL;
  }

  /**
   * If present, the item request is used to fetch a single item
   */
  protected $_itemRequest;

  public function setItemRequest($request) {
    $this->_itemRequest = $request;
    return $this;
  }
  public function getItemRequest() {
    return $this->_itemRequest ? $this->_itemRequest : NULL;
  }

  protected $_source;

  public function __construct($source) {

    $this->_source = $source;
    parent::__construct();

  }

  public function getIdList() {
    if (!empty($this->_idList)) {
      return $this->_idList;
    }

    ctools_include('object-cache');

    $reset = $this->_source->getCachePolicy();
    if (!$reset) {
      // Try to load the idlist from cache
      dsm('loading from cache');
      $cache_key = (string)$this->getListRequest() . '_idlist';
      $idList = ctools_object_cache_get('http-response-idlist', $cache_key);

      if (!empty($idList)) {
        return $this->_idList = $idList;
      }
    }

    // If no idlist exists in cache then we need to build one
    $listRequest = $this->getListRequest();
    $response = $listRequest->doRequest()->getResponseData('drupal_json_decode');

    if (empty($response)) {
      $response = array();
    }

    $this->_idList = array();

    // If a source is present then we can examine items for a key schema.
    // @todo: this is kind of important. there shouldn't be any if about it.
    if (!is_null($this->_source)) {
      $keySchema = $this->_source->getKeySchema();
    }
    else {
    }

    if (!empty($keySchema)) {
      foreach ($response as $item) {

        // Deal with multiple source keys
        foreach ($keySchema as $key => $keydef) {
          $item = (array)$item;
          //if (isset($keys[$key])) {dsm($item[$key], 'got a dup');}
          //NOTE: $item[$key] appears to be an empty array sonetimes,
          //      not a string, which throws off the implode() calls below
          if (!empty($item[$key])) $keys[$key] = $item[$key];
        }

        // Set a dummy value to be used below:
        if (empty($keys)) $keys[] = 'derp';

        // Construct the key and set it in
        $idList[implode('|', $keys)] = $keys;

        // In case no item request is set, we know that this response contains all our data.
        // so let's go ahead and set it into cache.
        $cache_key = (string)$this->getListRequest() . '_' . implode('-', $keys);
        ctools_object_cache_set('http-response-item', $cache_key, $item);
        //dsm($cache_key, 'Set cache key ');
        $this->_idList = $idList;
      }
    }
    else {
      // If we can't get the key schema then we have to simply send back array keys
      $this->_idList = array_keys($this->_items);
      $this->_items = $response;
    }

    $cache_key = (string)$this->getListRequest() . '_idlist';
    dsm($cache_key, 'setting cache');
    ctools_object_cache_set('http-response-idlist', $cache_key, $this->_idList);
    return $this->_idList;
  }

  /**
   * Get a string representing the source URI
   */
  public function __toString() {
    $request = $this->_response->getRequest();
    return $request->getRequestURI();
  }

  /**
   * Get an item by ID. Migrate iterates over the list
   * of IDs returned by getIdList(), passing each ID
   * through here to get a fully baked source 'row'.
   */
  public function getItem($id) {

    if (!is_null($itemRequest = $this->getItemRequest())) {
      $options = array(
        'replacements' => $id,
      );
      $response = $itemRequest->doRequest($params = array(), $options)->getResponseData('drupal_json_decode');

      return $response;
    }

    // Since we don't have an item request lets see if we can get one from cache
    ctools_include('object-cache');
    $cache_key = (string)$this->getListRequest() . '_' . implode('-', $id);
    $item = ctools_object_cache_get('http-response-item', $cache_key);

    if ($item) {
      return (object)$item;
    }

    // And barring all else, we'll just have to do it the brute force way.
    //else {
    $response = $this->getListRequest()->getResponseData('drupal_json_decode');
    foreach ($response as $item) {
      $matches = array_intersect_assoc((array)$item, $id);
      if (count($matches) == count($id)) {
        return (object)$item;
      }

    }
    //}
    return array();
  }

  public function getItems() {
    if (!empty($this->_items)) {
      return $this->_items;
    }

    foreach ($this->getIdList() as $id) {
      $items[] = $this->getItem($id);
    }
    return $this->_items = $items;
  }

  /**
   * Determine how many items are in the data set.
   */
  public function count() {
    return $this->getCount();
  }

  public function getCount() {
    return count($this->getIdList());
  }
  
  public function computeCount() {
    return $this->getCount();
  }

  /**
   * ADDITIONAL METHODS
   */

  /**
   * Introspect the source to get a list of available fields.
   */
  public function getFields() {
    $fields = array();
    $fields += $this->getResponse()->getFields();
    return $fields;
  }
}
