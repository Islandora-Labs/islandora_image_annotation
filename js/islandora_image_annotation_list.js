/*jslint browser: true*/
/*global jQuery, Drupal, IIAUtils*/
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
  var base = '#islandora-image-annotation-list';

  /**
   * Initialize the Tabs section of the Image annotation widget.
   *
   * We can only have one Tabs per page.
   */
  Drupal.behaviors.islandoraImageAnnotationList = {
    attach: function (context, settings) {
      $(base, context).once('islandoraImageAnnotationList', function () {
        Drupal.IslandoraImageAnnotationList[base] = new Drupal.IslandoraImageAnnotationList(base, settings.islandoraImageAnnotationList);
      });
    }
  };

  /**
   * Instantiate the Annotation List.
   *
   * @param {string} base
   *   The id of the html element in which bind the list to.
   *
   * @param {object} settings
   *   The Drupal.settings variable for this widget.
   *
   * @constructor
   */
  Drupal.IslandoraImageAnnotationList = function (base, settings) {

    /**
     * Gets the type container from the given type.
     *
     * @returns {jQuery}
     *   jQuery wrapper around the type container if found, otherwise an empty,
     *   jQuery wrapper.
     */
    function getAnnotationTypeContainer(type) {
      return $('.comment-type-container[type="' + type + '"]');
    }

    /**
     * The list of comment annotations are grouped by type.
     *
     * @returns {bool}
     *   True if the given type container exists false otherwise.
     */
    function hasAnnotationTypeContainer(type) {
      return getAnnotationTypeContainer(type).length !== 0;
    }

    /**
     * Creates a container in which to put annotations by the given type.
     *
     * @returns {jQuery}
     *   True if the given type container exists false otherwise.
     */
    function createAnnotationTypeContainer(type) {
      var $type, $title, $content;
      // Annotations are grouped by type.
      $type = $('<div />', {
        class: 'comment-type-container',
        type: type
      });
      $title = $('<div class="comment-type-title">' + type + '</div>');
      // Display the Content when the title is clicked.
      $title.click(function () {
        $('+ .comment-type-content', this).toggle();
      });
      // Each annotation is listed in the content section, which is initially
      // hidden.
      $content = $('<div />', {
        class: 'comment-type-content'
      }).hide();
      return $type.append($title, $content);
    }

    /**
     * Inserts the annotation container in alphabetical order.
     */
    function insertAnnotationTypeContainer($element) {
      var title, $containers;
      title = $('.comment-type-title', $element).text().toLowerCase();
      if (hasAnnotationTypeContainer(title)) {
        return getAnnotationTypeContainer(title);
      }
      $containers = $('.comment-type-container', base).filter(function () {
        return $('.comment-type-title', this).text().toLowerCase() > title;
      });
      if ($containers.length > 0) {
        $element.insertBefore($containers.get(0));
      } else {
        $(base).append($element);
      }
      return $element;
    }

    /**
     * Gets the annotation's list element given it's annotation id.
     *
     * @param {string} id
     *   The identifier for the annotation expected to be a UUID with the format
     *   "urn:"<NID>":"<NSS>.
     */
    function getAnnotationElement(id) {
      return $(".canvas-annotation[annotation='" + id + "']", base);
    }

    /**
     * Toggles the display of the annotation identified by the given id.
     *
     * Shows the comment text section of the annotation and the annotation color
     * icon, also displays the annotation shapes on the canvas.
     *
     * @param {string} id
     *   The identifier for the annotation.
     */
    function showAnnotation(id) {
      var $annotation, canvas;
      $annotation = getAnnotationElement(id);
      $('.comment-text, span.color', $annotation).show();
      $('.comment-show-hide', $annotation).text('-');
      $('.comment-title', $annotation).addClass('annotation-opened');
      canvas = Drupal.IslandoraImageAnnotationCanvas.getInstance();
      canvas.drawAnnotation(Drupal.IslandoraImageAnnotation.getInstance().getAnnotation(id));
    }

    /**
     * Hides the given annotation from view.
     *
     * @param {string} id
     *   The identifier for the annotation.
     */
    function hideAnnotation(id) {
      var $annotation, canvas;
      $annotation = getAnnotationElement(id);
      $('.comment-text, span.color', $annotation).hide();
      $('.comment-show-hide', $annotation).text('+');
      $('.comment-title', $annotation).removeClass('annotation-opened');
      canvas = Drupal.IslandoraImageAnnotationCanvas.getInstance();
      canvas.hideAnnotation(Drupal.IslandoraImageAnnotation.getInstance().getAnnotation(id));
    }

    /**
     * Toggles the display of the annotation identified by the given urn.
     *
     * @param id
     *   The id of the annotation to display.
     */
    function toggleAnnotationDisplay(id) {
      var $annotation = getAnnotationElement(id);
      $('.comment-text, span.color', $annotation).toggle();
      $('.comment-title', $annotation).toggleClass('annotation-opened');
      if ($('span.color', $annotation).is(':visible')) {
        $('.comment-show-hide', $annotation).text('-');
        showAnnotation(id);
      } else {
        $('.comment-show-hide', $annotation).text('+');
        hideAnnotation(id);
      }
    }

    /**
     * Gets the color of the given annotation.
     *
     * @param {RDF.BodyTarget} target
     *   The target to get the color from.
     *
     * @returns {string|null}
     *   The color in #ffffff format if found, otherwise null.
     */
    function getAnnotationTargetColor(target) {
      if (target.constraint !== undefined && target.constraint.value !== undefined) {
        return $(target.constraint.value).attr('stroke');
      }
      return null;
    }

    /**
     * Creates an Annotation element to insert into the list.
     *
     * @param {RDF.Annotation} annotation
     *   The annotation in which we will render into HTML.
     *
     * @returns {jQuery}
     *   The HTML as a jQuery object.
     */
    function createAnnotation(annotation) {
      var $annotation, $annotationTitle, $annotationText, canvas;
      canvas = annotation.getTargetCanvas();
      // Create annotation block.
      $annotation = $('<div />', {
        annotation: annotation.id,
        // Make the assumption that only one canvas will ever exist, and we'll
        // always have at least one target.
        canvas: canvas && canvas.id,
        class: 'canvas-annotation'
      });
      // Add Title, including the color icon.
      $annotationTitle = $('<div />', {
        class: 'comment-title'
      }).append('<span class="comment-show-hide">+</span>');
      $annotationTitle.append('<span>' + annotation.title + '</span>');
      // Append Color Icons
      $.each(annotation.targets, function (index, target) {
        var color = getAnnotationTargetColor(target);
        $annotationTitle.append($('<span />', {
          color: color,
          class: 'color',
          style: 'background:' + color + ';'
        }).hide());
      });
      // Display the annotation when the title is clicked.
      $annotationTitle.click(function () {
        toggleAnnotationDisplay(annotation.id);
      });
      // Add Text, initially hidden.
      $annotationText = $('<div class="comment-text" />').hide();
      $annotationText.append('<div class="comment-type">' + annotation.annotationType + '</div>');
      $annotationText.append('<div class="comment-content">' + annotation.body.value + '</div>');
      $annotation = $annotation.append($annotationTitle, $annotationText);
      return $annotation;
    }

    /**
     * Adds the given annotation to the list.
     *
     * @param {RDF.Annotation} annotation
     *   The annotation in to add to the list.
     */
    function addAnnotation(annotation) {
      var type, $container;
      type = annotation.annotationType;
      if (hasAnnotationTypeContainer(type)) {
        $container = getAnnotationTypeContainer(type);
      } else {
        $container = insertAnnotationTypeContainer(createAnnotationTypeContainer(type));
      }
      $('.comment-type-content', $container).append(createAnnotation(annotation));
    }

    /**
     * Deletes the given Annotation from Fedora and the interface.
     *
     * @param {string} id
     *   The id of the annotation.
     */
    function deleteAnnotation(id) {
      $.ajax({
        type: 'POST',
        url: IIAUtils.url('islandora/object/' + settings.pid + '/annotation/delete/' + id),
        success: function () {
          Drupal.IslandoraImageAnnotation.getInstance().deleteAnnotation(id);
        }
      });
    }

    // Create the comment type containers for the comment annotations.
    // trigger a load of each comment annotation.
    $.each(settings.annotations, function (pid, annotation) {
      insertAnnotationTypeContainer(createAnnotationTypeContainer(annotation.type));
    });

    // Fetch the comment annotations only after we've processed the canvas
    // they belong to.
    Drupal.IslandoraImageAnnotation.on('processedSequence', function (event, annotation) {
      // Create the comment type containers for the comment annotations.
      // trigger a load of each comment annotation.
      $.each(settings.annotations, function (pid, annotation) {
        var url = 'annotation/get/' + pid;
        Drupal.IslandoraImageAnnotation.getInstance().fetchTriples(url);
      });
    });

    // Add any processed comment annotations to the list.
    Drupal.IslandoraImageAnnotation.on('processedAnnotation', function (event, annotation) {
      if (annotation.getType() === 'comment') {
        addAnnotation(annotation);
      }
    });

    // Listen for removed 'comment' annotations and update th list accordingly.
    Drupal.IslandoraImageAnnotation.on('deleteAnnotation', function (event, annotation) {
      var $annotation, $content;
      if (annotation.getType() === 'comment') {
        $annotation = getAnnotationElement(annotation.id);
        $content = $annotation.parent();
        // Remove the annotation from the list.
        $annotation.remove();
        // If there are no annotations of this type left anymore remove the
        // type, group from the list.
        if ($content.children().length === 0) {
          $content.parent().remove();
        }
      }
    });

    // Set up context menu to display for each annotation title element.
    function createContextMenu() {
      $.contextMenu({
        selector : '.comment-title',
        items: {
          edit: {
            name: 'Edit',
            icon: 'edit'
          },
          delete: {
            name : 'Delete annotation',
            icon : 'delete'
          }
        },
        callback : function (key) {
          var $title, $annotation, id, annotation, title;
          $title = $(this);
          $annotation = $title.parent('.canvas-annotation');
          id = $annotation.attr('annotation');
          title = $('span:nth-child(2)', $title).text().trim();

          switch (key) {
          case 'edit':
            showAnnotation(id);
            annotation = Drupal.IslandoraImageAnnotation.getInstance().getAnnotation(id);
            Drupal.IslandoraImageAnnotationDialog.getInstance().show(annotation);
            break;

          case 'delete':
            if (confirm("Permanently Delete Annotation '" + title + "'")) {
              deleteAnnotation(id);
            }
            break;
          }
        }
      });
    }
    // Only initialize the context menu if the user is able to edit the object.
    if (settings.editable) {
      createContextMenu();
    }
  };

  /**
   * Gets the global singleton of this class.
   *
   * This function is meant to be used after Drupal.attachBehaviors() has been
   * called, otherwise the singleton instance won't exist.
   *
   * @return {Drupal.IslandoraImageAnnotationList}
   *   The singleton
   */
  Drupal.IslandoraImageAnnotationList.getInstance = function () {
    return Drupal.IslandoraImageAnnotationList[base];
  };

}(jQuery));
