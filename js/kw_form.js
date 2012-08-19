Drupal.behaviors.kWeb = {attach: function(context) {(function ($) {
  var custom_to_fixed = function (value, precision) {
    var power = Math.pow(10, precision || 0);
    return String(Math.round(value * power) / power);
  };
  // Numeric widget functions
  var nmw_change_handler = function($input, modifier) {
    input_value = $input.val().replace(",", ".");
    if (input_value == "" && modifier == 0) {return;}
    input_value = input_value != parseFloat(input_value) ? 0 : parseFloat(input_value);
    new_value = input_value + modifier;
    $container = $input.parents(".numeric-widget-instance");
    $container.find(".numeric-widget-button").removeClass("disabled");
    source_limits = $input.data("limits");
    if(source_limits["max"] != null && new_value >= source_limits["max"]) {
      new_value = source_limits["max"];
      $container.find(".nmw-plus").addClass("disabled");
    }
    else if(source_limits["min"] != null && new_value <= source_limits["min"]) {
      new_value = source_limits["min"];
      $container.find(".nmw-minus").addClass("disabled");
    }
    $input.val(parseFloat(custom_to_fixed(new_value,10)));
  };
  //Numeric widget init
  $("div.field-widget-number.numeric-widget").find("input.form-text").addClass("numeric-widget").addClass("container-info");
  $("input.numeric-widget").not(".nmw-processed").addClass("nmw-processed").each(function() {
    steps = new Array;
    min = null;
    max = null;
    params = null;
    classes = $(this).filter(".container-info").length > 0 ? $(this).parents("div.numeric-widget").attr("class").split(" ") : $(this).attr("class").split(" ");
    for(j=0; j < classes.length; j++) {
      if(classes[j].substr(0,5) == "step-") {
        steps[steps.length] = parseFloat(classes[j].substr(5));}
      else if(classes[j].substr(0,5) == "kwwp-") {
        params = jQuery.parseJSON($("#" + classes[j]).val());}
      else if(classes[j].substr(0,4) == "max-") {
        max = parseFloat(classes[j].substr(4));}
      else if(classes[j].substr(0,4) == "min-") {
        min = parseFloat(classes[j].substr(4));}
    }
    if(steps.length == 0) {
      steps[0] = 1;
    }
    hBefore = "<td><table class='numeric-widget-left-controls'>";
    hAfter = "<td><table class='numeric-widget-right-controls'>";
    $(this).data("limits", {min: min, max: max});
    for(i = 0; i < steps.length; i++) {
      if(params != null && params.labels && params.labels[steps[i]]) {
        label = params.labels[steps[i]];}
      else {
        label = steps[i];}
      hBefore += "<tr class='no-hover'><td><input type='button' class='numeric-widget-button nmw-minus' value='-"+label+"' title='-"+steps[i]+"' /></td></tr>";
      hAfter += "<tr class='no-hover'><td><input type='button' class='numeric-widget-button nmw-plus' value='+"+label+"' title='+"+steps[i]+"' /></td></tr>";
    }
    hBefore += "</table></td>";
    hAfter += "</table></td>";
    $(this).wrap("<table class='numeric-widget-instance'><tr class='no-hover'></tr></table>").before(hBefore).after(hAfter).wrap("<td></td>");
    nmw_change_handler($(this), 0);
  }).change(function() {
    nmw_change_handler($(this), 0);
  });
  $("input[type=button].numeric-widget-button").not(".nmw-processed").addClass("nmw-processed").click(function() {
    nmw_change_handler($(this).parents(".numeric-widget-instance").find("input.numeric-widget"), parseFloat($(this).attr("title")));
  });
  // Multi-select widget functions
  var msw_build_items_object = function($area, type) {
    var items = new Array();
    $area.find(type).each(function(i) {
      items[i] = {itemLabel : $(this).parents('.form-item').find("label").html(), itemValue : $(this).attr("id")};
    });
    html = "";
    if(items.length > 0) {
      html += "<ul>";
      for(var i = 0; i < items.length; i++) {
        html += "<li><a href='#' rel='"+items[i]["itemValue"]+"'>"+items[i]["itemLabel"]+"</a></li>"
      }
      html += "</ul>";
    }
    return {size: items.length, html: html};
  };
  var msw_check_selectable_items = function($area) {
    $area1 = $area.find(".unchecked-items");
    $area1.has("a.selected").length > 0 ? $area.removeClass("without-selectables").addClass("with-selectables") : $area.addClass("without-selectables").removeClass("with-selectables");
    $area1.has("a").length > 0 ? $area.removeClass("without-unchecked").addClass("with-unchecked") : $area.addClass("without-unchecked").removeClass("with-unchecked");
    $area2 = $area.find(".checked-items");
    $area2.has("a.selected").length > 0 ? $area.removeClass("without-removables").addClass("with-removables") : $area.addClass("without-removables").removeClass("with-removables");
    $area2.has("a").length > 0 ? $area.removeClass("without-checked").addClass("with-checked") : $area.addClass("without-checked").removeClass("with-checked");
  };
  var msw_define_multiselect_commands = function () {
    $(".multiselect-table .items a").not(".msw-processed").addClass("msw-processed").keyup(function(e) {
      if(e.which == 32 || e.which == 16 || e.which == 17) {
        $(this).toggleClass("selected");
        msw_check_selectable_items($(this).parents(".multiselect-table"));
        e.preventDefault();
      }
      else if(e.which == 13 || e.which == 46 || e.which == 8) {
        $rel = $("#"+$(this).attr("rel"));
        $rel.attr("checked", $rel.is(":checked") ? "" : "checked");
        $parents = $(this).parents(".multiselect-table");
        msw_rebuild_items_list($parents.siblings(".multiselect-widget"));
        msw_check_selectable_items($parents);
        e.preventDefault();
      }
      else if (e.which == 40 || e.which == 39) {
        $(this).removeClass("selected").parents("li").next("li").find("a").addClass("selected").focus();
        msw_check_selectable_items($(this).parents(".multiselect-table"));
        e.preventDefault();
      }
      else if (e.which == 37 || e.which == 38) {
        $(this).removeClass("selected").parents("li").prev("li").find("a").addClass("selected").focus();
        msw_check_selectable_items($(this).parents(".multiselect-table"));
        e.preventDefault();
      }
    }).click(function() {
      $(this).toggleClass("selected");
      msw_check_selectable_items($(this).parents(".multiselect-table"));
      return false;
    }).dblclick(function() {
      $rel = $("#"+$(this).attr("rel"));
      $rel.attr("checked", $rel.is(":checked") ? "" : "checked");
      $parents = $(this).parents(".multiselect-table");
      msw_rebuild_items_list($parents.siblings(".multiselect-widget"));
      msw_check_selectable_items($parents);
      return false;
    });
    $(".multiselect-table .select-one").not(".msw-processed").addClass("msw-processed").click(function() {
      $parents = $(this).parents(".multiselect-table");
      $parents.find(".unchecked-items a.selected").removeClass("selected").each(function(){
        $("#"+$(this).attr("rel")).attr("checked", "checked");
      });
      msw_rebuild_items_list($parents.siblings(".multiselect-widget"));
      msw_check_selectable_items($parents);
      return false;
    });
    $(".multiselect-table .deselect-one").not(".msw-processed").addClass("msw-processed").click(function() {
        $parents = $(this).parents(".multiselect-table");
        $parents.find(".checked-items a.selected").removeClass("selected").each(function(){
          $("#"+$(this).attr("rel")).attr("checked", "");
        });
        msw_rebuild_items_list($parents.siblings(".multiselect-widget"));
        msw_check_selectable_items($parents);
        return false;
      });
    $(".multiselect-table .select-all").not(".msw-processed").addClass("msw-processed").click(function() {
      $parents = $(this).parents(".multiselect-table");
      $parents.find(".unchecked-items a").each(function(){
        $("#"+$(this).attr("rel")).attr("checked", "checked");
      });
      msw_rebuild_items_list($parents.siblings(".multiselect-widget"));
      msw_check_selectable_items($parents);
      return false;
    });
    $(".multiselect-table .deselect-all").not(".msw-processed").addClass("msw-processed").click(function() {
      $parents = $(this).parents(".multiselect-table");
      $parents.find(".checked-items a").each(function(){
        $("#"+$(this).attr("rel")).attr("checked", "");
      });
      msw_rebuild_items_list($parents.siblings(".multiselect-widget"));
      msw_check_selectable_items($parents);
      return false;
    });
  };
  var msw_rebuild_items_list = function($widget) {
    checked = msw_build_items_object($widget, "input[type=checkbox]:checked");
    unchecked = msw_build_items_object($widget, "input[type=checkbox]:not(:checked)");
    $parents.find(".unchecked-size").html(unchecked["size"]);
    $parents.find(".checked-size").html(checked["size"]);
    $parents.find(".unchecked-items").html(unchecked["html"]);
    $parents.find(".checked-items").html(checked["html"]);
    msw_define_multiselect_commands();
  };
  // Multi-select widget init
  $("div.form-checkboxes.multiselect-widget").not(".msw-processed").addClass("msw-processed").hide().each(function() {
    checked = msw_build_items_object($(this), "input[type=checkbox]:checked");
    unchecked = msw_build_items_object($(this), "input[type=checkbox]:not(:checked)");
    $(this).before("<table class='multiselect-table without-selectables without-removables"
      +(checked["size"] > 0 ? "" : " without-checked")
      +(unchecked["size"] > 0 ? "" : " without-unchecked")
      +"'><thead><tr>"
      +"<th scope='col'>"+Drupal.t("Unselected items")+" (<span class='unchecked-size'>"+ unchecked["size"] +"</span>)</th>"
      +"<th></th>"
      +"<th scope='col'>"+Drupal.t("Selected items")+" (<span class='checked-size'>"+ checked["size"] +"</span>)</th>"
      +"</tr></thead><tbody><tr class='odd no-hover'>"
      +"<td class='items'><div class='unchecked-items'>"+ unchecked["html"] +"</div></td>"
      +"<td class='actions'>"
        +"<div><input type='submit' class='control select-all' value=\""+Drupal.t("Transfer all")+"\" /><div>"
        +"<div><input type='submit' class='control select-one' value=\""+Drupal.t("Transfer selection")+"\" /><div>"
        +"<div><input type='submit' class='control deselect-one' value=\""+Drupal.t("Remove selection")+"\" /><div>"
        +"<div><input type='submit' class='control deselect-all' value=\""+Drupal.t("Remove all")+"\" /><div>"
      +"</td>"
      +"<td class='items'><div class='checked-items'>"+ checked["html"] +"</div></td>"
      +"<tr><tbody></table>");
  });
  msw_define_multiselect_commands();
  // Moving tabledrag toogle weight wrapper control
  $(".form-item .tabledrag-toggle-weight-wrapper").each(function() {
    $(this).insertAfter($(this).parents(".form-item").find(".field-multiple-table"));
  });
  // Datepicker
  $("input.datepickable, input.datetimepickable").each(function() {
    settings_string = $(this).data('settings');
    settings = jQuery.parseJSON(settings_string.replace(/\\/g, ''));
    if ($(this).hasClass('datepickable')) {
      $(this).datepicker(settings);
    }
    else {
      settings.timeOnlyTitle = Drupal.t('Choose Time');
      settings.timeText = Drupal.t('Time');
      settings.hourText = Drupal.t('Hour');
      settings.minuteText = Drupal.t('Minute');
      settings.secondText = Drupal.t('Second');
      settings.millisecText = Drupal.t('Millisecond');
      settings.currentText = Drupal.t('Now');
      settings.closeText = Drupal.t('Done');
      settings.ampm = false;
      $(this).datetimepicker(settings);
    }
  });
})(jQuery);}};