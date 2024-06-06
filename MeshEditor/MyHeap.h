#pragma once
#include <iostream>
#include "Frame.h"
#include "optframe.h"
#include "MyDefines.h"
#include<stack>

class Con_set
{
private:
	std::vector<int> _set;
	int _maxn;
public:
	Con_set(int maxn);
	int idx(int x);
	void connect(int x, int y);
};

class meshtools
{
private:
public:
	static double distance(OmHaEgH hf, OP_Mesh *pr_mesh);
	static VECTOR getVector(OmHaEgH hf, OP_Mesh *pr_mesh);
	static void standalize(VECTOR &v);
	static double scalar(VECTOR &a, VECTOR &b);
	static double length(VECTOR &a);
	//
	static VECTOR unitize(VECTOR &x);
	static VECTOR mul(VECTOR &x, double k);
	//a x b
	static VECTOR tensor_production(VECTOR &a, VECTOR &b);
	//����������ʱ����ת
	static VECTOR rot_x(VECTOR &v, double theta);
	static VECTOR rot_y(VECTOR &v, double theta);
	static VECTOR rot_z(VECTOR &v, double theta);
	static double acos(VECTOR &a, VECTOR &b);
	//��b��ת��a, ret��bһ����ת������0��ʾ�ɹ�
	static VECTOR rotation(VECTOR a, VECTOR b, VECTOR tar);
	//��norm��ʱ����ת
	static VECTOR rotation(VECTOR norm, double theta, VECTOR tar);
	static VECTOR project(VECTOR &norm, VECTOR &obj);

	//����C_��
	static double w_e(VECTOR e, VECTOR Fd_e, double c_alpha = C_alpha);
	static double w_e(VECTOR norm, VECTOR e, VECTOR Fd_e, double c_alpha = C_alpha);

	//��a��b��࣬�򷵻�-1���Ҳ෵��1
	static int side(VECTOR a, VECTOR b, VECTOR norm);
	//�����������෴����
	static VECTOR rev(VECTOR x);
};

class hoopsshow
{
private:
	HoopsView *my_hoops_view;
	OP_Mesh *pr_mesh;
public:
	hoopsshow(OP_Mesh *mesh,  HoopsView *hoops_view_);
	void show_dot(VECTOR p, int color);
	void show_edge(OmHaEgH hf_handle, int color);
	void show_edge(OmEgH e_handle, int color);
	void show_edges(std::vector<OmHaEgH> es, int color);
	void show_path(std::vector<OmHaEgH> &path, int color);
	void show_vector(VECTOR a, VECTOR b, int color);
	void show_face(OmFaH fa, VECTOR ds, int color);
	void show_edge_Fd(OmHaEgH hfe, VECTOR Fd, int color);
};

/**/
class HeapNode
{
public:
		int idx;
		double value;
};

//Ĭ����С��
class baseHeap
{
protected:
	static const int max_n = 400001;
	HeapNode arr[max_n];
	int num;
public:
	baseHeap();
	void swap(HeapNode &x, HeapNode &y);
	virtual bool cmp(HeapNode &x, HeapNode &y);
	void push(int idx, double value);
	int pop();
	int size();
	void clear();
};

//����
class maxHeap : public baseHeap
{
public:
	bool cmp(HeapNode &x, HeapNode &y);
	
};

class dijkPath
{
private:
	int _maxv;
	int _maxe;
	OP_Mesh *pr_mesh;
	hoopsshow *hs;
	//int _thrall;
public:
	OmVeH sv;
	double *list;
	int *dk;
	OmVeH *v_records;
	OmHaEgH *e_records;
	std::vector<int> direction;
	dijkPath(OmVeH source_vertex, OP_Mesh *mesh);
	~dijkPath();
	void dijkstra(double *edge_length);
	void dijkstra(double *edge_length, std::vector<OmVeH> &k_vert, bool *illegal, int thrall);
	void path(OmVeH des, std::vector<OmHaEgH> &route);
	void setHoopsshow(hoopsshow *hs_);
	void showPath(int color);
};

class Primal_Separatrice
{
private:
	std::stack<int> sta;
public:
	std::vector<OmHaEgH> edges;
	void add_edge(int eidx, OP_Mesh *mesh);
	//��ʼ��stack
	void stack_inital();
	//��һ����ߵ�idx��ջ
	void stack_push(int eidx);
	//��һ��ջ�еı���ջ
	void stack_push(std::stack<int> &arg);
	//��ջ�еı߼���edges
	void stack2edges(OP_Mesh *mesh);
	//��ͼ����
	void show_path(hoopsshow *hs, int color);
};

class promoteLabel
{
public:
	int dualloop_idx;
	int flag;
	void set(int _dualloop_idx, int _flag);
	promoteLabel();
	promoteLabel(int idx);
	bool operator<(const promoteLabel &y) const;
};
typedef std::set<promoteLabel> Labels; 

class pHeapNode
{
public:
	int idx;
	double value;
	std::set<promoteLabel> flags;
	pHeapNode();
};

class promoteHeap
{
protected:
	static const int max_n = 400001;
	pHeapNode arr[max_n];
	int num;
public:
	promoteHeap();
	void swap(pHeapNode &x, pHeapNode &y);
	virtual bool cmp(pHeapNode &x, pHeapNode &y);
	void push(int idx, double value, Labels flags);
	pHeapNode pop();
	int size();
	void clear();
};