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

	struct PinClass {
		ull classId;
		ull pinType;
		ull pinKind;

		PinClass() {}

		PinClass(Json::Value j);

	};


	struct PinInstance {
		ull instanceId;
		ull classId;
		ull nodeInstanceId;


		PinInstance() {}

		PinInstance(Json::Value j);
	};

}