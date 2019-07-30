<?php

/**
 * @file
 * Configuration settings on how field values should be get or set.
 */
class EntityHelperField {

  /**
   * Global configuration, applies to all fields.
   * @var array
   */
  private $global = array();

  /**
   * Field level configuration, overrides global config.
   * @var array
   */
  private $fields = array();

  public function __construct() {
    $this->global = array();
    $this->fields = array();
  }

}
