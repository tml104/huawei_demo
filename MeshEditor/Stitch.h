#pragma once

// ACIS include
#include <boolapi.hxx>
#include <curextnd.hxx>
#include <cstrapi.hxx>
#include <face.hxx>
#include <lump.hxx>
#include <shell.hxx>
#include <split_api.hxx>
#include <kernapi.hxx>
#include <intrapi.hxx>
#include <intcucu.hxx>
#include <coedge.hxx>
#include <curdef.hxx>
#include <curve.hxx>
#include <ellipse.hxx>
#include <straight.hxx>
#include <intcurve.hxx>
#include <helix.hxx>
#include <undefc.hxx>
#include <elldef.hxx>
#include <interval.hxx>
#include <base.hxx>
#include <vector_utils.hxx>
#include <vertex.hxx>
#include <sps3crtn.hxx>
#include <sps2crtn.hxx>
#include <intdef.hxx>
#include <bool_api_options.hxx>
#include <loop.hxx>
#include <wire.hxx>
#include <pladef.hxx>
#include <plane.hxx>
#include <condef.hxx>
#include <cone.hxx>
#include <torus.hxx>
#include <tordef.hxx>
#include <attrib.hxx>
#include <af_api.hxx>
#include <attrib.hxx>
#include <geom_utl.hxx>
#include <body.hxx>
#include <point.hxx>
#include <ckoutcom.hxx>

// my include
#include "logger44/CoreOld.h"
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


namespace Stitch {
	const double EPSLION1 = 0.1; // EPSLION1: 距离误差：0.1
	const double EPSLION2 = cos(MYPI / 6); // EPSLION2: 角度误差：30 degree
	const double EPSLION2_SIN = sqrt(1.0 - EPSLION2 * EPSLION2);
	const int MIDPOINT_CNT = 5;
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
		std::vector<std::pair<Stitch::PoorCoedge,double>> Match(Stitch::PoorCoedge poor_coedge1);

		MatchTreeNode* root;
	};

	struct Singleton {
		static std::vector<Stitch::PoorCoedge> Stitch::Singleton::poor_coedge_vec; // 保存匹配的poor coedge pair 的 vector
		static std::set<COEDGE*> Stitch::Singleton::found_coedge_set; // 维护已经匹配的coedge集合
		static std::vector<std::pair<Stitch::PoorCoedge, Stitch::PoorCoedge>> Stitch::Singleton::poor_coedge_pair_vec;
		static Stitch::MatchTree Stitch::Singleton::match_tree;
	};

	namespace Debug {
		
	} // namespace Debug

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


	// 调用顺序：1
	void FindPoorCoedge(ENTITY_LIST &bodies);

	// 调用顺序：2
	void MatchPoorCoedge();

	// 调用顺序：3
	void StitchPoorCoedge(ENTITY_LIST &bodies);

	void Init(ENTITY_LIST &bodies, bool call_fix = true);

	void Clear();
} // namespace Stitch