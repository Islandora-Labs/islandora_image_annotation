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
  module_load_include('module', 'taxonomy', 'module');
  global $base_path;

  $mappings = variable_get('islandora_annotation_mappings', array());
  $optimized = variable_get('islandora_annotation_optimized', TRUE);
  $dropdown = variable_get('islandora_annotation_enforce_taxonomy', TRUE);

  $item = islandora_object_load($pid);
  $rels = $item->relationships->get();
  foreach ($rels as $relationship) {
    if ($relationship['predicate']['value'] == 'hasModel') {
      $cmodel = $relationship['object']['value'];
    }
  }
  $vid = isset($mappings[$cmodel]['TAX']) ? $mappings[$cmodel]['TAX'] : NULL;

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
  $results = array();
  $results['object_base'] = url("islandora/object/$pid");
  $results['no_edit'] = !user_access(ISLANDORA_IMAGE_ANNOTATION_CREATE);
  $results['categories'] = $categories;
  $results['use_dropdown'] = $dropdown;
  // Implemented in JavaScript in RDF, which breaks with URL-encoding.
  $results['manifest_url'] = rawurldecode(url(
    "islandora/anno/serve/$pid/Manifest/manifest.xml",
    array('absolute' => TRUE)));
  $results['islandora_post_url'] = url("islandora/anno/add_annotation/$pid");
  $results['get_annotation_list_url'] = url("islandora/anno/get_urns/$pid");
  $results['islandora_get_annotation'] = url("islandora/anno/get_annotation/");
  $results['islandora_delete_annotation'] = url("islandora/anno/delete_annotation/$pid/");
  $results['islandora_update_annotation'] = url("islandora/anno/update_annotation/");
  $results['islandora_base_url'] = $base_path;
  $results['islandora_anno_use_title_vocab'] = variable_get('annotation_use_title_vocabulary', FALSE);
  $results['islandora_anno_stroke_widths'] = variable_get('image_annotation_annotation_stroke_width', '.3 .6 .9 1.2 1.5');
  $results['can_choose'] = !$optimized;

  return $results;
}

/**
 * A library function for loading Javascript dependencies of shared Canvas.
 *
 * This can be used to support shared canvases in other modules.  They need
 * only call this function rather than include all the Javascript themselves.
 */
function islandora_image_annotation_include_common_js() {
  $image_annotation_module_path = drupal_get_path('module', 'islandora_image_annotation');

  $allow_entity_linking = variable_get('islandora_image_annotation_use_entity_tagging', TRUE);
  drupal_add_js(array('islandora_image_annotation' => array('allow_entity_linking' => $allow_entity_linking)), 'setting');

  $images_path = url($image_annotation_module_path . '/shared_canvas/imgs', array('absolute' => TRUE));
  $entity_link_path = url('islandora/object/', array('absolute' => TRUE));
  drupal_add_js(array('islandora_image_annotation' => array('images_path' => $images_path)), 'setting');
  drupal_add_js(array('islandora_image_annotation' => array('entity_link_path' => $entity_link_path)), 'setting');
  drupal_add_js(array('islandora_image_annotation' => array('entities_search_callback' => url('islandora/entities/search'))), 'setting');
  drupal_add_js($image_annotation_module_path . '/shared_canvas/stable/entity_search.js');

  drupal_add_js($image_annotation_module_path . '/shared_canvas/stable/dialog_annotate.js');
  drupal_add_js($image_annotation_module_path . '/shared_canvas/js/jquery/jquery.watermark.min.js');
  drupal_add_js($image_annotation_module_path . '/shared_canvas/js/ContextMenu/jquery.contextMenu.js');

  drupal_add_js($image_annotation_module_path . '/shared_canvas/js/jquery/jquery.svg.js');
  drupal_add_js($image_annotation_module_path . '/shared_canvas/js/jquery.rdfquery.rdfa.min-1.1.js');
  drupal_add_js($image_annotation_module_path . '/shared_canvas/js/jquery.rdf.turtle.js');
  drupal_add_js($image_annotation_module_path . '/shared_canvas/js/jquery.touchSwipe-1.2.4.js');
  drupal_add_js($image_annotation_module_path . '/shared_canvas/js/raphael.js');
  drupal_add_js($image_annotation_module_path . '/shared_canvas/js/scale.raphael.js');
  drupal_add_js($image_annotation_module_path . '/shared_canvas/js/uuid.js');
  drupal_add_js($image_annotation_module_path . '/shared_canvas/js/jquery/jquery.miniColors.js');

  drupal_add_js($image_annotation_module_path . '/shared_canvas/stable/sc_ui.js');
  drupal_add_js($image_annotation_module_path . '/shared_canvas/stable/sc_utils.js');
  drupal_add_js($image_annotation_module_path . '/shared_canvas/stable/sc_rdf.js');
  drupal_add_js($image_annotation_module_path . '/shared_canvas/stable/sc_rdfjson.js');
  drupal_add_js($image_annotation_module_path . '/shared_canvas/stable/sc_create.js');
  drupal_add_js($image_annotation_module_path . '/shared_canvas/stable/sc_pastebin.js');

}

/**
 * A library function for loading CSS dependencies of shared Canvas.
 *
 * This can be used to support shared canvases in other modules.  They need
 * only call this function rather than include all the CSS themselves.
 */
function islandora_image_annotation_include_common_css() {
  $image_annotation_module_path = drupal_get_path('module', 'islandora_image_annotation');

  drupal_add_css($image_annotation_module_path . '/shared_canvas/js/ContextMenu/jquery.contextMenu.css');
  drupal_add_css($image_annotation_module_path . '/shared_canvas/css/jquery.miniColors.css');
  drupal_add_css($image_annotation_module_path . '/shared_canvas/css/sc.css');
}
