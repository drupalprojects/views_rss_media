<?php

/**
 * Implements hook_field_formatter_info().
 */
function views_rss_media_field_formatter_info() {
  $formatters = array(
    'media_content' => array(
      'label' => t('RSS <media:content> element'),
      'field types' => array('image', 'file'),
      'settings' => array(
        'group_multiple_values' => 0,
        'medium' => '',
        'expression' => '',
        'image_style' => '',
      ),
    ),
    'media_thumbnail' => array(
      'label' => t('RSS <media:thumbnail> element'),
      'field types' => array('image'),
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

  if ($field['type'] == 'image') {
    $image_styles = image_style_options(FALSE);
    $element['image_style'] = array(
      '#title' => t('Image style'),
      '#type' => 'select',
      '#default_value' => $settings['image_style'],
      '#empty_option' => t('None (original image)'),
      '#options' => $image_styles,
    );
  }
  
  if ($display['type'] == 'media_content') {

    $element['group_multiple_values'] = array(
      '#title' => t('Group multiple field values in one &lt;media:group&gt; element'),
      '#type' => 'checkbox',
      '#default_value' => $settings['group_multiple_values'],
      '#description' => t('&lt;media:group&gt; element allows grouping of &lt;media:content&gt; elements that are effectively the same content, yet different representations. For instance: the same song recorded in both the WAV and MP3 format. It is an optional element that must only be used for this purpose. !more_link', array(
        '!more_link' => l('[?]', 'http://www.rssboard.org/media-rss#media-group', array('attributes' => array('title' => t('Need more information?')))),
      )),
    );

    $element['medium'] = array(
      '#title' => t('Medium'),
      '#type' => 'select',
      '#default_value' => $settings['medium'],
      '#empty_option' => t('None (do not display)'),
      '#options' => drupal_map_assoc(array('image', 'audio', 'video', 'document', 'executable')),
      '#description' => t('<em>medium</em> attribute of &lt;media:content&gt; element is the type of object. While this attribute can at times seem redundant if type is supplied, it is included because it simplifies decision making on the reader side, as well as flushes out any ambiguities between MIME type and object type. It is an optional attribute. !more_link', array(
        '!more_link' => l('[?]', 'http://www.rssboard.org/media-rss#media-content', array('attributes' => array('title' => t('Need more information?')))),
      )),
    );

    $element['expression'] = array(
      '#title' => t('Expression'),
      '#type' => 'select',
      '#default_value' => $settings['expression'],
      '#empty_option' => t('None (do not display)'),
      '#options' => drupal_map_assoc(array('sample', 'full', 'nonstop')),
      '#description' => t('<em>expression</em> attribute of &lt;media:content&gt; element determines if the object is a sample or the full version of the object, or even if it is a continuous stream. Default value is "full". It is an optional attribute. !more_link', array(
        '!more_link' => l('[?]', 'http://www.rssboard.org/media-rss#media-content', array('attributes' => array('title' => t('Need more information?')))),
      )),
    );
    
  }

  return $element;
}

/**
 * Implements hook_field_formatter_settings_summary().
 */
function views_rss_media_field_formatter_settings_summary($field, $instance, $view_mode) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];

  $summary = array();

  if ($field['type'] == 'image') {
    $image_styles = image_style_options(FALSE);
    // Unset possible 'No defined styles' option.
    unset($image_styles['']);
    // Styles could be lost because of enabled/disabled modules that defines
    // their styles in code.
    if (isset($image_styles[$settings['image_style']])) {
      $summary[] = t('Image style: @style', array('@style' => $image_styles[$settings['image_style']]));
    }
    else {
      $summary[] = t('Original image');
    }
  }
  
  if ($display['type'] == 'media_content') {
  
    if (!empty($display['settings']['group_multiple_values'])) {
      $summary[] = t('Group multiple values');
    }

    $value = (!empty($display['settings']['medium'])) ? $display['settings']['medium'] : t('none');
    $summary[] = t('Medium: !medium', array('!medium' => $value));

    $value = (!empty($display['settings']['expression'])) ? $display['settings']['expression'] : t('none');
    $summary[] = t('Expression: !expression', array('!expression' => $value));
  
  }

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

    // Default file URL.
    $item_uri = $item['uri'];
    $url = file_create_url($item_uri);

    // For images provide styled image URL if required.
    if (!empty($display['settings']['image_style']) && $image_style = image_style_load($display['settings']['image_style'])) {
      // Get full image URI based on provided image style.
      $url = image_style_url($display['settings']['image_style'], $item_uri);
      // Make sure that image style file has already been created.
      $path = image_style_path($display['settings']['image_style'], $item_uri);
      if (!file_exists(drupal_realpath($path))) {
        image_style_create_derivative($image_style, $item_uri, $path);
      }
      $item_uri = $path;
    }
    
    if ($display['type'] == 'media_content') {

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
      if ($langcode != LANGUAGE_NONE) {
        $rss_element['attributes']['lang'] = $langcode;
      }

      // Add "title" property of image fields
      // to <media:title> subelement of <media:content> element.
      if (!empty($item['title'])) {
        $rss_element['value'][] = array(
          'key' => 'media:title',
          'value' => $item['title'],
        );
      }

      // Add "description" property of file fields
      // to <media:description> subelement of <media:content> element.
      if (!empty($item['description'])) {
        $rss_element['value'][] = array(
          'key' => 'media:description',
          'value' => $item['description'],
        );
      }

      // Additional properties for images.
      if ($item['type'] == 'image') {
        $media_info = image_load($item_uri);
        if (!empty($media_info)) {
          $item['info'] = $media_info;
          $rss_element['attributes']['width'] = $media_info->info['width'];
          $rss_element['attributes']['height'] = $media_info->info['height'];
        }
      }

      // Additional properties for audio files.
      if ($item['type'] == 'audio' && module_exists('getid3')) {
        $media_info = getid3_analyze(drupal_realpath($item_uri));
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
        $media_info = getid3_analyze(drupal_realpath($item_uri));
        if (!empty($media_info)) {
          $item['info'] = $media_info;
          $rss_element['attributes']['bitrate'] = round($media_info['video']['bitrate'] / 1000);
          $rss_element['attributes']['framerate'] = $media_info['video']['frame_rate'];
          $rss_element['attributes']['duration'] = round($media_info['playtime_seconds']);
          $rss_element['attributes']['width'] = $media_info['video']['resolution_x'];
          $rss_element['attributes']['height'] = $media_info['video']['resolution_y'];
        }
      }
      
    }
    
    if ($display['type'] == 'media_thumbnail') {
      
      $rss_element = array(
        'key' => 'media:thumbnail',
        'attributes' => array(
          'url' => $url,
        ),
      );

      // Additional properties for images.
      if ($item['type'] == 'image') {
        $media_info = image_load($item_uri);
        if (!empty($media_info)) {
          $item['info'] = $media_info;
          $rss_element['attributes']['width'] = $media_info->info['width'];
          $rss_element['attributes']['height'] = $media_info->info['height'];
        }
      }

    }

    $element[$delta] = array(
      '#item' => $item,
      '#markup' => format_xml_elements(array($rss_element)),
      '#rss_element' => $rss_element,
      '#settings' => $display['settings'],
    );

  }

  return $element;
}