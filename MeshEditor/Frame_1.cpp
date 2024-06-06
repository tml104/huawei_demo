#include <StdAfx.h>
//#include "Frame.h"
//#include <cmath>
//#include "topologyoptwidget.h"
//
//#define PI 3.1415926
//
//Matrix_3 Frame::m()
//{
//	_Matrix_3 temp = {this->x_vec[0], this->x_vec[1], this->x_vec[2], this->y_vec[0], this->y_vec[1], this->y_vec[2], this->z_vec[0], this->z_vec[1], this->z_vec[2]};
//	return Matrix_3(temp);
//}
//
//Matrix_3::Matrix_3(double diagonal_value )
//{
//	for (int i = 0; i < 3; i++)
//	{
//		for (int j = 0; j < 3; j++)
//		{
//			this->M[i][j] = 0;
//		}
//	}
//	for (int i = 0; i < 3; i++)
//	{
//		this->M[i][i] = diagonal_value;
//	}
//}
//
//Matrix_3::Matrix_3(const _Matrix_3 m)
//{
//	for (int i = 0; i < 3; i++)
//	{
//		for (int j = 0; j < 3; j++)
//		{
//			M[i][j] = m[i][j];
//		}
//	}
//}
//
//double Matrix_3::det() const
//{
//	double det;
//	det = this->M[0][0]*(this->M[1][1]*this->M[2][2]-this->M[1][2]*this->M[2][1])-this->M[0][1]*(this->M[1][0]*this->M[2][2]-this->M[1][2]*this->M[2][0])+this->M[0][2]*(this->M[1][0]*this->M[2][1]-this->M[1][1]*M[2][0]);
//	return det;
//}
//
//Matrix_3 Matrix_3::inversion_M() const
//{
//	double det;
//	det = this->M[0][0]*(this->M[1][1]*this->M[2][2]-this->M[1][2]*this->M[2][1])-this->M[0][1]*(this->M[1][0]*this->M[2][2]-this->M[1][2]*this->M[2][0])+this->M[0][2]*(this->M[1][0]*this->M[2][1]-this->M[1][1]*M[2][0]);
//	if (det == 0)
//	{
//		std::cerr<<"Matrix can't reverse"<<std::endl;
//		return *this;
//	}
//	else
//	{
//		_Matrix_3 m;
//		m[0][0] = (M[1][1]*M[2][2]-M[1][2]*M[2][1])/det;
//		m[1][0] = (M[1][2]*M[2][0]-M[1][0]*M[2][2])/det;
//		m[2][0] = (M[1][0]*M[2][1]-M[1][1]*M[2][0])/det;
//		m[0][1] = (M[0][2]*M[2][1]-M[0][1]*M[2][2])/det;
//		m[1][1] = (M[0][0]*M[2][2]-M[0][2]*M[2][0])/det;
//		m[2][1] = (M[0][1]*M[2][0]-M[0][0]*M[2][1])/det;
//		m[0][2] = (M[0][1]*M[1][2]-M[1][1]*M[0][2])/det;
//		m[1][2] = (M[0][2]*M[1][0]-M[0][0]*M[1][2])/det;
//		m[2][2] = (M[0][0]*M[1][1]-M[0][1]*M[1][0])/det;
//		Matrix_3 result_M(m);
//		return result_M;
//	}
//}
//
//
//Matrix_3& Matrix_3::operator =(const Matrix_3 &_M)
//{
//	for (int i = 0; i < 3; i++)
//	{
//		for (int j = 0; j < 3; j++)
//		{
//			this->M[i][j] = _M.M[i][j];
//		}
//	}
//	return *this;
//}
//
//Matrix_3 Matrix_3::operator*(const Matrix_3 &_M)
//{
//	Matrix_3 result_M(0.0);
//	for (int i = 0; i < 3; i++)
//	{
//		for (int j = 0;j < 3;j++)
//		{
//			for (int s = 0;s < 3;s++)
//			{
//				result_M.M[i][j] += this->M[i][s]*_M.M[s][j];
//			}
//		}
//	}
//	return result_M;
//}
//
//
//Matrix_3 Matrix_3::operator/(const Matrix_3 &_M)
//{
//	Matrix_3 result_M(0.0);
//	Matrix_3 inversionM(0.0) ;
//	inversionM = _M.inversion_M();
//	result_M = *this*inversionM;
//	return result_M;
//}
//
//double distance_between_frames(Frame & f1, Frame & f2)
//{
//	double m1[3][3] = {f1.x_vec[0], f1.x_vec[1], f1.x_vec[2], f1.y_vec[0], f1.y_vec[1], f1.y_vec[2], f1.z_vec[0], f1.z_vec[1], f1.z_vec[2]};
//	double m2[3][3] = {f2.x_vec[0], f2.x_vec[1], f2.x_vec[2], f2.y_vec[0], f2.y_vec[1], f2.y_vec[2], f2.z_vec[0], f2.z_vec[1], f2.z_vec[2]};
//	Matrix_3 M1(m1), M2(m2);
//	Matrix_3 P = M1.inversion_M()*M2;
//	double dis(0);
//	for(int i = 0; i < 3; i++){
//		dis += P.M[i][0]*P.M[i][0]*P.M[i][1]*P.M[i][1];
//		dis += P.M[i][1]*P.M[i][1]*P.M[i][2]*P.M[i][2];
//		dis += P.M[i][2]*P.M[i][2]*P.M[i][0]*P.M[i][0];
//
//		//dis += P.M[0][i]*P.M[0][i]*P.M[1][i]*P.M[1][i];
//		//dis += P.M[1][i]*P.M[1][i]*P.M[2][i]*P.M[2][i];
//		//dis += P.M[2][i]*P.M[2][i]*P.M[0][i]*P.M[0][i];
//
//		//dis += (P.M[i][0]*P.M[i][0]+P.M[i][1]*P.M[i][1]+P.M[i][2]*P.M[i][2]-1)*(P.M[i][0]*P.M[i][0]+P.M[i][1]*P.M[i][1]+P.M[i][2]*P.M[i][2]-1);
//		//dis += (P.M[0][i]*P.M[0][i]+P.M[1][i]*P.M[1][i]+P.M[2][i]*P.M[2][i]-1)*(P.M[0][i]*P.M[0][i]+P.M[1][i]*P.M[1][i]+P.M[2][i]*P.M[2][i]-1);
//	}
//
//	//dis += (P.det()-1)*(P.det()-1);
//	return dis;
//}
//
//void Matrix2Euler(Matrix_3 & m, Euler_angle & euler)
//{
//	if(m.M[2][0] != 1 && m.M[2][0] != -1){
//		euler[1] = -asin(m.M[2][0]);
//		euler[0] = atan2(m.M[2][1]/cos(euler[1]), m.M[2][2]/cos(euler[1]));
//		euler[2] = atan2(m.M[1][0]/cos(euler[1]), m.M[0][0]/cos(euler[1]));
//	}
//	else{
//		euler[2] = 0;
//		if(m.M[2][0] == -1){
//			euler[1] = PI/2;
//			euler[0] = euler[2]+atan2(m.M[0][1], m.M[0][2]);
//		}
//		else{
//			euler[1] = -PI/2;
//			euler[0] = -euler[2]+atan2(-m.M[0][1], -m.M[0][2]);
//		}
//	}
//}
//
//void Euler2Matrix(Euler_angle & euler, Matrix_3 & M)
//{
//	double a(euler[0]), b(euler[1]), c(euler[2]);
//	_Matrix_3 m = {cos(b)*cos(c), sin(a)*sin(b)*cos(c)-cos(a)*sin(c), cos(a)*sin(b)*cos(c)+sin(a)*sin(c),
//	               cos(b)*sin(c), sin(a)*sin(b)*sin(c)+cos(a)*cos(c), cos(a)*sin(b)*sin(c)-sin(a)*cos(c),
//	               -sin(b), sin(a)*cos(b), cos(a)*cos(b)};
//	M = Matrix_3(m);
//}