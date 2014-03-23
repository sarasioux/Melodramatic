<?php

class Mandrill_Exception extends Exception {
}

class Mandrill {
  const API_VERSION = '1.0';
  const END_POINT = 'https://mandrillapp.com/api/';

  var $api;

  /**
   * Default to a 300 second timeout on server calls
   */
  var $timeout = 300;

  function __construct($api_key, $timeout = 300) {
    if (empty($api_key)) {
      throw new Mandrill_Exception('Invalid API key');
    }
    try {
      $response = $this->request('users/ping', array('key' => $api_key));
      if ($response != 'PONG!') {
        throw new Mandrill_Exception('Invalid API key');
      }

      $this->api = $api_key;
      $this->timeout = $timeout;

    } catch (Exception $e) {
      throw new Mandrill_Exception($e->getMessage());
    }
  }

  /**
   * Work horse. Every API call use this function to actually make the request to Mandrill's servers.
   *
   * @link https://mandrillapp.com/api/docs/
   *
   * @param string $method API method name
   * @param array $args query arguments
   * @param string $http GET or POST request type
   * @param string $output API response format (json,php,xml,yaml). json and xml are decoded into arrays automatically.
   *
   * @return array|string|Mandrill_Exception
   */
  function request($method, $args = array(), $http = 'POST', $output = 'json') {
    if (!isset($args['key'])) {
      $args['key'] = $this->api;
    }

    $api_version = self::API_VERSION;
    $dot_output = ('json' == $output) ? '' : ".{$output}";

    $url = self::END_POINT . "{$api_version}/{$method}{$dot_output}";
    $params = json_encode($args);

    switch ($http) {
      case 'GET':
        $url .= '?' . $params;
        $response = drupal_http_request($url, array(), 'GET', NULL, 3, $this->timeout);
        break;

      case 'POST':
        $response = drupal_http_request($url, array(), 'POST', $params, 3, $this->timeout);
        break;

      default:
        throw new Mandrill_Exception('Unknown request type');
    }

    $response_code = $response->code;
    $body = $response->data;

    switch ($output) {
      case 'json':
        $body = json_decode($body, TRUE);
        break;

      case 'php':
        $body = unserialize($body);
        break;
    }

    if (200 == $response_code) {
      return $body;
    }
    else {
      $message = isset($body['message']) ? $body['message'] : '';
      throw new Mandrill_Exception($message . ' - ' . $body, $response_code);
    }
  }

  /**
   * @link https://mandrillapp.com/api/docs/users.html#method=ping
   *
   * @return array|Mandrill_Exception
   */
  function users_ping() {
    return $this->request('users/ping');
  }

  /**
   * @link https://mandrillapp.com/api/docs/users.html#method=info
   *
   * @return array|Mandrill_Exception
   */
  function users_info() {
    return $this->request('users/info');
  }

  /**
   * @link https://mandrillapp.com/api/docs/users.html#method=senders
   *
   * @return array|Mandrill_Exception
   */
  function users_senders() {
    return $this->request('users/senders');
  }

  /**
   * @link https://mandrillapp.com/api/docs/senders.html#method=domains
   *
   * @return array|Mandrill_Exception
   */
  function senders_domains() {
    return $this->request('senders/domains');
  }

  /**
   * @link https://mandrillapp.com/api/docs/senders.html#method=list
   *
   * @return array|Mandrill_Exception
   */
  function senders_list() {
    return $this->request('senders/list');
  }

  /**
   * @link https://mandrillapp.com/api/docs/senders.html#method=info
   *
   * @return array|Mandrill_Exception
   */
  function senders_info($email) {
    return $this->request('senders/info', array('address' => $email));
  }

  /**
   * @link https://mandrillapp.com/api/docs/senders.html#method=time-series
   *
   * @return array|Mandrill_Exception
   */
  function senders_time_series($email) {
    return $this->request('senders/time-series', array('address' => $email));
  }

  /**
   * @link https://mandrillapp.com/api/docs/tags.html#method=list
   *
   * @return array|Mandrill_Exception
   */
  function tags_list() {
    return $this->request('tags/list');
  }

  /**
   * @link https://mandrillapp.com/api/docs/tags.html#method=info
   *
   * @return array|Mandrill_Exception
   */
  function tags_info($tag) {
    return $this->request('tags/info', array('tag' => $tag));
  }

  /**
   * @link https://mandrillapp.com/api/docs/tags.html#method=time-series
   *
   * @return array|Mandrill_Exception
   */
  function tags_time_series($tag) {
    return $this->request('tags/time-series', array('tag' => $tag));
  }

  /**
   * @link https://mandrillapp.com/api/docs/tags.html#method=all-time-series
   *
   * @return array|Mandrill_Exception
   */
  function tags_all_time_series() {
    return $this->request('tags/all-time-series');
  }

  /**
   * @link https://mandrillapp.com/api/docs/templates.html#method=add
   *
   * @return array|Mandrill_Exception
   */
  function templates_add($name, $code) {
    return $this->request('templates/add', array(
      'name' => $name,
      'code' => $code
    ));
  }

  /**
   * @link https://mandrillapp.com/api/docs/templates.html#method=update
   *
   * @return array|Mandrill_Exception
   */
  function templates_update($name, $code) {
    return $this->request('templates/update', array(
      'name' => $name,
      'code' => $code
    ));
  }

  /**
   * @link https://mandrillapp.com/api/docs/templates.html#method=delete
   *
   * @return array|Mandrill_Exception
   */
  function templates_delete($name) {
    return $this->request('templates/delete', array('name' => $name));
  }

  /**
   * @link https://mandrillapp.com/api/docs/templates.html#method=info
   *
   * @return array|Mandrill_Exception
   */
  function templates_info($name) {
    return $this->request('templates/info', array('name' => $name));
  }

  /**
   * @link https://mandrillapp.com/api/docs/templates.html#method=list
   *
   * @return array|Mandrill_Exception
   */
  function templates_list() {
    return $this->request('templates/list');
  }

  /**
   * @link https://mandrillapp.com/api/docs/templates.html#method=time-series
   *
   * @return array|Mandrill_Exception
   */
  function templates_time_series($name) {
    return $this->request('templates/time-series', array('name' => $name));
  }

  /**
   * @link https://mandrillapp.com/api/docs/urls.html#method=list
   *
   * @return array|Mandrill_Exception
   */
  function urls_list() {
    return $this->request('urls/list');
  }

  /**
   * @link https://mandrillapp.com/api/docs/urls.html#method=time-series
   *
   * @return array|Mandrill_Exception
   */
  function urls_time_series($url) {
    return $this->request('urls/time-series', array('url' => $url));
  }

  /**
   * @link https://mandrillapp.com/api/docs/urls.html#method=search
   *
   * @return array|Mandrill_Exception
   */
  function urls_search($q) {
    return $this->request('urls/search', array('q' => $q));
  }

  /**
   * @link https://mandrillapp.com/api/docs/webhooks.html#method=add
   *
   * @return array|Mandrill_Exception
   */
  function webhooks_add($url, $events) {
    return $this->request('webhooks/add', array(
      'url' => $url,
      'events' => $events
    ));
  }

  /**
   * @link https://mandrillapp.com/api/docs/webhooks.html#method=update
   *
   * @return array|Mandrill_Exception
   */
  function webhooks_update($url, $events) {
    return $this->request('webhooks/update', array(
      'url' => $url,
      'events' => $events
    ));
  }

  /**
   * @link https://mandrillapp.com/api/docs/webhooks.html#method=delete
   *
   * @return array|Mandrill_Exception
   */
  function webhooks_delete($id) {
    return $this->request('webhooks/delete', array('id' => $id));
  }

  /**
   * @link https://mandrillapp.com/api/docs/webhooks.html#method=info
   *
   * @return array|Mandrill_Exception
   */
  function webhooks_info($id) {
    return $this->request('webhooks/info', array('id' => $id));
  }

  /**
   * @link https://mandrillapp.com/api/docs/webhooks.html#method=list
   *
   * @return array|Mandrill_Exception
   */
  function webhooks_list() {
    return $this->request('webhooks/list');
  }

  /**
   * @link https://mandrillapp.com/api/docs/messages.html#method=search
   *
   * @return array|Mandrill_Exception
   */
  function messages_search($query, $date_from = '', $date_to = '', $tags = array(), $senders = array(), $limit = 100) {
    return $this->request('messages/search', compact('query', 'date_from', 'date_to', 'tags', 'senders', 'limit'));
  }

  /**
   * @link https://mandrillapp.com/api/docs/messages.html#method=send
   *
   * @return array|Mandrill_Exception
   */
  function messages_send($message) {
    return $this->request('messages/send', array('message' => $message));
  }

  /**
   * @link https://mandrillapp.com/api/docs/messages.html#method=send-template
   *
   * @return array|Mandrill_Exception
   */
  function messages_send_template($template_name, $template_content, $message) {
    return $this->request('messages/send-template', compact('template_name', 'template_content', 'message'));
  }

  static function getAttachmentStruct($path) {
    $struct = array();

    try {

      if (!@is_file($path)) {
        throw new Exception($path . ' is not a valid file.');
      }

      $filename = basename($path);

      if (!function_exists('get_magic_quotes')) {
        function get_magic_quotes() {
          return FALSE;
        }
      }
      if (!function_exists('set_magic_quotes')) {
        function set_magic_quotes($value) {
          return TRUE;
        }
      }

      if (strnatcmp(phpversion(), '6') >= 0) {
        $magic_quotes = get_magic_quotes_runtime();
        set_magic_quotes_runtime(0);
      }

      $file_buffer = file_get_contents($path);
      $file_buffer = chunk_split(base64_encode($file_buffer), 76, "\n");

      if (strnatcmp(phpversion(), '6') >= 0) {
        set_magic_quotes_runtime($magic_quotes);
      }

      $mime_type = file_get_mimetype($path);
      if (!Mandrill::isValidContentType($mime_type)) {
        throw new Exception($mime_type . ' is not a valid content type (it should be ' . implode('*,', self::getValidContentTypes()) . ').');
      }

      $struct['type'] = $mime_type;
      $struct['name'] = $filename;
      $struct['content'] = $file_buffer;

    } catch (Exception $e) {
      throw new Mandrill_Exception('Error creating the attachment structure: ' . $e->getMessage());
    }

    return $struct;
  }

  /**
   * Helper to determine attachment is valid.
   *
   * @static
   *
   * @param $ct
   *
   * @return bool
   */
  static function isValidContentType($ct) {
    $valids = self::getValidContentTypes();

    foreach ($valids as $vct) {
      if (strpos($ct, $vct) !== FALSE) {
        return TRUE;
      }
    }

    return FALSE;
  }

  /**
   * Return an array of valid content types.
   *
   * @static
   *
   * @return array
   */
  static function getValidContentTypes() {
    return array(
      'image/',
      'text/',
      'application/pdf',
    );
  }
}

