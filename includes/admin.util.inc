<?php
/**
 * @file
 * Helper functions for the admin interface.
 */

/**
 * Themes the form table.
 *
 * @param array $variables
 *   An array containing variables used in the theme.
 *
 * @return string
 *   The marked up output.
 */
function theme_islandora_annotation_table($variables) {
  // Set form.
  $form = $variables['form'];
  $rows = array();

  foreach ($form['name'] as $key => $element) {
    // Do not take form control structures.
    if (is_array($element) && element_child($key)) {
      $row = array();

      $row[] = array('data' => drupal_render($form['enabled'][$key]));
      $row[] = array('data' => drupal_render($form['name'][$key]), 'title' => $key);
      $row[] = array('data' => drupal_render($form['content_model'][$key]));
      $row[] = array('data' => drupal_render($form['DSID'][$key]));
      $row[] = array('data' => drupal_render($form['TAX'][$key]));

      // Add to rows.
      $rows[] = array('data' => $row, 'class' => array('draggable'));
    }
  }

  // Individual table headers.
  $header = array();

  $header[] = array('data' => t('Enable Annotating'));
  $header[] = array('data' => t('Content Model'));
  $header[] = array('data' => t('PID'));
  $header[] = array('data' => t('DSID'));
  $header[] = array('data' => t('Taxonomy'));

  // Render form.
  $output = '';
  $output .= theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array(
      'id' => 'annotation_admin_table',
    ),
  ));
  $output .= drupal_render_children($form);
  return $output;
}

/**
 * Retrieves content models from Fedora.
 *
 * @return array
 *   An array containing key value pairs of content model pids and titles.
 */
function islandora_annotation_get_content_models() {
  module_load_include('inc', 'islandora', 'includes/tuque');
  $repository = new IslandoraTuque();
  $query = 'select $object $title from <#ri>
               where $object <info:fedora/fedora-system:def/model#hasModel> <info:fedora/fedora-system:ContentModel-3.0>
               and $object <fedora-model:label> $title
               and ($object <fedora-view:disseminates> $ds1 in <#ri>
               and $ds1 <fedora-view:disseminationType> <info:fedora/*/DS-COMPOSITE-MODEL> in <#ri>)
               order by $title';
  $objects = $repository->repository->ri->itqlQuery($query, 'unlimited', '0');
  $return_array = array();
  foreach ($objects as $object) {
    $cmodel_pid = $object['object']['value'];
    if (strpos($cmodel_pid, 'fedora-system') !== 0) {
      $cmodel_title = $object['title']['value'];
      if ($cmodel_title) {
        $title_string = $cmodel_title . ' ~ ' . $cmodel_pid;
      }
      else {
        $title_string = $cmodel_pid;
      }

      $return_array[$cmodel_pid] = $cmodel_title;
    }
  }
  return $return_array;
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
function get_annotation_dsids_from_dscomp($pid) {
  module_load_include('inc', 'islandora', 'includes/tuque');
  $fedora_object = islandora_object_load($pid);
  $datastream = $fedora_object['DS-COMPOSITE-MODEL'];
  $ds_comp_stream = $datastream->content;
  $sxml = new SimpleXMLElement($ds_comp_stream);
  $output = array();
  foreach ($sxml->dsTypeModel as $ds) {
    $mimes = array();
    foreach ($ds->form as $form) {
      $mimetype = (string) $form['MIME'];
      if (strstr($mimetype, 'image')) {
        $mimes[] = $mimetype;
      }
    }
    $dsid = (string) $ds['ID'];
    if (count($mimes) > 0) {
      $output[(string) $ds['ID']] = (string) $ds['ID'];
    }
  }
  return $output;
}

/**
 * Retrieve a full list of collections from Fedora.
 *
 * @return array
 *   An array in the form of cmodel_pid => cmodel_title.
 */
function islandora_annotation_get_collections() {
  module_load_include('inc', 'islandora', 'includes/tuque');
  $repository = new IslandoraTuque();
  $query = 'select $object $title from <#ri>
               where ($object <info:fedora/fedora-system:def/model#hasModel> <info:fedora/islandora:collectionCModel>
               and $object <fedora-model:label> $title
               and $object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>)';
  $objects = $repository->repository->ri->itqlQuery($query, 'unlimited', '0');
  $return_array = array();
  foreach ($objects as $object) {
    $cmodel_pid = $object['object']['value'];
    if (strpos($cmodel_pid, 'fedora-system') !== 0) {
      $cmodel_title = $object['title']['value'];
      if ($cmodel_title) {
        $title_string = $cmodel_title . ' ~ ' . $cmodel_pid;
      }
      else {
        $title_string = $cmodel_pid;
      }

      $return_array[$cmodel_pid] = $cmodel_title;
    }
  }
  return $return_array;
}

/**
 * Retrieves all taxonomies defined within Drupal.
 *
 * @return array
 *   A key value pair array containing the vid and taxonomy name.
 */
function islandora_annotation_get_taxonomies() {
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
