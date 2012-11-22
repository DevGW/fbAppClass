/**************************************************************
 * fbAppClass - Copyright 2012 RazorWire Solutions, Inc.  
 **************************************************************
 *
 * Facebook App User Management Class
 * For use with Facebook PHP SDK 3.1.1
 *
 *  REQUIREMENTS:
 *      - PHP5+, with pdo_mysql
 *      - MySQL5+
 *      - Facebook PHP SDK 3.1.1 (included)
 *
 * @package fbAppClass
 * @author RWSDev Team (Jason Becht)
 * @version 1.0
 * @copyright 2012
 **************************************************************/


Using fbAppClass is very simple. Here are few examples:

Just configure app.inc and your database settings in fbUser_class.inc then include app.inc in your main index.php file for your app.
You will have to create a mysql database and users table.  The users table
can be created by running the below sql after creating the database:

delimiter $$

CREATE TABLE `users` (
  `userid` varchar(64) NOT NULL,
  `first_name` varchar(64) NOT NULL,
  `last_name` varchar(64) NOT NULL,
  `email` varchar(128) NOT NULL,
  `fb_access_token` varchar(255) NOT NULL,
  `registration_date` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `friends` longtext,
  PRIMARY KEY (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8$$

After you have created the database, users table, configured and included app.inc
when ever a user connects to you app that has authorized it, this class will update
any changed information in the users table for all fields except userid.

/*** IMPORTANT NOTES ***/
The variable $user is used by app.inc and stores the facebook userid of the connected user.
The array $userInfo stores the array returned by facebook for the connected user.

Example Calls to the fbUserClass

Register new user
$fbUserClass = new fbUserClass();
$fbUserClass->userid = 'facebook userid';
$fbUserClass->first_name = 'john';
$fbUserClass->last_name = 'doe';
$fbUserClass->email = 'john@example.com';
$fbUserClass->registration_date = date('Y-m-d H:i:s', time());
$fbUserClass->friends = comma separated list of friend id’s
$fbUserClass->save();

/* prints validation errors */
print_r($fbUserClass->validationErrors);

Update user data
Update user is same as insert/register, just pass userid of user

$fbUserClass = new fbUserClass(facebook userid);
/* all fields are optional, just set what you want to update */
$fbUserClass->userid = 'facebook userid';
$fbUserClass->first_name = 'john';
$fbUserClass->last_name = 'doe';
$fbUserClass->email = 'john@example.com';
$fbUserClass->registration_date = date('Y-m-d H:i:s', time());
$fbUserClass->friends = comma separated list of friend id’s
$fbUserClass->save();

Load user
$fbUserClass = new fbUserClass();

//load by -- anything
$result = $fbUserClass->loadby('first_name', 'john');

//or
$result = $fbUserClass->loadby('userid', 'facebook userid');
print_r($result);

//or by id
$fbUserClass = new fbUserClass(facebook userid);

//or
$fbUserClass->load(facebook userid);

Delete user
$fbUserClass = new fbUserClass(facebook userid);
$fbUserClass->delete();


Search
$fbUserClass = new fbUserClass();
$results = $fbUserClass->search('jo*hn'); 

// * acts as a wild card, wildcards are automatically added to the begining and end of string

$fbUserClass->search('jo*n', array('first_name'));
//if you wish to search just by first name, other field names can be added

/* prints results */
print_r($results);

