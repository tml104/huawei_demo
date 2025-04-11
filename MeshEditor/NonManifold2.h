#pragma once

// ACIS include
#include "ACISincluded.h"

// my include
#ifndef IN_HUAWEI
#include "logger44/CoreOld.h"
#else
#include "CoreOld.h"
#endif
#include "MyConstant.h"
#include "MarkNum.h"
#include "UtilityFunctions.h"

// std include
#include <set>
#include <map>
#include <vector>
#include <array>
#include <bitset>
#include <algorithm>
#include <functional>
#include <string>
#include <ctime>
#include <cmath>
#include <exception>

namespace NonManifold {

	namespace Debug {
		void PrintLoopsInfo(ENTITY_LIST &bodies);
	}

	/*
		loop���鼯
		���ã�����ͨ��һ���������һ���loop�ֳ�ͬһ�顰group��
	*/
	struct LoopFindUnionSet {
		std::map<LOOP*, LOOP*> father_map;

		LOOP* get_father(LOOP* loop1);
		void unite(LOOP* loop1, LOOP* loop2);
		std::map<LOOP*, std::set<LOOP*>> get_group_map();
		void clear();
	};

	struct NonManifoldFixer {

		// �ҵ��ķ����αߵļ��ϣ�edge��coedge��Ŀ����2��
		std::set<EDGE*> nonmanifold_edge_set;
		// ��������αߣ�����xloop�����ģ�����map��key�Ƕ�Ӧ���γɻ�����������Σ�value�Ƕ�Ӧ�Ǹ�����loopָ��
		std::map<EDGE*, LOOP*> special_nonmanifold_edge_map;
		// ά��ԭ�����бߵĳ�ʼ��ʼ���㡢�������㼯�ϣ�(edge* -> vertex*)
		std::map<EDGE*, VERTEX*> old_start_vertex_map;
		std::map<EDGE*, VERTEX*> old_end_vertex_map;
		// ģ����ÿ���㵽��Ӧ���������ڱߵ�ӳ�伯��
		std::map<VERTEX*, std::set<EDGE*>> vertex_to_edge_map;
		// ���鼯 ����ʵ��
		NonManifold::LoopFindUnionSet loop_findunionset;

		ENTITY_LIST & bodies;

		// ����˳��1
		void FindNonManifold();

		// ����˳��2
		// ���У������ĳ�����������㻷�����ظ��ķ����αߵ��Ǹ�������xloop����ô�Ͱ�����nonmanifold_edge_set��ȥ����������special_nonmanifold_edge_set��
		void SpecialCheckNonManifold();

		// ����˳��3
		void SolveSpecialNonManifold();

		// ����˳��(version:1) 4
		void MaintainFindUnionSet();

		// ����˳��(version:1) 5
		void SolveNonManifold();

		// ����˳��(version:2) 4
		// �ڶ���SolveNonManifoldʵ�֣���ԭMaintainFindUnionSet��SolveNonManifold�ϲ����佫��ÿ�������α�Ϊ��λ����������
		void SolveNonManifold2();

		void Start();

		void Clear();

		NonManifoldFixer(ENTITY_LIST &bodies): bodies(bodies) {}
	};

	struct NonManifoldFixer2 {
		// �ҵ��ĳ�������αߵļ��ϣ�edge��coedge��Ŀ����2��
		std::set<EDGE*> nonmanifold_edge_set;

		// ά��ԭ�����бߵĳ�ʼ��ʼ���㡢�������㼯�ϣ�(edge* -> vertex*)
		std::map<EDGE*, VERTEX*> old_start_vertex_map;
		std::map<EDGE*, VERTEX*> old_end_vertex_map;
		// ģ����ÿ���㵽��Ӧ���������ڱߵ�ӳ�伯��
		std::map<VERTEX*, std::set<EDGE*>> vertex_to_edge_map;

		std::set<EDGE*> marked_edges;

		NonManifold::LoopFindUnionSet loop_findunionset;

		ENTITY_LIST & bodies;

		// Status
		int xloop_count;
		int normal_count;

		void RemovePartnerFromEdge(EDGE* origin_edge, COEDGE* coedge);

		std::vector<std::pair<COEDGE*, COEDGE*>> FindXloopCoedgePairs(EDGE* edge);

		std::vector<std::pair<std::vector<COEDGE*>, std::set<LOOP*>>>  FindNormalCoedgePairs(EDGE* edge, std::set<COEDGE*> excluded_coedges);

		void SolveXloopCoedgePairs(std::vector<std::pair<COEDGE*, COEDGE*>> coedge_pairs, EDGE* origin_edge);

		void SolveNormalCoedgePairs(std::vector<std::pair<std::vector<COEDGE*>, std::set<LOOP*>>> coedge_pairs, EDGE* origin_edge);

		// 1
		void FindNonManifold();

		// 2
		void SolveForEachNonManifoldEdge();

		// 3
		void SplitInnerLoop();

		void Status();

		bool Start();

		void Clear();

		NonManifoldFixer2(ENTITY_LIST &bodies) : bodies(bodies), xloop_count(0), normal_count(0){}
	};

} // namespace NonManifold