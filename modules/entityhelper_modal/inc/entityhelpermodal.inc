<?php

/**
 * @file
 * Builds a modal to edit fields.
 */
class EntityHelperModal {

  /**
   * @see entityhelper_modal_modal_build()
   */
  public function build($entity, $field_name, $js) {
    if ($js) {
      ctools_include('ajax');
      ctools_include('modal');

      $form_state = array('ajax' => TRUE);
      $form_state['build_info']['args'] = array($entity, $field_name);
  //    $form_state['storage']['entity'] = $entity;
  //    $form_state['storage']['field_name'] = $field_name;

      $output = ctools_modal_form_wrapper('entityhelper_modal_form', $form_state);
      if (!empty($form_state['executed'])) {
        $output = array();
        // Adds ajax commands to update content on the parent page.
        $this->setReplace($output, $entity, $field_name);
        $output[] = ctools_modal_command_dismiss();
      }
      print ajax_render($output);
      drupal_exit();
    }
    else {
      return drupal_get_form('entityhelper_modal_form', $entity, $field_name);
    }
  }

  /**
   * @see entityhelper_modal_form()
   */
  public function form($form, &$form_state, $entity, $field_name) {
    $entity_type = $entity->base_type;
    $langcode = LANGUAGE_NONE;

    $form_state['storage']['entity'] = $entity;
    $form_state['storage']['field_name'] = $field_name;

    // Adds the form elements for only the specified field to the form.
    $options = array(
      'field_name' => $field_name,
      'language' => field_valid_language($langcode),
      'default' => TRUE,
    );
    field_attach_form($entity_type, $entity, $form, $form_state, $langcode, $options);
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
      '#weight' => 9000,
    );
    $form['cancel'] = array(
      '#type' => 'button',
      '#value' => t('Cancel'),
      '#name' => 'cancel',
      '#executes_submit_callback' => TRUE,
      '#limit_validation_errors' => array(),
      '#attributes' => array('class' => array('ctools-close-modal')),
      '#weight' => 10000,
    );
    return $form;
  }

  /**
   * @see entityhelper_modal_form_validate()
   */
  public function validate($form, &$form_state) {
    $entity = $form_state['storage']['entity'];
    $field_name = $form_state['storage']['field_name'];
    field_attach_form_validate($entity->base_type, $entity, $form, $form_state, array('field_name' =>  $field_name));
  }

  /**
   * @see entityhelper_modal_form_submit()
   */
  public function submit($form, &$form_state) {
    $entity = $form_state['storage']['entity'];
    if ($form_state['triggering_element']['#value'] === t('Submit')) {
      entityhelper_save($entity);
    }
    // @todo: JS is disabled, try to find out a good place to redirect after submission.
    if (!function_exists('ctools_modal_command_dismiss')) {
      drupal_goto($entity->base_type . '/' . entityhelper_get_id($entity));
    }
  }

  /**
   * Adds ajax command to update the appropriate field in the parent page.
   */
  protected function setReplace(&$output, $entity, $field_name) {
    $value = field_view_field($entity->base_type, $entity, $field_name, 'default');
    $selector = 'div[data-entityid="' . entityhelper_get_id($entity)
        . '"][data-entitytype="' . $entity->base_type . '"][data-fieldname="'
        . $field_name . '"]';
    $output[] = ajax_command_replace($selector, drupal_render($value));
  }
}
