// Toggle full window shared canvas.
(function ($) {
  Drupal.behaviors.islandorAnnoFullWindow = {
    attach: function (context, settings) {
      $('#full-window-button').click(function() {

        $('.islandora-anno-wrapper').toggleClass('islandora-anno-fullwindow');
        resizeCanvas();

        if ($(this).val() == Drupal.t('Full Window')) {
          $(this).val(Drupal.t('Exit Full Window'));
        }
        else {
          $(this).val(Drupal.t('Full Window'));
        }
      });
    }
  };
})(jQuery);
