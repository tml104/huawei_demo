#include "StdAfx.h"
#include "NonManifoldFixNodeExecute.h"

void BPSystem::NonManifoldFixNodeExecute::Run(NodeParamsBase * input_params, NodeParamsBase * output_params)
{
	ENTITY_LIST* bodies = static_cast<ENTITY_LIST*>(input_params->ptrs[1]);

	for (int i = 0; i < (*bodies).count(); i++) {

		ENTITY* ibody = (*bodies)[i];
		ENTITY_LIST ibody_list;
		ibody_list.add(ibody);
		bool selected = false;


		LOG_INFO("Solving Nonmanifold for body: [%d]", i);
		NonManifold::NonManifoldFixer2 nonManifoldFixer2(ibody_list);
		selected |= nonManifoldFixer2.Start();

	}

	output_params->ptrs[1] = bodies;
}
