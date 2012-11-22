<?php
/**************************************************************
 **   fbUser_class - Copyright 2012 RazorWire Solutions, Inc. 
 **************************************************************
 *
 * Facebook App User Management Class
 * For use with Facebook PHP SDK 3.1.1
 *
 *  REQUIREMENTS:
 *      - PHP5+, with pdo_mysql
 *      - MySQL5+
 *      - Facebook PHP SDK 3.1.1
 *
 * @package fbAppClass
 * @author RWSDev Team (Jason Becht)
 * @version 1.0
 * @copyright 2012
 **************************************************************/

class fbUserClass {

	/**** configuration ****/
	private $database 		= array(
								'hostname'	=>	'DB_HOST',
								'username'	=>	'DB_USER',
								'password'	=>	'DB_PASSWORD',
								'db_name'	=>	'DB_NAME'
							);
	private $table_name 	 = 'users';
	private $primary_key 	 = 'userid';
	private $db_engine 		 = 'innodb';
	/**** requred fields in table, advised to leave intact ****/
	private $struct_required = array(
								'userid' => array(
										'type'			=>	'string',
										'length'		=> 	64,
										'null'			=>	false,
										'validation'	=>	array(
																array(
																	'type'		=> 'unique',
																	'used_for'	=> array('new'),
																	'message'	=> 'Userid already exists.'
																	)
																)
										),
								'fb_access_token' => array(
										'type'			=>	'string',
										'length'		=> 	255,
										'null'			=>	false
										),
								'email'		=> array(
										'type'			=>	'string',
										'length'		=> 	255,
										'null'			=>	false,
										'validation'	=>	array(
																array(
																	'type'		=> 'unique',
																	'used_for'	=> array('new'),
																	'message'	=> 'Email already exists'
																	),
																array(
																	'type'		=> 'func',
																	'value'		=> 'validEmail',
																	'used_for'	=> array('new', 'update'),
																	'message'	=> 'Email is not valid'
																	)
																)
										),
							'first_name'	=> array(
										'type'			=>	'string',
										'length'		=> 	32,
										'null'			=>	false,
										'validation'	=>  array(
																array(
																	'type'		=>	'regexp',
																	'value'		=>	'/^[a-zA-Z\ ]+$/',
																	'used_for'	=>  array('new', 'update'),
																	'message'	=>	'Firstname can include only letters and spaces'
																	)
															)
										),
							'last_name'	=> array(
										'type'			=>	'string',
										'length'		=> 	32,
										'null'			=>	false,
										'validation'	=>  array(
																array(
																	'type'		=>	'regexp',
																	'value'		=>	'/^[a-zA-Z\ ]+$/',
																	'used_for'	=>  array('new', 'update'),
																	'message'	=>	'Lastname can include only letters and spaces'
																	)
															)
										),
							'friends'	=> array(
										'type'			=>	'longtext',
										'null'			=>	true,
										),
								);
	/**** optional fields in db table, add remove modify... do as you please ****/
	private $struct_optional = array();

	private $table_indexes = array(
								'userid'	=>	'primary key',
								'email'		=>	'unique'
							);
	/**** end of configuration ****/
	private $objInstance; 
	private $classStatus = 'NEW';
	public $validationErrors = array();
	
	/**
	 * Constructor
	 * @param $id int userid
	 */
	public function __construct($id='-1') {
		$this->translateGlobals();
		if (!$this->checkIfTableExists()) {
			$this->suggestCreateTable();
        }
        
		if ($id!='-1') {
			$this->{$this->primary_key} = $id;
			$this->load();
		}
	}

	/**
	 * load user by userid
	 * @param $id intreger - userid
	 * @return Object - user class object
	 */
	public function load($id='-1') {
		if ((!isset($this->{$this->primary_key}) || $this->{$this->primary_key}=='-1') &&	$id!='-1') {
				$this->{$this->primary_key} = $id;
			}
			
		$sql = "select * from {$this->table_name}
				where {$this->primary_key}=:primary_key";
		$stmt = $this->db()->prepare($sql);
		$stmt->bindValue(":primary_key", $this->{$this->primary_key});
		$stmt->execute();
		$result = $stmt->fetch(PDO::FETCH_OBJ);

		if (!$result) return false;

		$this->classStatus = 'UPDATE';
        $this->user = $result;
		return $this;
	}

	/**
	 * load user by any
	 * @param $field string - database field name to load with, must be unique
	 * @param $value string - value to load by
	 * @return Array - results
	 */
	public function loadBy($field, $value) {
		$result = array();
		$sql = "select * from {$this->table_name}
				where {$field}=:value";
		$stmt = $this->db()->prepare($sql);
		$stmt->bindValue(":value", $value);
		$stmt->execute();
		$result = $stmt->fetch(PDO::FETCH_OBJ);

        if (!$result) return false;

        $this->classStatus = 'UPDATE';
        $this->user = $result;
		return $this;
	}

	/**
	 * save user to db (insert and update)
	 * @param $id string - userid
	 * @return Object - user class object
	 */
	public function save($id='-1') {
		if ($id!='-1') {
			$this->load($id);
		}
		if (!$this->validate()){
			$this->load($id);
			return false;
        }
		if ($this->classStatus == 'NEW')
			return $this->insert();
		else if ($this->classStatus == 'UPDATE')
			return $this->update($id);
	}

	/**
	 * insert new user to db
	 * @return Object - user class object
	 */
	private function insert() {
		$fields = array();
		$values = array();
		$sql = "insert into {$this->table_name} (";
		$struct = $this->getTableStructure();
		foreach($struct as $field_name=>$field)
		{
			if (isset($this->$field_name)) { 
				$fields[$field_name] = ":".$field_name;
				$values[$field_name] = $this->prepare($field_name, $this->$field_name);
			} elseif (isset($field['default'])) {
				$fields[$field_name] = ":".$field_name;
				$values[$field_name] = $this->prepare($field_name, $field['default']);
			}
		}
		$sql .= implode(",",array_keys($fields)).') values ('.implode(",",$fields).')';

		$stmt = $this->db()->prepare($sql);
		foreach($values as $fn=>$value)
		{
			$stmt->bindValue(":{$fn}", $value);
		}
		$stmt->execute();
		$this->setLastInsertId($this->db()->lastInsertId());
		return $this;
	}

	/**
	 * update user in db
	 * @param $id string - userid - optional
	 * @return Object - user class object
	 */
	private function update($id='-1') {
		if ($id!='-1') {
			$this->load($id);
		}
        $updateSessionArr = array();

		$sql = "select * from {$this->table_name}
				where userid='$id'";
		$stmt = $this->db()->prepare($sql);
		$stmt->execute();
		$result = $stmt->fetch(PDO::FETCH_ASSOC);

		$fields = array();
		$values = array();
		$sql = "update {$this->table_name} set ";
		foreach ($result as $field_name=>$value) {
			if (isset($this->$field_name)) {
				if ($this->$field_name!=$value) {
					$fields[$field_name] = "$field_name=:$field_name";
					$values[$field_name] = $this->prepare($field_name, $this->$field_name);
				}
			}
		}
        if (!$fields) {
            return $this;
        }
		$sql .= implode(",", $fields);
        $sql .= ' WHERE userid="'.$id.'"';
		$stmt = $this->db()->prepare($sql);
		foreach ($values as $fn=>$value) {
            $updateSessionArr[$fn] = $value;
			$stmt->bindValue(":$fn", $value);
		}
		$stmt->execute();
		return $this;
	}

	/**
	 * delete user from db
	 * @param $id integer - userid - optional
	 */
	public function delete($id='-1') {
		if ($id!='-1')
			$this->load($id);

		$sql = "delete from {$this->table_name}
				where {$this->primary_key}=:primary_key";
		$stmt = $this->db()->prepare($sql);
		$stmt->bindValue(":primary_key", $this->{$this->primary_key});
		$stmt->execute();
		$this->resetClassStatus();	
	}
    
	/**
	 *	Searches user database for given germ through given fields,
	 *	accepts * as wildcard
	 *	@param search_term string
	 *	@param operator string - accepted values: OR, AND
	 *	@param $fields array - list of fields you want to search through
	 *	@return Array
	 */
	public function search($search_term, $operator = 'or', $fields=array('memberid', 'email')) {
	
		$query = str_replace("*", "%", $search_term);
		$likes = array();
		$results = array();
		$sql = "select * from {$this->table_name}
				where";
				foreach($fields as $field) {
					$likes[] = " $field like '%$query%' ";
				}
		$sql .= implode(" $operator ", $likes);

		$stmt = $this->db()->prepare($sql);
		$stmt->execute();
		while ($row = $stmt->fetch(PDO::FETCH_OBJ)) {
			$results[] = $row;
		}
		
		return $results;
	}

	/**
	 * reset class status, NEW and UPDATE modes
	 * @param $id integer - userid - optional
	 * @return Object - user class object
	 */
	private function resetClassStatus() {
		$this->classStatus = 'NEW';
		$struct = $this->getTableStructure();
		foreach($struct as $field_name=>$field)
		{
			if (isset($this->$field_name))
				unset($this->$field_name);
		}
		return $this;
	}
	
	/**
	 * helper: sets userid
	 * @param $id integer - userid 
	 * @return Object - user class object
	 */
	private function setLastInsertId($id) {
		$this->{$this->primary_key} = $id;
		return $this;
	}
	
	/**
	 * prepares value for insert
	 * @param $field_name string - db field name 
	 * @param $field_value mixed - value for field
	 * @return mixed - prepared value for field
	 */
	private function prepare($field_name, $field_value)	{
		$struct = $this->getTableStructure();
		if (isset($struct[$field_name]['passthrough']))
			return call_user_func($struct[$field_name]['passthrough'], $field_value);
		else if ($struct[$field_name]=='date' || $struct[$field_name]=='datetime') {
			$timestamp = strtodate($field_value);
			return date('Y-m-d H:i:s', $timestamp );
		} else {
			return $field_value;
		}
	}
	
	/**
	 * validate method, sets self::validationErrors
	 * @return boolean
	 */
	public function validate() {
		$this->validationErrors = array();
		$struct = $this->getTableStructure();
		foreach($struct as $field_name=>$field)
		{
			if (!$field['null'] && !isset($this->$field_name) && $field['auto_increment']!=true) {
				$this->validationErrors[] = "{$field_name} is required";
				continue;
			}
			if (isset($field['validation']) && sizeof($field['validation']>0) && trim($this->$field_name)!='') {
				foreach($field['validation'] as $validation_data) {
					if (!in_array(strtolower($this->classStatus), $validation_data['used_for'])) continue;
					/* unique validation */
					if ($validation_data['type']=='unique') {
						if (!$this->uniqueValidation($field_name, $validation_data))
							$this->validationErrors[] = $validation_data['message'];
					}
					/* regexp validation */
					if ($validation_data['type']=='regexp') {
						if (!$this->regexpValidation($field_name, $validation_data))
							$this->validationErrors[] = $validation_data['message'];
					}
					/* func validation */
					if ($validation_data['type']=='func') {
						if (!call_user_func($validation_data['value'], $this->$field_name))
							$this->validationErrors[] = $validation_data['message'];
					}
					/* oneof validation */
					if ($validation_data['type']=='oneof') {
						if (!in_array($this->$field_name, $validation_data['value']))
							$this->validationErrors[] = $validation_data['message'];
					}
				}
			}
			
		}
		if (sizeof($this->validationErrors)>0) {
			return false;
        } else {
			return true;
        }
	}

	/**
	 * regexp validation helper
	 * @param $field_name string - db field name 
	 * @param $validation_data array
	 * @return boolean
	 */
	private function regexpValidation($field_name, $validation_data){
		return preg_match($validation_data['value'], $this->$field_name);
	}

	/**
	 * unique validation helper
	 * @param $field_name string - db field name 
	 * @param $validation_data array
	 * @return boolean
	 */
	private function uniqueValidation($field_name, $validation_data) {
		$sql = "select count(*) num from {$this->table_name}
				where ".$this->safeFieldName($field_name)." = :field_value";
		$stmt = $this->db()->prepare($sql);
		$stmt->bindParam(':field_value', $this->$field_name);
		$stmt->execute();
		$result = $stmt->fetch(PDO::FETCH_OBJ);
		if ($result->num == 0)
			return true;
		else
			return false;
	}

	/**
	 * checks if field name is configured - safe
	 * @param $field_name string - db field name 
	 * @return $field_name string
	 */
	private function safeFieldName($field_name) {
		$struct = $this->getTableStructure();
		if (array_key_exists($field_name, $struct))
			return $field_name;
	}

	/**
	 * get user table structure, helper
	 * @return array
	 */
	private function getTableStructure() {
		return array_merge($this->struct_required, $this->struct_optional);
	}
	
	/**
	 *	Checks if defined table exists
	 *	@return boolean
	 */
	private function checkIfTableExists() {
		$sql = "select count(*) as num from information_schema.tables 
				where table_schema = :db_name
				and table_name = :table_name";
		$stmt = $this->db()->prepare($sql);
		$stmt->bindParam(':table_name', $this->table_name);
		$stmt->bindParam(':db_name', $this->database['db_name']);
		$stmt->execute();
		$result = $stmt->fetch(PDO::FETCH_OBJ);
		if ($result->num>0){
			return true;
		} else {
			return false;
		}
	}

	/**
	 *	Generates sql statement to create table based on definitions in 
	 *	self::$struct and self::$table_indexes
	 *	@return boolean
	 */
	public function suggestCreateTable()
	{
		$ai_set = false;
		$sql = "create table if not exists {$this->table_name} (\n";
		$fields = array();
		/* format fields for create table statement */
		$struct = $this->getTableStructure();
		foreach($struct as $field_name=>$def) {
			$null			= '';
			$auto_increment	= '';
			$default		= '';
			if (!$def['null'])
				$null = 'not null';
			if (isset($def['auto_increment']) && $def['auto_increment']==true) {
				$auto_increment = 'auto_increment';
				$ai_set = true;
			}
			if (isset($def['default'])) {
				$default = "default '".$def['default']."'";
			}
			
			$fields[] = trim("{$field_name} {$this->getMysqlType($def['type'], $def['length'])} $null {$default} {$auto_increment}");
		}

		/* format indexes for create table */
		foreach($this->table_indexes as $field_name=>$index_type)
		{
			$fields[] = "{$index_type} ({$field_name})";
		}
		$sql .= implode(",\n", $fields);
		$ai_start = "";
		if ($ai_set)
			$ai_start = "auto_increment=1";
		$sql .= "\n) engine={$this->db_engine} {$ai_start};";

		die("<pre><strong>err:</strong> you don't have mysql table \"{$this->table_name}\" created. May I suggest you use following statement to create it:\n\n".$sql."</pre>");
		return true;
	}

	/**
	 * get real type of mysql field
	 * @param $_type string
	 * @param $length intreger
	 * @return array
	 */
	private function getMysqlType($_type, $length=null)
	{
		if ($_type=='integer')
			return "int({$length})";
		if ($_type=='string') {
			if ($length<256)
				return "varchar({$length})";
			else
				return "text";
		}
        if ($_type=='longtext') {
            return 'longtext';
        }
		if ($_type=='date')
			return "date";
		if ($_type=='datetime')
			return "datetime";
	}
	
	 /* 
	 * Returns DB instance or create initial connection 
	 * @param 
	 * @return mixed $objInstance|boolean; 
	 */ 
	private function db() {
		if(!$this->objInstance){
			try {
				$this->objInstance = new PDO("mysql:host={$this->database['hostname']};dbname={$this->database['db_name']}", $this->database['username'], $this->database['password']); 
				$this->objInstance->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING); 
			} catch(PDOException $e) {
				trigger_error($e->getMessage(), E_USER_ERROR);
				return false;
			} 
		} 
		return $this->objInstance; 
	}

	private function translateGlobals() {
		global $_SESSION;
		$this->session =& $_SESSION;
	}
    
}


/********** Validate Email, EXAMPLE *************/
function validEmail($email) {
	$isValid = true;
	$atIndex = strrpos($email, "@");
	if (is_bool($atIndex) && !$atIndex) {
		  $isValid = false;
	} else {
		  $domain = substr($email, $atIndex+1);
		  $local = substr($email, 0, $atIndex);
		  $localLen = strlen($local);
		  $domainLen = strlen($domain);
		  if ($localLen < 1 || $localLen > 64) {
			 // local part length exceeded
			 $isValid = false;
		  }
		  else if ($domainLen < 1 || $domainLen > 255) {
			 // domain part length exceeded
			 $isValid = false;
		  }
		  else if ($local[0] == '.' || $local[$localLen-1] == '.') {
			 // local part starts or ends with '.'
			 $isValid = false;
		  }
		  else if (preg_match('/\\.\\./', $local))  {
			 // local part has two consecutive dots
			 $isValid = false;
		  }
		  else if (!preg_match('/^[A-Za-z0-9\\-\\.]+$/', $domain)) {
			 // character not valid in domain part
			 $isValid = false;
		  }
		  else if (preg_match('/\\.\\./', $domain)) {
			 // domain part has two consecutive dots
			 $isValid = false;
		  }
		  else if(!preg_match('/^(\\\\.|[A-Za-z0-9!#%&`_=\\/$\'*+?^{}|~.-])+$/', str_replace("\\\\","",$local))) {
			 // character not valid in local part unless 
			 // local part is quoted
			 if (!preg_match('/^"(\\\\"|[^"])+"$/', str_replace("\\\\","",$local))) {
				$isValid = false;
			 }
		  }
		  if ($isValid && !(checkdnsrr($domain,"MX") || checkdnsrr($domain,"A"))){
			 // domain not found in DNS
			 $isValid = false;
		  }
	}
	return $isValid;
}
?>