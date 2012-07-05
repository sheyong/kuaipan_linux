#ifndef KP_OAUTH_H
#define KP_OAUTH_H

#include<iostream>
#include<oauth.h>
#include<json/json.h>
#include<string>
#include<curl/curl.h>
using namespace std;

class kp_oauth
{
 public:

	const static string request_token_url;
	const static string authorise_url;
	const static string accesstoken_url;
	const static string authorise_post_url;	

	static string consumer_key;
	static string consumer_secret;
	static string username;
	static string password;
	static string oauth_token;
	static string oauth_token_secret;

	kp_oauth();

	void kp_oauth_init();
 private:
	void kp_oauth_request();
	void kp_oauth_authorise();
	void kp_oauth_accessToken();	
};
#endif
