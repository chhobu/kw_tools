<?php
define('KW_FILTRABLE_NO', 0);
define('KW_FILTRABLE_STRING', 1);
define('KW_FILTRABLE_DATE', 2);
define('KW_FILTRABLE_DATETIME', 5);
define('KW_FILTRABLE_NUM', 3);
define('KW_FILTRABLE_ENUM', 4);

class KwDatagrid {
  /** @var string identifant de la table */
  protected $id;
  /** @var array Fonction où est appelée la grille */
  protected $constructor;
  /** @var string Table principale de la BdD de la requête */
  protected $table;
  /** @var array Tables supplémentaires de la BdD présentes dans la requête */
  protected $tables = array();
  /** @var array En-tête du tableau */
  protected $headers = array();
  /** @var array Conditions présentes sur la requête */
  protected $conditions = array();
  /** @var array Champs issus de la requête */
  protected $fields = array();
  /** @var array Colonnes de la grille */
  protected $columns = array();
  /** @var array Colonnes par défaut */
  protected $defaultColumns = array();
  /** @var array Tri par défaut */
  protected $defaultOrders = array();
  /** @var  string */
  protected $defaultOrderIndex;
  /** @var array Attributs HTML présents sur le tableau */
  protected $attributes = array();
  /** @var array Liste des filtres disponibles */
  protected $filters = array();
  /** @var array Liste des filtres actifs */
  protected $activeFilters = array();
  /** @var array  Liste des filtres appliqués */
  protected $appliedFilters = array();
  /** @var array  Liste des tris possibles */
  protected $sortables = array();
  /** @var array  Traitements supplémentaires enregistrés pour chaque colonne
   * lors de la transformation requête -> grille */
  protected $replacements = array();
  /** @var string Regroupement de colonnes dans la grille */
  protected $colgroup;
  /** @var string Titre du tableau */
  protected $caption;
  /** @var string Titre global */
  protected $title;
  /** @var string Mode d'affichage du titre : balise HTML ou caption */
  protected $titleMode = 'caption';
  /** @var array Chaînes de texte personnalisables */
  protected $extraCaption;
  /** @var string Texte affiché si aucun résultat */
  protected $emptyText;
  /** @var bool Utilisation de l'en-tête collant des tableaux */
  protected $useSticky = TRUE;
  /** @var bool Utilisation d'Ajax (oui/non) */
  protected $useAjax = TRUE;
  /** @var int Position de la datagrid parmi les autres éléments affichés */
  protected $weight = 0;
  /** @var bool Utilisation des sessions pour conserver les choix effectués */
  protected $persistant = TRUE;
  /** @var int Pager : identifiant du pager (si plusieurs utilisations dans la page) */
  protected $pager = 0;
  /** @var array Options du pager */
  protected $pagerOptions = array('default_value' => 0);
  /** @var int Nombre total de résultats */
  protected $totalResults = 0;
  /** @var int Nombre de résultats sur la requête non filtrée */
  protected $unfilteredTotalResults = 0;
  /** @var int Dans le cas d'un affichage en mode item : position de l'enregistrement courant */
  protected $rankResult = 0;
  /** @var int Page active */
  protected $currentPage = -1;
  /** @var string Tri actif */
  protected $currentSort;
  /** @var string Mode d'affichage : liste ou item ou mass-edition */
  protected $mode = 'list';
  /** @var array Contexte d'affichage du mode : tableau conteant item_id et entity_type */
  protected $modeContext = array();
  /** @var stdClass Dans le cas d'un affichage en mode item : node d'origine */
  protected $linkedNode;
  /** @var string Colonne servant de clé unique pour les résultats de la requête */
  protected $key;
  /** @var string Texte affiché avant la grille */
  protected $intro;
  /** @var string Texte affiché après la grille  */
  protected $outro;
  /** @var bool Présence de colonnes sélectionnables */
  protected $selectableColumns = FALSE;
  /** @var string Utilisation d'un 'pool' pour conserver les préférences utilisateurs d'une grille à l'autre */
  protected $pool;
  /** @var array Blocs dynamiques dépendant des sélections du tableau, affichés avant ou après la grille */
  protected $relatedBlocks = array();
  /** @var array Liste des blocs pouvant être affichés / masqués par l'utilisateur */
  protected $configurableBlocks = array();
  /** @var array Liste des blocs affichés par défaut */
  protected $defaultBlocks = array();
  /** @var string Chemin vers la page de traitement des requêtes */
  protected $path;
  /** @var string Type d'affichage (standard ou advanced) */
  protected $formDisplay = 'standard';
  /** @var int Durée de rafraichissement automatique de la grille (en s) */
  protected $autoRefresh = 0;
  /** @var array  */
  protected $op = array();

  public function __construct($id, $table, $alias, $function, array $args, array $options = array()) {
    foreach ($args as $key => $arg) {
      if (is_array($arg)) {
        trigger_error('Argument #' . $key . ' for datagrid ' . $id . ' can not be an array.', E_USER_ERROR);
      }
      elseif (is_object($arg)) {
        trigger_error('Argument #' . $key . ' for datagrid ' . $id . ' can not be an object.', E_USER_ERROR);
      }
    }
    $this->id = $id;
    $this->table = $table;
    $this->tables[$alias] = $table;
    $this->pool = $id;
    if (isset($options["form_display"])) {
      $this->setFormDisplay($options['form_display']);
    }
    if (isset($options["weight"])) {
      $this->weight = $options["weight"];
    }
    if (isset($options["mode"])) {
      $this->mode = $options["mode"];
    }
    if (isset($options["mode_context"])) {
      $this->modeContext = $options["mode_context"];
    }
    if (isset($options["node"])) {
      $this->linkedNode = $options["node"];
      $home = 'node/' . $this->linkedNode->nid;
      $update_constructor = TRUE;
    }
    elseif (isset($options["home"])) {
      $home = $options["home"];
      $update_constructor = TRUE;
    }
    else {
      $home = $_GET['q'];
      $update_constructor = FALSE;
    }
    if (isset($options["pool"])) {
      $this->pool = $options["pool"];
    }
    if (isset($options["path"])) {
      $path = $options["path"];
    }
    else {
      $path = 'kw_datagrid';
    }
    $this->path = $path . '/' . time();
    if (isset($options["allowable_parameters"]) && is_array($options["allowable_parameters"])) {
      $allowable_parameters = $options["allowable_parameters"];
    }
    else {
      $allowable_parameters = array();
    }
    if (isset($options["constructor_storage"]) && in_array($options["constructor_storage"], array('db', 'session'))) {
      $constructor_storage = $options["constructor_storage"];
    }
    else {
      $constructor_storage = 'db';
    }
    $this->extraCaption = array(
      "no_results" => t("No results found."),
      "single_result" => t("Only 1 result found."),
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
      "prev_label" => t("Previous"),
    );
    if (isset($options["extra_captions"]) && is_array($options["extra_captions"])) {
      $this->extraCaption = array_merge($this->extraCaption, $options["extra_captions"]);
    }
    if (isset($_GET["_r"])) {
      $this->reset($_GET['_r'] > 1 ? 'all' : 'filters');
    }
    $this->constructor = self::retrieveConstructor($this->id);
    if (is_null($this->constructor)) {
      $update_constructor = TRUE;
    }
    elseif (isset($options['file']) && $this->constructor['required_file'] != $options['file']) {
      $update_constructor = TRUE;
    }
    elseif ($update_constructor && $this->constructor['page'] == $home) {
      $update_constructor = FALSE;
    }
    if ($this->mode != "item" && $update_constructor) {
      if (!is_null($this->constructor) && $constructor_storage == 'db') {
        db_delete('kw_datagrid')
          ->condition('name', $this->id)
          ->condition('page', $this->constructor['page'])
          ->execute();
        watchdog('kw_datagrid', "Datagrid !name in page %page updated", array(
            '!name' => $this->id,
            '%page' => $this->constructor['page'],
        ));
      }
      if (!empty($allowable_parameters)) {
        $parameters = drupal_get_query_parameters();
        foreach ($parameters as $key => $value) {
          if (in_array($key, $allowable_parameters)) {
            $parameters[$key] = $value;
          }
        }
      }
      $this->constructor = array(
        'name' => $this->id,
        'function' => $function,
        'args' => !empty($args) ? serialize($args) : NULL,
        'page' => $home,
        'parameters' => !empty($parameters) ? serialize($parameters) : NULL,
        'required_file' => !empty($options['file']) ? $options['file'] : NULL,
      );
      if ($constructor_storage == 'db') {
        db_insert('kw_datagrid')->fields($this->constructor)->execute();
      }
      else {
        $this->storeInSession('constructor', $this->constructor);
      }
    }
    return $this;
  }

  public static function retrieveConstructor($id) {
    $constructor = NULL;
    if (!empty($_SESSION[$id]['constructor'])) {
      return $_SESSION[$id]['constructor'];
    }
    $q = db_select('kw_datagrid', 'k');
    $q->fields('k', array('function', 'args', 'page', 'parameters', 'required_file'));
    $q->condition('name', $id);
    $result = $q->execute()->fetchAssoc();
    if ($result) {
      $constructor = array(
        'function' => $result['function'],
        'args' => !empty($result['args']) ? unserialize($result['args']) : NULL,
        'page' => $result['page'],
        'parameters' => !empty($result['parameters']) ? unserialize($result['parameters']) : NULL,
        'required_file' => $result['required_file'],
      );
    }
    return $constructor;
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
    if (!is_null($key)) {
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

  public function getAutoRefresh() {
    return $this->autoRefresh;
  }

  public function getCurrentPage() {
    if ($this->pager == 0) {
      $this->currentPage = -1;
    }
    elseif ($this->currentPage == -1) {
      $p = isset($_GET["_p"]) ? $_GET["_p"] : NULL;
      if (!is_null($p) && is_numeric($p) && $p >= 0) {
        $this->currentPage = $p;
        if ($this->persistant) {
          $this->storeInSession("p", $p, FALSE, 0);
        }
      }
      elseif ($this->persistant && $this->isStoredInSession("p")) {
        $this->currentPage = $this->retrieveSessionData("p");
      }
      if ($this->currentPage < 0) {
        $this->currentPage = 0;
      }
    }
    if ($this->totalResults > 0 && $this->pager > 0 && $this->currentPage >= ceil($this->totalResults / $this->pager)) {
      $this->currentPage = ceil($this->totalResults / $this->pager) - 1;
    }
    return $this->currentPage;
  }

  public function getUnfilteredTotalResults() {
    return $this->unfilteredTotalResults;
  }

  public function getTotalResults() {
    return $this->totalResults;
  }

  public function isUsingAjax() {
    return ($this->useAjax ? TRUE : FALSE);
  }

  public function setUseAjax($bool) {
    $this->useAjax = $bool;
    return $this;
  }

  public function isUsingSticky() {
    return ($this->useSticky ? TRUE : FALSE);
  }

  public function setUseSticky($bool) {
    $this->useSticky = $bool;
    return $this;
  }

  public function getFormDisplay() {
    return $this->formDisplay;
  }

  public function setFormDisplay($display) {
    if (in_array($display, array('standard', 'advanced'))) {
      $this->formDisplay = $display;
    }
    return $this;
  }

  public function hasOp($op) {
    return in_array($op, $this->op);
  }

  public function getPath() {
    return $this->path;
  }

  public function getPager() {
    return $this->pager;
  }

  public function getPagerOptions($variable = 'choices') {
    if (!is_null($variable) && isset($this->pagerOptions[$variable])) {
      return $this->pagerOptions[$variable];
    }
    return $this->pagerOptions;
  }

  public function isPersitant() {
    return ($this->persistant ? TRUE : FALSE);
  }

  public function setPersistant($bool) {
    $this->persistant = $bool;
    return $this;
  }

  public function addPager($pager, $options = array()) {
    if (!is_numeric($pager)) {
      $pager = 0;
    }
    $this->pager = $pager;
    if (empty($options['choices']) || !is_array($options['choices'])) {
      $options['choices'] = array(5, 10, 20, 30, 50, 100);
    }
    if (empty($options['display']) || !in_array($options['display'], array('none', 'form', 'advanced'))) {
      $options['display'] = 'none';
    }
    if (empty($options['quantity']) || $options['quantity'] <= 0) {
      $options['quantity'] = 9;
    }
    $options['default_value'] = $pager;
    $this->pagerOptions = $options;
    return $this;
  }

  public function addAutoRefresh($delay) {
    $this->autoRefresh = max(1, $delay);
  }

  public function addCondition($field, $value = NULL, $operand = NULL) {
    $this->conditions[] = array(
      "field" => $field,
      "value" => $value,
      "operand" => $operand,
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
        "count_method" => $count_method,
      );
    }
    return $this;
  }

  public function addKey($alias, $table_alias = NULL, $field = NULL, $options = array()) {
    if (is_null($table_alias)) {
      $table_alias = key($this->tables);
    }
    $this->addField($alias, $table_alias, $field, $options);
    $this->key = $alias;
    return $this;
  }

  public function addField($alias, $table_alias, $field = NULL, $options = array()) {
    if (is_null($field) && !is_null($table_alias)) {
      $field = $alias;
    }
    $this->fields[$alias] = array(
      "table_alias" => $table_alias,
      "table_field" => $field,
      "selectable" => isset($options['selectable']) ? $options['selectable'] : TRUE,
      "sortable" => isset($options['sortable']) ? $options['sortable'] : FALSE,
      "filtrable" => isset($options['filtrable']) ? $options['filtrable'] : KW_FILTRABLE_NO,
      "hidden" => isset($options['hidden']) ? $options['hidden'] : FALSE,
      "always_execute" => isset($options['always_execute']) ? $options['always_execute'] : FALSE,
      "expression" => isset($options['expression']) ? $options['expression'] : NULL,
      "sort_expression" => isset($options['sort_expression']) ? $options['sort_expression'] : NULL,
    );
    return $this;
  }

  public function addExpression($alias, $expression, $options = array()) {
    $options['expression'] = $expression;
    $this->addField($alias, NULL, NULL, $options);
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

  public function addRankingColumn($options = array('uncacheable' => TRUE), $label = "#", $output = NULL) {
    if (empty($output)) {
      $output = "<div class='digit rank'>!rank</div>";
    }
    $this->addColumn("!rank", $label, $output, NULL, NULL, $options);
    return $this;
  }

  public function addColumn($alias, $label, $output, $options = NULL) {
    $sortable = FALSE;
    $filtrable = KW_FILTRABLE_NO;
    $filtrable_options = array();
    $itemable = FALSE;
    $hidden = FALSE;
    $main = FALSE;
    $uncacheable = FALSE;
    $expression = NULL;
    $modifiable = FALSE;
    $always_execute = FALSE;
    $modifiable_options = array();
    $table = NULL;
    $field = NULL;
    $weight = count($this->columns);
    $sort_expression = NULL;
    $header = NULL;
    if (is_array($options)) {
      if (isset($options['field'])) {
        if (substr_count($options['field'], '.') > 0) {
          $table = substr($options['field'], 0, strpos($options['field'], '.'));
          $field = substr($options['field'], strpos($options['field'], '.') + 1);
        }
        else {
          $field = $options['field'];
        }
      }
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
      if (isset($options["always_execute"])) {
        $always_execute = $options["always_execute"];
      }
      if (isset($options["main"])) {
        $main = $options["main"];
      }
      if (isset($options["uncacheable"])) {
        $uncacheable = $options["uncacheable"];
      }
      if (!empty($options["expression"])) {
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
      if (isset($options["weight"])) {
        $weight = $options["weight"];
        $op['reorder_columns'] = TRUE;
      }
      if (isset($options['header'])) {
        $header = $options['header'];
      }
    }
    if ($main) {
      $uncacheable = TRUE;
      $modifiable = FALSE;
    }
    $this->columns[$alias] = array(
      'label' => $label,
      'header' => $header,
      'is_db_field' => !is_null($table) && !is_null($field) ? TRUE : FALSE,
      'is_expression' => !is_null($expression) ? TRUE : FALSE,
      'output' => $output,
      'itemable' => $itemable,
      'main' => $main,
      'hidden' => $hidden,
      'always_execute' => $always_execute,
      'uncacheable' => $uncacheable,
      'modifiable' => $modifiable,
      'modifiable_options' => $modifiable_options,
      'weight' => $weight,
    );
    if (!$this->columns[$alias]['uncacheable'] && !$this->columns[$alias]['hidden']) {
      $this->defaultColumns[] = $alias;
    }
    if ($this->columns[$alias]["is_db_field"] || $this->columns[$alias]['is_expression']) {
      $this->addField($alias, $table, $field, array(
          'selectable' => TRUE,
          'sortable' => $sortable,
          'filtrable' => $filtrable,
          'hidden' => $hidden,
          'always_execute' => $always_execute,
          'expression' => $expression,
          'sort_expression' => $sort_expression,
      ));
    }
    if ($hidden) {
      $this->selectableColumns = TRUE;
    }
    if ($sortable) {
      $this->addSortable($alias, $label, 0, FALSE);
    }
    if ($filtrable) {
      $this->addFilter($alias, $label, $filtrable, $filtrable_options);
    }
    return $this;
  }

  protected function reorderColumns() {
    $cols['labels'] = array();
    $cols['weights'] = array();
    $cols['datas'] = array();
    foreach ($this->columns as $key => $column) {
      $cols['weights'][$key] = $column['weight'];
      $cols['labels'][$key] = $column['label'];
      $cols['datas'][$key] = $column;
    }
    array_multisort($cols['weights'], $cols['labels'], $cols['datas']);
    $this->columns = $cols['datas'];
  }

  public function hasSelectablesColumns() {
    return $this->selectableColumns;
  }

  public function toggleSelectableColumns($value = NULL) {
    if (!is_null($value)) {
      $this->selectableColumns = $value;
    }
    else {
      $this->selectableColumns = !$this->selectableColumns;
    }
  }

  public function addVariable($variable, $options) {
    if (isset($options['subquery'])) {
      $subquery = $options['subquery'];
      $key = !empty($options['subquery_key']) ? $options['subquery_key'] : 0;
      $output = !empty($options['subquery_output']) ? $options['subquery_output'] : 1;
      $output_mode = !empty($options['output_mode']) ? $options['output_mode'] : 'comma-separated';
      $input = !empty($options['subquery_input']) ? $options['subquery_input'] : $this->key;
      $args = !empty($options['subquery_args']) && is_array($options['subquery_args']) ? $options['subquery_args'] : array();
      $callback = !empty($options['subquery_callback']) ? $options['subquery_callback'] : NULL;
      $export = !empty($options['export']) && is_array($options['export']) ? $options['export'] : array();
      $this->replacements[$variable] = array(
        'type' => 'query',
        'output' => $output,
        'key' => $key,
        'query' => $subquery,
        'args' => $args,
        'input' => $input,
        'callback' => $callback,
        'export' => $export,
        'output_mode' => $output_mode,
      );
    }
    elseif (isset($options['callback'])) {
      $callback = $options['callback'];
      $args = !empty($options['args']) && is_array($options['args']) ? $options['args'] : array();
      $this->replacements[$variable] = array(
        'type' => 'callback',
        'callback' => $callback,
        'args' => $args,
      );
    }
    elseif (isset($options['final_callback'])) {
      $callback = $options['final_callback'];
      $args = !empty($options['final_args']) && is_array($options['final_args']) ? $options['final_args'] : array();
      $this->replacements[$variable] = array(
        'type' => 'final_callback',
        'callback' => $callback,
        'args' => $args,
      );
    }
    return $this;
  }

  public function addDefaultOrder($alias, $order = "asc") {
    $this->defaultOrders[] = array(
      "column" => $alias,
      "order" => $order,
    );
    return $this;
  }

  public function getDefaultOrders() {
    return $this->defaultOrders;
  }

  public function addSortable($aliases, $label, $weight = 0, $direct_call = TRUE) {
    if (!is_array($aliases)) {
      $aliases = array($aliases);
    }
    sort($aliases);
    if (!isset($this->sortables["aliases"]) || array_search($aliases, $this->sortables["aliases"]) === FALSE) {
      $this->sortables["aliases"][] = $aliases;
      $this->sortables["label"][] = $label;
      $this->sortables["weight"][] = $weight;
      $this->sortables["specific"][] = $direct_call;
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
    if (!empty($options['id'])) {
      $filter_id = $options['id'];
      unset($options['id']);
    }
    else {
      $filter_id = implode('_', $aliases);
    }
    $this->filters["aliases"][] = $aliases;
    $this->filters["label"][] = $label;
    $this->filters["type"][] = $type;
    $this->filters["weight"][] = $weight;
    $this->filters["options"][] = $options;
    $this->filters["id"][] = $filter_id;
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

  public function addActiveFilter($key, $mode = NULL, $op = NULL, $value = NULL) {
    if (count($this->activeFilters) > 50) {
      drupal_set_message("Too much filters !", "error");
      return $this;
    }
    $this->activeFilters[] = array(
      "filter_id" => $key,
      "mode" => $mode,
      "op" => $op,
      "value" => $value,
    );
    return $this;
  }

  public function removeActiveFilter($index) {
    if (isset($this->activeFilters[$index])) {
      unset($this->activeFilters[$index]);
    }
    if ($this->persistant) {
      $this->storeInSession('filters', $this->activeFilters);
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

  public function setEmptyText($empty_text) {
    $this->emptyText = $empty_text;
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

  public function buildFilterForm() {
    return drupal_get_form('kw_datagrid_filter_form', array('datagrid' => $this, 'filters_only' => TRUE));
  }

  protected function buildForm() {
    static $form = array();
    if (!isset($form[$this->id])) {
      $has_filters = isset($this->filters['aliases']) && count($this->filters['aliases']) > 0 ? TRUE : FALSE;
      $has_pager_choices = !empty($this->pagerOptions) && $this->pagerOptions['display'] == 'form' && count($this->pagerOptions['choices']) > 0 ? TRUE : FALSE;
      $has_sort_options = !empty($this->sortables) && count(array_keys($this->sortables['specific'], TRUE)) > 0 ? TRUE : FALSE;
      if ($has_filters || $this->selectableColumns || $has_pager_choices || $has_sort_options) {
        $form[$this->id] = drupal_get_form('kw_datagrid_filter_form', array('datagrid' => $this));
      }
      else {
        $form[$this->id] = NULL;
      }
    }
    return $form[$this->id];
  }

  public function build() {
    $target_grid = $this->id;
    if (isset($_GET["_id"])) {
      $target_grid = $_GET["_id"];
    }
    // Récupération du mode
    if (!is_null($this->key) && $target_grid == $this->id && isset($_GET["mass_edit"]) && isset($this->columns[$_GET["mass_edit"]])
      && $this->columns[$_GET["mass_edit"]]["modifiable"] && $this->getAliasField($_GET["mass_edit"])) {
      $this->mode = "mass-edition";
      $this->modeContext = array("column" => $_GET["mass_edit"]);
    }
    // Récupération de la valeur du pager
    if ($target_grid == $this->id && isset($_GET["_g"]) && is_numeric($_GET["_g"]) && $_GET["_g"] >= 0) {
      $this->pager = $_GET["_g"];
      if ($this->persistant) {
        $this->storeInSession("pager", $_GET["_g"], TRUE, $this->getPagerOptions('default_value'));
      }
    }
    elseif ($this->persistant && $this->isStoredInSession("pager")) {
      $this->pager = $this->retrieveSessionData("pager");
    }
    // Récupération des colonnes affichées
    if (!empty($this->op['reorder_columns'])) {
      $this->reorderColumns();
    }
    if ($this->mode == "mass-edition") {
      foreach ($this->columns as $alias => $column) {
        if ($column["uncacheable"] || $alias == $this->modeContext["column"]) {
          $this->columns[$alias]["hidden"] = FALSE;
          if ($column["is_db_field"] || $column['is_expression']) {
            $this->fields[$alias]["hidden"] = FALSE;
          }
          continue;
        }
        else {
          $this->columns[$alias]["hidden"] = TRUE;
          if ($column["is_db_field"] || $column['is_expression']) {
            $this->fields[$alias]["hidden"] = TRUE;
          }
        }
      }
    }
    elseif ($this->hasSelectablesColumns()) {
      $columns = $this->retrieveSelectedColumns();
      if (is_array($columns)) {
        foreach ($this->columns as $alias => $column) {
          if ($column["uncacheable"]) {
            continue;
          }
          $this->columns[$alias]["hidden"] = !in_array($alias, $columns);
          if ($column["is_db_field"] || $column['is_expression']) {
            $this->fields[$alias]["hidden"] = $this->columns[$alias]["hidden"];
          }
        }
      }
    }
    // Récupération des filtres actifs
    if ($this->id == $target_grid && isset($_GET['addFilter']) && in_array($_GET['addFilter'], $this->filters['id']) && isset($_GET['filterValue'])) {
      $this->activeFilters = array();
      $filter_value = $_GET['filterValue'];
      $filter_op = isset($_GET['filterOp']) ? $_GET['filterOp'] : 'EQUALS';
      $filter_mode = isset($_GET['filterMode']) ? $_GET['filterMode'] : NULL;
      $this->addActiveFilter($_GET['addFilter'], $filter_mode, $filter_op, $filter_value);
      if ($this->persistant) {
        $this->storeInSession("filters", $this->activeFilters, FALSE);
      }
    }
    if (isset($_GET["_f"]) && $this->id == $target_grid) {
      $filters = drupal_json_decode($_GET["_f"]);
      if (is_array($filters)) {
        $this->activeFilters = $filters;
        if ($this->persistant) {
          $this->storeInSession("filters", $filters, FALSE);
        }
      }
    }
    elseif ($this->persistant && $this->isStoredInSession("filters")) {
      $this->activeFilters = $this->retrieveSessionData("filters");
    }
    // Récupération des informations sur les blocks
    $this->retrieveSelectedBlocks();
    // Création de la requête
    $query = $this->createQuery();
    // Remplissage des lignes du tableau
    $rows = array();
    $subqueries = array();
    $final_callbacks = array();
    $orig_rows = array();
    if ($this->totalResults > 0) {
      $result = $query->execute();
      $pattern = "/\:(\w+)/";
      $i = $this->getCurrentPage() * $this->pager;
      foreach ($result as $key => $row) {
        $i++;
        if ($this->mode != "item" && !is_null($this->key)) {
          $key = $row->{$this->key};
        }
        foreach ($this->columns as $alias => $column) {
          if ($column["hidden"] && !$column['always_execute']) {
            continue;
          }
          if (!$column['hidden']) {
            $this->headers[$alias]["data"] = !empty($column['header']) ? $column['header'] : $column["label"];
            if (isset($this->fields[$alias]) && $this->fields[$alias]["sortable"]) {
              if (!empty($this->fields[$alias]["table_field"])) {
                $this->headers[$alias]["field"] = $this->fields[$alias]["table_field"];
              }
              elseif (!empty($this->fields[$alias]["expression"])) {
                $this->headers[$alias]["field"] = $this->fields[$alias]["expression"];
              }
            }
          }
          $output = $column["output"];
          if ($this->mode == "item" && !$column["itemable"]) {
            continue;
          }
          elseif ($this->mode == "mass-edition" && $this->modeContext["column"] == $alias) {
            $rows[$key][$alias]["data"] = $row->$alias;
            continue;
          }
          if (!isset($matches[$alias])) {
            preg_match_all($pattern, $output, $matches[$alias], PREG_SET_ORDER);
          }
          if (isset($matches[$alias]) && is_array($matches[$alias])) {
            foreach ($matches[$alias] as $match) {
              $variable = $match[1];
              // Remplacement par une nouvelle requête
              if (!empty($this->replacements[$variable]) && $this->replacements[$variable]['type'] == "query") {
                if (!isset($subqueries[$variable])) {
                  $subqueries[$variable] = $this->replacements[$variable];
                }
                $subqueries[$variable]["columns"][$alias] = $match[0];
                $input = $this->replacements[$variable]['input'];
                $subqueries[$variable]["values"][$key] = $row->$input;
              }
              // Remplacement par une fonction de callback à la fin
              elseif (!empty($this->replacements[$variable]) && $this->replacements[$variable]['type'] == 'final_callback') {
                if (!isset($final_callbacks[$variable])) {
                  $final_callbacks[$variable] = $this->replacements[$variable];
                }
                $final_callbacks[$variable]["columns"][$alias] = $match[0];
              }
              // Remplacement par une fonction de callback immédiatement
              elseif (!empty($this->replacements[$variable]) && $this->replacements[$variable]['type'] == 'callback') {
                $args = $this->replacements[$variable]["args"];
                $args = $this->proceedArgsReplacement($variable, $args, $row, $pattern);
                $rpl_string = call_user_func_array($this->replacements[$variable]["callback"], $args);
                $output = str_replace($match[0], $rpl_string, $output);
                $row->$variable = $output;
              }
              elseif (property_exists($row, $variable)) {
                $output = str_replace($match[0], $row->$variable, $output);
              }
            }
          }
          if (!$column['hidden']) {
            if ($column["main"]) {
              $rows[$key][$alias]["scope"] = "row";
              $rows[$key][$alias]["class"][] = "main-cell";
            }
            $output = str_replace("!rank", $i, $output);
            $rows[$key][$alias]["data"] = $output;
          }
        }
        $orig_rows[$key] = $row;
      }
      // Traitement des sous-requêtes
      foreach ($subqueries as $placeholder => $subquery) {
        $args = $subquery["args"];
        $query = $subquery["query"];
        $keys = array();
        if (isset($subquery["input"])) {
          $args[':input'] = $subquery['values'];
        }
        $sq = db_query($query, $args);
        $result_sq = array();
        while ($sq_row = $sq->fetchAssoc()) {
          // Clé de stockage pour chaque ligne
          if (is_numeric($subquery['key'])) {
            $sq_i = 0;
            foreach ($sq_row as $sq_column_value) {
              if ($sq_i == $subquery['key']) {
                $sq_key = $sq_column_value;
                break;
              }
              $sq_i++;
            }
          }
          elseif (isset($sq_row[$subquery['key']])) {
            $sq_key = $sq_row[$subquery['key']];
          }
          else {
            $sq_key = current($sq_row);
          }
          // Chargement / modification des données par une fonction
          if (!empty($subquery['callback'])) {
            $sq_row = call_user_func_array($subquery['callback'], array($sq_row));
          }
          // Génération de la sortie
          $sq_row_output = $subquery['output'];
          $sq_i = 0;
          foreach ($sq_row as $sq_column => $sq_column_value) {
            if (is_numeric($sq_row_output)) {
              if ($sq_i == $sq_row_output) {
                $sq_row_output = $sq_column_value;
              }
              $sq_i++;
            }
            elseif (substr_count($sq_row_output, ':' . $sq_column) > 0) {
              $sq_row_output = str_replace(':' . $sq_column, $sq_column_value, $sq_row_output);
            }
            if (in_array($sq_column, $subquery['export']) && isset($orig_rows[$key])) {
              $orig_rows[$sq_key]->$sq_column = $sq_column_value;
            }
          }
          // Sauvegarde dans le tableau de données
          $orig_rows[$sq_key]->{$placeholder}[] = $sq_row_output;
          $result_sq[$sq_key][] = $sq_row_output;
        }
        // Remplacement dans la grille finale
        foreach ($orig_rows as $key => $row) {
          if (isset($rows[$key])) {
            foreach ($subquery["columns"] as $column => $output) {
              if (!empty($result_sq[$key])) {
                if ($subquery['output_mode'] == 'ul') {
                  $data = theme('item_list', array('items' => $result_sq[$key], 'type' => 'ul'));
                }
                elseif ($subquery['output_mode'] == 'ol') {
                  $data = theme('item_list', array('items' => $result_sq[$key], 'type' => 'ol'));
                }
                else {
                  $data = implode(", ", $result_sq[$key]);
                }
              }
              else {
                $data = t("(None)");
              }
              if (isset($rows[$key][$column])) {
                $rows[$key][$column]["data"] = str_replace(":" . $placeholder, $data, $rows[$key][$column]["data"]);
              }
            }
          }
        }
      }
      // Traitement des sous-requêtes
      foreach ($final_callbacks as $variable => $replacement) {
        foreach ($orig_rows as $key => $row) {
          $args = $replacement['args'];
          $args['context'] = $row;
          $new_value = call_user_func_array($replacement['callback'], $args);
          foreach ($replacement['columns'] as $column => $output) {
            $rows[$key][$column]["data"] = str_replace(':' . $variable, $new_value, $rows[$key][$column]["data"]);
          }
        }
      }
    }
    $supercaption = '';
    // Création de l'en-tête et des titres
    if ($this->titleMode == 'caption' && !is_null($this->title)) {
      $supercaption = $this->title;
    }
    elseif (!is_null($this->caption)) {
      $supercaption = $this->caption;
    }
    if (trim($supercaption) != '') {
      $supercaption = '<strong> ' . $supercaption . ' :</strong> ';
    }
    if (count($this->appliedFilters) == 0) {
      if (!empty($this->extraCaption["no_results"]) && $this->totalResults == 0) {
        $supercaption .= "<span class='results'>" .
          $this->extraCaption["no_results"] . "</span>";
      }
      elseif (!empty($this->extraCaption["single_result"]) && $this->totalResults == 1) {
        $supercaption .= "<span class='results'>" .
          $this->extraCaption["single_result"] . "</span>";
      }
      elseif (!empty($this->extraCaption["many_results"]) && $this->totalResults > 1) {
        $current_sort = $this->getCurrentSort();
        if ($current_sort != 0) {
          $current_key = array_search(abs($current_sort), $this->sortables["id"]);
        }
        $supercaption .= "<span class='results'>"
        . t($this->extraCaption["many_results"], array(
          "%total_results" => $this->totalResults,
            "!order" => $current_sort == 0 ? t("default") : "<em>" . $this->sortables["label"][$current_key] . "</em>"
            . ($current_sort == 0 ? "": " ". ($current_sort > 0 ? t("ascending") : t("descending"))))
          )
        . "</span>";
      }
    }
    else {
      if (isset($this->extraCaption["single_result_filtered"]) && $this->totalResults == 1) {
        $supercaption .= "<span class='results'>" .
          t($this->extraCaption["single_result_filtered"], array("%unfiltered_total" => $this->unfilteredTotalResults)) . "</span>";
      }
      elseif (isset($this->extraCaption["many_results_filtered"]) && $this->totalResults > 1) {
        $current_sort = $this->getCurrentSort();
        if ($current_sort != 0) {
          $current_key = array_search(abs($current_sort), $this->sortables["id"]);
        }
        $supercaption .= "<span class='results'>" .
          t($this->extraCaption["many_results_filtered"], array(
            "%total_results" => $this->totalResults,
            "%unfiltered_total" => $this->unfilteredTotalResults,
            "!order" => $current_sort == 0 ? t("default")
              : "<em>" . $this->sortables["label"][$current_key] . "</em>"
              . ($current_sort == 0 ? "": " ". ($current_sort > 0 ? t("ascending") : t("descending")))
          )) . "</span>";
      }
      if (count($this->appliedFilters) == 1 && isset($this->extraCaption["single_filter"])) {
        $supercaption .= " <span class='filters'>"
        . t($this->extraCaption["single_filter"],
            array("!filters" => implode(" ", $this->appliedFilters))) . "</span>";
      }
      elseif (isset($this->extraCaption["many_filters"])) {
        $supercaption .= " <span class='filters'>"
        . t($this->extraCaption["many_filters"],
            array("%nb_filters" => count($this->appliedFilters),
                "!filters" => implode(" ", $this->appliedFilters))) . "</span>";
      }
      $options['query'] = $this->transmitUrlParameters(TRUE);
      $supercaption .= " <span class='reset-filters'>"
        . l(t("(Remove all filters)"), $this->getConstructor('page'), $options)
        . "</span>";
    }
    if ($this->pager > 0 && $this->totalResults > $this->pager) {
      $supercaption .= " <span class='pager-active'>" .
        t($this->extraCaption["pager"], array("%start" => 1 + $this->currentPage * $this->pager,
          "%end" => min(($this->currentPage + 1) * $this->pager, $this->totalResults))) . "</span>";
    }
    // Message des filtres
    if ($this->totalResults == 0 && $this->totalResults != $this->unfilteredTotalResults) {
      $this->emptyText = t("No results found matching your criteria. Try changing the filters values.");
    }
    // Déclaration des éléments envoyés à la fonction drupal_render
    if ($this->mode == "item" && !is_null($this->key)) {
      $datagrid = $this->buildModeItem($rows, $orig_rows);
    }
    elseif ($this->mode == 'mass-edition') {
      $datagrid = $this->buildModeMassEdition($rows, $supercaption);
    }
    else {
      if (isset($_GET['_op']) && $_GET['_op'] == 'refresh') {
        $this->op[] = 'partial-refresh';
      }
      $datagrid = $this->buildModeList($rows, $supercaption);
    }
    return $datagrid;
  }

  protected function buildModeItem($rows, $orig_rows) {
    $this->autoRefresh = 0;
    if (!isset($this->modeContext["entity_type"])) {
      $this->modeContext["entity_type"] = "node";
    }
    $title = NULL;
    if (!empty($this->caption) || !empty($this->title)) {
      $title = !empty($this->caption) ? $this->caption : $this->title;
      if (count($this->activeFilters) > 0) {
        $title .= " (" . t("Filtered :") . " " . implode(" ", $this->appliedFilters) . ")";
      }
      if ($this->rankResult > 0) {
        $title .= " - " . t("result !rank of !total",
          array("!rank" => $this->rankResult, "!total" => $this->totalResults));
      }
      else {
        $title .= " - " . t("unmatched result");
      }
    }
    $next = NULL;
    $prev = NULL;
    if ($this->totalResults > 0) {
      reset($orig_rows);
      $ikey = $this->key;
      if ($this->pager > 0 && $this->rankResult > 0) {
        $new_page = floor(($this->rankResult - 1) / $this->pager);
      }
      foreach ($orig_rows as $key => $row) {
        if ($row->$ikey == $this->modeContext["item_id"]) {
          if (isset($rows[$key - 1])) {
            $prev["rows"] = $rows[$key - 1];
            $prev["link"] = $this->modeContext["entity_type"] . "/" . $orig_rows[$key - 1]->$ikey;
            $prev["label"] = $this->extraCaption["prev_label"];
          }
          if (isset($rows[$key + 1])) {
            $next["rows"] = $rows[$key + 1];
            $next["link"] = $this->modeContext["entity_type"] . "/" . $orig_rows[$key + 1]->$ikey;
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
    $path = (isset($this->linkedNode)) ? "node/" . $this->linkedNode->nid : $this->getConstructor('page');
    $datagrid = array(
      "#theme" => "datagrid_item_navigation",
      "#title" => array(
        "datagrid" => $title,
        "node" => isset($this->linkedNode) ? l($this->linkedNode->title, "node/" . $this->linkedNode->nid) : NULL,
        "custom" => isset($this->modeContext["title"]) ? $this->modeContext["title"] : NULL,
        "list" => count($this->activeFilters) == 0 ?
            l($this->extraCaption["backlink"], $path,
              array(
                "fragment" => $this->id,
                "query" => array("_p" => isset($new_page) ? $new_page : 0),
                "attributes" => array("class" => array("back", "icon")),
              ))
            : l($this->extraCaption["filtered_backlink"], "node/" . $path,
              array(
                "fragment" => $this->id,
                "query" => array("_p" => isset($new_page) ? $new_page : 0),
                "attributes" => array("class" => array("back", "icon")),
              )) . " | " .
            l($this->extraCaption["unfiltered_backlink"], "node/" . $path,
              array(
                "fragment" => $this->id,
                "query" => array("_r" => 1),
                "attributes" => array("class" => "back", "icon"),
              )),
      ),
      "#next" => $next,
      "#prev" => $prev,
    );
    return $datagrid;
  }

  protected  function buildModeList($rows, $supercaption) {
    $refresh = $this->hasOp('partial-refresh');
    if (!$refresh) {
      $datagrid = array(
        "#prefix" => "<div id='" . $this->id . "' class='kw-datagrid'>",
        "#suffix" => "</div>",
        "#weight" => $this->weight,
      );
    }
    else {
      $datagrid = NULL;
    }
    if ($this->titleMode != "caption") {
      $datagrid["title"] = array(
        "#markup" => (!empty($this->title) ? ("<" . $this->titleMode . ">" . $this->title . "</"
        . (substr_count($this->titleMode, " ") == 0 ? $this->titleMode : substr($this->titleMode, 0, strpos($this->titleMode, " "))) . ">") : '')
        . (empty($this->caption) ? theme('datagrid_supercaption', array('supercaption' => $supercaption)) : ''),
      );
    }
    $before_blocks = array();
    $after_blocks = array();
    $after_output = '';
    $before_output = '';
    if (!empty($this->relatedBlocks) && !empty($rows) && !$refresh) {
      foreach ($this->relatedBlocks as $block) {
        if ($block['position'] == 'after') {
          $after_blocks[$block["weight"]][] = $block;
        }
        else {
          $before_blocks[$block["weight"]][] = $block;
        }
      }
      if (!empty($before_blocks)) {
        $before_output = $this->buildBlocks('before-block', $before_blocks, $rows, $datagrid);
      }
      if (!empty($after_blocks)) {
        $after_output = $this->buildBlocks('after-block', $after_blocks, $rows, $datagrid);
      }
    }
    if (!is_null($this->intro) && !$refresh) {
      $datagrid["intro"] = array(
        "#markup" => "<div class='datagrid-intro'>" . $this->intro . "</div>",
      );
    }
    if (!empty($before_blocks) && !$refresh) {
      $datagrid = array_merge($datagrid, $before_output);
    }
    if (!empty($this->caption) || $this->titleMode == 'caption') {
      $this->caption = $supercaption;
    }
    $datagrid["grid"] = array(
      "#theme" => "datagrid",
      "#datagrid" => $this,
      "#rows" => $rows,
    );
    if ($this->formDisplay == 'standard' && !$refresh) {
      $form = $this->buildForm();
      if (!is_null($form) && ($this->getUnfilteredTotalResults() > 1
          || ($this->getUnfilteredTotalResults() > 0 && count($this->getConfigurableBlocks()) > 0)
          || ($this->getUnfilteredTotalResults() > 0 && $this->hasSelectablesColumns()))) {
        $datagrid["form"] = array(
          "#markup" => "<div class='datagrid-filters'>" . drupal_render($form) . "</div>",
        );
      }
    }
    if (!empty($after_blocks) && !$refresh) {
      $datagrid = array_merge($datagrid, $after_output);
    }
    if (!is_null($this->outro) && !$refresh) {
      $datagrid["outro"] = array(
        "#markup" => "<div class='datagrid-outro'>" . $this->outro . "</div>",
      );
    }
    return $datagrid;
  }

  protected function buildModeMassEdition($rows, $supercaption) {
    $datagrid = array(
      "#prefix" => "<div id='" . $this->id . "' class='kw-datagrid'>",
      "#suffix" => "</div>",
      "#weight" => $this->weight,
    );
    if ($this->titleMode != "caption") {
      $datagrid["title"] = array(
        "#markup" => (!empty($this->title) ? ("<" . $this->titleMode . ">" . $this->title . "</"
        . (substr_count($this->titleMode, " ") == 0 ? $this->titleMode : substr($this->titleMode, 0, strpos($this->titleMode, " "))) . ">") : '')
        . (empty($this->caption) ? theme('datagrid_supercaption', array('supercaption' => $supercaption)) : ''),
      );
    }
    $datagrid["form"] = array(
      "#markup" => drupal_render(
        drupal_get_form("kw_datagrid_mass_edit_form", array(
          "datagrid" => $this,
          "rows" => $rows,
        ))
      ),
    );
    return $datagrid;
  }

  public function retrieveSelectedBlocks($input = NULL, $serialize = FALSE) {
    if (is_array($input)) {
      $blocks = $input;
    }
    elseif ($this->persistant) {
      if (isset($_GET['_b'])) {
        $blocks = explode(';', $_GET['_b']);
        $this->storeInSession('blocks', $blocks, TRUE, $this->getDefaultBlocks());
      }
      else {
        $blocks = $this->retrieveSessionData('blocks');
      }
      if (is_null($blocks)) {
        $blocks = FALSE;
      }
    }
    elseif (isset($_GET['_b'])) {
      $blocks = explode(';', $_GET['_b']);
    }
    else {
      $blocks = FALSE;
    }
    if ($serialize) {
      return implode(';', $blocks);
    }
    else {
      if ($blocks !== FALSE && !empty($blocks)) {
        sort($blocks);
      }
      return $blocks;
    }
  }

  public function storeSelectedBlocks($blocks) {
    if ($this->persistant) {
      $this->storeInSession('blocks', $blocks, TRUE, $this->defaultBlocks);
    }
  }

  protected function buildBlocks($type, $blocks_array_group_by_weight, &$rows) {
    $output = array();
    $block_policy = $this->retrieveSelectedBlocks();
    ksort($blocks_array_group_by_weight);
    foreach ($blocks_array_group_by_weight as $blocks_array) {
      foreach ($blocks_array as $block) {
        $block['html_id'] = $this->id . '-' . $block['id'];
        if ($block_policy !== FALSE && (empty($block_policy) || !in_array($block['id'], $block_policy))) {
          $block['hidden'] = TRUE;
        }
        $args = array(&$rows, $this->headers, $block);
        if (!empty($block['args'])) {
          $args = array_merge($args, $block['args']);
        }
        $block_content = call_user_func_array($block['function'], $args);
        if ($block_content !== FALSE) {
          $block_output = theme('datagrid_block', array(
            'content' => $block_content,
            'block' => $block,
            'datagrid' => $this,
          ));
          if (!$block['hidden']) {
            $output[$type . '-' . $block['id']] = array("#markup" => $block_output);
          }
          if ($block['hideable']) {
            $this->configurableBlocks[$block['id']] = $block;
          }
        }
      }
    }
    return $output;
  }

  protected function proceedArgsReplacement($name, $args, $row, $pattern) {
    static $matches = array();
    if (!is_array($args)) {
      return array();
    }
    foreach ($args as $key => $arg) {
      if (is_array($arg)) {
        $arg = $this->proceedArgsReplacement($name . "_" . $key, $arg, $row, $pattern);
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
            if (property_exists($row, $match[1])) {
              $arg = str_replace($match[0], $row->$match[1], $arg);
            }
          }
        }
      }
      $args[$key] = $arg;
    }
    return $args;
  }

  public function getCurrentSort() {
    // CAS 0 : on a déjà appelé la fonction...
    if (!is_null($this->currentSort)) {
      return $this->currentSort;
    }
    // CAS 2 : valeur envoyée par adresse
    $order = isset($_GET['_s']) ? $_GET['_s'] : FALSE;
    if ($order !== FALSE && !empty($this->sortables) && in_array(abs($order), $this->sortables["id"])) {
      $this->currentSort = $order;
      if ($this->persistant) {
        $this->storeInSession("sort", $this->currentSort, TRUE, $this->getDefaultOrderIndex());
      }
      return $this->currentSort;
    }
    // CAS 3 : valeur enregistrée en session
    if ($this->persistant) {
      $session_data = $this->retrieveSessionData("sort");
      if ($session_data !== FALSE && !empty($this->sortables) && in_array(abs($session_data), $this->sortables["id"])) {
        $this->currentSort = $session_data;
        return $this->currentSort;
      }
    }
    // CAS 4 : valeur par défaut
    $default_order = $this->getDefaultOrderIndex();
    if (!is_null($default_order)) {
      $this->currentSort = $default_order;
      return $this->currentSort;
    }
    else {
      $this->currentSort = 0;
      return $this->currentSort;
    }
  }

  public function getDefaultOrderIndex() {
    if (is_null($this->defaultOrderIndex)) {
      $default = current($this->getDefaultOrders());
      $index = $this->findSortable($default["column"]);
      if ($default["order"] == "desc") {
        $index = -$index;
      }
      $this->defaultOrderIndex = $index;
    }
    return $this->defaultOrderIndex;
  }

  protected function createQuery() {
    $main_table_alias = key($this->tables);
    $query = db_select($this->table, $main_table_alias);
    $groupier = FALSE;
    $count_method = "classic";
    // Ajout des jointures
    $tables = array($main_table_alias);
    foreach ($this->tables as $alias => $table) {
      if (is_array($table) && $table != $this->table) {
        // Ne pas ajouter de jointures si la colonne correspondante
        // n'est pas utilisée :
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
            foreach ($this->activeFilters as $active_filter) {
              $filter_key = array_search($active_filter["filter_id"], $this->filters["id"]);
              if (count(array_intersect($table["linked_columns"], $this->filters["aliases"][$filter_key])) > 0) {
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
      if (is_array($table) && $table["group_by"]) {
        $groupier = TRUE;
        if ($table["count_method"] != "classic") {
          $count_method = $table["count_method"];
        }
      }
    }
    // Ajout des conditions de base
    foreach ($this->conditions as $condition) {
      $query->condition($condition["field"], $condition["value"], $condition["operand"]);
    }
    // Ajout des champs
    foreach ($this->fields as $alias => $column) {
      if ($column["hidden"] && !$column["always_execute"]) {
        continue;
      }
      if ($column["selectable"]) {
        if (!is_null($column["expression"])) {
          $query->addExpression($column["expression"], $alias);
        }
        else {
          if (is_null($column['table_alias'])) {
            $column['table_alias'] = $main_table_alias;
          }
          if (in_array($column["table_alias"], $tables)) {
            $query->addField($column["table_alias"], $column["table_field"], $alias);
            if ($groupier) {
              $query->groupBy($column["table_alias"] . "." . $column["table_field"]);
            }
          }
        }
      }
    }
    // Ajout des conditions filtrées
    if (count($this->activeFilters) > 0) {
      if ($this->mode != "item") {
        $unfiltered_query = $query;
        // Comptage des résultats non filtrés
        $count_query = $unfiltered_query->countQuery();
        if ($groupier && $count_method != "classic") {
          $count_results = $count_query->execute()->rowCount();
        }
        else {
          $count_results = $count_query->execute()->fetchField();
        }
        $this->unfilteredTotalResults = $count_results;
      }
      $logical_and = db_and();
      $logical_or = db_or();
      $i = 0;
      if (empty($this->filters)) {
        $this->activeFilters = array();
      }
      foreach ($this->activeFilters as $active_filter) {
        if (is_null($active_filter["value"]) || trim($active_filter["value"]) == "") {
          continue;
        }
        $filter_key = array_search($active_filter["filter_id"], $this->filters["id"]);
        if ($filter_key === FALSE) {
          continue;
        }
        $expression = array();
        $logical1 = db_and();
        if ($this->filters["type"][$filter_key] == KW_FILTRABLE_NUM) {
          $active_filter["value"] = str_replace(array(" ", ","), array("", "."), $active_filter["value"]);
          if (!is_numeric($active_filter["value"])) {
            $active_filter["value"] = 0;
          }
        }
        switch ($active_filter["op"]) {
          case("CONTAIN"):
            $op = "LIKE";
            $active_filter["value"] = "%" . $active_filter["value"] . "%";
            $expression[] = $active_filter["value"];
            break;

          case("NOT_CONTAIN"):
            $op = "NOT LIKE";
            $active_filter["value"] = "%" . $active_filter["value"] . "%";
            $expression[] = $active_filter["value"];
            break;

          case("BEGIN"):
            $op = "LIKE";
            $active_filter["value"] .= "%";
            $expression[] = $active_filter["value"];
            break;

          case("NOT_BEGIN"):
            $op = "NOT LIKE";
            $active_filter["value"] .= "%";
            $expression[] = $active_filter["value"];
            break;

          case("END"):
            $op = "LIKE";
            $active_filter["value"] = "%" . $active_filter["value"];
            $expression[] = $active_filter["value"];
            break;

          case("NOT_END"):
            $op = "NOT LIKE";
            $active_filter["value"] = "%" . $active_filter["value"];
            $expression[] = $active_filter["value"];
            break;

          case("CONTAIN_ONE"):
            $op = "LIKE";
            $expression = explode(" ", $active_filter["value"]);
            foreach ($expression as $ke => $ve) {
              $expression[$ke] = "%" . $ve . "%";
            }
            $logical1 = db_or();
            break;

          case("NOT_CONTAIN_ONE"):
            $op = "NOT LIKE";
            $expression = explode(" ", $active_filter["value"]);
            foreach ($expression as $ke => $ve) {
              $expression[$ke] = "%" . $ve . "%";
            }
            break;

          case("CONTAIN_ALL"):
            $op = "LIKE";
            $expression = explode(" ", $active_filter["value"]);
            foreach ($expression as $ke => $ve) {
              $expression[$ke] = "%" . $ve . "%";
            }
            break;

          case("NOT_CONTAIN_ALL"):
            $op = "NOT LIKE";
            $expression = explode(" ", $active_filter["value"]);
            foreach ($expression as $ke => $ve) {
              $expression[$ke] = "%" . $ve . "%";
            }
            $logical1 = db_or();
            break;

          case("NOT_EQUAL"):
            $op = "<>";
            $expression[] = $active_filter["value"];
            break;

          case("SUPERIOR"):
            $op = ">";
            $expression[] = $active_filter["value"];
            break;

          case("SUPERIOR_EQUALS"):
            $op = ">=";
            $expression[] = $active_filter["value"];
            break;

          case("INFERIOR"):
            $op = "<";
            $expression[] = $active_filter["value"];
            break;

          case("INFERIOR_EQUALS"):
            $op = "<=";
            $expression[] = $active_filter["value"];
            break;

          default:
            $op = "=";
            $expression[] = $active_filter["value"];
        }
        if (count($this->filters["aliases"][$filter_key]) == 1) {
          $alias = $this->filters["aliases"][$filter_key][0];
          if (isset($this->fields[$alias])) {
            foreach ($expression as $exp) {
              $logical1->condition($this->fields[$alias]["table_alias"] . "."
              . $this->fields[$alias]["table_field"], $exp, $op);
            }
          }
          elseif (isset($this->filters["options"][$filter_key]["jointure"])) {
            $jointure = $this->filters["options"][$filter_key]["jointure"];
            if ($this->filters["type"][$filter_key] == KW_FILTRABLE_ENUM) {
              $operator = '=';
            }
            else {
              $operator = $op;
            }
            if (!empty($jointure['filtering'])) {
              $filtering = $jointure['filtering'];
            }
            else {
              $filtering = $alias;
            }
            $filter_condition = str_replace($jointure["alias"] . ".", $jointure["alias"] . $i . ".", $jointure["condition"])
            . " AND " . $jointure["alias"] . $i . "." . $filtering . ' ' . $operator . " :filter_key" . $i;
            $filter_args = array(":filter_key" . $i => $expression[0]);
            $query->leftJoin($jointure["table"], $jointure["alias"] . $i,
                $filter_condition, $filter_args);
            if ($this->filters["type"][$filter_key] != KW_FILTRABLE_ENUM || $op == "=") {
              $logical1->isNotNull($jointure["alias"] . $i . "." . $filtering);
            }
            else {
              $logical1->isNull($jointure["alias"] . $i . "." . $filtering);
            }
          }
        }
        else {
          $logical2 = db_or();
          foreach ($this->filters["aliases"][$filter_key] as $alias) {
            if (isset($this->fields[$alias])) {
              $logical2->condition($this->fields[$alias]["table_alias"] . " . " . $this->fields[$alias]["table_field"],
                $active_filter["value"], $op
              );
            }
          }
          $logical1->condition($logical2);
        }
        $string = "";
        if ($i == 0 || $active_filter["mode"] == "OR") {
          $logical_or->condition($logical1);
        }
        else {
          $logical_and->condition($logical1);
        }
        if ($i > 0) {
          $string .= t($active_filter["mode"]) . " ";
        }
        $ops = kw_datagrid_get_operators($this->filters["type"][$filter_key]);
        if ($this->filters["type"][$filter_key] == KW_FILTRABLE_ENUM) {
          $choices = kw_datagrid_get_enum_choices($this->filters, $filter_key);
          if (isset($choices[$active_filter["value"]])) {
            $active_filter["value"] = $choices[$active_filter["value"]];
          }
        }
        elseif (isset($this->filters["options"][$filter_key]["choices"]) && isset($this->filters["options"][$filter_key]["choices"][$active_filter["value"]])) {
          $active_filter["value"] = $this->filters["options"][$filter_key]["choices"][$active_filter["value"]];
        }
        if (!empty($this->filters['options'][$filter_key]['display_callback'])) {
          if (!empty($this->filters['options'][$filter_key]['display_args'])) {
            $args = $this->filters['options'][$filter_key]['display_args'];
            foreach ($args as $arg => $arg_value) {
              if ($arg_value == ':filter') {
                $args[$arg] = $active_filter["value"];
              }
            }
          }
          else {
            $args = array();
          }
          $active_filter['value'] = call_user_func_array($this->filters['options'][$filter_key]['display_callback'],
              $args);
        }
        $string .= t("<em>!field</em> !op %value", array(
          "!field" => drupal_strtolower($this->filters["label"][$filter_key]),
          "!op" => isset($ops[$active_filter["op"]]) ? $ops[$active_filter["op"]] : NULL,
          "%value" => str_replace("%", "", $active_filter["value"]),
        ));
        $this->appliedFilters[] = $string;
        $i++;
      }
      if ($i > 0) {
        $logical_and->condition($logical_or);
        $query->condition($logical_and);
      }
    }
    // Comptage des résultats filtrés
    if ($this->mode != "item") {
      $count_query = $query->countQuery();
      if ($groupier && $count_method != "classic") {
        $count_results = $count_query->execute()->rowCount();
      }
      else {
        $count_results = $count_query->execute()->fetchField();
      }
      $this->totalResults = $count_results;
      if ($this->unfilteredTotalResults == 0) {
        $this->unfilteredTotalResults = $this->totalResults;
      }
      if ($count_results == 0) {
        return FALSE;
      }
    }
    // Tri défini par l'utilisateur
    $orders = array();
    $ranking_orders = array();
    $rank = 0;
    if ($this->getCurrentSort() != 0) {
      $user_asc = $this->getCurrentSort() > 0 ? "ASC" : "DESC";
      $id = array_search(abs($this->getCurrentSort()), $this->sortables["id"]);
      foreach ($this->sortables["aliases"][$id] as $alias) {
        if (isset($this->fields[$alias])) {
          $orders[] = $alias;
          if (!is_null($this->fields[$alias]["sort_expression"])) {
            $q_order = $this->fields[$alias]["sort_expression"];
            $query->orderBy($q_order, $user_asc);
          }
          elseif (!is_null($this->fields[$alias]["table_field"])) {
            $q_order = $this->fields[$alias]["table_alias"] . "." . $this->fields[$alias]["table_field"];
            $ranking_orders[$q_order] = $user_asc;
            $query->orderBy($q_order, $user_asc);
          }
          elseif (!is_null($this->fields[$alias]["expression"])) {
            $q_order = $this->fields[$alias]["expression"];
            $ranking_orders[$q_order] = $user_asc;
            $query->orderBy($q_order, $user_asc);
          }
        }
      }
    }
    // Tri par défaut
    foreach ($this->defaultOrders as $order) {
      if (isset($this->fields[$order["column"]]) && !in_array($order["column"], $orders)) {
        if (!is_null($this->fields[$order["column"]]["table_field"])) {
          $q_order = $this->fields[$order["column"]]["table_alias"] . "." . $this->fields[$order["column"]]["table_field"];
          $query->orderBy($q_order, $order["order"]);
          $ranking_orders[$q_order] = $order["order"];
        }
        elseif (!is_null($this->fields[$order["column"]]["expression"])) {
          $q_order = $this->fields[$order["column"]]["expression"];
          $query->orderBy($q_order, $order["order"]);
          $ranking_orders[$q_order] = $order["order"];
        }
      }
    }
    if ($this->mode == "item" && !is_null($this->key)) {
      $ranking_query = clone $query;
      $sql_key = $this->fields[$this->key]["table_alias"] . "." . $this->fields[$this->key]["table_field"];
      $sql_order = " ORDER BY ";
      $ir = 0;
      foreach ($ranking_orders as $order => $sort) {
        if ($ir > 0) {
          $sql_order .= ",";
        }
        $sql_order .= $order . " " . $sort;
        $ir++;
      }
      $ranking_query->addExpression("GROUP_CONCAT(DISTINCT " . $sql_key . $sql_order . ")", "concat");
      $ranking_query->addExpression("COUNT(DISTINCT " . $sql_key . ")", "total");
      $ranking_query_altered = $ranking_query->__toString();
      $ranking_query_altered = "SELECT " . substr($ranking_query_altered, strpos($ranking_query_altered, "GROUP_CONCAT("));
      $ranking_query_altered = substr($ranking_query_altered, 0, strrpos($ranking_query_altered, "ORDER BY"));
      if ($groupier) {
        $ranking_query_altered = substr($ranking_query_altered, 0, strrpos($ranking_query_altered, "GROUP BY"));
      }
      db_query("SET @@group_concat_max_len := @@max_allowed_packet");
      $result_ranking_query = db_query("SELECT FIND_IN_SET(" . $sql_key . ",concat) as rank, total FROM {"
      . $this->tables[$this->fields[$this->key]["table_alias"]] . "} " . $this->fields[$this->key]["table_alias"] . " "
      . "CROSS JOIN (" . $ranking_query_altered . ") concat WHERE " . $sql_key . "=:nid",
        array_merge($ranking_query->getArguments(), array(":nid" => $this->modeContext["item_id"])));
      $results = $result_ranking_query->fetch();
      $this->rankResult = $rank = $results->rank;
      $this->totalResults = $results->total;
      $this->unfilteredTotalResults = $rank > 0 ? 1 : 0;
    }
    // Prise en compte du pager
    $this->currentPage = $this->getCurrentPage();
    if ($this->mode == "item") {
      $current_start = max(0, $rank - 2);
    }
    else {
      $current_start = $this->currentPage * $this->pager;
    }
    if ($this->pager > 0) {
      $query->range($current_start, ($this->mode == "item" ? 3 : $this->pager));
    }
    return $query;
  }

  protected function storeInSession($var, $value, $save_in_pool = TRUE, $default = NULL) {
    if ($save_in_pool && !is_null($this->pool)) {
      $index = $this->pool;
    }
    else {
      $index = $this->id;
    }
    if ($value != $default) {
      $_SESSION["kw_datagrid"][$index][$var] = $value;
    }
    elseif (isset($_SESSION["kw_datagrid"][$index][$var]) && (empty($value) || $default == $value)) {
      unset($_SESSION["kw_datagrid"][$index][$var]);
    }
  }

  public function retrieveSessionData($data) {
    if (isset($_SESSION["kw_datagrid"][$this->getPool()][$data])) {
      return $_SESSION["kw_datagrid"][$this->getPool()][$data];
    }
    elseif (isset($_SESSION["kw_datagrid"][$this->getId()][$data])) {
      return $_SESSION["kw_datagrid"][$this->getId()][$data];
    }
    else {
      return NULL;
    }
  }

  protected function isStoredInSession($var) {
    if (isset($_SESSION["kw_datagrid"][$this->pool][$var])) {
      return TRUE;
    }
    elseif (isset($_SESSION["kw_datagrid"][$this->id][$var])) {
      return TRUE;
    }
    return FALSE;
  }

  public function listSelectableColumns() {
    $selectables = array();
    foreach ($this->columns as $alias => $column) {
      if (!$column["uncacheable"]) {
        $selectables[$alias] = $column["label"];
      }
    }
    return $selectables;
  }

  public function listSelectedColumns($with_uncacheable = TRUE) {
    $selected = array();
    foreach ($this->columns as $alias => $column) {
      if (!$column["hidden"] && ($with_uncacheable || !$column['uncacheable'])) {
        $selected[$alias] = $column["label"];
      }
    }
    return $selected;
  }

  public function storeSelectedColumns($columns) {
    if ($this->persistant) {
      $this->storeInSession("columns", $columns, TRUE, $this->defaultColumns);
    }
  }

  public function retrieveSelectedColumns($input = NULL, $serialize = FALSE) {
    $columns = FALSE;
    if (is_array($input)) {
      $columns = $input;
    }
    elseif (isset($_GET["_c"])) {
      $columns = explode(';', $_GET["_c"]);
      if ($this->persistant) {
        $this->storeInSession('columns', $columns, !empty($this->pool) ? TRUE : FALSE, $this->getDefaultColumns());
      }
    }
    elseif ($this->persistant) {
      $columns = $this->retrieveSessionData("columns");
    }
    if (!$serialize) {
      return $columns;
    }
    else {
      $output = "";
      foreach ($columns as $column) {
        $output .= (empty($output) ? '' : ';') . $column;
      }
      return $output;
    }
  }

  public function retrieveModifiableColumnInfo() {
    if ($this->mode == "mass-edition") {
      $options = isset($this->columns[$this->modeContext["column"]]["modifiable_options"]) ? $this->columns[$this->modeContext["column"]]["modifiable_options"] : array();
      return array(
        "column" => $this->modeContext["column"],
        "entity" => isset($options["entity"]) ? $options["entity"] : "node",
        "field" => isset($options["field"]) ? $options["field"] : $this->modeContext["column"],
        "bundle" => isset($options["bundle"]) ? $options["bundle"] : (!is_null($this->linkedNode) ? $this->linkedNode->type : "page"),
      );
    }
    return array();
  }

  public function reset($mode = 'filters') {
    if (isset($_SESSION["kw_datagrid"][$this->id])) {
      if ($mode == 'filters' && isset($_SESSION["kw_datagrid"][$this->id]['filters'])) {
        unset($_SESSION["kw_datagrid"][$this->id]['filters']);
      }
      else {
        unset($_SESSION["kw_datagrid"][$this->id]);
      }
    }
    if ($mode != 'filters' && !is_null($this->pool) && isset($_SESSION["kw_datagrid"][$this->pool])) {
      unset($_SESSION["kw_datagrid"][$this->pool]);
    }
    if ($mode == 'filters' && isset($_GET['_f'])) {
      unset($_GET['_f']);
    }
    else {
      foreach ($_GET as $key => $value) {
        if (substr($key, 0, 5) == '_' && $mode != 'filters') {
          unset($_GET[$key]);
        }
      }
    }
  }

  public function addRelatedBlock($id, $title, $function, $options = array()) {
    $args = isset($options['args']) && is_array($options['args']) ? $options['args'] : array();
    $position = isset($options['position']) && $options['position'] == 'after' ? 'after' : 'before';
    $weight = isset($options['weight']) && is_numeric($options['weight']) ? $options['weight'] : 0;
    $show_title = (!isset($options['show_title']) || $options['show_title']) ? TRUE : FALSE;
    $hideable = (!isset($options['hideable']) || $options['hideable']) ? TRUE : FALSE;
    $hidden = (!isset($options['hidden']) || !$options['hidden']) ? FALSE : TRUE;
    $title_tag = !empty($options['title_tag']) ? $options['title_tag'] : 'div';
    $close_label = !empty($options['close_label']) ? $options['close_label'] : t('Hide !title', array('!title' => $title));
    $open_label = !empty($options['open_label']) ? $options['open_label'] : t('Show !title', array('!title' => $title));
    $this->relatedBlocks[$id] = array(
      'id' => $id,
      'function' => $function,
      'args' => $args,
      'position' => $position,
      'weight' => $weight,
      'hideable' => $hideable,
      'hidden' => $hidden,
      'show_title' => $show_title,
      'title' => $title,
      'title_tag' => $title_tag,
      'close_label' => $close_label,
      'open_label' => $open_label,
    );
    if (!$hidden && $hideable) {
      $this->defaultBlocks[] = $id;
    }
    return $this;
  }

  public function getDefaultBlocks() {
    return $this->defaultBlocks;
  }

  public function getDefaultColumns() {
    return $this->defaultColumns;
  }

  public function getConfigurableBlocks() {
    return $this->configurableBlocks;
  }

  public function transmitUrlParameters($reset = FALSE) {
    $exclude[] = 'q';
    $parameters = drupal_get_query_parameters(NULL, $exclude);
    $parameters['_id'] = $this->getId();
    if ($reset) {
      $parameters['_r'] = 1;
    }
    else {
      if (isset($parameters['_r'])) {
        unset($parameters['_r']);
      }
      // Page active :
      if ($this->getCurrentPage() > 0) {
        $parameters['_p'] = $this->getCurrentPage();
      }
      // Résultats affichés :
      if ($this->getPager() != $this->getPagerOptions('default_value')) {
        $parameters['_g'] = $this->getPager();
      }
      // Tri :
      if ($this->getCurrentSort() != $this->getDefaultOrderIndex()) {
        $parameters['_s'] = $this->getCurrentSort();
      }
      // Colonnes :
      if ($this->hasSelectablesColumns()) {
        $selected_columns = array_keys($this->listSelectedColumns(FALSE));
        if ($selected_columns != $this->defaultColumns) {
          $parameters['_c'] = $this->retrieveSelectedColumns($selected_columns, TRUE);
        }
      }
      // Blocs :
      if (count($this->getConfigurableBlocks()) > 0) {
        $selected_blocks = $this->retrieveSelectedBlocks();
        if ($selected_blocks !== FALSE && $selected_blocks != $this->getDefaultBlocks()) {
          $parameters['_b'] = implode(';', $selected_blocks);
        }
      }
      // Filtres :
      $active_filters = $this->getActiveFilters();
      if (!empty($active_filters)) {
        $parameters['_f'] = drupal_json_encode($active_filters);
      }
    }
    return $parameters;
  }

}
