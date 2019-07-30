<?php

/**
 * @file
 * Set field values for an entity.
 */
class EntityHelperSet {

  /**
   * Updates entity fields.
   *
   * @param string $entity_type
   * @param integer $entity_id
   * @param array $values
   *   An associative array keyed by field machine names, values are field
   *   values that should be set.
   * @param bool $skip_save
   *   Flag to indicate if the entity shouldn't be saved.
   */
  public function setFieldsByID($entity_type, $entity_id, $values, $skip_save = FALSE, $config = array()) {
    $entity = entity_load_single($entity_type, $entity_id);
    $this->setFields($entity, $values, $skip_save, $config);
  }

  /**
   * Updates entity fields.
   *
   * @param object $entity
   * @param array $values
   *   An associative array keyed by field machine names, values are field
   *   values that should be set.
   * @param bool $skip_save
   *   Flag to indicate if the entity shouldn't be saved.
   */
  public function setFields($entity, $values, $skip_save = FALSE, $config = array()) {
    $wrapper = entityhelper_get_wrapper($entity);
    // If there are custom settings on how the data should be saved, we need
    // to store the fields info for later use.
    if (!empty($config)) {
      $config['info'] = $wrapper->getPropertyInfo();
    }
    foreach ($values as $field_name => $value) {
      if (!empty($config)) {
        $value = $this->applyConfigToValue($wrapper, $field_name, $value, $config);
      }
      $this->setFieldValue($wrapper, $field_name, $value);
    }
    if (!$skip_save) {
      $wrapper->save();
    }
  }

  protected function applyConfigToValue($wrapper, $field_name, $value, $config) {
    // Sanity check, we need some info about all the fields in $config['info']
    // and we don't touch fields with no value.
    if (empty($config['info']) OR is_null($value)) {
      return $value;
    }

    $map = field_info_field_map();
    // This is a regular field.
    if (!empty($map[$field_name]['type'])) {
      $field_type = $map[$field_name]['type'];
    }
    // This is a proerty with known type.
    elseif (!empty($config['info'][$field_name]['type'])) {
      $field_type = $config['info'][$field_name]['type'];
    }
    // This is a property with undefined type. (e.g. node title)
    else {
      $field_type = 'unknown';
    }
    switch ($field_type) {
//      case 'list_text':
//      case 'list_boolean':
//        return $this->getListValuesByKeys($wrapper, $field_name, $config, $value);
      case 'taxonomy_term_reference':
        return $this->setTaxonomyTerms($wrapper, $field_name, $value, $config);
//      case 'datetime':
//      case 'date':
//        return $this->getDate($wrapper, $field_name, $config, $value);
      default:
        break;
    }
    return $value;
  }

  /**
   * For taxonomy terms the default is to return the term object. This function
   * finds out if names, term ids or links should be returned.
   */
  protected function setTaxonomyTerms(EntityMetadataWrapper $wrapper, $field_name, $value, $config) {
    // Do nothing is the calling function didn't explicitly asked us.
    if (empty($config['term'])) {
      return $value;
    }

    $ret = array();
    switch ($config['term']) {
      case 'names':
        $field_info = field_info_field($field_name);
        $vocabulary = $field_info['settings']['allowed_values'][0]['vocabulary'];
        $term_arr = taxonomy_get_term_by_name($value, $vocabulary);
        return empty($term_arr) ? NULL : key($term_arr);
      default:
        $ret = $value;
        break;
    }
    return $ret;
  }

  /**
   * Helper function that tries to correctly set the value of a field.
   *
   * @param EntityMetadataWrapper $wrapper
   * @param string $field_name
   * @param mixed $value
   */
  private function setFieldValue(EntityMetadataWrapper $wrapper, $field_name, $value) {
    // _none is usually passed when unsetting list fields.
    if ($value === '_none') {
      $value = NULL;
    }
    try {
      // This should usually do the trick.
      $wrapper->{$field_name}->set($value);
    } catch (Exception $e) {
      $field_info = field_info_field($field_name);
      if (isset($field_info['columns']['value'])) {
        $this->setFieldValue($wrapper, $field_name, array('value' => $value));
      }
      else {
        watchdog('entityhelper', t('Unable to update field. Entity type: @type,'
                . ' entity id: @id, field_name: @fieldname, value: @value', array(
          '@type' => $wrapper->type(),
          '@id' => $wrapper->getIdentifier(),
          '@fieldname' => $field_name,
          '@value' => $value,
                )
        ));
        watchdog_exception('EntityHelperSet::setFieldValue()', $e);
      }
    }
  }
}
