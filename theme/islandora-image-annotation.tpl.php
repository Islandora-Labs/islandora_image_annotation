<?php
/**
 * @file
 * islandora-image-annotation.tpl.php
 */
?>

<div class="islandora-anno-wrapper">
  <input id="full-window-button" type="button" value="<?php print t('Full Window'); ?>" />
  <div class="islandora-anno-iframe-wrapper">
    <iframe id="islandora-anno-iframe" src="<?php print $islandora_image_annotation_src; ?>" scrolling="0" frameborder="0" style="width: 100%; height: 800px;"><?php print t('Errors: unable to load SharedCanavas'); ?></iframe>
  </div>
</div>
