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

/*
	备注：对于这些单面模型根本不存在需要改single为double的情况啊？
*/
namespace SingleSideFaces {

	struct SingleSideFacesFixer {

		std::set<FACE*> single_side_faces;
		ENTITY_LIST& bodies;

		void FindSingleSideFaces();

		void SetFacesToDoubleSide();

		SingleSideFacesFixer(ENTITY_LIST & bodies) : bodies(bodies) {};

		void Start();

		void Clear();
	};

} // namespace SingleSideFaces