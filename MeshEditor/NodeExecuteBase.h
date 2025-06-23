#pragma once

#include "NodeParamsBase.h"

namespace BPSystem {

	struct NodeExecuteBase {
		virtual void Run(NodeParamsBase* input_params, NodeParamsBase* output_params) = 0;
	};
	
}