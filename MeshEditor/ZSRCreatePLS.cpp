#include "StdAfx.h"
//#include "ZSRCreatePLS.h"
//
//#include "ZSRACISNotify.h"
//#include "ZSRJudgeGeoElemRelationship.h"
//#include "FileManagement.h"
//
//#include <boolapi.hxx>
//
//FILE *pNotifyPLSFile=ZSRCreateFile("E:\\Projects\\Project_OutputFile\\PLSNotify.txt");
//
//ZSRCreatePLS::ZSRCreatePLS(void)
//{
//	
//}
//
//
//ZSRCreatePLS::~ZSRCreatePLS(void)
//{
//}
//
// ZSRCreatePLS::ZSRCreatePLS(const double &idCubeLength)
//{
//	if(idCubeLength==0)
//		ZSRWrite(pNotifyPLSFile,"the input of ZSRCreatePLS is empty!");
//	m_dCubeLength=idCubeLength;
//
//	ZSRWriteDouble(pNotifyPLSFile,"the input of ZSRCreatePLS is empty!",m_dCubeLength);
//}
//
//void ZSRCreatePLS::GenerateCandidatePLS()
//{
//	//implememt generate algorithm
//	//create key points
//	CreateKeypoints();
//
//	//select key faces
//	m_vecTKp.clear(), m_vecQKp.clear();
//	SelectLogicalKeyFaces(m_vecTKp,m_vecQKp);
//	DetermineKeyFaces();
//
//	CreatePrimaryPLS();
//	cout<<"-------------after CreatePrimaryPLS(), m_vecMicroPLS.size()="<<m_vecMicroPLS.size()<<endl;
//
//	//ZSRWriteInt(pNotifyPLSFile,"-------------after CreatePrimaryPLS(), m_vecMicroPLS.size()=",m_vecMicroPLS.size());
//
//	string strDirPath="E:\\Projects\\assemble_git_DS\\testdata\\PLSlibrary\\";
//	string strPLStype= "Keyface23\\";
//	string strFileType=".sat";
//	string strModelFile= "";
//	ENTITY_LIST listBody; 
//	double dPoint[4][3];
//	int i,j,k,m,iPtNum;
//	for(i=0;i<m_vecMicroPLS.size();i++)
//	{
//		listBody.clear();
//
//		if(i==10000) i=16400;
//		if(i==m_vecKfSet23.size()) strPLStype= "Keyface33\\";
//		else if(i==(m_vecKfSet23.size()+m_vecKfSet33.size())) strPLStype= "Keyface24\\";
//		else if(i==(m_vecKfSet23.size()+m_vecKfSet33.size()+m_vecKfSet24.size())) strPLStype= "Keyface34\\";
//
//
//		strModelFile=strDirPath+strPLStype+m_vecMicroPLS[i].strName+strFileType;
//
//		for(j=0;j<m_vecMicroPLS[i].vecPnSet.size();j++)
//		{
//			FACE *pTempFace=NULL;
//
//			iPtNum=m_vecMicroPLS[i].vecPnSet[j].vecPtSet.size();
//			for(k=0;k<iPtNum;k++)
//			{
//				for(m=0;m<3;m++) dPoint[k][m]=m_vecMicroPLS[i].vecPnSet[j].vecPtSet[k].dCoord[m];
//			}
//
//			pTempFace=CreateConvexPolygon(dPoint,iPtNum);
//			listBody.add(pTempFace);
//		}
//		save_acis_entity_list(listBody, strModelFile.c_str());
//	}
//	
//	//CreateReferenceHexahedrons(m_dCubeLength,listBody);
//	//ZSRWriteInt(pNotifyPLSFile,"the number of PLS is listBody.count()!",listBody.count());
//
//	/*BODY *pEntiRet=(BODY *)listBody[0];
//	for(int i=1;i<listBody.count();i++)
//	{
//		BODY *pTempEnti=(BODY *)listBody[i];
//		api_boolean(pTempEnti,pEntiRet,UNION);
//	}
//	save_acis_entity(pEntiRet,strModelFile.c_str());*/
//
//	/*FILE *pFile = fopen (strModelFile.c_str(), "w");
//	if (!pFile){
//		QMessageBox::critical (NULL, QObject::tr ("创建错误！"), QObject::tr("创建模型文件失败！"));
//		return ;
//	}*/
//}
//
//
//void ZSRCreatePLS::CreateKeypoints()
//{
//	double dCubeLength=m_dCubeLength;
//	
//	//iType=0
//	m_kpSet[0].dCoord[0]=0,                   m_kpSet[0].dCoord[1]=0,                     m_kpSet[0].dCoord[2]=0;
//	m_kpSet[1].dCoord[0]=0,                   m_kpSet[1].dCoord[1]=dCubeLength,  m_kpSet[1].dCoord[2]=0;
//	m_kpSet[2].dCoord[0]=dCubeLength, m_kpSet[2].dCoord[1]=dCubeLength,  m_kpSet[2].dCoord[2]=0;
//	m_kpSet[3].dCoord[0]=dCubeLength, m_kpSet[3].dCoord[1]=0,                    m_kpSet[3].dCoord[2]=0;
//	m_kpSet[4].dCoord[0]=0,                   m_kpSet[4].dCoord[1]=0,                     m_kpSet[4].dCoord[2]=dCubeLength;
//	m_kpSet[5].dCoord[0]=0,                   m_kpSet[5].dCoord[1]=dCubeLength,   m_kpSet[5].dCoord[2]=dCubeLength;
//	m_kpSet[6].dCoord[0]=dCubeLength, m_kpSet[6].dCoord[1]=dCubeLength,  m_kpSet[6].dCoord[2]=dCubeLength;
//	m_kpSet[7].dCoord[0]=dCubeLength, m_kpSet[7].dCoord[1]=0,                     m_kpSet[7].dCoord[2]=dCubeLength;
//	for(int i=0;i<8;i++) m_kpSet[i].iType=0;
//
//	//iType=1
//	m_kpSet[8].dCoord[0]=0,                           m_kpSet[8].dCoord[1]=0.5*dCubeLength,     m_kpSet[8].dCoord[2]=0;
//	m_kpSet[9].dCoord[0]=0.5*dCubeLength,  m_kpSet[9].dCoord[1]=dCubeLength,            m_kpSet[9].dCoord[2]=0;
//	m_kpSet[10].dCoord[0]=dCubeLength,       m_kpSet[10].dCoord[1]=0.5*dCubeLength,   m_kpSet[10].dCoord[2]=0;
//	m_kpSet[11].dCoord[0]=0.5*dCubeLength, m_kpSet[11].dCoord[1]=0,                            m_kpSet[11].dCoord[2]=0;
//	m_kpSet[12].dCoord[0]=0,                          m_kpSet[12].dCoord[1]=0.5*dCubeLength,   m_kpSet[12].dCoord[2]=dCubeLength;
//	m_kpSet[13].dCoord[0]=0.5*dCubeLength, m_kpSet[13].dCoord[1]=dCubeLength,          m_kpSet[13].dCoord[2]=dCubeLength;
//	m_kpSet[14].dCoord[0]=dCubeLength,        m_kpSet[14].dCoord[1]=0.5*dCubeLength,   m_kpSet[14].dCoord[2]=dCubeLength;
//	m_kpSet[15].dCoord[0]=0.5*dCubeLength, m_kpSet[15].dCoord[1]=0,                            m_kpSet[15].dCoord[2]=dCubeLength;
//	m_kpSet[16].dCoord[0]=0,                          m_kpSet[16].dCoord[1]=0,                            m_kpSet[16].dCoord[2]=0.5*dCubeLength;
//	m_kpSet[17].dCoord[0]=0                         , m_kpSet[17].dCoord[1]=dCubeLength,          m_kpSet[17].dCoord[2]=0.5*dCubeLength;
//	m_kpSet[18].dCoord[0]=dCubeLength,       m_kpSet[18].dCoord[1]=dCubeLength,          m_kpSet[18].dCoord[2]=0.5*dCubeLength;
//	m_kpSet[19].dCoord[0]=dCubeLength,       m_kpSet[19].dCoord[1]=0,                            m_kpSet[19].dCoord[2]=0.5*dCubeLength;
//	for(int i=8;i<20;i++) m_kpSet[i].iType=1;
//
//	//iType=2
//	m_kpSet[20].dCoord[0]=0,                          m_kpSet[20].dCoord[1]=0.5*dCubeLength,    m_kpSet[20].dCoord[2]=0.5*dCubeLength;
//	m_kpSet[21].dCoord[0]=0.5*dCubeLength, m_kpSet[21].dCoord[1]=dCubeLength,           m_kpSet[21].dCoord[2]=0.5*dCubeLength;
//	m_kpSet[22].dCoord[0]=dCubeLength,        m_kpSet[22].dCoord[1]=0.5*dCubeLength,    m_kpSet[22].dCoord[2]=0.5*dCubeLength;
//	m_kpSet[23].dCoord[0]=0.5*dCubeLength, m_kpSet[23].dCoord[1]=0,                             m_kpSet[23].dCoord[2]=0.5*dCubeLength;
//	m_kpSet[24].dCoord[0]=0.5*dCubeLength, m_kpSet[24].dCoord[1]=0.5*dCubeLength,    m_kpSet[24].dCoord[2]=0;
//	m_kpSet[25].dCoord[0]=0.5*dCubeLength, m_kpSet[25].dCoord[1]=0.5*dCubeLength,    m_kpSet[25].dCoord[2]=dCubeLength;
//	for(int i=20;i<26;i++) m_kpSet[i].iType=2;
//
//	//iType=3
//	m_kpSet[26].dCoord[0]=0.5*dCubeLength, m_kpSet[26].dCoord[1]=0.5*dCubeLength,    m_kpSet[26].dCoord[2]=0.5*dCubeLength;
//	m_kpSet[26].iType=3;
//
//	ZSRWrite(pNotifyPLSFile,"------------finish generating keypoints-------------");
//}
//
////all original face sets for candidate cases
//void ZSRCreatePLS::SelectLogicalKeyFaces(vector<vector<int>>  &veciTKp, vector<vector<int>> &veciQKp)
//{
//	int iFaceNum = 2, iFaceType = 3;
//	veciTKp.clear(),veciQKp.clear();
//
//	//select key faces for triangle or quadrangle
//	vector<vector<int>> vecOriTKp;
//	vector<vector<int>> vecOriQKp;
//	vecOriTKp.clear(),vecOriQKp.clear();
//
//	vector<int> vecValue; vecValue.clear();
//	for(int i=0;i<8;i++)   vecValue.push_back(i);
//    SelectMfromN(8,iFaceType,vecValue,vecOriTKp);
//	iFaceType=4;  
//	//for(int i=8;i<20;i++) vecValue.push_back(i);
//    SelectMfromN(8,iFaceType,vecValue,vecOriQKp);
//
//	ZSRWrite(pNotifyPLSFile,"------------after judging the validity of keyfaces, the number of combination case-------------");
//	ZSRWriteInt(pNotifyPLSFile,"the number of combination case of triangular key face is vecOriTKp.size()=",vecOriTKp.size());
//	ZSRWriteInt(pNotifyPLSFile,"the number of combination case of quadrangular key face is vecOriQKp.size()=",vecOriQKp.size());
//
//	//return;
//	//judge whether the keyface is logical
//	iFaceType=3; 
//	double dKeypoints[4][3];
//	for(int i=0;i<vecOriTKp.size();i++)
//	{
//		for(int j=0;j<iFaceType;j++)
//		{
//			dKeypoints[j][0]=m_kpSet[vecOriTKp[i][j]].dCoord[0];
//			dKeypoints[j][1]=m_kpSet[vecOriTKp[i][j]].dCoord[1];
//			dKeypoints[j][2]=m_kpSet[vecOriTKp[i][j]].dCoord[2];
//		}
//
//		if(IsLogicalFace(3,dKeypoints)) 
//		{
//			//ZSRWrite(pNotifyPLSFile,"---------after judging IsLogicalFace(3,dKeypoints)-----------");
//			if(IsSatisfyProcessAngle(vecOriTKp[i],45))
//				veciTKp.push_back(vecOriTKp[i]);
//		}
//			//select logical keyfaces for veciTKp or veciQKp
//	}
//	iFaceType=4; 
//	double dFirLine[2][3], dSecLine[2][3];
//	int iTemp,iChangeTimes=0;
//	for(int i=0;i<vecOriQKp.size();i++)
//	{
//		for(int j=0;j<iFaceType;j++)
//		{
//			dKeypoints[j][0]=m_kpSet[vecOriQKp[i][j]].dCoord[0];
//			dKeypoints[j][1]=m_kpSet[vecOriQKp[i][j]].dCoord[1];
//			dKeypoints[j][2]=m_kpSet[vecOriQKp[i][j]].dCoord[2];
//		}
//
//		if(IsLogicalFace(4,dKeypoints)) 
//		{
//			//ZSRWrite(pNotifyPLSFile,"---------after judging IsLogicalFace(4,dKeypoints)-----------");
//			if(IsSatisfyProcessAngle(vecOriQKp[i],45))
//			{
//				//check the order of keypoint, prevent keyline from intersecting.  23x14
//				for(int k=0;k<3;k++) dFirLine[0][k]=dKeypoints[0][k], dFirLine[1][k]=dKeypoints[3][k],dSecLine[0][k]=dKeypoints[1][k],dSecLine[1][k]=dKeypoints[2][k];
//				if(ZSRIsIntersectedByTwoLine(dFirLine,dSecLine))
//				{
//					iTemp=vecOriQKp[i][2], vecOriQKp[i][2]=vecOriQKp[i][3], vecOriQKp[i][3]=iTemp, iChangeTimes++;
//				}
//				//check the order of keypoint, prevent keyline from intersecting.  12x34
//				for(int k=0;k<3;k++) dFirLine[0][k]=dKeypoints[0][k], dFirLine[1][k]=dKeypoints[1][k],dSecLine[0][k]=dKeypoints[2][k],dSecLine[1][k]=dKeypoints[3][k];
//				if(ZSRIsIntersectedByTwoLine(dFirLine,dSecLine))
//				{
//					iTemp=vecOriQKp[i][2], vecOriQKp[i][2]=vecOriQKp[i][1], vecOriQKp[i][1]=iTemp, iChangeTimes++;
//				}
//
//				veciQKp.push_back(vecOriQKp[i]);
//			}
//		}
//		
//	}
//	ZSRWriteInt(pNotifyPLSFile,"the number of combination case of triangular key face is veciTKp.size()=",veciTKp.size());
//	ZSRWriteInt(pNotifyPLSFile,"the number of combination case of quadrangular key face is veciQKp.size()=",veciQKp.size());
//	ZSRWriteInt(pNotifyPLSFile,"In order to prevent keyline from intersecting, change some point's order for key face, the iChangeTimes=",iChangeTimes);
//
//	vecOriTKp.clear(),vecOriQKp.clear();
//}
//
////whether the face is logical or not
//bool ZSRCreatePLS::IsLogicalFace(const int &iFaceType,  double idKeypoints[][3])
//{
//	if(iFaceType<2) return false;
//
//	if(iFaceType==3)
//	{
//		if(ZSRIsPointsInSameSupportedLine(idKeypoints,3))
//			return false;
//	}
//	else if(iFaceType==4)
//	{
//		// if one of these four points in the line which is generated by two of four points.
//		double dTempKps[3][3], dMidPoint[3];;
//		vector<vector<int>> vecKp;
//		vecKp.clear();
//
//		int i,j; 
//		vector<int> vecValue; vecValue.clear();
//		for(i=0;i<4;i++)   vecValue.push_back(i);
//		SelectMfromN(4,3,vecValue,vecKp);
//		for(i=0;i<4;i++)
//		{
//			int iTempIndex[4]={0};
//			for(j=0;j<3;j++)
//			{
//				dTempKps[j][0]=idKeypoints[vecKp[i][j]][0];
//				dTempKps[j][1]=idKeypoints[vecKp[i][j]][1];
//				dTempKps[j][2]=idKeypoints[vecKp[i][j]][2];
//				iTempIndex[vecKp[i][j]]=1;
//			}
//			if(ZSRIsPointsInSameSupportedLine(dTempKps,3)) 
//				return false;
//			
//			//if four points form a trianglar, return false. it means that one of  points is the midpoint of other points.
//			for(j=0;j<4;j++)
//				if(iTempIndex[j]!=1) break;
//
//			dMidPoint[0]=1/3*(dTempKps[0][0]+dTempKps[1][0]+dTempKps[2][0]);
//			dMidPoint[1]=1/3*(dTempKps[0][1]+dTempKps[1][1]+dTempKps[2][1]);
//			dMidPoint[2]=1/3*(dTempKps[0][2]+dTempKps[1][2]+dTempKps[2][2]);
//			if(fabs(dMidPoint[0]-idKeypoints[j][0])<0.0001 && fabs(dMidPoint[1]-idKeypoints[j][1])<0.0001 &&fabs(dMidPoint[2]-idKeypoints[j][2])<0.0001 ) 
//				return false;
//		}
//
//		//if four points doesn't in the same face
//		if(!ZSRIsPointsInSameSupportedFace(idKeypoints,4))  
//			return false;
//	}
//	else
//		return false;
//
//	return true;
//}
//
////get all keyface sets which satisfy all design constraints. include L23,L33,    L24, and L34
//void ZSRCreatePLS::DetermineKeyFaces()
//{
//	//select key face Set for triangle or quadrangle
//	vector<vector<int>> vecOriKfS23; vecOriKfS23.clear();
//	vector<vector<int>> vecOriKfS33; vecOriKfS33.clear();
//	vector<vector<int>> vecOriKfS24; vecOriKfS24.clear();
//	vector<vector<int>> vecOriKfS34; vecOriKfS34.clear();
//	vector<int> vecValue; vecValue.clear();
//
//	ZSRWriteInt(pNotifyPLSFile,"the number of the logical triangular key face is m_vecTKp.size()=",m_vecTKp.size());
//	ZSRWriteInt(pNotifyPLSFile,"the number of the logical quadrangular key face is m_vecQKp.size()=",m_vecQKp.size());
//
//	int i,j;
//	for(i=0;i<m_vecTKp.size();i++)   vecValue.push_back(i);
//    SelectMfromN(m_vecTKp.size(),2,vecValue,vecOriKfS23);
//    SelectMfromN(m_vecTKp.size(),3,vecValue,vecOriKfS33);
//	vecValue.clear();
//	for (i=0;i<m_vecQKp.size();i++)   vecValue.push_back(i);
//    SelectMfromN(m_vecQKp.size(),2,vecValue,vecOriKfS24);
//    SelectMfromN(m_vecQKp.size(),3,vecValue,vecOriKfS34);
//
//	ZSRWrite(pNotifyPLSFile,"------------before judging the validity of keyfaceSet, the number of combination case-------------");
//	ZSRWriteInt(pNotifyPLSFile,"the number of combination case of triangular key face Set is vecOriKfS23.size()=",vecOriKfS23.size());
//	ZSRWriteInt(pNotifyPLSFile,"the number of combination case of triangular key face Set is vecOriKfS33.size()=",vecOriKfS33.size());
//	ZSRWriteInt(pNotifyPLSFile,"the number of combination case of quadrangular key face Set is vecOriKfS24.size()=",vecOriKfS24.size());
//	ZSRWriteInt(pNotifyPLSFile,"the number of combination case of quadrangular key face Set is vecOriKfS34.size()=",vecOriKfS34.size());
//
//	//select all logical face set from each type of keyfaceset L23,L33,    L24, and L34
//	vector<Keyface> vecTempKf; 
//	Keyface kfTemp;
//	KeyfaceSet kfsTemp;
//	for(i=0;i<vecOriKfS23.size();i++) 
//	{
//		if(vecOriKfS23[i].size()!=2) return;
//		vecTempKf.clear();
//
//		for(j=0;j<2;j++)
//		{
//			kfTemp.vecKpNum.clear();
//			kfTemp.vecKpNum=m_vecTKp[vecOriKfS23[i][j]];
//			vecTempKf.push_back(kfTemp);
//		}
//		
//		if(IsKeyFaceSetSatisfyConstraints(vecTempKf)) 
//		{
//			kfsTemp.vecKf.clear();
//			kfsTemp.vecKf=vecTempKf;
//			m_vecKfSet23.push_back(kfsTemp);
//		}
//	}
//	vecOriKfS23.clear();
//
//	cout<<"finish m_vecKfSet23"<<endl;
//
//	for(i=0;i<vecOriKfS33.size();i++) 
//	{
//		if(vecOriKfS33[i].size()!=3) return;
//		vecTempKf.clear();
//
//		for(j=0;j<3;j++)
//		{
//			kfTemp.vecKpNum.clear();
//			kfTemp.vecKpNum=m_vecTKp[vecOriKfS33[i][j]];
//			vecTempKf.push_back(kfTemp);
//		}
//		
//		if(IsKeyFaceSetSatisfyConstraints(vecTempKf)) 
//		{
//			kfsTemp.vecKf.clear();
//			kfsTemp.vecKf=vecTempKf;
//			m_vecKfSet33.push_back(kfsTemp);
//		}
//	}
//	vecOriKfS33.clear();
//
//	cout<<"finish m_vecKfSet33"<<endl;
//
//	for(i=0;i<vecOriKfS24.size();i++) 
//	{
//		if(vecOriKfS24[i].size()!=2) return;
//		vecTempKf.clear();
//
//		for(j=0;j<2;j++)
//		{
//			kfTemp.vecKpNum.clear();
//			kfTemp.vecKpNum=m_vecQKp[vecOriKfS24[i][j]];
//			vecTempKf.push_back(kfTemp);
//		}
//		
//		if(IsKeyFaceSetSatisfyConstraints(vecTempKf)) 
//		{
//			kfsTemp.vecKf.clear();
//			kfsTemp.vecKf=vecTempKf;
//			m_vecKfSet24.push_back(kfsTemp);
//		}
//	}
//	vecOriKfS24.clear();
//	cout<<"finish m_vecKfSet24"<<endl;
//
//	for(i=0;i<vecOriKfS34.size();i++) 
//	{
//		if(vecOriKfS34[i].size()!=3) return;
//		vecTempKf.clear();
//
//		for(j=0;j<3;j++)
//		{
//			kfTemp.vecKpNum.clear();
//			kfTemp.vecKpNum=m_vecQKp[vecOriKfS34[i][j]];
//			vecTempKf.push_back(kfTemp);
//		}
//		
//		if(IsKeyFaceSetSatisfyConstraints(vecTempKf)) 
//		{
//			kfsTemp.vecKf.clear();
//			kfsTemp.vecKf=vecTempKf;
//			m_vecKfSet34.push_back(kfsTemp);
//		}
//	}
//	vecOriKfS34.clear();
//	cout<<"finish m_vecKfSet34"<<endl;
//
//	//select each face set, judge whether it satifies all constraints or not
//
//	ZSRWrite(pNotifyPLSFile,"------------after judging the validity of keyfaceSet, the number of combination case-------------");
//	ZSRWriteInt(pNotifyPLSFile,"the number of combination case of triangular key face Set is m_vecKfSet23.size()=",m_vecKfSet23.size());
//	ZSRWriteInt(pNotifyPLSFile,"the number of combination case of triangular key face Set is m_vecKfSet33.size()=",m_vecKfSet33.size());
//	ZSRWriteInt(pNotifyPLSFile,"the number of combination case of quadrangular key face Set is m_vecKfSet24.size()=",m_vecKfSet24.size());
//	ZSRWriteInt(pNotifyPLSFile,"the number of combination case of quadrangular key face Set is m_vecKfSet34.size()=",m_vecKfSet34.size());
//}
//
//bool ZSRCreatePLS::IsKeyFaceSetSatisfyConstraints(const vector<Keyface> &ivecKfs)
//{
//	if(ivecKfs.size()<1) return false;
//
//	if(HaveThreeInterfacesAndConnectedWithZaxis(ivecKfs))
//	{
//		if(AllKeyfacesJoined(ivecKfs)) 
//			return true;
//	}
//	return false;
//}
//
////generate primary PLS by symmetry operation with X, Y, and Z axis
//void ZSRCreatePLS::CreatePrimaryPLS()
//{
//	m_vecMicroPLS.clear();
//	MicroPLS *pTempMPLS=new MicroPLS;
//	Mypolygon *pTempMpn=new Mypolygon;
//	Mypoint *pMpt=new Mypoint;
//
//	int i,j,k,m;
//	for(i=0;i<m_vecKfSet23.size();i++)
//	{
//		pTempMPLS->vecPnSet.clear();
//		pTempMPLS->strName="";
//		for(j=0;j<m_vecKfSet23[i].vecKf.size();j++)
//		{
//			pTempMpn->vecPtSet.clear();
//			for(k=0;k<m_vecKfSet23[i].vecKf[j].vecKpNum.size();k++)
//			{
//				for(m=0;m<3;m++) 
//					pMpt->dCoord[m]=m_kpSet[m_vecKfSet23[i].vecKf[j].vecKpNum[k]].dCoord[m];
//				pTempMpn->vecPtSet.push_back(*pMpt);
//			}
//			pTempMPLS->vecPnSet.push_back(*pTempMpn);
//		}
//		pTempMPLS->strName="T23-"+convertToString(i);
//
//		if(i==1) ZSRWriteInt(pNotifyPLSFile,"before OpteratePlaneSymmetryforKeyfaceSet, pTempMPLS->vecPnSet.size()=",pTempMPLS->vecPnSet.size());
//		OpteratePlaneSymmetryforKeyfaceSet(*pTempMPLS);
//		if(i==1) ZSRWriteInt(pNotifyPLSFile,"------after OpteratePlaneSymmetryforKeyfaceSet, pTempMPLS->vecPnSet.size()=",pTempMPLS->vecPnSet.size());
//
//		m_vecMicroPLS.push_back(*pTempMPLS);
//	}
//
//	cout<<"finish create primary PLS for T23"<<endl;
//
//	for(i=0;i<m_vecKfSet33.size();i++)
//	{
//		pTempMPLS->vecPnSet.clear();
//		pTempMPLS->strName="";
//		for(j=0;j<m_vecKfSet33[i].vecKf.size();j++)
//		{
//			pTempMpn->vecPtSet.clear();
//			for(k=0;k<m_vecKfSet33[i].vecKf[j].vecKpNum.size();k++)
//			{
//				for(m=0;m<3;m++) 
//					pMpt->dCoord[m]=m_kpSet[m_vecKfSet33[i].vecKf[j].vecKpNum[k]].dCoord[m];
//				pTempMpn->vecPtSet.push_back(*pMpt);
//			}
//			pTempMPLS->vecPnSet.push_back(*pTempMpn);
//		}
//		pTempMPLS->strName="T33-"+convertToString(i);
//
//		if(i==1) ZSRWriteInt(pNotifyPLSFile,"before OpteratePlaneSymmetryforKeyfaceSet, pTempMPLS->vecPnSet.size()=",pTempMPLS->vecPnSet.size());
//		OpteratePlaneSymmetryforKeyfaceSet(*pTempMPLS);
//		if(i==1) ZSRWriteInt(pNotifyPLSFile,"------after OpteratePlaneSymmetryforKeyfaceSet, pTempMPLS->vecPnSet.size()=",pTempMPLS->vecPnSet.size());
//
//		m_vecMicroPLS.push_back(*pTempMPLS);
//	}
//
//	cout<<"finish create primary PLS for T33"<<endl;
//
//	for(i=0;i<m_vecKfSet24.size();i++)
//	{
//		//cout<<"in the process of  the  "<<i<<"th primary PLS by T24"<<i<<endl;
//
//		for(j=0;j<m_vecKfSet24[i].vecKf.size();j++)
//		{
//			for(k=0;k<m_vecKfSet24[i].vecKf[j].vecKpNum.size();k++)
//			{
//				for(m=0;m<3;m++) 
//					pMpt->dCoord[m]=m_kpSet[m_vecKfSet24[i].vecKf[j].vecKpNum[k]].dCoord[m];
//				pTempMpn->vecPtSet.push_back(*pMpt);
//
//				//if(i==0 && j==0) cout<<"k="<<k<<endl; 
//			}
//			pTempMPLS->vecPnSet.push_back(*pTempMpn);
//
//			//if(i==0) cout<<"j="<<j<<endl; 
//
//			pTempMpn->vecPtSet.clear();
//		}
//		pTempMPLS->strName="T24-"+convertToString(i);
//
//		//cout<<"begin OpteratePlaneSymmetry!"<<endl; 
//		if(i==1) ZSRWriteInt(pNotifyPLSFile,"before OpteratePlaneSymmetryforKeyfaceSet, pTempMPLS->vecPnSet.size()=",pTempMPLS->vecPnSet.size());
//		OpteratePlaneSymmetryforKeyfaceSet(*pTempMPLS);
//		if(i==1) ZSRWriteInt(pNotifyPLSFile,"------after OpteratePlaneSymmetryforKeyfaceSet, pTempMPLS->vecPnSet.size()=",pTempMPLS->vecPnSet.size());
//		//cout<<"finish OpteratePlaneSymmetry!"<<endl; 
//
//		m_vecMicroPLS.push_back(*pTempMPLS);
//
//		pTempMPLS->vecPnSet.clear();
//		pTempMPLS->strName="";
//
//	}
//
//	cout<<"finish create primary PLS for T24"<<endl;
//
//	for(i=0;i<m_vecKfSet34.size();i++)
//	{
//		pTempMPLS->vecPnSet.clear();
//		pTempMPLS->strName="";
//		for(j=0;j<m_vecKfSet34[i].vecKf.size();j++)
//		{
//			pTempMpn->vecPtSet.clear();
//			for(k=0;k<m_vecKfSet34[i].vecKf[j].vecKpNum.size();k++)
//			{
//				for(m=0;m<3;m++) 
//					pMpt->dCoord[m]=m_kpSet[m_vecKfSet34[i].vecKf[j].vecKpNum[k]].dCoord[m];
//				pTempMpn->vecPtSet.push_back(*pMpt);
//			}
//			pTempMPLS->vecPnSet.push_back(*pTempMpn);
//		}
//		pTempMPLS->strName="T34-"+convertToString(i);
//
//		if(i==1) ZSRWriteInt(pNotifyPLSFile,"before OpteratePlaneSymmetryforKeyfaceSet, pTempMPLS->vecPnSet.size()=",pTempMPLS->vecPnSet.size());
//		OpteratePlaneSymmetryforKeyfaceSet(*pTempMPLS);
//		if(i==1) ZSRWriteInt(pNotifyPLSFile,"------after OpteratePlaneSymmetryforKeyfaceSet, pTempMPLS->vecPnSet.size()=",pTempMPLS->vecPnSet.size());
//
//		m_vecMicroPLS.push_back(*pTempMPLS);
//	}
//
//	cout<<"finish create primary PLS for T34"<<endl;
//}
//void ZSRCreatePLS::OpteratePlaneSymmetryforKeyfaceSet(MicroPLS &ioMicroPLS)
//{
//	if(ioMicroPLS.vecPnSet.size()<1 || ioMicroPLS.strName=="") return;
//
//	Mypolygon *pTempMpn=new Mypolygon;
//	Mypoint *pMpt=new Mypoint;
//
//	int i,j,k,iOriSize;
//	//symmetry by ZY plane
//	iOriSize=ioMicroPLS.vecPnSet.size();
//	for(i=0;i<iOriSize;i++)
//	{
//		pTempMpn->vecPtSet.clear();
//		for(j=0;j<ioMicroPLS.vecPnSet[i].vecPtSet.size();j++)
//		{
//			pMpt->dCoord[0]=-ioMicroPLS.vecPnSet[i].vecPtSet[j].dCoord[0];
//			pMpt->dCoord[1]= ioMicroPLS.vecPnSet[i].vecPtSet[j].dCoord[1];
//			pMpt->dCoord[2]= ioMicroPLS.vecPnSet[i].vecPtSet[j].dCoord[2];
//			pTempMpn->vecPtSet.push_back(*pMpt);
//		}
//		ioMicroPLS.vecPnSet.push_back(*pTempMpn);
//	}
//
//	//symmetry by ZX plane
//	iOriSize=ioMicroPLS.vecPnSet.size();
//	for(i=0;i<iOriSize;i++)
//	{
//		pTempMpn->vecPtSet.clear();
//		for(j=0;j<ioMicroPLS.vecPnSet[i].vecPtSet.size();j++)
//		{
//			pMpt->dCoord[0]= ioMicroPLS.vecPnSet[i].vecPtSet[j].dCoord[0];
//			pMpt->dCoord[1]=-ioMicroPLS.vecPnSet[i].vecPtSet[j].dCoord[1];
//			pMpt->dCoord[2]= ioMicroPLS.vecPnSet[i].vecPtSet[j].dCoord[2];
//			pTempMpn->vecPtSet.push_back(*pMpt);
//		}
//		ioMicroPLS.vecPnSet.push_back(*pTempMpn);
//	}
//
//	//symmetry by XY plane
//	iOriSize=ioMicroPLS.vecPnSet.size();
//	for(i=0;i<iOriSize;i++)
//	{
//		pTempMpn->vecPtSet.clear();
//		for(j=0;j<ioMicroPLS.vecPnSet[i].vecPtSet.size();j++)
//		{
//			pMpt->dCoord[0]= ioMicroPLS.vecPnSet[i].vecPtSet[j].dCoord[0];
//			pMpt->dCoord[1]= ioMicroPLS.vecPnSet[i].vecPtSet[j].dCoord[1];
//			pMpt->dCoord[2]=-ioMicroPLS.vecPnSet[i].vecPtSet[j].dCoord[2];
//			pTempMpn->vecPtSet.push_back(*pMpt);
//		}
//		ioMicroPLS.vecPnSet.push_back(*pTempMpn);
//	}
//}
////constraint: keep connecting in internal structure of PLS
//bool ZSRCreatePLS::IsConnectedWithZaxis(const vector<Keyface> &ivecKfs)
//{
//	if(ivecKfs.size()<1) return false;
//
//	//judge whether one of keypoints is on the Zaxis or not. 
//	Keyface kfTemp;
//	int i,j;
//	for(i=0;i< ivecKfs.size();i++)
//	{
//		kfTemp.vecKpNum.clear();
//
//		kfTemp=ivecKfs[i];
//		for(j=0;j<kfTemp.vecKpNum.size();j++)
//		{
//			//it should be selected from kp[0], kp[4], kp[16], their are on the z axis
//			if(m_kpSet[kfTemp.vecKpNum[j]].dCoord[0]==0  && m_kpSet[kfTemp.vecKpNum[j]].dCoord[1]==0)
//				return true;
//		}
//	}
//	return false;
//}
//
////each interface isn't connected with one of faces 
//bool ZSRCreatePLS::HaveThreeInterfacesAndConnectedWithZaxis(const vector<Keyface> &ivecKfs)
//{
//	if(ivecKfs.size()<1) return false;
//
//	int i,j;
//	Keyface kfTemp;
//	int iPlanePoint[3]={0,0,0}, iZaxisPoint=0,iXYplane=0;
//	//one of faces intersects with one of interfaces.
//	for(i=0;i<ivecKfs.size();i++)
//	{
//		kfTemp.vecKpNum.clear();
//
//		kfTemp=ivecKfs[i];
//		for(j=0;j<kfTemp.vecKpNum.size();j++)
//		{
//			//it should be selected from kp[0], kp[4], kp[16], their are on the z axis
//			if(m_kpSet[kfTemp.vecKpNum[j]].dCoord[0]==0  && m_kpSet[kfTemp.vecKpNum[j]].dCoord[1]==0 /*&& m_kpSet[kfTemp.vecKpNum[j]].dCoord[2]==0 */) 
//			{
//				iZaxisPoint++;
//				if(m_kpSet[kfTemp.vecKpNum[j]].dCoord[2]==0) iXYplane++;
//			}
//			else
//			{
//				//plane 1: x=length, plane 2: y=length
//				if(m_kpSet[kfTemp.vecKpNum[j]].dCoord[0]==m_dCubeLength) 
//					iPlanePoint[0]++;
//				if(m_kpSet[kfTemp.vecKpNum[j]].dCoord[1]==m_dCubeLength) 
//					iPlanePoint[1]++;	
//				//plane 3: z=length
//				if(m_kpSet[kfTemp.vecKpNum[j]].dCoord[2]==m_dCubeLength) 
//					iPlanePoint[2]++;
//				//plane 4: z=0
//				if(m_kpSet[kfTemp.vecKpNum[j]].dCoord[2]==0) 
//					iXYplane++;
//
//			}
//		}
//
//		if(iPlanePoint[0]>1 &&iPlanePoint[1]>1 &&iPlanePoint[2]>1 && iZaxisPoint>0 &&iXYplane>1) 
//			return true;
//	}
//	return false;
//}
//
////all faces are intersected with other faces.
//bool ZSRCreatePLS::AllKeyfacesJoined(const vector<Keyface> &ivecKfs)
//{
//	if(ivecKfs.size()<1) return false;
//
//	double dFirPolygon[4][3];
//	double dSecPolygon[4][3];
//	int iFPn,iSPn;
//
//	Keyface tempKf[2];
//	tempKf[0]=ivecKfs[0];
//	iFPn=tempKf[0].vecKpNum.size();
//	if(iFPn>4) return false;
//	for(int i=0;i<iFPn;i++)
//		for(int j=0;j<3;j++)
//			dFirPolygon[i][j]=m_kpSet[tempKf[0].vecKpNum[i]].dCoord[j];
//
//	for(int k=1;k<ivecKfs.size();k++)
//	{
//		tempKf[1]=ivecKfs[k];
//		iSPn=tempKf[1].vecKpNum.size();
//		if(iSPn>4) return false;
//
//		for(int i=0;i<iSPn;i++)
//		for(int j=0;j<3;j++)
//			dSecPolygon[i][j]=m_kpSet[tempKf[1].vecKpNum[i]].dCoord[j];
//
//		if(!ZSRIsPolygonsIntersect(dFirPolygon,iFPn,dSecPolygon,iSPn)) return false;
//	}
//
//	return true;
//}
//
////the process constraint can be represented as : alpha<betal<alpha+90
////alpha is the maximal self-supporting angle, betal is a angle between the face normal and the printing direction. note that face normal is belong to the external face.
//bool ZSRCreatePLS::IsSatisfyProcessAngle(const vector<int> &ivecKfSet, const double &idProcessAngle)
//{
//	if(ivecKfSet.size()<3 || idProcessAngle<0) return false;
//	
//	double dTempVec[3][3],dTempAngle,dTempA,dTempB;
//
//	dTempVec[0][0]=m_kpSet[ivecKfSet[1]].dCoord[0]-m_kpSet[ivecKfSet[0]].dCoord[0];
//	dTempVec[0][1]=m_kpSet[ivecKfSet[1]].dCoord[1]-m_kpSet[ivecKfSet[0]].dCoord[1];
//	dTempVec[0][2]=m_kpSet[ivecKfSet[1]].dCoord[2]-m_kpSet[ivecKfSet[0]].dCoord[2];
//
//	dTempVec[1][0]=m_kpSet[ivecKfSet[2]].dCoord[0]-m_kpSet[ivecKfSet[0]].dCoord[0];
//	dTempVec[1][1]=m_kpSet[ivecKfSet[2]].dCoord[1]-m_kpSet[ivecKfSet[0]].dCoord[1];
//	dTempVec[1][2]=m_kpSet[ivecKfSet[2]].dCoord[2]-m_kpSet[ivecKfSet[0]].dCoord[2];
//
//	dTempVec[2][0]=dTempVec[0][1]*dTempVec[1][2]-dTempVec[0][2]*dTempVec[1][1];
//	dTempVec[2][1]=dTempVec[1][0]*dTempVec[0][2]-dTempVec[0][0]*dTempVec[1][2];
//	dTempVec[2][2]=dTempVec[0][0]*dTempVec[1][1]-dTempVec[0][1]*dTempVec[1][0];
//
//	dTempA=1; //dZ[3]={0,0,1};
//	dTempB=dTempVec[2][0]*dTempVec[2][0]+dTempVec[2][1]*dTempVec[2][1]+dTempVec[2][2]*dTempVec[2][2];
//	dTempAngle =acos(dTempVec[2][2] / (sqrt(dTempA)+sqrt(dTempB)))*180.0/3.14159265359;  //a*b=|a||b|cos(dTempAngle)
//	
//	ZSRWriteDouble(pNotifyPLSFile,"----------------------dTempAngle=",dTempAngle);
//
//	if((dTempAngle>=-45 && dTempAngle<idProcessAngle) || ((dTempAngle>90+idProcessAngle) && dTempAngle<=180)) 
//	{
//			//ZSRWriteDouble(pNotifyPLSFile,"this keyface doesn't satisfy the process constraint, it' dTempAngle=",dTempAngle);
//			return false;
//	}
//	return true;
//}
//
////select m elements from n elements --key algorithm
//void ZSRCreatePLS::SelectMfromN(const int &n,const int &m, const vector<int> &ivecValue,vector<vector<int>> &vecCandiCase)  
//{
//	if(m==0 || n==0) return;
//	vector<int> vecMelem;
//	vector<int> vecIndex;  vecIndex.clear();
//
//	int i,j,k;
//	for(i=0;i<n;i++) vecIndex.push_back(0);
//    for(i=0;i<m;i++)
//    {
//        vecIndex[i]=1;
//		vecMelem.push_back(i);
//    }//the first case
//	vecCandiCase.push_back(vecMelem);
//
//    while(!JudgeNextAction(n,m,vecIndex))  //if don't make all 1 in the most right side, continue!!!
//    {
//        for(i=0;i<n-1;i++)  //noted that i<n-1
//        {
//            //find the first 10 and change it into 01
//            if(vecIndex[i]==1&&vecIndex[i+1]==0)
//            {
//                vecIndex[i]=0,  vecIndex[i+1]=1;
//                //move all 1 in the left of 01 into the most left.
//                int count=0;
//                for(j=0;j<i;j++)
//                {
//                    if(vecIndex[j])
//                    {
//                        vecIndex[j]=0;
//                        vecIndex[count++]=1;
//                    }
//                }
//
//				vecMelem.clear();
//				for(k=0;k<n;k++)
//					if(vecIndex[k]==1)
//						vecMelem.push_back(ivecValue[k]);
//
//				vecCandiCase.push_back(vecMelem);
//                break;
//            }
//        }
//    }
//
//}
//
////combination
//bool ZSRCreatePLS::JudgeNextAction(const int &n,const int &m,const vector<int> &ivecIndex)
//{
//    for(int i=n-1;i>=n-m;i--)
//		if(!ivecIndex[i])  return false;
//
//    return true;
//}
//
//FACE * ZSRCreatePLS::CreateConvexPolygon(double idPoint[][3], const int &iPointNum)
//{
//	if(iPointNum<1) return NULL;
//	FACE * pTempFace=  NULL;
//
//	outcome result;
//	
//	SPAposition * pos_array = ACIS_NEW SPAposition[iPointNum+1];
//	for(int i=0; i<iPointNum;i++)
//	{
//		pos_array[i] = SPAposition(idPoint[i][0], idPoint[i][1], idPoint[i][2]);
//	}
//	pos_array[iPointNum] = SPAposition(idPoint[0][0], idPoint[0][1], idPoint[0][2]);
//
//	BODY* pTempBody=NULL;
//	WIRE * pTempWire = NULL;
//    result = api_make_wire (pTempBody, iPointNum+1, pos_array, pTempBody);
//    pTempWire = pTempBody->lump()->shell()->wire();
//    result = api_cover_wire (pTempWire, *(surface*)NULL_REF, pTempFace);
//
//	if(pTempFace==NULL) return NULL;
//	
//	return pTempFace;
//}
//void  ZSRCreatePLS::CreateReferenceHexahedrons(const double &idCubeLength, ENTITY_LIST &oListBody)
//{
//	//bulid three symmetry plans
//	outcome result;
//	
//	BODY* pTempBody=NULL;
//	SPAposition * pos_array = ACIS_NEW SPAposition[5];
//	WIRE * pTempWire = NULL;
//
//	FACE * pTempFace1 = NULL;
//	pos_array[0] = SPAposition(0, 0.5*idCubeLength, 0);
//    pos_array[1] = SPAposition(0, 0.5*idCubeLength, idCubeLength);
//    pos_array[2] = SPAposition( idCubeLength, 0.5*idCubeLength, idCubeLength);
//    pos_array[3] = SPAposition(idCubeLength, 0.5*idCubeLength,0);
//	pos_array[4] = SPAposition(0, 0.5*idCubeLength, 0);
//    result = api_make_wire (pTempBody, 5, pos_array, pTempBody);
//    pTempWire = pTempBody->lump()->shell()->wire();
//    result = api_cover_wire (pTempWire, *(surface*)NULL_REF, pTempFace1);
//	oListBody.add(pTempFace1);
//
//	FACE * pTempFace2 = NULL;
//    pos_array[0] = SPAposition(0.5*idCubeLength, 0, 0);
//    pos_array[1] = SPAposition(0.5*idCubeLength, 0,idCubeLength);
//    pos_array[2] = SPAposition( 0.5*idCubeLength, idCubeLength,idCubeLength);
//    pos_array[3] = SPAposition( 0.5*idCubeLength, idCubeLength, 0);
//	pos_array[4] =  SPAposition(0.5*idCubeLength, 0, 0);
//    result = api_make_wire (pTempBody, 5, pos_array, pTempBody);
//    pTempWire = pTempBody->lump()->shell()->wire();
//    result = api_cover_wire (pTempWire, *(surface*)NULL_REF, pTempFace2);
//	oListBody.add(pTempFace2);
//
//	FACE * pTempFace3 = NULL;
//	pos_array[0] = SPAposition(0, 0, 0.5*idCubeLength);
//    pos_array[1] = SPAposition(0,  idCubeLength,  0.5*idCubeLength);
//    pos_array[2] = SPAposition( idCubeLength,  idCubeLength, 0.5*idCubeLength);
//    pos_array[3] = SPAposition( idCubeLength, 0,  0.5*idCubeLength);
//	pos_array[4] =  SPAposition(0, 0, 0.5*idCubeLength);
//    result = api_make_wire (pTempBody, 5, pos_array, pTempBody);
//    pTempWire = pTempBody->lump()->shell()->wire();
//    result = api_cover_wire (pTempWire, *(surface*)NULL_REF, pTempFace3);
//	oListBody.add(pTempFace3);
//
//	ACIS_DELETE [] pos_array;  // De-allocate the array.
//
//	EDGE* pXMeshEdge = NULL;
//	api_curve_line(SPAposition(0,0,0),SPAposition(10.0,10.0,10.0), pXMeshEdge);
//	oListBody.add(pXMeshEdge);
//
//}