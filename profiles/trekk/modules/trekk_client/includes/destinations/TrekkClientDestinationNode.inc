<?php
class TrekkClientDestinationNode extends MigrateDestinationNode implements TrekkClientAggregatorDestinationInterface {
	public $bundle = '';

	public function __sleep() {
		return array('bundle');
  }

	public static function info() {
		return array(
		  'name' => t('Node'),
		  );
	}

	public function __construct(Array $config = array(), $options = array()) {
		if (isset($config['bundle'])) {
			// Parent sets bundle
		  $bundle = $config['bundle'];
		}

    parent::__construct($bundle, $options);
  }

  public function configForm($form, &$form_state, $entity, $op = 'edit') {
  	$form = array();

    $types = node_type_get_types();

    $options = array();
    foreach ($types as $name => $type) {
    	$options[$name] = $type->name;
    }

    $form['bundle'] = array(
      '#type' => 'select',
      '#title' => t('Type'),
      '#default_value' => isset($entity->bundle) ? $entity->bundle : '',
      '#empty_option' => '--Select one--',
      '#empty_value' => '',
      '#options' => $options,
      );

  	return $form;
  }

  public function configFormValidate($form, &$form_state) {
  	$this->bundle = $form_state['values']['bundle'];
  }

	public function createStub($aggregator) {
    if (empty($this->bundle)) return FALSE;

    // Create a stub node
    $node = new stdClass;
    $node->title = t('Stub');
    $node->body = t('Stub body');
    $node->type = $this->bundle;
    $node->uid = 1;
    $node->status = 0;

    node_save($node);

    if (isset($node->nid)) {
      return array($node->nid);
    }
    else {
      return FALSE;
    }
  }
}
