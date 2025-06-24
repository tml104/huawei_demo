#include "StdAfx.h"
#include "ExportEachBodyNodeExecute.h"

void BPSystem::ExportEachBodyNodeExecute::Run(NodeParamsBase * input_params, NodeParamsBase * output_params)
{
	std::string* s = static_cast<std::string*> (input_params->ptrs[1]);
	ENTITY_LIST* bodies = static_cast<ENTITY_LIST*> (input_params->ptrs[2]);

	auto split_path_tuple = Utils::SplitPath(*s);

	Utils::SaveModifiedBodies(split_path_tuple, *bodies);

	LOG_INFO("Model Saved!");
}
