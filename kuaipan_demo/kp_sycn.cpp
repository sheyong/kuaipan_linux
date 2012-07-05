//g++ -o kp_sycn kp_sycn.cpp ./kp_api.o ./kp_oauth.o -lsqlite3 -loauth -ljson -lcurl
#include "kp_sycn.h"
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <iostream>
#include <sqlite3.h>
using namespace std;

const string kp_sycn::m_sycnPath("/home/sheyong/kuaipantest");
kp_api kp_sycn::kpapi;

static string dbpath("kuaipan_sdk/data.db");
kp_sycn::kp_sycn()
{
	sqlite3 *db;
	int iRetDb = sqlite3_open(dbpath.c_str(),&db);
	
	string szHasFile = "select name from Sqlite_master where type='table' and name='file'";
	char  *zErrMsg = 0;
	char  **resultp;
   	int  nrow;
    int  ncolumn;
	sqlite3_get_table(db,szHasFile.c_str(),&resultp, &nrow, &ncolumn, &zErrMsg);
	//cout<<nrow<<endl;
	int iHasFile = nrow;//table not exist when row is 0 
	
	if(iHasFile==0)
	{
		cout<<"no exist!\n 第一次同步"<<endl;	
		//id,filename,realpath,filestate,local_mtime,path,check_exist,server_mtime
		string szCreateDb("create table file(id integer primary key autoincrement,filename varchar(100),realpath varchar(200),filestate varchar(20),local_mtime varchar(50),path varchar(100),size varchar(100),server_mtime varchar(50),checkexist varchar(50))");
		int ret = sqlite3_exec(db,szCreateDb.c_str(),NULL,NULL,&zErrMsg);
		//cout<<ret<<endl;  0为成功，1为不成功
    }
	sqlite3_close(db);
}

kp_sycn::~kp_sycn()
{
}
/******************************************本地文件信息入库******************************************************/

static string getFileSize(string filename)
{
	struct stat buf;
	if(stat(filename.c_str(),&buf)>=0)
	{		
		unsigned long size = buf.st_size;
		//cout<<size<<endl;
		char tmp[10];//!!!不能写char*，因为char*的内容是在常量区，指针在栈上，并且编译器不知道字符串到底有多长，可能会把后面的一块读取出来。
		sprintf(tmp,"%lu",size);
		//cout<<"point1:"<<&tmp<<endl;
		return tmp;
	}
	return "";
}
//get file modify time
static string getFileMtime(string filename)
{
	struct stat buf;
	struct tm *tmp;
	time_t t;
	char str_time[100];
	if(stat(filename.c_str(),&buf)>=0)
	{
		//cout<<buf.st_ctime<<endl;
		t = buf.st_mtime;
		tmp = localtime(&t);
		strftime(str_time,sizeof(str_time),"%Y-%m-%d %H:%M:%S",tmp);
		//cout<<str_time<<endl;
		return str_time;
	}
	return "";
}

static string getNowTime()
{
	struct tm *tmp;
	time_t t;
	char str_time[100];
	time(&t);
	tmp = localtime(&t);
	strftime(str_time,sizeof(str_time),"%Y-%m-%d %H:%M:%S",tmp);
	return str_time;
}


static void localFile2db(string filename,string realpath,string path,string local_mtime,string size,string checkexist)
{
	sqlite3 *db;
	char  *zErrMsg = 0;
	char  **resultp;
   	int  nrow;
    int  ncolumn;
	sqlite3_open(dbpath.c_str(),&db);
	string sql("select local_mtime from file where realpath='"+realpath+"'");
	sqlite3_get_table(db,sql.c_str(),&resultp, &nrow, &ncolumn, &zErrMsg);
	if(nrow==0)
	{
		//cout<<path<<endl;
		//该文件为新增的，需要入库，并设filestate为new
		string sql("insert into file(filename,realpath,path,local_mtime,size,filestate)values('"+filename+"','"+realpath+"','"+path+"','"+local_mtime+"','"+size+"','new')");
		//cout<<sql<<endl;
		sqlite3_exec(db,sql.c_str(),NULL,NULL,&zErrMsg);
	}
	else if(resultp[1]!=local_mtime)
	{
		//cout<<resultp[1]<<endl;
		//判断local_mtime是否一致，如不一致则该文件为修改，设filestate为modify；一致则不动。
		string sql("update file set local_mtime='"+local_mtime+"',filestate='modify' where realpath='"+realpath+"'");
		//cout<<sql<<endl;
		sqlite3_exec(db,sql.c_str(),NULL,NULL,&zErrMsg);
	}
	//每一次遍历都给本次遍历存在的文件更新这个checkexist
	string sql_check("update file set checkexist='"+checkexist+"' where realpath='"+realpath+"'");
	sqlite3_exec(db,sql_check.c_str(),NULL,NULL,&zErrMsg);
	sqlite3_close(db);
}

static void local_traversal(string path,string nowTime)//nowTime 是为了找出上次遍历有而这次没有的文件
{
	DIR* pDir;
	pDir = opendir(path.c_str());
	struct dirent* ent = NULL;
	while(NULL!=(ent=readdir(pDir)))
	{
		if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
			continue;
		string realpath = path +"/"+ ent->d_name;	
		if(ent->d_type==8)//8为文件
		{//取得filename,realpath,path,local_mtime,size,checkexist（nowTime）					
			//cout<<"file:"<<realpath<<endl;
			string noNamePath = realpath.substr(kp_sycn::m_sycnPath.length(),realpath.length()-kp_sycn::m_sycnPath.length()-strlen(ent->d_name));			
			string size(getFileSize(realpath));//有问题！！！！！！！已解决！！！！					
			string mtime(getFileMtime(realpath));
			//cout<<size<<endl;
			//cout<<mtime<<endl;
			localFile2db(ent->d_name,realpath,noNamePath,mtime,size,nowTime);
		}
		else if(ent->d_type==4)//4为文件夹
		{
			local_traversal(realpath,nowTime);
			realpath+='/';
		}
	}
}
void kp_sycn::local_file(string path)
{
	sqlite3 *db;
	char  *zErrMsg = 0;
	sqlite3_open(dbpath.c_str(),&db);

	string nowTime = getNowTime();
	local_traversal(path,nowTime);
	string sql("update file set filestate='delete' where id in(select id from file where checkexist!='"+nowTime+"' and filestate!='removed')");
	//cout<<sql<<endl;
	sqlite3_exec(db,sql.c_str(),NULL,NULL,&zErrMsg);	
	sqlite3_close(db);
}

/******************************************服务器文件信息入库******************************************************/
static void serverFile2db(string filename,string realpath,string path,string server_mtime,string size,bool isDelete)
{
	sqlite3 *db;
	char  *zErrMsg = 0;
	char  **resultp;
   	int  nrow;
    int  ncolumn;
	sqlite3_open(dbpath.c_str(),&db);
	if(isDelete)
	{
		string sql("update file set filestate='delete' where realpath='"+realpath+"'");
		sqlite3_exec(db,sql.c_str(),NULL,NULL,&zErrMsg);
	}
	else
	{
		string sql("select server_mtime from file where realpath='"+realpath+"'");
		sqlite3_get_table(db,sql.c_str(),&resultp, &nrow, &ncolumn, &zErrMsg);
		//cout<<sizeof(resultp)/sizeof(char*)<<endl;
		int iResult = sizeof(resultp)/sizeof(char*);
		if(nrow==0)
		{
			//该文件为服务器新上传的，需要入库，并设filestate为download
			string sql("insert into file(filename,realpath,path,local_mtime,server_mtime,size,filestate)values('"+filename+"','"+realpath+"','"+path+"','"+server_mtime+"','"+server_mtime+"','"+size+"','download')");
			//cout<<sql<<endl;
			sqlite3_exec(db,sql.c_str(),NULL,NULL,&zErrMsg);
		}
		else if(iResult>1 && resultp[1]!=server_mtime)
		{
			//cout<<resultp[1]<<endl;
			//判断local_mtime是否一致，如不一致则该文件为修改，设filestate为modify；一致则不动。
			string sql("update file set server_mtime='"+server_mtime+"',filestate='download' where realpath='"+realpath+"'");
			//cout<<sql<<endl;
			sqlite3_exec(db,sql.c_str(),NULL,NULL,&zErrMsg);
		}
	}

	sqlite3_close(db);	
}

static void server_traversal(string path)
{
	string meta = kp_sycn::kpapi.GetMetadata(path);
	Json::Reader reader;
	Json::Value json_object;
	Json::Value json_sub;
	reader.parse(meta,json_object);
	json_sub = json_object["files"];
	for(int i=0;i<json_sub.size();i++)
	{
		//cout<<i<<":"<<json_sub[i]<<endl;
		//cout<<json_sub[i]["type"].asString()<<endl;
		bool isDelete = json_sub[i]["is_deleted"].asBool();//跳过服务器上已被删除的文件。
		if(json_sub[i]["type"].asString()=="file")
		{	
			string filename(json_sub[i]["name"].asString());				
			string realpath(kp_sycn::m_sycnPath+path+filename);
			string modify_time(json_sub[i]["modify_time"].asString());
			int tmp = json_sub[i]["size"].asInt();
			char size_tmp[10];
			sprintf(size_tmp,"%i",tmp);
			string size(size_tmp);
			//cout<<realpath<<endl;
			//cout<<modify_time<<endl;
			//cout<<size<<endl;
			serverFile2db(filename,realpath,path,modify_time,size,isDelete);
			//找到服务器上已删除的文件，并查看本地是否已经删除，如果本地未删除，则删除；否则不动！
		}
		else
		{	
			string path_str =path+json_sub[i]["name"].asString()+"/";
			//cout<<"folder:"<<path_str<<endl;
			server_traversal(path_str);
		}
	}
}


/*服务器文件入库程序基本完成，下次先测试一下，（检测删除文件的功能还没加，下次分析一下逻辑然后做。）
做完这个，开始做同步的程序。2012-06-11  */
void kp_sycn::server_file()
{
	//cout<<kpapi.m_root<<endl;
	server_traversal("/");
}

/**************************************双向同步**********************************************/


static string getServer_mtime(string filename)
{
	string meta(kp_sycn::kpapi.GetMetadata(filename));
	Json::Reader reader;
	Json::Value json_object;
	reader.parse(meta,json_object);
	//cout<<json_object["modify_time"].asString()<<endl;
	return json_object["modify_time"].asString();
}
//查询数据库，找出需要上传，下载，删除的文件，进行相应操作，并且完成操作后要更行filestate为ok
void kp_sycn::sycn()
{
	sqlite3 *db;
	char  *zErrMsg = 0;
	char  **resultp;
   	int  nrow;
    int  ncolumn;
	sqlite3_open(dbpath.c_str(),&db);
	/*************上传本地新添加和修改过的文件******************/
	string sql_uoload("select filename,realpath,path from file where filestate='new' or filestate='modify'");
	sqlite3_get_table(db,sql_uoload.c_str(),&resultp, &nrow, &ncolumn, &zErrMsg);
	if(nrow!=0)
	{
		cout<<"发现本地新增和已修改文件："<<endl;
	   for(int i=1;i<=nrow;i++)//第0行为查询的选项，因此从第一行开始
		{
			string filename(resultp[i*ncolumn+0]);
			string realpath(resultp[i*ncolumn+1]);
			string path(resultp[i*ncolumn+2]);
			string upload_path(path+filename);
			cout<<"filename:"<<filename<<"\t本地位置："<<realpath<<"\t服务器位置："<<path+filename<<endl;
			kpapi.CreateFolder(path);
			bool bUpload = kpapi.Upload(upload_path,realpath);
			if(bUpload)
			{
				cout<<realpath<<"\t上传完成！"<<endl;
				//上传完文件后，把服务器上该文件的modify_time取出来更新到数据库中的server_mtime,避免下次遍历时找不到文件的server_mtime.
				string server_mtime(getServer_mtime(upload_path));
				string sql("update file set filestate='ok',server_mtime='"+server_mtime+"' where realpath='"+realpath+"'");	 
				sqlite3_exec(db,sql.c_str(),NULL,NULL,&zErrMsg);  
			} 
		}
	}
	/*****************************下载服务器新添加的文件************************************/
	string sql_download("select filename,realpath,path from file where filestate='download'");
	sqlite3_get_table(db,sql_download.c_str(),&resultp,&nrow,&ncolumn,&zErrMsg);
	if(nrow!=0)
	{
		cout<<"发现服务器新增文件："<<endl;
		for(int i=1;i<=nrow;i++)
		{
			string filename(resultp[i*ncolumn+0]);
			string realpath(resultp[i*ncolumn+1]);
			string path(resultp[i*ncolumn+2]);
			string download_path(path+filename);
			cout<<"filename:"<<filename<<"\t本地位置："<<realpath<<"\t服务器位置："<<path+filename<<endl;
			bool bDownload = kpapi.Download(download_path,realpath);
			if(bDownload)
			{
				cout<<realpath<<"\t下载完成"<<endl;
				string local_mtime(getFileMtime(realpath));
				string sql("update file set filestate='ok',local_mtime='"+local_mtime+"' where realpath='"+realpath+"'");	 
				sqlite3_exec(db,sql.c_str(),NULL,NULL,&zErrMsg);  
			}
		}
	}
/************************************在服务器上删除本地已经删除的文件*********************************************/
//下次做。。。
	string sql_delete("select filename,realpath,path from file where filestate='delete'");
	sqlite3_get_table(db,sql_delete.c_str(),&resultp,&nrow,&ncolumn,&zErrMsg);
	if(nrow!=0)
	{
		cout<<"发现被删除文件："<<endl;
		for(int i=1;i<=nrow;i++)
		{
			string filename(resultp[i*ncolumn+0]);
			string realpath(resultp[i*ncolumn+1]);
			string path(resultp[i*ncolumn+2]);
			string delete_path(path+filename);
			cout<<"filename:"<<filename<<"\t本地位置："<<realpath<<"\t服务器位置："<<path+filename<<endl;
			kpapi.Delete(delete_path);
			cout<<filename<<"删除成功\n";
			string sql("update file set filestate='removed' where realpath='"+realpath+"'");	 
			sqlite3_exec(db,sql.c_str(),NULL,NULL,&zErrMsg);  
		}
	}
	sqlite3_close(db);
}
int main()
{
	kp_sycn sycn;
	sycn.local_file("/home/sheyong/kuaipantest");
	sycn.server_file();
	sycn.sycn();
	cout<<"已完成同步！\n";
}

