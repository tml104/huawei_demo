#pragma once
#include <iostream>
#include "Frame.h"
#include "MyDefines.h"
#include "optframe.h"
#include "MyHeap.h"
#include <hash_map>

class surr_ds
{
public:
	std::hash_map<int, int> face_dr;

	void cal_dr(OmVeH mesh_v, OP_Mesh *pr_mesh, Cross_Field *cf);
};

class BCtools
{
private:
	OP_Mesh *pr_mesh;
	Cross_Field *cf;
	surr_ds *v_dr;
public:
	

	BCtools(OP_Mesh *mesh, Cross_Field *crossfield);
	//返回与s相邻的面fa的ds
	int ds(OmVeH s, OmFaH fa);
};

class CoveringNode
{
private:
public:
	int meshIdx;
	VECTOR orth;//顶点的法向量
	int edge_begin;
	int edge_end;
	//Fd表示顶点所在的层，以其在mesh上对应点的第一条出边所在平面为参考平面
	int layer;
	//顶点周围的面上的方向

	CoveringNode();
	CoveringNode(int _meshIdx, int _Fd);
};

class CoveringEdge
{
private:
public:
	//在mesh上对应半边的标号
	size_t meshIdx;
	int next;
	int to_vertex;
	int from_vertex;
	//对边两侧平面的方向插值后得到该边的方向向量Fd
	VECTOR Fd;
	//
	VECTOR e;

	CoveringEdge();
};

class CoveringSurface
{
private:
protected:
	
public:
	OP_Mesh *pr_mesh;
	Cross_Field *cf;
	bool *singularities;
	std::vector<CoveringNode> vertices;
	std::vector<CoveringEdge> edges;
	BCtools *bct;

	CoveringSurface(OP_Mesh *mesh, Cross_Field *crossfield, std::vector<OmVeH> s);

	int add_edge(int from, int to, OmHaEgH hfe);
};

/*
	顶点的代表面是其第一条出边属于的面
*/
class M4 : public CoveringSurface
{
private:
	
public:
	hoopsshow *hs;
	void setHoopsshow(hoopsshow *hs_);
	void show_edge(int eidx, int color);
	void show_edge(std::vector<int> &s, int color);
	void show_vertex(int idx, int color);
	hoopsshow* getHS();

	//给出Mesh上顶点的标号和层数，返回M4上点的标号
	int mesh2covering(int idx, int layer);
	//给出M4上点的标号，返回Mesh上顶点的标号
	int covering2mesh(int idx);

	//构造函数 参数为 网格指针， 向量场指针， 奇点的集合
	M4(OP_Mesh *mesh, Cross_Field *crossfield, std::vector<OmVeH> s) ;

	//M4的点v1，其在mesh上的映射点为hfe的出点，返回v2的层数；v2对应于hfe的入点且与v1在M4的同一层
	int find_layer(OmHaEgH hfe, int layer);

	//给出面的handle和层数，返回该层的direction向量
	VECTOR face_ds(OmFaH fa, int layer);

	//对给定的顶点及层数，返回相应的代表面上的向量
	VECTOR vertex_ds(OmVeH ve, int layer);

	//对给定的半边和方向，求插值后得到的Fd
	VECTOR edge_Fd(OmHaEgH hfe, int r);

	//给出M4上顶点的个数
	int n_vertices();

	//给出点在同一M2上的另一点的标号
	int reverse_node(int idx);

	//给出m4的边标号，返回对应的mesh上的边的标号
	int edge_idx(int eidx);

	CoveringNode& operator[](int i);
	const CoveringNode& operator[](int i) const;

	//构造M4
	void construct();
	
	//给出标号为idx的点的vector
	VECTOR point(int idx);

	//测试用
	void test_bfs(OmVeH v);
	void test();
};

/*
dijkstraOnCS
dijkstra shortest path on covering surfaces
基于M4的最短路算法
在构造指数映射图P时需要
*/

class dijkstraOnCS
{
private:
	int maxv;
	int maxe;
	CoveringSurface *cs;
	
public:
	double length[max_edge];
	int e_records[max_vertice];
	int v_records[max_vertice];
	int dk[max_vertice];
	double list[max_vertice];

	//构造函数
	dijkstraOnCS(CoveringSurface *branchedcovering);

	//各数组清零
	void clear();

	//找出最短路，并返回与原点start_v离散最短路不超过threshold的点的集合k_vert，形式为M4上的标号
	void dijkstra(int start_v, int threshold, std::vector<int> &k_vert);
};