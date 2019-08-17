<?php

/**
 * Page callback: Displays a modal edit form for a field.
 *
 * @param object $entity
 *   The entity that is being edited.
 * @param string $field_name
 *   The machine name of the field.
 * @param bool $js
 *   Whether or not javascript is enabled.
 *
 * @see entityhelper_modal_menu()
 *
 * @todo: make this multilingual, language code should be passed from hook_menu
 */
function entityhelper_modal_modal_build($entity, $field_name, $js) {
  entityhelper_container()->EntityHelperModal->build($entity, $field_name, $js);
}

/**
 * Form constructor for the entityhelper_modal_form form.
 */
function entityhelper_modal_form($form, &$form_state, $entity, $field_name) {
  return entityhelper_container()->EntityHelperModal
      ->form($form, $form_state, $entity, $field_name);
}

/**
 * Form validation handler for entityhelper_modal_form().
 *
 * @see entityhelper_modal_form()
 * @see entityhelper_modal_form_submit()
 */
function entityhelper_modal_form_validate($form, &$form_state) {
  entityhelper_container()->EntityHelperModal->validate($form, $form_state);
}

/**
 * Form submission handler for entityhelper_modal_form().
 *
 * @see entityhelper_modal_form()
 * @see entityhelper_modal_form_validate()
 */
function entityhelper_modal_form_submit($form, &$form_state) {
  entityhelper_container()->EntityHelperModal->submit($form, $form_state);
}