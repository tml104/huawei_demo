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
#include "MyConstant.h"
#include "UtilityFunctions.h"
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

/*
	时间：2024年3月8日 16:42:44
	这个文件的主要目的就是做一些检查几何上的实验，后续如果要做求交之类的算法实验的话再说吧
*/

namespace Exp1 {
	struct Exp1 {
		std::set<EDGE*> bad_edge_set;

		void UpdateBadCoedgeSet(ENTITY_LIST& bodies);

		void TraverseLoops(LOOP* iloop, bool traverse_incident_loops, COEDGE* begin_coedge = nullptr);

		void StartExperiment(ENTITY_LIST & bodies);

		void Init(ENTITY_LIST &bodies);
	};
}