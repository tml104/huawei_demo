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

namespace DebugShow {

	struct Singleton {
		// 保存名字到点的映射，输出的时候遍历这个东西即可
		static std::map<std::string, SPAposition> debug_points_map;
	};

	void Init();

	void Clear();

	void AddPoint(const std::string& name, const SPAposition& pos);
	void AddPoint(const int num_as_name, const SPAposition& pos);

} // namespace DebugShow