#ifndef LS_ASSEMBLY_H
#define LS_ASSEMBLY_H
#include "StdAfx.h"
#include "Frame.h"
#include "topologyoptwidget.h"
#include "Frame.h"
#include "Eigen/SparseQR"
#include "Eigen/IterativeLinearSolvers"
//#include "Eigen/SPQRSupport"
#include "Eigen/OrderingMethods"
#include <fstream>
#include <ostream>
#include <sstream>
#include <set>
using namespace OpenVolumeMesh;
typedef Eigen::SparseMatrix<double> SpMat; 
typedef Eigen::Triplet<double> T;
typedef Eigen::COLAMDOrdering<int> CMOrd;
typedef double CO_Ma[24][12];
using namespace Eigen;
void initial_size(VolumeMesh *mesh);
//����24��24�������
class M24
{
public:
	double* m;//��Ϊ��һ������ľ������Բ�ȡ����36��Ԫ������ʾ����ʡ�ռ䣬ͬʱ
	//Ҳ��֤ʵ�ֵ������������±�0��2��5��9��14��20��27��35�ǶԽ�����Ԫ��
	//û���κβ����ĳ�ʼ���������ڿ��ٿռ䣬����ÿһ��ֵ��Ϊ0
	M24();
	//���ٿռ䣬���ҽ��ΪA^T*A�Ľ��
	M24(double A[3][8]);

	M24(double xi,double et,double ze);//�����������������\xi,\eta,\zeta

	M24(const M24& M);
	//���������������ͷ��ڴ�
	~M24();
	//���巶�����޲������뼴Ϊ2������������ΪFrobenius�����������������1��0��0�����������
	double norm_M24(int p = 2);
	//������룬���ú��ַ��������ڶ���������û�еڶ���������Ϊ�����
	double dist_norm(M24& M,int p = 0);

	//��������ص��ں�,���źͳ��Ժ�
	M24& operator=(const M24& M);

	friend M24 operator-(const M24& M1,const M24& M2);

	friend M24 operator+(const M24& M1,const M24& M2);

	M24 operator/(double detj);

	friend M24 operator*(const M24& M,double coe);

	friend M24 operator*(double coe,const M24& M);

	M24& operator+=(const M24& M);

	M24& operator/=(double detj);

	void print_M24();
};
struct cel_inf
{
	M24 subl;
	double** subb;
};
struct EP//��ֵ����Cell�Ľ������ݽṹ
{
	OvmVeH vh1,vh2;//EP��ʵ�������������˵����Ի�ϳ���������vh1��index�϶���vh2С
	double alph;//alpha���ڱ�ʾ���߻�ϵı��� ���յ�ֵ��vh1*alph+(1-alph)*vh2
	//EP();
	EP(const EP& ep);
	EP(OvmVeH v1,OvmVeH v2,double lembda);
	//bool operator <(const EP &ep1)const;//��Ҫ����һ���ļ������
	friend bool operator==(const EP& ep1,const EP& ep2);
};
struct EP_hash//EP ��hash value����
{
	std::size_t operator()(const EP& ep) const;
};
struct CV//cell�����֮��Ļ�ϵĽṹ��
{
	OvmCeH ch;//cell��handle
	int variant;//0 ��ʾu 1 ��ʾv 2��ʾw
	CV(const CV& cv);
	CV(OvmCeH c_h,int v);
	//bool operator<(const CV &cv1)const;
	friend bool operator==(const CV& cv1,const CV& cv2);
};
struct CV_hash//CV hash value
{
	std::size_t operator()(const CV& cv) const;
};
void get_initial_coe(double* phy_coord);
double** b_symbol_integration();
M24 Simpson_triple_numerical_alg(int segment,double eps1,double eps2);
void initial_cells_vertices(VolumeMesh *mesh,std::vector<std::hash_map<int,OvmVeH>> &cells_index_vertices,std::vector<std::hash_map<OvmVeH,int>> &cells_vertices_index);
void initial_inf(VolumeMesh* mesh,std::vector<std::hash_map<int,OvmVeH>> &cells_index_vertices,std::hash_map<OvmCeH,cel_inf>& pre_inf);
void volume_LS_assembly(VolumeMesh *mesh,std::vector<Frame> &frames,double *f);
void volume_LS_assembly_sp(VolumeMesh *mesh,std::vector<Frame> &frames,double *f);
void iso_face_extraction(VolumeMesh *mesh,std::vector<Frame> &frames,std::unordered_set<EP,EP_hash> &iso_f_points,double *f);
void iso_face_extraction_general_ver(VolumeMesh *mesh,std::vector<Frame> &frames,std::unordered_set<EP,EP_hash> &iso_f_points,double *f,int cell_index,int edge_index,int para);
void iso_face_extraction_ind(VolumeMesh *mesh,std::vector<Frame> &frames,std::unordered_set<EP,EP_hash> &iso_f_points,std::unordered_set<CV,CV_hash> &seeked_cells,EP start_p,CV target_cell,std::vector<std::hash_map<OvmVeH,int>> &cells_vertices_index,double *f);
void read_f(VolumeMesh *mesh,double* f);
#endif