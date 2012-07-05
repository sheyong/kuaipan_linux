#ifndef KP_API_H
#define KP_API_H

#include "kp_oauth.h"
#include <string>
class kp_api
{
 public:
	static kp_oauth m_kpoauth;
	const static string m_root;//root:kuaipan,app_folder
	const static string m_accountinfo_url;
	const static string m_metadata_url;
	const static string m_createfolder_url;
	const static string m_delete_url;
	const static string m_uploadlocate_url;
	const static string m_download_url;
	const static string m_move_url;
	//member variables...

	kp_api();
	
	string GetAcountinfo();
	string GetMetadata(string path);
	string CreateFolder(string path);
	string Delete(string path);
	string GetUploadUrl();
	bool Upload(string upload_path,string local_path);
	bool Download(string download_path,string local_path);
	void move(string from_path,string to_path);
	void rename(string path,string new_name);
	//member functions...
// private:
	//void curl_upload(string upload_path,string local_path);
};
#endif
