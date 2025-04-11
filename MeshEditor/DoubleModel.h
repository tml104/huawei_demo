#pragma once

// ACIS include
#include "ACISincluded.h"

// my include

#ifndef IN_HUAWEI
#include "logger44/CoreOld.h"
#else
#include "CoreOld.h"
#endif
#include "MyConstant.h"
#include "MarkNum.h"
#include "UtilityFunctions.h"

// std include
#include <set>
#include <map>
#include <vector>
#include <array>
#include <bitset>
#include <algorithm>
#include <functional>
#include <string>
#include <ctime>
#include <cmath>
#include <exception>

namespace DoubleModel {

	struct DoubleModelMaker {

		ENTITY_LIST& bodies;

		DoubleModelMaker(ENTITY_LIST& bodies) : bodies(bodies) {};

		bool HasSingleSideEdge(BODY* body);

		void CheckSingleSideEdgeAndDoubleModel();

		void Start();

		void Clear();

	};

} // namespace DoubleModel