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
	//��ͼ��
	hoopsshow *hosh;
	//M4
	M4 *m4;
	//
	ExpMap *P;
	//separation indicators
	Sep_Indicators *SIfinder;
	//�ƽ���
	Propagation *candidate;
	//���ļ���
	std::vector<OmVeH> singularities;
	//�ж�һ�����Ƿ������
	bool *is_singularity;
	bool *isolated;
	//�ж�һ�����Ƿ���loops��
	bool *onLoops;
	//��¼����ߵĳ���
	double *edge_length;
	
	/*
		���Ա���
	*/
	int global_cnt;


	/**/
	//��¼��������������ı��
	int *edge_block;
	//�ж�һ�����Ƿ����������ڵ�
	int *around_singularity;
	//��¼�Ѿ�������ԭ��������
	std::set<int> dealt_s;
	//��¼�õ���dualloop
	std::vector<DualLoop> dualloops;
	//����������
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
	//���dual loopsÿ��loop�Ķ�Ӧedge_handle��vector
	//std::vector<std::vector<OmEgH>> dualloops_ehs;
	//void get_dual_loops_ehs();

	void show_loop(DualLoop &ss);
	//LoopConstruct���캯��
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
	//�ҵ��и�si_idx����si����̵�loop
	DualLoop find_min_loop(int si_idx);
	//����¼����SI(si_start->si_end)�Ƿ��Ѿ����ڵ�dualloop�и�
	void new_si_cut_check(int si_start, int si_end);
	void promote_with_featureCurves(bool show);
	
	
public:
	//������OmEgH��ʾ��dual loops
	OmDualloop M4_loop_to_Om_loop(DualLoop &loop);
	Set_of_OmDualloop dualloops_OmEgH();
	//����һ��M4�ϵĵ㣬����Ϊ��㹹��dual loop
	DualLoop promote_from(int start_vertex);
	//��ں���
	void process(std::vector<std::set<size_t>> hard_feature_vhs, std::vector<std::vector<int>> singularity_pair_he_idx
		,std::set<std::pair<int, int>> hardedge_pairing);

	//����һ��mesh�ϵ�����һ��dualloop�ı�ţ����ع������������dualloop������dualloop
	OmDualloop find_loop(OmVeH start_vertex, int dualloop_idx);
	//����һ��mesh�ϵ�����һ����ʼ���򣬷��ع��õ���÷�������е�dual loop
	OmDualloop find_loop_along_direction(OmVeH start_vertex, VECTOR tangent_dir);

	OmDualloop swap_loop(OmVeH start_vertex, int dualloop_idx, int target_idx);
	//���ļ��ж�ȡexistingloops����Ϣ
	void read_from_file(std::string filename, std::vector<std::set<size_t>> hard_feature_vhs);
	//��existingloops����Ϣд���ļ�
	void write_to_file(std::string filename);

	//�ж�����loop�Ƿ���M2���ཻ
	bool loop_cross_type(int loop1_idx, int loop2_idx);
	//�ж�һ��dualloop�Ƿ��������
	bool dualloop_is_redundant(int dualloop_idx, bool *removed);
	//�Ƴ������dualloop
	void remove_redundant_loops(int n);
	//�Ƴ�ָ��id��Loops
	void remove_specified_loops(std::unordered_set<int> sp_ids);
};