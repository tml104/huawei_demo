#include "StdAfx.h"
#include "GapFixNodeExecute.h"

void BPSystem::GapFixNodeExecute::Run(NodeParamsBase * input_params, NodeParamsBase * output_params)
{
	ENTITY_LIST* bodies = static_cast<ENTITY_LIST*>(input_params->ptrs[1]);

	for (int i = 0; i < (*bodies).count(); i++) {

		ENTITY* ibody = (*bodies)[i];
		ENTITY_LIST ibody_list;
		ibody_list.add(ibody);
		bool selected = false;


		LOG_INFO("Stitching for body: [%d]", i);

		Stitch::StitchGapFixer stitchGapFixer(ibody_list);
		selected |= stitchGapFixer.Start(true, true);

		stitchGapFixer.Clear();

		LOG_INFO("Stitching for body (second pass): [%d]", i);
		selected |= stitchGapFixer.Start(true, false);

	}

	output_params->ptrs[1] = bodies;

}
