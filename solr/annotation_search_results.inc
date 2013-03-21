<?php

/**
 * @file
 * Class override for grid display in solr search results.
 */

/**
 * Extention of IslandoraSolrResults to create an alternative display type.
 *
 * This display is designed for the VDP project at UPEI and assumes
 * certain fields.
 */
class IslandoraSolrResultsAnno extends IslandoraSolrResults {

  /**
   * Renders the primary or secondary display profile based on the Solr results.
   *
   * This is the method most Islandora Solr display plugins would override.
   *
   * @param array $results
   *   The raw Solr results from IslandoraSolrQueryProcessor::solrResult
   *
   * @return markup
   *   Rendered Solr results
   *
   * @see IslandoraSolrResults::displayResults()
   */
  function printResults($results) {

    // Set variables.
    $results_array = array();
    $elements = array();

    // Total.
    $elements['solr_total'] = $results->response->numFound;

    // Start.
    $elements['solr_start'] = $results->response->start;

    // Get prepared search results.
    $docs = $this->prep_results($results);

    // Optionally limit results to values given.
    $limitResults = variable_get('islandora_solr_limit_result_fields', 0);
    // We will honor the limit results for display but we need some fields
    // like PID and content model, the content model so we can determine what
    // object to link to.
    $key_fields = array();

    // loop over results array
    foreach ($docs as $doc) {
      $rows = array();
      $row = 0; // keeping $row in there for now until the snippet issue is cleared out
      $snippet = NULL; // @TODO: how is this suposed to work if the $snippet always is set to NULL ?
      $this->add_key_field($key_fields, 'rels.hasModel', $doc);
      $this->add_key_field($key_fields, 'PARENT_pid', $doc);
      //if (isset($doc['rels.hasModel'])) {
      //  foreach ($doc['rels.hasModel'] as $model) {
       //   if ($model == 'info:fedora/islandora:OACCModel') {
       //     $doc['PID'] = $doc['PARENT_pid'];
       //   }
       // }
      //}
      // loop over individual results to process fields
      foreach ($doc as $field => $value) {
        if ($field == 'PID') {
          $rows[$field]['label'] = 'Object ID'; //we need the pid regardless, the label may get changed below
        }
        // check if this field should be included
        if ($limitResults && empty($this->resultFieldArray[$field])) {
          continue;
        }

        // check for field label substitutions
        $translated_field_name = isset($this->allSubsArray[$field]) ? $this->allSubsArray[$field] : $field;

        // add label
        $rows[$field]['label'] = $translated_field_name;

        // add class: render solr field name to string that can be used as class name
        $rows[$field]['class'] = drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $field));

        // split value if the result value is an array
        if (is_array($value)) {
          $value = implode(", ", $value);
        }
        // add value
        $rows[$field]['value'] = $value;

        $row++; // keeping $row in there for now until the snippet issue is cleared out
      }
      // @TODO: how is this suposed to work if the $snippet always is set to NULL ?
      if ($snippet) {
        $rows[$row][] = array(
          'data' => 'Full Text',
          'header' => TRUE,
        );
        $rows[$row][] = $snippet[0];
      }
      // append array to results array
      $results_array[] = $rows;
    }

    // return themed search results
    return theme('islandora_image_annotation_solr', array('results' => $results_array, 'elements' => $elements, 'key_fields' => $key_fields));
  }

  /**
   * key_fileds are feilds used for logic but not display
   * in the solr clienn users can specify what fields to return, if they don't
   * specify PID or cModel we can't use them in links or for logic.  We add them
   * to a key_fields array in case the user doesn't want them displayed.  These
   * probably should be single valued fields
   * @param array $key_fields
   * @param string $field
   * @param array $doc
   */
  function add_key_field(&$key_fields, $field, $doc) {
    if (isset($doc[$field])) {
      foreach ($doc[$field] as $key => $value) {
        $key_fields[$field][$doc['PID']] = $value;
      }
    }
  }

}

