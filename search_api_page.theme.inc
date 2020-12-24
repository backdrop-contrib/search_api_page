<?php

/**
 * Implements theme for rendering search-performance
 */
function theme_search_api_page_search_performance($variables) {
  $element = array_shift($variables);

  return $element['#markup'];
}

/**
 * Preprocess variables for search-api-page-results.tpl.php.
 *
 * @param array $variables
 *   An associative array containing:
 *   - index: The index this search was executed on.
 *   - results: An array of search results, as returned by
 *     SearchApiQueryInterface::execute().
 *   - view_mode: The view mode to use for results. Either one of the searched
 *     entity's view modes (if applicable), or "search_api_page_result" to use
 *     the module's builtin theme function to render results.
 *   - keys: The keywords of the executed search.
 *   - page: The search page whose search results are being displayed.
 *
 * @see search-api-page-results.tpl.php
 */
function template_preprocess_search_api_page_results(array &$variables) {
  $page = $variables['page'];
  $results = $variables['results'];
  $results += array(
    'results' => array(),
  );
  $variables += array(
    'items' => array(),
  );
  if ($results['results'] && empty($variables['items'])) {
    $variables['items'] = $variables['index']->loadItems(array_keys($results['results']));
  }
  $variables['result_count'] = $results['result count'];
  $variables['sec'] = 0;
  if (isset($results['performance']['complete'])) {
    $variables['sec'] = round($results['performance']['complete'], 3);
  }
  $variables['search_performance'] = array(
    '#theme' => "search_api_page_search_performance__{$page->machine_name}",
    '#markup' => format_plural(
      $results['result count'],
      'The search found 1 result in @sec seconds.',
      'The search found @count results in @sec seconds.',
      array('@sec' => $variables['sec'])
    ),
    '#prefix' => '<p class="search-performance">',
    '#suffix' => '</p>',
  );

  // Make the help text depend on the parse mode used.
  switch ($page->options['mode']) {
    case 'direct':
      if ($variables['index']->server()->class == 'search_api_solr') {
        $variables['no_results_help'] = t('<ul>
<li>Check if your spelling is correct.</li>
<li>Remove quotes around phrases to search for each word individually. <em>bike shed</em> will often show more results than <em>&quot;bike shed&quot;</em>.</li>
<li>Consider loosening your query with <em>OR</em>. <em>bike OR shed</em> will often show more results than <em>bike shed</em>.</li>
</ul>');
      }
      else {
        $variables['no_results_help'] = t('<ul>
<li>Check if your spelling is correct.</li>
<li>Use fewer keywords to increase the number of results.</li>
</ul>');
      }
      break;
    case 'single':
      $variables['no_results_help'] = t('<ul>
<li>Check if your spelling is correct.</li>
<li>Use fewer keywords to increase the number of results.</li>
</ul>');
      break;
    case 'terms':
      $variables['no_results_help'] = t('<ul>
<li>Check if your spelling is correct.</li>
<li>Remove quotes around phrases to search for each word individually. <em>bike shed</em> will often show more results than <em>&quot;bike shed&quot;</em>.</li>
<li>Use fewer keywords to increase the number of results.</li>
</ul>');
      break;
  }

  // Prepare CSS classes.
  $classes_array = array(
    'search-api-page',
    'search-api-page-' . backdrop_clean_css_identifier($page->machine_name),
    'view-mode-' . backdrop_clean_css_identifier($variables['view_mode']),
  );
	$variables['classes'] = $classes_array;

  // Distinguish between the native search_api_page_result "view mode" and
  // proper entity view modes (e.g., Teaser, Full content, RSS and so forth).
  $variables['search_results'] = array();
  if ($variables['view_mode'] != 'search_api_page_result') {
    // Check if we have any entities to show and, if yes, show them.
    if (!empty($variables['items'])) {
      $variables['search_results'] = entity_plus_view($variables['index']->getEntityType(), $variables['items'], $variables['view_mode']);
    }
  }
  else {
    foreach ($results['results'] as $item) {
      if(!empty($variables['items'][$item['id']])) {
        $variables['search_results'][] = array(
          '#theme' => "search_api_page_result__{$page->machine_name}",
          '#index' => $variables['index'],
          '#result' => $item,
          '#item' => $variables['items'][$item['id']],
        );
      }
    }
  }
  // Load CSS.
  $base_path = backdrop_get_path('module', 'search_api_page') . '/';
  backdrop_add_css($base_path . 'css/search_api_page.css');
}

/**
 * Process variables for search-result.tpl.php.
 *
 * The $variables array contains the following arguments:
 * - $result
 *
 * @see search_api_page-result.tpl.php
 */
function template_preprocess_search_api_page_result(&$variables) {
  $index = $variables['index'];
  $variables['id'] = $variables['result']['id'];
  $item = $variables['item'];

  $wrapper = $index->entityWrapper($item, FALSE);

  $variables['url'] = $index->datasource()->getItemUrl($item);
  $variables['title'] = $index->datasource()->getItemLabel($item);

  if (!empty($variables['result']['excerpt'])) {
    $variables['excerpt'] = $variables['result']['excerpt'];
    $text = $variables['result']['excerpt'];
  }
  else {
    $fields = $index->options['fields'];
    $fields = array_intersect_key($fields, backdrop_map_assoc($index->getFulltextFields()));
    $fields = search_api_extract_fields($wrapper, $fields);
    $text = '';
    $length = 0;
    foreach ($fields as $field_name => $field) {
      if (search_api_is_list_type($field['type']) || !isset($field['value'])) {
        continue;
      }
      $val_length = backdrop_strlen($field['value']);
      if ($val_length > $length) {
        $text = $field['value'];
        $length = $val_length;

        $format = NULL;
        if (($pos = strrpos($field_name, ':')) && substr($field_name, $pos + 1) == 'value') {
          $tmp = $wrapper;
          try {
            foreach (explode(':', substr($field_name, 0, $pos)) as $part) {
              if (!isset($tmp->$part)) {
                $tmp = NULL;
              }
              $tmp = $tmp->$part;
            }
          }
          catch (EntityMetadataWrapperException $e) {
            $tmp = NULL;
          }
          if ($tmp && $tmp->type() == 'text_formatted' && isset($tmp->format)) {
            $format = $tmp->format->value();
          }
        }
      }
    }
    if ($text && function_exists('text_summary')) {
      $text = text_summary($text, $format);
    }
  }

  // Check for existence. User search does not include snippets.
  $variables['snippet'] = isset($text) ? $text : '';

  // Meta information
  global $language;
  if (isset($item->language) && $item->language != $language->language && $item->language != LANGUAGE_NONE) {
    $variables['title_attributes_array']['xml:lang'] = $item->language;
    $variables['content_attributes_array']['xml:lang'] = $item->language;
  }

  $info = array();
  if (!empty($item->name)) {
    $info['user'] = $item->name;
  }
  if (!empty($item->created)) {
    $info['date'] = format_date($item->created, 'short');
  }
  // Provide separated and grouped meta information..
  $variables['info_split'] = $info;
  $variables['info'] = implode(' - ', $info);
}
