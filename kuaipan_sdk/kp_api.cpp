//g++ -o kp_api kp_api.cpp ./kp_oauth.o -loauth -ljson -lsqlite3

#include "kp_api.h"
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//member vaiables define:
kp_oauth kp_api::m_kpoauth;
const string kp_api::m_root("app_folder");
const string kp_api::m_accountinfo_url("http://openapi.kuaipan.cn/1/account_info");
const string kp_api::m_metadata_url("http://openapi.kuaipan.cn/1/metadata");
const string kp_api::m_createfolder_url("http://openapi.kuaipan.cn/1/fileops/create_folder");
const string kp_api::m_delete_url("http://openapi.kuaipan.cn/1/fileops/delete");
const string kp_api::m_uploadlocate_url("http://api-content.dfs.kuaipan.cn/1/fileops/upload_locate");
const string kp_api::m_download_url("http://api-content.dfs.kuaipan.cn/1/fileops/download_file");
const string kp_api::m_move_url("http://openapi.kuaipan.cn/1/fileops/move");

//member function define:
kp_api::kp_api()
{
	m_kpoauth.kp_oauth_init();  //oauth验证
}
/************************************Get user account information****************************************/
string kp_api::GetAcountinfo()
{
	//cout<<kp_api::m_kpoauth.oauth_token_secret<<endl;
	string req_url;
	req_url = oauth_sign_url2(kp_api::m_accountinfo_url.c_str(), NULL, OA_HMAC, NULL, m_kpoauth.consumer_key.c_str(),m_kpoauth.consumer_secret.c_str(),
								m_kpoauth.oauth_token.c_str(),m_kpoauth.oauth_token_secret.c_str());
	string reply = oauth_http_get(req_url.c_str(),NULL);
	return reply;
}
/************************************Get file(folder) information*****************************************/
string kp_api::GetMetadata(string path)
{		
    path = oauth_url_escape(path.c_str());
	string url = kp_api::m_metadata_url+"/"+kp_api::m_root+path;
	string req_url;
	req_url = oauth_sign_url2(url.c_str(), NULL, OA_HMAC, NULL, m_kpoauth.consumer_key.c_str(),m_kpoauth.consumer_secret.c_str(), m_kpoauth.oauth_token.c_str(),m_kpoauth.oauth_token_secret.c_str());
	string reply = oauth_http_get(req_url.c_str(),NULL);
	return reply;
}
/************************************create Folder*****************************************/
string kp_api::CreateFolder(string path)
{
	string path_encode = oauth_url_escape(path.c_str());	
	string url = kp_api::m_createfolder_url+"?root="+kp_api::m_root+"?&path="+path_encode;
    string req_url =
        oauth_sign_url2(url.c_str(), NULL, OA_HMAC, NULL, m_kpoauth.consumer_key.c_str(),m_kpoauth.consumer_secret.c_str(), m_kpoauth.oauth_token.c_str	(),m_kpoauth.oauth_token_secret.c_str());
	//cout<<req_url<<endl;
	string reply = oauth_http_get(req_url.c_str(),NULL);
	return reply;
}
/************************************delete function:*****************************************/
string kp_api::Delete(string path)
{
	string path_encode = oauth_url_escape(path.c_str());	
	string url = kp_api::m_delete_url+"?root="+kp_api::m_root+"?&path="+path_encode;
    string req_url =
        oauth_sign_url2(url.c_str(), NULL, OA_HMAC, NULL, m_kpoauth.consumer_key.c_str(),m_kpoauth.consumer_secret.c_str(), m_kpoauth.oauth_token.c_str	(),m_kpoauth.oauth_token_secret.c_str());
	//cout<<req_url<<endl;
	string reply = oauth_http_get(req_url.c_str(),NULL);
	return reply;
}
/************************************upload*****************************************
kp_api::GetUploadUrl:取得一个上传文件的链接函数，返回string 型URL；被upload调用
kp_api::Upload：上传文件函数
*/
string kp_api::GetUploadUrl()
{
	//获取上传地址加不加oauth参数都可以，结果都一样的！
	//string req_url = oauth_sign_url2(kp_api::m_uploadlocate_url.c_str(),NULL,OA_HMAC,NULL,m_kpoauth.consumer_key.c_str(),m_kpoauth.consumer_secret.c_str(),m_kpoauth.oauth_token.c_str(),m_kpoauth.oauth_token_secret.c_str());
	//string reply = oauth_http_get(req_url.c_str(),NULL);
	string reply = oauth_http_get(kp_api::m_uploadlocate_url.c_str(),NULL);
	Json::Reader reader;
	Json::Value json_object;
	reader.parse(reply,json_object);
	string upload_url = json_object["url"].asString();
	//cout<<upload_url<<endl;
	return upload_url;
}
bool kp_api::Upload(string upload_path,string local_path)
{
	char *postarg = NULL;
	string path = oauth_url_escape(upload_path.c_str());
	string upload_url = kp_api::GetUploadUrl()+"1/fileops/upload_file?overwrite=true?&root="+kp_api::m_root+"?&path="+path;
	string req_url = oauth_sign_url2(upload_url.c_str(),&postarg,OA_HMAC,NULL,m_kpoauth.consumer_key.c_str(),m_kpoauth.consumer_secret.c_str(),m_kpoauth.oauth_token.c_str(),m_kpoauth.oauth_token_secret.c_str());
/*Parameters:
url 	The request URL to be signed. append all GET or POST query-parameters separated by either '?' or '&' to this parameter.
postargs 	This parameter points to an area where the return value is stored. If 'postargs' is NULL, no value is stored.
method 	specify the signature method to use. It is of type OAuthMethod and most likely OA_HMAC.
http_method 	The HTTP request method to use (ie "GET", "PUT",..) If NULL is given as 'http_method' this defaults to "GET" when 'postargs' is also NULL and when postargs is not NULL "POST" is used.
*/
//回来在这里写liburl上传文件的程序...
	//cout<<req_url<<endl;
	req_url = req_url+"?"+postarg;
	//cout<<req_url<<endl;
//upload file using the http post way of curl
	CURL *curl;
	CURLcode res;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	struct curl_slist *headerlist=NULL;
	static const char buf[]="Expect:";
	
	curl_global_init(CURL_GLOBAL_ALL);
	
	curl_formadd(&formpost,
				&lastptr,
				CURLFORM_COPYNAME,"file",
				CURLFORM_FILE,local_path.c_str(),
				CURLFORM_END);

	curl = curl_easy_init();
	headerlist = curl_slist_append(headerlist,buf);
	curl_easy_setopt(curl,CURLOPT_URL,req_url.c_str());
	curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headerlist);	
	curl_easy_setopt(curl,CURLOPT_HTTPPOST,formpost);
	//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	//curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	res = curl_easy_perform(curl);
	int infocode;
	curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&infocode);
	
	curl_easy_cleanup(curl);
	curl_formfree(formpost);	
	curl_slist_free_all(headerlist);
	return infocode==200 ? true:false;
}
/************************************download******************************************
write_data:curl 中把数据往文件写的回调函数；
WriteMemoryCallback：curl 中把数据往内存写的回调函数
kp_api::Download：下载函数
*/
//把返回的文件内容写到文件
static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}
//下载需要跳转时，把返回的http header 写入内存，存在字符串中等待处理
struct MemoryStruct {
  char *memory;
  size_t size;
}; 
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) {
	/* out of memory! */ 
	printf("not enough memory (realloc returned NULL)\n");
	exit(EXIT_FAILURE);
	}
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	return realsize;
}

bool kp_api::Download(string download_path,string local_path)
{
	string path_encode = oauth_url_escape(download_path.c_str());	
	string url = kp_api::m_download_url+"?root="+kp_api::m_root+"?&path="+path_encode;
    string req_url =
        oauth_sign_url2(url.c_str(), NULL, OA_HMAC, NULL, m_kpoauth.consumer_key.c_str(),m_kpoauth.consumer_secret.c_str(), m_kpoauth.oauth_token.c_str	(),m_kpoauth.oauth_token_secret.c_str());
	//cout<<req_url<<endl;
//req_url is download url , tomorrow try to solute the download problem!
    CURL *curl;
    FILE *fp;
    CURLcode res;
    //char outfilename[FILENAME_MAX] = "x.txt";
	struct MemoryStruct chunk;
	chunk.memory = (char *)malloc(1);  /* will be grown as needed by the realloc above */ 
	chunk.size = 0;    /* no data at this point */ 
	string cookie("");
	fp = fopen(local_path.c_str(),"wb");
	bool isJump = true;
	int infocode = 0;
	while(isJump)
	{
        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL,req_url.c_str());
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);		
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *)&chunk);
		curl_easy_setopt(curl,CURLOPT_COOKIE,cookie.c_str());

        res = curl_easy_perform(curl);
		//cout<<"***"<<chunk.memory<<endl;
		curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&infocode);//get the return http info code,if it is 302,it means that the url will jump another url.
		//cout<<infocode<<endl;
		if(infocode==302)
			{//if it is 302,we need get the jump url and cookie!
				string str = chunk.memory;
				int k = str.find("Set-Cookie: ",0);
				int i = str.find("Location: ",0);
				int j = str.find("Content-Length:",0);
				k+=12;
				int len1 = i-k-2;
				cookie = str.substr(k,len1);
				i+=10;
				//cout<<"***"<<cookie<<endl;
				int len2 = j-i-2;
				req_url = str.substr(i,len2);
				//cout<<"***"<<req_url<<endl;
				continue;
			}
		else if(infocode==200)
			{
				cout<<local_path+"\t\tdownload success!"<<endl;//2012-06-14
			}
		isJump = false;
			
	  }
	  curl_easy_cleanup(curl);
	  fclose(fp);
	  return infocode==200 ? true:false;
}
/************************************move******************************************/
void kp_api::move(string from_path,string to_path)
{
	char* from_path_encode = oauth_url_escape(from_path.c_str());
	char* to_path_encode = oauth_url_escape(to_path.c_str());
	string url = kp_api::m_move_url+"?root="+kp_api::m_root+"?&from_path="+from_path_encode+"?&to_path="+to_path_encode;
	string req_url = oauth_sign_url2(url.c_str(),NULL,OA_HMAC, NULL, m_kpoauth.consumer_key.c_str(),m_kpoauth.consumer_secret.c_str(), m_kpoauth.oauth_token.c_str	(),m_kpoauth.oauth_token_secret.c_str());
	//cout<<req_url<<endl;
	string reply = oauth_http_get(req_url.c_str(),NULL);
	cout<<reply<<endl;
}

/************************************rename*****************************************
调用move，只要from_path和to_path只有文件名不一样时，就可以将from_path中的文件名改为to_path中的文件名
*/
void kp_api::rename(string path,string new_name)
{
	string first_path(path);
	int i = path.rfind("/");
	if(i==-1)
		path = new_name;
	else
	{
		int len = path.length() - i;
		//cout<<i<<"**"<<len<<endl;
		path.replace(i+1,len,new_name);		
	}
	//cout<<path<<endl;
	kp_api::move(first_path,path);	
}
/*
int main()
{	
	kp_api kp;
	//cout<<kp.GetAcountinfo()<<endl;
	cout<<kp.GetMetadata("fuck5.txt")<<endl;
	//cout<<kp.CreateFolder("new")<<endl;
	//cout<<kp.Delete("fuck.txt")<<endl;
	//cout<<kp.GetUploadUrl()<<endl;
	//kp.Upload("metadata.txt","metadata.c");
	//kp.Download("metadata.txt","metadata.txt");
	//kp.move("metadata.txt","/1/metadata.txt");
	//kp.rename("metadata.c","meta.c");



	//cout<<"***"<<'\n';
	//cout<<"*&&&"<<"\n";
	//endl(cout);

}*/
