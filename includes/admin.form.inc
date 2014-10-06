<?php

/**
 * @file
 * Handles the display/submission of the admin settings form for this module.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_image_annotation_admin_settings_form(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_image_annotation', 'includes/utils');
  $content_models = islandora_image_annotation_get_content_models();
  $taxonomies = islandora_image_annotation_get_taxonomies();
  $annotation_mappings = variable_get('islandora_annotation_mappings', array());
  $options = array();
  foreach ($content_models as $pid => $label) {
    $options[$pid] = array(
      'content_model' => array(
        'data' => array(
          '#type' => 'link',
          '#title' => filter_xss($label),
          '#href' => "islandora/object/{$pid}",
        ),
      ),
      'pid' => check_plain($pid),
      'dsid' => array(
        'data' => array(
          '#type' => 'select',
          '#name' => "dsid[$pid]",
          '#attributes' => array('style' => 'width: 120px'),
          '#options' => islandora_image_annotation_get_dsids_from_dscomp($pid) + array('TN' => 'TN'),
          '#value' => isset($annotation_mappings[$pid]['DSID']) ? $annotation_mappings[$pid]['DSID'] : '',
        ),
      ),
      'taxonomy' => array(
        'data' => array(
          '#type' => 'select',
          '#name' => "taxonomy[$pid]",
          '#attributes' => array('style' => 'width: 120px'),
          '#options' => $taxonomies,
          '#value' => isset($annotation_mappings[$pid]['TAX']) ? $annotation_mappings[$pid]['TAX'] : '',
        ),
      ),
    );
  }
  return system_settings_form(array(
    'content_models' => array(
      '#type' => 'fieldset',
      '#title' => t('Configure Content Models'),
      '#description' => t('Allow annotation of objects associated with these content models.'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
      'islandora_annotation_mappings' => array(
        '#type' => 'tableselect',
        '#header' => array(
          'content_model' => t('Content Model'),
          'pid' => t('PID'),
          'dsid' => t('DSID'),
          'taxonomy' => t('Taxonomy'),
        ),
        '#options' => $options,
        '#default_value' => array_filter($annotation_mappings),
        '#empty' => t('No content models available.'),
        '#value_callback' => 'islandora_image_annotation_admin_settings_form_islandora_annotation_mappings_value',
      ),
      // Required to include fields in a select table. They are just temporary,
      // and should not be submitted otherwise it will cause them to be saved as
      // variables.
      'dsid' => array(
        '#type' => 'value',
      ),
      // Required to include fields in a select table. They are just temporary,
      // and should not be submitted otherwise it will cause them to be saved as
      // variables.
      'taxonomy' => array(
        '#type' => 'value',
      ),
    ),
    'annotation_categories' => array(
      '#type' => 'fieldset',
      '#title' => t('Annotation categories'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
      'islandora_annotation_enforce_taxonomy' => array(
        '#type' => 'radios',
        '#title' => t('Annotation categories'),
        '#options' => array(0 => t('User defined'), 1 => t("Administrator defined")),
        '#description' => t('Allows administrators to select predefined taxonomies to determine annotation categorization terminology'),
        '#default_value' => variable_get('islandora_annotation_enforce_taxonomy', 0),
      ),
      'islandora_annotation_type_search_field' => array(
        '#type' => 'textfield',
        '#title' => t('Annotation type search field'),
        '#description' => t("If Annotation Categories is set to User Defined and you want to do auto complete enter a solr field. This field's contents will be used to populate the auto complete."),
        '#default_value' => variable_get('islandora_annotation_type_search_field', 'mads_categories_s'),
        '#weight' => 3,
      ),
    ),
    'annotation_colors' => array(
      '#type' => 'fieldset',
      '#title' => t('Annotation stroke & colors'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
      'islandora_annotation_optimized' => array(
        '#type' => 'radios',
        '#title' => t('Annotation colors'),
        '#options' => array(0 => t('User defined (user selects when creating annotation)'), 1 => t("Optimized (Colors are selected randomly. Recommended for overlapping annotations)")),
        '#default_value' => variable_get('islandora_annotation_optimized', 1),
      ),
      'islandora_image_annotation_stroke_width' => array(
        '#type' => 'textfield',
        '#title' => t('Annotation stroke widths for drop down'),
        '#required' => TRUE,
        '#default_value' => variable_get('islandora_image_annotation_stroke_width', '.3 .6 .9 1.2 1.5'),
        '#description' => t('The width of the lines to use when drawing the annotations. Must be a number, can include a decimal. For instance .5 is valid. You can Enter several values here separated by a space.'),
      ),
    ),
    // @todo Do we really want to tag entities without the module enabled?
    'islandora_image_annotation_use_entity_tagging' => array(
      '#title' => t('Allow entity tagging on images.'),
      '#type' => 'checkbox',
      '#default_value' => variable_get('islandora_image_annotation_use_entity_tagging', TRUE),
      '#description' => t('If checked, users will be able to tag image annotations with an entity.'),
    ),
    'islandora_image_annotation_use_title_vocabulary' => array(
      '#type' => 'checkbox',
      '#title' => t('Use a controlled vocabulary for annotation titles'),
      '#description' => t('If checked you must configure a Solr field to retrieve the titles from.'),
      '#default_value' => variable_get('islandora_image_annotation_use_title_vocabulary', FALSE),
    ),
    'annotation_controlled_titles' => array(
      '#type' => 'fieldset',
      '#title' => t('Controlled vocabulary for titles'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
      '#states' => array(
        // Hide the settings when the cancel notify checkbox is disabled.
        'invisible' => array(
          ':input[name="islandora_image_annotation_use_title_vocabulary"]' => array('checked' => FALSE),
        ),
      ),
      'islandora_image_annotation_title_search_field' => array(
        '#type' => 'textfield',
        '#title' => t('Annotation title search field'),
        '#description' => t('This must be a single valued Solr field.'),
        '#default_value' => variable_get('islandora_image_annotation_title_search_field', 'mads_titles_sv'),
        '#states' => array(
          // Hide the settings when the cancel notify checkbox is disabled.
          'invisible' => array(
            ':input[name="islandora_image_annotation_use_title_vocabulary"]' => array('checked' => FALSE),
          ),
        ),
      ),
      'islandora_image_annotation_genre_search_field' => array(
        '#type' => 'textfield',
        '#title' => t('Annotation genre search field'),
        '#default_value' => variable_get('islandora_image_annotation_genre_search_field', 'mads_genre_it'),
        '#states' => array(
          // Hide the settings when the cancel notify checkbox is disabled.
          'invisible' => array(
            ':input[name="islandora_image_annotation_use_title_vocabulary"]' => array('checked' => FALSE),
          ),
        ),
      ),
    ),
  ));
}

/**
 * Maps the values for dsid and taxonomy to the appropriate variable.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_image_annotation_admin_settings_form_validate(array $form, array &$form_state) {
  $value = array();
  $selected = array_filter($form_state['values']['islandora_annotation_mappings']);
  foreach ($selected as $pid) {
    $value[$pid] = array(
      'DSID' => $form_state['values']['dsid'][$pid],
      'TAX' => $form_state['values']['taxonomy'][$pid],
    );
  }
  form_set_value($form['content_models']['islandora_annotation_mappings'], $value, $form_state);
  // These two fields should NOT get saved as variables.
  unset($form_state['values']['dsid']);
  unset($form_state['values']['taxonomy']);
}
