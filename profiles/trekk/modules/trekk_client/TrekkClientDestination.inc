<?php
/**
 * There's nothing to do here, really. We extend Migrate's own MigrateDestination classes, e.g., 
 * MigrateDestinationNode, MigrateDestinationTerm ... but we don't provide any additional 
 * functionality beyond being able to register ourselves to have our settings included in 
 * configuration forms.
 */
interface TrekkClientAggregatorDestinationInterface {
	public static function info();
	public function __construct(Array $config = array(), $options = array());
	public function configForm($form, &$form_state, $entity, $op = 'edit');
	public function configFormValidate($form, &$form_state);
}