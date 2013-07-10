/**
 * Produces a jQuery dialog box, used for
 * creating an image annotations in the
 * image annotation module and the critical
 * edition module.
 * @returns DialogBox(jQuery)
 */
function annotation_dialog() {
  var img_base_path = Drupal.settings.islandora_image_annotation.images_path;
  $(document.body).append(''+
 '<div id="create_annotation_box">'+
    '<div style="display:inline; margin-top: 3px; padding-left: 5px;">'+
      '<img id="annoShape_rect" class="annoShape" src="' + img_base_path + '/draw_rect.png" style="padding-left: 2px; padding-top: 1px;"/>'+
      '<img id="annoShape_circ" class="annoShape" src="' + img_base_path + '/draw_circ.png" style="padding-left: 1px;"/>'+
      '<img id="annoShape_poly" class="annoShape" src="' + img_base_path + '/draw_poly.png" style="padding: 2px;"/>'+
      '<hr style="margin: 0px; padding: 0px; height: 1px;"/>'+
    '</div>'+
    '<div id="create_annos_block" class="dragBlock">'+
      '<div class="element-wrap">'+
        '<label for="anno_title">Title:</label>'+
        '<input id="anno_title" type="text" size="28"></input>'+
      '</div>'+
      '<div id ="islandora_classification"class="element-wrap">'+
        '<label for="anno_type">Type:</label>'+
        '<input id="anno_classification" type="text" size="28"></input>'+
      '</div>'+
      '<div id ="color-picker-wrapper" class="element-wrap">'+
        '<label for="anno_color">Color:</label>'+
        '<input id ="anno_color" type="hidden" name="color4" value="#91843c" class="color-picker" size="7" />'+
        '<input id ="anno_color_activated" type="hidden" value ="" size="7" />'+
      '</div>'+
      '<div id ="stroke-width-wrapper" class="element-wrap">'+
        '<label for="stroke_width">Stroke Width:</label>'+
        '<input id="stroke_width" type="text" size="5" value=".3"></input>'+
      '</div>'+
      '<div class="element-wrap">'+
        '<label for="anno_text">Annotation:</label>'+
        '<textarea id="anno_text" cols="40" rows="5"></textarea>'+
      '</div>'+
      '<span style="width:200px;margin:0px;padding:0px;float:left">'+
        '<ul id="create_body" style="width: 200px; list-style:none;font-size:10pt;margin:0;padding:0;">'+
        '</ul>'+
      '</span>'+
    '</div>'+
  '</div>');
  var annotation_dialog = $('#create_annotation_box');
  return annotation_dialog.dialog({
    modal: true,
    title: 'Annotate',
    resizable: false,
    closeOnEscape: false,
    height: 470,
    width: 380,
    buttons: {
    'Save': function() {
      saveAndEndAnnotating();
      annotation_dialog.dialog('close');
      closeAndEndAnnotating();
    },
    'Cancel': function() {
      closeAndEndAnnotating();
      annotation_dialog.dialog('close');
    }
  }
  });
};

