<?php

/**
 * @file
 * This file contains all theme and preprocess functions.
 */

/**
 * Proprocess hook.
 *
 * @global string $base_url
 *
 * @param array $variables
 *   An array of variables passed from the theme function.
 */
function islandora_image_annotation_preprocess_islandora_image_annotation_view(&$variables) {
  $message = islandora_deprecated('7.x-1.4', 'use theme("islandora_image_annotation") instead.');
  trigger_error(filter_xss($message), E_USER_DEPRECATED);
  // Set preprocessor variables, for use in JavaScript.
  $variables['anno_list_pane'] = theme('islandora_anno_list_pane');
  $variables['anno_img_pane'] = theme('islandora_anno_image_pane');

  module_load_include('inc', 'islandora_image_annotation', 'includes/utils');
  $module_path = drupal_get_path('module', 'islandora_image_annotation');

  // Add css.
  drupal_add_css($module_path . '/shared_canvas/css/window_behaviors.css');
  drupal_add_css($module_path . '/shared_canvas/css/islandora_shared_canvas_style.css');
  drupal_add_css($module_path . '/shared_canvas/css/smoothness/jquery-ui-1.8.16.custom.css');

  // Add JavaScript.
  drupal_add_js($module_path . '/shared_canvas/js/jquery/jquery-1.7.js');
  drupal_add_js($module_path . '/shared_canvas/js/jquery/jquery-ui-1.8.13.custom.min.js');

  // Include common JavaScript/CSS.
  islandora_image_annotation_include_common_js();
  islandora_image_annotation_include_common_css();

  // Add JavaScript.
  drupal_add_js(array('islandora_image_annotation' => array('PID' => $variables['islandora_object']->id)), 'setting');
  $module_path = drupal_get_path('module', 'islandora_image_annotation');
  drupal_add_js($module_path . '/shared_canvas/stable/islandora_image_annotation_init.js');
}

/**
 * Implements template_preprocess_theme().
 */
function template_preprocess_islandora_image_annotation(array &$variables) {
  $module_path = drupal_get_path('module', 'islandora_image_annotation');
  $object = $variables['object'];
  $settings = array(
    'pid' => $object->id,
  );
  drupal_add_js(array('islandoraImageAnnotation' => $settings), 'setting');
  // Set preprocessor variables, for use in JavaScript.
  $variables['dialog_box'] = array(
    '#theme' => 'islandora_image_annotation_dialog',
    '#object' => $object,
  );
  // We embed the list in our tabs, cause why not.
  drupal_add_library('system', 'ui.tabs');
  drupal_add_js("$module_path/js/islandora_image_annotation_tabs.js");
  $variables['list'] = array(
    '#theme' => 'islandora_image_annotation_list',
    '#object' => $object,
  );
  // The canvas is where the image and it's annotations get rendered.
  $variables['canvas'] = array(
    '#theme' => 'islandora_image_annotation_canvas',
    '#object' => $object,
  );
  // Include required Javascript libraries..
  drupal_add_js("$module_path/lib/jquery/contextMenu/jquery.contextMenu.js");
  drupal_add_js("$module_path/lib/jquery/miniColors/jquery.miniColors.js");
  drupal_add_js("$module_path/lib/jquery/jquery.rdfquery.rdfa.min-1.1.js");
  drupal_add_js("$module_path/lib/raphael/raphael.js");
  drupal_add_js("$module_path/lib/raphael/scale.raphael.js");
  drupal_add_js("$module_path/lib/uuid.js");
  drupal_add_js("$module_path/lib/he.js");
  // Include this modules Javascript.
  drupal_add_js("$module_path/js/islandora_image_annotation_utils.js");
  drupal_add_js("$module_path/js/rdf.js");
  drupal_add_js("$module_path/js/islandora_image_annotation.js");
  drupal_add_js("$module_path/js/islandora_image_annotation_buttons.js");
  // Include required Javascript libraries CSS files.
  drupal_add_css("$module_path/lib/jquery/contextMenu/jquery.contextMenu.css");
  drupal_add_css("$module_path/lib/jquery/miniColors/jquery.miniColors.css");
  // Include this modules CSS.
  drupal_add_css("$module_path/css/islandora_image_annotation.base.css");
}

/**
 * Implements template_process_theme().
 */
function template_process_islandora_image_annotation(array &$variables) {
  $variables['dialog_box'] = drupal_render($variables['dialog_box']);
  $variables['list'] = drupal_render($variables['list']);
  $variables['canvas'] = drupal_render($variables['canvas']);
}

/**
 * Implements template_preprocess_theme().
 */
function template_preprocess_islandora_image_annotation_dialog(array &$variables) {
  module_load_include('inc', 'islandora_image_annotation', 'includes/utils');
  $module_path = drupal_get_path('module', 'islandora_image_annotation');
  $variables['annotation_icons'] = array(
    'rectangle' => array(
      '#theme' => 'image',
      '#path' => "$module_path/images/draw_rectangle.png",
      '#attributes' => array(
        'id' => 'islandora-image-annotation-shape-rectangle',
        // Rectangle is selected by default.
        'class' => 'islandora-image-annotation-shape-icon selected',
        'shape' => 'rectangle',
      ),
    ),
    'circle' => array(
      '#theme' => 'image',
      '#path' => "$module_path/images/draw_circle.png",
      '#attributes' => array(
        'id' => 'islandora-image-annotation-shape-circle',
        'class' => 'islandora-image-annotation-shape-icon',
        'shape' => 'circle',
      ),
    ),
    'polygon' => array(
      '#theme' => 'image',
      '#path' => "$module_path/images/draw_polygon.png",
      '#attributes' => array(
        'id' => 'islandora-image-annotation-shape-polygon',
        'class' => 'islandora-image-annotation-shape-icon',
        'shape' => 'polygon',
      ),
    ),
  );
  $variables['use_entity_tagging'] = variable_get('islandora_image_annotation_use_entity_tagging', TRUE);
  $variables['can_choose_colour'] = !variable_get('islandora_annotation_optimized', TRUE);
  $object = $variables['object'];
  $settings = array(
    'pid' => $object->id,
    'categories' => islandora_image_annotation_get_categories($object),
    'enforceTaxonomy' => variable_get('islandora_annotation_enforce_taxonomy', TRUE),
    'allowEntityLinking' => variable_get('islandora_image_annotation_use_entity_tagging', TRUE),
    'useTitleVocabulary' => variable_get('islandora_image_annotation_use_title_vocabulary', FALSE),
    'strokeWidths' => variable_get('islandora_image_annotation_stroke_width', '.3 .6 .9 1.2 1.5'),
    'canChooseColour' => $variables['can_choose_colour'],
  );
  drupal_add_js(array('islandoraImageAnnotationDialog' => $settings), 'setting');
  drupal_add_library('system', 'ui.dialog');
  drupal_add_js("$module_path/js/islandora_image_annotation_dialog.js");
  drupal_add_css("$module_path/css/islandora_image_annotation.dialog.css");
}

/**
 * Implements template_process_theme().
 */
function template_process_islandora_image_annotation_dialog(array &$variables) {
  $variables['annotation_icons'] = drupal_render($variables['annotation_icons']);
}

/**
 * Implements template_preprocess_theme().
 */
function template_preprocess_islandora_image_annotation_list(array &$variables) {
  $module_path = drupal_get_path('module', 'islandora_image_annotation');
  $object = $variables['object'];
  $settings = array(
    'pid' => $object->id,
    'editable' => user_access(ISLANDORA_IMAGE_ANNOTATION_CREATE),
    'annotations' => islandora_image_annotation_get_annotations($object),
  );
  drupal_add_js(array('islandoraImageAnnotationList' => $settings), 'setting');
  drupal_add_js("$module_path/js/islandora_image_annotation_list.js");
  drupal_add_css("$module_path/css/islandora_image_annotation.list.css");
}

/**
 * Implements template_preprocess_theme().
 */
function template_preprocess_islandora_image_annotation_canvas(array &$variables) {
  $module_path = drupal_get_path('module', 'islandora_image_annotation');
  $object = $variables['object'];
  $settings = array(
    'pid' => $object->id,
  );
  drupal_add_js(array('islandoraImageAnnotationCanvas' => $settings), 'setting');
  drupal_add_js("$module_path/js/islandora_image_annotation_canvas.js");
  drupal_add_css("$module_path/css/islandora_image_annotation.canvas.css");
}
