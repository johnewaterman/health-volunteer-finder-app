<?php

/**
 * @file
 * Implementation of VoIP validation classes
 */

class VoipNumber {

  /*
  * internal id associated with the number; created after the instance is saved into the database
  */
  private $vnid;

  /*
   * might be used for "caller name" functionality
   * if not provided, assume NULL
   */
  private $name;

  /*
   * required in order to differentiate between countries with the same country code
   * if not provided, set to NULL
   */
  private $country_name;

  private $phone_number;

  /*
   * Help users figure out the capabilities associated with the number
   * Possible values: VOIPNUMBER_PSTN, VOIPNUMBER_SIP, VOIPNUMBER_SMS,
   * VOIPNUMBER_MOBILE. The default is VOIPNUMBER_PSTN
   */
  private $type;

  /*  set with the following bit flags (see voipusernumber.module)
   *  VOIPNUMBER_ACTIVE (default is 0x0001)
   *  VOIPNUMBER_VERIFIED (0x0002; default is not verified)
   *  VOIPNUMBER_DEFAULT (0x0004; default is not set)
   */
  private $status;

  private $entity_id; // NULL in case the number is not associated with any entity
  private $entity_type;

  function __construct($number_data = array()) {
    $this->vnid = isset($number_data['vnid']) ? $number_data['vnid'] : NULL;
    $this->name = isset($number_data['name']) ? $number_data['name'] : NULL;
    $this->country_name = isset($number_data['country_name']) ? $number_data['country_name'] : NULL;
    $this->phone_number = isset($number_data['phone_number']) ? $number_data['phone_number'] : NULL;
    $this->type =isset($number_data['type']) ? $number_data['type'] : NULL;
    $this->status = isset($number_data['status']) ? $number_data['status'] : NULL;
    $this->entity_id = isset($number_data['entity_id']) ? $number_data['entity_id'] : NULL;
    $this->entity_type = isset($number_data['entity_type']) ? $number_data['entity_type'] : NULL;

  }

  /**
   * Returns an array with the VoIP Numbers associated with the given number
   *
   * @param $number
   *   String representing either a SIP or an ITU E.164 number
   *
   * @return
   *   Array of VoIP number objects.
   */
  public static function getVoipNumbersByNumber($number, $entity_type = NULL) {
    //Invoke hook_get_voip_numbers()
    $numbers = module_invoke_all('get_voipnumber_by_phone_number', $number, $entity_type);
    return $numbers;
  }

  /**
   * Returns an array with the VoIP Numbers associated with the given user
   *
   * @deprecated use getNumbersFromEid($entity_id, $entity_type)
   *
   * @param $entity_id
   *   Integer representing user id value. If left empty then global user id will be used instead.
   *
   * @return
   *   Array of VoIP number objects.
   */
  public static function getNumbersFromUid($entity_id = NULL) {
    if (empty($entity_id)) {
      global $user;
      $entity_id = $user->uid;
    }
    //Invoke hook_get_voip_numbers()
    $numbers = module_invoke_all('get_voipnumber_by_eid', $entity_id, 'user');
    return $numbers;
  }

  /**
   * Returns the default VoIP Number associated with the given user
   *
   * @deprecated use getDefaultNumberFromEid($entity_id, $entity_type)
   *
   * @param $uid
   *   Integer representing user id value. If left empty then global user id will be used instead.
   *
   * @return
   *   Single VoIP number object.
   */
  public static function getDefaultNumberFromUid($entity_id) {
    if (empty($entity_id)) {
      global $user;
      $entity_id = $user->uid;
    }

    $user_numbers = VoipNumber::getNumbersFromUid($entity_id);
    if (empty($user_numbers)) {
      return NULL;
    }
    foreach($user_numbers as $user_number) {
      if ($user_number->isDefault()) {
        return $user_number;
      }
    }
    #Else: if no default number then return first number in array
    return $user_numbers[0];
  }

  /**
   * Returns an array with the VoIP Numbers associated with the given node
   *
   * @deprecated use getNumbersFromEid($entity_id, $entity_type)
   *
   * @param $nid
   *   Integer representing node id value.
   *
   * @return
   *   Array of VoIP number objects.
   */
  public static function getNumbersFromNid($entity_id) {
    //Invoke hook_get_voip_numbers()
    $numbers = module_invoke_all('get_voipnumber_by_eid', $entity_id, 'node');
    return $numbers;
  }

  /**
   * Returns the default VoIP Number associated with the given node
   *
   * @deprecated use getDefaultNumberFromEid($entity_id, $entity_type)
   *
   * @param $nid
   *   Integer representing node id value.
   *
   * @return
   *   Single VoIP number object.
   */
  public static function getDefaultNumberFromNid($entity_id) {
    $numbers = VoipNumber::getNumbersFromNid($entity_id);
    if (empty($numbers)) {
      return NULL;
    }
    foreach($numbers as $number) {
      if ($number->isDefault()) {
        return $number;
      }
    }
    #Else: if no default number then return first number in array
    return $numbers[0];
  }


  /**
   * Returns an array with the VoIP Numbers associated with the given entity
   *
   * @param $entity_id
   *   Integer representing entity id value.
   * @param $entity_type
   *   String representing entity type.
   *
   * @return
   *   Array of VoIP number objects.
   */
  public static function getNumbersFromEid($entity_id, $entity_type) {
    //Invoke hook_get_voip_numbers()
    $numbers = module_invoke_all('get_voipnumber_by_eid', $entity_id, $entity_type);
    return $numbers;
  }

  /**
   * Returns the default VoIP Number associated with the given entity
   *
   * @param $entity_id
   *   Integer representing entity id value.
   * @param $entity_type
   *   String representing entity type.
   *
   * @return
   *   Single VoIP number object.
   */
  public static function getDefaultNumberFromEid($entity_id, $entity_type) {
    $numbers = VoipNumber::getNumbersFromEid($entity_id, $entity_type);
    if (empty($numbers)) {
      return NULL;
    }
    foreach($numbers as $number) {
      if ($number->isDefault()) {
        return $number;
      }
    }
    #Else: if no default number then return first number in array
    return $numbers[0];
  }

  /**
   * Returns an array with the user ids associated with the specified number
   *
   * @deprecated use getEidsByNumber($number, $type = NULL)
   *
   * @param $number
   *   String containing the number
   *
   * @return
   *   Array of user ids.
   */
  public static function getUidsByNumber($number) {
    $uids = module_invoke_all('get_eid_by_phone_number', $number, 'user');
    return $uids;
  }

  /**
   * Returns an array with the node ids associated with the given number
   *
   * @deprecated use getEidsByNumber($number, $type = NULL)
   *
   * @param $number
   *   String containing the number
   *
   * @return
   *   Array of node ids.
   */
  public static function getNidsByNumber($number) {
    $nids = module_invoke_all('get_eid_by_phone_number', $number, 'node');
    return $nids;
  }

  /**
   * Returns an array with the entity ids associated with the given number
   *
   * @param $number
   *   String containing the number
   * @param $type
   *   String containing the entity type
   *
   * @return
   *   Array of entity ids.
   */
  //@todo: return entity objects instead as this function doesn't make sense if called with $type=NULL
  public static function getEidsByNumber($number, $type = NULL) {
    $entity_ids = module_invoke_all('get_eid_by_phone_number', $number, $type);
    return $entity_ids;
  }


  /**
   * Save the VoIP Number data to the database
   *
   * @return
   *   The updated VoipNumber instance, or NULL in case of an invalid vnid
   */
  public function save() {

    // if this is a new call data being created
    if (is_null($this->vnid)) {
      $data = new stdClass();
      $update = array();

      // initialize 'Name' field
      if (!isset($this->name)) {
        $data->name = NULL;
      }

      // initialize 'country_name' field
      if (!isset($this->country_name)) {
        $data->country_name = NULL;
      }

      // initialize 'phone_number' field
      if (!isset($this->phone_number)) {
        $data->phone_number = '';
      }

      // initialize 'type' field
      if (!isset($this->type)) {
        $data->type = VOIPNUMBER_PSTN;
      }

      // initialize 'status' field
      if (!isset($this->status)) {
        $data->status = 0;
      }

      // initialize 'entity_id' field
      if (!isset($this->entity_id)) {
        $data->entity_id = NULL;
      }

      // initialize 'entity_type' field
      if (!isset($this->entity_type)) {
        $data->entity_type = NULL;
      }
    }
    else {
      // load the data
      $data = db_query("SELECT * FROM {voipnumber} WHERE vnid=:vnid", array(':vnid' => $this->vnid))->fetchObject();
      $update = 'vnid';
    }

    // update voipnumber-specific fields
      $new_name = $this->name;
      if (isset($new_name)) {
        $data->name = $new_name;
      }

      $new_country_name = $this->country_name;
      if (isset($new_country_name)) {
        $data->country_name = $new_country_name;
      }

      $new_phone_number = $this->phone_number;
      if (isset($new_phone_number)) {
        $data->phone_number = $new_phone_number;
      }

      $new_type = $this->type;
      if (isset($new_type)) {
        $data->type = $new_type;
      }

      $new_status = $this->status;
      if (isset($new_status)) {
        $data->status = $new_status;
      }

      $new_entity_id = $this->entity_id;
      if (isset($new_entity_id)) {
        $data->entity_id = $new_entity_id;
      }

      $new_entity_type = $this->entity_type;
      if (isset($new_entity_type)) {
        $data->entity_type = $new_entity_type;
      }

    if($this->isDefault()) {
      //remove default status from any other numbers attached to this entity_id
      if(isset($new_entity_id)) {
        db_query("UPDATE {voipnumber} SET status = (status &~ ".VOIPNUMBER_DEFAULT.")
            WHERE (status & ".VOIPNUMBER_DEFAULT.") = ".VOIPNUMBER_DEFAULT." AND entity_id=:eid AND entity_type=:etype", array(':eid' => $new_entity_id, ':etype' => $new_entity_type));
      }
    }
    // save the data
    drupal_write_record('voipnumber', $data, $update);
    $this->vnid = $data->vnid;
    $this->name = $data->name;
    $this->country_name = $data->country_name;
    $this->phone_number = $data->phone_number;
    $this->type = $data->type;
    $this->status = $data->status;
    $this->entity_id = $data->entity_id;
    $this->entity_type = $data->entity_type;
    return $this;
  }

  /**
   * Load a VoIP Number from the database
   *
   * @param $vnid
   *   The vnid of the voip number row
   *
   * @return
   *   The VoipNumber instance, or NULL in case of an invalid vnid
   */
  public static function load($vnid) {
    $voipnumber = NULL;
    $voipnumber_data = db_query("SELECT * FROM {voipnumber} WHERE vnid=:vnid", array(':vnid'=>$vnid))->fetchAssoc();
    if ($voipnumber_data) {
      $voipnumber = new VoipNumber($voipnumber_data);
    }
    return $voipnumber;
  }

  /**
   * Delete the VoIP Number data from the database
   *
   * @return
   *   TRUE or FALSE result of operation
   */
  public function delete() {
    $vnid = $this->vnid;
    $res = db_delete('voipnumber')
      ->condition('vnid', $vnid)
      ->execute();
    $this->vnid = NULL;
    return $res;
  }

  /*
   * Getters and setters
   */
  function getName() {
    return $this->name;
  }

  function setName($name) {
    $this->name = $name;
  }

  function getCountryName() {
    return $this->country_name;
  }

  function setCountryName($country_name) {
    $this->country_name = $country_name;
  }

  function getNumber() {
    return $this->phone_number;
  }

  function setNumber($phone_number) {
    $this->phone_number = $phone_number;
  }

  function getType() {
    return $this->type;
  }

  function setType($type) {
    $this->type = $type;
  }

  /*function getStatus() {
    return $this->status;
  }
*/
  /*function setStatus($status) {
    $this->status = $status;
  }*/

  /**
   * Make number in/active.
   *
   * @param bool $active
   *   TRUE active; FALSE inactive.
   */
  public function setActive($active) {
    if ($active) {
      $this->status = $this->status | VOIPNUMBER_ACTIVE;
    }
    else {
      $this->status = $this->status & ~ VOIPNUMBER_ACTIVE;
    }
  }

  /**
   * Make number un/confirmed.
   *
   * @param bool $confirmed
   *   (optional) TRUE confirmed, default; FALSE unconfirmed.
   */
  public function setVerified($verified = TRUE) {
    if ($verified) {
      $this->status = $this->status | VOIPNUMBER_VERIFIED;
    }
    else {
      //$this->status = $this->status ^ VOIPNUMBER_VERIFIED;
      $this->status = $this->status & ~ VOIPNUMBER_VERIFIED;
    }
  }

  /**
   * Make number (not) default.
   *
   * NOTE this method is removing default
   * status from any other numbers attached to this uid or nid after save()
   * operation
   *
   * @param bool $default
   *   (optional) TRUE default, default; FALSE not default.
   */
  public function setDefault($default) {
    if ($default) {
      $this->status = $this->status | VOIPNUMBER_DEFAULT;
    }
    else {
      $this->status = $this->status & ~ VOIPNUMBER_DEFAULT;
    }
  }

  /**
   * Get active status.
   *
   * @return bool
   *   TRUE if active.
   */
  public function isActive() {
    if($this->status & VOIPNUMBER_ACTIVE) {
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Get verified status.
   *
   * @return bool
   *   TRUE if verified.
   */
  public function isVerified() {
    if($this->status & VOIPNUMBER_VERIFIED) {
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Get default status.
   *
   * @return bool
   *   TRUE if this is a default number.
   */
  public function isDefault() {
    if($this->status & VOIPNUMBER_DEFAULT) {
      return TRUE;
    }
    return FALSE;
  }

  /**
   * @deprecated use getEid()
   */
  function getUid() {
    if($this->entity_type == 'user') {
      return $this->entity_id;
    } else {
      return NULL;
    }
  }

  /**
   * @deprecated use setEid($entity_id) and setEtype($entity_type)
   */
  function setUid($uid) {
    $this->entity_id = $uid;
    $this->entity_type = 'user';
  }

  /**
   * @deprecated use getEid()
   */
  function getNid() {
    if($this->entity_type == 'node') {
      return $this->entity_id;
    } else {
      return NULL;
    }
  }

  /**
   * @deprecated use setEid($entity_id) and setEtype($entity_type)
   */
  function setNid($nid) {
    $this->entity_id = $nid;
    $this->entity_type = 'node';
  }

  function getEid() {
    return $this->entity_id;
  }

  function setEid($entity_id) {
    $this->entity_id = $entity_id;
  }

  function setEntityType($entity_type) {
    $this->entity_type = $entity_type;
  }

  function getEntityType() {
    return $entity_type;
  }

  function getVnid() {
    return $this->vnid;
  }

  /**
   * Returns string with the country code
   *
   * @param $country
   *   String containing the country name
   *
   * @return
   *   string with the country code
   */
  public static function getCodeFromCountry($country) {
    $code = db_query("SELECT code FROM {voipnumber_dialing_codes} WHERE LOWER(country) =:country",
      array(':country' => strtolower($country)))->fetchField();

    return $code;
  }

  /**
   * Returns string with the country name or an array of country names
   *
   * @param $code
   *   Country code in normalized format
   *
   * @return
   *   string or array with country names
   */
  public static function getCountryFromCode($code) {
    //$code = VoipNumber::NormalizeNumber($code);
    $code = str_replace('+', '', $code);
    $country = array();
    $query = db_select('voipnumber_dialing_codes', 'v')
      ->fields('v', array(
        'country',
      ))
      ->condition('code' , $code)
      ->execute();
    while($row = $query->fetchAssoc()) {
      $country[] = $row['country'];
    }
    if(empty($country)) {
      return NULL;
    }
    else if(count($country) === 1) {
      return $country[0];
    }
    #else
    return $country;
  }

  /**
   * Returns string with the country code
   *
   * @param $number
   *   Telephone number in normalized format
   *
   * @return
   *   string with country code
   */
  public static function getCodeFromNumber($number) {
    //$number = VoipNumber::NormalizeNumber($number);
    $number = str_replace('+', '', $number);
    $tmp_code = substr($number, 0, 4); //Country code length is max 4 digits
    for($i=strlen($tmp_code); $i>=0; $i--) {
      $code = substr($tmp_code, 0, $i);
      $country_code = db_query("SELECT code FROM {voipnumber_dialing_codes} WHERE code =:code",
        array(':code' => $code))->fetchField();
      if($country_code) {
        return $country_code;
      }
    }
    return NULL;
  }

  /**
   * Returns string with the country name or an array of country names
   *
   * @param $number
   *   Telephone number in any of following formats(123456789, +123456789, 00123456789)
   *
   * @return
   *   string or array with country names
   */
  public static function getCountryFromNumber($number) {
    $code = VoipNumber::getCodeFromNumber($number);
    $country = VoipNumber::getCountryFromCode($code);
    return $country;
  }
}
