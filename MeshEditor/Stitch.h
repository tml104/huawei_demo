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
	const double EPSLION1 = 0.1; // EPSLION1: ������0.1
	const double EPSLION2 = cos(MYPI / 6); // EPSLION2: �Ƕ���30 degree
	const double EPSLION2_SIN = sqrt(1.0 - EPSLION2 * EPSLION2);
	const int MIDPOINT_CNT = 5;
	const double MINVAL = -1e9;
	const double MAXVAL = 1e9;

	//using MyPoint = std::array<double,3>; // for easy use of KDTree
	//using MyVector = std::array<double, 3>; // ����û����using����Ϊc++�İ汾���⡭��
	typedef std::array<double, 3> MyPoint;
	typedef std::array<double, 3> MyVector;

	/*
		�����ҵ����Ʊ���Ϣ���Ʊ߶�Ӧָ���Լ��Ʊ߶�Ӧ������
	*/
	struct PoorCoedge {
		COEDGE* coedge;
		MyPoint midpoint_coords[MIDPOINT_CNT];

		PoorCoedge();
	};

	/*
		����ƥ���Ʊߵ�KDTree��
		�ƱߵĹؼ��㱻������PoorCoedge�� (����������>>1) ��
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
		static std::vector<Stitch::PoorCoedge> Stitch::Singleton::poor_coedge_vec; // ����ƥ���poor coedge pair �� vector
		static std::set<COEDGE*> Stitch::Singleton::found_coedge_set; // ά���Ѿ�ƥ���coedge����
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
		������������PoorCoedge��ƥ�����
		����MatchTree::Match�л��õ���
	*/
	double CalculatePoorCoedgeScore(const PoorCoedge& poor_coedge1, const PoorCoedge& poor_coedge2);


	// ����˳��1
	void FindPoorCoedge(ENTITY_LIST &bodies);

	// ����˳��2
	void MatchPoorCoedge();

	// ����˳��3
	void StitchPoorCoedge(ENTITY_LIST &bodies);

	void Init(ENTITY_LIST &bodies, bool call_fix = true);

	void Clear();
} // namespace Stitch