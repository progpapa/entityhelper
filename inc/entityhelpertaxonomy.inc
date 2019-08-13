<?php

/**
 * @file
 * Taxonomy related helper functions.
 */
class EntityHelperTaxonomy {

  /**
   * @see entityhelper_get_tids_by_name_and_vocab_id()
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
   * @see entityhelper_get_tids_by_name_and_vocab_name()
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