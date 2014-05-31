<?php
/**
 * @file
 * PHPUnit_Framework_TestListener implementation for Drupal integration tests
 */

class DrupalTestListener implements PHPUnit_Framework_TestListener {

  /**
   * @var DatabaseTransaction
   */
  protected $transaction;

  /**
   * Runs at the start of each test suite.
   *
   * @param \PHPUnit_Framework_TestSuite $suite
   *   A test suite
   */
  public function startTestSuite(\PHPUnit_Framework_TestSuite $suite) {
    if (!function_exists('drupal_boostrap')) {
      require_once DRUPAL_ROOT . '/includes/bootstrap.inc';
    }
    $phase = drupal_bootstrap();
    if ($phase < DRUPAL_BOOTSTRAP_FULL) {
      // @codingStandardsIgnoreStart
      $_SERVER['REMOTE_ADDR'] = '127.0.0.1';
      $_SERVER['REQUEST_METHOD'] = NULL;
      // @codingStandardsIgnoreEnd
      drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
    }
    require_once 'TestDatabaseTransaction.php';

    // Use the test mail class instead of the default mail handler class.
    variable_set('mail_system', array('default-system' => 'TestingMailSystem'));

  }

  /**
   * Runs at the end of each test suite.
   *
   * @param \PHPUnit_Framework_TestSuite $suite
   *   A test suite
   */
  public function endTestSuite(\PHPUnit_Framework_TestSuite $suite) {
  }

  /**
   * Runs before each test.
   *
   * Wraps all database statements executed during a test in a transaction.
   *
   * @param \PHPUnit_Framework_Test $test
   *   A test case
   */
  public function startTest(\PHPUnit_Framework_Test $test) {
    $this->transaction = new TestDatabaseTransaction(Database::getConnection('default'));
  }

  /**
   * Runs after each test.
   *
   * Rolls back the trasaction started before each test to leave the database
   * unchanged for the next test.
   *
   * @param \PHPUnit_Framework_Test $test
   *   A test case
   * @param float $time
   *   A timestamp
   */
  public function endTest(\PHPUnit_Framework_Test $test, $time) {
    $this->transaction->rollback();
  }

  /**
   * Runs when an error occurs.
   *
   * @param \PHPUnit_Framework_Test $test
   *   A test case
   * @param \Exception $e
   *   An exception thrown by the test
   * @param float $time
   *   A timestamp
   */
  public function addError(\PHPUnit_Framework_Test $test, \Exception $e, $time) {
  }

  /**
   * Runs when a failure occurs.
   *
   * @param \PHPUnit_Framework_Test $test
   *   A test case
   * @param \PHPUnit_Framework_AssertionFailedError $e
   *   An exception thrown by the test case
   * @param float $time
   *   A timestamp
   */
  public function addFailure(\PHPUnit_Framework_Test $test, \PHPUnit_Framework_AssertionFailedError $e, $time) {
  }

  /**
   * Runs a test case is incomplete.
   *
   * @param \PHPUnit_Framework_Test $test
   *   A test case
   * @param \Exception $e
   *   An exception thrown by the test case
   * @param float $time
   *   A timestamp
   */
  public function addIncompleteTest(\PHPUnit_Framework_Test $test, \Exception $e, $time) {
  }

  /**
   * Runs when a test case is skipped.
   *
   * @param \PHPUnit_Framework_Test $test
   *   A test case
   * @param \Exception $e
   *   An exception thrown by the test case
   * @param float $time
   *   A timestamp
   */
  public function addSkippedTest(\PHPUnit_Framework_Test $test, \Exception $e, $time) {
  }
}
