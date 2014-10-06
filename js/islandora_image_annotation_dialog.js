/*jslint browser: true*/
/*global jQuery, Drupal, UUID, IIAUtils*/
/**
 * @file
 * Defines the Islandora Image Annotation widget and it's Drupal behaviour.
 *
 * Requires jQuery 1.7+.
 * Requires jQuery UI.
 */
(function ($) {
  'use strict';
  /**
   * The DOM element that represents the Singleton Instance of this class.
   * @type {string}
   */
  var base = '#islandora-image-annotation-dialog';

  /**
   * Initialize the Create Annotation Dialog Box.
   *
   * We can only have one Create Annotation Dialog Box per page.
   */
  Drupal.behaviors.islandoraImageAnnotationDialog = {
    attach: function (context, settings) {
      $(base, context).once('islandoraImageAnnotationDialog', function () {
        Drupal.IslandoraImageAnnotationDialog[base] = new Drupal.IslandoraImageAnnotationDialog(base, settings.islandoraImageAnnotationDialog);
      });
    }
  };

  /**
   * Instantiate the Create Annotation Dialog Box.
   *
   * @param {string} base
   *   The id of the html element in which to bind the dialog box to.
   *
   * @param {object} settings
   *   The Drupal.settings variable for this widget.
   *   - canSelectColour: boolean, indicates the user can choose the color of
   *     the annotation.
   *   - canLinkEntity: boolean, indicates the user can link an existing entity
   *     to the annotation.
   *
   * @constructor
   */
  Drupal.IslandoraImageAnnotationDialog = function (base, settings) {
    // Private Variables.
    var $dialog, defaultDialogProperties;
    $dialog = $(base);
    defaultDialogProperties = {
      height: 680,
      width: 460,
      resizable: true,
      closeOnEscape: true,
      modal: false,
      position: {
        my: "left top",
        at: "left bottom",
        of: '#content'
      }
    };

    /**
     * Fetches a random color from the 'svgAreaColors' table.
     *
     * @returns {string}
     */
    function getRandomColour() {
      var svgAreaColors = [
        '#FF0000', '#FF6600', '#FF9400', '#FEC500', '#FFFF00', '#8CC700',
        '#0FAD00', '#00A3C7', '#0064B5', '#0010A5', '#6300A5', '#C5007C'];
      return svgAreaColors[Math.floor(Math.random() * svgAreaColors.length)];
    }

    /**
     * Takes a Date object and converts it into an ISO formatted date.
     *
     * @param {Date} date
     *
     * @returns {string}
     */
    function isoDate(date) {
      var dt = date.getUTCFullYear().toString(),
        m = (date.getUTCMonth() + 1).toString(),
        dy = date.getUTCDate().toString(),
        hr = date.getUTCHours().toString(),
        mn = date.getUTCMinutes().toString(),
        sc = date.getUTCSeconds().toString();
      m = m.length === 1 ? '0' + m : m;
      dy = dy.length === 1 ? '0' + dy : dy;
      hr = hr.length === 1 ? '0' + hr : hr;
      mn = mn.length === 1 ? '0' + mn : mn;
      sc = sc.length === 1 ? '0' + sc : sc;
      return dt + '-' + m + '-' + dy + ' ' + hr + ':' + mn + ':' + sc + ' UTC';
    }

    /**
     * Maps the given DOMNode to an SVG XML string.
     *
     * @param {DOMNode} node
     *
     * @returns {string}
     */
    function nodeToSVG(node) {
      var SVG_NS = 'http://www.w3.org/2000/svg',
        name = node.nodeName,
        attributes;
      attributes = $.map(node.attributes, function (attribute) {
        return attribute.nodeName + "='" + attribute.nodeValue + "'";
      });
      attributes.push(" xmlns:svg='" + SVG_NS + "' ");
      return '<svg:' + name + ' ' + attributes.join(' ') + '></svg:' + name + '>';
    }

    /**
     * Generates RDFa representing the annotation the user has created.
     *
     * The parameters define the information the user has provided in the create
     * annotation form and the canvas and shapes they have drawn.
     *
     * All the parameters are required. Except entityID, entityLabel which are
     * optional.
     *
     * @param {object} properties
     * @param {string} properties.title
     * @param {string} properties.type
     * @param {string} properties.entity
     * @param {string} properties.entityLabel
     * @param {string} properties.color
     * @param {string} properties.content
     * @param {string} properties.canvas
     * @param {[object]} [properties.shapes]
     *
     * @returns {string}
     *   The RDFa representation of this annotation.
     */
    function createAnnotation(properties) {
      var namespaces, title, type, entityID, entityLabel, color, text,
        canvas, shapes, xmlns, rdfa, annotationUUID, contentUUID;
      namespaces = {
        dc: 'http://purl.org/dc/elements/1.1/',
        dcterms: 'http://purl.org/dc/terms/',
        dctype: 'http://purl.org/dc/dcmitype/',
        oa: 'http://www.w3.org/ns/openannotation/core/',
        cnt: 'http://www.w3.org/2008/content#',
        dms: 'http://dms.stanford.edu/ns/',
        rdf: 'http://www.w3.org/1999/02/22-rdf-syntax-ns#',
        ore: 'http://www.openarchives.org/ore/terms/',
        exif: 'http://www.w3.org/2003/12/exif/ns#'
      };
      title = properties.title;
      type = properties.type;
      entityID = properties.entity;
      entityLabel = properties.entity_label;
      color = properties.color;
      text = properties.text;
      canvas = properties.canvas;
      shapes = properties.shapes || [];

      // We build the rdfa as a string.
      rdfa = '';
      annotationUUID = new UUID();
      contentUUID = new UUID();

      // Generate namespaces from RDF options
      xmlns = $.map(namespaces, function (value, key) {
        return 'xmlns:' + key + '="' + value + '"';
      });

      // Build the RDFa, open the Root Tag.
      rdfa = '<div ' + xmlns.join(' ') + '>'; // Open Root Div

      // Build Annotation, include UUID and type and creation date.
      rdfa += '<div about="urn:uuid:' + annotationUUID + '"> '; // Open Annotation Div
      rdfa += ('<a rel="rdf:type" href="' + namespaces.oa + 'Annotation' + '"></a>');
      rdfa += '<span property="dcterms:created" content="' + isoDate(new Date()) + '"></span> ';

      // Include the linked entity if defined.
      if (typeof entityID === 'string' && entityID !== '') {
        rdfa += '<span property="dcterms:relation" content="' + entityID + '"></span>';
        rdfa += '<span property="dcterms:hasPart" content="' + entityLabel + '"></span>';
      }

      // Include title / type.
      rdfa += '<span property="dc:title" content="' + title + '"></span>';
      rdfa += '<span property="dc:type" content="' + type + '"></span>';

      // Include the actual annotation field's content.
      rdfa += '<a rel="oa:hasBody" href="urn:uuid:' + contentUUID + '"></a>';
      rdfa += '<div about="urn:uuid:' + contentUUID + '">'; // Open Body Div
      rdfa += '<a rel="rdf:type" href="http://www.w3.org/2008/content#ContentAsText"></a>';
      rdfa += '<span property="cnt:chars">' + text + '</span> ';
      rdfa += '<span property="cnt:characterEncoding" content="utf-8"></span>';
      rdfa += '</div>'; // Close Body Div

      // Include shape SVG data, filter out ones that don't exist.
      shapes = $.grep(shapes, function (shape) {
        return shape.removed === undefined;
      });
      $.each(shapes, function (index, shape) {
        var svgXml, constrainedTargetUUID, svgConstraintUUID;
        svgXml = nodeToSVG(shape.node);
        constrainedTargetUUID = new UUID();
        svgConstraintUUID = new UUID();
        svgXml = svgXml.replace("stroke='#000000'", "stroke='" + color +  "'");
        // Escape html entities, probably could use some core
        // javascript for this.
        svgXml = svgXml.replace('<', '&lt;');
        svgXml = svgXml.replace('<', '&lt;');
        svgXml = svgXml.replace('>', '&gt;');
        svgXml = svgXml.replace('>', '&gt;');
        rdfa += '<a rel="oa:hasTarget" href="urn:uuid:' + constrainedTargetUUID + '"></a>';
        rdfa += '<div about="urn:uuid:' + constrainedTargetUUID + '">';
        rdfa += '<a rel="rdf:type" href="http://www.w3.org/ns/openannotation/core/ConstrainedTarget"></a>';
        rdfa += '<a rel="oa:constrains" href="' + canvas + '"></a>';
        rdfa += '<a rel="oa:constrainedBy" href="urn:uuid:' + svgConstraintUUID + '"></a>';
        rdfa += '<div about="urn:uuid:' + svgConstraintUUID + '">';
        rdfa += '<a rel="rdf:type" href="http://www.w3.org/ns/openannotation/core/SvgConstraint"></a>';
        rdfa += '<a rel="rdf:type" href="http://www.w3.org/2008/content#ContentAsText"></a>';
        rdfa += '<span property="cnt:chars" content="' + svgXml + '"></span>';
        rdfa += '<span property="cnt:characterEncoding" content="utf-8"></span>';
        rdfa += "</div>"; // Close Constraint
        rdfa += "</div>"; // Close Constrained Target
      });
      rdfa += "</div>"; // Close Annotation
      rdfa += "</div>"; // Close Root
      return rdfa;
    }

    /**
     * Creates an Annotation Object in Fedora from the data provided.
     *
     * @param {string} rdfa
     *   An rdfa xml encoded representation of the annotation.
     */
    function ingestAnnotation(rdfa) {
      $.ajax({
        type: 'POST',
        async: true,
        url: IIAUtils.url('islandora/object/' + settings.pid + '/annotation/add'),
        data: {
          data: rdfa
        },
        success: function (pid) {
          var url = IIAUtils.url('islandora/object/' + settings.pid + '/annotation/get/' + pid);
          Drupal.IslandoraImageAnnotation.getInstance().fetchTriples(url);
        },
        error: function () {
          console.log('Failed to Create Annotation for: ' + settings.pid);
        }
      });
    }

    /**
     * Updates the given annotation server side.
     *
     * @param annotation
     * @param properties
     */
    function updateAnnotation(annotation, properties) {
      // We need to store slash fetch the pid here.
      $.ajax({
        type: 'POST',
        async: false,
        url: IIAUtils.url('islandora/object/' + settings.pid + '/annotation/update/' + annotation.id),
        data: {
          label: properties.title,
          type: properties.type,
          content: properties.text
        },
        success: function () {
          var url = IIAUtils.url('islandora/object/' + settings.pid + '/annotation/get/' + annotation.id);
          // Trigger reprocessing by deleting it first.
          Drupal.IslandoraImageAnnotation.getInstance().deleteAnnotation(annotation.id);
          Drupal.IslandoraImageAnnotation.getInstance().fetchTriples(url);
        }
      });
    }

    /**
     * Populates the form with the given name / values pairs.
     */
    function setFormValues(values) {
      $.each(values, function (name, value) {
        $('*[name="' + name + '"]', $dialog).val(value);
      });
    }

    /**
     * Resets the form to it's defaults.
     */
    function getFormValues() {
      var values, canvas;
      values = {};
      // Get all fields values.
      $('input,select,textarea', $dialog).each(function () {
        var $this, name;
        $this = $(this);
        name = $(this).attr('name');
        values[name] = $this.val();
      });
      canvas = Drupal.IslandoraImageAnnotationCanvas.getInstance();
      // If no specific canvas is given assume the current canvas.
      if (!values.canvas) {
        values.canvas = canvas.getCurrentCanvas();
      }
      values.shapes = canvas.getShapes('comment', values.canvas);
      return values;
    }

    /**
     * Resets the form to it's defaults.
     */
    function populateForm(annotation) {
      // Although there can be multiple targets, we just assume one for this
      // form.
      var $svg = $(annotation.targets[0].constraint.value);
      setFormValues({
        title: annotation.title,
        type: annotation.annotationType,
        stroke: $svg.attr('stroke-width'),
        color: $svg.attr('stroke'),
        text: annotation.body.value,
        canvas: annotation.getTargetCanvas().id,
        shape: 'rectangle'
      });
    }

    /**
     * Choose the shape to annotate with.
     *
     * @param {string} shape
     *   Expected to be 'rectangle', 'circle', or 'polygon'.
     */
    function chooseShape(shape) {
      $('.islandora-image-annotation-shape-icon', $dialog).removeClass('selected');
      $('.islandora-image-annotation-shape-icon[shape="' + shape + '"]', $dialog).addClass('selected');
      $('input[name="shape"]', $dialog).val(shape);
    }

    /**
     * Resets the form to it's defaults.
     */
    function clearForm(edit) {
      // Clear All the fields.
      $('input,select,textarea', $dialog).val('');
      // Reset Defaults.
      $('input[name="stroke"]', $dialog).val('.3%');
      chooseShape('rectangle');
      // Set a random color for the annotations.
      $('input[name="color"]', $dialog).attr('value', getRandomColour());
      // Set up the color chooser.
      if (settings.canChooseColour) {
        // Destroy the miniColor
        $('input[name="color"]', $dialog).miniColors('destroy');
        $('.color-picker').miniColors();
      }
      // In edit mode we can only see some fields.
      if (edit) {
        $('#islandora-image-annotation-dialog-icons').hide();
        $('#islandora-image-annotation-dialog-entity-type').hide();
        $('#islandora-image-annotation-dialog-color').hide();
        $('#islandora-image-annotation-dialog-stroke').hide();
      } else {
        $('#islandora-image-annotation-dialog-icons').show();
        $('#islandora-image-annotation-dialog-entity-type').show();
        $('#islandora-image-annotation-dialog-color').show();
        $('#islandora-image-annotation-dialog-stroke').show();
      }
    }

    /**
     * Used as a callback to get the display properties of the annotation.
     *
     * @returns {object}
     */
    function getAnnotationProperties() {
      var values = getFormValues();
      return {
        shape: values.shape,
        attributes: {
          stroke: values.color,
          'stroke-width': values.stroke
        }
      };
    }

    /**
     * Shows the Create Annotation Dialog.
     *
     * Used for creating new annotations.
     */
    function showCreateDialog() {
      $dialog.dialog($.extend(defaultDialogProperties, {
        title: Drupal.t('Annotate'),
        open: function () {
          var canvas = Drupal.IslandoraImageAnnotationCanvas.getInstance();
          // Clear the form to be safe.
          clearForm();
          // If we aren't editing an existing annotation we must be creating a
          // new one, so prepare the canvas.
          canvas.startAnnotating(canvas.getCurrentCanvas(), getAnnotationProperties);
        },
        close: function () {
          // Stop all annotations.
          var canvas = Drupal.IslandoraImageAnnotationCanvas.getInstance();
          canvas.stopAnnotating(canvas.getCurrentCanvas());
          // Reset to defaults.
          clearForm();
        },
        buttons: [{
          text: 'Save',
          // Assumes only one canvas with a valid 'canvas' attribute.
          click: function () {
            var values = getFormValues();

            // Minimally we only allow users to create content if they have
            // entered a title and annotation.
            if (!values.text || !values.title) {
              alert('An annotation needs both title and content');
              return 0;
            }
            // Also the user must have actually marked up the image.
            if (values.canvas === null) {
              alert('You must draw a shape around the target.');
              return 0;
            }

            // Set default type if not specified.
            values.type = (values.type === '' || values.type === null) ? Drupal.t('unclassified') : values.type;

            // @todo Entity Linking!

            // Create RDFa representing the current annotation, all the
            // parameters are required. Except entityID, entityLabel, it also
            // generates identifiers for the annotation and it's content.
            ingestAnnotation(createAnnotation(values));
            $dialog.dialog('close');
          }
        }, {
          text: 'Cancel',
          click: function () {
            $dialog.dialog('close');
          }
        }]
      }));
    }

    /**
     * Shows the Create Annotation Dialog.
     *
     * Used for creating new annotations.
     */
    function showEditDialog(annotation) {
      $dialog.dialog($.extend(defaultDialogProperties, {
        title: Drupal.t('Update Annotation'),
        height: 550,
        open: function () {
          // Clear the form to be safe.
          clearForm(true);
          // Populate the form with the given annotation.
          populateForm(annotation);
        },
        close: function () {
          // Reset to defaults.
          clearForm();
        },
        buttons: [{
          text: 'Save',
          // Assumes only one canvas with a valid 'canvas' attribute.
          click: function () {
            var values = getFormValues();

            // Minimally we only allow users to create content if they have
            // entered a title and annotation.
            if (!values.text || !values.title) {
              alert('An annotation needs both title and content');
              return 0;
            }
            // Also the user must have actually marked up the image.
            if (values.canvas === null) {
              alert('You must draw a shape around the target.');
              return 0;
            }

            // Set default type if not specified.
            values.type = (values.type === '' || values.type === null) ? Drupal.t('unclassified') : values.type;

            // Update the annotation.
            updateAnnotation(annotation, values);
            $dialog.dialog('close');
          }
        }, {
          text: 'Cancel',
          click: function () {
            $dialog.dialog('close');
          }
        }]
      }));
    }

    /**
     * Creates a dialog box and displays it.
     *
     * @param {RDF.Annotation} annotation
     */
    this.show = function (annotation) {
      if (annotation !== undefined) {
        showEditDialog(annotation);
      } else {
        showCreateDialog();
      }
    };

    // Link the Shapes to the the 'shape' hidden input field.
    $('.islandora-image-annotation-shape-icon', $dialog).click(function () {
      chooseShape($(this).attr('shape'));
    });
  };

  /**
   * Gets the global singleton of this class.
   *
   * This function is meant to be used after Drupal.attachBehaviors() has been
   * called, otherwise the singleton instance won't exist.
   *
   * @return {Drupal.IslandoraImageAnnotationDialog}
   *   The singleton
   */
  Drupal.IslandoraImageAnnotationDialog.getInstance = function () {
    return Drupal.IslandoraImageAnnotationDialog[base];
  };

}(jQuery));
