<?php

/**
 * @file
 * Utility functions for islandora image annotation module.
 */

/**
 * Retrieves the first found datastream from our default list from the object.
 *
 * @param string $pid
 *   The pid of the object.
 *
 * @return Datastream
 *   The datastream retrieved.
 */
function islandora_image_annotation_get_dsid($pid) {
  $object = islandora_object_load($pid);
  $dsid = NULL;
  $datastreams = variable_get('islandora_image_annotation_datastreams', 'OBJ JPG JPEG IMAGE');
  $datastreams = explode(' ', $datastreams);
  foreach ($datastreams as $datastream) {
    if (isset($object["$datastream"])) {
      $dsid = $datastream;
      break;
    }
  }
  return $dsid;
}

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
  $item = islandora_object_load($pid);
  $models = $item->models;
  if (($key = array_search('fedora-system:FedoraObject-3.0', $models)) !== FALSE) {
    unset($models[$key]);
  }
  $models_to_dsids = variable_get('islandora_annotation_mappings');
  foreach ($models as $model) {
    if (isset($models_to_dsids[$model])) {
      $tmp_dsid = $models_to_dsids[$model]['DSID'];
      if (isset($item[$tmp_dsid])) {
        $dsid = $tmp_dsid;
      }
    }
  }
  return $dsid;
}

/**
 * Shared canvas setup utilitiy.
 * 
 * Ajax callback for Shared Canvas, shared functionality in
 * islandora_image_annotation and islandora_critical_edition.
 * Supplies the basic data, including manifest URI. Returns json
 * to be echoed to the client.
 *
 * @param string $pid
 *   The Fedora object id to supply the data for.
 */
function islandora_image_annotation_canvas_init($pid) {

  module_load_include('inc', 'islandora_image_annotation', 'includes/build_streams');
  module_load_include('inc', 'islandora_image_annotation', 'includes/callbacks');
  module_load_include('inc', 'islandora_paged_content', 'includes/utilities');
  module_load_include('inc', 'islandora_image_annotation', 'includes/rdf_builder');
  module_load_include('module', 'taxonomy', 'module');

  global $base_path;
  global $user;

  $item = islandora_object_load($pid);
  $rels = $item->relationships->get();

  foreach ($rels as $relationship) {
    if ($relationship['predicate']['value'] == 'hasModel') {
      $cmodel = $relationship['object']['value'];
    }
    if ($relationship['predicate']['value'] == 'isEditionOf') {
      $page_object = islandora_object_load($relationship['object']['value']);
    }
  }

  $stroke_width = variable_get('annotation_stroke_width', '1%');
  $mappings = variable_get('islandora_annotation_mappings', array());
  $optimized = variable_get('islandora_annotation_optimized', TRUE);
  $dropdown = variable_get('islandora_annotation_enforce_taxonomy', TRUE);

  $vid = isset($mappings[$cmodel]['TAX']) ? $mappings[$cmodel]['TAX'] : NULL;

  $dsid = $mappings[$cmodel]['DSID'];

  if ($dropdown) {
    $terms = taxonomy_get_tree($vid, 0, 1);
    $categories = array();
    foreach ($terms as $term) {
      $categories[] = $term->name;
    }
  }
  else {
    $categories = islandora_image_annotation_get_type_terms();
  }

  $image_url = url("islandora/object/$pid/datastream/$dsid");
  $pages = array();
  $results = array();
  $pid_final;
  if (isset($page_object)) {
    // TODO: Add function in islandora_page_content to do this.
    $page_object_of_relationship = $page_object->relationships->get(ISLANDORA_RELS_EXT_URI, 'isPageOf');
    $book_object = islandora_object_load($page_object_of_relationship[0]['object']['value']);
    $pid_final = $page_object->id;
    $title = $book_object->label;
    $pages = islandora_paged_content_get_pages($book_object);
    $pages = array_keys($pages);
    $position = array_search($page_object->id, $pages);
  }
  else {
    $title = $item->label;

    $pages[1] = $pid;
    $position = array_search($pid, $pages);
    $pid_final = $pid;
  }

  $results['page_title'] = $title;
  $results['object_base'] = url("islandora/object/$pid_final");
  $results['position'] = $position;
  $results['pages'] = $pages;
  $results['no_edit'] = !user_access(ISLANDORA_IMAGE_ANNOTATION_CREATE);
  $results['categories'] = $categories;
  $results['use_dropdown'] = $dropdown;
  // Implemented in JavaScript in RDF, which breaks with URL-encoding.
  $results['manifest_url'] = rawurldecode(url("islandora/anno/serve/$pid_final/Manifest/manifest.xml", array('absolute' => TRUE)));
  $results['islandora_post_url'] = url("islandora/anno/add_annotation/$pid_final");
  $results['get_annotation_list_url'] = url("islandora/anno/get_urns/$pid_final");
  $results['islandora_get_annotation'] = url("islandora/anno/get_annotation/");
  $results['islandora_delete_annotation'] = url("islandora/anno/delete_annotation/$pid_final/");
  $results['islandora_update_annotation'] = url("islandora/anno/update_annotation/");
  $results['islandora_base_url'] = "$base_path";
  $results['islandora_anno_use_title_vocab'] = variable_get('annotation_use_title_vocabulary', FALSE);
  $results['islandora_anno_stroke_width'] = $stroke_width;
  $results['islandora_anno_stroke_widths'] = variable_get('annotation_stroke_widths', '.3 .6. .9  1.2 1.5');
  $results['image_url'] = $image_url;
  $results['can_choose'] = !$optimized;

  $json = json_encode($results);
  return $json;
}
