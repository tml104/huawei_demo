#pragma once

#include "NodeExecuteBase.h"
#include "NodeParamsBase.h"

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
#include "GeometryExporter.h"
#include "GeometryImporter.h"
#include "Timer.h"

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


#include "NonManifoldCountDataStruct.h"

namespace BPSystem {

	struct NonManifoldEdgeCountNodeExecute : public NodeExecuteBase {
		void Run(NodeParamsBase* input_params, NodeParamsBase* output_params) override;
	};

}