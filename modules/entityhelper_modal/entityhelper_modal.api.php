<?php

/**
 * @file
 * Describe hooks provided by the Entity Helper Modal module.
 */

/**
 * Adds custom modal styles.
 *
 * @param string $style
 *   Name of the custom modal style.
 */
function hook_entityhelper_modal_add_style($style) {
  if ($style !== 'some-custom-style') {
    return;
  }
  $modal_style = array(
    'some-custom-style' => array(
      'modalSize' => array(
        'type' => 'fixed',
        'width' => 600,
        'height' => 240,
        'addWidth' => 10,
        'addHeight' => 10,
        'contentRight' => 0,
        'contentBottom' => 0,
      ),
      // add other settings
    )
  );
  drupal_add_js($modal_style, 'setting');
}