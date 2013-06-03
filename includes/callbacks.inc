<?php

/**
 * @file
 * Functions that implement ajax callbacks.
 */

/**
 * Ajax callback for Shared Canvas.
 *
 * Supplies the basic data, including manifest URI.
 * Returns JSON to be echoed to the client.
 *
 * @param string $pid
 *   The PID of a object.
 */
function islandora_image_annotation_canvas_setup($pid) {
  module_load_include('inc', 'islandora_image_annotation', 'includes/utils');
  drupal_json_output(islandora_image_annotation_canvas_init($pid));
}


/**
 * Creates annotation objects from html supplied by Shared Canvas.
 *
 * @param string $pid
 *   The pid of a object.
 */
function islandora_image_annotation_add_annotation($pid) {
  global $user;
  module_load_include('inc', 'islandora', 'includes/utilities');
  $data = $_POST['data'];
  // todo: In the future, the following fix could be eliminated
  // by adding the 'stroke width' option to the annotation jquery dialog
  // box (as a combo box selection).
  $stroke_width;
  if (isset($_POST['strokeWidth'])) {
    $stroke_width = $_POST['strokeWidth'];
  }
  else {
    $stroke_width = 0;
  }

  $annotation_xml = islandora_image_annotation_shared_transform_annotation_list(urldecode($_POST['data']), 'html');

  $color = $_POST['color'];
  $type = $_POST['type'];
  if (!$type) {
    $type = 'unclassified';
  }
  $xml = simplexml_load_string($annotation_xml);
  $labels = $xml->xpath('//dc:title');
  $annotations = $xml->xpath('//rdf:type[@rdf:resource="http://www.w3.org/ns/openannotation/core/Annotation"]');
  $parents = $annotations[0]->xpath("parent::*");
  $parent = $parents[0];
  foreach ($parent->attributes('rdf', TRUE) as $key => $value) {
    $urn = (string) $value;
  }
  $namespace = substr($pid, 0, strpos($pid, ":"));
  $relationship = array();
  $relationship['relationship'] = 'isMemberOf';
  $relationship['pid'] = $pid;
  $item = islandora_prepare_new_object($namespace,
    $labels[0],
    array(),
    array('islandora:OACCModel'),
    array($relationship));

  $item->label = (string) $labels[0];

  $item->relationships->add(ISLANDORA_RELS_EXT_URI, 'targetedBy', $user->name, TRUE);
  $item->relationships->add(ISLANDORA_RELS_EXT_URI, 'targets', $pid);
  if ($color) {
    $item->relationships->add(ISLANDORA_RELS_EXT_URI, 'hasColor', $color, TRUE);
  }
  $item->relationships->add(FEDORA_RELS_EXT_URI, 'isAnnotationOf', $pid);
  $item->relationships->add('http://islandora.ca/ontology/relsext#', 'isAnnotationType', $type, TRUE);
  $item->relationships->add(ISLANDORA_RELS_EXT_URI, 'hasURN', $urn, TRUE);
  $item->relationships->add(ISLANDORA_RELS_EXT_URI, 'strokeWidth', $stroke_width, TRUE);
  $new_ds = $item->constructDatastream('OAC', 'X');
  $new_ds->label = 'OAC';
  $new_ds->mimetype = 'text/xml';
  $new_ds->content = $annotation_xml;
  $item->ingestDatastream($new_ds);
  islandora_add_object($item);
  echo $item->id;
}

/**
 * Transforms input from SharedCanvas to AnnotationList.
 *
 * @param string $xml_in
 *   The XML we are transforming.
 *
 * @return string
 *   The transformed XML.
 */
function islandora_image_annotation_shared_transform_annotation_list($xml_in, $input_type) {

  if ($xml_in == NULL || strlen($xml_in) < 5) {
    return " ";
  }

  $path = drupal_get_path('module', 'islandora_image_annotation') . '/xslt/';
  $xslts = array(
    'html' => $path . 'html_to_annotation_list.xsl',
    'rdf' => $path . 'annotation_list_to_html.xsl',
  );
  try {
    $proc = new XsltProcessor();
  }
  catch (Exception $e) {
    drupal_set_message(t('@e', array('@e' => check_plain($e->getMessage()))), 'error');
    return " ";
  }

  $xsl = new DomDocument();
  $xsl->load($xslts[$input_type]);
  $input = new DomDocument();
  $input->loadXML(trim($xml_in));
  $xsl = $proc->importStylesheet($xsl);
  $newdom = $proc->transformToDoc($input);
  $return_xml = $newdom->saveXML();

  return $return_xml;
}

/**
 * Gets individual annotation pids from RI.
 *
 * Called with islandora/anno/get_urns.
 *
 * @param array $pid
 *   An array of pids.
 */
function islandora_image_annotation_shared_get_annotation_pids($pid) {

  $query = <<<QUERY
    PREFIX io: <http://islandora.ca/ontology/relsext#>
    PREFIX fm: <info:fedora/fedora-system:def/model#>

    SELECT ?id ?type ?urn ?strokeWidth ?color  from <#ri>
    WHERE {
      ?id io:targets <info:fedora/$pid> ;
      io:hasURN ?urn;
      io:isAnnotationType ?type;
      fm:state fm:Active ;

      OPTIONAL{
        ?id io:strokeWidth ?strokeWidth;
      }
      OPTIONAL{
        ?id io:hasColor ?color
      }
    }
    ORDER BY ASC(?type)
QUERY;

  $connection = islandora_get_tuque_connection();
  $objects = $connection->repository->ri->sparqlQuery($query);
  $pids = array();
  foreach ($objects as $object) {
    $itemdata = array(
      'id' => $object['id']['value'],
      'type' => $object['type']['value'],
      'color' => $object['color']['value'],
      'strokeWidth' => $object['strokeWidth']['value'],
      'urn' => $object['urn']['value'],
    );

    $pids['pids'][] = $itemdata;
    $pids['types'][] = $object['type']['value'];
  }
  if (!empty($pids)) {
    $pids['types'] = array_values(array_unique($pids['types']));
  }
  $json = json_encode($pids);
  echo $json;
}

/**
 * Retrieves details of individual annotation from datastream.
 *
 * @param string $pid
 *   A pid of an object.
 *
 * @return XML
 *   The XML content of the OAC datastream.
 */
function islandora_image_annotation_get_annotation_data($pid) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $item = islandora_object_load($pid);
  if (!isset($item)) {
    return;
  }
  $annotation_xml = $item['OAC']->content;
  return $annotation_xml;
}

/**
 * Retrieves transforms annotation data for the editor.
 *
 * Called with /islandora/anno/get_annotation/.
 *
 * @param string $pid
 *   The pid of an object
 *
 * @param urn $urn
 *   Urn.
 */
function islandora_image_annotation_shared_get_annotation($pid, $urn = NULL) {
  if (isset($urn)) {
    $pid = islandora_image_annotation_canvas_get_pid_from_urn($urn);
  }
  $xml = islandora_image_annotation_get_annotation_data($pid);
  $results = islandora_image_annotation_shared_transform_annotation_list($xml, 'rdf');
  // Remove the doc type declarations.
  $results = preg_replace('/<\?xml.*?\?>/', '', $results);
  $results = preg_replace('/<!DOCTYPE.*?>/', '', $results);

  echo $results;
}

/**
 * Deletes an annotation object.
 *
 * @param string $pid
 *   The pid of an object.
 *
 * @param urn $urn
 *   Urn.
 */
function islandora_delete_annotation($pid, $urn) {
  $pid_to_delete = islandora_image_annotation_canvas_get_pid_from_urn($urn);
  $item = islandora_object_load($pid_to_delete);
  islandora_delete_object($item);
}

/**
 * Persists new annotations.
 *
 * @global url $base_url
 *
 * @param string $pid
 *   The pid of an object.
 *
 * @param RDF $rdf
 *   An IslandoraRdfBuilder object.
 */
function islandora_save_updated_annotations($pid, $rdf) {
  $item = islandora_object_load($pid);
  $updated_annotation_list = $rdf->printRDF();
  $item['AnnotationList']->content = $updated_annotation_list;
}

/**
 * Gets the pid of an annotation object identified by a know urn.
 *
 * @param URN $urn
 *   A urn.
 *
 * @return string
 *   The pid retrived from an urn.
 */
function islandora_image_annotation_canvas_get_pid_from_urn($urn) {
  module_load_include('inc', 'islandora', 'includes/tuque');
  $connection = islandora_get_tuque_connection();
  $query = "select \$id  from <#ri> where \$id <http://islandora.ca/ontology/relsext#hasURN> \$urn
and \$urn <mulgara:is> 'urn:uuid:$urn'";

  $objects = $connection->repository->ri->itqlQuery($query);
  $pid = $objects[0]['id']['value'];

  return $pid;
}

/**
 * Updates existing annotation.
 *
 * Called by Shared Canvas with ajax
 */
function islandora_image_annotation_canvas_update_annotation() {
  $changed = FALSE;
  // Retrieve variables from POST.
  $urn = urldecode($_POST['urn']);
  $title = urldecode($_POST['title']);
  $content = urldecode($_POST['content']);
  $type = urldecode($_POST['annoType']);
  $new_color = urldecode($_POST['color']);
  $new_stroke_width = urldecode($_POST['strokeWidth']);
  $pid = islandora_image_annotation_canvas_get_pid_from_urn($urn);
  $fedora_object = islandora_object_load($pid);

  $annotation = $fedora_object['OAC']->content;
  $xml = simplexml_load_string($annotation);

  // Identify URN of body to retrieve chars node to hold new annotation content.
  $body_urn_array = $xml->xpath("//oa:hasBody");
  $body_urn = (string) $body_urn_array[0]->attributes('rdf', 'about');
  $content_node = $xml->xpath("//rdf:Description[@rdf:about = '$body_urn']//cnt:chars");

  $constrained_urn_array = $xml->xpath("//oa:constrainedBy");
  $constrained_by_urn = $body_urn = (string) $constrained_urn_array[0]->attributes('rdf', 'resource');
  $target_node = $xml->xpath("//rdf:Description[@rdf:about = '$constrained_by_urn']//cnt:chars");
  $target_value = (string) $target_node[0];

  // If color picking is allowed, update here.
  if (!$optimized = variable_get('islandora_annotation_optimized', TRUE)) {
    $rels = $fedora_object->relationships->get(ISLANDORA_RELS_EXT_URI, 'hasColor');
    $old_color = (!empty($rels)) ? $rels[0]['object']['value'] : '';
    if (!empty($new_color) && $old_color != $new_color) {
      $fedora_object->relationships->remove(ISLANDORA_RELS_EXT_URI, 'hasColor', NULL, TRUE);
      $fedora_object->relationships->add(ISLANDORA_RELS_EXT_URI, 'hasColor', $new_color, TRUE);
      $changed = TRUE;
    }
  }

  $rels = $fedora_object->relationships->get(ISLANDORA_RELS_EXT_URI, 'strokeWidth');
  $old_stroke_width = (!empty($rels)) ? $rels[0]['object']['value'] : '';
  if ($old_stroke_width != $new_stroke_width) {
    $fedora_object->relationships->remove(ISLANDORA_RELS_EXT_URI, 'strokeWidth', NULL, TRUE);
    $fedora_object->relationships->add(ISLANDORA_RELS_EXT_URI, 'strokeWidth', $new_stroke_width, TRUE);
    $svg_xml = simplexml_load_string($target_value);
    $svg_xml['stroke-width'] = $new_stroke_width . '%';
    $target_node[0]->{0} = str_replace('<?xml version="1.0"?>', '', $svg_xml->asXML());
    $changed = TRUE;
  }

  // Update the RELS if the category has changed.
  $anno_type = $fedora_object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isAnnotationType');
  if (!empty($anno_type) && array_key_exists('object', $anno_type) && array_key_exists('value', $anno_type['object'])) {
    $anno_type_value = $anno_type['object']['value'];
    if ($type != $anno_type_value) {
      $fedora_object->relationships->remove(ISLANDORA_RELS_EXT_URI, 'isAnnotationType', NULL, TRUE);
      $fedora_object->relationships->add(ISLANDORA_RELS_EXT_URI, 'isAnnotationType', $type, TRUE);
    }
  }

  // Identify Title element and replace content.
  $title_node = $xml->xpath("//rdf:Description[@rdf:about = 'urn:uuid:$urn']/dc:title");
  if ($title_node[0][0] != $title) {
    $title_node[0][0] = $title;
    $changed = TRUE;
  }

  $type_node = $xml->xpath("//rdf:Description[@rdf:about = 'urn:uuid:$urn']/dc:type");
  if ($type_node[0][0] != $type) {
    $type_node[0][0] = $type;
    $changed = TRUE;
  }

  if ((string) $content_node[0][0] != $content) {
    $content_node[0][0] = $content;
    $changed = TRUE;
  }

  // Persist updated annotation.
  if ($changed) {
    $test_xml = $xml->asXML();
    $fedora_object['OAC']->content = $xml->asXML();
  }
}

/**
 * Get type terms via solr.
 *
 * @return array
 *   Keys of the terms returned from solr.
 */
function islandora_image_annotation_get_type_terms() {
  $type_field = variable_get('annotation_type_search_field', 'anno_category_s');
  $query = format_string('!field:("!content_model" OR "info:fedora/!content_model")', array(
    '!field' => variable_get('islandora_solr_content_model_field', 'RELS_EXT_hasModel_uri_ms'),
    '!content_model' => 'islandora:OACCModel',
  ));

  $params = array(
    'fl' => "$type_field, PID",
    'start' => '0',
    'rows' => '10000',
    'qt' => variable_get('islandora_solr_request_handler', ''),
  );

  $results = islandora_image_annotation_query_solr($query, $params);
  $terms = array();
  if (isset($results['solr_doc'])) {
    foreach ($results['solr_doc'] as $doc) {
      if (array_key_exists($type_field, $doc)) {
        $terms[$doc[$type_field][0]] = 1;
      }
    }
  }

  return array_keys($terms);
}

/**
 * Return each solr doc found.
 *
 * @type string
 *   The type to search for.
 * @return string
 *   JSON string.
 */
function islandora_image_annotation_get_title_terms($type) {
  $title_field = variable_get('annotation_title_search_field', 'mads_titles_sv');
  $solr = variable_get('islandora_solr_url', 'localhost:8080/solr');
  $data = array(
    'q' => format_string('!field:"!value"', array(
      '!field' => variable_get('annotation_genre_field', 'mads_genre_it'),
      '!value' => $type,
    )),
    'fl' => "$title_field, PID",
    'start' => '0',
    'rows' => '10000',
    'wt' => 'json',
    'qt' => variable_get('islandora_solr_request_handler', 'standard'),
    'sort' => "$title_field asc",
  );
  $solr_url = url($solr . '/select', array('query' => $data));
  $result_request = drupal_http_request($solr_url);
  $results = json_decode($result_request->data, TRUE);
  $docs = $results['response']['docs'];
  echo json_encode($docs);
  exit();
}


/**
 * Reads MADS data stream and returns content to populate an annotation.
 *
 * This is specific to UPEI VDP project.
 *
 * @TODO: call module invoke all here and move this function out to be
 * implemented by a submodule, this way other modules could control the
 * population of an anno body * from a controlled vocabulary.  As is this
 * function has no real affect on anything except instances where both the Title
 * and Type are determined from a controlled vocabulary.
 *
 * @param string $pid
 *   the identifier of the object to read the datatstream from
 */
function islandora_image_annotation_get_mads_as_json($pid) {
  $object = islandora_object_load($pid);
  if (!isset($object) || !isset($object['MADS'])) {
    echo 'error loading MADS datastream';
    exit();
  }
  $mads = $object['MADS']->content;
  $doc = new DomDocument();
  $doc->loadXML($mads);
  $elements = $doc->getElementsByTagName('related');
  $arr = array();
  $str = "";
  foreach ($elements as $element) {
    $attr = $element->getAttribute('otherType');
    if (!empty($attr)) {
      $str = ucfirst(trim($attr));
      $str = str_replace('_', ' ', $str);
      $arr[$str] = trim($element->nodeValue);
    }
  }
  echo json_encode($arr);
  exit();
}

/**
 * Helper function to query solr.
 *
 * @param string $query
 *   A query string.
 *
 * @param array $params
 *   An associate array containing solr params.
 */
function islandora_image_annotation_query_solr($query, $params) {
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = $query;
  foreach ($params as $field => $value) {
    if ($field == 'start') {
      $query_processor->solrStart = $value;
    }
    elseif ($field == 'rows') {
      $query_processor->solrLimit = $value;
    }
    else {
      $query_processor->solrParams[$field] = $value;
    }
  }
  $query_processor->executeQuery(FALSE);
  $results = $query_processor->islandoraSolrResult;
  return $results;
}
