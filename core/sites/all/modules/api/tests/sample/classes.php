<?php

/**
 * @file Object-oriented tests.
 */

/**
 * @defgroup class_samples Class Samples
 *
 * A sample group of classes. Should not include members.
 *
 * @{
 */

/**
 * Sample class.
 */
class Sample implements SampleInterface {
  /**
   * A class constant.
   */
  const constant = 'constant value';

  /**
   * A property.
   *
   * @var SampleInterface
   */
  private $property = 'variable value';

  /**
   * Metasyntatic member function.
   *
   * @throws SampleException when it all goes wrong.
   */
  public function foo() {
  }

  /**
   * Only implemented in children.
   */
  public function baz() {
  }
}

/**
 * Sample interface.
 */
interface SampleInterface {
  /**
   * Implement this API.
   */
  public function foo();
}

/**
 * Subclass.
 *
 * @see Sample::foo() should be a link
 */
class SubSample extends Sample implements SampleInterfaceTwo {
  // Not documented.
  public function bar() {
  }
}

/**
 * Another Sample interface.
 */
interface SampleInterfaceTwo {
  /**
   * A public method.
   */
  public function bar();
}

$random_assignment_not_to_be_parsed = NULL;

class Sample2 implements SampleInterface {
  public function baz() {
  }
}

/**
 * @} end samples
 */
