<?php

/**
 * @file
 * Admin callbacks for Islandora Image Annotation
 */

/**
 * Creates the admin form for islandora_image_annotation.
 *
 * @return array
 *   Islandora annotation admin form.
 */
function islandora_image_annotation_admin($form, &$form_state) {
  module_load_include('inc', 'islandora_image_annotation', 'includes/admin.util');
  drupal_add_css(drupal_get_path('module', 'islandora_image_annotation') . '/css/islandora_annotation.css');

  $coptions = islandora_image_annotation_get_content_models();
  $selected = variable_get('islandora_annotation_mappings', array());
  $enforced = variable_get('islandora_annotation_enforce_taxonomy', 0);
  $optimized = variable_get('islandora_annotation_optimized', 1);
  $chosen = array_keys($selected);
  $pids = array_keys($coptions);
  $alink_options = array(
    'attributes' => array('target' => '_blank'),
    'html' => TRUE,
  );
  foreach ($selected as $key => $value) {
    $label = isset($coptions[$key]) ? $coptions[$key] : "Unlabeled";
    unset($coptions['$key']);
    $coptions = array($key => $label) + $coptions;
  }

  $form['content_model_wrapper'] = array(
    '#type' => 'fieldset',
    '#title' => t('Configure Content Models'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $form['content_model_wrapper']['annotation_table'] = array(
    '#type' => 'item',
    '#title' => t('Enable annotating'),
    '#description' => t('Allow annotation of objects associated with these content models.'),
    '#tree' => TRUE,
    '#theme' => 'islandora_image_annotation_admin_table',
  );

  foreach ($coptions as $pid => $label) {
    $dsids = islandora_image_annotation_get_annotation_dsids_from_dscomp($pid);
    $taxes = islandora_image_annotation_get_taxonomies();
    if (empty($dsids)) {
      continue;
    }
    $options[$pid] = '';
    // Label.
    $form['content_model_wrapper']['annotation_table']['name'][$pid] = array(
      '#type' => 'item',
      '#markup' => l($label, "islandora/object/$pid", $alink_options),
    );
    // Pid.
    $form['content_model_wrapper']['annotation_table']['content_model'][$pid] = array(
      '#type' => 'item',
      '#markup' => $pid,
    );
    // Dsid.
    $form['content_model_wrapper']['annotation_table']['DSID'][$pid] = array(
      '#prefix' => '<div class="dsidSelect">',
      '#suffix' => '</div>',
      '#type' => 'select',
      '#options' => $dsids,
      '#default_value' => isset($selected[$pid]['DSID']) ? $selected[$pid]['DSID'] : '',
    );
    $form['content_model_wrapper']['annotation_table']['TAX'][$pid] = array(
      '#prefix' => '<div class="dsidSelect">',
      '#suffix' => '</div>',
      '#type' => 'select',
      '#options' => $taxes,
      '#default_value' => isset($selected[$pid]['TAX']) ? $selected[$pid]['TAX'] : '',
    );
    $form['content_model_wrapper']['annotation_table']['enabled'] = array(
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => $chosen,
    );
  }

  $form['annotation_categories_fs'] = array(
    '#title' => 'Annotation categories',
    '#type' => 'fieldset',
    '#collapsed' => FALSE,
  );
  $form['annotation_categories_fs']['annotation_categories'] = array(
    '#type' => 'radios',
    '#title' => t('Annotation categories'),
    '#options' => array(0 => t('User defined'), 1 => t("Administrator defined")),
    '#description' => t('Allows administrators to select predefined taxonomies to determine annotation categorization terminology'),
    '#default_value' => $enforced,
  );
  $form['annotation_categories_fs']['annotation_type_search_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Annotation type search field'),
    '#description' => t('If Annotation Categories is set to User Defined and you want to do auto complete enter a solr field.
        This field\'s contents will be used to populate the auto complete.'),
    '#default_value' => variable_get('annotation_type_search_field', 'mads_categories_s'),
    '#weight' => 3,
  );
  $form['annotation_colors_fs'] = array(
    '#title' => 'Annotation stroke & colors',
    '#type' => 'fieldset',
    '#collapsed' => FALSE,
  );
  $form['annotation_colors_fs']['annotation_colors'] = array(
    '#type' => 'radios',
    '#title' => t('Annotation colors'),
    '#options' => array(0 => t('User defined (user selects when creating annotation)'), 1 => t("Optimized (Colors are selected randomly. Recommended for overlapping annotations)")),
    '#default_value' => $optimized,
  );
  $form['annotation_colors_fs']['image_annotation_annotation_stroke_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Annotation stroke widths for drop down'),
    '#default_value' => variable_get('image_annotation_annotation_stroke_width', '.3 .6 .9 1.2 1.5'),
    '#description' => t('The width of the lines to use when drawing the annotations. Must be a number,
      can include a decimal.  For instance .5 is valid.  You can Enter several values here seperated by a space.'),
    '#required' => TRUE,
  );
  $form['annotation_entity_linking'] = array(
    '#title' => 'Allow entity tagging on images.',
    '#type' => 'checkbox',
    '#default_value' => variable_get('islandora_image_annotation_use_entity_tagging', TRUE),
    '#description' => t('If checked, users will be able to tag image annotations with an entity.'),
  );
  $form['annotation_controlled_titles'] = array(
    '#title' => 'Controlled vocabulary for titles',
    '#type' => 'fieldset',
    '#collapsed' => FALSE,
  );
  $form['annotation_controlled_titles']['annotation_use_title_vocabulary'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use a controlled vocabulary for annotation titles'),
    '#default_value' => variable_get('annotation_use_title_vocabulary', FALSE),
    '#weight' => -5,
    '#description' => t('If checked you must configure a Solr field to retrieve the titles from.'),
  );
  $form['annotation_controlled_titles']['annotation_title_search_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Annotation title search field'),
    '#description' => t('This must be a single valued Solr field.'),
    '#default_value' => variable_get('annotation_title_search_field', 'mads_titles_sv'),
    '#weight' => 3,
    '#states' => array(
      // Hide the settings when the cancel notify checkbox is disabled.
      'invisible' => array(
        ':input[name="annotation_use_title_vocabulary"]' => array('checked' => FALSE),
      ),
    ),
  );
  $form['annotation_controlled_titles']['annotation_genre_search_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Annotation genre search field'),
    '#default_value' => variable_get('annotation_genre_search_field', 'mads_genre_it'),
    '#weight' => 4,
    '#states' => array(
      // Hide the settings when the cancel notify checkbox is disabled.
      'invisible' => array(
        ':input[name="annotation_use_title_vocabulary"]' => array('checked' => FALSE),
      ),
    ),
  );
  $form['actions'] = array(
    '#type' => 'actions',
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Configuration'),
    '#weight' => 0,
    '#submit' => array('islandora_image_annotation_admin_submit'),
  );
  return $form;
}

/**
 * Submit handler for the admin form.
 *
 * @todo Needs Validation
 *
 * @param array $form
 *   The admin form.
 *
 * @param array $form_state
 *   The admin form state.
 */
function islandora_image_annotation_admin_submit($form, &$form_state) {
  $enabled = array_filter($form_state['values']['annotation_table']['enabled']);
  $content_model_mappings = array();
  foreach ($enabled as $label => $pid) {
    $content_model_mappings[$pid]['DSID'] = $form_state['values']['annotation_table']['DSID'][$pid];
    $content_model_mappings[$pid]['TAX'] = $form_state['values']['annotation_table']['TAX'][$pid];
  }
  variable_set('islandora_annotation_mappings', $content_model_mappings);
  variable_set('islandora_annotation_enforce_taxonomy', $form_state['values']['annotation_categories']);
  variable_set('islandora_annotation_optimized', $form_state['values']['annotation_colors']);
  variable_set('annotation_use_title_vocabulary', $form_state['values']['annotation_use_title_vocabulary']);
  variable_set('annotation_title_search_field', $form_state['values']['annotation_title_search_field']);
  variable_set('annotation_genre_search_field', $form_state['values']['annotation_genre_search_field']);
  variable_set('annotation_type_search_field', $form_state['values']['annotation_type_search_field']);
  variable_set('image_annotation_annotation_stroke_width', $form_state['values']['image_annotation_annotation_stroke_width']);
  variable_set('islandora_image_annotation_use_entity_tagging', $form_state['values']['annotation_entity_linking']);
}
