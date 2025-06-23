#pragma once

// Project include
#ifndef IN_HUAWEI
#include "logger44/CoreOld.h"
#include "json/json.h"
#else
#include "CoreOld.h"
#include "json.h"
#endif

#include <vector>

namespace BPSystem {

	typedef unsigned long long ull;


	struct Link {
		ull linkId;

		ull startPinInstanceId;
		ull endPinInstanceId;

		Link() {}

		Link(Json::Value j);

	};



}