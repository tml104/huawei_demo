#include "StdAfx.h"
#include "InitMarkNumNodeExecute.h"

void BPSystem::InitMarkNumNodeExecute::Run(NodeParamsBase * input_params, NodeParamsBase * output_params)
{
	ENTITY_LIST* bodies = static_cast<ENTITY_LIST*> (input_params->ptrs[1]);

	MarkNum::Init(*bodies);

	output_params->ptrs[1] = bodies;
}
