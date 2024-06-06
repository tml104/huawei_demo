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
	//������s���ڵ���fa��ds
	int ds(OmVeH s, OmFaH fa);
};

class CoveringNode
{
private:
public:
	int meshIdx;
	VECTOR orth;//����ķ�����
	int edge_begin;
	int edge_end;
	//Fd��ʾ�������ڵĲ㣬������mesh�϶�Ӧ��ĵ�һ����������ƽ��Ϊ�ο�ƽ��
	int layer;
	//������Χ�����ϵķ���

	CoveringNode();
	CoveringNode(int _meshIdx, int _Fd);
};

class CoveringEdge
{
private:
public:
	//��mesh�϶�Ӧ��ߵı��
	size_t meshIdx;
	int next;
	int to_vertex;
	int from_vertex;
	//�Ա�����ƽ��ķ����ֵ��õ��ñߵķ�������Fd
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
	����Ĵ����������һ���������ڵ���
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

	//����Mesh�϶���ı�źͲ���������M4�ϵ�ı��
	int mesh2covering(int idx, int layer);
	//����M4�ϵ�ı�ţ�����Mesh�϶���ı��
	int covering2mesh(int idx);

	//���캯�� ����Ϊ ����ָ�룬 ������ָ�룬 ���ļ���
	M4(OP_Mesh *mesh, Cross_Field *crossfield, std::vector<OmVeH> s) ;

	//M4�ĵ�v1������mesh�ϵ�ӳ���Ϊhfe�ĳ��㣬����v2�Ĳ�����v2��Ӧ��hfe���������v1��M4��ͬһ��
	int find_layer(OmHaEgH hfe, int layer);

	//�������handle�Ͳ��������ظò��direction����
	VECTOR face_ds(OmFaH fa, int layer);

	//�Ը����Ķ��㼰������������Ӧ�Ĵ������ϵ�����
	VECTOR vertex_ds(OmVeH ve, int layer);

	//�Ը����İ�ߺͷ������ֵ��õ���Fd
	VECTOR edge_Fd(OmHaEgH hfe, int r);

	//����M4�϶���ĸ���
	int n_vertices();

	//��������ͬһM2�ϵ���һ��ı��
	int reverse_node(int idx);

	//����m4�ı߱�ţ����ض�Ӧ��mesh�ϵıߵı��
	int edge_idx(int eidx);

	CoveringNode& operator[](int i);
	const CoveringNode& operator[](int i) const;

	//����M4
	void construct();
	
	//�������Ϊidx�ĵ��vector
	VECTOR point(int idx);

	//������
	void test_bfs(OmVeH v);
	void test();
};

/*
dijkstraOnCS
dijkstra shortest path on covering surfaces
����M4�����·�㷨
�ڹ���ָ��ӳ��ͼPʱ��Ҫ
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

	//���캯��
	dijkstraOnCS(CoveringSurface *branchedcovering);

	//����������
	void clear();

	//�ҳ����·����������ԭ��start_v��ɢ���·������threshold�ĵ�ļ���k_vert����ʽΪM4�ϵı��
	void dijkstra(int start_v, int threshold, std::vector<int> &k_vert);
};