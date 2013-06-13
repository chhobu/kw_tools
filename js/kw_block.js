Drupal.behaviors.kBlock = {attach: function(context, settings) {(function ($) {
  var getCookieData = function () {
    if ($.cookie) {
      var cookieString = $.cookie('collapsiblock');
      return cookieString ? $.parseJSON(cookieString) : {};
    }
    else {
      return '';
    }
  };
  $('.block.collapsible').not('.collapsible-processed').each(function() {
    $title = $(this).find('h2');
    blockId = $(this).attr('id');
    if ($title) {
      $title.wrapInner('<a href="#" class="collapse-link icon">');
    }
    $(this).addClass('collapsible-processed');
    items = getCookieData();
    collapse = 0;
    if (items[blockId] == 0) {
      $(this).addClass('collapsed').removeClass('expanded');
      collapse = 1;
    }
    else if (items[blockId] == 1) {
      $(this).removeClass('collapsed').addClass('expanded');
    }
    else if ($(this).hasClass('collapsed')) {
      collapse = 1;
    }
    if (collapse) {
      $(this).find('.content').hide();
    }
  });
  $('.block.collapsible .collapse-link').not('.collapsible-processed').addClass('collapsible-processed').click(function() {
    $block = $(this).parents('.block');
    $content = $block.find('.content').removeClass('animated').stop();
    if ($block.hasClass('expanded')) {
      blockStatus = 0;
      $content.removeAttr('style').addClass('animated').slideUp('fast', function() {
        $(this).removeClass('animated');
        $block.addClass('collapsed').removeClass('expanded');
      });
    }
    else {
    	blockStatus = 1;
      $block.removeClass('collapsed').addClass('expanded');
      $content.addClass('animated').slideDown('fast', function() {
        $(this).removeClass('animated');
      });
    }
    items = getCookieData();
    items[$block.attr('id')] = blockStatus;
    var cookieString = '{ ';
    var cookieParts = [];
    $.each(items, function(id, setting) {
      cookieParts[cookieParts.length] = ' "' + id + '": ' + setting;
    });
    cookieString += cookieParts.join(', ') + ' }';
    $.cookie('collapsiblock', cookieString, {path: settings.basePath});
    return false;
  });
})(jQuery);}};
