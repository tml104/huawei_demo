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
#include "DebugShow.h"
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

namespace GeometryImporter {

	/*
		用于将在外部修改的内容（例如：面的控制点信息）导入后应用到当前数据结构上
	*/

	Json::Value LoadJsonFile(const std::string& json_path);

	struct Importer {

		ENTITY_LIST& bodies;

		void ModifyFaces(const Json::Value& root);

		void Start(const std::string& json_path);

		Importer(ENTITY_LIST& bodies) : bodies(bodies) {}
	};

}