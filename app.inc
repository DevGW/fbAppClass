<?php
/**************************************************************
 * fbAppConfig for FB PHP SDK 3.1.1
 * Copyright 2012 RazorWire Solutions, Inc.  
 **************************************************************
 *
 * Facebook App User Management Class
 *
 * REQUIREMENTS:
 *      - PHP5+, with pdo_mysql
 *      - MySQL5+
 *      - Facebook PHP SDK 3.1.1
 *
 * @package fbAppClass
 * @author RWSDev Team (Jason Becht)
 * @version 1.0
 * @copyright 2012
 **************************************************************/
require_once('fbcore/facebook.php');
require_once('fbUser_class.inc');
session_start();

    //facebook application SHAREBOX
    $config['appid' ] = "FB_APP_ID";
    $config['secret'] = "FB_APP_SECRET";
    $config['baseurl'] = "http://www.EXAMPLE.com/your-app";
    $config['appbaseurl'] = "http://apps.facebook.com/YOUR-APP-ON-FACEBOOK";
    

    // Create our Application instance.
    $facebook = new Facebook(array(
      'appId'  => $config['appid'],
      'secret' => $config['secret'],
      'cookie' => true,
    ));

    //Facebook Authentication part
    $user = $facebook->getUser();
    // We may or may not have this data based 
    // on whether the user is logged in.
    // If we have a $user id here, it means we know 
    // the user is logged into
    // Facebook, but we don’t know if the access token is valid. An access
    // token is invalid if the user logged out of Facebook.
    
    $loginUrl = $facebook->getLoginUrl(
            array(
                'scope' => 'email,publish_stream,user_birthday,user_location'
            )
    );

    if ($user) {
      try {
        //get user basic description
        $userInfo = $facebook->api("/$user");
        $fb_access_token = $facebook->getAccessToken();
      } catch (FacebookApiException $e) {
        //you should use error_log($e); instead of printing the info on browser
        error_log('Sharebox: '.$e); // d is a debug function defined at the end of this file
        $user = null;
      }
    }

    if (!$user) {
        echo "<script type='text/javascript'>top.location.href = '$loginUrl';</script>";
        exit;
    }

    // create or update userInfo in users table
    $fbUserClass = new fbUserClass($user);
    $fbUserClass->userid = $user;
    $fbUserClass->first_name = $userInfo['first_name'];
    $fbUserClass->last_name = $userInfo['last_name'];
    $fbUserClass->email = $userInfo['email'];
    $fbUserClass->fb_access_token = $fb_access_token;
    
    // get ids of all friends and add or update in users table
    $getFriends = $facebook->api('/me/friends?access_token='.$fb_access_token.'&fields=id');
    $friendIds = array();
    foreach ($getFriends['data'] as $friend) {
        array_push($friendIds,$friend['id']);
    }
    $fbUserClass->friends = implode(',',$friendIds);
    
    $fbUserClass->validate(); //validate will be called on save, but you can manually call it also
    $fbUserClass->save($user);
    
/* prints validation errors */
    if (isset($_GET['code'])){
        header("Location: " . $config['appbaseurl']);
        exit;
    }

?>