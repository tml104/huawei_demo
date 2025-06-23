#pragma once

#include "NodeExecuteBase.h"
#include "NodeParamsBase.h"

namespace BPSystem {

	struct StartLoadNodeExecute : public NodeExecuteBase {
		void Run(NodeParamsBase* input_params, NodeParamsBase* output_params) override;
	};

}