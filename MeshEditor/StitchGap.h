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
	const double EPSLION1 = 0.05; // EPSLION1: ������0.02
	const double EPSLION2 = cos(MYPI / 6); // EPSLION2: �Ƕ���30 degree
	const double EPSLION2_SIN = sqrt(1.0 - EPSLION2 * EPSLION2);
	const int MIDPOINT_CNT = 5;  //���������
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
		std::vector<std::pair<Stitch::PoorCoedge, double>> Match(Stitch::PoorCoedge poor_coedge1, const std::set<COEDGE*>& found_coedge_set, bool dont_stitch_coincident);

		MatchTreeNode* root;
	};

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

	struct EdgesData {
		// �м������Ҫ�õ������ݽṹά��
		std::map<std::pair<VERTEX*, VERTEX*>, std::vector<EDGE*>> vertex_pair_to_edge_map; // ׼��<vertex,vertex> to edge��map: ���mapֻ�������ж�����֮���Ƿ���ڱ������õģ�������ʲô�����Ӳ���Ҫ��������ģ��������ͬ����ıߵ����Ҳû��ϵ
		std::vector<EDGE*> all_edge_vector; // ˳��ά��һ������ʵ���ȫ���ߵ�vector

		void Init(ENTITY_LIST& bodies);

		void Clear();

		std::vector<EDGE*> FindEdgesBetweenVertices(VERTEX* v1, VERTEX* v2);
	};

	struct StitchGapFixer {

		std::vector<Stitch::PoorCoedge> poor_coedge_vec; // ����ƥ���poor coedge pair �� vector
		std::set<COEDGE*> found_coedge_set; // ά���Ѿ�ƥ���coedge����
		std::vector<std::pair<Stitch::PoorCoedge, Stitch::PoorCoedge>> poor_coedge_pair_vec;

		EdgesData edges_data;

		Stitch::MatchTree match_tree;
		ENTITY_LIST &bodies;

		bool dont_stitch_coincident;

		// ����˳��1
		void FindPoorCoedge(ENTITY_LIST &bodies);

		// ����˳��2
		void MatchPoorCoedge();

		// ����˳��3
		void RearrangePoorCoedge();

		// ����˳��4
		void StitchPoorCoedge(ENTITY_LIST &bodies);

		// ������

		void Status();

		void PreProcess();

		StitchGapFixer(ENTITY_LIST &bodies) : bodies(bodies) {};

		void Start(bool call_fix, bool dont_stitch_coincident);

		void Clear();
	};


} // namespace Stitch