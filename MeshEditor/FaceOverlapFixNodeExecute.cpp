#include "StdAfx.h"
#include "FaceOverlapFixNodeExecute.h"

#include <cstdlib>

void BPSystem::FaceOverlapFixNodeExecute::Run(NodeParamsBase * input_params, NodeParamsBase * output_params)
{
	ENTITY_LIST* bodies = static_cast<ENTITY_LIST*>(input_params->ptrs[1]);

	LOG_INFO("FaceOverlap Fix Test");

	//std::string bat_file_path = "D:\\hqh_study\\huawei_frame\\huawei_demo\\JsonData\\backup\\cp.bat";

	//std::string cmd = "\"" + bat_file_path + "\"";

	//int result = std::system(cmd.c_str());

	output_params->ptrs[1] = bodies;

}
