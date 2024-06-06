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
//定义24×24矩阵的类
class M24
{
public:
	double* m;//因为是一个特殊的矩阵，所以采取仅用36个元素来表示，节省空间，同时
	//也保证实现的清晰，其中下标0，2，5，9，14，20，27，35是对角线上元素
	//没有任何参数的初始化，仅用于开辟空间，并将每一个值赋为0
	M24();
	//开辟空间，并且结果为A^T*A的结果
	M24(double A[3][8]);

	M24(double xi,double et,double ze);//定义矩阵函数，参数是\xi,\eta,\zeta

	M24(const M24& M);
	//析构函数，用于释放内存
	~M24();
	//定义范数，无参数输入即为2范数，可以视为Frobenius范数，其余可以输入1，0（0代表无穷范数）
	double norm_M24(int p = 2);
	//定义距离，采用何种范数，看第二个参数，没有第二个输入视为无穷范数
	double dist_norm(M24& M,int p = 0);

	//运算符重载等于号,减号和除以号
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
struct EP//等值面与Cell的交点数据结构
{
	OvmVeH vh1,vh2;//EP其实是用网格两个端点线性混合出来，其中vh1的index肯定比vh2小
	double alph;//alpha用于表示两者混合的比例 最终的值是vh1*alph+(1-alph)*vh2
	//EP();
	EP(const EP& ep);
	EP(OvmVeH v1,OvmVeH v2,double lembda);
	//bool operator <(const EP &ep1)const;//需要容许一定的计算误差
	friend bool operator==(const EP& ep1,const EP& ep2);
};
struct EP_hash//EP 的hash value函数
{
	std::size_t operator()(const EP& ep) const;
};
struct CV//cell与变量之间的混合的结构体
{
	OvmCeH ch;//cell的handle
	int variant;//0 表示u 1 表示v 2表示w
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