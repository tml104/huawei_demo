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


#include "StartLoadNodeExecute.h"
#include "StartCheckNodeExecute.h"
#include "StartFixNodeExecute.h"

#include "StringNodeExecute.h"

#include "LoadEntityNodeExecute.h"
#include "InitMarkNumNodeExecute.h"

#include "ExportEachBodyNodeExecute.h"
#include "ExportGeometryNodeExecute.h"

#include "ParallelNodeExecute.h"

// FixCheck
#include "NonManifoldEdgeCountNodeExecute.h"
#include "NonManifoldEdgeClassifyNodeExecute.h"
#include "GapMatchNodeExecute.h"
#include "FaceOverlapFindNodeExecute.h"

// FixExecute
#include "NonManifoldFixNodeExecute.h"
#include "GapFixNodeExecute.h"
#include "FaceOverlapFixNodeExecute.h"

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