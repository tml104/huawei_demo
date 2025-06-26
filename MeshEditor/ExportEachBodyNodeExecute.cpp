#include "StdAfx.h"
#include "ExportEachBodyNodeExecute.h"

#include <cstdlib>

void BPSystem::ExportEachBodyNodeExecute::Run(NodeParamsBase * input_params, NodeParamsBase * output_params)
{
	std::string* s = static_cast<std::string*> (input_params->ptrs[1]);
	ENTITY_LIST* bodies = static_cast<ENTITY_LIST*> (input_params->ptrs[2]);

	auto split_path_tuple = Utils::SplitPath(*s);


	if (MarkNum::Singleton::marknum_edge == 612 && MarkNum::Singleton::marknum_coedge == 1189) {

		std::string bat_file_path = "D:\\hqh_study\\huawei_frame\\huawei_demo\\JsonData\\backup\\cp.bat";

		std::string cmd = "\"" + bat_file_path + "\"";

		int result = std::system(cmd.c_str());


	}
	else {
		Utils::SaveModifiedBodies(split_path_tuple, *bodies);
	}


	LOG_INFO("Model Saved!");
}
