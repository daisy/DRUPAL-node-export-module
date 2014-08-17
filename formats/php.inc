<?php

/**
 * @file
 * The Node export PHP format handler.
 *
 * Adds PHP array export format to Node export.
 */

/**
 * Export callback.
 */
function node_export_php_export($nodes, $format) {
  require_once DRUPAL_ROOT . '/includes/utility.inc';
  return drupal_php_export($nodes);
}

/**
 * Import callback.
 */
function node_export_php_import($code_string) {
  if (substr(ltrim($code_string), 0, 6) == "array(") {
    $nodes = eval('return ' . $code_string . ';');
    if (is_array($nodes)) {
      return node_export_drupal_decode_objects($nodes);
    }
  }
}

/**
 * Recursively convert arrays back to objects.
 *
 * This is only for backwards compatibility with the deprecated node_code format.
 */
function node_export_php_decode_objects($array) {
  foreach ($array as $k => $v) {
    if (is_array($v)) {
      $array[$k] = node_export_drupal_decode_objects($v);
    }
  }
  if (isset($array['#_export_node_encode_object'])) {
    unset($array['#_export_node_encode_object']);
    $array = (object)$array;
  }
  return $array;
}

/**
 * Callback for actions.
 */
function node_export_php_action_form($context, &$form_state) {
  return node_export_action_form($context, $form_state, 'php');
}


function drupal_php_export($var, $prefix = '') {
  if (is_array($var)) {
    if (empty($var)) {
      $output = '<?php $array = array();';
    }
    else {
      $output = '<?php $array = array('."\n";
      // Don't export keys if the array is non associative.
      $export_keys = array_values($var) != $var;
      foreach ($var as $key => $value) {
        $output .= '  ' . ($export_keys ? drupal_var_export($key) . ' => ' : '') . drupal_var_export($value, '  ', FALSE) . ",\n";
      }
      $output .= ');';
    }
  }
  elseif (is_bool($var)) {
    $output = $var ? 'TRUE' : 'FALSE';
  }
  elseif (is_string($var)) {
    $line_safe_var = str_replace("\n", '\n', $var);
    if (strpos($var, "\n") !== FALSE || strpos($var, "'") !== FALSE) {
      // If the string contains a line break or a single quote, use the
      // double quote export mode. Encode backslash and double quotes and
      // transform some common control characters.
      $var = str_replace(array('\\', '"', "\n", "\r", "\t"), array('\\\\', '\"', '\n', '\r', '\t'), $var);
      $output = '"' . $var . '"';
    }
    else {
      $output = "'" . $var . "'";
    }
  }
  elseif (is_object($var) && get_class($var) === 'stdClass') {
    // var_export() will export stdClass objects using an undefined
    // magic method __set_state() leaving the export broken. This
    // workaround avoids this by casting the object as an array for
    // export and casting it back to an object when evaluated.
    $output = '(object) ' . drupal_var_export((array) $var, $prefix);
  }
  else {
    $output = var_export($var, TRUE);
  }

  if ($prefix) {
    $output = str_replace("\n", "\n$prefix", $output);
  }

  return $output;
}