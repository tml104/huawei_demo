#include "StdAfx.h"
#include "ZSRReadInfoInJsonfile.h"


ZSRReadInfoInJsonfile::ZSRReadInfoInJsonfile(void)
{
}


ZSRReadInfoInJsonfile::~ZSRReadInfoInJsonfile(void)
{
}

int ZSRReadInfoInJsonfile::ReadJsonFromFile(const char* filename)  
{  
	//if(filename==NULL)
	//	return 0;

 //   Json::Reader reader; // ����json  
 //   Json::Value root; // Json::Value�����������͡���int, string, object, array         

 //   std::ifstream isStream;  
 //   isStream.open (filename, std::ios::binary );    
 //   if (reader.parse(isStream, root, FALSE))  
 //   {  
 //       std::string code;  
 //       if (!root["files"].isNull())  // ���ʽڵ㣬Access an object value by name, create a null member if it does not exist.  
 //           code = root["uploadid"].asString();  
 //       
 //       code = root.get("uploadid", "null").asString(); // ���ʽڵ㣬Return the member named key if it exist, defaultValue otherwise.    

 //       int file_size = root["files"].size();  // �õ�"files"���������  
 //       for(int i = 0; i < file_size; ++i)  // ��������  
 //       {  
 //           Json::Value val_image = root["files"][i]["images"];  
 //           int image_size = val_image.size();  
 //           for(int j = 0; j < image_size; ++j)  
 //           {  
 //               std::string type = val_image[j]["type"].asString();  
 //               std::string url  = val_image[j]["url"].asString(); 
 //               printf("type : %s, url : %s \n", type.c_str(), url.c_str());
 //           }  
 //       }  
 //   }  
 //   isStream.close();  

    return 0;  
}

