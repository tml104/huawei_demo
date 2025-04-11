#pragma once

// ACIS include
#include "ACISincluded.h"

#ifndef IN_HUAWEI
#include "logger44/CoreOld.h"
#else
#include "CoreOld.h"
#endif
#include "MyConstant.h"
#include "UtilityFunctions.h"


#include <set>
#include <map>
#include <algorithm>
#include <functional>
#include <string>
#include <ctime>
#include <cstdio>
#include <cstring>

#ifdef USE_HOOPSVIEW
#include "hoopsview.h"
#endif

namespace MarkNum {
	struct Singleton {
		static std::map<ENTITY*, std::pair<std::string, int>> marknum_map;
		static std::map<ENTITY*, int> body_map;

		static int marknum_body;
		static int marknum_lump;
		static int marknum_shell;
		static int marknum_wire;
		static int marknum_face;
		static int marknum_loop;
		static int marknum_coedge;
		static int marknum_edge;
		static int marknum_vertex;
	};

	void Init(ENTITY_LIST & bodies);

	int GetId( ENTITY*const & ptr);
	std::string GetTypeName(ENTITY*const & ptr);
	int GetBody(ENTITY*const & ptr);

#ifdef USE_HOOPSVIEW
	void ShowEdgeMark(HoopsView* hv);

	void ShowEdgeMark(HoopsView* hv, std::set<int>& show_edge_marknum_set);

	void ShowFaceMark(HoopsView* hv);
#endif

	void Clear();

	namespace Debug {
		// 打印整个map的所有指针信息
		void PrintMap();
	}

} //namespace MarkNum