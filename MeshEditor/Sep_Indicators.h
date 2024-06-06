#include "Frame.h"
#include "optframe.h"
#include <iostream>
#include"MyHeap.h"
#include "MyDefines.h"
#include <exception>

class SIpath
{
private:
	std::set<int> vertices;
	//size_t是OmEgH的标号
	std::set<size_t> left;
	std::set<size_t> right;
public:

	bool cut;
	//static const int maxsi = 120000;
	std::vector<OmHaEgH> route;
	

	SIpath();
	void getSI(OmVeH des, dijkPath &dp);
	bool in_si(int idx);
	OmHaEgH mid_edge();
	void clear();
	void push_back(OmHaEgH &he_handle);
	int size();
	void insert(int x);
	void erase(int x, int y);
	std::set<int>::iterator begin();
	std::set<int>::iterator end();
	void getWhiskers(OP_Mesh *mesh);
	bool left_cross(int eidx);
	bool right_cross(int eidx);
	//输入边的标号,若是一条左侧的边则返回-1， 若是右侧的边则返回1， 不相交则返回0
	int side(int eidx);
	bool whiskers_exist();
};

class Sep_Indicators
{
private:
	std::vector<OmEgH> test_e;
	OmEgH test_wrong;

	OP_Mesh *pr_mesh;
	hoopsshow *hs;
	
	double *edge_length;


	void showloop(OmEgH e, dijkPath &a, dijkPath &b, int color);
	void find_SI(OmVeH singu_a, OmVeH singu_b);
	void cal_sigma_e(double sigma_e[],dijkPath &bx,  int in_T[]);
	void cal_sigma_e(double sigma_e[],dijkPath &a, dijkPath &b, int in_T[]);
	void get_T_star(double sigma_e[], int in_T[], int T_star[]);
public:
	std::vector<SIpath> sp;
	double* edgeLength();

	//
	void show_SIpath(SIpath &path, int color);
	void show_SIpath(int sii, int color);
	void show_whisker(int sii);

	Sep_Indicators(OP_Mesh *bs_mesh, HoopsView *hoops_view);
	
	void add_SI(OmVeH start_vertex, OmVeH estination_vertex);
	void get_SIs(std::vector<OmVeH> &singularities);
	void get_whiskers();
};