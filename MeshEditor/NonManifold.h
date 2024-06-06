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

#include "logger44/CoreOld.h"
#include "MarkNum.h"
#include "UtilityFunctions.h"

#include <set>
#include <map>
#include <vector>
#include <array>
#include <bitset>
#include <algorithm>
#include <functional>
#include <string>
#include <ctime>
#include <exception>

namespace NonManifold {

	namespace Debug {
		void PrintLoopsInfo(ENTITY_LIST &bodies);
	}

	/*
		loop���鼯
		���ã�����ͨ��һ���������һ���loop�ֳ�ͬһ�顰group��
	*/
	struct LoopFindUnionSet{
		std::map<LOOP*, LOOP*> father_map;

		LOOP* get_father(LOOP* loop1);
		void unite(LOOP* loop1, LOOP* loop2);
		std::map<LOOP*, std::set<LOOP*>> get_group_map();
		void clear();
	};

	struct Singleton {
		// �ҵ��ķ����αߵļ��ϣ�edge��coedge��Ŀ����2��
		static std::set<EDGE*> NonManifold::Singleton::nonmanifold_edge_set; 
		// ��������αߣ�����xloop�����ģ�����map��key�Ƕ�Ӧ���γɻ�����������Σ�value�Ƕ�Ӧ�Ǹ�����loopָ��
		static std::map<EDGE*, LOOP*> NonManifold::Singleton::special_nonmanifold_edge_map;

		// ά��ԭ�����бߵĳ�ʼ��ʼ���㡢�������㼯�ϣ�(edge* -> vertex*)
		static std::map<EDGE*, VERTEX*> NonManifold::Singleton::old_start_vertex_map; 
		static std::map<EDGE*,VERTEX*> NonManifold::Singleton::old_end_vertex_map;
		// ģ����ÿ���㵽��Ӧ���������ڱߵ�ӳ�伯��
		static std::map<VERTEX*, std::set<EDGE*>> NonManifold::Singleton::vertex_to_edge_map;
		// ���鼯 ����ʵ��
		static NonManifold::LoopFindUnionSet NonManifold::Singleton::loop_findunionset;
	};


	// ����˳��1
	void FindNonManifold(ENTITY_LIST &bodies);

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

	void Init(ENTITY_LIST &bodies);

	void Clear();
} // namespace NonManifold