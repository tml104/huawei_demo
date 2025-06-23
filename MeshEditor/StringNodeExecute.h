#pragma once

#include "NodeExecuteBase.h"
#include "NodeParamsBase.h"

namespace BPSystem {

	//struct StringNodeInputParams: public NodeParamsBase{
	//	// None
	//};

	//struct StringNodeOutputParams : public NodeParamsBase {

	//	std::string* s;

	//};


	struct StringNodeExecute: public NodeExecuteBase{
		void Run(NodeParamsBase* input_params, NodeParamsBase* output_params) override;
	};

}