#include "StdAfx.h"
#include "LoadEntityNodeExecute.h"

#include <string>

// Project include
#ifndef IN_HUAWEI
#include "logger44/CoreOld.h"
#include "json/json.h"
#else
#include "CoreOld.h"
#include "json.h"
#endif


void BPSystem::LoadEntityNodeExecute::Run(NodeParamsBase * input_params, NodeParamsBase * output_params)
{
	std::string* str_ptr = static_cast<std::string*>(input_params->ptrs[1]);

	LOG_INFO("string content: %s", str_ptr->c_str());
}
