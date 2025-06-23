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

	struct NodeClass {
		ull classId;
		ull nodeType;

		std::vector<ull> inputPinsClassId;
		std::vector<ull> outputPinsClassId;

		NodeClass() {}

		NodeClass(Json::Value j);
	};

	struct NodeInstance {
		ull instanceId;
		ull classId;

		std::vector<ull> inputPinsInstaceId;
		std::vector<ull> outputPinsInstaceId;

		NodeInstance() {}

		NodeInstance(Json::Value j);
	};

}