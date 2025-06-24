#include "StdAfx.h"
#include "NonManifoldEdgeCountNodeExecute.h"

void BPSystem::NonManifoldEdgeCountNodeExecute::Run(NodeParamsBase * input_params, NodeParamsBase * output_params)
{
	ENTITY_LIST* bodies = static_cast<ENTITY_LIST*>(input_params->ptrs[1]);

	NonManifoldCountDataStruct* ds = new NonManifoldCountDataStruct();

	// 只是统计一下非双面边，然后塞到ds里面
	for (int i = 0; i < (*bodies).count(); i++) {

		ENTITY* ibody = ((*bodies)[i]);

		ENTITY_LIST edge_list;
		api_get_edges(ibody, edge_list);

		for (int j = 0; j < edge_list.count(); j++) {

			EDGE* iedge = dynamic_cast<EDGE*>(edge_list[j]);
			int coedge_count = Utils::CoedgeCount(iedge);

			if (coedge_count != 2) {
				LOG_DEBUG("NonManifold edge found: iedge: %d, coedge_cnt: %d", MarkNum::GetId(iedge), coedge_count);

				ds->nonmanifoldEdgeMap[coedge_count].emplace_back(iedge);
			}
		}
	}


	output_params->ptrs[1] = ds;

}
