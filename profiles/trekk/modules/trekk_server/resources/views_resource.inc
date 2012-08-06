<?php
function views_resource_index() {
}

/**
 * Retrieve callback for the views resource
 *
 * Mostly copied from services_views.module's retrieve callback. We've altered.
 */
function views_resource_retrieve($display_id, $view_name, $args = array(), $offset = 0, $limit = 10, $filters = array(), $output) {
  $result = array();
  $view = views_get_view($view_name);

  // Put all arguments and filters and then execute.
  $view->set_arguments($args, FALSE);
  $view->set_exposed_input($filters);
  $view->set_offset($offset);

  // Support for Views 2
  if (method_exists($view, 'set_use_pager')) {
    // If offset is set we can't have a user pager.
    if (empty($offset)) {
      $view->set_use_pager(TRUE);
      $view->set_items_per_page($limit);
    }
    else {
      // Disable the user pager.
      $view->set_use_pager(FALSE);
    }
  }
  else {
    $view->set_items_per_page($limit);
  }

  // If we display values and not themed view output.
  if (!strcmp('data', strtolower($output))) {
    $view->set_display($display_id);
    $view->pre_execute();
    $view->execute();
    $result = $view->result;

    $row_plugin = $view->display_handler->get_option('row_plugin');
    // If row plugin is node, then we should load each node
    if ($row_plugin == 'node') {
      $nodes = array();
      foreach ($view->result as $row) {
        $nodes[] = node_load($row->nid);
      }
      $result = $nodes;
    }
    elseif ($row_plugin == 'fields') {
      $items = array();
      foreach ($view->result as $row) {
        $item = new stdClass();
        foreach ($view->field as $field_id => $field) {
          if (!empty($field->options['label'])) {
            $field_label = strtolower($field->options['label']);
          }
          elseif ($field->field_alias) {
            $field_label = $field->field_alias;
          }
          else {
            $field_label = $field_id;
          }
          $item->$field_label = $field->theme($row);
        }
        $items[] = $item;
      }
      if ($items) {
        $result = $items;
      }
    }
  }
  elseif (!strcmp('html', strtolower($output))) {
    // We want to keep the result an array.
    $result[] = $view->preview($display_id);
  }
  return $result;
}
