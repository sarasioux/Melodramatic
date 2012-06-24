<?php

/**
 * @file
 * A sample file.
 */

/**
 * @defgroup samples Samples
 *
 * A sample group.
 *
 * @{
 */

/**
 * A sample global.
 */
global $sample_global;

/**
 * A sample constant.
 */
define('SAMPLE_CONSTANT');

/**
 * A sample function.
 *
 * @see duplicate_function()
 *
 * Use for sample-related purposes.
 *
 * @param $parameter
 *   A generic parameter.
 * @param $complex_parameter
 *   Information about the $complex_parameter parameter. Example:
 *   @code
 *     $complex_parameter = 3;
 *   @endcode
 *
 *   A second paragraph about the $complex_parameter parameter.
 *
 *   @link http://php.net this is a link for the parameter @endlink
 *
 * @return
 *   Something about the return value.
 *
 *   A second paragraph about the return value.
 *
 * @see SAMPLE_CONSTANT
 */
function sample_function($parameter, $complex_parameter) {
}

/**
 * @} end samples
 */

/**
 * For testing duplicate function name linking.
 */
function duplicate_function() {
}

/**
 * For testing duplicate constant linking.
 */
define('DUPLICATE_CONSTANT');
