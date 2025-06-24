#include "StdAfx.h"
#include "StringNodeExecute.h"

void BPSystem::StringNodeExecute::Run(NodeParamsBase * input_params, NodeParamsBase * output_params)
{
	std::string* s = new std::string("D:\\hqh_study\\huawei_frame\\huawei_demo\\JsonData\\C_ent(1)_mod_body_2.sat");

	//output_params->ptrs.clear();
	//output_params->ptrs.emplace_back(s);

	//LOG_INFO("output_params->ptrs size: %d", output_params->ptrs.size());

	output_params->ptrs[0] = s;
}
