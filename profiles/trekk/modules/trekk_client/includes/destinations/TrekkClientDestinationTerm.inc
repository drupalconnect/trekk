<?php
class TrekkClientDestinationTerm extends MigrateDestinationTerm implements TrekkClientAggregatorDestinationInterface {

	public $bundle = '';

	public function __sleep() {
		return array('bundle');
  }

	public static function info() {
		return array(
		  'name' => t('Term'),
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

    $vocabularies = taxonomy_vocabulary_get_names();

    foreach ($vocabularies as $vocabulary) {
    	$options[$vocabulary->machine_name] = $vocabulary->name;
    }

    $form['bundle'] = array(
      '#type' => 'select',
      '#title' => t('Vocabulary'),
      '#default_value' => $entity->bundle ? $entity->bundle : '',
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
    // @todo: how to not create a stub when there is no parent?
    if (!empty($this->bundle)) {
    	$vocab = taxonomy_vocabulary_machine_name_load($this->bundle);
    	$vid = $vocab->vid;
    }
    // Create a stub term
    $term = new stdClass;
    $term->name = t('Stub');
    $term->vid = $vid;
    $term->vocabulary_machine_name = $this->bundle;

    taxonomy_term_save($term);

    if (isset($term->tid)) {
      return array($term->tid);
    }
    else {
      return FALSE;
    }
  }
}
