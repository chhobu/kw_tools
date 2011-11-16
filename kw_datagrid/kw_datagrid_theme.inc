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
  drupal_add_css(drupal_get_path("module", "kw_datagrid") . "/kw_datagrid.css");
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
    $ts['query'] = drupal_get_query_parameters($_GET, array('q', 'sort', 'grid', 'page', 'open'));
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
          isset($_GET["page"]) ? $_GET["page"] : $_GET["q"], array("html" => TRUE, "query" => array("grid" => $datagrid->getId(), "mass_edit" => $column_key), "fragment" => $datagrid->getId()));
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
  $footer_output = "";
  if ($datagrid->getPager() > 0) {
    $footer_output .= "<tr><td colspan='" . $nb_cols . "'>"
      . theme("datagrid_pager", array("datagrid" => $datagrid))
      ."</td></tr>";
  }
  if($footer_output != "") {
    $output .= "<tfoot>" . $footer_output . "</tfoot>";
  }
  $output .= "</table>\n";
  $form = $datagrid->buildForm();
  if (!is_null($form) && ($datagrid->getUnfilteredTotalResults() > 1 || ($datagrid->getUnfilteredTotalResults() > 0 && $datagrid->hasSelectablesColumns()))) {
    $output .= "<div class='datagrid-filters'>"
      . drupal_render($form)
      ."</div>";
  }
  return $output;
}

function kw_datagrid_tablesort_header(KwDatagrid $datagrid, $key, $cell, $header, $ts) {
  // Special formatting for the currently sorted column header.
  if (is_array($cell) && isset($cell['field'])) {
    $title = t('sort by @s', array('@s' => $cell['data']));
    $currentSort = $datagrid->getCurrentSort();
    $sortKey = $datagrid->findSortable($key);
    if (abs($currentSort) == $sortKey) {
      $cell['class'][] = 'active';
      $image = theme('tablesort_indicator', array('style' => $currentSort > 0 ? "asc" : "desc"));
    }
    else {
      // If the user clicks a different header, we want to sort ascending initially.
      $image = '';
    }
    $cell['data'] = l($cell['data'] . $image, "kw_datagrid/nojs/",
      array(
        'attributes' => array(
          'title' => $title,
          'class' => array($datagrid->isUsingAjax() ? 'use-ajax' : 'standard', 'sortable')
        ),
        'query' => array_merge($ts['query'], array(
          'grid' => $datagrid->getId(),
          'page' => isset($_GET["page"]) ? $_GET["page"] : $_GET["q"],
          'sort' => $sortKey == $currentSort ? -$sortKey : $sortKey
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
  $parameters = drupal_get_query_parameters($_GET, array('q', 'grid', 'page', 'p', 'open'));
  $quantity = $datagrid->getPagerQuantity();
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
    return '<h3 class="element-invisible">' . t('Pages') . '</h3>' . theme('item_list', array('items' => $items, 'title' => NULL, 'type' => 'ul', 'attributes' => array('class' => array('pager'))));
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
  $parameters['p'] = $page_new;
  $parameters['page'] = isset($_GET["page"]) ? $_GET["page"] : $_GET["q"];
  $parameters['grid'] = $datagrid->getId();
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
  if($datagrid->isUsingAjax()) {
    $attributes["class"][] = "use-ajax";
  }
  return l($text, "kw_datagrid/nojs/", array(
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
      "data" => (isset($form[$child]["filter-mode-" . $child]) ? theme($form[$child]["filter-mode-" . $child]["#type"], array("element" => $form[$child]["filter-mode-" .$child])) : "")
    );
    $row["field"] = array(
      "data" => theme("form_element_label", array("element" => $form[$child]["filter-value-" .$child]))
    );
    $row["operator"] = array(
      "data" => theme($form[$child]["filter-op-" . $child]["#type"], array("element" => $form[$child]["filter-op-" .$child]))
    );
    $row["value"] = array(
      "data" => theme($form[$child]["filter-value-" . $child]["#type"], array("element" => $form[$child]["filter-value-" .$child]))
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
  drupal_add_css(drupal_get_path("module","kw_datagrid") . "/kw_datagrid.css");
  $output = "<div class='kw-datagrid-item'>";
  if (!is_null($variables["title"])) {
    $title_content = "";
    $title_content .= (!is_null($variables["title"]["custom"]) ? $variables["title"]["custom"] . " " : "");
    $title_content .= (!is_null($variables["title"]["node"]) ? $variables["title"]["node"] : "");
    $title_content .= ($title_content == "" ? "" : " - ") . $variables["title"]["datagrid"];
    $output .= "<div class='nav-title'>". $title_content . "</div>";
  }
  $output .= "<div class='nav-content'>";
  if (!is_null($variables["prev"]) || !is_null($variables["next"])) {
    if (!is_null($variables["prev"])) {
      $prev = $variables["prev"];
      $content = array();
      foreach ($prev["rows"] as $row) {
        $content[] = $row["data"];
      }
      $output .= "<div class='nav-prev'>" . l($prev["label"], $prev["link"], array("attributes" => array("class" => array("prev"), "title" => strip_tags(implode(" | ", $content))))) . (count($prev["rows"]) == 0 ?
        "" : " : ". implode(" | ", $content)) . "</div>";
    }
    if (!is_null($variables["next"])) {
      $next = $variables["next"];
      $content = array();
      foreach ($next["rows"] as $row) {
        $content[] = $row["data"];
      }
      $output .= "<div class='nav-next'>" . l($next["label"], $next["link"], array("attributes" => array("class" => array("next"), "title" => strip_tags(implode(" | ", $content))))). (count($next["rows"]) == 0 ?
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