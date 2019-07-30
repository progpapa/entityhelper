<?php

/**
 * Simple service container for the entityhelper module.
 *
 * @property EntityHelperGet $EntityHelperGet
 * @property EntityHelperSet $EntityHelperSet
 * @property EntityHelperWrapper $EntityHelperWrapper
 */
class EntityHelperContainer extends AbstractContainer {

  /**
   * @return EntityHelperGet
   */
  public function create_EntityHelperGet() {
    return new EntityHelperGet();
  }

  /**
   * @return EntityHelperSet
   */
  public function create_EntityHelperSet() {
    return new EntityHelperSet();
  }

  /**
   * @return EntityHelperWrapper
   */
  public function create_EntityHelperWrapper() {
    return new EntityHelperWrapper();
  }
}