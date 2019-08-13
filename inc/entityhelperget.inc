<?php

/**
 * @file
 * Get field values for an entity.
 */
class EntityHelperGet {

  /**
   * Gets one or more field values of an entity.
   *
   * @see entityhelper_get_field_values()
   */
  public function get($entity, $fields, EntityHelperConfig $config = NULL, $skip_cache = FALSE) {
    // If wrong data is provided, e.g. empty $entity, return NULL for all fields.
    if (empty($entity) OR ! is_object($entity)) {
      if (is_array($fields)) {
        return array_fill_keys($fields, NULL);
      }
      return NULL;
    }

    $wrapper = entityhelper_get_wrapper($entity, $skip_cache);
    // Single field name passed:
    if (is_string($fields)) {
      return $this->getFieldValue($wrapper, $fields, $config);
    }
    // Multiple names passed in $fields:
    $values = array();
    foreach ($fields as $field) {
      $values[$field] = $this->getFieldValue($wrapper, $field, $config);
    }
    return $values;
  }

  /**
   * Gets field value of an entity.
   *
   * @param EntityMetadataWrapper $wrapper
   * @param string $field_name
   *   Field machine name.
   * @param EntityHelperConfig $config
   *   Configuration object.
   *   @see EntityHelperConfig
   *
   * @return mixed
   *   The field value as returned by EntityMetadataWrapper::value(). Some
   *   additional changes might be done, depending on the $config array.
   */
  protected function getFieldValue(EntityMetadataWrapper $wrapper, $field_name, EntityHelperConfig $config = NULL) {
    try {
      $value = $wrapper->{$field_name}->value();
      // Handle formatted text field and entity references.
      if (!is_object($value) && !empty($value['value'])) {
        $value = $value['value'];
      }
      if (!empty($config)) {
        $value = $this->applyConfigToValue($wrapper, $field_name, $value, $config);
      }
    } catch (Exception $e) {
      $value = NULL;
    }
    return $value;
  }

  /**
   * Applies custom changes to field values.
   *
   * @param EntityMetadataWrapper $wrapper
   * @param string $field_name
   *   Field machine name.
   * @param mixed $value
   *   The field value as returned by EntityMetadataWrapper::value().
   * @param EntityHelperConfig $config
   *   Configuration object.
   *   @see EntityHelperConfig
   *
   * @return mixed
   *   The updated field value.
   */
  protected function applyConfigToValue(EntityMetadataWrapper $wrapper, $field_name, $value, EntityHelperConfig $config = NULL) {
    $conf = $config->getConfigForField($field_name);
    if (empty($conf) OR is_null($value)) {
      return $value;
    }
    switch ($conf['type']) {
      case 'term_id':
        $value = $this->getTaxonomyTermIDs($value);
        break;
      case 'term_name':
        $value = $this->getTaxonomyTermNames($value);
        break;
      case 'term_link':
        $value = $this->getTaxonomyTermLinks($value);
        break;
      case 'list_value':
        $value = $this->getListValues($wrapper, $field_name, $value);
        break;
      case 'entity_id':
        $value = $this->getEntityIDs($value);
        break;
      case 'entity_label':
        $value = $this->getEntityLabels($value);
      case 'date':
        $value = $this->getFormattedDates($value, $conf);
        break;

      default:
        break;
    }
    return $value;
  }

  /**
   * Return term ids instead of taxonomy objects.
   */
  protected function getTaxonomyTermIDs($value) {
    return $this->applyConfigToTaxonomy($value, 'tid');
  }

  /**
   * Return term names instead of taxonomy objects.
   */
  protected function getTaxonomyTermNames($value) {
    return $this->applyConfigToTaxonomy($value, 'name');
  }

  /**
   * Return links to term pages instead of taxonomy objects.
   */
  protected function getTaxonomyTermLinks($value) {
    if (is_object($value) && !empty($value->tid)) {
      return l($value->name, 'taxonomy/term/' . $value->tid);
    }
    $ret = array();
    foreach ($value as $term) {
      $ret[] = l($term->name, 'taxonomy/term/' . $term->tid);
    }
    return $ret;
  }

  /**
   * Helper function to extract the name or the ID of a term.
   */
  protected function applyConfigToTaxonomy($value, $prop) {
    // Single value field, $value is a term object.
    if (is_object($value) && !empty($value->{$prop})) {
      return $value->{$prop};
    }
    // Else this is a multivalue field, $value is an array of term objects.
    $ret = array();
    foreach ($value as $term) {
      if (!empty($term->{$prop})) {
        $ret[] = $term->{$prop};
      }
    }
    return $ret;
  }

  /**
   * For list fields the default is to return keys. This function changes keys
   * to values.
   */
  protected function getListValues(EntityMetadataWrapper $wrapper, $field_name, $value) {
    // Boolean fields are treated as lists. FALSE should be changed to 0 in
    // order to get the appropriate label.
    if ($value === FALSE) {
      $value = 0;
    }
    $keys = is_array($value) ? $value : (array) $value;
    try {
      $allowed = $wrapper->{$field_name}->optionsList();
      $values = array_values(array_intersect_key(
              $allowed, array_combine($keys, $keys)));
    } catch (Exception $e) {
      $values = $keys;
    }
    // If the original $value is an array, the field accepts multiple values,
    // so return $values as array. However if $value is not an array, we only
    // need the first (and probably the only) element of $values.
    if (is_array($value)) {
      return $values;
    }
    return array_shift($values);
  }

  /**
   * For entity references the default is to return the entity. This function
   * returns the ids instead.
   */
  protected function getEntityIDs($value) {
    if (is_object($value)) {
      return entityhelper_get_id($value);
    }
    $ret = array();
    foreach ($value as $entity) {
      $ret[] = entityhelper_get_id($entity);
    }
    return $ret;
  }

  /**
   * For entity references the default is to return the entity. This function
   * returns the label instead.
   */
  protected function getEntityLabels($value) {
    if (is_object($value)) {
      return entity_label($value->base_type, $value);
    }
    $ret = array();
    foreach ($value as $entity) {
      $ret[] = entity_label($entity->base_type, $entity);
    }
    return $ret;
  }

  /**
   * Formats timestamps as dates.
   *
   * @param string $value
   *   A UNIX timestamp.
   * @param array $conf
   *   An array with a 'date_format' key, value is a valid PHP date format.
   *
   * @return string
   *   The formatted date.
   */
  protected function getFormattedDates($value, $conf) {
    if (is_string($value)) {
      return empty($conf['date_format'])
        ? $value : date($conf['date_format'], $value);
    }
    $ret = array();
    foreach ($value as $date) {
      $ret[] = empty($conf['date_format'])
        ? $value : date($conf['date_format'], $date);
    }
    return $ret;
  }

}
