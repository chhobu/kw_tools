<?php
function theme_datagrid($variables) {
  /* @var $datagrid  KwDatagrid */
  $datagrid = $variables['datagrid'];
  $rows = $variables['rows'];
  $header = $datagrid->getHeaders();
  $attributes = $datagrid->getAttributes();
  $caption = $datagrid->getCaption();
  $colgroups = $datagrid->getColgroup();
  $sticky = $datagrid->isUsingSticky();
  $ajax = $datagrid->isUsingAjax();
  $empty = $datagrid->getEmptyText();
  $columns = $datagrid->getColumns();
  $refresh = $datagrid->hasOp('partial-refresh');
  drupal_add_css(drupal_get_path("module", "kw_datagrid") . "/kw_datagrid.css",
      array('every_page' => TRUE));
  if($ajax) {
    drupal_add_js('misc/ajax.js');
    drupal_add_js('misc/jquery.form.js');
  }
  if (count($header) && $sticky) {
    drupal_add_js('misc/tableheader.js');
    $attributes['class'][] = 'sticky-enabled';
  }
  $attributes["class"][] = "datagrid";
  $output = '<table' . drupal_attributes($attributes) . ">\n";
  if (isset($caption)) {
    $output .= '<caption>' . $caption . "</caption>\n";
  }
  // Format the table columns:
  if (count($colgroups)) {
    foreach ($colgroups as $number => $colgroup) {
      $attributes = array();
      // Check if we're dealing with a simple or complex column
      if (isset($colgroup['data'])) {
        foreach ($colgroup as $key => $value) {
          if ($key == 'data') {
            $cols = $value;
          }
          else {
            $attributes[$key] = $value;
          }
        }
      }
      else {
        $cols = $colgroup;
      }
      // Build colgroup
      if (is_array($cols) && count($cols)) {
        $output .= ' <colgroup' . drupal_attributes($attributes) . '>';
        $i = 0;
        foreach ($cols as $col) {
          $output .= ' <col' . drupal_attributes($col) . ' />';
        }
        $output .= " </colgroup>\n";
      }
      else {
        $output .= ' <colgroup' . drupal_attributes($attributes) . " />\n";
      }
    }
  }
  // Add the 'empty' row message if available.
  $is_empty = FALSE;
  if (!count($rows) && $empty) {
    $is_empty = TRUE;
    $header_count = 0;
    foreach ($header as $header_cell) {
      if (is_array($header_cell)) {
        $header_count += isset($header_cell['colspan']) ? $header_cell['colspan'] : 1;
      }
      else {
        $header_count++;
      }
    }
    $rows[] = array(array('data' => $empty, 'colspan' => $header_count, 'class' => array('empty', 'message')));
  }
  $nb_cols = count(current($rows));
  // Format the table header:
  $active_header = NULL;
  if (count($header) && !$is_empty) {
    $head_output = "";
    $sort = $datagrid->getCurrentSort();
    if($sort > 0) {
      $aliases = $datagrid->getSortables("aliases");
      $ts['name'] = $aliases[0];
      $ts['sort'] = $sort > 0 ? "asc" : "desc";
    }
    $ts['query'] = $datagrid->transmitUrlParameters();
    $head_output .= '<tr>';
    foreach ($header as $column_key => $cell) {
      $cell = kw_datagrid_tablesort_header($datagrid, $column_key, $cell, $header, $ts);
      $cell["id"] = $datagrid->getId() . "-col-" . $column_key;
      $cell["scope"] = "col";
      if (isset($cell["class"]) && in_array("active",$cell["class"])) {
        $active_header = $column_key;
      }
      if (isset($columns[$column_key]["modifiable"]) && $columns[$column_key]["modifiable"]) {
        $title = strip_tags(t("Mass modification of !item data", array("!item" => strip_tags($cell["data"]))));
        $cell["data"] .= l("<img src='/misc/configure.png' alt=\"" . $title . "\" title=\"" . $title . "\" />",
          $datagrid->getConstructor('page'), array(
              "html" => TRUE,
              "query" => array("_id" => $datagrid->getId(), "mass_edit" => $column_key),
              "fragment" => $datagrid->getId(),
              "attributes" => array('rel' => 'nofollow')
          ));
      }
      $head_output .= _theme_table_cell($cell, TRUE);
    }
    // Using ternary operator to close the tags based on whether or not there are rows
    $head_output .= "</tr>";
    if ($head_output != "") {
      $output .= "<thead>" . $head_output . "</thead>";
    }
  }
  else {
    $ts = array();
  }
  // Format the table rows:
  if (count($rows)) {
    $output .= "<tbody>\n";
    $flip = array('even' => 'odd', 'odd' => 'even');
    $class = 'even';
    foreach ($rows as $number => $row) {
      $attributes = array();
      // Check if we're dealing with a simple or complex row
      if (isset($row['data'])) {
        foreach ($row as $key => $value) {
          if ($key == 'data') {
            $cells = $value;
          }
          else {
            $attributes[$key] = $value;
          }
        }
      }
      else {
        $cells = $row;
      }
      if (count($cells)) {
        // Add odd/even class
        $class = $flip[$class];
        $attributes['class'][] = $class;
        // Build row
        $output .= ' <tr' . drupal_attributes($attributes) . '>';
        $i = 0;
        foreach ($cells as $column_key => $cell) {
          if ($active_header == $column_key) {
            $cell["class"][] = "active";
          }
          $output .= _theme_table_cell($cell);
        }
        $output .= " </tr>\n";
      }
    }
    $output .= "</tbody>\n";
  }
  $footer_output_part_one = "";
  if ($datagrid->getPager() > 0) {
    $pager_output = theme("datagrid_pager", array("datagrid" => $datagrid));
    if (!empty($pager_output)) {
      $pager_output = str_replace(
        array('<h3>', '</h3>'),
        array('<div class="datagrid-pager-title">', '</div>'),
        $pager_output
      );
      $footer_output_part_one .= "<div class='datagrid-pager'>" . $pager_output . "</div>";
    }
  }
  $footer_output_part_two = '';
  $choices = $datagrid->getPagerOptions('choices');
  $parameters = $datagrid->transmitUrlParameters();
  if (($datagrid->getPagerOptions('display') == 'advanced' || $datagrid->getFormDisplay() == 'advanced')
      && count($choices) > 0) {
    $pg_parameters = $parameters;
    $items = array();
    $all = FALSE;
    sort($choices);
    foreach ($choices as $choice) {
      if ($choice == 0) {
        $all = TRUE;
        continue;
      }
      elseif ($choice >= $datagrid->getTotalResults() && $choice != $datagrid->getPager()) {
        continue;
      }
      if ($choice == $datagrid->getPager()) {
        $items[$choice] = array(
          'class' => array('pager-current'),
          'data' => $choice
        );
      }
      else {
        $pg_parameters['_g'] = $choice;
        $items[$choice] = array(
          'class' => array('pager-choices'),
          'data' => theme('datagrid_pager_link', array(
            'text' => $choice,
            'page_new' => $datagrid->getCurrentPage(),
            'datagrid' => $datagrid,
            'parameters' => $pg_parameters,
            'attributes' => array('class' => array('pager-item'), 'title' => t('Display !x results per page', array('!x' => $choice)))
          ))
        );
      }
    }
    if ($all && ($datagrid->getTotalResults() > $datagrid->getPager() || $datagrid->getPager() == 0)) {
      $pg_parameters['_g'] = 0;
      $items[0]['class'][] = 'pager-choices-all';
      if ($datagrid->getPager() == 0) {
        $items[0]['class'][] = 'pager-current';
        $items[0]['data'] = t('All');
      }
      else {
        $items[0]['data'] = theme('datagrid_pager_link', array(
          'text' => t('All'),
          'page_new' => $datagrid->getCurrentPage(),
          'datagrid' => $datagrid,
          'parameters' => $pg_parameters,
          'attributes' => array('class' => array('pager-item'))
        ));
      }
    }
    if (count($items) > 1) {
      $pager_choices = "<div class='datagrid-pager-choices datagrid-advanced-control'>" . theme('item_list', array(
        'items' => $items,
        'title' => t('Results per page :'),
        'type' => 'ul',
        'attributes' => array('class' => array('pager'))
      )) . "</div>";
      $footer_output_part_two .= str_replace(
        array('<h3>', '</h3>'),
        array('<div class="datagrid-pager-choices-title">', '</div>'),
        $pager_choices
      );
    }
  }
  if ($datagrid->getAutoRefresh() > 0) {
    drupal_add_js(drupal_get_path('module', 'kw_datagrid') . '/kw_datagrid.js');
    $footer_output_part_one .= '<div class="datagrid-bottom-links links">';
    $attributes['class'][] = 'refresh-link';
    $attributes['rel'] = 'nofollow';
    $attributes['data-refresh'] = $datagrid->getAutoRefresh() * 1000;
    if ($datagrid->isUsingAjax()) {
      $attributes["class"][] = "use-ajax";
    }
    $parameters['_op'] = 'refresh';
    $footer_output_part_one .= l(t('Refresh data'), $datagrid->getPath(), array(
        'query' => $parameters,
        'fragment' => $datagrid->getId(),
        'attributes' => $attributes
    ));
    $footer_output_part_one .= '</div>';
  }
  if (!empty($footer_output_part_one)) {
    $output .= "<tfoot><tr><td colspan='" . $nb_cols . "'>"
    . $footer_output_part_one
    . "</td></tr></tfoot>";
  }
  $output .= "</table>\n";
  if (!$refresh && $datagrid->getFormDisplay() != 'standard') {
    $footer_output_part_two .= theme('datagrid_advanced_controls', array('datagrid' => $datagrid));
  }
  if (!$refresh && !empty($footer_output_part_two)) {
    $output .= '<div class="datagrid-controls">' . $footer_output_part_two . '</div>';
  }
  return $output;
}

function kw_datagrid_tablesort_header(KwDatagrid $datagrid, $key, $cell, $header, $ts) {
  // Special formatting for the currently sorted column header.
  if (is_array($cell) && isset($cell['field'])) {
    $currentSort = $datagrid->getCurrentSort();
    $sortKey = $datagrid->findSortable($key);
    $columns = $datagrid->getColumns();
    $link_classes = array($datagrid->isUsingAjax() ? 'use-ajax' : 'standard', 'sortable', 'icon');
    if (abs($currentSort) == $sortKey) {
      $cell['class'][] = 'active';
      $link_classes[] = $currentSort > 0 ? "ascending" : "descending";
      if ($currentSort > 0) {
        $title = t('sort by @s descending', array('@s' => $columns[$key]['label']));
      }
    }
    if (!isset($title)) {
      $title = t('sort by @s ascending', array('@s' => $columns[$key]['label']));
    }
    $cell['data'] = l($cell['data'], $datagrid->getPath(),
      array(
        'attributes' => array(
          'rel' => 'nofollow',
          'title' => $title,
          'class' => $link_classes
        ),
        'query' => array_merge($ts['query'], array(
          '_s' => $sortKey == $currentSort ? -$sortKey : $sortKey,
          '_p' => 0
        )),
        'fragment' => $datagrid->getId(),
        'html' => TRUE
      )
    );
    unset($cell['field'], $cell['sort']);
  }
  return $cell;
}

function theme_datagrid_pager($variables) {
  /* @var $datagrid  KwDatagrid */
  $datagrid = $variables['datagrid'];
  if($datagrid->getPager() == 0) {
    return;
  }
  $pager_total = $datagrid->getTotalResults();
  $current_page = $datagrid->getCurrentPage();
  $parameters = $datagrid->transmitUrlParameters();
  $quantity = $datagrid->getPagerOptions('quantity');
  // Calculate various markers within this pager piece:
  // Middle is used to "center" pages around the current page.
  $pager_middle = ceil($quantity / 2);
  // current is the page we are currently paged to
  $pager_current = $current_page + 1;
  // first is the first page listed by this pager piece (re quantity)
  $pager_first = $pager_current - $pager_middle + 1;
  // last is the last page listed by this pager piece (re quantity)
  $pager_last = $pager_current + $quantity - $pager_middle;
  // max is the maximum page number
  $pager_max = ceil($pager_total / $datagrid->getPager());
  // End of marker calculations.
  // Prepare for generation loop.
  $i = $pager_first;
  if ($pager_last > $pager_max) {
    // Adjust "center" if at end of query.
    $i = $i + ($pager_max - $pager_last);
    $pager_last = $pager_max;
  }
  if ($i <= 0) {
    // Adjust "center" if at start of query.
    $pager_last = $pager_last + (1 - $i);
    $i = 1;
  }
  // End of generation loop preparation.
  if ($pager_max > 1) {
    $li_first = theme('datagrid_pager_first', array('text' => t('« first'), 'datagrid' => $datagrid, 'parameters' => $parameters));
    $li_previous = theme('datagrid_pager_previous', array('text' => t('‹ previous'), 'datagrid' => $datagrid, 'interval' => 1, 'parameters' => $parameters));
    $li_next = theme('datagrid_pager_next', array('text' => t('next ›'), 'datagrid' => $datagrid, 'interval' => 1, 'parameters' => $parameters));
    $li_last = theme('datagrid_pager_last', array('text' => t('last »'), 'datagrid' => $datagrid, 'parameters' => $parameters));
    if ($li_first) {
      $items[] = array(
        'class' => array('pager-first'),
        'data' => $li_first,
      );
    }
    if ($li_previous) {
      $items[] = array(
        'class' => array('pager-previous'),
        'data' => $li_previous,
      );
    }
    // When there is more than one page, create the pager list.
    if ($i != $pager_max) {
      if ($i > 1) {
        $items[] = array(
          'class' => array('pager-ellipsis'),
          'data' => '…',
        );
      }
      // Now generate the actual pager piece.
      for (; $i <= $pager_last && $i <= $pager_max; $i++) {
        if ($i < $pager_current) {
          $items[] = array(
            'class' => array('pager-item'),
            'data' => theme('datagrid_pager_previous', array('text' => $i, 'datagrid' => $datagrid, 'interval' => ($pager_current - $i), 'parameters' => $parameters)),
          );
        }
        if ($i == $pager_current) {
          $items[] = array(
            'class' => array('pager-current'),
            'data' => $i,
          );
        }
        if ($i > $pager_current) {
          $items[] = array(
            'class' => array('pager-item'),
            'data' => theme('datagrid_pager_next', array('text' => $i, 'datagrid' => $datagrid, 'interval' => ($i - $pager_current), 'parameters' => $parameters)),
          );
        }
      }
      if ($i < $pager_max) {
        $items[] = array(
          'class' => array('pager-ellipsis'),
          'data' => '…',
        );
      }
    }
    // End generation.
    if ($li_next) {
      $items[] = array(
        'class' => array('pager-next'),
        'data' => $li_next,
      );
    }
    if ($li_last) {
      $items[] = array(
        'class' => array('pager-last'),
        'data' => $li_last,
      );
    }
    return theme('item_list', array(
      'items' => $items,
      'title' => t('Pages :'),
      'type' => 'ul',
      'attributes' => array('class' => array('pager'))
    ));
  }
}

function theme_datagrid_pager_first($variables) {
  $text = $variables['text'];
  $datagrid = $variables['datagrid'];
  $parameters = $variables['parameters'];
  $output = '';
  // If we are anywhere but the first page
  if ($datagrid->getCurrentPage() > 0) {
    $output = theme('datagrid_pager_link', array(
      'text' => $text,
      'page_new' => 0,
      'datagrid' => $datagrid,
      'parameters' => $parameters));
  }
  return $output;
}

function theme_datagrid_pager_last($variables) {
  $text = $variables['text'];
  $datagrid = $variables['datagrid'];
  $parameters = $variables['parameters'];
  $output = '';
  // If we are anywhere but the first page
  $last_page = ceil($datagrid->getTotalResults() / $datagrid->getPager()) - 1;
  if ($datagrid->getCurrentPage() != $last_page) {
    $output = theme('datagrid_pager_link', array(
      'text' => $text,
      'page_new' => $last_page,
      'datagrid' => $datagrid,
      'parameters' => $parameters));
  }
  return $output;
}

function theme_datagrid_pager_previous($variables) {
  $text = $variables['text'];
  $datagrid = $variables['datagrid'];
  $interval = $variables['interval'];
  $parameters = $variables['parameters'];
  $output = '';
  // If we are anywhere but the first page
  if ($datagrid->getCurrentPage() > 0) {
    $page_new = $datagrid->getCurrentPage() - $interval;
    // If the previous page is the first page, mark the link as such.
    if ($page_new == 0) {
      $output = theme('datagrid_pager_first', array(
        'text' => $text,
        'datagrid' => $datagrid,
        'parameters' => $parameters));
    }
    // The previous page is not the first page.
    else {
      $output = theme('datagrid_pager_link', array(
        'text' => $text,
        'page_new' => $page_new,
        'datagrid' => $datagrid,
        'parameters' => $parameters));
    }
  }
  return $output;
}

function theme_datagrid_pager_next($variables) {
  $text = $variables['text'];
  $datagrid = $variables['datagrid'];
  $interval = $variables['interval'];
  $parameters = $variables['parameters'];
  $output = '';
  $last_page = ceil($datagrid->getTotalResults() / $datagrid->getPager()) - 1;
  // If we are anywhere but the last page
  if ($datagrid->getCurrentPage() < $last_page) {
    $page_new = $datagrid->getCurrentPage() + $interval;
    // If the next page is the last page, mark the link as such.
    if ($page_new == $last_page) {
      $output = theme('datagrid_pager_last', array(
        'text' => $text,
        'datagrid' => $datagrid,
        'parameters' => $parameters));
    }
    // The next page is not the last page.
    else {
      $output = theme('datagrid_pager_link', array(
        'text' => $text,
        'page_new' => $page_new,
        'datagrid' => $datagrid,
        'parameters' => $parameters));
    }
  }
  return $output;
}

function theme_datagrid_pager_link($variables) {
  $text = $variables['text'];
  $page_new = $variables['page_new'];
  $datagrid = $variables['datagrid'];
  $parameters = $variables['parameters'];
  $attributes = $variables['attributes'];
  $parameters['_p'] = $page_new;
  // Set each pager link title
  if (!isset($attributes['title'])) {
    static $titles = NULL;
    if (!isset($titles)) {
      $titles = array(
        t('« first') => t('Go to first page'),
        t('‹ previous') => t('Go to previous page'),
        t('next ›') => t('Go to next page'),
        t('last »') => t('Go to last page'),
      );
    }
    if (isset($titles[$text])) {
      $attributes['title'] = $titles[$text];
    }
    elseif (is_numeric($text)) {
      $attributes['title'] = t('Go to page @number', array('@number' => $text));
    }
  }
  if ($datagrid->isUsingAjax()) {
    $attributes["class"][] = "use-ajax";
  }
  return l($text, $datagrid->getPath(), array(
    'attributes' => $attributes,
    'query' => $parameters,
    'fragment' => $datagrid->getId()
  ));
}

function theme_datagrid_active_filters($variables) {
  //$output = "";
  $form = $variables["form"];
  $children = element_children($form);
  $headers = array(
    "logical" => array("data" => t("And/or"), "scope" => "col"),
    "field" => array("data" => t("Field"), "scope" => "col"),
    "operator" => array("data" => t("Operator"), "scope" => "col"),
    "value" => array("data" => t("Value"), "scope" => "col"),
    "remove" => array("data" => "-"),
  );
  $rows = array();
  foreach($children as $child) {
    $row["logical"] = array(
      "data" => (isset($form[$child]["filter-mode-" . $child]) ?
          theme($form[$child]["filter-mode-" . $child]["#type"],
              array("element" => $form[$child]["filter-mode-" .$child])) : "")
    );
    $row["field"] = array(
      "data" => theme("form_element_label",
          array("element" => $form[$child]["filter-value-" .$child]))
    );
    $row["operator"] = array(
      "data" => theme($form[$child]["filter-op-" . $child]["#type"],
          array("element" => $form[$child]["filter-op-" .$child]))
    );
    $row["value"] = array(
      "data" => theme($form[$child]["filter-value-" . $child]["#type"],
          array("element" => $form[$child]["filter-value-" .$child]))
    );
    $row["remove"] = array(
      "data" => drupal_render($form[$child]["filter-remove-" .$child])
    );
    $rows[] = $row;
  }
  $output = theme("table", array("header" => $headers, "rows" => $rows));
  return $output;
}

function theme_datagrid_item_navigation($variables) {
  drupal_add_css(drupal_get_path("module","kw_datagrid") . "/kw_datagrid.css",
      array('every_page' => TRUE));
  $output = "<div class='kw-datagrid-item'>";
  if (!is_null($variables["title"])) {
    $title_content = "";
    $title_content .= (!is_null($variables["title"]["custom"]) ? $variables["title"]["custom"] . " " : "");
    $title_content .= (!is_null($variables["title"]["node"]) ? $variables["title"]["node"] : "");
    $title_content .= ($title_content == "" ? "" : " - ") . $variables["title"]["datagrid"];
    $output .= "<div class='nav-title icon radio-checked'>". $title_content . "</div>";
  }
  $output .= "<div class='nav-content'>";
  if (!is_null($variables["prev"]) || !is_null($variables["next"])) {
    if (!is_null($variables["prev"])) {
      $prev = $variables["prev"];
      $content = array();
      foreach ($prev["rows"] as $row) {
        $content[] = $row["data"];
      }
      $output .= "<div class='nav-prev'>" . l($prev["label"], $prev["link"], array("attributes" => array(
          "class" => array("prev", "icon"),
          "title" => strip_tags(implode(" | ", $content)))
        )) . (count($prev["rows"]) == 0 ?
        "" : " : ". implode(" | ", $content)) . "</div>";
    }
    if (!is_null($variables["next"])) {
      $next = $variables["next"];
      $content = array();
      foreach ($next["rows"] as $row) {
        $content[] = $row["data"];
      }
      $output .= "<div class='nav-next'>" . l($next["label"], $next["link"], array("attributes" => array(
          "class" => array("next", 'icon'),
          "title" => strip_tags(implode(" | ", $content)))
        )). (count($next["rows"]) == 0 ?
        "" : " : ". implode(" | ", $content)) . "</div>";
    }
  }
  $output .= "<div class='nav-back'>" . $variables["title"]["list"] . "</div>";
  $output .= "</div></div>";
  return $output;
}

function theme_datagrid_mass_edit($variables) {
  $form = $variables["form"];
  $rows = $form["args"]["#value"]["rows"];
  $header = $form["args"]["#value"]["header"];
  $edit = $form["args"]["#value"]["edit"];
  foreach ($header as $column => $data) {
    if (isset($data["field"])) {
      $header[$column]["scope"] = "col";
      unset($header[$column]["field"]);
    }
  }
  foreach ($rows as $key => $row) {
    foreach ($row as $column => $data) {
      if ($column == $edit["column"]) {
        $rows[$key][$column] = (isset($form["instance-" . $key]["#field_prefix"]) ? $form["instance-" . $key]["#field_prefix"] : "")
          .theme($form["instance-" . $key]["#type"], array("element" => $form["instance-" . $key]))
          .(isset($form["instance-" . $key]["#field_suffix"]) ? $form["instance-" . $key]["#field_suffix"] : "");
      }
      elseif ($column == $form["title_column"]["#value"]) {
        $rows[$key][$column]["data"] = theme("form_element_label",
          array("element" => $form["instance-" .$key]));
      }
    }
  }
  $output = theme("table", array("header" => $header, "rows" => $rows));
  return $output;
}

function theme_datagrid_supercaption($variables) {
  $output = "<div class='info'>" . $variables['supercaption'] . "</div>";
  return $output;
}

function theme_datagrid_block($variables) {
  $block = $variables["block"];
  $content = $variables["content"];
  /* @var $datagrid  KwDatagrid */
  $datagrid = $variables["datagrid"];
  if (empty($content)) {
    return;
  }
  $output = "<div id='" . $block['html_id'] . "' class='" . strtolower($block['id']) . "'>";
  if (!empty($block['title']) && $block['show_title']) {
    $output .= "<" . $block['title_tag'] . " class='block-title'>" . $block['title'] . "</" . $block['title_tag'] . ">";
  }
  if ($block['hideable']) {
    $output .= "<div class='block-controls'>";
    $parameters = $datagrid->transmitUrlParameters();
    $blocks = $datagrid->retrieveSelectedBlocks();
    if (!empty($blocks) && in_array($block['id'], $blocks)) {
      unset($blocks[array_search($block['id'], $blocks)]);
      $datagrid->storeSelectedBlocks($blocks);
      $parameters['_b'] = $datagrid->retrieveSelectedBlocks($blocks, TRUE);
    }
    else {
      $parameters['_b'] = '';
    }
    $output .= l('<img src="/' . drupal_get_path('module', 'kw_datagrid')
      . '/img/close-icon3.png" alt="' . $block['close_label'] . '" title="' . $block['close_label'] . '" />',
      $datagrid->getPath(), array(
        'attributes' => array(
            'class' => array($datagrid->isUsingAjax() ? 'use-ajax' : 'standard', 'close-button'),
            'rel' => 'nofollow'
        ),
        'query' => $parameters,
        'fragment' => $datagrid->getId(),
        'html' => TRUE
    ));
    $output .= "</div>";
  }
  $output .= "<div class='block-content'>" . $content . "</div>";
  $output .= "</div>";
  return $output;
}

function theme_datagrid_advanced_controls($variables) {
  $output = '';
  /* @var $datagrid  KwDatagrid */
  $datagrid = $variables['datagrid'];
  $parameters = $datagrid->transmitUrlParameters();
  $panel = '';
  $has_filters = FALSE;
  $has_custom_pager = FALSE;
  $has_custom_sort = FALSE;
  $has_custom_columns = FALSE;
  $has_custom_blocks = FALSE;
  if ($datagrid->getUnfilteredTotalResults() > 1) {
    // Filtres
    $test = $datagrid->getFilters("aliases");
    $classes = array('datagrid-button', 'icon', 'filter');
    if ($datagrid->isUsingAjax()) {
      $classes[] = 'use-ajax';
    }
    if (is_array($test) && count($test) > 0) {
      $output .= "<div class='datagrid-advanced-control datagrid-filters'>";
      $filters_toogle_parameters = $parameters;
      $filters = $datagrid->getActiveFilters();
      $has_filters = count($filters) > 0;
      if ((isset($parameters['_o']) && $parameters['_o'] == 'f') || (!isset($parameters['_o']) && $has_filters)) {
        $filters_toogle_parameters['_o'] = '';
        $classes[] = 'activated';
        $output .= l(t('Hide filter panel'), $datagrid->getPath(), array(
            'query' => $filters_toogle_parameters,
            'attributes' => array('class' => $classes, 'rel' => 'nofollow')
        ));
        $result = $datagrid->buildFilterForm();
        $panel = drupal_render($result);
      }
      else {
        $filters_toogle_parameters['_o'] = 'f';
        $output .= l(t('Show filter panel'), $datagrid->getPath(), array(
            'query' => $filters_toogle_parameters,
            'attributes' => array('class' => $classes, 'rel' => 'nofollow')
        ));
      }
      $output .= "</div>";
    }
    // Tris
    $sortables = $datagrid->getSortables();
    if (!empty($sortables) && count(array_keys($datagrid->getSortables("specific"), TRUE)) > 0) {
      if ($datagrid->getCurrentSort() != $datagrid->getDefaultOrderIndex()) {
        $has_custom_sort = TRUE;
      }
      $output .= "<div class='datagrid-advanced-control datagrid-sorts'>";
      $sort_toogle_parameters = $parameters;
      $classes = array('datagrid-button');
      if ($datagrid->isUsingAjax()) {
        $classes[] = 'use-ajax';
      }
      if (isset($parameters['_o']) && $parameters['_o'] == 's') {
        $filters_toogle_parameters['_o'] = '';
        $classes[] = 'activated';
        $output .= l(t('Hide sort panel'), $datagrid->getPath(), array(
            'query' => $sort_toogle_parameters,
            'attributes' => array('class' => $classes, 'rel' => 'nofollow'),
        ));
        $sortables = $datagrid->getSortables();
        $rows = array();
        foreach ($sortables['id'] as $key => $value) {
          $row = array();
          $classes = array();
          $label = $sortables['label'][$key];
          $row[] = array('data' => $label);
          $classes[] = 'datagrid-button';
          if ($datagrid->isUsingAjax()) {
            $classes[] = 'use-ajax';
          }
          $asc_classes = $desc_classes = $classes;
          $asc_parameters = $desc_parameters = $parameters;
          if ($datagrid->getCurrentSort() == $value) {
            $asc_classes[] = 'activated';
          }
          elseif ($datagrid->getCurrentSort() == -$value) {
            $desc_classes[] = 'activated';
          }
          $asc_parameters['_s'] = $value;
          $asc_parameters['_p'] = 0;
          $asc_link = l(t('Ascending'), $datagrid->getPath(), array(
              'query' => $asc_parameters,
              'attributes' => array(
                  'class' => $asc_classes,
                  'title' => t('Sort by !name ascending', array('!name' => $label)),
                  'rel' => 'nofollow'
              ),
              'fragment' => $datagrid->getId()
          ));
          $desc_parameters['_s'] = -$value;
          $desc_parameters['_p'] = 0;
          $desc_link = l(t('Descending'), $datagrid->getPath(), array(
              'query' => $desc_parameters,
              'attributes' => array(
                  'class' => $desc_classes,
                  'title' => t('Sort by !name descending', array('!name' => $label)),
                  'rel' => 'nofollow'
              ),
              'fragment' => $datagrid->getId()
          ));
          $row[] = array('data' => $asc_link . ' | '. $desc_link);
          $rows[] = $row;
        }
        $header = array(t('Sort parameter'), t('Sort order'));
        $panel = theme('table', array('header' => $header, 'rows' => $rows, 'sticky' => FALSE));
      }
      else {
        $sort_toogle_parameters['_o'] = 's';
        $output .= l(t('Show sort panel'), $datagrid->getPath(), array(
            'query' => $sort_toogle_parameters,
            'attributes' => array('class' => $classes, 'rel' => 'nofollow')
        ));
      }
      $output .= "</div>";
    }
  }
  // Sélection des colonnes
  if ($datagrid->hasSelectablesColumns() && $datagrid->getUnfilteredTotalResults() > 0) {
    $output .= "<div class='datagrid-advanced-control datagrid-columns'>";
    $columns_toogle_parameters = $parameters;
    $classes = array('datagrid-button', 'icon', 'equalizer');
    if ($datagrid->isUsingAjax()) {
      $classes[] = 'use-ajax';
    }
    $selected = $datagrid->listSelectedColumns(FALSE);
    if (array_keys($selected) != $datagrid->getDefaultColumns()) {
      $has_custom_columns = TRUE;
    }
    if (isset($parameters['_o']) && $parameters['_o'] == 'c') {
      $columns_toogle_parameters['_o'] = '';
      $classes[] = 'activated';
      $output .= l(t('Hide columns panel'), $datagrid->getPath(), array(
          'query' => $columns_toogle_parameters,
          'attributes' => array('class' => $classes, 'rel' => 'nofollow')
      ));
      $columns = $datagrid->listSelectableColumns();
      $header = array(
        array('data' => t('Column')),
        array('data' => t('Action'))
      );
      $rows = array();
      foreach ($columns as $key => $column) {
        $row = array();
        //$row[] = array('data' => $column);
        $row[] = $column;
        $columns_parameters = $parameters;
        $classes = array('datagrid-button', 'icon');
        if ($datagrid->isUsingAjax()) {
          $classes[] = 'use-ajax';
        }
        if (isset($selected[$key])) {
          $classes[] = 'activated';
          $classes[] = 'eye-close';
          $columns_link = $selected;
          unset($columns_link[$key]);
          $columns_parameters['_c'] = $datagrid->retrieveSelectedColumns(array_keys($columns_link), TRUE);
          $label = t('Hide');
        }
        else {
          $classes[] = 'eye-open';
          $columns_link = array_keys($selected);
          $columns_link[] = $key;
          $columns_parameters['_c'] = $datagrid->retrieveSelectedColumns($columns_link, TRUE);
          $label = t('Display');
        }
        $action = l($label, $datagrid->getPath(), array(
            'query' => $columns_parameters,
            'attributes' => array('class' => $classes, 'rel' => 'nofollow')
        ));
        $row[] = $action;
        $rows[] = implode(' : ', $row);
        //$row[] = array('data' => $action);
        //$rows[] = $row;
      }
      $panel = theme('item_list', array('items' => $rows, 'attributes' => array('class' => 'inline')));
      //$panel = theme('table', array('header' => $header, 'rows' => $rows, 'sticky' => FALSE));
    }
    else {
      $columns_toogle_parameters['_o'] = 'c';
      $output .= l(t('Show columns panel'), $datagrid->getPath(), array(
          'query' => $columns_toogle_parameters,
          'attributes' => array('class' => $classes, 'rel' => 'nofollow')
      ));
    }
    $output .= "</div>";
  }
  // Affichage des blocs
  if (count($datagrid->getConfigurableBlocks()) > 0 && $datagrid->getUnfilteredTotalResults() > 0) {
    $output .= "<div class='datagrid-advanced-control datagrid-blocks'>";
    $current_blocks = $datagrid->retrieveSelectedBlocks();
    if ($current_blocks  !== FALSE && $current_blocks != $datagrid->getDefaultBlocks()) {
      $has_custom_blocks = TRUE;
    }
    if (empty($current_blocks[0])) {
      unset($current_blocks[0]);
    }
    foreach ($datagrid->getConfigurableBlocks() as $key => $block) {
      $block_parameters = $parameters;
      if ($block['hidden']) {
        $current_blocks[] = $block['id'];
        $block_parameters['_b'] = implode(';', $current_blocks);
        $output .= l($block['open_label'], $datagrid->getPath(), array(
            'query' => $block_parameters,
            'fragment' => $block['html_id'],
            'attributes' => array('class' => array('datagrid-button'), 'rel' => 'nofollow')
        ));
      }
      else if ($block['hideable']) {
        if (!empty($current_blocks)) {
          unset($current_blocks[array_search($block['id'], $current_blocks)]);
          $block_parameters['_b'] = implode(';', $current_blocks);
        }
        else {
          $block_parameters['_b'] = '';
        }
        $output .= l($block['close_label'], $datagrid->getPath(), array(
            'query' => $block_parameters,
            'fragment' => $datagrid->getId(),
            'attributes' => array(
                'rel' => 'nofollow',
                'class' => array('datagrid-button', 'activated', $datagrid->isUsingAjax() ? 'use-ajax' : NULL)
            )
        ));
      }
    }
    $output .= "</div>";
  }
  if ($datagrid->getPager() != $datagrid->getPagerOptions('default_value')) {
    $has_custom_pager = TRUE;
  }
  if ($has_filters || $has_custom_pager || $has_custom_sort || $has_custom_columns || $has_custom_blocks) {
    $output .= "<div class='datagrid-advanced-control datagrid-reset'>"
      . l(t('Reset'), $datagrid->getPath(), array(
          'query' => array('_id' => $datagrid->getId(), '_r' => 2),
          'fragment' => $datagrid->getId(),
          'attributes' => array(
              'rel' => 'nofollow',
              'class' => array('datagrid-button', $datagrid->isUsingAjax() && count($datagrid->getConfigurableBlocks()) == 0 ? 'use-ajax' : NULL)
          )
      ))
      . "</div>";
  }
  if (!empty($panel)) {
    $output .= '<div class="datagrid-panel">' . $panel . '</div>';
  }
  return $output;
}