#pragma once

#include "StdAfx.h"
#include "DoubleModel.h"


bool DoubleModel::DoubleModelMaker::HasSingleSideEdge(BODY * body)
{
	ENTITY_LIST edge_list;
	api_get_edges(body, edge_list);

	for (int j = 0; j < edge_list.count(); j++) {
		EDGE* edge = dynamic_cast<EDGE*>(edge_list[j]);

		if (Utils::CoedgeCount(edge) == 1) {
			return true;
		}
	}

	return false;
}

void DoubleModel::DoubleModelMaker::CheckSingleSideEdgeAndDoubleModel()
{
	int origin_size = bodies.count();
	ENTITY_LIST new_list;

	for (int i = 0; i < origin_size; i++) {
		BODY* ibody_ptr = dynamic_cast<BODY*>(bodies[i]);
		bool has_single_side_edge = HasSingleSideEdge((ibody_ptr));

		if (has_single_side_edge) {
			// 复制一个新body
			BODY* new_body;
			api_copy_body(ibody_ptr, new_body);
			new_list.add(new_body);

			LOG_INFO("Body %d copied.", MarkNum::GetId(ibody_ptr));
		}
	}

	if (new_list.count())
	{
		bodies.add(new_list);
		LOG_INFO("Add new copied bodies.");
	}
}

void DoubleModel::DoubleModelMaker::Start()
{
	LOG_INFO("Start.");
	api_initialize_constructors();
	api_initialize_booleans();

	CheckSingleSideEdgeAndDoubleModel();

	api_terminate_constructors();
	api_terminate_booleans();
	LOG_INFO("End.");
}

void DoubleModel::DoubleModelMaker::Clear()
{
}
