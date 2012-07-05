#include"kp_oauth.h"
#include<sqlite3.h>
#include <termios.h>  //终端设置

//static variables initialize
const string kp_oauth::request_token_url("https://openapi.kuaipan.cn/open/requestToken");
const string kp_oauth::authorise_url("https://www.kuaipan.cn/api.php?ac=open&op=authorise&oauth_token=");
const string kp_oauth::accesstoken_url("https://openapi.kuaipan.cn/open/accessToken");
const string kp_oauth::authorise_post_url("https://www.kuaipan.cn/api.php?ac=open&op=authorisecheck");

string kp_oauth::consumer_key("xc9wsMCl2QmwmvXf");
string kp_oauth::consumer_secret("A7bDiZv7NytLSc2R");
string kp_oauth::username("");
string kp_oauth::password("");
string kp_oauth::oauth_token("");
string kp_oauth::oauth_token_secret("");

//define class function
//constructor 
kp_oauth::kp_oauth()
{
}

//终端不回显输入密码函数
static string getPasswd(void)
{
	struct termios oldt, newt;
	string passwd("");
	tcgetattr( STDIN_FILENO, &oldt ); // 记录旧设置
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt ); // 设置新的
	cin>>passwd;
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt ); // 设置旧的
	//cout<<tmp<<endl;
	cout<<"\n";
	return passwd;
}


void kp_oauth::kp_oauth_init()
{
//下午回来重新修改，不要去检测文件是否存在，而是直接打开数据库，不存在则创建一个
//打开数据库查找那张表，如果表不存在则判断是第一次使用，需要全部的oauth步骤；每一步引导用户输入个人信息！
//如果存在那张表，则查找每一项的值，并去赋给程序的变量，后面程序要用
	sqlite3 *db;
	int iRetDb = sqlite3_open("data.db",&db);
//查询conf表是否存在，存在则提取用户信息，不存在则创建表并进行全套oauth认证，然后把信息插入表
	string szHasConf = "select name from Sqlite_master where type='table' and name='conf'";
	char  *zErrMsg = 0;
	char  **resultp;
   	int  nrow;
    int  ncolumn;
	sqlite3_get_table(db,szHasConf.c_str(),&resultp, &nrow, &ncolumn, &zErrMsg);
	//cout<<nrow<<endl;
	int iHasConf = nrow;//table not exist when row is 0 
	//cout<<"iHasConf"<<iHasConf<<endl;
	
	if(!iHasConf)
	{ 
		cout<<"no exist!\n 第一次使用"<<endl;	
		string szCreateDb("create table conf(name varchar(30),value varchar(100))");
		int tmpRet = sqlite3_exec(db,szCreateDb.c_str(),NULL,NULL,&zErrMsg);		
		string sql = "insert into conf values('consumer_key','"+kp_oauth::consumer_key+"')";
		sqlite3_exec(db,sql.c_str(),NULL,NULL,&zErrMsg);
		sql = "insert into conf values('consumer_secret','"+kp_oauth::consumer_secret+"')";
		sqlite3_exec(db,sql.c_str(),NULL,NULL,&zErrMsg);
		//输入用户名和密码
		cout<<"******************用户授权************************\n请输入用户名：\n";
		string username("");
		cin>>username;
		kp_oauth::username = username;
		sql = "insert into conf values('username','"+kp_oauth::username+"')";
		sqlite3_exec(db,sql.c_str(),NULL,NULL,&zErrMsg);
		cout<<"请输入密码：\n";
		string password("");
		password = getPasswd();
		kp_oauth::password = password;
		sql = "insert into conf values('password','"+kp_oauth::password+"')";
		sqlite3_exec(db,sql.c_str(),NULL,NULL,&zErrMsg);
		kp_oauth_request();	
		kp_oauth_authorise();
		kp_oauth_accessToken();
		sql = "insert into conf values('oauth_token','"+kp_oauth::oauth_token+"')";	
		sqlite3_exec(db,sql.c_str(),NULL,NULL,&zErrMsg);
		sql = "insert into conf values('oauth_token_secret','"+kp_oauth::oauth_token_secret+"')";
		sqlite3_exec(db,sql.c_str(),NULL,NULL,&zErrMsg);
	}
	else
	{	
		//检测表中数据是否是正确的，这一步验证以后做	
		cout<<"exist！（不是第一次使用）"<<endl;	
		string sql = "select value from conf where name='oauth_token'";
		//cout<<sql<<endl;
		sqlite3_get_table(db,sql.c_str(),&resultp, &nrow, &ncolumn, &zErrMsg);
		//cout<<nrow<<"****"<<ncolumn<<endl;
		//cout<<resultp[1]<<endl;
		kp_oauth::oauth_token = resultp[1];		
		sql = "select value from conf where name='oauth_token_secret'";
		sqlite3_get_table(db,sql.c_str(),&resultp, &nrow, &ncolumn, &zErrMsg);
		//cout<<nrow<<"****"<<ncolumn<<endl;
		//cout<<resultp[1]<<endl;
		kp_oauth::oauth_token_secret = resultp[1];
	}
	sqlite3_close(db);

}	 
void kp_oauth::kp_oauth_request()
{
	cout<<"**********************oauth begin*************************\n";
	string req_url = oauth_sign_url2(request_token_url.c_str(), NULL, OA_HMAC, NULL, consumer_key.c_str(),consumer_secret.c_str(), NULL, NULL);
	//cout<<req_url<<endl;
	string reply = oauth_http_get(req_url.c_str(),NULL);//request oauth https url,and return a json string
	//cout<<reply<<endl;
	Json::Reader reader;
	Json::Value json_object;
	reader.parse(reply,json_object);
	//cout<<json_object["oauth_token"]<<endl;
	oauth_token = json_object["oauth_token"].asString();//asString can convert a value to a string
	//cout<<oauth_token<<endl;
	oauth_token_secret = json_object["oauth_token_secret"].asString();
	//cout<<oauth_token_secret<<endl;
}
static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}
void kp_oauth::kp_oauth_authorise()
{
	string postfield = "username="+username+"&userpwd="+password+"&acceptopenapi=checked&app_name=sheyong&oauth_token="+oauth_token+"&sumit=登录并授权";
	const char *postfields = postfield.c_str();
	const char *post_url = authorise_post_url.c_str();
	CURL *curl;
	CURLcode res;
	struct curl_slist *http_header = NULL;
	FILE *fptr = fopen("x.txt","w");

	curl = curl_easy_init();
	curl_easy_setopt(curl,CURLOPT_URL,post_url);
	curl_easy_setopt(curl,CURLOPT_POSTFIELDS,postfields);
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,fptr);
	curl_easy_setopt(curl,CURLOPT_POST,1);
	curl_easy_setopt(curl,CURLOPT_VERBOSE,1);
	curl_easy_setopt(curl,CURLOPT_HEADER,0);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEHEADER, fptr);

	curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,1);
	curl_easy_setopt(curl,CURLOPT_COOKIEFILE,"/home/sheyong/socket_test/curlposttest.cookie");
	res = curl_easy_perform(curl);
	int infocode = 0;
	curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&infocode);
	//cout<<infocode<<endl;
	if(200==infocode)
		cout<<"*******************授权成功*****************************\n";
	else
		cout<<"授权不成功，请检查 用户名和密码受否错误！\n";
	curl_easy_cleanup(curl);
	fclose(fptr);
}
void kp_oauth::kp_oauth_accessToken()
{
	string access_url = oauth_sign_url2(accesstoken_url.c_str(), NULL, OA_HMAC, NULL, consumer_key.c_str(),consumer_secret.c_str(), oauth_token.c_str(), oauth_token_secret.c_str());
	//cout<<access_url<<endl;	
	string access_reply = oauth_http_get(access_url.c_str(),NULL);
	//cout<<access_reply<<endl;
	Json::Reader reader;
	Json::Value json_access;
	reader.parse(access_reply,json_access);
	oauth_token = json_access["oauth_token"].asString();
	oauth_token_secret = json_access["oauth_token_secret"].asString();
	//cout<<oauth_token<<endl;
	//cout<<oauth_token_secret<<endl;
	cout<<"*******************oauth end *****************************\n";
}
/*
int main()
{
	kp_oauth kp_oauth;
	kp_oauth.kp_oauth_init();
	return 0;
}
*/
