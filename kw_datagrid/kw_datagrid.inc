<?php
define("KW_FILTRABLE_NO", 0);
define("KW_FILTRABLE_STRING", 1);
define("KW_FILTRABLE_DATE", 2);
define("KW_FILTRABLE_NUM", 3);
define("KW_FILTRABLE_ENUM", 4);

class KwDatagrid {
  private $id;
  private $constructor;
  private $table;
  private $tables = array();
  private $headers = array();
  private $conditions = array();
  private $fields = array();
  private $columns = array();
  private $defaultOrders = array();
  private $attributes = array();
  private $filters = array();
  private $activeFilters = array();
  private $appliedFilters = array();
  private $sortables = array();
  private $replacements = array();
  private $colgroup = NULL;
  private $caption = NULL;
  private $title = NULL;
  private $titleMode = "h2";
  private $extraCaption;
  private $emptyText = NULL;
  private $use_sticky = TRUE;
  private $use_ajax = TRUE;
  private $weight = 0;
  private $persistant = TRUE;
  private $pager = 0;
  private $pager_quantity = 9;
  private $pager_options = array();
  private $total_results = 0;
  private $unfiltered_total_results = 0;
  private $rank_result = 0;
  private $current_page = -1;
  private $current_sort = NULL;
  private $mode = "list";
  private $mode_context = array();
  private $linked_node = NULL;
  private $key = NULL;
  private $intro = NULL;
  private $outro = NULL;
  private $selectable_columns = FALSE;
  private $pool = NULL;

  public function __construct($id, $table, $alias, $caption, $options = array()) {
    $this->id = $id;
    $this->table = $table;
    $this->tables[$alias] = $table;
    $this->caption = trim($caption);
    $this->pool = $id;
    if (isset($options["weight"])) {
      $this->weight = $options["weight"];
    }
    if (isset($options["mode"])) {
      $this->mode = $options["mode"];
    }
    if (isset($options["mode_context"])) {
      $this->mode_context = $options["mode_context"];
    }
    if (isset($options["node"])) {
      $this->linked_node = $options["node"];
    }
    if (isset($options["pool"])) {
      $this->pool = $options["pool"];
    }
    $this->extraCaption = array(
      "single_result" => "Only 1 result found.",
      "many_results" => "%total_results results found, order by !order.",
      "single_result_filtered" => "Only 1 result found (out of %unfiltered_total).",
      "many_results_filtered" => "%total_results results found (out of %unfiltered_total), order by !order.",
      "pager" => "Displaying results %start to %end.",
      "single_filter" => "1 filter active : !filters.",
      "many_filters" => "%nb_filters filters active : !filters.",
      "backlink" => t("View full list"),
      "filtered_backlink" => t("View filtered list"),
      "unfiltered_backlink" => t("View unfiltered list"),
      "next_label" => t("Next"),
      "prev_label" => t("Previous")
    );
    if(isset($options["extra_captions"]) && is_array($options["extra_captions"])) {
      $this->extraCaption = array_merge($this->extraCaption, $options["extra_captions"]);
    }
    if (isset($_GET["reset"]) && $_GET["reset"] == 1) {
      $this->reset();
    }
    if($this->mode != "item" && !kw_datagrid_retrieve_datagrid_data($this, "constructor")) {
      $trace = debug_backtrace(FALSE);
      $this->constructor = array(
        "function" => $trace[1]["function"],
        "args" => $trace[1]["args"]
      );
      $this->storeInSession("constructor", $this->constructor, FALSE);
    }
    return $this;
  }

  public function getAttributes() {
    return $this->attributes;
  }

  public function setAttributes($array) {
    if (is_array($array)) {
      $this->attributes = $array;
    }
    return $this;
  }

  public function getCaption() {
    return $this->caption;
  }

  public function getTitle() {
    return $this->title;
  }

  public function setTitle($title, $mode = NULL) {
    $this->title = trim($title);
    if (!is_null($mode)) {
      $this->setTitleMode($mode);
    }
    return $this;
  }

  public function setTitleMode($mode) {
    $this->titleMode = $mode;
    return $this;
  }

  public function getColgroup() {
    return $this->colgroup;
  }

  public function getConstructor($key = NULL) {
    if (!is_null($key) && isset($this->constructor[$key])) {
      return $this->constructor[$key];
    }
    return $this->constructor;
  }

  public function getPool() {
    return $this->pool;
  }

  public function getEmptyText() {
    return $this->emptyText;
  }

  public function getHeaders() {
    return $this->headers;
  }

  public function getId() {
    return $this->id;
  }

  public function getCurrentPage() {
    if ($this->pager == 0) {
      $this->current_page = -1;
    }
    elseif ($this->current_page == -1) {
      if (isset($_GET["p"]) && is_numeric($_GET["p"]) && $_GET["p"] >= 0) {
        $this->current_page = $_GET["p"];
        if($this->persistant) {
          $this->storeInSession("p", $_GET["p"], FALSE);
        }
      }
      elseif ($this->persistant && $this->isStoredInSession("p")) {
        $this->current_page = kw_datagrid_retrieve_datagrid_data($this, "p");
      }
      if ($this->current_page < 0) {
        $this->current_page = 0;
      }
    }
    if ($this->total_results > 0 && $this->pager > 0 && $this->current_page >= ceil($this->total_results / $this->pager)) {
      $this->current_page = ceil($this->total_results / $this->pager) - 1;
    }
    return $this->current_page;
  }

  public function getUnfilteredTotalResults() {
    return $this->unfiltered_total_results;
  }

  public function getTotalResults() {
    return $this->total_results;
  }

  public function isUsingAjax() {
    return ($this->use_ajax ? TRUE : FALSE);
  }

  public function setUseAjax($bool) {
    $this->use_ajax = $bool;
    return $this;
  }

  public function isUsingSticky() {
    return ($this->use_sticky ? TRUE : FALSE);
  }

  public function setUseSticky($bool) {
    $this->use_sticky = $bool;
    return $this;
  }

  public function getPager() {
    return $this->pager;
  }

  public function getPagerOptions() {
    return $this->pager_options;
  }

  public function isPersitant() {
    return ($this->persistant ? TRUE : FALSE);
  }

  public function setPersistant($bool) {
    $this->persistant = $bool;
    return $this;
  }

  public function addPager($pager, $exposed = FALSE) {
    if (!is_numeric($pager)) {
      $pager = 0;
    }
    $this->pager = $pager;
    if ($exposed) {
      if (is_array($exposed)) {
        $this->pager_options = $exposed;
      }
      else {
        $this->pager_options = array(5, 10, 20, 30, 50, 100);
      }
    }
    return $this;
  }

  public function getPagerQuantity() {
    return $this->pager_quantity;
  }

  public function setPagerQuantity($value) {
    if ($value > 0) {
      $this->pager_quantity = $value;
    }
    return $this;
  }

  public function addCondition($field, $value = NULL, $operand = NULL) {
    $this->conditions[] = array(
      "field" => $field,
      "value" => $value,
      "operand" => $operand
    );
    return $this;
  }

  public function addTable($table, $alias, $type_jointure, $jointure = NULL, $options = array()) {
    if (!isset($this->tables[$alias])) {
      $needs_group_by = FALSE;
      $linked_columns = NULL;
      $args = array();
      $count_method = "classic";
      if (is_array($options)) {
        if (isset($options["group_by"])) {
          $needs_group_by = $options["group_by"];
        }
        if (isset($options["args"])) {
          $args = $options["args"];
        }
        if (isset($options["count_method"])) {
          $count_method = $options["count_method"];
        }
        if (isset($options["linked_columns"])) {
          $linked_columns = $options["linked_columns"];
          if (!is_null($linked_columns) && $linked_columns && !is_array($linked_columns)) {
            $linked_columns = array($linked_columns);
          }
        }
      }
      $this->tables[$alias] = array(
        "table" => $table,
        "type_jointure" => $type_jointure,
        "jointure" => $jointure,
        "args" => $args,
        "group_by" => $needs_group_by,
        "linked_columns" => $linked_columns,
        "count_method" => $count_method
      );
    }
    return $this;
  }

  public function addKey($alias, $table_alias, $field, $selectable = TRUE, $sortable = FALSE, $filtrable = KW_FILTRABLE_NO) {
    $this->addField($alias, $table_alias, $field, $selectable, $sortable, $filtrable);
    $this->key = $alias;
    return $this;
  }

  public function addField($alias, $table_alias, $field, $selectable = TRUE, $sortable = FALSE, $filtrable = KW_FILTRABLE_NO, $hidden = FALSE, $expression = NULL, $sort_expression = NULL) {
    $this->fields[$alias] = array(
      "table_alias" => $table_alias,
      "table_field" => $field,
      "selectable" => $selectable,
      "sortable" =>  $sortable,
      "filtrable" => $filtrable,
      "hidden" => $hidden,
      "expression" => $expression,
      "sort_expression" => $sort_expression
    );
    return $this;
  }

  public function getAliasField($alias) {
    if (isset($this->fields[$alias])) {
      return $this->fields[$alias];
    }
    return FALSE;
  }

  public function getColumns() {
    return $this->columns;
  }

  public function addColumn($alias, $label, $output, $table_alias = NULL, $field = NULL, $options = NULL) {
    $sortable = FALSE;
    $filtrable = FALSE;
    $filtrable_options = array();
    $itemable = FALSE;
    $hidden = FALSE;
    $main = FALSE;
    $uncacheable = FALSE;
    $expression = NULL;
    $modifiable = FALSE;
    $modifiable_options = array();
    $sort_expression = NULL;
    if (is_array($options)) {
      if (isset($options["sortable"])) {
        $sortable = $options["sortable"];
      }
      if (isset($options["filtrable"])) {
        $filtrable = $options["filtrable"];
      }
      if (isset($options["filtrable_options"])) {
        $filtrable_options = $options["filtrable_options"];
      }
      if (isset($options["itemable"])) {
        $itemable = $options["itemable"];
      }
      if (isset($options["sortable"])) {
        $sortable = $options["sortable"];
      }
      if (isset($options["hidden"])) {
        $hidden = $options["hidden"];
      }
      if (isset($options["main"])) {
        $main = $options["main"];
      }
      if (isset($options["uncacheable"])) {
        $uncacheable = $options["uncacheable"];
      }
      if (isset($options["expression"])) {
        $expression = $options["expression"];
      }
      if (isset($options["sort_expression"])) {
        $sort_expression = $options["sort_expression"];
      }
      if (isset($options["modifiable"])) {
        $modifiable = $options["modifiable"];
      }
      if (isset($options["modifiable_options"])) {
        $modifiable_options = $options["modifiable_options"];
      }
    }
    if ($main) {
      $uncacheable = TRUE;
      $modifiable = FALSE;
    }
    $this->columns[$alias] = array(
      "label" => $label,
      "is_db_field" => !is_null($table_alias) && !is_null($field) ? TRUE : FALSE,
      "output" => $output,
      "itemable" => $itemable,
      "main" => $main,
      "hidden" => $hidden,
      "uncacheable" => $uncacheable,
      "modifiable" => $modifiable,
      "modifiable_options" => $modifiable_options
    );
    if ($this->columns[$alias]["is_db_field"]) {
      $this->addField($alias, $table_alias, $field, TRUE, $sortable, $filtrable, $hidden, $expression, $sort_expression);
    }
    if ($hidden) {
      $this->selectable_columns = TRUE;
    }
    if ($sortable) {
      $this->addSortable($alias, $label, 0, FALSE);
    }
    if ($filtrable) {
      $this->addFilter($alias, $label, $filtrable, $filtrable_options);
    }
    return $this;
  }

  public function hasSelectablesColumns() {
    return $this->selectable_columns;
  }

  public function toggleSelectableColumns($value = NULL) {
    if (!is_null($value)) {
      $this->selectable_columns = $value;
    }
    else {
      $this->selectable_columns = !$this->selectable_columns;
    }
  }

  public function addQueryReplacement($alias, $query, $key, $args = array()) {
    $this->replacements[$alias] = array(
      "function" => "!query",
      "args" => array(
        "query" => $query,
        "args" => $args,
        "key" => $key
      )
    );
    return $this;
  }

  public function addReplacement($alias, $function, $args = array()) {
    $this->replacements[$alias] = array(
      "function" => $function,
      "args" => $args
    );
    return $this;
  }

  public function addDefaultOrder($alias, $order = "asc") {
    $this->defaultOrders[] = array(
      "column" => $alias,
      "order" => $order
    );
    return $this;
  }

  public function getDefaultOrders() {
    return $this->defaultOrders;
  }

  public function addSortable($aliases, $label, $weight = 0, $directCall = TRUE) {
    if (!is_array($aliases)) {
      $aliases = array($aliases);
    }
    sort($aliases);
    if (!isset($this->sortables["aliases"]) || array_search($aliases, $this->sortables["aliases"]) === FALSE) {
      $this->sortables["aliases"][] = $aliases;
      $this->sortables["label"][] = $label;
      $this->sortables["weight"][] = $weight;
      $this->sortables["specific"][] = $directCall;
      $this->sortables["id"][] = count($this->sortables["aliases"]);
    }
    return $this;
  }

  public function findSortable($alias) {
    if (count($this->sortables) == 0) {
      return FALSE;
    }
    if (!is_array($alias)) {
      $alias = array($alias);
    }
    sort($alias);
    $key = array_search($alias, $this->sortables["aliases"]);
    if ($key !== FALSE) {
      return $this->sortables["id"][$key];
    }
    else {
      return 0;
    }
  }

  public function getSortables($var = NULL) {
    if (is_null($var)) {
      return $this->sortables;
    }
    if (isset($this->sortables[$var])) {
      return $this->sortables[$var];
    }
    return FALSE;
  }

  public function addFilter($aliases, $label, $type, $options = array(), $weight = 0) {
    if (!is_array($aliases)) {
      $aliases = array($aliases);
    }
    sort($aliases);
    if ($type == KW_FILTRABLE_STRING && !isset($options["size"])) {
      $options["size"] = 20;
    }
    if ($type == KW_FILTRABLE_STRING && !isset($options["maxlength"])) {
      $options["maxlength"] = 255;
    }
    $this->filters["aliases"][] = $aliases;
    $this->filters["label"][] = $label;
    $this->filters["type"][] = $type;
    $this->filters["weight"][] = $weight;
    $this->filters["options"][] = $options;
    $this->filters["id"][] = count($this->filters["aliases"]);
    return $this;
  }

  public function getFilters($var = NULL) {
    if (is_null($var)) {
      return $this->filters;
    }
    if (isset($this->filters[$var])) {
      return $this->filters[$var];
    }
    return FALSE;
  }

  public function findFilter($aliases) {
    if (count($this->filters["id"]) == 0) {
      return FALSE;
    }
    if (!is_array($aliases)) {
      $aliases = array($aliases);
    }
    sort($aliases);
    $key = array_search($aliases, $this->filters["aliases"]);
    if ($key !== FALSE) {
      return $this->filters["id"][$key];
    }
    else {
      return FALSE;
    }
  }

  public function addActiveFilter($key) {
    if (count($this->activeFilters) > 50) {
      drupal_set_message("Too much filters !", "error");
      return $this;
    }
    $this->activeFilters[] = array(
      "filter_id" => $key,
      "mode" => NULL,
      "op" => NULL,
      "value" => NULL
    );
    return $this;
  }

  public function removeActiveFilter($index) {
    if (isset($this->activeFilters[$index])) {
      unset($this->activeFilters[$index]);
    }
  }

  public function alterActiveFilter($index, $mode, $op, $value) {
    if (!isset($this->activeFilters[$index])) {
      return FALSE;
    }
    $this->activeFilters[$index]["mode"] = $mode;
    $this->activeFilters[$index]["op"] = $op;
    $this->activeFilters[$index]["value"] = $value;
    return TRUE;
  }

  public function getActiveFilters() {
    return $this->activeFilters;
  }

  public function setCaption($caption) {
    $this->caption = $caption;
    return $this;
  }

  public function setEmptyText($emptyText) {
    $this->emptyText = $emptyText;
    return $this;
  }

  public function setIntro($text) {
    $this->intro = $text;
    return $this;
  }

  public function setOutro($text) {
    $this->outro = $text;
    return $this;
  }

  public function buildForm() {
    static $form = array();
    if (!isset($form[$this->id])) {
      if ((isset($this->filters["aliases"]) && count($this->filters["aliases"]) > 0) || $this->selectable_columns || count($this->pager_options) > 0 || count(array_keys($this->sortables["specific"], TRUE)) > 0) {
        $form[$this->id] = drupal_get_form("kw_datagrid_filter_form", array("datagrid" => $this));
      }
      else {
        $form[$this->id] = NULL;
      }
    }
    return $form[$this->id];
  }

  public function build() {
    $target_grid = $this->id;
    if (isset($_GET["grid"])) {
      $target_grid = $_GET["grid"];
    }
    // Récupération du mode
    if (!is_null($this->key) && $target_grid == $this->id && isset($_GET["mass_edit"]) && isset($this->columns[$_GET["mass_edit"]])
      && $this->columns[$_GET["mass_edit"]]["modifiable"] && $this->getAliasField($_GET["mass_edit"])) {
      $this->mode = "mass-edition";
      $this->mode_context = array("column" => $_GET["mass_edit"]);
    }
    // Récupération de la valeur du pager
    if (isset($_GET["pager"]) && is_numeric($_GET["pager"]) && $_GET["pager"] >= 0) {
      $this->pager = $_GET["pager"];
      if ($this->persistant) {
        $this->storeInSession("pager", $_GET["pager"]);
      }
    }
    elseif ($this->persistant && $this->isStoredInSession("pager")) {
      $this->pager = kw_datagrid_retrieve_datagrid_data($this, "pager");
    }
    // Récupération des colonnes affichées
    if ($this->mode == "mass-edition") {
      foreach ($this->columns as $alias => $column) {
        if ($column["uncacheable"] || $alias == $this->mode_context["column"]) {
          $this->columns[$alias]["hidden"] = FALSE;
          if ($column["is_db_field"]) {
            $this->fields[$alias]["hidden"] = FALSE;
          }
          continue;
        }
        else {
          $this->columns[$alias]["hidden"] = TRUE;
          if ($column["is_db_field"]) {
            $this->fields[$alias]["hidden"] = TRUE;
          }
        }
      }
    }
    else if ($this->hasSelectablesColumns()) {
      $columns = $this->retrieveSelectedColumns();
      if (is_array($columns)) {
        foreach ($this->columns as $alias => $column) {
          if ($column["uncacheable"]) {
            continue;
          }
          $this->columns[$alias]["hidden"] = !in_array($alias, $columns);
          if ($column["is_db_field"]) {
            $this->fields[$alias]["hidden"] = $this->columns[$alias]["hidden"];
          }
        }
      }
    }
    // Récupération des filtres actifs
    if (isset($_GET["filters"])) {
      $filters = unserialize($_GET["filters"]);
      if (is_array($filters)) {
        $this->activeFilters = $filters;
        if ($this->persistant) {
          $this->storeInSession("filters", $filters, FALSE);
        }
      }
    }
    elseif ($this->persistant && $this->isStoredInSession("filters")) {
      $this->activeFilters = kw_datagrid_retrieve_datagrid_data($this, "filters");
    }
    // Génération du formulaire de filtrage
    if ($this->mode != "item") {
      $form = $this->buildForm();
    }
    // Création de la requête
    $query = $this->createQuery();
    // Remplissage des lignes du tableau
    $rows = array();
    $subqueries = array();
    if($this->total_results > 0) {
      $result = $query->execute();
      $orig_rows = array();
      $pattern = "/\:(\w+)/";
      foreach ($result as $key => $row) {
        if ($this->mode != "item" && !is_null($this->key)) {
          $key = $row->{$this->key};
        }
        $orig_rows[$key] = $row;
        foreach ($this->columns as $alias => $column) {
          if ($column["hidden"]) {
            continue;
          }
          $this->headers[$alias]["data"] = $column["label"];
          if (isset($this->fields[$alias]) && $this->fields[$alias]["sortable"]) {
            $this->headers[$alias]["field"] = $this->fields[$alias]["table_field"];
          }
          $output = $column["output"];
          if ($this->mode == "item" && !$column["itemable"]) {
            continue;
          }
          elseif ($this->mode == "mass-edition" && $this->mode_context["column"] == $alias) {
            $rows[$key][$alias]["data"] = $row->$alias;
            continue;
          }
          if (!isset($matches[$alias])) {
            preg_match_all($pattern, $output, $matches[$alias], PREG_SET_ORDER);
          }
          if (isset($matches[$alias]) && is_array($matches[$alias])) {
            foreach ($matches[$alias] as $match) {
              if (isset($this->replacements[$match[1]])) {
                $args = $this->replacements[$match[1]]["args"];
                if ($this->replacements[$match[1]]["function"] == "!query") {
                  if (!isset($subqueries[$match[1]]) && isset($args["query"]) && isset($args["args"])
                  && is_array($args["args"]) && isset($args["key"]) && isset($row->$args["key"])) {
                    $subqueries[$match[1]] = $args;
                    $subqueries[$match[1]]["columns"][] = $alias;
                  }
                  if (isset($subqueries[$match[1]])) {
                    $subqueries[$match[1]]["keys"][$key] = $row->$args["key"];
                  }
                  continue;
                }
                $args = $this->proceedArgsReplacement($match[1], $args, $row, $pattern);
                $rpl_string = call_user_func_array($this->replacements[$match[1]]["function"], $args);
                $output = str_replace($match[0], $rpl_string, $output);
              }
              if (property_exists($row, $match[1])) {
               $output = str_replace($match[0], $row->$match[1], $output);
              }
            }
          }
          if ($column["main"]) {
            $rows[$key][$alias]["scope"] = "row";
            $rows[$key][$alias]["class"][] = "main-cell";
          }
          $rows[$key][$alias]["data"] = $output;
        }
      }
      foreach ($subqueries as $placeholder => $subquery) {
        $args = $subquery["args"];
        $query = $subquery["query"];
        $keys = array();
        if (isset($subquery["keys"])) {
          foreach ($subquery["keys"] as $key => $value) {
            $args[":key" . $key] = $value;
            $keys[$key] = ":key" . $key;
          }
        }
        $query = str_replace(":keys", implode(",", $keys), $query);
        $sq = db_query($query, $args);
        $result_sq = array();
        foreach ($sq as $row) {
          if (isset ($row->sq_key) && isset($row->sq_value)) {
            $result_sq[$row->sq_key][] = $row->sq_value;
          }
        }
        foreach ($orig_rows as $key => $row) {
          if (isset($rows[$key]) && isset($row->$subquery["key"])) {
            foreach ($subquery["columns"] as $column) {
              $data = t("(None)");
              if (isset($result_sq[$row->$subquery["key"]])) {
                $data = implode(", ", $result_sq[$row->$subquery["key"]]);
              }
              $rows[$key][$column]["data"] = str_replace(":" . $placeholder, $data, $rows[$key][$column]["data"]);
            }
          }
        }
      }
    }
    $orig_caption = $this->caption;
    // Création de l'en-tête et des titres
    if (!is_null($this->caption)) {
      if ($this->titleMode == "caption" && !is_null($this->title)) {
        $this->caption = $this->title;
      }
      if (trim($this->caption) != "") {
        $this->caption = "<strong> " . $this->caption . " :</strong> ";
      }
      if (count($this->appliedFilters) == 0) {
        if (isset($this->extraCaption["single_result"]) && $this->total_results == 1) {
          $this->caption .= "<span class='results'>" .
            t($this->extraCaption["single_result"]) . "</span>";
        }
        elseif (isset($this->extraCaption["many_results"]) && $this->total_results > 1) {
          $currentSort = $this->getCurrentSort();
          if($currentSort != 0) {
            $currentKey = array_search(abs($currentSort), $this->sortables["id"]);
          }
          $this->caption .= "<span class='results'>"
            . t($this->extraCaption["many_results"], array(
              "%total_results" => $this->total_results,
              "!order" => $currentSort == 0 ? t("default")
                : "<em>" . $this->sortables["label"][$currentKey] . "</em>"
                . ($currentSort == 0 ? "": " ". ($currentSort > 0 ? t("ascending") : t("descending")))))
            . "</span>";
        }
      }
      else {
        if (isset($this->extraCaption["single_result_filtered"]) && $this->total_results == 1) {
          $this->caption .= "<span class='results'>" .
            t($this->extraCaption["single_result_filtered"], array("%unfiltered_total" => $this->unfiltered_total_results)) . "</span>";
        }
        elseif (isset($this->extraCaption["many_results_filtered"]) && $this->total_results > 1) {
          $currentSort = $this->getCurrentSort();
          if($currentSort != 0) {
            $currentKey = array_search(abs($currentSort), $this->sortables["id"]);
          }
          $this->caption .= "<span class='results'>" .
            t($this->extraCaption["many_results_filtered"], array(
              "%total_results" => $this->total_results,
              "%unfiltered_total" => $this->unfiltered_total_results,
              "!order" => $currentSort == 0 ? t("default")
                : "<em>" . $this->sortables["label"][$currentKey] . "</em>"
                . ($currentSort == 0 ? "": " ". ($currentSort > 0 ? t("ascending") : t("descending")))
            )) . "</span>";
        }
        if (count($this->appliedFilters) == 1 && isset($this->extraCaption["single_filter"])) {
          $this->caption .= " <span class='filters'>"
          . t($this->extraCaption["single_filter"], array("!filters" => implode(" ", $this->appliedFilters))) . "</span>";
        }
        elseif (isset($this->extraCaption["many_filters"])) {
          $this->caption .= " <span class='filters'>"
          . t($this->extraCaption["many_filters"], array("%nb_filters" => count($this->appliedFilters), "!filters" => implode(" ", $this->appliedFilters))) . "</span>";
        }
        $this->caption .= " <span class='reset-filters'><a href='?reset=1#". $this->id ."'>"
          . t("(Remove all filters)") . "</a>.</span>";
      }
      if ($this->pager > 0 && $this->total_results > $this->pager) {
        $this->caption .= " <span class='pager-active'>" .
          t($this->extraCaption["pager"], array("%start" => 1 + $this->current_page * $this->pager,
            "%end" => min(($this->current_page + 1) * $this->pager, $this->total_results))) . "</span>";
      }
    }
    // Message des filtres
    if ($this->total_results == 0 && $this->total_results != $this->unfiltered_total_results) {
      $this->emptyText = t("No results found matching your criteria. Try changing the filters values.");
    }
    // Déclaration des éléments envoyés à la fonction drupal_render
    if ($this->mode == "item" && !is_null($this->key)) {
      if (!isset($this->mode_context["entity_type"])) {
        $this->mode_context["entity_type"] = "node";
      }
      $title = $orig_caption;
      if(count($this->activeFilters) > 0) {
        $title .= " (". t("Filtered :") . " " . implode(" ", $this->appliedFilters) . ")";
      }
      if ($this->rank_result > 0) {
        $title .= " - ". t("result !rank of !total",
          array("!rank" => $this->rank_result, "!total" => $this->total_results));
      }
      else {
        $title .= " - ". t("unmatched result");
      }
      $next = NULL;
      $prev = NULL;
      if ($this->total_results > 0) {
        reset($orig_rows);
        $ikey = $this->key;
        foreach ($orig_rows as $key => $row) {
          if ($row->$ikey == $this->mode_context["item_id"]) {
            if (isset($rows[$key - 1])) {
              $prev["rows"] = $rows[$key - 1];
              $prev["link"] = $this->mode_context["entity_type"] . "/". $orig_rows[$key - 1]->$ikey;
              $prev["label"] = $this->extraCaption["prev_label"];
            }
            if (isset($rows[$key + 1])) {
              $next["rows"] = $rows[$key + 1];
              $next["link"] = $this->mode_context["entity_type"] . "/". $orig_rows[$key + 1]->$ikey;
              $next["label"] = $this->extraCaption["next_label"];
            }
            break;
          }
        }
      }
      foreach($this->columns as $key => $column) {
        if (!$column["itemable"]) {
          unset($prev["rows"][$key]);
          unset($next["rows"][$key]);
        }
      }
      $datagrid = array(
        "#theme" => "datagrid_item_navigation",
        "#title" => array(
          "datagrid" => $title,
          "node" => isset($this->linked_node) ? l($this->linked_node->title, "node/".$this->linked_node->nid) : NULL,
          "custom" => isset($this->mode_context["title"]) ? $this->mode_context["title"] : NULL,
          "list" => count($this->activeFilters) == 0 ?
            l($this->extraCaption["backlink"], "node/" . $this->linked_node->nid,
              array("fragment" => $this->id, "attributes" => array("class" => "back")))
            : l($this->extraCaption["filtered_backlink"], "node/" . $this->linked_node->nid,
              array("fragment" => $this->id, "attributes" => array("class" => "back"))) . " | ".
              l($this->extraCaption["unfiltered_backlink"], "node/" . $this->linked_node->nid,
              array("fragment" => $this->id, "query" => array("reset" => 1), "attributes" => array("class" => "back"))),
          ),
        "#next" => $next,
        "#prev" => $prev
      );
    }
    else {
      $datagrid = array(
        "#prefix" => "<div id='" . $this->id . "' class='kw-datagrid'>",
        "#suffix" => "</div>",
        "#weight" => $this->weight
      );
      if ($this->titleMode != "caption" && !is_null($this->title) && trim($this->title) != "") {
        $datagrid["title"] = array(
          "#markup" => "<" . $this->titleMode . ">" . $this->title . "</"
            . (substr_count($this->titleMode, " ") == 0 ? $this->titleMode : substr($this->titleMode, 0, strpos($this->titleMode, " ", $needle))) . ">"
        );
      }
      if($this->mode == "mass-edition") {
        $datagrid["form"] = array(
          "#markup" => drupal_render(
            drupal_get_form("kw_datagrid_mass_edit_form",
              array("datagrid" => $this, "rows" => $rows)
            )
          )
        );
      }
      else {
        if(!is_null($this->intro)) {
          $datagrid["intro"] = array(
            "#markup" => "<div class='datagrid-intro'>" . $this->intro . "</div>"
          );
        }
        $datagrid["grid"] = array(
          "#theme" => "datagrid",
          "#datagrid" => $this,
          "#rows" => $rows
        );
        if(!is_null($this->outro)) {
          $datagrid["outro"] = array(
            "#markup" => "<div class='datagrid-outro'>" . $this->outro . "</div>"
          );
        }
      }
    }
    return $datagrid;
  }

  private function proceedArgsReplacement($name, $args, $row, $pattern) {
    static $matches = array();
    if (!is_array($args)) {
      return array();
    }
    foreach ($args as $key => $arg) {
      if (is_array($arg)) {
        $arg = $this->proceedArgsReplacement($name . "_". $key, $arg, $row, $pattern);
      }
      elseif (is_string($arg)) {
        if (!isset($matches[$name][$key])) {
          preg_match_all($pattern, $arg, $matches[$name][$key], PREG_SET_ORDER);
        }
        if (is_array($matches[$name][$key])) {
          foreach ($matches[$name][$key] as $match) {
            if (isset($this->replacements[$match[1]])) {
              $clb_args = $this->replacements[$match[1]]["args"];
              $clb_args = $this->proceedArgsReplacement($match[1], $clb_args, $row, $pattern);
              $rpl_clb = call_user_func_array($this->replacements[$match[1]]["function"], $clb_args);
              $arg = str_replace($match[0], $rpl_clb, $arg);
            }
            if (property_exists($row,$match[1])) {
              $arg = str_replace($match[0], $row->$match[1], $arg);
            }
          }
        }
      }
      $args[$key] = $arg;
    }
    return $args;
  }

  function getCurrentSort() {
    // CAS 0 : on a déjà appelé la fonction...
    if(!is_null($this->current_sort)) {
      return $this->current_sort;
    }
    // CAS 2 : valeur envoyée par adresse
    $order = isset($_GET['sort']) ? $_GET['sort'] : FALSE;
    if($order !== FALSE && in_array(abs($order), $this->sortables["id"])) {
      $this->current_sort = $order;
      if($this->persistant) {
        $this->storeInSession("sort", $this->current_sort);
      }
      return $this->current_sort;
    }
    // CAS 3 : valeur enregistrée en session
    if($this->persistant) {
      $session_data = kw_datagrid_retrieve_datagrid_data($this, "sort");
      if($session_data !== FALSE && in_array(abs($session_data),$this->sortables["id"])) {
        $this->current_sort = $session_data;
        return $this->current_sort;
      }
    }
    // CAS 4 : valeur par défaut
    $defaultOrder = $this->getDefaultOrders();
    $defaults = array();
    foreach ($defaultOrder as $default) {
      $index = $this->findSortable($default["column"]);
      if($default["order"] == "desc") {
        $index = -$index;
      }
      $this->current_sort = $index;
      return $this->current_sort;
    }
    return 0;
  }

  private function createQuery() {
    $main_table_alias = array_search($this->table, $this->tables);
    $query = db_select($this->table, $main_table_alias);
    $groupier = FALSE;
    $count_method = "classic";
    // Ajout des jointures
    $tables = array($main_table_alias);
    foreach ($this->tables as $alias => $table) {
      if (is_array($table) && $table != $this->table) {
        // Ne pas ajouter de jointures si la colonne correspondante n'est pas utilisée
        if (is_array($table["linked_columns"]) && count($table["linked_columns"]) > 0) {
          $is_linked = FALSE;
          foreach ($table["linked_columns"] as $linked_column) {
            if (isset($this->fields[$linked_column]) && !$this->fields[$linked_column]["hidden"]) {
              $is_linked = TRUE;
              break;
            }
          }
          if (!$is_linked) {
            $sort = array_search(abs($this->getCurrentSort()), $this->sortables["id"]);
            if (count(array_intersect($table["linked_columns"], $this->sortables["aliases"][$sort])) > 0) {
              $is_linked = TRUE;
            }
          }
          if (!$is_linked && count($this->activeFilters) > 0) {
            foreach ($this->activeFilters as $activeFilter) {
              $filterKey = array_search($activeFilter["filter_id"], $this->filters["id"]);
              if (count(array_intersect($table["linked_columns"], $this->filters["aliases"][$filterKey])) > 0) {
                $is_linked = TRUE;
                break;
              }
            }
          }
          if (!$is_linked) {
            continue;
          }
        }
        $tables[] = $alias;
        // Traitement des différents cas de jointure
        switch ($table["type_jointure"]) {
          case ("left"):
            $query->leftJoin($table["table"], $alias, $table["jointure"], $table["args"]);
            break;
          case ("right"):
            $query->rightJoin($table["table"], $alias, $table["jointure"], $table["args"]);
            break;
          case ("inner"):
            $query->innerJoin($table["table"], $alias, $table["jointure"], $table["args"]);
            break;
          default:
            $query->join($table["table"], $alias, $table["jointure"], $table["args"]);
        }
      }
      // Ajout des groupements
      if(is_array($table) && $table["group_by"]) {
        $groupier = TRUE;
        if ($table["count_method"] != "classic") {
          $count_method = $table["count_method"];
        }
      }
    }
    // Ajout des conditions de base
    foreach ($this->conditions as $key => $condition) {
      $query->condition($condition["field"], $condition["value"], $condition["operand"]);
    }
    // Ajout des champs
    foreach ($this->fields as $alias => $column) {
      if ($column["hidden"] || !in_array($column["table_alias"], $tables)) {
        continue;
      }
      if ($column["selectable"]) {
        if (!is_null($column["expression"])) {
          $query->addExpression($column["expression"], $alias);
        }
        else {
          $query->addField($column["table_alias"], $column["table_field"], $alias);
        }
        if ($groupier && is_null($column["expression"])) {
          $query->groupBy($column["table_alias"] . "." .$column["table_field"]);
        }
      }
    }
    // Ajout des conditions filtrées
    if (count($this->activeFilters) > 0) {
      if($this->mode != "item") {
        $unfiltered_query = $query;
        // Comptage des résultats non filtrés
        $count_query = $unfiltered_query->countQuery();
        if ($groupier && $count_method != "classic") {
          $count_results = $count_query->execute()->rowCount();
        }
        else {
          $count_results = $count_query->execute()->fetchField();
        }
        $this->unfiltered_total_results = $count_results;
      }
      $logicalAnd = db_and();
      $logicalOr = db_or();
      $i = 0;
      foreach ($this->activeFilters as $key => $activeFilter) {
        if (is_null($activeFilter["value"]) || trim($activeFilter["value"]) == "") {
          continue;
        }
        $filterKey = array_search($activeFilter["filter_id"], $this->filters["id"]);
        if ($filterKey === FALSE) {
          continue;
        }
        $expression = array();
        $logical1 = db_and();
        if($this->filters["type"][$filterKey] == KW_FILTRABLE_NUM) {
          $activeFilter["value"] = str_replace(array(" ",","),array("","."), $activeFilter["value"]);
          if (!is_numeric($activeFilter["value"])) {
            $activeFilter["value"] = 0;
          }
        }
        switch ($activeFilter["op"]) {
          case("CONTAIN"):
            $op = "LIKE";
            $activeFilter["value"] = "%" . $activeFilter["value"] . "%";
            $expression[] = $activeFilter["value"];
            break;
          case("NOT_CONTAIN"):
            $op = "NOT LIKE";
            $activeFilter["value"] = "%" . $activeFilter["value"] . "%";
            $expression[] = $activeFilter["value"];
            break;
          case("BEGIN"):
            $op = "LIKE";
            $activeFilter["value"] .= "%";
            $expression[] = $activeFilter["value"];
          break;
          case("NOT_BEGIN"):
            $op = "NOT LIKE";
            $activeFilter["value"] .= "%";
            $expression[] = $activeFilter["value"];
            break;
          case("END"):
            $op = "LIKE";
            $activeFilter["value"] = "%" . $activeFilter["value"];
            $expression[] = $activeFilter["value"];
            break;
          case("NOT_END"):
            $op = "NOT LIKE";
            $activeFilter["value"] = "%" . $activeFilter["value"];
            $expression[] = $activeFilter["value"];
            break;
          case("CONTAIN_ONE"):
            $op = "LIKE";
            $expression = explode(" ", $activeFilter["value"]);
            foreach($expression as $ke => $ve) {
              $expression[$ke] = "%" . $ve . "%";
            }
            $logical1 = db_or();
            break;
          case("NOT_CONTAIN_ONE"):
            $op = "NOT LIKE";
            $expression = explode(" ", $activeFilter["value"]);
            foreach($expression as $ke => $ve) {
              $expression[$ke] = "%" . $ve . "%";
            }
            break;
          case("CONTAIN_ALL"):
            $op = "LIKE";
            $expression = explode(" ", $activeFilter["value"]);
            foreach($expression as $ke => $ve) {
              $expression[$ke] = "%" . $ve . "%";
            }
            break;
          case("NOT_CONTAIN_ALL"):
            $op = "NOT LIKE";
            $expression = explode(" ", $activeFilter["value"]);
            foreach($expression as $ke => $ve) {
              $expression[$ke] = "%" . $ve . "%";
            }
            $logical1 = db_or();
            break;
          case("NOT_EQUAL"):
            $op = "<>";
            $expression[] = $activeFilter["value"];
            break;
          case("SUPERIOR"):
            $op = ">";
            $expression[] = $activeFilter["value"];
            break;
          case("SUPERIOR_EQUALS"):
            $op = ">=";
            $expression[] = $activeFilter["value"];
            break;
          case("INFERIOR"):
            $op = "<";
            $expression[] = $activeFilter["value"];
            break;
          case("INFERIOR_EQUALS"):
            $op = "<=";
            $expression[] = $activeFilter["value"];
            break;
          default:
            $op = "=";
            $expression[] = $activeFilter["value"];
        }
        if (count($this->filters["aliases"][$filterKey]) == 1) {
          $alias = $this->filters["aliases"][$filterKey][0];
          if (isset($this->fields[$alias])) {
            foreach($expression as $exp) {
              $logical1->condition($this->fields[$alias]["table_alias"] . "."
              . $this->fields[$alias]["table_field"], $exp, $op);
            }
          }
          elseif (isset($this->filters["options"][$filterKey]["jointure"])) {
            $jointure = $this->filters["options"][$filterKey]["jointure"];
            $query->leftJoin($jointure["table"], $jointure["alias"] . $i,
              str_replace($jointure["alias"] . ".", $jointure["alias"] . $i . ".", $jointure["condition"])
              . " AND " . $jointure["alias"] . $i . "." . $alias . "=:filter_key" . $i,
              array(":filter_key" . $i => $expression[0]));
            if ($op == "=") {
              $logical1->isNotNull($jointure["alias"] . $i . "." . $alias);
            }
            else {
              $logical1->isNull($jointure["alias"] . $i . "." . $alias);
            }
          }
        }
        else {
          $logical2 = db_or();
          foreach ($this->filters["aliases"][$filterKey] as $alias) {
            if (isset($this->fields[$alias])) {
              $logical2->condition($this->fields[$alias]["table_alias"] ." . " . $this->fields[$alias]["table_field"],
                $activeFilter["value"], $op
              );
            }
          }
          $logical1->condition($logical2);
        }
        $string = "";
        if ($i == 0 || $activeFilter["mode"] == "OR") {
          $logicalOr->condition($logical1);
        }
        else {
          $logicalAnd->condition($logical1);
        }
        if ($i > 0) {
          $string .= t($activeFilter["mode"])." ";
        }
        $ops = kw_datagrid_get_operators($this->filters["type"][$filterKey]);
        if ($this->filters["type"][$filterKey] == KW_FILTRABLE_ENUM) {
          $choices = kw_datagrid_get_enum_choices($this->filters, $filterKey);
          if (isset($choices[$activeFilter["value"]])) {
            $activeFilter["value"] = $choices[$activeFilter["value"]];
          }
        }
        elseif (isset($this->filters["options"][$filterKey]["choices"]) && isset($this->filters["options"][$filterKey]["choices"][$activeFilter["value"]])) {
          $activeFilter["value"] = $this->filters["options"][$filterKey]["choices"][$activeFilter["value"]];
        }
        $string .= t("<em>!field</em> !op %value", array(
          "!field" => drupal_strtolower($this->filters["label"][$filterKey]),
          "!op" => $ops[$activeFilter["op"]],
          "%value" => str_replace("%", "", $activeFilter["value"])
        ));
        $this->appliedFilters[] = $string;
        $i++;
      }
      if ($i>0) {
        $logicalAnd->condition($logicalOr);
        $query->condition($logicalAnd);
      }
    }
    // Comptage des résultats filtrés
    if ($this->mode != "item") {
      $count_query = $query->countQuery();
      if($groupier && $count_method != "classic") {
        $count_results = $count_query->execute()->rowCount();
      }
      else {
        $count_results = $count_query->execute()->fetchField();
      }
      $this->total_results = $count_results;
      if($this->unfiltered_total_results == 0) {
        $this->unfiltered_total_results = $this->total_results;
      }
      if ($count_results == 0) {
        return FALSE;
      }
    }
    // Tri défini par l'utilisateur
    $orders = array();
    $ranking_orders = array();
    if ($this->getCurrentSort() != 0) {
      $user_asc = $this->getCurrentSort() > 0 ? "asc" : "desc";
      $id = array_search(abs($this->getCurrentSort()), $this->sortables["id"]);
      foreach($this->sortables["aliases"][$id] as $alias) {
        if (isset($this->fields[$alias])) {
          $orders[] = $alias;
          if(is_null($this->fields[$alias]["sort_expression"])) {
            $q_order = $this->fields[$alias]["table_alias"] . "." . $this->fields[$alias]["table_field"];
            $ranking_orders[$q_order] = $user_asc;
          }
          else {
            $q_order = $this->fields[$alias]["sort_expression"];
          }
          $query->orderBy($q_order, $user_asc);
        }
      }
    }
    // Tri par défaut
    foreach ($this->defaultOrders as $key => $order) {
      if (isset($this->fields[$order["column"]]) && !in_array($order["column"], $orders)) {
        $q_order = $this->fields[$order["column"]]["table_alias"] . "."
        . $this->fields[$order["column"]]["table_field"];
        $query->orderBy($q_order, $order["order"]);
        $ranking_orders[$q_order] = $order["order"];
      }
    }
    if ($this->mode == "item" && !is_null($this->key)) {
      $ranking_query = clone $query;
      $sql_key = $this->fields[$this->key]["table_alias"] . "." . $this->fields[$this->key]["table_field"];
      $sql_order = " ORDER BY ";
      $ir = 0;
      foreach($ranking_orders as $order => $sort) {
        if ($ir > 0) {
          $sql_order .= ",";
        }
        $sql_order .= $order." ".$sort;
        $ir++;
      }
      $ranking_query->addExpression("GROUP_CONCAT(DISTINCT ".$sql_key.$sql_order.")", "concat");
      $ranking_query->addExpression("COUNT(DISTINCT ".$sql_key.")", "total");
      $ranking_query_altered = $ranking_query->__toString();
      $ranking_query_altered = "SELECT ".substr($ranking_query_altered, strpos($ranking_query_altered, "GROUP_CONCAT("));
      $ranking_query_altered = substr($ranking_query_altered, 0, strrpos($ranking_query_altered, "ORDER BY"));
      if ($groupier) {
        $ranking_query_altered = substr($ranking_query_altered, 0, strrpos($ranking_query_altered, "GROUP BY"));
      }
      db_query("SET @@group_concat_max_len := @@max_allowed_packet");
      $result_ranking_query = db_query("SELECT FIND_IN_SET(".$sql_key.",concat) as rank, total FROM {" . $this->tables[$this->fields[$this->key]["table_alias"]] . "} " . $this->fields[$this->key]["table_alias"] . " "
        ."CROSS JOIN (".$ranking_query_altered.") concat WHERE ".$sql_key."=:nid",
        array_merge($ranking_query->getArguments(), array(":nid" => $this->mode_context["item_id"])));
      $results = $result_ranking_query->fetch();
      $this->rank_result = $rank = $results->rank;
      $this->total_results = $results->total;
      $count_results = $this->unfiltered_total_results = $rank > 0 ? 1 : 0;
    }
    // Prise en compte du pager
    $this->current_page = $this->getCurrentPage();
    if ($this->mode == "item") {
      $current_start = max(0, $rank - 2);
    }
    else {
      $current_start = $this->current_page * $this->pager;
    }
    if ($this->pager > 0) {
      $query->range($current_start, ($this->mode == "item" ? 3 : $this->pager));
    }
    //echo str_replace(array("{","}"),array("dp_",""),$query);die();
    return $query;
  }

  private function storeInSession($var, $value, $save_in_pool = TRUE) {
    if($save_in_pool) {
      $_SESSION["kw_datagrid"][$this->pool][$var] = $value;
    }
    else {
      $_SESSION["kw_datagrid"][$this->id][$var] = $value;
    }
  }

  private function isStoredInSession($var) {
    if(isset($_SESSION["kw_datagrid"][$this->pool][$var])) {
      return TRUE;
    }
    else if (isset($_SESSION["kw_datagrid"][$this->id][$var])) {
      return TRUE;
    }
    return FALSE;
  }

  public function listSelectableColumns() {
    $selectables = array();
    foreach($this->columns as $alias => $column) {
      if (!$column["uncacheable"]) {
        $selectables[$alias] = $column["label"];
      }
    }
    return $selectables;
  }

  public function listSelectedColumns() {
    $selected = array();
    foreach($this->columns as $alias => $column) {
      if (!$column["hidden"]) {
        $selected[$alias] = $column["label"];
      }
    }
    return $selected;
  }

  public function storeSelectedColumns($columns) {
    if($this->persistant) {
      $this->storeInSession("columns", $columns);
    }
  }

  public function retrieveSelectedColumns($input = NULL) {
    $columns = FALSE;
    if (is_array($input)) {
      $columns = $input;
    }
    elseif ($this->persistant) {
      $columns = kw_datagrid_retrieve_datagrid_data($this, "columns");
    }
    elseif (isset($_GET["columns"])) {
      $columns = unserialize($_GET["columns"]);
    }
    return $columns;
  }

  public function retrieveModifiableColumnInfo() {
    if ($this->mode == "mass-edition") {
      $options = isset($this->columns[$this->mode_context["column"]]["modifiable_options"]) ? $this->columns[$this->mode_context["column"]]["modifiable_options"] : array();
      return array(
        "column" => $this->mode_context["column"],
        "entity" => isset($options["entity"]) ? $options["entity"] : "node",
        "field" =>  isset($options["field"]) ? $options["field"] : $this->mode_context["column"],
        "bundle" => isset($options["bundle"]) ? $options["bundle"] :
          (!is_null($this->linked_node) ? $this->linked_node->type : "page")
      );
    }
    return array();
  }

  public function reset() {
    if (isset($_SESSION["kw_datagrid"][$this->id])) {
      unset($_SESSION["kw_datagrid"][$this->id]);
    }
  }
}