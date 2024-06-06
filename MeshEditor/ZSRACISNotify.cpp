#include "StdAfx.h"
#include "ZSRACISNotify.h"

FILE *ZSRCreateFile(const string &FilePath)
{
	if(FilePath=="")
	{
		std::cout<<"The input data of ZSRCreateFile is empty!"<<std::endl;
		return NULL;
	}

	string str=FilePath;
	std::cout<<str<<":"<<std::endl;

	//Ҫ������·���Ѿ������ã����ĵ������ڣ��ɴ������ĵ�
	FILE *pFile = fopen((char*)str.data(),"w");
	if(pFile==NULL)
	{
		std::cout<<"ZSRCreateFile failed!"<<std::endl;
	}

	return pFile;
}

void ZSRWrite(FILE *_File,const string &strMessage)
{
	string str=strMessage;
	str += "\r\n";
	if(_File!=NULL && str!="")
		fprintf(_File,"%s",(char*)str.data());	   
	else 
		std::cout<<"ZSRWrite for "<<str<<" failed!!"<<std::endl;
}

void ZSRWriteDouble(FILE *_File,const string &head, double d)
{
	string str=head;
	str += convertToString(d);

	ZSRWrite(_File,str);
}
void ZSRWriteDoubles(FILE *_File,const string &head, const double *d,const int &n)
{
	if(n<=0) return;

	string str,str1;
	str = head;

	double x;
	x=d[0];
	str1 = convertToString(x);
	str += str1;
	for(int i=1;i<n;i++)
	{
		str+=" , ";

		x=d[i];
		str1 = convertToString(x);
		str += str1;
	}
	ZSRWrite(_File,str);
}
void ZSRWriteInt(FILE *_File,const string &head, const int &i)
{
	string str=head;

	string str1;
	str1 = convertToString(i);
	str+=str1;

	ZSRWrite(_File,str);
}
void ZSRWriteString(FILE *_File,const string &head, const string &istr)
{
	string str=head;

	str+=istr;

	ZSRWrite(_File,str);
}


//CString ZSRFindOutputPath()
//{
//	//char ModuleFileName[_MAX_PATH]={0};
// //   GetModuleFileName(NULL , ModuleFileName , _MAX_PATH );
//	//char ExePath[_MAX_DIR];           //��ǰ�ļ���
// //   char ExtName[_MAX_EXT];           //�ļ���׺
// //   char ExeFName[_MAX_FNAME];        //�ļ���
// //   char ExeDiver[_MAX_DRIVE];        //�ļ�����������C:
// //   _splitpath(ModuleFileName , ExeDiver , ExePath , ExeFName, ExtName );
//
//
//	TCHAR szPath[ MAX_PATH ] = {0};
//    if ( GetModuleFileName( NULL, szPath,MAX_PATH ) )
//     {
//      (_tcsrchr(szPath,_T('\\')))[1] = 0;
//     }
//     CString strPath = szPath;
//     strPath +="\\ACIS_OutputFile";
//
//	 return strPath;
//}

FILE *ZSRCreateFile(const string &FilePath,const string &FileName)
{
	if(FilePath=="" || FileName=="")
	{
		return NULL;
	}

	string str=FilePath;
	str +=FileName;

	char *fileWholePath=(char *)str.data();
	FILE *pFile = fopen(fileWholePath,"w");

	return pFile;
}


//#include "json/json.h"
//
//void ZSRReadInfoInJsonfile()
//{
//		////20211102 ��ȡjson�ļ��е�����
//	ifstream ifs;
//	//ifs.open(file_path.toStdString());
//    ifs.open("E:\\Projects\\assemble_git_DS\\testdata\\meshGenerate\\grid.json");
//    assert(ifs.is_open());
//
//    Json::Reader reader;
//    Json::Value root;
//
//    if (!reader.parse(ifs, root, false))
//    {
//		//ZSRWrite(pNotifyFile,"**************reader parse error!!!******************");
//        return ;
//    }
//
//	int iModelSize=0, iMeshSize=0, iTotalSize=0;
//    iTotalSize = root.size();
//	//ZSRWriteInt(pNotifyFile,"the total elements is:",iTotalSize);
//
//	const Json::Value arrayModel = root["solids"]; 
//	const Json::Value arrayMesh =root["grid"];
//	iModelSize = arrayModel.size();
//	iMeshSize = arrayMesh.size();
//
//	string strModelFilename; 
//	ENTITY_LIST bodylist;
//	//��ȡ���е�ʵ���ļ������ɲ���ʵ����װ����
//	for (int i = 0; i < iModelSize; ++i)
//    {
//		strModelFilename = arrayModel[i]["sat"].asString();
//
//		//��ʾģ��
//		bodylist.clear();
//		FILE *pFile = fopen (strModelFilename.c_str(), "r");
//		if (!pFile){
//			//ZSRWriteInt(pNotifyFile,"opening the file of solid model failed! ", i);
//			//ZSRWriteInt(pNotifyFile,"the index of this model is", arrayModel[i]["Index"].asInt());
//			QMessageBox::critical (NULL, QObject::tr ("�򿪴���"), QObject::tr("��ģ���ļ�ʧ�ܣ�"));
//			return ;
//		}
//		api_restore_entity_list (pFile, TRUE, bodylist);
//		std::cout<<"body����"<<bodylist.count()<<std::endl;
//	
//		for(int i=0;i<bodylist.count();i++)
//		{
//			//hoopsview->show_body_faces(bodylist[i]);
//
//			ENTITY_LIST edgelist;
//			api_get_edges(bodylist[i],edgelist);
//			for(int j=0;j<edgelist.count();j++)
//				;
//				//hoopsview->show_body_edges(edgelist[j]);
//		}
//    }
//
//
//	//����������λ�ã����Ʋ��Լ�������ģ��
//	int iKeyPtSize=0;
//	double dTempPoint[3];
//	double dStartPoint[3], dEndPoint[3];
//	SPAposition spaStartPt(0,0,0);
//	SPAposition spaEndPt(0,0,0);
//	for (int j = 0; j < arrayMesh["xs"].size(); ++j)
//	{
//		//ȡ����Դ��
//		dTempPoint[0] = arrayMesh["xs"][j].asDouble();
//		dTempPoint[1] = arrayMesh["ys"][j].asDouble();
//		dTempPoint[2] = arrayMesh["ys"][j].asDouble();
//		iKeyPtSize++;
//
//		//������������ֱ��
//		for(int k=0;k<3;k++)
//		{
//			dTempPoint[k] =  dTempPoint[k]-100;
//			spaStartPt.set_x(dTempPoint[0]);
//			spaStartPt.set_y(dTempPoint[1]);
//			spaStartPt.set_z(dTempPoint[2]);
//
//			dTempPoint[k] = dTempPoint[k]+200;
//			spaEndPt.set_x(dTempPoint[0]);
//			spaEndPt.set_y(dTempPoint[1]);
//			spaEndPt.set_z(dTempPoint[2]);
//
//			dTempPoint[k] =  dTempPoint[k]-100;
//	
//			EDGE* pMeshEdge = NULL;
//			api_curve_line(spaStartPt, spaEndPt, pMeshEdge);
//
//			//hoopsview->show_body_edges_1(pMeshEdge);
//		}
//	}
//}

