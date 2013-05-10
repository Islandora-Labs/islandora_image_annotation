<?php
/**
 * @file
 *   islandora-image-annotation.tpl.php
 */

?>

<div class="islandora-anno-wrapper">

  <input id="full-window-button" type="button" value="<?php print t('Full Window'); ?>" />
  <div class="islandora-image-annotation-wrapper">

    <!-- Header -->
    <div id="islandora_shared_canvas_header">
    </div>
    <!-- Body -->
      <div class="colmask threecol">
        <div class="colleft">
          <div class="col2">
            <!-- Tabs -->
            <div id="tabs">

              <ul>
                <li id="annotation_tab"><a href="#image-annotations">Image Annotations</a></li>
              </ul>

              <!-- Image Annotations Panel -->
              <div id="image-annotations">
                <div id="comment_annos_block" class="dragBlock"></div>
              </div>
            </div>
          </div>
        </div>
        <div id="colright" class="colright">

          <!-- Column Separator -->
          <!--<div id="column-separator"></div>-->
          <div class="col1">
            <!-- Image annotation -->
            <button id="create_annotation" class="menu_button">Annotate</button>
            <div class="image-annotation-wrapper">

              <!-- Persist a single player and build new interface to it -->
              <div id="canvas-body-wrapper">
                <div id="canvas-body">

                  <!--  Wrapper to create Canvas divs in -->
                  <div id="canvases"></div>

                  <!--  Wrapper to create SVG divs in -->
                  <div id="svg_wrapper"></div>

                  <!--  Wrapper to create annotations in, then reposition -->
                  <div id="annotations"></div>

                  <!-- Progress bar -->
                  <div id="loadprogress"></div>

                </div>
              </div>
            </div>
          </div>
        </div>
      </div>

    <div id="dialogs" style="width: 500px">

      <!-- Image annotation box -->
      <div id="create_annotation_box" style="width: 380px" class="dragBox ui-dialog ui-widget ui-corner-all ui-draggable ui-resizable">
        <div id="create_annos_header" class="dragHead ui-dialog-titlebar ui-widget-header ui-corner-all">
          <span>Annotate</span>
        </div>

        <!-- Annotation shapes -->
        <div style="display:inline; margin-top: 3px; padding-left: 5px;">
          <img id="annoShape_rect" class="annoShape" src="<?php print $images_path; ?>/draw_rect.png" style="padding-left: 2px; padding-top: 1px;"/>
          <img id="annoShape_circ" class="annoShape" src="<?php print $images_path; ?>/draw_circ.png" style="padding-left: 1px;"/>
          <img id="annoShape_poly" class="annoShape" src="<?php print $images_path; ?>/draw_poly.png" style="padding: 2px;"/>
          <hr style="margin: 0px; padding: 0px; height: 1px;"/>
        </div>
        <div id="create_annos_block" class="dragBlock">

          <!--Annotation Classification -->
          <div id ="islandora_classification"class="element-wrap">
            <label for="anno_classification">Type:</label>
            <input id="anno_classification" type="text" size="28"></input>
          </div>

          <!-- Annotation Title -->
          <div class="element-wrap" id ="islandora_titles">
            <label for="anno_title">Title:</label>
            <input id="anno_title" type="text" size="28"></input>
          </div>


          <div id ="color-picker-wrapper" class="element-wrap">
            <label for="anno_color">Color:</label>
            <input id ="anno_color" type="hidden" name="color4" value="#91843c" class="color-picker" size="7" />
            <input id ="anno_color_activated" type="hidden" value ="" size="7" />
          </div>

          <div id ="stroke-width-wrapper" class="element-wrap">
            <label for="stroke_width">Stroke Width:</label>
            <input id="stroke_width" type="text" size="5" value=".3"></input>
          </div>


          <div class="element-wrap">
            <label for="anno_text">Annotation:</label>
            <textarea id="anno_text" cols="40" rows="5"></textarea>
          </div>
          <!-- Services - to be removed -->
          <span style="width:200px;margin:0px;padding:0px;float:left">
            <ul id="create_body" style="width: 200px; list-style:none;font-size:10pt;margin:0;padding:0;">
            </ul>
          </span>
          <!-- Cancel/Save buttons -->
          <span style="float: right; padding-top: 3px;">
            <button class="diabutton" id="cancelAnno">Cancel</button>
            <button class="diabutton" id="saveAnno">Save</button>
          </span>
        </div>
      </div>
    </div>

    <!-- Footer -->
    <div id="islandora_shared_canvas_footer">
      <div class="shared-canvas-logo" style="font-size:8pt">
        <img height="15" src="<?php print $images_path; ?>/small-logo.png" style="padding: 0px; margin: 0px; border: 0px; border-top: 2px;" />
        Powered by SharedCanvas
      </div>
    </div>
  </div>
</div>
