<?php

/**
 * @file
 * Code for the entityhelper_dropdown field formatter.
 */
class EntityHelperModal {

  /**
   * Implements hook_field_formatter_settings_form_alter().
   */
  public function settingsFormAlter(array &$settings_form, array $context) {
    $settings = $context['instance']['display'][$context['view_mode']]['settings'];
    $settings_form['entityhelper_modal'] = array(
      '#type' => 'fieldset',
      '#title' => t('Modal Edit'),
      '#description' => t('Display a link that opens field edit in a modal.'),
      '#weight' => -10,
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
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
      '#default_value' => $settings['entityhelper_modal']['link_text'],
      '#size' => 40,
      '#maxlength' => 100,
      '#weight' => 2,
    );
    $settings_form['entityhelper_modal']['display'] = array(
      '#type' => 'radios',
      '#title' => t('Display as'),
      '#default_value' => $settings['entityhelper_modal']['display'],
      '#weight' => 3,
      '#options' => array(
        'block' => t('Block - wrapped in div'),
        'inline' => t('Inline - wrapped in span')),
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
    if (!empty($settings['entityhelper_modal']['enabled'])) {
      $summary .= '<br />' . t('Modal edit enabled, link text: @link_text'
              , array('@link_text' => $settings['entityhelper_modal']['link_text']));
    }
  }

  /**
   * Implements hook_field_attach_view_alter().
   */
  public function fieldAttachViewAlter(&$output, $context) {
    $view_mode = $context['view_mode'];
    foreach (field_info_instances($output['#entity_type'], $output['#bundle'])
    as $field_name => $instance) {
      $display = field_get_display($instance, $view_mode, $context['entity']);
      if (empty($output[$field_name])) {
        continue;
      }
      if (!empty($display['settings']['entityhelper_modal']['enabled'])) {
        $output[$field_name]['#post_render'] = array('entityhelper_modal_post_render_add_link');
      }
    }
  }

  /**
   * @see entityhelper_modal_post_render_add_link()
   */
  public function postRenderAddLink($html, $render) {
    $entity_type = $render['#entity_type'];
    $field_name = $render['#field_name'];
    $entity_id = entity_id($entity_type, $render['#object']);
    $instances = field_info_instances($entity_type, $render['#bundle']);
    $instance = $instances[$field_name];
    $display = field_get_display($instance, $render['#view_mode']
        , $render['#object']);
    $link_text = $display['settings']['entityhelper_modal']['link_text'];
    $url = 'entityhelper_modal/edit/' . $entity_type . '/' . $entity_id
        . '/' . $field_name . '/nojs';
    $link = array(
      '#theme' => 'link',
      '#text' => $link_text,
      '#path' => $url,
      '#options' => array(
        'attributes' => array(),
        'html' => FALSE
      ),
    );
    return $html . drupal_render($link);
  }
}
