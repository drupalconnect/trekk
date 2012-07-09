<?php
class TrekkClientSourceHTTP extends TrekkClientSource implements TrekkClientAggregatorSourceInterface {

  // These are the minimum needed to get this thing to work. Additionally, an item request for fetching a single record may be supplied.
	public $method;
	public $endpoint;
	public $params = array();

	// If present then these are used to create an auth request
	public $username;
	public $password;

	/**
 	 * This is the configuration we need to keep
 	 */
  public function __sleep() {
  	return array('schema', 'method', 'endpoint', 'params', 'username', 'password');
  }

  public function __construct(Array $config = array(), $options = array()) {
		parent::__construct($config, $options);

		if (!empty($config['endpoint'])) {
		  $this->endpoint = $config['endpoint'];
		}
		if (!empty($config['method'])) {
		  $this->method = $config['method'];
		}
		if (!empty($config['params'])) {
		  $this->params = $config['params'];
		}
		if (!empty($config['username'])) {
			$this->username = $config['username'];
		}
		if (!empty($config['password'])) {
			$this->password = $config['password'];
		}
	}

  public function getItemsManager() {
  	if (isset($this->_itemsManager)) {
  		return $this->_itemsManager;
  	}

  	$listRequest = new HTTPCurlRequest($this->endpoint, $this->method, $this->params);

    $manager = new ItemsManagerHTTP($this);
    $manager->setListRequest($listRequest);

    return $this->_itemsManager = $manager;
	}

	public function getIdList() {
		return $this->getItemsManager()->getIdList();
	}

	public function getItem($id) {
		return $this->getItemsManager()->getItem($id);
	}

	public static function info() {
		return array(
		  'name' => t('HTTP (single URI)'),
		  'description' => t('Use this handler if you have one endpoint that lists multiple, complete items.'),
		  );
	}

	public function configForm($form, &$form_state, $entity, $op = 'edit') {
		$form = array();

		//$form['username'] = array(
		//  '#type' => 'textfield',
		//  '#title' => t('Username'),
		//  '#default_value' => isset($entity->username) ? $entity->username : NULL,
		//  );

		//$form['password'] = array(
		//  '#type' => 'password',
		//  '#title' => t('Password'),
		//  '#default_value' => isset($entity->password) ? $entity->password : NULL,
		//  );

		$form['items_method'] = array(
		  '#type' => 'select',
		  '#title' => t('Items request method'),
		  '#default_value' => isset($entity->method) ? $entity->method : 'GET',
		  '#options' => array(
		    'GET' => 'GET',
		    'POST' => 'POST',
		    'PUT' => 'PUT',
		    ),
		  );

		$form['items_uri'] = array(
		  '#type' => 'textfield',
		  '#title' => t('Items request URI'),
		  '#default_value' => isset($entity->endpoint) ? $entity->endpoint : '',
		  );

    $params = isset($entity->params) ? $entity->params : array();
    foreach ($params as $key => $value) {
    	$params[$key] = implode('=', array($key, $value));
    }
    $params = implode("\n", $params);

		$form['items_params'] = array(
		  '#type' => 'textarea',
		  '#title' => t('Item request params'),
      '#default_value' => $params,
		  );

		$schemas = trekk_client_source_schema();
		foreach ($schemas as $name => $schema) {
			$options[$name] = !empty($schema['label']) ? $schema['label'] : ucwords($schema['name']);
		}

    $form['schema'] = array(
      '#type' => 'select',
      '#title' => t('Schema'),
      '#default_value' => isset($entity->schema) ? $entity->schema : '',
      '#empty_value' => '',
      '#empty_option' => '--Select one--',
      '#options' => !empty($options) ? $options : array(),
      );

		$form['test_connection'] = array(
		  '#type' => 'button',
		  '#value' => t('Test Connection'),
		  '#name' => 'test_connection',
		  );

    if (isset($form_state['triggering_element']) && !strcmp('test_connection', $form_state['triggering_element']['#name'])) {
    	// The validator sets up necessary properties
    	$this->configFormValidate($form, $form_state);

    	$config = array(
    	  'schema' => $this->schema,
    	  'method' => $this->method,
    	  'endpoint' => $this->endpoint,
    	  'params' => $this->params,
    	  'username' => $this->username,
    	  'password' => $this->password,
    	  );
    	$this->__construct($config, $options = array());

  	  $count = $this->getItemsManager()->count();

      $idList = $this->getItemsManager()->getIdList();
      $idList = array_slice($idList, 0, 20);
      foreach ($idList as $id) {
      	$items[] = $this->getItemsManager()->getItem($id);
  	  }

  	  $form['count'] = array(
  	   '#type' => 'markup',
  	   '#markup' => t('%count items found', array('%count' => $count)),
  	   );
  	  $form['response'] = array(
  	    '#type' => 'fieldset',
  	    '#title' => t('Response'),
  	    '#collapsible' => TRUE,
  	    '#collapsed' => TRUE,
  	    '#description' => t('Display up to the first 20 items.'),
  	    );
  	  $form['response']['output'] = array(
  	    '#type' => 'markup',
  	    '#markup' => module_exists('devel') ? dprint_r($items, TRUE) : print_r($items, TRUE),
  	    '#description' => !module_exists('devel') ? t('Enable the devel module for more readable output.') : '',
  	    );

    }

		return $form;
	}

	public function configFormValidate($form, &$form_state) {
		//$this->username = $form_state['values']['username'];
    //$this->password = $form_state['values']['password'];

		$this->schema = $form_state['values']['schema'];
		$this->method = $form_state['values']['items_method'];
		$this->endpoint = $form_state['values']['items_uri'];
		$this->items_method = $form_state['values']['items_method'];

		// Process params into an array
    if ($form_state['values']['items_params']) {
    	$lines = explode("\n", $form_state['values']['items_params']);
    	foreach ($lines as $line) {
    	  list($key, $value) = explode('=', $line);
    	  $params[$key] = rtrim($value);
    	}
    	$this->params = $params;
    }
	}
}
