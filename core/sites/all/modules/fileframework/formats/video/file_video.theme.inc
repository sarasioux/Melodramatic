<?php
// $Id$

/**
 * @file
 * Themes for video file formats.
 */

//////////////////////////////////////////////////////////////////////////////
// Theme callbacks

/**
 * Theme for admin getID checkboxes.
 */
function theme_file_video_admin_settings($form) {
  $rows = array();
  foreach ($form['getid3'] as $name => $element) {
    if (preg_match('/getid3_/', $name)) {
      $rows[] = array(
        drupal_render($form['getid3'][$name][0]),
        drupal_render($form['getid3'][$name][1]),
        drupal_render($form['getid3'][$name][2])
      );
      unset($form['getid3'][$name]);
    }
  }
  $form['getid3']['getid3'] = array('#type' => 'markup', '#value' => theme('table', NULL, $rows));
  return drupal_render($form);
}

/**
 * Theme for video rendering.
 */
function theme_file_video_render_player($options, $output, $url, $link_text, $title = '') {
  extract($options, EXTR_SKIP);

  $output = "\n".'<div'. drupal_attributes(array('class' => $class)) .'>'. $output;
  if (!isset($container) || !empty($container)) {
    $output .= '<p><small>'. t('Problems viewing the video?') .' ';
    $output .= str_replace('&gt;&gt;', '&raquo;', l($link_text .' >>', $url, array('title' => $title), NULL, NULL, TRUE));
    $output .= '</small></p>';
  }
  $output .= '</div>'."\n";
  return $output;
}

/**
 * Theme for QuickTime video rendering.
 */
function theme_file_video_quicktime_render($options = array()) {
  extract($options, EXTR_SKIP);

  $height += 16; // Accommodate video controls

  $output = '<!--[if !IE]> <--><object type="video/quicktime" width="'. $width .'" height="'. $height .'" data="'. $src .'"><!--> <![endif]-->'."\n";
  $output .= '<!--[if IE]><object classid="clsid:02BF25D5-8C17-4B23-BC80-D3488ABDDC6B" codebase="http://www.apple.com/qtactivex/qtplugin.cab" width="'. $width .'" height="'. $height .'" scale="tofit"><![endif]-->'."\n";
  $output .= '<param name="src" value="'. $src .'" /><param name="autoplay" value="false" /><param name="kioskmode" value="false" />';
  $output .= "\n".'<p>'. t('Your browser is unable to display this multimedia content.') .'</p>';
  $output .= '</object>';

  return theme('file_video_render_player', $options, $output, FILE_VIDEO_QUICKTIME_LINK, t('Download the latest Quicktime Player'));
}

/**
 * Theme for Windows Media video rendering.
 */
function theme_file_video_windowsmedia_render($options = array()) {
  extract($options, EXTR_SKIP);

  $height += preg_match('/Macintosh/', $_SERVER['HTTP_USER_AGENT']) ? 16 : 68; // Accommodate video controls (Flip4Mac vs MSWiMP)

  $output = '<!--[if !IE]> <--><object type="application/x-mplayer2" width="'. $width .'" height="'. $height .'" data="'. $src .'"><!--> <![endif]-->'."\n";
  $output .= '<!--[if IE]><object type="application/x-oleobject" width="'. $height .'" height="'. $width .'" classid="clsid:6BF52A52-394A-11d3-B153-00C04F79FAA6"><![endif]-->'."\n";
  $output .= '<param name="src" value="'. $src .'" /><param name="URL" value="'. $src .'" /><param name="animationatStart" value="false" /><param name="transparentatStart" value="true" /><param name="autoStart" value="false" /><param name="showControls" value="true" /><param name="loop" value="true" />';
  $output .= "\n".'<p>'. t('Your browser is unable to display this multimedia content.') .'</p>';
  $output .= '</object>';

  return theme('file_video_render_player', $options, $output, FILE_VIDEO_WINDOWSMEDIA_LINK, t('Download the latest Windows Media Player'));
}

/**
 * Theme for RealMedia video rendering.
 */
function theme_file_video_realmedia_render($options = array()) {
  extract($options, EXTR_SKIP);

  $height += 40; // Accommodate video controls

  $output = '<!--[if !IE]> <--><object type="audio/x-pn-realaudio-plugin" width="'. $width .'" height="'. $height .'" data="'. $src .'"><!--> <![endif]-->'."\n";
  $output .= '<!--[if IE]><object type="audio/x-pn-realaudio-plugin" width="'. $width .'" height="'. $height .'" classid="clsid:CFCDAA03-8BE4-11cf-B84B-0020AFBBCCFA"><![endif]-->'."\n";
  $output .= '<param name="src" value="'. $src .'" /><param name="_ExtentX" value="7276" /><param name="" value="3307" /><param name="AUTOSTART" value="false" /><param name="SHUFFLE" value="0" /><param name="PREFETCH" value="0" /><param name="NOLABELS" value="0" /><param name="CONTROLS" value="All" /><param name="CONSOLE" value="Clip1" /><param name="LOOP" value="0" /><param name="NUMLOOP" value="0" /><param name="CENTER" value="0" /><param name="MAINTAINASPECT" value="1" /><param name="BACKGROUNDCOLOR" value="#000000" />';
  $output .= "\n".'<p>'. t('Your browser is unable to display this multimedia content.') .'</p>';
  $output .= '</object>';

  return theme('file_video_render_player', $options, $output, FILE_VIDEO_REALMEDIA_LINK, t('Download the latest RealPlayer'));
}

/**
 * Theme for Adobe Flash .flv video rendering.
 */
function theme_file_video_flash_flv_render($options = array()) {
  extract($options, EXTR_SKIP);

  $height += 24; // Accommodate video controls
  $width_min = 175; //smaller than that breaks player controls
  $player_path = url() . drupal_get_path('module', 'file') .'/vendor/flowplayer';

  $output = '';

  // Check player version.
  if (drupal_substr(FILE_VIDEO_FLOWPLAYER_VERSION, 0, 1) == '2') {
    $output .= '<embed ';
    $output .= 'id="FlowPlayer" ';
    $output .= 'width="'. ($width > $width_min ? $width : $width_min) .'" ';
    $output .= 'height="'. $height .'" ';
    $output .= 'flashvars="config={ playList: [ '. (isset($image) ? '{ url: \''. $image .'\', type: \'jpg\' }, ' : '') .'{ url: \''. $src .'\' } ], fullScreenScriptURL: \''. $player_path .'/html/fullscreen.js\', autoPlay: true, showPlayListButtons: false, showMenu: false, loop: false, initialScale: \'orig\', autoBuffering: false, useNativeFullScreen: true }" ';
    $output .= 'allowfullscreen="true" ';
    $output .= 'allowscriptaccess="sameDomain" ';
    $output .= 'quality="high" ';
    $output .= 'bgcolor="#ffffff" ';
    $output .= 'name="FlowPlayer" ';
    $output .= 'src="'. $player_path .'/FlowPlayerClassic.swf" ';
    $output .= 'type="application/x-shockwave-flash" ';
    $output .= '/>';
  }
  else {
    static $initialized = FALSE;
    if (!$initialized) {
      $initialized = TRUE;
      $output .= '<script src="'. $player_path .'/example/flowplayer-'. FILE_VIDEO_FLOWPLAYER_VERSION .'.min.js"></script>';
    }

    $id = '';
    for ($i = 0; $i < 8; $i++)
      $id .= mt_rand(0, 9);

    $output .= '<div id="file-player-'. $nid .'-'. $id .'" style="width: '. ($width > $width_min ? $width : $width_min) .'px; height: '. $height .'px;"></div>';
    $output .= '<script language="JavaScript">';
    $output .= 'setTimeout(\'flowplayer("file-player-'. $nid .'-'. $id .'", "'. $player_path .'/flowplayer-'. FILE_VIDEO_FLOWPLAYER_VERSION .'.swf", { playlist: [ '. (isset($image) ? '{ url: "'. $image .'", type: "jpg" }, ' : '') .'{ url: "'. $src .'", autoPlay: false, autoBuffering: true } ] });\', \'100\')';
    $output .= '</script>';
  }

  return theme('file_video_render_player', $options, $output, FILE_VIDEO_FLASH_LINK, t('Download the latest Adobe Flash Player'));
}

/**
 * Theme for Adobe Flash .swf video rendering.
 */
function theme_file_video_flash_swf_render($options = array()) {
  extract($options, EXTR_SKIP);

  $output = '<!--[if !IE]> <--><object type="application/x-shockwave-flash" width="'. $width .'" height="'. $height .'" data="'. $src .'"><!--> <![endif]-->'."\n";
  $output .= '<!--[if IE]><object type="application/x-shockwave-flash" width="'. $width .'" height="'. $height .'" classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000" codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,40,0"><![endif]-->'."\n";
  $output .= '<param name="movie" value="'. $src .'" />';
  $output .= "\n".'<p>'. t('Your browser is unable to display this multimedia content.') .'</p>';
  $output .= '</object>';

  return theme('file_video_render_player', $options, $output, FILE_VIDEO_FLASH_LINK, t('Download the latest Adobe Flash Player'));
}

