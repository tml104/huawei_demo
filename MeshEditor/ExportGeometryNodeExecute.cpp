#include "StdAfx.h"
#include "ExportGeometryNodeExecute.h"

void BPSystem::ExportGeometryNodeExecute::Run(NodeParamsBase * input_params, NodeParamsBase * output_params)
{
	std::string* s = static_cast<std::string*> (input_params->ptrs[1]);
	ENTITY_LIST* bodies = static_cast<ENTITY_LIST*> (input_params->ptrs[2]);

	GeometryExporter::Exporter exporter(*bodies);

	auto split_path_tuple = Utils::SplitPath(*s);

	exporter.Start(split_path_tuple);

	LOG_INFO("Geometry Saved!");
}
