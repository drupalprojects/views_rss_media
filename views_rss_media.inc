<?php

/**
 * @file
 * Preprocess functions for Views RSS: Media Elements module.
 */

/**
 * Preprocess function for item <media:content> element.
 */
function views_rss_media_preprocess_item_content(&$variables) {
  $value = array();
  if (!empty($variables['raw'])) {
    foreach ($variables['raw'] as $delta => $raw_element) {
      if (!empty($raw_element['rendered']['#markup'])) {
        // If there is more than one item in the field,
        // let's render it as <media:group> feed element.
        if (count($variables['raw']) > 1 && !empty($raw_element['rendered']['#settings']['group_multiple_values'])) {
          // First item will be the default one.
          if ($delta == 0) {
            $raw_element['rendered']['#rss_element']['attributes']['isDefault'] = 'true';
            $raw_element['rendered']['#markup'] = format_xml_elements(array($raw_element['rendered']['#rss_element']));
          }
          $value['subelements'][$delta] = array(
            'element' => $variables['element'],
            'markup' => $raw_element['rendered']['#markup'],
          );
        }
        // Only one item in the field, no <media:group> then.
        else {
          $value['elements'][$delta]['markup'] = $raw_element['rendered']['#markup'];
        }
      }
    }
    if (count($variables['raw']) > 1) {
      $variables['element'] = 'media:group';
    }
    if (!empty($value)) {
      $variables['value'] = $value;
    }
  }
}

/**
 * Preprocess function for item <media:title> and <media:description> elements.
 */
function views_rss_media_preprocess_item_text(&$variables) {
  // No value = no preprocessing.
  if (empty($variables['value'])) {
    return;
  }
  // Check whether original value format is text or HTML.
  $type = 'plain';
  $value_decoded = htmlspecialchars_decode($variables['value'], ENT_QUOTES);
  if ($value_decoded != strip_tags($value_decoded)) {
    $type = 'html';
  }
  // Add "type" argument to an element with its format.
  $variables['value'] = array(
    'value' => $variables['value'],
    'arguments' => array(
      'type' => $type,
    ),
  );
}
