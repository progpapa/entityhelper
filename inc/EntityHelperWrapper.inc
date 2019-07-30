<?php

/**
 * @file
 * Get metadatawrapper for entities. Wrappers are stored in a variable to avoid
 * wrapping the same entity multiple times.
 */
class EntityHelperWrapper {

  /**
   * This array holds all the wrappers. Structure is the following:
   * 'entity_type' => array('entity_id1' => wrapper1, ...), ...
   * @var array
   */
  private $wrappers = array();

  /**
   * Gets the metadatawrapper of an entity. Makes sure the entity is wrapped
   * only once.
   *
   * @param string $entity_type
   * @param int $entity_id
   * @param object $entity
   *   Optional, might not be loaded when calling this method. If not passed,
   *   the function will load the entity.
   *
   * @return EntityMetadataWrapper
   *
   * @todo: handle empty ids, i.e. entity creation
   */
  public function get_by_id($entity_type, $entity_id, $entity = NULL) {
    if (isset($this->wrappers[$entity_type][$entity_id])) {
      return $this->wrappers[$entity_type][$entity_id];
    }
    if (is_null($entity)) {
      $entity = entity_load_single($entity_type, $entity_id);
    }
    // Check if wrapper is already stored temporarily. In this case the entity
    // has been created recently and hasn't been wrapped yet after saving.
    if (!empty($entity->entityhelper_temp_id)
        && !empty($this->wrappers[$entity_type]['temp'][$entity->entityhelper_temp_id])) {
      $this->wrappers[$entity_type][$entity_id]
          = $this->wrappers[$entity_type]['temp'][$entity->entityhelper_temp_id];
      unset($this->wrappers[$entity_type]['temp'][$entity->entityhelper_temp_id]);
      unset($entity->entityhelper_temp_id);
    }
    else {
      $this->wrappers[$entity_type][$entity_id]
        = entity_metadata_wrapper($entity_type, $entity);
    }
    return $this->wrappers[$entity_type][$entity_id];
  }

  /**
   * Gets the metadatawrapper of an entity.
   *
   * @param object $entity
   *
   * @return EntityMetadataWrapper
   *
   * @throws Exception
   */
  public function get($entity) {
    // Base type should be set by the entity_base_type module. However it cannot
    // set this property for newly created entities. In this case it should
    // either be set manually from code or the entityhelper_create() function
    // should be used to create the entity.
    if (empty($entity->base_type)) {
      throw new Exception(
        t('Entity base type not set. Set it in code or use "entityhelper_create()" for new entities.')
      );
    }
    $entity_type = $entity->base_type;
    $entity_id = entity_id($entity_type, $entity);
    if (!empty($entity_id)) {
      return $this->get_by_id($entity_type, $entity_id, $entity);
    }
    // This is a newly created entity, it has no id yet, let's create a
    // temporary id and store the wrapper using that.
    $temp_id = spl_object_hash($entity);
    $entity->entityhelper_temp_id = $temp_id;
    $this->wrappers[$entity_type]['temp'][$temp_id]
        = entity_metadata_wrapper($entity_type, $entity);
    return $this->wrappers[$entity_type]['temp'][$temp_id];
  }
}
