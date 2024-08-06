#pragma once

//#include <QFileDialog>
//#include <QFile>
//#include <QFileInfo>
//#include <QDockWidget>

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

//#include "json/json.h"
//#include "FileManagement.h"
//#include "ohmConnection.h"
//#include "pixel.h"
//#include "GetPatchType.h"
//#include "JsonHandle.h"
//#include "GeometricFill.h"
//#include "setAttr.h"

#include "UtilityFunctions.h"
#include "logger44/CoreOld.h"

#include <set>
#include <map>
#include <algorithm>
#include <functional>
#include <string>
#include <ctime>
#include <cstdio>
#include <cstring>

#ifndef USE_HOOPSVIEW
#define USE_HOOPSVIEW
#endif

#ifdef USE_HOOPSVIEW
#include "hoopsview.h"
#endif

namespace MarkNum {
	struct Singleton {
		static std::map<ENTITY*, std::pair<std::string, int>> marknum_map;

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