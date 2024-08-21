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
#include <exception>

namespace NonManifold {

	namespace Debug {
		void PrintLoopsInfo(ENTITY_LIST &bodies);
	}

	/*
		loop并查集
		作用：将能通过一般边连接在一起的loop分成同一组“group”
	*/
	struct LoopFindUnionSet {
		std::map<LOOP*, LOOP*> father_map;

		LOOP* get_father(LOOP* loop1);
		void unite(LOOP* loop1, LOOP* loop2);
		std::map<LOOP*, std::set<LOOP*>> get_group_map();
		void clear();
	};

	struct NonManifoldFixer {

		// 找到的非流形边的集合（edge中coedge数目大于2）
		std::set<EDGE*> nonmanifold_edge_set;
		// 特殊非流形边（满足xloop条件的）集合map，key是对应的形成环的特殊非流形，value是对应那个环的loop指针
		std::map<EDGE*, LOOP*> special_nonmanifold_edge_map;
		// 维护原来所有边的初始开始顶点、结束顶点集合，(edge* -> vertex*)
		std::map<EDGE*, VERTEX*> old_start_vertex_map;
		std::map<EDGE*, VERTEX*> old_end_vertex_map;
		// 模型中每个点到对应若干条相邻边的映射集合
		std::map<VERTEX*, std::set<EDGE*>> vertex_to_edge_map;
		// 并查集 单例实例
		NonManifold::LoopFindUnionSet loop_findunionset;

		ENTITY_LIST & bodies;

		// 调用顺序：1
		void FindNonManifold();

		// 调用顺序：2
		// 特判：如果有某个非流形满足环中有重复的非流形边的那个特征（xloop）那么就把它从nonmanifold_edge_set中去除，并加入special_nonmanifold_edge_set中
		void SpecialCheckNonManifold();

		// 调用顺序：3
		void SolveSpecialNonManifold();

		// 调用顺序：(version:1) 4
		void MaintainFindUnionSet();

		// 调用顺序：(version:1) 5
		void SolveNonManifold();

		// 调用顺序：(version:2) 4
		// 第二版SolveNonManifold实现，将原MaintainFindUnionSet、SolveNonManifold合并，其将以每个非流形边为点位来处理问题
		void SolveNonManifold2();

		void Start();

		void Clear();

		NonManifoldFixer(ENTITY_LIST &bodies): bodies(bodies) {}
	};

	//struct NonManifoldFixer2 {
	//	// 找到的常规非流形边的集合（edge中coedge数目大于2）
	//	std::set<EDGE*> normal_nonmanifold_edges;
	//	// 特殊非流形边（满足xloop条件的）集合
	//	std::set<EDGE*> special_nonmanifold_edges;


	//};

} // namespace NonManifold