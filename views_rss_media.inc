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
        $value['elements'][$delta]['markup'] = $raw_element['rendered']['#markup'];
      }
    }
    if (!empty($value)) {
      $variables['value'] = $value;
    }
  }
}
