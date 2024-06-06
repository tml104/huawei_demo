#pragma once
#include <iostream>
#include "Frame.h"
#include "MyDefines.h"
#include "optframe.h"
#include "MyHeap.h"
#include "BranchedCovering.h"

class P_edge
{
private:
	int _idx;
public:
	int idx();
	void set_idx(int idx);
	//这3个在add_edge中初始化
	int to_vertice;
	int from_vertice;
	int next;

	//边的权重
	double weight;

	//是否满足θcosntraint
	bool theta_con;
	
	//在指数映射平面上的坐标向量
	VECTOR e;
	VECTOR Fd;
	P_edge();
	//记录m4上对应边的标号
	std::vector<int> support;

	//返回e和Fd的夹角余弦
	double cos_theta();
};

class P_node
{
private:
	int _idx;
public:
	int Fd;
	int edge_begin;//edge begin
	int edge_end;//edge end
	VECTOR orth;
	P_node();

	//idx表示该点在M4上对应点的标号
	int idx();
	void set_idx(int idx);
};



//对给定的一个曲面，返回指数映射的图
class ExpMap
{
private:
	static const int maxv = 1000000;
	static const int maxe = 2000000;
	double eps;
	int _degree;
	
	hoopsshow *hs;
	//
	int mesh2P[maxv];
	//m4上的边在P上的对应边
	int edge2P[maxe];

	//判断e是否满足θ-constraint
	bool theta_constraint(P_edge &e);
public:
	OP_Mesh *mesh;
	M4 *m4;
	std::vector<P_node> nodes;
	std::vector<P_edge> edges;

	//绘图相关函数
	void setHoopsshow(hoopsshow *hoops);
	void show_edge(int eidx, int color);

	ExpMap(M4 *m4, int degree);

	//加入一条p1->p2的边，返回该边的idx
	int add_edge(P_node &p1, P_node &p2);

	//构造1-ring
	void construct_1ring();

	//构造k-ring
	void construct_kring();
	
	//构造指数映射图
	void construct(); 

	//返回eidx的support所对应的mesh上的半边的集合
	void getMeshHaEg(int eidx, std::vector<OmHaEgH> &ret);

	//倒序返回support上边对应的半边的标号
	void getHaEgStack(int eidx, std::stack<int> &ret);

	void test_e(int v);
};

class DualLoop
{
private:
	/*数据区*/
	std::vector<size_t> mesh_halfedges;
	std::vector<int> M4_edges;
	
	std::set<int> M4_vertices;
	std::set<int> crossed_si;

	//whiskers
	std::set<size_t> left;
	std::set<size_t> right;
	/*函数区*/
public:
	/*数据区*/
	double length;
	//测试用
	std::vector<int> P_edges;

	/*函数区*/
	DualLoop();
	//将一条M4边加入dualloop
	void add_path(int eidx, M4 *m4);
	//将推进图P上的一条边加入dualloop中
	void add_path(int eidx, ExpMap *P);
	//将一个M4上点加入dualloop
	void add_vertex(int vidx);

	//获得迭代器的函数
	std::vector<int>::iterator M4_edge_begin();
	std::vector<int>::iterator M4_edge_end();
	std::vector<size_t>::iterator mesh_begin();
	std::vector<size_t>::iterator mesh_end();
	std::set<int>::iterator M4_vertex_begin();
	std::set<int>::iterator M4_vertex_end();
	std::set<int>::iterator si_begin();
	std::set<int>::iterator si_end();
	std::vector<OmHaEgH> getHalfedgesHandle(OP_Mesh *mesh);

	//
	bool M4_vertex_cross(int vidx);

	//判断vidx是否与dual loops在M2上相交，vidx在M4上
	bool M2_vertex_cross(int vidx);

	//
	bool M2_edge_cross(int eidx, ExpMap *P);

	//
	int size();

	//
	bool crossed(int si_idx);

	void cross(int si_idx);

	//
	bool in_left(size_t idx);
	bool in_right(size_t idx);
	bool whiskers_exist();

	//比较dualloop的长度
	bool operator<(DualLoop &y);

	//
	void getWhiskers(OP_Mesh *mesh);

	//将dualloop写入到文件
	void write_to_file(std::ofstream &fout);
	//从文件中读取dualloop
	void read_from_file(std::ifstream &fin, M4 *m4);
	void read_from_file(std::ifstream &fin, ExpMap *P);

	//绘图函数
	void show_whiskers(hoopsshow *hs, OP_Mesh *mesh);
};

class Propagation
{
private:
	ExpMap *P;
	//baseHeap heap;
	promoteHeap heap;
	int maxv;
	int maxe;

	bool *inS;
	int *v_records;
	int *e_records;
	double *list;

	//清空数组
	void clear();
public:
	//推进起始点
	int source_vertex;
	
	std::vector<DualLoop> existingLoops;

	Propagation(ExpMap *expmap);

	//绘图函数
	void show_dualloop(DualLoop &loop, int color);
	void show_existingLoops(int color);
	
	//初始化
	void init(int sv);

	//若找到的loop不合法，则需重置
	void reset();

	//返回最短loop的长度
	double promote();

	//将feature curves作为existingLoops的一部分
	double promote(std::vector<DualLoop> &featureCurves);

	//返回loop
	DualLoop getLoop();

	//判断P上一条边pedge是否与给定的一条dual loop M2相交,1表示mesh上相交而M2不相交，0表示mesh上不相交，-1表示M2相交
	int Pedge_M2_cross(P_edge &pedge, DualLoop &loop);
	//判断P上一条边pedge是否与dual loops M2相交,返回M2不相交但mesh上相交的dualloop的标号+1，不与所有dualloop相交返回0
	//与某条dualloop M2相交返回-1
	int Pedge_M2_cross(P_edge &pedge);
	int Pedge_M2_cross(P_edge &pedge, Labels &flags);
	int Pedge_M2_cross(P_edge &pedge, std::vector<DualLoop> &featureCurves);
	//判断一个点在是否在M2意义上在existingLoops上
	bool M4vertex_loops_cross(int vidx);
	//返回-1表示该条P边不合法；1表示该条P边合法。在dualloop左侧flag置1，在右侧flag置-1，否则置0
	int get_flags(P_edge &pedge, DualLoop &loop, int &flag);
	//
	void add2existingloops(DualLoop &s);
	//从文件中读取existingloops的信息
	void read_from_file(std::string filename);
	//将existingloops的信息写入文件
	void write_to_file(std::string filename);
	//判断两条loop是否是M2不相交
	bool loop_cross_type(int loop1_idx, int loop2_idx);
	//判断一条dualloop是否是冗余的
	bool dualloop_is_redundant(int dualloop_idx);
};