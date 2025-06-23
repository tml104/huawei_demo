#pragma once

#include <memory>

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

#include "NodeACIS.h"
#include "PinACIS.h"
#include "LinkACIS.h"

#include "NodeExecuteBase.h"
#include "NodeParamsBase.h"

#include "LoadEntityNodeExecute.h"
#include "StartLoadNodeExecute.h"
#include "StringNodeExecute.h"

namespace BPSystem {

	typedef unsigned long long ull;

struct BPSystem {

	std::map<ull, NodeClass> nodeClassMap;
	std::map<ull, PinClass> pinClassMap;

	std::map<ull, NodeInstance> nodeInstaceMap;
	std::map<ull, PinInstance> pinInstaceMap;

	std::map<ull, Link> linkMap;

	// Ö´ÐÐÓÐ¹Ø

	std::map<ull, NodeParamsBase*> inputNodeParamsMap;
	std::map<ull, NodeParamsBase*> outputNodeParamsMap;
	std::map<ull, NodeExecuteBase*> nodeExecuteMap;

	void InitPlay();

	// ---

	void ParseClasses(Json::Value j);

	void ParseInstances(Json::Value j);

	void PlayFrom(ull start_class_id);

	BPSystem(std::string class_json_path, std::string instance_json_path);
};

}