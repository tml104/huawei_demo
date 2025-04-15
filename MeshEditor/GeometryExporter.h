#pragma once

// ACIS include
#include "ACISincluded.h"

// Project include
#ifndef IN_HUAWEI
#include "logger44/CoreOld.h"
#include "json/json.h"
#else
#include "CoreOld.h"
#include "json.h"
#endif
#include "MarkNum.h"
#include "MyConstant.h"
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
		Json::Value SPAparposToJson(const SPApar_pos& pos);
		Json::Value SPAunitvectorToJson(const SPAunit_vector& unit_vec);
		Json::Value SPAvectorToJson(const SPAvector& vec);

		Json::Value VertexToJson(VERTEX* vertex, int marknum);
		Json::Value EdgeToJson(EDGE* edge, int marknum);
		Json::Value CoedgeToJson(COEDGE* coedge, int marknum);
		Json::Value LoopToJson(LOOP* loop, int marknum);
		Json::Value FaceToJson(FACE* face, int marknum);
		Json::Value ShellToJson(SHELL* shell, int marknum);
		Json::Value LumpToJson(LUMP* lump, int marknum);
		Json::Value BodyToJson(BODY* body, int marknum);

		// TODO
		//void ExportTriangles();

		// 
		Json::Value ExportGeometryInfo(int ibody_marknum);

		void Start(const std::tuple<std::string, std::string, std::string>& split_path_tuple, const std::set<int>& selected_bodies);

		void Start(const std::tuple<std::string, std::string, std::string>& split_path_tuple);

		// TODO: 导出用于调试用的点集合（参数还要改）
		void ExportDebugPoints(const std::tuple<std::string, std::string, std::string>& split_path_tuple);

		Exporter(ENTITY_LIST& bodies) : bodies(bodies) {}
	};

} // namespace GeometryExporter