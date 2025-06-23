#include "StdAfx.h"
#include "LoadEntityNodeExecute.h"


void BPSystem::LoadEntityNodeExecute::Run(NodeParamsBase * input_params, NodeParamsBase * output_params)
{
	std::string* input_file_path = static_cast<std::string*>(input_params->ptrs[1]);

	LOG_INFO("input_file_path: %s", input_file_path->c_str());

	ENTITY_LIST* bodies = new ENTITY_LIST();


	FILE* f = fopen(input_file_path->c_str(), "r");

	if (!f) {
		LOG_ERROR("打开模型文件失败！");
		return;
	}

	api_restore_entity_list(f, TRUE, *bodies);
	fclose(f);

	output_params->ptrs[1] = static_cast<void*>(bodies);
}
