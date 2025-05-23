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
#include "GeometryUtils.h"

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


namespace Stitch {
	const double EPSLION1 = 5; // EPSLION1: 距离误差：0.02
	const double EPSLION2 = cos(MYPI / 6); // EPSLION2: 角度误差：30 degree
	const double EPSLION2_SIN = sqrt(1.0 - EPSLION2 * EPSLION2);
	const int MIDPOINT_CNT = 5;  //最好是奇数
	const double MINVAL = -1e9;
	const double MAXVAL = 1e9;

	//using MyPoint = std::array<double,3>; // for easy use of KDTree
	//using MyVector = std::array<double, 3>; // 这里没法用using，因为c++的版本问题……
	typedef std::array<double, 3> MyPoint;
	typedef std::array<double, 3> MyVector;

	/*
		保存找到的破边信息：破边对应指针以及破边对应采样点
	*/
	struct PoorCoedge {
		COEDGE* coedge;
		MyPoint midpoint_coords[MIDPOINT_CNT];

		PoorCoedge();
	};

	/*
		用于匹配破边的KDTree：
		破边的关键点被设置在PoorCoedge的 (采样点数量>>1) 处
	*/
	struct MatchTree {
		struct MatchTreeNode {
			MyPoint min_range_point;
			MyPoint max_range_point;
			MatchTreeNode* left_node;
			MatchTreeNode* right_node;
			int split_dim;
			bool is_leaf;
			PoorCoedge leaf_poor_coedge;

			MatchTreeNode();
			~MatchTreeNode();
		};

		MatchTree();
		~MatchTree();

		void ConstructTree(std::vector<Stitch::PoorCoedge>& poor_coedge_vec);
		void DeleteTree();
		std::vector<std::pair<Stitch::PoorCoedge, double>> Match(Stitch::PoorCoedge poor_coedge1, const std::set<COEDGE*>& found_coedge_set, bool dont_stitch_coincident);

		MatchTreeNode* root;
	};

	Stitch::MyVector GetMyVector(Stitch::MyPoint a, Stitch::MyPoint b);
	double Distance(Stitch::MyPoint a, Stitch::MyPoint b);
	double Distance(Stitch::MyVector v);
	Stitch::MyVector Normalize(Stitch::MyVector v);
	double Dot(Stitch::MyVector v1, Stitch::MyVector v2);

	/*
		计算两个给定PoorCoedge的匹配分数
		（在MatchTree::Match中会用到）
	*/
	double CalculatePoorCoedgeScore(const PoorCoedge& poor_coedge1, const PoorCoedge& poor_coedge2);

	struct EdgesData {
		// 中间过程需要用到的数据结构维护
		std::map<std::pair<VERTEX*, VERTEX*>, std::vector<EDGE*>> vertex_pair_to_edge_map; // 准备<vertex,vertex> to edge的map: 这个map只是用来判断两点之间是否存在边连接用的，具体是什么边连接不重要，因此如果模型中有相同顶点的边的情况也没关系
		std::vector<EDGE*> all_edge_vector; // 顺带维护一下整个实体的全部边的vector

		void Init(ENTITY_LIST& bodies);

		void Clear();

		std::vector<EDGE*> FindEdgesBetweenVertices(VERTEX* v1, VERTEX* v2);
	};

	struct StitchGapFixer {

		std::vector<Stitch::PoorCoedge> poor_coedge_vec; // 保存匹配的poor coedge pair 的 vector
		std::set<COEDGE*> found_coedge_set; // 维护已经匹配的coedge集合
		std::vector<std::pair<Stitch::PoorCoedge, Stitch::PoorCoedge>> poor_coedge_pair_vec;

		EdgesData edges_data;

		Stitch::MatchTree match_tree;
		ENTITY_LIST &bodies;

		bool dont_stitch_coincident;

		// 调用顺序：1
		void FindPoorCoedge(ENTITY_LIST &bodies);

		// 调用顺序：2
		void MatchPoorCoedge();

		// 调用顺序：3
		void RearrangePoorCoedge();

		// 调用顺序：4
		void StitchPoorCoedge(ENTITY_LIST &bodies);

		// 其他↓

		void Status();

		void PreProcess();

		StitchGapFixer(ENTITY_LIST &bodies) : bodies(bodies) {};

		bool Start(bool call_fix, bool dont_stitch_coincident);

		void Clear();
	};


} // namespace Stitch