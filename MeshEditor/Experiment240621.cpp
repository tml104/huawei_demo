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

/*
	构造实体
*/
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


void Exp4::Exp4::PrintFacesGeometry()
{
	LOG_INFO("start.");

	for (int i = 0; i < this->bodies_to_be_checked.count(); i++)
	{
		ENTITY_LIST face_list;
		api_get_faces(this->bodies_to_be_checked[i], face_list);

		for (int j = 0; j < face_list.count(); j++) {
			FACE* face_p = dynamic_cast<FACE*>(face_list[j]);
			SHELL* shell_p = face_p->shell();

			LOG_DEBUG("face: %d, shell: %d, sense: %d", MarkNum::GetId(face_p), MarkNum::GetId(shell_p), face_p->sense());
			GeometryUtils::PrintFaceGeometry(face_p);
		}
	}

	LOG_INFO("end.");
}



void Exp4::Exp4::FaceFaceIntersectionExperiment()
{
	LOG_INFO("start.");

	// get f1 && f7
	FACE *f1, *f7;

	for (int i = 0; i < this->bodies_to_be_checked.count(); i++)
	{
		ENTITY_LIST face_list;
		api_get_faces(this->bodies_to_be_checked[i], face_list);

		for (int j = 0; j < face_list.count(); j++) {
			FACE* face_p = dynamic_cast<FACE*>(face_list[j]);
			
			if (MarkNum::GetId(face_p) == 1) {
				f1 = face_p;
			}

			if (MarkNum::GetId(face_p) == 7) { // (4,11)NO (5,10)YES
				f7 = face_p;
			}
		}
	}

	GeometryUtils::PrintFaceGeometry(f1);
	GeometryUtils::PrintFaceGeometry(f7);

	// 交图
	BODY *int_graph;
	api_fafa_int(f1, f7, int_graph);

	// 看看WIRE里面有什么
	ENTITY_LIST int_graph_edge_list;
	ENTITY_LIST int_graph_coedge_list;
	ENTITY_LIST int_graph_loop_list;
	ENTITY_LIST int_graph_face_list;
	ENTITY_LIST int_graph_wire_list;
	ENTITY_LIST int_graph_shell_list;
	ENTITY_LIST int_graph_lump_list;
	
	api_get_edges(dynamic_cast<ENTITY*>(int_graph), int_graph_edge_list);
	api_get_coedges(dynamic_cast<ENTITY*>(int_graph), int_graph_coedge_list);
	api_get_loops(dynamic_cast<ENTITY*>(int_graph), int_graph_loop_list);
	api_get_faces(dynamic_cast<ENTITY*>(int_graph), int_graph_face_list);
	api_get_wires(dynamic_cast<ENTITY*>(int_graph), int_graph_wire_list);
	api_get_shells(dynamic_cast<ENTITY*>(int_graph), int_graph_shell_list);
	api_get_lumps(dynamic_cast<ENTITY*>(int_graph), int_graph_lump_list);

	LOG_INFO("int_graph_edge_list count: %d", int_graph_edge_list.count());
	LOG_INFO("int_graph_face_list count: %d", int_graph_face_list.count());
	LOG_INFO("int_graph_wire_list count: %d", int_graph_wire_list.count());
	LOG_INFO("int_graph_shell_list count: %d", int_graph_shell_list.count());
	LOG_INFO("int_graph_lump_list count: %d", int_graph_lump_list.count());

	for (int j = 0; j < int_graph_edge_list.count(); j++)
	{
		EDGE* iedge = dynamic_cast<EDGE*>(int_graph_edge_list[j]);
		VERTEX* st = iedge->start();
		VERTEX* ed = iedge->end();

		LOG_INFO("int_graph_edge_list[%d]", j);

		if (st == ed) {
			LOG_INFO("iedge: st == ed");
		}

		//LOG_INFO("st: %.5lf, %.5lf, %.5lf", st->geometry()->coords().x(), st->geometry()->coords().y(), st->geometry()->coords().z());
		//LOG_INFO("ed: %.5lf, %.5lf, %.5lf", ed->geometry()->coords().x(), ed->geometry()->coords().y(), ed->geometry()->coords().z());

		LOG_INFO("Geometry Check for iedge");
		GeometryUtils::PrintEdgeGeometry(iedge);

		// icoedge partner检查
		COEDGE* icoedge = iedge->coedge();

		do {
			if (icoedge == nullptr) {
				LOG_ERROR("icoedge: nullptr");
				break;
			}

			COEDGE* icoedge_prev = icoedge->previous();
			COEDGE* icoedge_next = icoedge->next();
			LOOP* icoedge_loop = icoedge->loop();
			//FACE* icoedge_face = icoedge_loop->face(); // ERROR：会导致程序崩溃，看上去就是没有

			LOG_INFO("icoedge: %d (prev: %d, next: %d), iloop: %d", 
				icoedge,
				icoedge->previous(),
				icoedge->next(),
				icoedge->loop()
			);

			icoedge = icoedge->partner();
		} while (icoedge && icoedge != iedge->coedge());

	}

	/*
		事实证明用int_graph->wire();去拿这个交线关系是高危操作……
	*/
	//LOG_INFO("A");
	//WIRE *int_graph_wire = int_graph->wire();
	//LOG_INFO("B");
	//if (int_graph_wire == nullptr) {
	//	LOG_ERROR("int_graph_wire: nullptr");
	//	// return;
	//}

	for (int j = 0; j < int_graph_wire_list.count(); j++) {
		WIRE* wr = dynamic_cast<WIRE*>(int_graph_wire_list[j]);
		LOG_INFO("wire in list: %d", wr);
	}

	//while (int_graph_wire) {
	//	COEDGE* icoedge = int_graph_wire->coedge();
	//	if (icoedge == nullptr) {
	//		LOG_ERROR("icoedge: nullptr");
	//		return;
	//	}
	//	EDGE* iedge = icoedge->edge();
	//	if (iedge == nullptr) {
	//		LOG_ERROR("iedge: nullptr");
	//		return;
	//	}

	//	// 边的起始点和结束点相同：退化的点（根据书中第6章描述）
	//	// (潜在的空指针判断……)
	//	VERTEX* st = iedge->start();
	//	if (st == nullptr) {
	//		LOG_ERROR("st: nullptr");
	//		return;
	//	}
	//	VERTEX* ed = iedge->end();
	//	if (ed == nullptr) {
	//		LOG_ERROR("ed: nullptr");
	//		return;
	//	}

	//	if (st == ed) {
	//		LOG_INFO("iedge: st == ed");
	//	}

	//	LOG_INFO("st: %.5lf, %.5lf, %.5lf", st->geometry()->coords().x(), st->geometry()->coords().y(), st->geometry()->coords().z());
	//	LOG_INFO("ed: %.5lf, %.5lf, %.5lf", ed->geometry()->coords().x(), ed->geometry()->coords().y(), ed->geometry()->coords().z());

	//	GeometryExperiment::PrintEdgeGeometry(iedge);

	//	int_graph_wire = int_graph_wire->next();
	//}

	LOG_INFO("end.");
}

void Exp4::Exp4::FaceFaceIntersectionExperiment2()
{
	LOG_INFO("start.");

	// get f185 && f71
	FACE *f185, *f71;

	for (int i = 0; i < this->bodies_to_be_checked.count(); i++)
	{
		ENTITY_LIST face_list;
		api_get_faces(this->bodies_to_be_checked[i], face_list);

		for (int j = 0; j < face_list.count(); j++) {
			FACE* face_p = dynamic_cast<FACE*>(face_list[j]);

			if (MarkNum::GetId(face_p) == 185) {
				f185 = face_p;
			}

			if (MarkNum::GetId(face_p) == 71) { // (4,11)NO (5,10)YES
				f71 = face_p;
			}
		}
	}

	GeometryUtils::PrintFaceGeometry(f185);
	GeometryUtils::PrintFaceGeometry(f71);

	// 交图
	BODY *int_graph;
	api_fafa_int(f185, f71, int_graph);

	// 看看WIRE里面有什么
	ENTITY_LIST int_graph_edge_list;
	ENTITY_LIST int_graph_coedge_list;
	ENTITY_LIST int_graph_loop_list;
	ENTITY_LIST int_graph_face_list;
	ENTITY_LIST int_graph_wire_list;
	ENTITY_LIST int_graph_shell_list;
	ENTITY_LIST int_graph_lump_list;

	api_get_edges(dynamic_cast<ENTITY*>(int_graph), int_graph_edge_list);
	api_get_coedges(dynamic_cast<ENTITY*>(int_graph), int_graph_coedge_list);
	api_get_loops(dynamic_cast<ENTITY*>(int_graph), int_graph_loop_list);
	api_get_faces(dynamic_cast<ENTITY*>(int_graph), int_graph_face_list);
	api_get_wires(dynamic_cast<ENTITY*>(int_graph), int_graph_wire_list);
	api_get_shells(dynamic_cast<ENTITY*>(int_graph), int_graph_shell_list);
	api_get_lumps(dynamic_cast<ENTITY*>(int_graph), int_graph_lump_list);

	LOG_INFO("int_graph_edge_list count: %d", int_graph_edge_list.count());
	LOG_INFO("int_graph_face_list count: %d", int_graph_face_list.count());
	LOG_INFO("int_graph_wire_list count: %d", int_graph_wire_list.count());
	LOG_INFO("int_graph_shell_list count: %d", int_graph_shell_list.count());
	LOG_INFO("int_graph_lump_list count: %d", int_graph_lump_list.count());

	for (int j = 0; j < int_graph_edge_list.count(); j++)
	{
		EDGE* iedge = dynamic_cast<EDGE*>(int_graph_edge_list[j]);
		VERTEX* st = iedge->start();
		VERTEX* ed = iedge->end();

		LOG_INFO("int_graph_edge_list[%d]: st: %d, ed: %d", j, st, ed);

		if (st == ed) {
			LOG_INFO("iedge: st == ed");
		}

		LOG_INFO("Geometry Check for iedge");
		GeometryUtils::PrintEdgeGeometry(iedge);

		// icoedge partner检查
		COEDGE* icoedge = iedge->coedge();

		do {
			if (icoedge == nullptr) {
				LOG_ERROR("icoedge: nullptr");
				break;
			}

			COEDGE* icoedge_prev = icoedge->previous();
			COEDGE* icoedge_next = icoedge->next();
			LOOP* icoedge_loop = icoedge->loop();
			//FACE* icoedge_face = icoedge_loop->face(); // ERROR：会导致程序崩溃，看上去就是没有

			LOG_INFO("icoedge: %d (prev: %d, next: %d), iloop: %d",
				icoedge,
				icoedge->previous(),
				icoedge->next(),
				icoedge->loop()
			);

			icoedge = icoedge->partner();
		} while (icoedge && icoedge != iedge->coedge());

	}

	LOG_INFO("end.");
}

void Exp4::Exp4::FaceFaceIntersectionExperiment3()
{
	LOG_INFO("start.");

	// get f 148 && f 77
	FACE *f148, *f77;

	for (int i = 0; i < this->bodies_to_be_checked.count(); i++)
	{
		ENTITY_LIST face_list;
		api_get_faces(this->bodies_to_be_checked[i], face_list);

		for (int j = 0; j < face_list.count(); j++) {
			FACE* face_p = dynamic_cast<FACE*>(face_list[j]);

			if (MarkNum::GetId(face_p) == 148) {
				f148 = face_p;
			}

			if (MarkNum::GetId(face_p) == 77) { // (4,11)NO (5,10)YES
				f77 = face_p;
			}
		}
	}

	GeometryUtils::PrintFaceGeometry(f148);
	GeometryUtils::PrintFaceGeometry(f77);

	// 包围盒
	SPAbox* box_fA = f148->bound();
	SPAbox* box_fB = f77->bound();

	SPAposition box_fA_low = box_fA->low();
	SPAposition box_fA_high = box_fA->high();

	SPAposition box_fB_low = box_fB->low();
	SPAposition box_fB_high = box_fB->high();

	LOG_INFO("face A (%d) Box: low: (%.5lf, %.5lf, %.5lf), high: (%.5lf, %.5lf, %.5lf)",
		MarkNum::GetId(f148),

		box_fA_low.x(),
		box_fA_low.y(),
		box_fA_low.z(),

		box_fA_high.x(),
		box_fA_high.y(),
		box_fA_high.z()
	);

	LOG_INFO("face B (%d) Box: low: (%.5lf, %.5lf, %.5lf), high: (%.5lf, %.5lf, %.5lf)",
		MarkNum::GetId(f77),

		box_fA_low.x(),
		box_fA_low.y(),
		box_fA_low.z(),

		box_fA_high.x(),
		box_fA_high.y(),
		box_fA_high.z()
	);

	LOG_INFO("Box A >> Box B: %d", (*box_fA) >> (*box_fB));
	LOG_INFO("Box A << Box B: %d", (*box_fA) << (*box_fB));
	LOG_INFO("Box A && Box B: %d", (*box_fA) && (*box_fB));

	SPAbox box_overlap = (*box_fA) & (*box_fB);
	SPAposition box_overlap_low = box_overlap.low();
	SPAposition box_overlap_high = box_overlap.high();

	LOG_INFO("Box overlap: low: (%.5lf, %.5lf, %.5lf), high: (%.5lf, %.5lf, %.5lf)",
		box_overlap_low.x(),
		box_overlap_low.y(),
		box_overlap_low.z(),

		box_overlap_high.x(),
		box_overlap_high.y(),
		box_overlap_high.z()
	);

	// 交图
	BODY *int_graph;
	api_fafa_int(f148, f77, int_graph);

	// 看看WIRE里面有什么
	ENTITY_LIST int_graph_edge_list;
	ENTITY_LIST int_graph_coedge_list;
	ENTITY_LIST int_graph_loop_list;
	ENTITY_LIST int_graph_face_list;
	ENTITY_LIST int_graph_wire_list;
	ENTITY_LIST int_graph_shell_list;
	ENTITY_LIST int_graph_lump_list;

	api_get_edges(dynamic_cast<ENTITY*>(int_graph), int_graph_edge_list);
	api_get_coedges(dynamic_cast<ENTITY*>(int_graph), int_graph_coedge_list);
	api_get_loops(dynamic_cast<ENTITY*>(int_graph), int_graph_loop_list);
	api_get_faces(dynamic_cast<ENTITY*>(int_graph), int_graph_face_list);
	api_get_wires(dynamic_cast<ENTITY*>(int_graph), int_graph_wire_list);
	api_get_shells(dynamic_cast<ENTITY*>(int_graph), int_graph_shell_list);
	api_get_lumps(dynamic_cast<ENTITY*>(int_graph), int_graph_lump_list);

	LOG_INFO("int_graph_edge_list count: %d", int_graph_edge_list.count());
	LOG_INFO("int_graph_face_list count: %d", int_graph_face_list.count());
	LOG_INFO("int_graph_wire_list count: %d", int_graph_wire_list.count());
	LOG_INFO("int_graph_shell_list count: %d", int_graph_shell_list.count());
	LOG_INFO("int_graph_lump_list count: %d", int_graph_lump_list.count());

	for (int j = 0; j < int_graph_edge_list.count(); j++)
	{
		EDGE* iedge = dynamic_cast<EDGE*>(int_graph_edge_list[j]);
		VERTEX* st = iedge->start();
		VERTEX* ed = iedge->end();

		LOG_INFO("int_graph_edge_list[%d]: st: %d, ed: %d", j, st, ed);

		if (st == ed) {
			LOG_INFO("iedge: st == ed");
		}

		LOG_INFO("Geometry Check for iedge");
		GeometryUtils::PrintEdgeGeometry(iedge);

		// icoedge partner检查
		COEDGE* icoedge = iedge->coedge();

		do {
			if (icoedge == nullptr) {
				LOG_ERROR("icoedge: nullptr");
				break;
			}

			COEDGE* icoedge_prev = icoedge->previous();
			COEDGE* icoedge_next = icoedge->next();
			LOOP* icoedge_loop = icoedge->loop();
			//FACE* icoedge_face = icoedge_loop->face(); // ERROR：会导致程序崩溃，看上去就是没有

			LOG_INFO("icoedge: %d (prev: %d, next: %d), iloop: %d",
				icoedge,
				icoedge->previous(),
				icoedge->next(),
				icoedge->loop()
			);

			icoedge = icoedge->partner();
		} while (icoedge && icoedge != iedge->coedge());

	}

	LOG_INFO("end.");
}

void Exp4::Exp4::FaceFaceIntersectionExperiment4()
{
	LOG_INFO("start.");

	// get f 148 && f 77
	FACE *fA, *fB;

	for (int i = 0; i < this->bodies_to_be_checked.count(); i++)
	{
		ENTITY_LIST face_list;
		api_get_faces(this->bodies_to_be_checked[i], face_list);

		for (int j = 0; j < face_list.count(); j++) {
			FACE* face_p = dynamic_cast<FACE*>(face_list[j]);

			if (MarkNum::GetId(face_p) == 99) {
				fA = face_p;
			}

			if (MarkNum::GetId(face_p) == 149) { // (4,11)NO (5,10)YES
				fB = face_p;
			}
		}
	}

	GeometryUtils::PrintFaceGeometry(fA);
	GeometryUtils::PrintFaceGeometry(fB);

	// 包围盒
	SPAbox* box_fA = fA->bound();
	SPAbox* box_fB = fB->bound();

	SPAposition box_fA_low = box_fA->low();
	SPAposition box_fA_high = box_fA->high();

	SPAposition box_fB_low = box_fB->low();
	SPAposition box_fB_high = box_fB->high();

	LOG_INFO("face A (%d) Box: low: (%.5lf, %.5lf, %.5lf), high: (%.5lf, %.5lf, %.5lf)",
		MarkNum::GetId(fA),

		box_fA_low.x(),
		box_fA_low.y(),
		box_fA_low.z(),

		box_fA_high.x(),
		box_fA_high.y(),
		box_fA_high.z()
	);

	LOG_INFO("face B (%d) Box: low: (%.5lf, %.5lf, %.5lf), high: (%.5lf, %.5lf, %.5lf)",
		MarkNum::GetId(fB),

		box_fA_low.x(),
		box_fA_low.y(),
		box_fA_low.z(),

		box_fA_high.x(),
		box_fA_high.y(),
		box_fA_high.z()
	);

	LOG_INFO("Box A >> Box B: %d", (*box_fA) >> (*box_fB));
	LOG_INFO("Box A << Box B: %d", (*box_fA) << (*box_fB));
	LOG_INFO("Box A && Box B: %d", (*box_fA) && (*box_fB));

	SPAbox box_overlap = (*box_fA) & (*box_fB);
	SPAposition box_overlap_low = box_overlap.low();
	SPAposition box_overlap_high = box_overlap.high();

	LOG_INFO("Box overlap: low: (%.5lf, %.5lf, %.5lf), high: (%.5lf, %.5lf, %.5lf)",
		box_overlap_low.x(),
		box_overlap_low.y(),
		box_overlap_low.z(),

		box_overlap_high.x(),
		box_overlap_high.y(),
		box_overlap_high.z()
	);

	// 交图
	BODY *int_graph;
	api_fafa_int(fA, fB, int_graph);

	// 看看WIRE里面有什么
	ENTITY_LIST int_graph_edge_list;
	ENTITY_LIST int_graph_coedge_list;
	ENTITY_LIST int_graph_loop_list;
	ENTITY_LIST int_graph_face_list;
	ENTITY_LIST int_graph_wire_list;
	ENTITY_LIST int_graph_shell_list;
	ENTITY_LIST int_graph_lump_list;

	api_get_edges(dynamic_cast<ENTITY*>(int_graph), int_graph_edge_list);
	api_get_coedges(dynamic_cast<ENTITY*>(int_graph), int_graph_coedge_list);
	api_get_loops(dynamic_cast<ENTITY*>(int_graph), int_graph_loop_list);
	api_get_faces(dynamic_cast<ENTITY*>(int_graph), int_graph_face_list);
	api_get_wires(dynamic_cast<ENTITY*>(int_graph), int_graph_wire_list);
	api_get_shells(dynamic_cast<ENTITY*>(int_graph), int_graph_shell_list);
	api_get_lumps(dynamic_cast<ENTITY*>(int_graph), int_graph_lump_list);

	LOG_INFO("int_graph_edge_list count: %d", int_graph_edge_list.count());
	LOG_INFO("int_graph_face_list count: %d", int_graph_face_list.count());
	LOG_INFO("int_graph_wire_list count: %d", int_graph_wire_list.count());
	LOG_INFO("int_graph_shell_list count: %d", int_graph_shell_list.count());
	LOG_INFO("int_graph_lump_list count: %d", int_graph_lump_list.count());

	for (int j = 0; j < int_graph_edge_list.count(); j++)
	{
		EDGE* iedge = dynamic_cast<EDGE*>(int_graph_edge_list[j]);
		VERTEX* st = iedge->start();
		VERTEX* ed = iedge->end();

		LOG_INFO("int_graph_edge_list[%d]: st: %d, ed: %d", j, st, ed);

		if (st == ed) {
			LOG_INFO("iedge: st == ed");
		}

		LOG_INFO("Geometry Check for iedge");
		GeometryUtils::PrintEdgeGeometry(iedge);

		// icoedge partner检查
		COEDGE* icoedge = iedge->coedge();

		do {
			if (icoedge == nullptr) {
				LOG_ERROR("icoedge: nullptr");
				break;
			}

			COEDGE* icoedge_prev = icoedge->previous();
			COEDGE* icoedge_next = icoedge->next();
			LOOP* icoedge_loop = icoedge->loop();
			//FACE* icoedge_face = icoedge_loop->face(); // ERROR：会导致程序崩溃，看上去就是没有

			LOG_INFO("icoedge: %d (prev: %d, next: %d), iloop: %d",
				icoedge,
				icoedge->previous(),
				icoedge->next(),
				icoedge->loop()
			);

			icoedge = icoedge->partner();
		} while (icoedge && icoedge != iedge->coedge());

	}

	LOG_INFO("end.");
}

/*
	拿到这个面的几何后输出
*/
void Exp4::Exp4::StartExperiment()
{
	LOG_INFO("start.");

	this->PrintFacesGeometry();
	//this->FaceFaceIntersectionExperiment();
	//this->FaceFaceIntersectionExperiment2();
	//this->FaceFaceIntersectionExperiment3();
	this->FaceFaceIntersectionExperiment4();

	LOG_INFO("end.");
}
