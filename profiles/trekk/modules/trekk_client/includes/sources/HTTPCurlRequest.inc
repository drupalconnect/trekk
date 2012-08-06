<?php
/**
 * Takes care of performing the request and composing the response
 * entity.
 */
class HTTPCurlRequest {

  protected $_endpoint;

  protected $_method;

  protected $_params = array();

  private $_headers;
  private $_data;

  public function __toString() {
  	$subj = $this->_endpoint . $this->_method;
  	$pred = implode(':', $this->_params);
  	return md5(implode(':', array($subj, $pred)));
  }

  public function getResponseData($hydrator = NULL) {
  	if (!is_null($hydrator)) {
  		return call_user_func($hydrator, $this->_data);

    }
  	return $this->_data;
  }

  public function getHeaders() {
  	return $this->_headers;
  }

  public function __construct($endpoint, $method, $params = array(), $data = array()) {
    $this->_endpoint = $endpoint;
    $this->_method = $method;
    $this->_params = $params;
  }

  public function getRequestURI($params = array(), $options = array()) {

  	// Allow callers to alter params before sending the request
    if (is_array($this->_params) && !strcmp('GET', strtoupper($this->_method))) {
      $params += $this->_params;
    }

    $endpoint = $this->_endpoint;

    if (isset($options['replacements'])) {
    	$endpoint = array_shift($this->_placeholderReplace($endpoint, $options['replacements']));
    	$params = $this->_placeholderReplace($params, $options['replacements']);
    }

		// Token replacement:
		$params = $this->_placeholderReplaceTokens($params);

    return url($endpoint, $options = array('query' => $params));
  }

  protected function _placeholderReplace($subjects, $replacements) {
  	if (!is_array($subjects)) {
  		$subjects = array($subjects);
  	}

  	foreach ($subjects as &$subject) {
  		if (is_array($subject)) {
  			$subject = $this->_placeholderReplace($subject, $replacements);
  		}
  		else {
  			foreach ($replacements as $name => $value) {
  				$search = '[:' . $name . ']';
  				$subject = str_replace($search, (string)$value, $subject);
  			}
  		}
  	}

  	return $subjects;
  }

  protected function _placeholderReplaceTokens($subjects) {
  	if (!is_array($subjects)) {
  		$subjects = array($subjects);
  	}

  	foreach ($subjects as &$subject) {
  		if (is_array($subject)) {
  			$subject = $this->_placeholderReplaceTokens($subject);
  		}
  		else {
				$subject = token_replace($subject);
  		}
  	}

  	return $subjects;
  }

  public function getRequestMethod() {
  	return strtoupper(trim($this->_method));
  }

  public function doRequest($requestParams = array(), $options = array()) {
  	// Caching = nice to have, but currently no way to flag this for reset.
  	//ctools_include('object-cache');
  	//$cached = ctools_object_cache_get('http-response', (string)$this, $skip_cache = FALSE);
  	//if ($cached) {
  	//	$this->_data = $cached;
  	//	return $this;
    //}

  	$this->_data = $this->_doRequest($requestParams, $options);
  	//ctools_object_cache_set('http-response', (string)$this, $this->_data);
  	return $this;
  }

  /**
   * Internal method for performing an HTTP request and setting up the
   * response.
   *
   * @params Array Array of additional parameters for this request
   */
  protected function _doRequest($requestParams = array(), $options = array()) {
    $url = $this->getRequestURI($requestParams, $options);
    $method = $this->getRequestMethod();

    $ch = curl_init();

    curl_setopt($ch, CURLOPT_HTTPHEADER, array('Accept: application/json')); // Accept JSON response
		curl_setopt($ch, CURLOPT_HEADER, FALSE);  // Ask to not return Header
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
		curl_setopt($ch, CURLOPT_FAILONERROR, TRUE);

    switch (strtoupper($method)) {
      case 'GET':
    	  curl_setopt($ch, CURLOPT_URL, $url);
    	  break;
    	case 'POST':
    	case 'PUT':
    	  curl_setopt($ch, CURLOPT_URL, $url);
    	  curl_setopt($ch, CURLOPT_POST, TRUE); // Do a regular HTTP POST
		    curl_setopt($ch, CURLOPT_POSTFIELDS, $this->_params); // Set POST data
    }

    $data = curl_exec($ch);

    $this->_headers = curl_getinfo($ch);

    curl_close($ch);
    dpm($this->_headers);
    dpm(json_decode($data));

    return $data;
  }
}
