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
		���ڽ����ⲿ�޸ĵ����ݣ����磺��Ŀ��Ƶ���Ϣ�������Ӧ�õ���ǰ���ݽṹ��
	*/

	Json::Value LoadJsonFile(const std::string& json_path);

	struct Importer {

		ENTITY_LIST& bodies;

		void ModifyFaces(const Json::Value& root);

		void Start(const std::string& json_path);

		Importer(ENTITY_LIST& bodies) : bodies(bodies) {}
	};

}