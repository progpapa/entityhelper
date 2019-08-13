<?php

/**
 * @file
 * Checks if fields have changed for an entity.
 */
class EntityHelperChanged {

  /**
   * Checks if certain fields of an entity have changed during an update.
   *
   * @param object $entity
   *   The entity to check.
   * @param array $field_names
   *   An array of field machine names.
   *
   * @return boolean
   *   Returns TRUE if any of the fields in $field_names has changed, FALSE
   *   otherwise.
   */
  public function haveFieldsChanged($entity, $field_names) {
    $changed = $this->haveFieldsChangedDetailed($entity, $field_names);
    return in_array(TRUE, array_values($changed));
  }

  /**
   * Checks if certain fields of an entity has changed during an update.
   *
   * @param object $entity
   *   The entity to check.
   * @param array $field_names
   *   An array of field machine names.
   *
   * @return array
   *   Each element is keyed by the field machine name and value is TRUE if that
   *   field has changed, FALSE otherwise.
   */
  public function haveFieldsChangedDetailed($entity, $field_names) {
    if (empty($entity->original)) {
      return array_fill_keys($field_names, TRUE);
    }

    $values = entityhelper_get_field_values($entity, $field_names);
    $orig_values = entityhelper_get_field_values($entity->original, $field_names, NULL, TRUE);
    $changed = array();
    foreach ($values as $field_name => $value) {
      // Some fields might return empty values in certain cases, e.g. link field
      // returns 'attributes' => array() but only for the original property.
      // Let's try to remove such values before making a comparison.
      $new_value = $this->removeEmpty($value);
      $orig_value = $this->removeEmpty($orig_values[$field_name]);
      $changed[$field_name] = !(bool)($new_value == $orig_value);
    }
    return $changed;
  }

  /**
   * Recursively remove empty elements from an associative array.
   */
  protected function removeEmpty($value) {
    if (!is_array($value)) {
      return $value;
    }
    foreach ($value as $k => $v) {
      if (is_array($v)) {
        $value[$k] = $this->removeEmpty($v);
      }
      if (empty($v)) {
        unset($value[$k]);
      }
    }
    return $value;
  }

}