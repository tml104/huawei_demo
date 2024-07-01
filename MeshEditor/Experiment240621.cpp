#include "StdAfx.h"
#include "Experiment240621.h"


/*
	1: 获取从3610到3642的所有bounds然后合并
*/
std::pair<SPAposition, SPAposition> Exp3::GetEdgesLowHigh(BODY * blank)
{
	//return std::pair<SPAposition, SPAposition>();

	ENTITY_LIST edge_list;
	api_get_edges(blank, edge_list);

	const double MIN_VAL = -1e10;
	const double MAX_VAL = 1e10;

	SPAposition l(MAX_VAL, MAX_VAL, MAX_VAL), h(MIN_VAL, MIN_VAL, MIN_VAL);

	for (int i = 0; i < edge_list.count(); i++){
		EDGE* edge_ptr = dynamic_cast<EDGE*>(edge_list[i]);
		int edge_id = MarkNum::GetId(edge_ptr);

		if (edge_id >= 3610 && edge_id <= 3642){

			SPAposition tl = edge_ptr->bound()->low();
			SPAposition th = edge_ptr->bound()->high();

			// 更新l,h值
			for (int j = 0; j < 3; j++)
			{
				l.set_coordinate(j, std::min(l.coordinate(j), tl.coordinate(j)));
				h.set_coordinate(j, std::max(h.coordinate(j), th.coordinate(j)));
			}

		}
	}

	// temp add up
	l.x() -= 1.0;
	l.y() -= 1.0;
	l.z() -= 1.0;

	h.x() += 1.0;
	h.y() += 1.0;
	h.z() += 1.0;

	// [debug] check range
	LOG_INFO("low point: (%.5lf, %.5lf, %.5lf)", l.x(), l.y(), l.z());
	LOG_INFO("high point: (%.5lf, %.5lf, %.5lf)", h.x(), h.y(), h.z());

	return std::make_pair(l, h);
}

BODY * Exp3::MakeTool(std::pair<SPAposition, SPAposition> LH)
{
	BODY* block;
	api_solid_block(LH.first, LH.second, block);

	return block;
}

/*
	3: 布尔运算切除
*/
void Exp3::DoCut(BODY * tool, BODY * blank)
{
	//api_subtract(tool, blank);
	api_intersect(tool, blank);
}

void Exp3::Init(BODY * blank)
{
	api_initialize_booleans();
	api_initialize_constructors();

	auto LH = Exp3::GetEdgesLowHigh(blank);
	BODY* tool = Exp3::MakeTool(LH);
	DoCut(tool, blank);

	api_terminate_booleans();
	api_terminate_constructors();
}
