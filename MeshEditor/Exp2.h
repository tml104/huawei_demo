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

#ifdef USE_HOOPSVIEW
#include "hoopsview.h"
#endif

/*
	对于C_ent(1)_body_0中的特殊例子的环的特判
*/

namespace Exp2 {

	struct Exp2 {
		std::vector<LOOP*> bad_loop_vec;

		void StartExperiment(ENTITY_LIST & bodies);

		void PrintBadLoopVec();

#ifdef USE_HOOPSVIEW
		void ShowBadLoopEdgeMark(HoopsView* hv);
#endif

		void Clear();

		void Init(ENTITY_LIST &bodies);
	};
}