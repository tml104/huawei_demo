#pragma once
#include <iostream>
#include "Frame.h"
#include "optframe.h"
#include "MyDefines.h"
#include "Sep_Indicators.h"
#include "DualLoop.h"

typedef std::vector<OmEgH> OmDualloop;
typedef  std::vector<OmDualloop> Set_of_OmDualloop;

class LoopConstruct
{
private:
	//Mesh
	OP_Mesh *mesh;
	//Cross field
	Cross_Field *cf;
	HoopsView *hoopsview;
	//绘图类
	hoopsshow *hosh;
	//M4
	M4 *m4;
	//
	ExpMap *P;
	//separation indicators
	Sep_Indicators *SIfinder;
	//推进类
	Propagation *candidate;
	//奇点的集合
	std::vector<OmVeH> singularities;
	//判断一个点是否是奇点
	bool *is_singularity;
	bool *isolated;
	//判断一条边是否在loops上
	bool *onLoops;
	//记录网格边的长度
	double *edge_length;
	
	/*
		测试变量
	*/
	int global_cnt;


	/**/
	//记录边所在区块的奇点的标号
	int *edge_block;
	//判断一个点是否是奇点的相邻点
	int *around_singularity;
	//记录已经经过还原处理的奇点
	std::set<int> dealt_s;
	//记录得到的dualloop
	std::vector<DualLoop> dualloops;
	//保存特征线
	std::vector<DualLoop> featureCurves;
	std::vector<Primal_Separatrice> separatrice;
	//
	bool primal_Pedge_constraint(int eidx, int s_mesh, int last_block, int &block);
	void primal_flood(OmVeH sv);
	void primal_promote(int sv);
	//
	double halfedge_length(int hf_idx);

	//
	bool loop_SI_cross_simple(DualLoop &loop, int si_idx, bool record = false);
	bool loop_SI_cross_whiskers(DualLoop &loop, int si_idx, bool record = false);
	
public:
	//存放dual loops每条loop的对应edge_handle的vector
	//std::vector<std::vector<OmEgH>> dualloops_ehs;
	//void get_dual_loops_ehs();

	void show_loop(DualLoop &ss);
	//LoopConstruct构造函数
	LoopConstruct(OP_Mesh *bs_mesh, Cross_Field* croos_field, HoopsView *hs);
	
	void embedding();
	void embedding(DualLoop &loop);
	bool bfs(OmVeH vs);
	bool post_validation();

	/**/

	//
	void Primalization();

	//
	void process();

	//Feature Curve
private:
	DualLoop construct_feature_curve(std::set<size_t> feature_vhs);
	void construct_singularity2featureCurve_SI(int feature_curves_idx, int singularity_idx);
	bool findLoop_with_featureCurves();
	bool findLoop_with_featureCurves_simple();
	//
	int featureCurve_bfs(OmVeH sv, std::set<size_t> &feature_vhs);
	int featureCurves_bfs(std::set<size_t> &feature_vhs1, std::set<size_t> &feature_vhs2);
	//找到切割si_idx这条si的最短的loop
	DualLoop find_min_loop(int si_idx);
	//检查新加入的SI(si_start->si_end)是否被已经存在的dualloop切割
	void new_si_cut_check(int si_start, int si_end);
	void promote_with_featureCurves(bool show);
	
	
public:
	//返回用OmEgH表示的dual loops
	OmDualloop M4_loop_to_Om_loop(DualLoop &loop);
	Set_of_OmDualloop dualloops_OmEgH();
	//给定一个M4上的点，以其为起点构造dual loop
	DualLoop promote_from(int start_vertex);
	//入口函数
	void process(std::vector<std::set<size_t>> hard_feature_vhs, std::vector<std::vector<int>> singularity_pair_he_idx
		,std::set<std::pair<int, int>> hardedge_pairing);

	//给定一个mesh上的起点和一条dualloop的编号，返回过这个点与这条dualloop正交的dualloop
	OmDualloop find_loop(OmVeH start_vertex, int dualloop_idx);
	//给定一个mesh上的奇点和一个初始方向，返回过该点与该方向（最）相切的dual loop
	OmDualloop find_loop_along_direction(OmVeH start_vertex, VECTOR tangent_dir);

	OmDualloop swap_loop(OmVeH start_vertex, int dualloop_idx, int target_idx);
	//从文件中读取existingloops的信息
	void read_from_file(std::string filename, std::vector<std::set<size_t>> hard_feature_vhs);
	//将existingloops的信息写入文件
	void write_to_file(std::string filename);

	//判断两条loop是否是M2不相交
	bool loop_cross_type(int loop1_idx, int loop2_idx);
	//判断一条dualloop是否是冗余的
	bool dualloop_is_redundant(int dualloop_idx, bool *removed);
	//移除冗余的dualloop
	void remove_redundant_loops(int n);
	//移除指定id的Loops
	void remove_specified_loops(std::unordered_set<int> sp_ids);
};