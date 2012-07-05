#include "../kuaipan_sdk/kp_oauth.h"
#include "../kuaipan_sdk/kp_api.h"


int main()
{	
	kp_api kp;
	cout<<kp.GetAcountinfo()<<endl;
	cout<<kp.GetMetadata("fuck5.txt")<<endl;
	cout<<kp.CreateFolder("new")<<endl;
	cout<<kp.Delete("fuck.txt")<<endl;
	cout<<kp.GetUploadUrl()<<endl;
	kp.Upload("metadata.txt","metadata.c");
	kp.Download("metadata.txt","metadata.txt");
	kp.move("metadata.txt","/1/metadata.txt");
	kp.rename("metadata.c","meta.c");
}
