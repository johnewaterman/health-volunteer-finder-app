<?php

require 'DrupalIntegrationTestCase.php';

class TbcareNotificationsTestCase extends DrupalIntegrationTestCase {

  public function setUp() {
    parent::setUp();
    $this->allCommentsSubscriber = $this->drupalCreateUser();
    $this->superEditorCommentsSubscriber = $this->drupalCreateUser();
    $this->commenter = $this->drupalCreateUser();
    $node_settings = array(
      'title' => 'An example case study',
      'type' => TBCARE_NODE_TYPE_CASE_PRESENTATION,
    );
    $this->commentableNode = $this->drupalCreateNode($node_settings);
    tbcare_notifications_subscription_update($this->allCommentsSubscriber->uid, 'all_comments');
    tbcare_notifications_subscription_update($this->superEditorCommentsSubscriber->uid, 'super_editor_comments');
  }

  public function test_tbcare_notifications_all_comments_subscriptions() {
    $subscribers = tbcare_notifications_subscribers(array('all_comments'));
    $this->assertNotEmpty($subscribers);
    $this->assertArrayHasKey($this->allCommentsSubscriber->uid, $subscribers);
    $this->assertArrayNotHasKey($this->superEditorCommentsSubscriber->uid, $subscribers);
  }

  public function test_tbcare_notifications_super_editor_comments_subscriptions() {
    $subscribers = tbcare_notifications_subscribers(array('super_editor_comments'));
    $this->assertNotEmpty($subscribers);
    $this->assertArrayHasKey($this->allCommentsSubscriber->uid, $subscribers);
    $this->assertArrayHasKey($this->superEditorCommentsSubscriber->uid, $subscribers);
  }

  public function test_tbcare_notifications_test() {
    $comment = (object) array(
      'nid' => $this->commentableNode->nid,
      'uid' => $this->commenter->uid,
      'mail' => '',
      'is_anonymous' => 0,
      'homepage' => '',
      'status' => COMMENT_PUBLISHED,
      'subject' => 'Test comment subject.',
      'language' => LANGUAGE_NONE,
      'comment_body' => array(
        LANGUAGE_NONE => array(
          0 => array (
            'value' => 'Test comment body',
            'format' => 'filtered_html'
          )
        ),
      ),
      'created' => REQUEST_TIME,
      'changed' => REQUEST_TIME,
    );
    comment_submit($comment);
    comment_save($comment);

    $captured_emails = variable_get('drupal_test_email_collector', array());

    $this->assertNotEmpty($captured_emails);

    // Each Super Admin (role id 6) receive exactly one of each e-mail.
    $emails = array();
    $super_user_uids = array();
    $query = 'SELECT DISTINCT(ur.uid) FROM {users_roles} AS ur LEFT JOIN {users} u ON ur.uid = u.uid WHERE ur.rid IN (:rids) AND u.status = 1';
    $result = db_query($query, array(':rids' => array(TBCARE_ROLE_SUPER_EDITOR)));
    foreach ($result as $row) {
      $super_user_uids[] = $row->uid;
    }
    $super_users = user_load_multiple($super_user_uids);
    $super_user_tos = array();
    foreach ($super_users as $super_user) {
      $super_user_tos[] = $super_user->name . ' <' . $super_user->mail . '>';
    }
    foreach ($captured_emails as $email) {
      $id = $email['id'];
      if (!isset($emails[$id])) {
        $emails[$id] = array();
      }
      if (in_array($email['to'], $super_user_tos)) {
        $emails[$id][$email['to']] = TRUE;
      }
    }
    foreach ($emails as $id => $data) {
      // An ID should be a string like 'tbcare_notifications_case_presentation_first_comment_all'
      $this->assertStringStartsWith('tbcare_', $id);
      // Each e-mail should be sent to each super user (same count).
      $this->assertEquals(count($data), count($super_users));
    }
  }
}
