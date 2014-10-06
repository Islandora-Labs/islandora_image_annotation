<?php

/**
 * @file
 * The dialog box used to create new annotations.
 */
?>
<div id="islandora-image-annotation-dialog" style="display: none;">
  <div id="islandora-image-annotation-dialog-icons">
    <?php print $annotation_icons; ?>
  </div>
  <hr/>
  <label>Title:</label>
  <input name="title" type="text" size="28" />
  <label>Type:</label>
  <input name="type" type="text" size="28" />
  <?php if ($use_entity_tagging): ?>
    <div id="islandora-image-annotation-dialog-entity-type">
      <label>Entity</label>
      <select class="easyui-combobox" name="entity_type" style="width:100px;">
        <option value="person"><?php print t('Tag Person'); ?></option>
        <option value="event"><?php print t('Tag Event'); ?></option>
        <option value="place"><?php print t('Tag Place'); ?></option>
        <option value="organization"><?php print t('Tag Organization'); ?></option>
      </select>
      <input id="islandora-image-annotation-dialog-entity" name="entity"/>
    </div>
  <?php endif; ?>
  <?php if ($can_choose_colour): ?>
    <div id ="islandora-image-annotation-dialog-color">
      <label>Color:</label>
      <input name="color" type="hidden" value="#91843c" class="color-picker" size="7" />
    </div>
  <?php endif; ?>
  <div id="islandora-image-annotation-dialog-stroke">
    <label>Stroke Width:</label>
    <input name="stroke" type="text" size="5" value=".3" />
  </div>
  <label>Annotation:</label>
  <textarea name="text" cols="40" rows="5"></textarea>
  <!-- The Canvas this annotation belongs to. -->
  <input name="canvas" type="hidden" />
  <!-- The Shape to use. -->
  <input name="shape" type="hidden" />
</div>