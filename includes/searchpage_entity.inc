<?php

/**
 * @file
 * Contains SearchApiPage.
 */

/**
 * Class representing a search page.
 */
class SearchApiPage extends Entity {

	/**
     * The primary identifier for a search page.
     * Immutable.
     *
     * @var integer
     */
    public $id;

	/**
     * The machine name of the index this search page uses.
     * Immutable
     *
     * @var string
     */
    public $index_id;

	/**
     * The path at which this search page can be viewed.
     *
     * @var string
     */
    public $path;

	/**
     * The displayed name for a search page.
     *
     * @var string
     */
    public $name;

	/**
     * The internally used machine name for a search page.
     *
     * @var string
     */
    public $machine_name;

	/**
     * The displayed description for a search page.
     *
     * @var string
     */
    public $description;

	/**
     * The options used to configure the search page.
     *
     * @var array
     */
    public $options = array();

	/**
     * A flag indicating whether the search page is enabled.
     *
     * @var integer
     */
    public $enabled = 1;

    /**
     * Constructor as a helper to the parent constructor.
     */
    public function __construct(array $values = array(), $entity_type = 'search_api_page') {
      parent::__construct($values, $entity_type);
    }

    /**
     * Checks if the entity has a certain exportable status.
     *
     * @param $status
     *   A status constant, i.e. one of ENTITY_CUSTOM, ENTITY_IN_CODE,
     *   ENTITY_OVERRIDDEN or ENTITY_FIXED.
     *
     * @return bool
     *   For exportable entities TRUE if the entity has the status, else FALSE.
     *   In case the entity is not exportable, NULL is returned.
     *
     * @see entity_has_status()
     */
    public function hasStatus($status) {
      if (!empty($this->entityInfo['exportable'])) {
        return isset($this->{$this->statusKey}) && ($this->{$this->statusKey} & $status) == $status;
      }
    }

    /**
     * Implements EntityInterface::id().
     */
    public function id() {
      return $this->id;
    }

    /**
     * Implements EntityInterface::entityType().
     */
    public function entityType() {
      return 'search_api_page';
    }

    /**
     * Implements EntityInterface::label().
     */
    public function label() {}

    /**
     * Overrides Entity\Entity::uri().
     */
    public function uri() {
      return array();
    }






  }

  #[id] => serial
  #[index_id] => varchar
  #[path] => varchar
  #[name] => varchar
  #[machine_name] => varchar
  #[description] => text
  #[options] => text
  #[enabled] => int
  #[status] => int
  #[module] => varchar
