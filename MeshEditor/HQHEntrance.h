#pragma once

// ACIS include
#include "ACISincluded.h"

// Project include
#ifndef IN_HUAWEI
#include "logger44/CoreOld.h"
#else
#include "CoreOld.h"
#endif
#include "MarkNum.h"
#include "DebugShow.h"
#include "MyConstant.h"
#include "GeometryExporter.h"
#include "GeometryImporter.h"
#include "Timer.h"

#ifndef IN_HUAWEI
#include "ConstructModel.h"
#include "Exp1.h"
#include "Exp2.h"
#include "Experiment240621.h"
#include "Experiment250305.h"
#include "SingleSideFaces.h"
#endif


#include "DegeneratedFaces.h"
#include "NonManifold2.h"
#include "StitchGap.h"
#include "DoubleModel.h"

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

#ifndef IN_HUAWEI
#include "hoopsview.h"
#endif

namespace HQHEntrance {

#ifdef IN_HUAWEI

	// 这上面还有函数

	static std::vector<std::string> path_vec2;

	ENTITY_LIST LoadEntityListFromID(int model_id, int flag = 0);

	ENTITY_LIST OpenBody(string file_path);

	void Run(int model_id, int option1);

#else

	void Run(const std::string& file_path, HoopsView* hoopsview);

#endif

}// namespace HQHEntrance