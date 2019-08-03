<?php

/**
 * @file
 * Contains the EntityhelperDropdownEmpty plugin for EmptyFieldHandler.
 */
class EntityhelperDropdownEmpty extends EmptyFieldHandler {

  /**
   * Implementation of EmptyFieldText::react().
   *
   * Sets up variable and calls
   * EntityHelperDropdown::renderFieldFormatterView() to dislay a dropdown
   * with no values pre-selected.
   */
  public function react($context) {
    $display = field_info_instance($context['entity_type'],
        $context['field_name'], $context['entity']->type);
    $view_modes = array_keys($display['display']);
    $view_mode = in_array($context['view_mode'], $view_modes)
        ? $context['view_mode'] : 'default';
    $dropdown = entityhelper_container()->EntityHelperDropdown
      ->forceFieldFormatterView(
        $context['entity_type'],
        $context['entity'],
        $context['field'],
        $context['instance'],
        $context['language'],
        array(),
        $display['display'][$view_mode],
        TRUE
      );
    return drupal_render($dropdown);
  }

  /**
   * Implementation of EmptyFieldText:summaryText().
   */
  public function summaryText() {
    return t('Empty field is displayed as dropdown');
  }

}