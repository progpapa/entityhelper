<?php

/**
 * @file
 * Configuration settings on how field values should be get or set.
 */
class EntityHelperConfig {

  /**
   * Field level configuration.
   *
   * Associative array, keyed by field machine names, value is a configuration
   * settings for the appropriate field. The configuration array is an
   * associative array with the following keys:
   * - type (required): the type of data to use, e.g. term_name
   * - date_format: for date fields, return dates in this format, e.g.: "Y. m. d."
   *
   * @var array
   */
  private $fields = array();

  /**
   *
   * @param array $fields
   *   An associative array keyed by field machine names, values define how a
   *   field value should be transformed. Both field getters and setters use
   *   this configuration object.
   *
   *   Possible values when getting field values:
   *   - term_id: return taxonomy term ids instead of objects
   *   - term_name: return taxonomy term names instead of objects
   *   - term_link: return links to taxonomy term pages instead of objects
   *   - list_value: return list values instead of keys, also applies to boolean
   *                 fields, the label is returned in that case
   *   - entity_id: return entity ids instea of entities
   *   - entity_label: return entity labels instea of entities
   *   - date: return a formatted date instead of a timestamp, need to specify
   *           a date format with EntityHelperConfig::addDateFormat()
   *           @see EntityHelperConfig::addDateFormat()
   *
   *   Possible values when settings field values:
   *   - term_name: term names are expected, change them to term ids
   *   - list_value: list values are expecte, change them to keys
   */
  public function __construct($fields) {
    foreach ($fields as $field_name => $return_type) {
      $this->fields[$field_name]['type'] = $return_type;
    }
  }

  /**
   * Gets the configuration for a certain field.
   *
   * @return array
   *   The appropriate elements of the EntityHelperConfig::fields array.
   */
  public function getConfigForField($field_name) {
    return empty($this->fields[$field_name]) ? NULL : $this->fields[$field_name];
  }

  /**
   * Adds PHP date format to date fields.
   *
   * @param string $format
   *   A valid PHP date format.
   * @param string|array $fields
   *   A field name or array of field names to apply this format. If empty, the
   *   format will be applied to all fields with 'type' => 'date'.
   */
  public function addDateFormat($format, $fields = NULL) {
    foreach ($this->fields as $field_name => &$config) {
      if ($config['type'] !== 'date') {
        continue;
      }
      if (!empty($fields) && in_array($field_name, $fields)) {
        $config['date_format'] = $format;
      }
      elseif (empty($fields)) {
        $config['date_format'] = $format;
      }
    }
  }
}
