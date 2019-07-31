<?php

/**
 * @file
 * Taxonomy related helper functions.
 */
class EntityHelperTaxonomy {

  /**
   * Gets term ids by term names and vocabulary id.
   *
   * There seems to be no other way to access the term ids by their names but
   * loading each term with taxonomy_term_load_by_name().
   * If only the term ids are needed, this seems to be a better way.
   *
   * @param string|array $names
   *   A term name or an array of term names.
   * @param int $vid
   *   A vocabulary id.
   *
   * @return string|array
   *   A term id or an array of term ids.
   */
  public function getTermIDsByNameAndVid($names, $vid) {
    $single = FALSE;
    if (is_string($names)) {
      $names = (array)$names;
      $single = TRUE;
    }
    $query = new EntityFieldQuery;
    $result = $query
        ->entityCondition('entity_type', 'taxonomy_term')
        ->propertyCondition('name', $names, 'IN')
        ->propertyCondition('vid', $vid)
        ->execute();
    if (empty($result['taxonomy_term'])) {
      return $single ? NULL : array();
    }
    return $single ? array_keys($result['taxonomy_term'])[0]
        : array_keys($result['taxonomy_term']);
  }

  /**
   * Gets term ids by term names and vocabulary name.
   *
   * @param string|array $names
   *   A term name or an array of term names.
   * @param string $vocab_name
   *   A vocabulary machine name.
   *
   * @return string|array
   *   A term id or an array of term ids.
   */
  public function getTermIDsByNameAndVocabName($names, $vocab_name) {
    $vocab = taxonomy_vocabulary_machine_name_load($vocab_name);
    if (empty($vocab->vid)) {
      $message = t('Cannot load vocabulary with machine name: @vocab_name'
          , array('@vocab_name' => $vocab_name));
      throw new Exception($message);
    }
    return $this->getTermIDsByNameAndVid($names, $vocab->vid);
  }
}