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
     *
     * @var integer
     */
    public $id;

	/**
     * The machine name of the index this search page uses.
     *
     * @var string
     */
    public $index_id;

    /**
     * The index this search page uses.
     *
     * @var object
     *
     * @return SearchApiIndex
     */
    public $index;

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
     * Implements EntityInterface::id().
     */
    public function id() {
      return $this->id;
    }

    /**
    *  The machine name of the index this search page uses
    */
    public function indexID() {
        return $this->index_id;
    }

    /**
     * Implements EntityInterface::entityType().
     */
    public function entityType() {
      return 'search_api_page';
    }

    /**
     * Implements EntityInterface::bundle().
     */
    public function bundle() {}

    /**
     * Implements EntityInterface::label().
     */
    public function label() {
        return $this->name;
    }

    /**
     * Overrides Entity\Entity::uri().
     */
    public function uri() {
        return array(
          'path' => $this->path,
        );
    }

    /**
    * @return SearchApiIndex|false
    *
    * A completely loaded index object, or FALSE if no such index exists.
    */
    public function getIndex() {
      return search_api_index_load($this->index_id);
    }

    /**
     * Changes a page's settings.
     *
     * @param $id
     *   The edited page's ID.
     * @param array $fields
     *   The new field values to set.
     *
     * @return
     *   1 if fields were changed, 0 if the fields already had the desired values.
     */
    public function edit(array $fields) {
      $changeable = array(
          'name' => 1,
          'description' => 1,
          'path' => 1,
          'options' => 1,
          'enabled' => 1
      );
      foreach ($fields as $field => $value) {
        if (isset($changeable[$field]) || $value === $this->$field) {
          $this->$field = $value;
          $new_values = TRUE;
        }
      }
      // If there are no new values, just return 0.
      if (empty($new_values)) {
        return 0;
      }
      $this->save();
      return 1;
    }

}
