<?php
/**
 * @file
 * User page callbacks for the Search pages module.
 */

/**
 * Displays a search page.
 *
 * @param string $id
 *   The search page's machine name.
 * @param string|null $keys
 *   The keys to search for.
 * @param array $overridden_options
 *   An associative array of page options that should be overridden for this
 *   search only.
 */
function search_api_page_view($id, $keys = NULL, $overridden_options = array()) {
  $page = search_api_page_load($id);
  if (!$page) {
    return MENU_NOT_FOUND;
  }

  // Merge in overridden options.
  if ($overridden_options && is_array($overridden_options)) {
    $page->original_options = $page->options;
    $page->options = $overridden_options + $page->options;
  }

  // Pre-process keys (unescape \ and /).
  if (isset($keys) && $keys !== '') {
    $max_length = 128;
    if (isset($page->options['max_length'])) {
      $max_length = $page->options['max_length'] > 0 ? $page->options['max_length'] : NULL;
    }
    if ($max_length && backdrop_strlen($keys) > $max_length) {
      $keys = backdrop_substr($keys, 0, $max_length);
    }
    $keys = explode("\\\\", $keys);
    $keys = str_replace("\\", "/", $keys);
    $keys = implode("\\", $keys);
  }
  else {
    $keys = NULL;
  }

  $ret = array(
    '#theme' => "search_api_page_full_page__{$page->machine_name}",
  );
  $ret['#contextual_links']['search_api_page'] = array(
    'admin/config/search/search_api/page',
    array($page->machine_name),
  );

  if (!isset($page->options['result_page_search_form']) || $page->options['result_page_search_form']) {
    $ret['#form'] = backdrop_get_form('search_api_page_search_form', $page, $keys);
  }

  // Do a search if we have keys, or our empty behavior and facets dictate.
  if ($keys || !empty($page->options['empty_behavior'])) {
    // Override per_page setting with GET parameter.
    $limit = $page->options['per_page'];
    if (!empty($page->options['get_per_page'])
        && !empty($_GET['per_page'])
        && ((int) $_GET['per_page']) > 0) {
      $limit = (int) $_GET['per_page'];
    }

    try {
      $results = search_api_page_search_execute($page, $keys, $limit);
    }
    catch (SearchApiException $e) {
      backdrop_set_message(t('An error occurred while executing the search. Please try again, or contact the site administrator if the problem persists.'), 'error');
      $link = l(t('search page'), $_GET['q'], array('query' => backdrop_get_query_parameters()));
      watchdog_exception('search_api_page', $e,
          '%type while executing a search: !message in %function (line %line of %file).',
          array(), WATCHDOG_ERROR, $link);
      return $ret;
    }

    if (empty($results)) {
      return $ret;
    }

    $ret['#results']['#theme'] = "search_api_page_results__{$page->machine_name}";
    $ret['#results']['#index'] = search_api_index_load($page->index_id);
    $ret['#results']['#view_mode'] = isset($page->options['view_mode']) ? $page->options['view_mode'] : 'search_api_page_result';
    $ret['#results']['#page'] = $page;

    // If spellcheck results were returned then add them to the render array.
    if (isset($results['search_api_spellcheck'])) {
      $ret['#results']['#spellcheck'] = array(
        '#theme' => 'search_api_spellcheck',
        '#spellcheck' => $results['search_api_spellcheck'],
      // Let the theme function know where the key is stored by passing its arg
      // number. We can work this out from the number of args in the page path.
        '#options' => array(
          'arg' => array(count(arg(NULL, $page->path))),
        ),
        '#prefix' => '<p class="search-api-spellcheck-suggestion">',
        '#suffix' => '</p>',
      );
    }

    $ret['#results']['#results'] = $results;
    $ret['#results']['#keys'] = check_plain($keys);
    // Add a clean-up function to reset page options to their original values.
    if ($overridden_options) {
      $ret['#results']['#post_render'] = array('search_api_page_reset_original_options');
    }

    // Load pager.
    if ($results['result count'] > $limit) {
      pager_default_initialize($results['result count'], $limit);
      $ret['#results']['#pager']['#theme'] = 'pager';
    }

    if (!empty($results['ignored'])) {
      backdrop_set_message(t('The following search keys are too short or too common and were therefore ignored: "@list".', array('@list' => implode(t('", "'), $results['ignored']))), 'warning');
    }
    if (!empty($results['warnings'])) {
      foreach ($results['warnings'] as $warning) {
        backdrop_set_message(check_plain($warning), 'warning');
      }
    }
  }

  return $ret;
}

/**
 * Executes a search.
 *
 * @param SearchApiPage $page
 *   The page for which a search should be executed.
 * @param string|null $keys
 *   The keywords to search for, or NULL for a search without keywords.
 * @param int $limit
 *   The maximum number of results to return.
 *
 * @return array|false
 *   FALSE if no keys were given, no facet filters are present and the page is
 *   configured to show no results in this case. Otherwise, the search results
 *   as returned by SearchApiQueryInterface::execute().
 *
 * @throws SearchApiException
 *   If an error occurred during the search.
 */
function search_api_page_search_execute(SearchApiPage $page, $keys = NULL, $limit = 10) {
  $offset = pager_find_page() * $limit;
  $options = array(
    'search id' => 'search_api_page:' . $page->path,
    'search_api_page id' => $page->machine_name,
    'parse mode' => $page->options['mode'],
  );

  if (!empty($page->options['search_api_spellcheck'])) {
    $options['search_api_spellcheck'] = TRUE;
  }

  $query = search_api_query($page->index_id, $options)
    ->keys($keys)
    ->range($offset, $limit);
  if (!empty($page->options['fields'])) {
    $query->fields($page->options['fields']);
  }
  if (!empty($page->options['language_filter'])) {
    $languages = array_unique(array_map('_search_api_page_map_languages', $page->options['language_filter']));
    if (count($languages) == 1) {
      $query->condition('search_api_language', reset($languages));
    }
    else {
      $filter = $query->createFilter('OR');
      foreach ($languages as $lang) {
        $filter->condition('search_api_language', $lang);
      }
      $query->filter($filter);
    }
  }
  $results = $query->execute();
  if (!$keys && $page->options['empty_behavior'] === 'blocks' && !search_api_page_query_has_facets($query)) {
    return FALSE;
  }
  return $results;
}

/**
 * Determines whether an executed query had any facet filters set.
 *
 * @param SearchApiQueryInterface $query
 *   The query in question.
 *
 * @return bool
 *   TRUE if there are filters with a "facet:*" tag present in the query object,
 *   FALSE otherwise.
 */
function search_api_page_query_has_facets(SearchApiQueryInterface $query) {
  // Check that the search server supports facets.
  $index = $query->getIndex();
  if (module_exists('facetapi') && $index->server()->supportsFeature('search_api_facets')) {
    // Load the search index's adapter plugin and process the facets.
    $adapter = facetapi_adapter_load('search_api@' . $index->machine_name);
    $adapter->processFacets();

    // Check if there are any active facets and return a boolean accordingly.
    return (bool) $adapter->getAllActiveItems();
  }

  // If the search server doesn't support facets, there aren't any facet filters
  // set by definition.
  return FALSE;
}

/**
 * Maps some special languages to their correct value.
 *
 * Callback for array_map() in search_api_page_search_execute().
 *
 * @param string $lang
 *   "current", "default" or a valid ISO 639 language code.
 *
 * @return string
 *   The ISO 639 language code of the language represented by $lang.
 */
function _search_api_page_map_languages($lang) {
  if ($lang == 'current') {
    return $GLOBALS['language_content']->language;
  }
  elseif ($lang == 'default') {
    return language_default('language');
  }
  return $lang;
}

/**
 * Resets the page's options, if they were overridden, to their original values.
 *
 * Used as a "#post_render" callback for the search page results in
 * search_api_page_view().
 *
 * @param string $children
 *   The rendered output of the element.
 * @param array $element
 *   The rendered element.
 *
 * @return array
 *   The processed rendered output of the element.
 */
function search_api_page_reset_original_options($children, array $element) {
  $page = $element['#page'];
  if (!empty($page->original_options)) {
    $page->options = $page->original_options;
    unset($page->original_options);
  }
  return $children;
}
