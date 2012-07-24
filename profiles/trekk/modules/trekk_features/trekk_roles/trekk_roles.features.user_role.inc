<?php
/**
 * @file
 * trekk_roles.features.user_role.inc
 */

/**
 * Implements hook_user_default_roles().
 */
function trekk_roles_user_default_roles() {
  $roles = array();

  // Exported role: editor.
  $roles['editor'] = array(
    'name' => 'editor',
    'weight' => '2',
  );

  // Exported role: site admin.
  $roles['site admin'] = array(
    'name' => 'site admin',
    'weight' => '3',
  );

  // Exported role: writer.
  $roles['writer'] = array(
    'name' => 'writer',
    'weight' => '4',
  );

  return $roles;
}
