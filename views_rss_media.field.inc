<?php

/**
 * Implements hook_field_formatter_info().
 */
function views_rss_media_field_formatter_info() {
  $formatters = array(
    'media_content' => array(
      'label' => t('RSS <media:content> element'),
      'field types' => array('image', 'file'),
      'settings' => array('medium' => '', 'expression' => ''),
    ),
  );
  return $formatters;
}

/**
 * Implements hook_field_formatter_settings_form().
 */
function views_rss_media_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];

  $element['medium'] = array(
    '#title' => t('Medium'),
    '#type' => 'select',
    '#default_value' => $settings['medium'],
    '#empty_option' => t('None (do not display)'),
    '#options' => drupal_map_assoc(array('image', 'audio', 'video', 'document', 'executable')),
    '#description' => t('<em>medium</em> is the type of object. While this attribute can at times seem redundant if type is supplied, it is included because it simplifies decision making on the reader side, as well as flushes out any ambiguities between MIME type and object type. It is an optional attribute.'),
  );

  $element['expression'] = array(
    '#title' => t('Expression'),
    '#type' => 'select',
    '#default_value' => $settings['expression'],
    '#empty_option' => t('None (do not display)'),
    '#options' => drupal_map_assoc(array('sample', 'full', 'nonstop')),
    '#description' => t('<em>expression</em> determines if the object is a sample or the full version of the object, or even if it is a continuous stream. Default value is "full". It is an optional attribute.'),
  );

  return $element;
}

/**
 * Implements hook_field_formatter_settings_summary().
 */
function views_rss_media_field_formatter_settings_summary($field, $instance, $view_mode) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];

  $summary = array();

  $value = (!empty($display['settings']['medium'])) ? $display['settings']['medium'] : t('none');
  $summary[] = t('Medium: !medium', array('!medium' => $value));

  $value = (!empty($display['settings']['expression'])) ? $display['settings']['expression'] : t('none');
  $summary[] = t('Expression: !expression', array('!expression' => $value));

  return implode('<br />', $summary);
}

/**
 * Implements hook_field_formatter_view().
 */
function views_rss_media_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();
  
  foreach ($items as $delta => $item) {

    // Inside a view item may contain NULL data. In that case, just return.
    if (empty($item['fid'])) {
      unset($items[$delta]);
      continue;
    }

    $url = file_create_url($item['uri']);
    $rss_element = array(
      'key' => 'media:content',
      'attributes' => array(
        'url' => $url,
        'fileSize' => $item['filesize'],
        'type' => $item['filemime'],
      ),
    );
    if (!empty($display['settings']['medium'])) {
      $rss_element['attributes']['medium'] = $display['settings']['medium'];
    }
    if (!empty($display['settings']['expression'])) {
      $rss_element['attributes']['expression'] = $display['settings']['expression'];
    }

    // Additional properties for images.
    if ($item['type'] == 'image') {
      $media_info = image_load($item['uri']);
      if (!empty($media_info)) {
        $item['info'] = $media_info;
        $rss_element['attributes']['width'] = $media_info->info['width'];
        $rss_element['attributes']['height'] = $media_info->info['height'];
      }
    }

    // Additional properties for audio files.
    if ($item['type'] == 'audio' && module_exists('getid3')) {
      $media_info = getid3_analyze(drupal_realpath($item['uri']));
      if (!empty($media_info)) {
        $item['info'] = $media_info;
        $rss_element['attributes']['bitrate'] = round($media_info['audio']['bitrate'] / 1000);
        $rss_element['attributes']['samplingrate'] = $media_info['audio']['sample_rate'];
        $rss_element['attributes']['channels'] = $media_info['audio']['channels'];
        $rss_element['attributes']['duration'] = round($media_info['playtime_seconds']);
      }
    }

    // Additional properties for video files.
    if ($item['type'] == 'video' && module_exists('getid3')) {
      $media_info = getid3_analyze(drupal_realpath($item['uri']));
      if (!empty($media_info)) {
        $item['info'] = $media_info;
        $rss_element['attributes']['bitrate'] = round($media_info['video']['bitrate'] / 1000);
        $rss_element['attributes']['framerate'] = $media_info['video']['frame_rate'];
        $rss_element['attributes']['duration'] = round($media_info['playtime_seconds']);
        $rss_element['attributes']['width'] = $media_info['video']['resolution_x'];
        $rss_element['attributes']['height'] = $media_info['video']['resolution_y'];
      }
    }

    $element[$delta] = array(
      '#item' => $item,
      '#markup' => format_xml_elements(array($rss_element)),
      '#rss_element' => $rss_element,
    );

  }

  return $element;
}