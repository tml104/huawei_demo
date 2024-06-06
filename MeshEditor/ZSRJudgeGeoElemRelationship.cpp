#include "StdAfx.h"
//#include "ZSRJudgeGeoElemRelationship.h"
//
//#include<math.h>
//
////ZSRJudeGeoElemRelationship::ZSRJudeGeoElemRelationship(void)
////{
////}
////
////
////ZSRJudeGeoElemRelationship::~ZSRJudeGeoElemRelationship(void)
////{
////}
//bool ZSRIsIntersectedByTwoLine(double idFirLine[2][3], double idSecLine[2][3])
//{
//	//caxcd=a, cbxcd=b, ab<=0
//
//	double dVect[3][3], dA[3],dB[3];
//
//	dVect[0][0]=idFirLine[0][0]-idSecLine[0][0];
//	dVect[0][1]=idFirLine[0][1]-idSecLine[0][1];
//	dVect[0][2]=idFirLine[0][2]-idSecLine[0][2];
//
//	dVect[1][0]=idFirLine[0][0]-idSecLine[1][0];
//	dVect[1][1]=idFirLine[0][1]-idSecLine[1][1];
//	dVect[1][2]=idFirLine[0][2]-idSecLine[1][2];
//
//	dVect[2][0]=idFirLine[0][0]-idFirLine[1][0];
//	dVect[2][1]=idFirLine[0][1]-idFirLine[1][1];
//	dVect[2][2]=idFirLine[0][2]-idFirLine[1][2];
//
//	ZSRCrossProduct(dVect[0],dVect[1],dA);
//	ZSRCrossProduct(dVect[0],dVect[2],dB);
//
//	if(ZSRDotProduct(dA,dB)<=0) return false;
//
//	return true;
//}
//
//bool ZSRIsPointsInSameSupportedLine(double iPointSet[][3],const int &iPointNum)
//{
//	if(iPointNum<1) return false;
//
//	bool bRet=false;
//	if(iPointNum==3)
//	{
//		double fk, sk, tk;
//		fk=(iPointSet[1][0]-iPointSet[0][0])*(iPointSet[2][1]-iPointSet[0][1])-(iPointSet[2][0]-iPointSet[0][0])*(iPointSet[1][1]-iPointSet[0][1]);
//		sk=(iPointSet[1][0]-iPointSet[0][0])*(iPointSet[2][2]-iPointSet[0][2])-(iPointSet[2][0]-iPointSet[0][0])*(iPointSet[1][2]-iPointSet[0][2]);
//		tk=(iPointSet[1][2]-iPointSet[0][2])*(iPointSet[2][1]-iPointSet[0][1])-(iPointSet[2][2]-iPointSet[0][2])*(iPointSet[1][1]-iPointSet[0][1]);
//
//		//if three points in the same line, then they can't form a keyface.
//		if((fabs(fk)<0.01) && (fabs(sk)<0.01)&& (fabs(tk)<0.01))
//			bRet = true;
//	}
//	else if(iPointNum==4)
//	{
//		bRet = false;
//	}
//
//	return bRet;
//}
//
//bool ZSRIsPointsInSameSupportedFace(double iPointSet[][3],const int &iPointNum)
//{
//	if(iPointNum<1) return false;
//	bool bRet=false;
//
//	if(iPointNum==4)
//	{
//		double dA[3][3];
//		dA[0][0] = iPointSet[1][0]-iPointSet[0][0];
//		dA[0][1] = iPointSet[1][1]-iPointSet[0][1];
//		dA[0][2] = iPointSet[1][2]-iPointSet[0][2];
//		dA[1][0] = iPointSet[2][0]-iPointSet[0][0];
//		dA[1][1] = iPointSet[2][1]-iPointSet[0][1];
//		dA[1][2] = iPointSet[2][2]-iPointSet[0][2];
//		dA[2][0] = iPointSet[3][0]-iPointSet[0][0];
//		dA[2][1] = iPointSet[3][1]-iPointSet[0][1];
//		dA[2][2] = iPointSet[3][2]-iPointSet[0][2];
//
//		double dDotRet=dA[0][0]*(dA[1][1]*dA[2][2]-dA[1][2]*dA[2][1])-dA[0][1]*(dA[1][0]*dA[2][2]-dA[1][2]*dA[2][0])+dA[0][2]*(dA[1][0]*dA[2][1]-dA[1][1]*dA[2][0]);
//
//		if(fabs(dDotRet-0)<0.0001) bRet=true;
//	}
//	else if(iPointNum>4)
//	{
//		bRet=false;
//	}
//	else
//		bRet=true;
//
//	return bRet;
//}
//
//bool ZSRIsPolygonsIntersect(double idFirPolygon[][3],const int &iFPn,double idSecPolygon[][3],const int &iSPn)
//{
//	double dFirLine[2][3], dSecLine[2][3];
//
//	for(int i=0;i<iFPn;i++)
//	{
//		if(i!=iFPn-1) 
//			for(int k=0;k<3;k++) dFirLine[0][k]=idFirPolygon[i][k], dFirLine[1][k]=idFirPolygon[i+1][k];
//		else
//			for(int k=0;k<3;k++) dFirLine[0][k]=idFirPolygon[i][k], dFirLine[1][k]=idFirPolygon[0][k];
//
//		for(int j=0;j<iSPn;j++)
//		{
//			if(i!=iSPn-1) 
//				for(int k=0;k<3;k++) dSecLine[0][k]=idSecPolygon[i][k], dSecLine[1][k]=idSecPolygon[i+1][k];
//			else
//				for(int k=0;k<3;k++) dSecLine[0][k]=idSecPolygon[i][k], dSecLine[1][k]=idSecPolygon[0][k];
//
//			if(ZSRIsIntersectedByTwoLine(dFirLine,dSecLine)) return true;
//		}
//	}
//	
//	return false;
//}
//
//double ZSRDotProduct(double idFVect[3],double idSVect[3])
//{
//	return idFVect[0]*idSVect[0]+idFVect[1]*idSVect[1]+idFVect[2]*idSVect[2];
//}
//
//void ZSRCrossProduct(double idFVect[3],double idSVect[3],double *opRVect)
//{
//	opRVect[0]=idFVect[1]*idSVect[2]-idFVect[2]*idSVect[1];
//	opRVect[1]=idFVect[2]*idSVect[0]-idFVect[0]*idSVect[2];
//	opRVect[2]=idFVect[0]*idSVect[1]-idFVect[1]*idSVect[0];
//}