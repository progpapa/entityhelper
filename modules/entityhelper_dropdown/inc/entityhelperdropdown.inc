<?php

/**
 * @file
 * Code for the entityhelper_dropdown field formatter.
 */
class EntityHelperDropdown {

  /**
   * Implements hook_field_formatter_settings_summary().
   */
  public function formatterSettingsSummary($field, $instance, $view_mode) {
    $display = $instance['display'][$view_mode];
    if (($display['type'] !== 'entityhelper_dropdown')) {
      return;
    }
    $settings = $display['settings'];
    $info = array();
    $fallback = $settings['fallback'];
    $info[] = '<p>' . t('Fallback formatter: @fallback'
        , array('@fallback' => $fallback)) . '</p>';
    if (!empty($settings['callbacks'])) {
      $info[] = '<p>' . t('Callbacks: @callbacks'
          , array('@callbacks' => $settings['callbacks'])) . '</p>';
    }
    $summary = implode('', $info);
    return $summary;
  }

  /**
   * Implements hook_field_formatter_settings_form().
   */
  public function fieldFormatterSettingsForm($field, $instance, $view_mode, $form, &$form_state) {
    $display = $instance['display'][$view_mode];
    if (($display['type'] !== 'entityhelper_dropdown')) {
      return array();
    }
    $settings = $display['settings'];
    $element = array();
    $element['fallback'] = array(
      '#title' => t('Fallback:'),
      '#description' => t('This formatter will be used if a user cannot edit the field.'),
      '#type' => 'radios',
      '#options' => $this->getFormatterOptions($field['type'], 'entityhelper_dropdown'),
      '#default_value' => $settings['fallback'],
      '#required' => TRUE,
      '#weight' => 0
    );

    $element['callbacks'] = array(
      '#type' => 'textarea',
      '#title' => t('Callbacks'),
      '#description' => t('Enter one per line'),
      '#default_value' => $settings['callbacks'],
      '#rows' => 10,
      '#columns' => 40,
      '#required' => FALSE,
      '#weight' => 20,
    );

    return $element;
  }

  /**
   * Implements hook_field_formatter_view().
   *
   * @param bool $force_empty
   *   Builds the render array even if $items is empty. This happens when a
   *   field has not been set. The empty_fields module requires this to display
   *   the dropdown even if the field is not yet set.
   *
   */
  public function fieldFormatterView($entity_type, $entity, $field, $instance, $langcode, $items, $display, $force_empty = FALSE) {
    if (($display['type'] != 'entityhelper_dropdown')) {
      return;
    }
    // If $items is empty, we must not return a render array as it won't be
    // displayed, but it will interfere with the empty_fields module.
    if (empty($items) && !$force_empty) {
      return;
    }

    if (!entityhelper_user_can_edit_field($entity, $field['field_name'])) {
      return $this->displayGivenFormatter($entity_type, $entity, $field,
          $items, $langcode, $display['settings']['fallback']);
    }

    $values =array_column($items, 'value');
    $options = list_allowed_values($field);

    if (empty($instance['required'])) {
      // Possible better way: options_field_widget_form
      // _options_properties()
      // _options_get_options()
      // @todo: add this to a list helper class with list_allowed_values.
      $vars['instance'] = $instance;
      $vars['option'] = 'option_none';
      $option_none_text = theme('options_none', $vars);
      // $option_none_text is empty for boolean fields.
      if (empty($option_none_text)) {
        $option_none_text = t('- Select a value -');
      }
      $options = array('_none' => $option_none_text) + $options;
      if (empty($values)) {
        $values = array('_none');
      }
    }

    $elem = array(
      '#theme' => 'select',
      '#options' => $options,
      '#value' => $values,
      '#attributes' => array(
        'class' => array('entityhelper-dropdown'),
        // @todo: implement this?
        // 'data-required' => ...
        // @todo: adding all these could go into a helper funcion for fields
        'data-entityid' => entityhelper_get_id($entity),
        'data-entitytype' => $entity_type,
        'data-fieldname' => $field['field_name'],
        'data-cardinality' => $field['cardinality'],
        // This will be used in js when more values selected than allowed.
        'data-last-selection' => json_encode($values),
      ),
      '#attached' => array(
        'js' => array(drupal_get_path('module', 'entityhelper_dropdown')
                  . '/js/entityhelper_dropdown.js'),
      ),
    );
    if (!empty($display['settings']['callbacks'])) {
      $elem['#attributes']['data-callbacks'] = str_replace(
          array("\n", ' '), array(',', ''), $display['settings']['callbacks']);
    }
    if ($field['cardinality'] != 1) {
      $elem['#attributes']['multiple'] = 'multiple';
    }
    $element[] = $elem;
    return $element;
  }

  /**
   * Makes sure the dropdown is built even if the field has not been set yet.
   */
  public function forceFieldFormatterView($entity_type, $entity, $field, $instance, $langcode, $items, $display, $force_empty = TRUE) {
    return $this->fieldFormatterView($entity_type, $entity, $field, $instance, $langcode, $items, $display, $force_empty);
  }

  /**
   * Gets the default formatter for a field.
   *
   * @param string $type
   *   Field type name.
   *
   * @return string|NULL
   *   The name of the default field formatter.
   *
   * @todo: Not used, could be removed or moved to a field helper class.
   */
  protected function getDefaultFormatter($type) {
    return empty(field_info_field_types($type)['default_formatter'])
      ? NULL : field_info_field_types($type)['default_formatter'];
  }

  /**
   * Gets all allowed formatter for a field type. Optionally excludes formatters.
   *
   * @param string $type
   *   Field type name.
   * @param string $exclude
   *   Optionally exclude a formatter.
   *
   * @return array
   *   An associative array keyed by formatter machine names, values are
   *   formatter labels. Includes all formatters for a given field type.
   *
   * @todo: This might be moved into a separate field helper class.
   * @todo: $exclude should be optional and it should be an array
   */
  protected function getFormatterOptions($type, $exclude) {
    $all_formatters = field_info_formatter_types();
    unset($all_formatters[$exclude]);
    $allowed_formatters = array();
    foreach ($all_formatters as $name => $formatter) {
      if (in_array($type, $formatter['field types'])) {
        $allowed_formatters[$name] = $formatter['label'];
      }
    }
    return $allowed_formatters;
  }

  /**
   * Displays a field using a given formatter (e.g. fallback formatter.
   *
   * @return array
   *   A render array for the field.
   */
  protected function displayGivenFormatter($entity_type, $entity, $field, $items, $langcode, $formatter_name) {
    $ret = array();
    $formatter = field_info_formatter_types($formatter_name);
    foreach ($items as $item) {
      $ret[] = field_view_value($entity_type, $entity, $field['field_name'],
          $item, $formatter, $langcode);
    }
    return $ret;
  }

}