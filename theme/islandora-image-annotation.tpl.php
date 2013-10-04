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
         <?php print $anno_list_pane; ?>
            </div>
         </div>
       </div>
       <div id="colright" class="colright">

        <!-- Column Separator -->
        <!--<div id="column-separator"></div>-->
        <div class="col1">
          <button id="create_annotation" class="menu_button">Annotate</button>
          <div class="image-annotation-wrapper">
            <!-- Persist a single player and build new interface to it -->
            <div id="canvas-body-wrapper">
              <?php print $anno_img_pane; ?>
            </div>
          </div>
        </div>
      </div>
    </div>
    <!-- Footer -->
    <div id="islandora_shared_canvas_footer">
    </div>
  </div>
</div>
