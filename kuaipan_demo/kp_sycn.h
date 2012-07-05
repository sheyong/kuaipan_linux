#ifndef KP_SYCN_H
#define KP_SYCN_H

#include "../kuaipan_sdk/kp_api.h"
#include "../kuaipan_sdk/kp_oauth.h"

class kp_sycn
{
 public:
	const static string m_sycnPath;
	static kp_api kpapi;
    kp_sycn();
	~kp_sycn();

    /*为了每个函数内部简洁，根据上次python的经验，把file into db的函数分为三个，traversal中调用getFileInfo，getFileInfo中调用file2db*/

	void local_file(string path);//本地文件信息入库
	
	void server_file();
	
	void sycn();//读取数据库，针对filestate需要同步的文件进行双向同步
};
#endif

//    12|3|/home/sheyong/kuaipantest/1/1/3|730cd928e8a768387713b4d42e68c0eb1be44f24|unchanged|2012-05-12 23:16:50|10|/1/1|2012-05-12 23:17:33
//    id,filename,realpath,filestate,local_mtime,path,check_exist,server_mtime
