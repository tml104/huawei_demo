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
	//��3����add_edge�г�ʼ��
	int to_vertice;
	int from_vertice;
	int next;

	//�ߵ�Ȩ��
	double weight;

	//�Ƿ������cosntraint
	bool theta_con;
	
	//��ָ��ӳ��ƽ���ϵ���������
	VECTOR e;
	VECTOR Fd;
	P_edge();
	//��¼m4�϶�Ӧ�ߵı��
	std::vector<int> support;

	//����e��Fd�ļн�����
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

	//idx��ʾ�õ���M4�϶�Ӧ��ı��
	int idx();
	void set_idx(int idx);
};



//�Ը�����һ�����棬����ָ��ӳ���ͼ
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
	//m4�ϵı���P�ϵĶ�Ӧ��
	int edge2P[maxe];

	//�ж�e�Ƿ������-constraint
	bool theta_constraint(P_edge &e);
public:
	OP_Mesh *mesh;
	M4 *m4;
	std::vector<P_node> nodes;
	std::vector<P_edge> edges;

	//��ͼ��غ���
	void setHoopsshow(hoopsshow *hoops);
	void show_edge(int eidx, int color);

	ExpMap(M4 *m4, int degree);

	//����һ��p1->p2�ıߣ����ظñߵ�idx
	int add_edge(P_node &p1, P_node &p2);

	//����1-ring
	void construct_1ring();

	//����k-ring
	void construct_kring();
	
	//����ָ��ӳ��ͼ
	void construct(); 

	//����eidx��support����Ӧ��mesh�ϵİ�ߵļ���
	void getMeshHaEg(int eidx, std::vector<OmHaEgH> &ret);

	//���򷵻�support�ϱ߶�Ӧ�İ�ߵı��
	void getHaEgStack(int eidx, std::stack<int> &ret);

	void test_e(int v);
};

class DualLoop
{
private:
	/*������*/
	std::vector<size_t> mesh_halfedges;
	std::vector<int> M4_edges;
	
	std::set<int> M4_vertices;
	std::set<int> crossed_si;

	//whiskers
	std::set<size_t> left;
	std::set<size_t> right;
	/*������*/
public:
	/*������*/
	double length;
	//������
	std::vector<int> P_edges;

	/*������*/
	DualLoop();
	//��һ��M4�߼���dualloop
	void add_path(int eidx, M4 *m4);
	//���ƽ�ͼP�ϵ�һ���߼���dualloop��
	void add_path(int eidx, ExpMap *P);
	//��һ��M4�ϵ����dualloop
	void add_vertex(int vidx);

	//��õ������ĺ���
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

	//�ж�vidx�Ƿ���dual loops��M2���ཻ��vidx��M4��
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

	//�Ƚ�dualloop�ĳ���
	bool operator<(DualLoop &y);

	//
	void getWhiskers(OP_Mesh *mesh);

	//��dualloopд�뵽�ļ�
	void write_to_file(std::ofstream &fout);
	//���ļ��ж�ȡdualloop
	void read_from_file(std::ifstream &fin, M4 *m4);
	void read_from_file(std::ifstream &fin, ExpMap *P);

	//��ͼ����
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

	//�������
	void clear();
public:
	//�ƽ���ʼ��
	int source_vertex;
	
	std::vector<DualLoop> existingLoops;

	Propagation(ExpMap *expmap);

	//��ͼ����
	void show_dualloop(DualLoop &loop, int color);
	void show_existingLoops(int color);
	
	//��ʼ��
	void init(int sv);

	//���ҵ���loop���Ϸ�����������
	void reset();

	//�������loop�ĳ���
	double promote();

	//��feature curves��ΪexistingLoops��һ����
	double promote(std::vector<DualLoop> &featureCurves);

	//����loop
	DualLoop getLoop();

	//�ж�P��һ����pedge�Ƿ��������һ��dual loop M2�ཻ,1��ʾmesh���ཻ��M2���ཻ��0��ʾmesh�ϲ��ཻ��-1��ʾM2�ཻ
	int Pedge_M2_cross(P_edge &pedge, DualLoop &loop);
	//�ж�P��һ����pedge�Ƿ���dual loops M2�ཻ,����M2���ཻ��mesh���ཻ��dualloop�ı��+1����������dualloop�ཻ����0
	//��ĳ��dualloop M2�ཻ����-1
	int Pedge_M2_cross(P_edge &pedge);
	int Pedge_M2_cross(P_edge &pedge, Labels &flags);
	int Pedge_M2_cross(P_edge &pedge, std::vector<DualLoop> &featureCurves);
	//�ж�һ�������Ƿ���M2��������existingLoops��
	bool M4vertex_loops_cross(int vidx);
	//����-1��ʾ����P�߲��Ϸ���1��ʾ����P�ߺϷ�����dualloop���flag��1�����Ҳ�flag��-1��������0
	int get_flags(P_edge &pedge, DualLoop &loop, int &flag);
	//
	void add2existingloops(DualLoop &s);
	//���ļ��ж�ȡexistingloops����Ϣ
	void read_from_file(std::string filename);
	//��existingloops����Ϣд���ļ�
	void write_to_file(std::string filename);
	//�ж�����loop�Ƿ���M2���ཻ
	bool loop_cross_type(int loop1_idx, int loop2_idx);
	//�ж�һ��dualloop�Ƿ��������
	bool dualloop_is_redundant(int dualloop_idx);
};