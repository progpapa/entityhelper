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
   * @param EntityHelperConfig $config
   *   @see EntityHelperConfig
   */
  public function setFields($entity, $values, $skip_save = FALSE, EntityHelperConfig $config = NULL) {
    $wrapper = entityhelper_get_wrapper($entity);
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

  protected function applyConfigToValue(EntityMetadataWrapper $wrapper, $field_name, $value, EntityHelperConfig $config = NULL) {
    $conf = $config->getConfigForField($field_name);
    if (empty($conf) OR is_null($value)) {
      return $value;
    }

    switch ($conf['type']) {
      case 'term_name':
        $value = $this->setTaxonomyTermByName($wrapper, $field_name, $value);
        break;
      default:
        break;
    }
    return $value;
  }

  /**
   * For taxonomy terms the default is to set by term id. This function takes
   * term names and returns the appropriate term ids.
   */
  protected function setTaxonomyTermByName(EntityMetadataWrapper $wrapper, $field_name, $value) {
    $field_info = field_info_field($field_name);
    // This should never happen for term reference fields.
    if (empty($field_info['settings']['allowed_values'][0]['vocabulary'])) {
      $message = t('Looks like @field is not a term reference field.'
          , array('@field' => $field_name));
      throw new Exception($message);
    }
    $vocab_name = $field_info['settings']['allowed_values'][0]['vocabulary'];
    return entityhelper_container()->EntityHelperTaxonomy
        ->getTermIDsByNameAndVocabName($value, $vocab_name);
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
