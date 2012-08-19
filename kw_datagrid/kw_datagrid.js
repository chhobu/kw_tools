(function ($) {
  Drupal.behaviors.KwDatagrid = {attach: function(context, settings) {
    $('.kw-datagrid').find('.refresh-link').hide().parent().addClass('processing');
    $('.kw-datagrid', context).once('KwDatagrid', function() {
      var gridId = $(this).attr('id');
      var $refreshLink = $(this).find('.refresh-link');
      if ($refreshLink.length > 0 && $(this).not('.KwDatagrid-processed')) {
        var refreshDelay = $refreshLink.data('refresh');
        if (refreshDelay >= 1) {
          var refreshDatagrid = setInterval(function() {
            $('#' + gridId).find('.refresh-link').show().html(Drupal.t('Refreshing...')).click();
          }, refreshDelay);
        }
      }
    });
  }}
})(jQuery);