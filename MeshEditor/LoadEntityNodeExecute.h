#pragma once

#include "NodeExecuteBase.h"
#include "NodeParamsBase.h"


namespace BPSystem {

	struct LoadEntityNodeExecute : public NodeExecuteBase {
		void Run(NodeParamsBase* input_params, NodeParamsBase* output_params) override;
	};

}