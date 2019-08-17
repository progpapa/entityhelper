<?php

/**
 * @file
 * Code for configuring field formatters to append a modal edit link to fields.
 */
class EntityHelperModalSettings {

  /**
   * Implements hook_field_formatter_settings_form_alter().
   */
  public function settingsFormAlter(array &$settings_form, array $context) {
    $settings = $context['instance']['display'][$context['view_mode']]['settings'];
    $settings_form['entityhelper_modal'] = array(
      '#type' => 'fieldset',
      '#title' => t('Modal Edit'),
      '#description' => t('Display a link that opens field edit in a modal.'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#weight' => 10,
    );
    $settings_form['entityhelper_modal']['enabled'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable modal edit'),
      '#default_value' => $settings['entityhelper_modal']['enabled'],
      '#weight' => 1,
    );
    $settings_form['entityhelper_modal']['link_text'] = array(
      '#type' => 'textfield',
      '#title' => t('Link text'),
      '#description' => t('The default text is: edit'),
      '#default_value' => $settings['entityhelper_modal']['link_text'],
      '#size' => 40,
      '#maxlength' => 100,
      '#weight' => 2,
    );
    $settings_form['entityhelper_modal']['style'] = array(
      '#type' => 'textfield',
      '#title' => t('Modal style'),
      '#description' => t('Apply custom style to the modal.'),
      '#default_value' => $settings['entityhelper_modal']['style'],
      '#size' => 40,
      '#maxlength' => 100,
      '#weight' => 3,
    );
    $settings_form['entityhelper_modal']['link_classes'] = array(
      '#type' => 'textfield',
      '#title' => t('Custom link class names'),
      '#description' => t('Will be added to the edit link. Separate with spaces.'),
      '#default_value' => $settings['entityhelper_modal']['link_classes'],
      '#size' => 40,
      '#maxlength' => 100,
      '#weight' => 4,
    );
    $settings_form['entityhelper_modal']['wrapper_classes'] = array(
      '#type' => 'textfield',
      '#title' => t('Custom wrapper class names'),
      '#description' => t('Will be added to the wrapper. Separate with spaces.'),
      '#default_value' => $settings['entityhelper_modal']['wrapper_classes'],
      '#size' => 40,
      '#maxlength' => 100,
      '#weight' => 4,
    );
  }

  /**
   * Implements hook_field_formatter_settings_summary_alter().
   */
  public function settingsSummaryAlter(&$summary, $context) {
    $settings = $context['instance']['display'][$context['view_mode']]['settings'];
    $style = empty($settings['entityhelper_modal']['style'])
        ? t('default') : $settings['entityhelper_modal']['style'];
    if (!empty($settings['entityhelper_modal']['enabled'])) {
      $summary .= '<br />'
        . t('Modal edit enabled, link text: @link_text, style: @style', array(
          '@link_text' => $settings['entityhelper_modal']['link_text'],
          '@style' => $style,
        ));
    }
  }

  /**
   * Implements hook_field_attach_view_alter().
   */
  public function fieldAttachViewAlter(&$output, $context) {
    if (empty($context['view_mode']) || empty($context['display'])) {
      return;
    }

    $entity_type = $context['entity_type'];
    $entity = $context['entity'];
    $view_mode = $context['view_mode'];
    list($id, $vid, $bundle) = entity_extract_ids($entity_type, $entity);

    foreach (field_info_instances($entity_type, $bundle)
    as $field_name => $instance) {
      $display = field_get_display($instance, $view_mode, $context['entity']);
      if (empty($output[$field_name])) {
        continue;
      }
      if (!empty($display['settings']['entityhelper_modal']['enabled'])) {
        $output[$field_name]['#post_render'] = array('entityhelper_modal_post_render_add_link');
        $output[$field_name]['#entityhelper_modal']
            = $display['settings']['entityhelper_modal'];
      }
    }
  }

  /**
   * @see entityhelper_modal_post_render_add_link()
   */
  public function postRenderAddLink($html, $render) {
    ctools_include('ajax');
    ctools_include('modal');
    ctools_modal_add_js();

    $settings = $this->getSettings($render);
    // Implement hook_entityhelper_modal_add_style() to add custom modal styles.
    if (!empty($settings['style'])) {
      module_invoke_all('entityhelper_modal_add_style', $settings['style']);
    }

    $link = ctools_modal_text_button($settings['link_text'], $settings['url']
        , '', implode(' ', $settings['link_classes']));

    return $this->buildFieldHTML($html, $link, $settings);
  }

  /**
   * Builds the HTML output of the field.
   *
   * Adds custom attributes like class names and data attributes to the wrapper
   * and appends a modal edit link.
   *
   * @param string $html
   *   The original HTML output of the field.
   * @param string $link
   *   A modal link.
   * @param array $settings
   *   Configuration settings.
   *
   * @return string
   *   The update HTML output of the field.
   */
  protected function buildFieldHTML($html, $link, $settings) {
    $attributes = array(
      'class' => $settings['wrapper_classes'],
      'data-entitytype' => $settings['entity_type'],
      'data-entityid' => $settings['entity_id'],
      'data-fieldname' => $settings['field_name'],
    );
    return '<div ' . drupal_attributes($attributes) . '>'
        . $html . $link . '</div>';
  }

  /**
   * Gets all configuration settings for the modal link.
   *
   * @param array $render
   *   Field render array from the post_render function.
   *
   * @return array
   *   An associative array with the following elements:
   *   - enabled: 0/1 whether modal link is enabled for the field or not
   *   - link_text: anchor text for the modal link, if empty t('edit') is used
   *   - link_classes: array, additional classes to apply to the link
   *   - wrapper_classes: array, classes apply to the entire field
   *   - entity_type: the machine name of the entity type the field belongs to
   *   - entity_id: well, the entity id
   *   - field_name: machine name of the field
   *   - url: the url of the modal that will be opened when clicking on the link
   */
  protected function getSettings($render) {
    $entity_type = $render['#entity_type'];
    $field_name = $render['#field_name'];
    $entity_id = entity_id($entity_type, $render['#object']);
    $instances = field_info_instances($entity_type, $render['#bundle']);
    $instance = $instances[$field_name];
    $display = field_get_display($instance, $render['#view_mode']
        , $render['#object']);

    $settings = $display['settings']['entityhelper_modal'];
    if (empty($settings['link_text'])) {
      $settings['link_text'] = t('edit');
    }

    $wrapper_classes = array('entityhelper-modal-wrapper');
    if (!empty($settings['wrapper_classes'])) {
      $custom_wrapper_classes = explode(' ', $settings['wrapper_classes']);
      foreach ($custom_wrapper_classes as $class_name) {
        $wrapper_classes[] = drupal_html_class($class_name);
      }
    }
    $settings['wrapper_classes'] = $wrapper_classes;
    $link_classes = array('entityhelper-modal-link');
    if (!empty($settings['link_classes'])) {
      $custom_link_classes = explode(' ', $settings['link_classes']);
      foreach ($custom_link_classes as $class_name) {
        $link_classes[] = drupal_html_class($class_name);
      }
    }
    if (!empty($settings['style'])) {
      $link_classes[] = 'ctools-modal-' . $settings['style'];
    }
    $settings['link_classes'] = $link_classes;

    $settings['entity_type'] = $entity_type;
    $settings['field_name'] = $field_name;
    $settings['entity_id'] = $entity_id;
    $settings['url'] = 'entityhelper_modal/edit/' . $entity_type . '/'
        . $entity_id . '/' . $field_name . '/nojs';
    return $settings;
  }
}
