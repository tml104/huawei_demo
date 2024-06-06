//#pragma once
//
////QT类
//#include <qstring.h>
////数据结构类
//#include <vector>
//#include <list>
//#include <algorithm>
//#include <hash_map>
//#include <queue>
//#include <iostream>
//#include <fstream>
//#include <sstream>
//#include <string>
//using namespace std;
////几何信息类
//#include <boolapi.hxx>
//#include <curextnd.hxx>
//#include <cstrapi.hxx>
//#include <face.hxx>
//#include <lump.hxx>
//#include <kernapi.hxx>
//
////私有类
//#include "hoopsview.h"
//
//struct Keypoint{
//	double dCoord[3];
//	int iType; //顶点-0，边中点-1，面中点-2，体中点-3
//};
//struct Keyface{
//	vector<int> vecKpNum; //note down the number of keypoints
//};
//struct KeyfaceSet{
//	vector<Keyface> vecKf; //some keyfaces form a keyface set. for instance, the face type is triangluar, and the face number is 2, all this kind of keyfaceSet form a L23  
//};
//
//struct Mypoint{
//	double dCoord[3];
//};
//struct Mypolygon{
//	vector<Mypoint> vecPtSet;
//};
//struct MicroPLS{
//	vector<Mypolygon> vecPnSet;
//	string strName;
//};
//class ZSRCreatePLS
//{
//	//function
//public:
//	ZSRCreatePLS(void);
//	ZSRCreatePLS(const double &idCubelength);
//	~ZSRCreatePLS(void);
//public:
//	void GenerateCandidatePLS();
//
//private:
//	void CreatePrimaryPLS();
//	void OpteratePlaneSymmetryforKeyfaceSet(MicroPLS &ioMicroPLS);
//
//	void CreateKeypoints();
//	void SelectLogicalKeyFaces(vector<vector<int>>  &veciTKp, vector<vector<int>> &veciQKp);
//	bool IsLogicalFace(const int &iFaceType, double idKeypoints[][3]);
//	bool IsSatisfyProcessAngle(const vector<int> &ivecKfSet, const double &idProcessAngle);
//
//	void DetermineKeyFaces();
//	bool IsKeyFaceSetSatisfyConstraints(const vector<Keyface> &ivecKfs);
//	bool IsConnectedWithZaxis(const vector<Keyface> &ivecKfs);
//	bool HaveThreeInterfacesAndConnectedWithZaxis(const vector<Keyface> &ivecKfs);
//	bool AllKeyfacesJoined(const vector<Keyface> &ivecKfs);
//
//	bool IsSameKeyPoints(const vector<int> veciFKp,const vector<int> veciSKp);
//	bool JudgeNextAction(const int &n,const int &m,const vector<int> &ivecIndex);
//	void SelectMfromN(const int &n,const int &m,const vector<int> &ivecValue,vector<vector<int>> &vecCandiCase) ;
//
//	void  CreateReferenceHexahedrons(const double &idCubeLength, ENTITY_LIST &oListBody);
//	FACE * CreateConvexPolygon(double idPoint[][3], const int &iPointNum);
//
//	//data
//public:
//	//input
//	double m_dCubeLength;
//	double m_dDensity;
//	int m_iKFaceNum;
//	int m_iKFaceType;
//	//medium
//	Keypoint m_kpSet[27];
//	vector<vector<int>> m_vecTKp;
//	vector<vector<int>> m_vecQKp;
//
//	vector<KeyfaceSet> m_vecKfSet23;
//	vector<KeyfaceSet> m_vecKfSet33;
//	vector<KeyfaceSet> m_vecKfSet24;
//	vector<KeyfaceSet> m_vecKfSet34;
//	//result
//
//	//output
//	vector<MicroPLS> m_vecMicroPLS;
//
//private:
//	HoopsView *hoopsview;
//
//};
//
