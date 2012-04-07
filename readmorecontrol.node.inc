<?php

/**
 * @file
 * Contains hooks defined in readmorecontrol_hook_info().
 */

/**
 * Implements hook_field_display_alter().
 */
function readmorecontrol_field_display_alter(&$display, $context) {
  extract($context, EXTR_SKIP);
  $field_name = $field['field_name'];

  // Initiate global setting checks.
  readmorecontrol_set_entity_behaviour($entity_type, $entity);

  // If readmore_required has been set on the entity no further calculations
  // are needed. This could be either TRUE or FALSE.
  if (isset($entity->readmore_required)) {
    return;
  }

  // Terminate processing early based on the instance settings.
  if (!empty($instance['readmorecontrol_ignore'])) {
    return;
  }

  /*
   * @todo - This is the last remains of node specific behaviour here.
   * This needs refactoring out to allow other entities and view modes.
   */

  // Currently limited to node entities with the teaser view mode.
  if ($entity_type == 'node' && $view_mode == 'teaser') {

    // Discover if the field is empty. This mimics _field_invoke() to pull out
    // the $items array. (@todo determine if we can use the global $language).
    $available_languages = field_available_languages($entity_type, $field);
    $languages = _field_language_suggestion($available_languages, $entity->language, $field_name);
    $items = array();
    foreach ($languages as $langcode) {
      $lang_items = isset($entity->{$field_name}[$langcode]) ? $entity->{$field_name}[$langcode] : array();
      $items = array_merge($items, $lang_items);
    }
    $items = _field_filter_items($field, $items);
    if (empty($items)) {
      return;
    }

    // Text summary fields are processed latter.
    // @todo - Fix me, this assumes full display on default view mode.
    if ($field['type'] != 'text_with_summary' && $field['type'] != 'text_long') {
      // Get the field display info for full view mode.
      $fulldisplay = field_get_display($instance, 'full', $entity);

      // If the full view mode is hidden, we can ignore this field.
      if ($fulldisplay['type'] != 'hidden') {
        if ($fulldisplay['type'] != $display['type'] || $fulldisplay['settings'] !== $display['settings']) {
          $entity->readmore_required = TRUE;
        }
      }
    }
  }
}

/**
 * Helper function to do late processing fields.
 *
 * This currently only handles the text with summary/trimmed fields. And it
 * assumes that the display in the default full view is full text.
 */
function readmorecontrol_post_process_fields($content, $entity_type, $entity, $view_mode, $langcode, $bundle = NULL) {
  if (!isset($bundle)) {
    list($id, $vid, $bundle) = entity_extract_ids($entity_type, $entity);
  }

  $instances = field_info_instances($entity_type, $bundle);
  foreach ($instances as $field_name => $instance) {
    // @todo - how does this interact with groups?
    // @todo - how does this interact with field access?
    $element = isset($content[$field_name]) ? $content[$field_name] : FALSE;
    if ($element) {
      $field = field_info_field($field_name);
      switch ($field['type']) {
        case 'text_with_summary':
        case 'text_long':
          // @todo - Fix me, this assumes full display on default view mode.
          // See text_field_formatter_view() for the mechanics.
          if ($element['#formatter'] == 'text_trimmed' || $element['#formatter'] == 'text_summary_or_trimmed') {
            foreach ($element['#items'] as $delta => $item) {
              $sanitized = _text_sanitize($instance, $langcode, $item, 'value');
              if ($element[$delta]['#markup'] !== $sanitized) {
                return TRUE;
              }
            }
          }
          break;

      }
    }
  }
  return FALSE;
}

/**
 * Implements hook_node_view().
 */
function readmorecontrol_node_view($node, $view_mode, $langcode) {
  // Only if the view mode is for a teaser.
  if ($view_mode == 'teaser' && isset($node->content['links']['node']['#links']['node-readmore'])) {
    // Initiate global setting checks. This is needed if all fields are empty.
    readmorecontrol_set_entity_behaviour('node', $node, $node->type);

    // Preprocessing the display may have already discovered the behaviour.
    if (!isset($node->readmore_required)) {
      $node->readmore_required = readmorecontrol_post_process_fields($node->content, 'node', $node, $view_mode, $langcode, $node->type);
    }
    if (!$node->readmore_required) {
      unset($node->content['links']['node']['#links']['node-readmore']);
    }
  }
}

/**
 * Helper function to predetermine the global level read more settings.
 *
 * This sets one or two entity properties as required.
 *   - readmorebehaviour: The default behaviour for this entities bundle.
 *   - readmore_required: Main control flag to show or hide the read more link.
 *                        This is set we this can be determined from the global
 *                        settings.
 *
 * @param string $entity_type
 *   The name of the entity type, (i.e. 'node', 'user', etc).
 *
 * @param object $entity
 *   The entity object to check the read more control settings against.
 *
 * @param string|null $bundle
 *   Optional bundle name, (i.e. node content type), to save doing a lookup
 *   using entity_extract_ids().
 */
function readmorecontrol_set_entity_behaviour($entity_type, $entity, $bundle = NULL) {
  // Determine the default read more behaviour from the entity bundle.
  if (!isset($bundle)) {
    list($id, $vid, $bundle) = entity_extract_ids($entity_type, $entity);
  }
  if (!isset($entity->readmorebehaviour)) {
    $entity->readmorebehaviour = variable_get('readmorecontrol_' . $entity_type . '_' . $bundle, 'always');
  }
  switch ($entity->readmorebehaviour) {
    case 'always':
      $entity->readmore_required = TRUE;
      return;

    case 'never':
      $entity->readmore_required = FALSE;
      return;
  }
}
