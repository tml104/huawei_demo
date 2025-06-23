#include "StdAfx.h"
#include "StartLoadNodeExecute.h"

void BPSystem::StartLoadNodeExecute::Run(NodeParamsBase * input_params, NodeParamsBase * output_params)
{
	output_params->ptrs.emplace_back(nullptr);
}
