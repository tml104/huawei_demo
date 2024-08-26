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
#include <debug.hxx>
#include <eulerapi.hxx>
#include <geometry.hxx>

// Project include
#include "logger44/CoreOld.h"
#include "MarkNum.h"
#include "MyConstant.h"
#include "json/json.h"
//#include "UtilityFunctions.h"
#include "GeometryUtils.h"

// STL
#include <time.h>
#include <fstream>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <functional>
#include <string>
#include <ctime>
#include <cmath>

namespace GeometryExporter {

	void SaveJson(const std::string& file_name, const Json::Value& root);

	struct Exporter {

		ENTITY_LIST& bodies;

		Json::Value SPApositionToJson(const SPAposition& pos);

		Json::Value VertexToJson(VERTEX* vertex, int marknum);
		Json::Value EdgeToJson(EDGE* edge, int marknum);
		Json::Value CoedgeToJson(COEDGE* coedge);
		Json::Value LoopToJson(LOOP* loop);
		Json::Value ShellToJson(SHELL* shell);
		Json::Value LumpToJson(LUMP* lump);
		Json::Value BodyToJson(BODY* body);

		// TODO
		void ExportTriangles();

		// 
		Json::Value ExportGeometryInfo();

		void Start(const std::tuple<std::string, std::string, std::string>& split_path_tuple);

		Exporter(ENTITY_LIST& bodies) : bodies(bodies) {}
	};

} // namespace GeometryExporter