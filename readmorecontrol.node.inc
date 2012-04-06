<?php

/**
 * @file
 * Contains hooks defined in readmorecontrol_hook_info().
 */

/**
 * Implements hook_field_display_alter().
 */
function readmorecontrol_field_display_alter(&$display, $context) {
  // Only if we the entity is a node and the view mode is teaser.
  if ($context['entity_type'] == 'node' && $context['view_mode'] == 'teaser') {
    $node = $context['entity'];

    // Make sure that node_readmorecontrol in the node object has been grabbed
    // for the content type.
    if (!isset($node->node_readmorecontrol)) {
      $node->node_readmorecontrol = variable_get('node_readmorecontrol_' . $node->type, 'always');
    }

    // If node_readmorecontrol is not set to when required, just return now.
    if ($node->node_readmorecontrol != 'when_required') {
      return;
    }
    // If readmore_required has already been set on the node, just return now.
    elseif (!empty($node->readmore_required)) {
      return;
    }
    // If the field has been set to be ignored for purposes of calculating
    // whether to show the read more link, just return now.
    elseif (!empty($context['instance']['readmorecontrol_ignore'])) {
      return;
    }
    // Text fields need have their differences calculated later, but we need
    // the field instance data. So we store it on the node and return.
    elseif ($context['field']['type'] == 'text_with_summary' || $context['field']['type'] == 'text_long') {
      $fieldname = $context['field']['field_name'];
      $node->{$fieldname}['#instance'] = $context['instance'];
      return;
    }

    // Get the field display info for full view mode.
    $fulldisplay = field_get_display($context['instance'], 'full', $node);

    // If both the teaser and full view mode are hidden then we can ignore this.
    if ($display['type'] == 'hidden' && $fulldisplay['type'] == 'hidden') {
      return;
    }
    // Otherwise we will compare the display details. If they don't match set
    // the readmore_required flag on the node.
    elseif ($fulldisplay['type'] != $display['type'] ||
            $fulldisplay['settings'] !== $display['settings']) {
      $node->readmore_required = TRUE;
    }
  }
}

/**
 * Implements hook_node_view().
 */
function readmorecontrol_node_view($node, $view_mode, $langcode) {
  // Only if the view mode is for a teaser.
  if ($view_mode == 'teaser') {
    // Make sure that node_readmorecontrol in the node object has been grabbed
    // for the content type.
    if (!isset($node->node_readmorecontrol)) {
      $node->node_readmorecontrol = variable_get('node_readmorecontrol_' . $node->type, 'always');
    }

    // If node_readmorecontrol has been set to always, (i.e. not required and
    // not never), just return now.
    if ($node->node_readmorecontrol != 'when_required' && $node->node_readmorecontrol != 'never') {
      return;
    }

    // Make sure that readmore_required in the node object has been set.
    if (!isset($node->readmore_required)) {
      $node->readmore_required = FALSE;
    }

    // If node_readmorecontrol is set to when required, and readmore_required
    // isn't already set to TRUE, we need to iterate through all the renderable
    // elements to find text fields that might have a summary or have been
    // trimmed.
    if ($node->node_readmorecontrol == 'when_required' && !$node->readmore_required) {
      foreach (element_children($node->content) as $field_name) {
        $element = $node->content[$field_name];

        if (isset($element['#field_type']) && ($element['#field_type'] == 'text_with_summary' || $element['#field_type'] == 'text_long')) {
          // If the field is set with readmorecontrol_ignore, then instance will
          // get set in readmorecontrol_field_display_alter.
          if (!isset($node->{$element['#field_name']}['#instance'])) {
            continue;
          }

          // Get the instance info for the field.
          $instance = $node->{$element['#field_name']}['#instance'];

          // Need to check each item in the field to see if the content being
          // displayed matches full view mode.
          foreach ($element['#items'] as $delta => $item) {
            switch ($element['#formatter']) {
              case 'text_plain':
              case 'text_default':
                // These two will not cause read more to be displayed.
                continue;

              case 'text_trimmed':
              case 'text_summary_or_trimmed':
                $sanitized = _text_sanitize($instance, $langcode, $item, 'value');

                if ($element[$delta]['#markup'] !== $sanitized) {
                  $node->readmore_required = TRUE;
                  break 2;
                }
                break;

            }
          }
        }
      }
    }

    // Finally if readmore_required is set to FALSE, remove the link from the
    // nodes content array.
    if (!$node->readmore_required) {
      unset($node->content['links']['node']['#links']['node-readmore']);
    }
  }
}
