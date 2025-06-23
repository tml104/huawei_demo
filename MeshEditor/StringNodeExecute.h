#pragma once

#include "NodeExecuteBase.h"
#include "NodeParamsBase.h"

// Project include
#ifndef IN_HUAWEI
#include "logger44/CoreOld.h"
#include "json/json.h"
#else
#include "CoreOld.h"
#include "json.h"
#endif

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