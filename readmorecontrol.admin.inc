<?php

/**
 * @file
 * Contains hooks defined in readmorecontrol_hook_info().
 */

/**
 * Implements hook_form_FORM_ID_alter().
 */
function readmorecontrol_form_node_type_form_alter(&$form, &$form_state, $form_id) {
  $type = $form['#node_type'];

  // Add node_readmorecontrol setting to node_type_form to control the
  // circumstances when the Read More link should be displayed.
  $form['readmorecontrol'] = array(
    '#type' => 'fieldset',
    '#title' => t('Read More Settings'),
    '#descriptions' => t('Settings to control when Read More is displayed.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
  );

  // The actual setting itself. Nodes module automatically saves it as a
  // variable in the database.
  $form['readmorecontrol']['node_readmorecontrol'] = array(
    '#type' => 'radios',
    '#title' => t('Display Read More'),
    '#default_value' => variable_get('node_readmorecontrol_' . $type->type, 'always'),
    '#options' => array(
      'always' => t('Always'),
      'when_required' => t('When Required'),
      'never' => t('Never'),
    ),
  );
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function readmorecontrol_form_field_ui_field_edit_form_alter(&$form, &$form_state, $form_id) {
  $instance = $form['#instance'];
  $bundle = $instance['bundle'];
  $entity_type = $instance['entity_type'];
  $bundles = field_info_bundles();

  // Add readmorecontrol_ignore flag to field_ui_field_edit_form so a fields
  // contents can be ignored when calculating whether to display read more link.
  if ($entity_type === 'node') {
    $form['instance']['readmorecontrol_ignore'] = array(
      '#type' => 'checkbox',
      '#title' => t('Read More Ignores Field'),
      '#description' => t('Should the %field field be ignored when the %display_read_more setting for %type is set to %when_required.',
        array(
          '%field' => $instance['label'],
          '%type' => $bundles[$entity_type][$bundle]['label'],
          '%display_read_more' => t('Display Read More'),
          '%when_required' => t('When Required'),
        )),
      '#default_value' => !empty($instance['readmorecontrol_ignore']),
      '#weight' => -9,
    );
  }
}

/**
 * Implements hook_node_type_update().
 */
function readmorecontrol_node_type_update($info) {
  if (!empty($info->old_type) && $info->old_type != $info->type) {
    $setting = variable_get('node_readmorecontrol_' . $node->type, 'always');
    variable_del('node_readmorecontrol_' . $info->old_type);
    variable_set('node_readmorecontrol_' . $info->type, $setting);
  }
}

/**
 * Implements hook_node_type_delete().
 */
function readmorecontrol_node_type_delete($info) {
  variable_del('node_readmorecontrol_' . $info->type);
}
