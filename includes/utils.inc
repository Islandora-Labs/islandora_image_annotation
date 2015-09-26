<?php

/**
 * @file
 * Utility functions for islandora image annotation module.
 */

/**
 * Checks our admin table for an allowed datastream.
 *
 * @param string $pid
 *   The pid of an object.
 *
 * @return string
 *   The DSID of the annotation.
 */
function islandora_image_annotation_get_dsid_from_model($pid) {
  $dsid = 'OBJ';
  $object = islandora_object_load($pid);
  $models = $object->models;
  if (($key = array_search('fedora-system:FedoraObject-3.0', $models)) !== FALSE) {
    unset($models[$key]);
  }
  $mapping = variable_get('islandora_image_annotation_mappings', array());
  foreach ($models as $model) {
    if (isset($mapping[$model])) {
      $tmp_dsid = $mapping[$model]['DSID'];
      if (isset($object[$tmp_dsid])) {
        $dsid = $tmp_dsid;
      }
    }
  }
  return $dsid;
}

/**
 * Retrieves content models from Fedora.
 *
 * @return array
 *   An array containing key value pairs of content model pids and titles.
 */
function islandora_image_annotation_get_content_models() {
  $query = <<<EOT
select ?object ?title from <#ri> where
  ?object <info:fedora/fedora-system:def/model#hasModel> <info:fedora/fedora-system:ContentModel-3.0> and
  ?object <fedora-model:label> ?title and
  (?object <fedora-view:disseminates> ?ds1 in <#ri> and ?ds1 <fedora-view:disseminationType> <info:fedora/*/DS-COMPOSITE-MODEL> in <#ri>)
  minus ?object <mulgara:is> <info:fedora/fedora-system:FedoraObject-3.0>
  minus ?object  <mulgara:is> <info:fedora/fedora-system:ContentModel-3.0>
  minus ?object  <mulgara:is> <info:fedora/fedora-system:ServiceDefinition-3.0>
  minus ?object  <mulgara:is> <info:fedora/fedora-system:ServiceDeployment-3.0>
  order by ?title
EOT;
  $results = islandora_get_tuque_connection()->repository->ri->itqlQuery($query, 'unlimited', '0');
  $ret = array();
  foreach ($results as $result) {
    $pid = $result['object']['value'];
    $ret[$pid] = $result['title']['value'];
  }
  return $ret;
}

/**
 * Retrieves datastream ids from the DS-COMPOSITE-MODEL.
 *
 * @param string $pid
 *   Pid of the object.
 *
 * @return array
 *   Array containing the DSIDs of datastreams.
 */
function islandora_image_annotation_get_dsids_from_dscomp($pid) {
  module_load_include('inc', 'islandora', 'includes/tuque');
  $fedora_object = islandora_object_load($pid);
  $datastream = $fedora_object['DS-COMPOSITE-MODEL'];
  $ds_comp_stream = $datastream->content;
  $xml = new SimpleXMLElement($ds_comp_stream);
  $output = array();
  foreach ($xml->dsTypeModel as $ds) {
    $mimes = array();
    foreach ($ds->form as $form) {
      $mimetype = (string) $form['MIME'];
      if (strstr($mimetype, 'image')) {
        $mimes[] = $mimetype;
      }
    }
    if (count($mimes) > 0) {
      $output[(string) $ds['ID']] = (string) $ds['ID'];
    }
  }
  return $output;
}

/**
 * Retrieves all taxonomies defined within Drupal.
 *
 * @return array
 *   A key value pair array containing the vid and taxonomy name.
 */
function islandora_image_annotation_get_taxonomies() {
  module_load_include('module', 'taxonomy', 'module');
  $taxonomies = taxonomy_get_vocabularies();
  $output = array(t('None') => 'none');
  if (!empty($taxonomies)) {
    foreach ($taxonomies as $tax) {
      $output[$tax->vid] = $tax->name;
    }
  }
  return $output;
}

/**
 * Gets the type values as supplied by solr.
 *
 * @return array
 *   An array of strings each representing a unique type.
 */
function islandora_image_annotation_get_types() {
  if (!module_exists('islandora_solr')) {
    return array();
  }
  $solr_field = variable_get('islandora_image_annotation_solr_field_type', 'RELS_EXT_isAnnotationType_literal_s');
  $data = array(
    'q' => format_string('!field:("info:fedora/!content_model")', array(
      '!field' => variable_get('islandora_solr_content_model_field', 'RELS_EXT_hasModel_uri_ms'),
      '!content_model' => 'islandora:OACCModel',
    )),
    'fl' => $solr_field,
    'start' => '0',
    'rows' => '10000',
    'wt' => 'json',
    'qt' => variable_get('islandora_solr_request_handler', 'standard'),
    'sort' => "$solr_field asc",
  );
  $solr = variable_get('islandora_solr_url', 'localhost:8080/solr');
  $solr_url = url($solr . '/select', array('query' => $data));
  $result_request = drupal_http_request($solr_url);
  // Test whether the reponse status code is 200.
  if ($result_request->code == 200) {
    $results = json_decode($result_request->data, TRUE);
    $types = array();
    if (isset($results['response']['docs'])) {
      foreach ($results['response']['docs'] as $doc) {
        $types[] = $doc[$solr_field];
      }
    }
    $types = array_unique($types);
    sort($types, SORT_STRING);
    return $types;
  }
  else {
    return array();
  }
}

/**
 * Helper function that lists all of the comments annotations.
 *
 * @param AbstractObject $object
 *   The object in which the annotations are targeted at.
 *
 * @return array
 *   An associative array where each key is an annotation PID.
 *   - pid: The PID for a annotation.
 *   - type: The type of the annotation.
 */
function islandora_image_annotation_get_annotations(AbstractObject $object) {
  $query = <<<QUERY
    PREFIX islandora-rels-ext: <http://islandora.ca/ontology/relsext#>
    PREFIX fedora-model: <info:fedora/fedora-system:def/model#>
    SELECT ?pid ?type from <#ri> WHERE {
      ?pid islandora-rels-ext:targets <info:fedora/{$object->id}> ;
        islandora-rels-ext:isAnnotationType ?type;
        fedora-model:state fedora-model:Active;
    }
    ORDER BY ASC(?type)
QUERY;
  $results = $object->repository->ri->sparqlQuery($query);
  $return = array();
  foreach ($results as $result) {
    $pid = $result['pid']['value'];
    $type = $result['type']['value'];
    $return[$pid] = array(
      'pid' => $pid,
      'type' => $type,
    );
  }
  return $return;
}

/**
 * Fetches the settings for all the widgets.
 *
 * @param AbstractObject $object
 *   The object to annotate.
 *
 * @return array
 *   The Drupal.settings for each Image Annotation Widget.
 */
function islandora_image_annotation_get_settings(AbstractObject $object) {
  $settings = &drupal_static(__FUNCTION__);
  if (!isset($settings[$object->id])) {
    $settings[$object->id] = array(
      'islandoraImageAnnotation' => array(
        'pid' => $object->id,
      ),
      'islandoraImageAnnotationDialog' => array(
        'pid' => $object->id,
        // Types is used in the autocomplete for the "Types" field.
        'types' => islandora_image_annotation_get_types(),
        'allowEntityLinking' => module_exists('islandora_entities') && variable_get('islandora_image_annotation_use_entity_tagging', FALSE),
        'useTitleVocabulary' => variable_get('islandora_image_annotation_use_title_vocabulary', FALSE),
        'canChooseColour' => !variable_get('islandora_image_annotation_choose_color', TRUE),
        'dialog' => array(
          'dialogClass' => 'islandora-image-annotation',
        ),
      ),
      'islandoraImageAnnotationList' => array(
        'pid' => $object->id,
        'editable' => user_access(ISLANDORA_IMAGE_ANNOTATION_CREATE),
        'annotations' => islandora_image_annotation_get_annotations($object),
      ),
    );
  }
  return $settings[$object->id];
}
