#include "StdAfx.h"
#include "MarkNum.h"

std::map<ENTITY*, std::pair<std::string, int>> MarkNum::Singleton::marknum_map;
int MarkNum::Singleton::marknum_body = 0;
int MarkNum::Singleton::marknum_lump = 0;
int MarkNum::Singleton::marknum_shell = 0;
int MarkNum::Singleton::marknum_wire = 0;
int MarkNum::Singleton::marknum_face = 0;
int MarkNum::Singleton::marknum_loop = 0;
int MarkNum::Singleton::marknum_coedge = 0;
int MarkNum::Singleton::marknum_edge = 0;
int MarkNum::Singleton::marknum_vertex = 0;

/*
	计数
	用于计数的map从指针映射到二元组<类型,编号>
*/
void MarkNum::Init(ENTITY_LIST & bodies){

	// 计时开始
	clock_t marknum_start_clock = std::clock();

	// for every body
	for (int i = 0; i < bodies.count(); i++) {

		ENTITY* ibody_ptr = (bodies[i]);

		if (MarkNum::Singleton::marknum_map.count(ibody_ptr) == 0) {
			MarkNum::Singleton::marknum_map[ibody_ptr] = std::make_pair("body", ++MarkNum::Singleton::marknum_body );
		}

		ENTITY_LIST lump_list;
		ENTITY_LIST shell_list;
		ENTITY_LIST wire_list;
		ENTITY_LIST face_list;
		ENTITY_LIST edge_list;
		ENTITY_LIST coedge_list;
		ENTITY_LIST vertex_list;
		ENTITY_LIST loop_list;

		api_get_lumps(ibody_ptr, lump_list);
		api_get_shells(ibody_ptr, shell_list);
		api_get_wires(ibody_ptr, wire_list);
		api_get_faces(ibody_ptr, face_list);
		api_get_edges(ibody_ptr, edge_list);
		api_get_coedges(ibody_ptr, coedge_list);
		api_get_vertices(ibody_ptr, vertex_list);
		api_get_loops(ibody_ptr, loop_list);

		// lump
		for (int j = 0; j < lump_list.count(); j++) {
			ENTITY* ptr = lump_list[j];
			if (MarkNum::Singleton::marknum_map.count(ptr) == 0) {
				MarkNum::Singleton::marknum_map[ptr] = std::make_pair( "lump", ++MarkNum::Singleton::marknum_lump );
			}
		}

		// shell
		for (int j = 0; j < shell_list.count(); j++) {
			ENTITY* ptr = shell_list[j];
			if (MarkNum::Singleton::marknum_map.count(ptr) == 0) {
				MarkNum::Singleton::marknum_map[ptr] = std::make_pair ("shell", ++MarkNum::Singleton::marknum_shell );
			}
		}

		// wire
		for (int j = 0; j < wire_list.count(); j++) {
			ENTITY* ptr = wire_list[j];
			if (MarkNum::Singleton::marknum_map.count(ptr) == 0) {
				MarkNum::Singleton::marknum_map[ptr] = std::make_pair( "wire", ++MarkNum::Singleton::marknum_wire );
			}
		}

		// face
		for (int j = 0; j < face_list.count(); j++) {
			ENTITY* ptr = face_list[j];
			if (MarkNum::Singleton::marknum_map.count(ptr) == 0) {
				MarkNum::Singleton::marknum_map[ptr] = std::make_pair( "face", ++MarkNum::Singleton::marknum_face );
			}
		}

		// edge
		for (int j = 0; j < edge_list.count(); j++) {
			ENTITY* ptr = edge_list[j];
			if (MarkNum::Singleton::marknum_map.count(ptr) == 0) {
				MarkNum::Singleton::marknum_map[ptr] = std::make_pair( "edge", ++MarkNum::Singleton::marknum_edge );
			}
		}

		// coedge
		for (int j = 0; j < coedge_list.count(); j++) {
			ENTITY* ptr = coedge_list[j];
			if (MarkNum::Singleton::marknum_map.count(ptr) == 0) {
				MarkNum::Singleton::marknum_map[ptr] = std::make_pair( "coedge", ++MarkNum::Singleton::marknum_coedge );
			}
		}

		// vertex
		for (int j = 0; j < vertex_list.count(); j++) {
			ENTITY* ptr = vertex_list[j];
			if (MarkNum::Singleton::marknum_map.count(ptr) == 0) {
				MarkNum::Singleton::marknum_map[ptr] = std::make_pair( "vertex", ++MarkNum::Singleton::marknum_vertex );
			}
		}

		// loop
		for (int j = 0; j < loop_list.count(); j++) {
			ENTITY* ptr = loop_list[j];
			if (MarkNum::Singleton::marknum_map.count(ptr) == 0) {
				MarkNum::Singleton::marknum_map[ptr] = std::make_pair("loop", ++MarkNum::Singleton::marknum_loop);
			}
		}

	}

	// 计时结束
	clock_t marknum_end_clock = std::clock();
	double marknum_time_cost = static_cast<double>(marknum_end_clock - marknum_start_clock) / CLOCKS_PER_SEC;
	LOG_INFO("marknum_time_cost: %.5lf sec", marknum_time_cost);


	LOG_INFO("body count:	%d\n", MarkNum::Singleton::marknum_body);
	LOG_INFO("lump count:	%d\n", MarkNum::Singleton::marknum_lump);
	LOG_INFO("shell count:	%d\n", MarkNum::Singleton::marknum_shell);
	LOG_INFO("wire count:	%d\n", MarkNum::Singleton::marknum_wire);
	LOG_INFO("face count:	%d\n", MarkNum::Singleton::marknum_face);
	LOG_INFO("edge count:	%d\n", MarkNum::Singleton::marknum_edge);
	LOG_INFO("coedge count:	%d\n", MarkNum::Singleton::marknum_coedge);
	LOG_INFO("vertex count:	%d\n", MarkNum::Singleton::marknum_vertex);
	LOG_INFO("loop count:	%d\n", MarkNum::Singleton::marknum_loop);

	//MarkNum::Debug::PrintMap();

	//if(MarkNum::Singleton::marknum_map.size() <= 1000)
	//{
	//	MarkNum::Debug::PrintMap();
	//}
	//else {
	//	DEBUGINFO
	//		printf("marknum_map size TOO MUCH (>1000)\n");
	//}
}

int MarkNum::GetId( ENTITY *const & ptr)
{
	if (MarkNum::Singleton::marknum_map.count(ptr) != 0) {
		return MarkNum::Singleton::marknum_map[ptr].second;
	}
	else {
		// 不存在？？你居然能不存在？？
		LOG_INFO("GetId Failed, Not exist: Pointer: %d", ptr);
	}

	return 0;
}

std::string MarkNum::GetTypeName( ENTITY *const & ptr)
{
	if (MarkNum::Singleton::marknum_map.count(ptr) != 0) {
		return MarkNum::Singleton::marknum_map[ptr].first;
	}
	else {
		// 不存在？？你居然能不存在？？
		LOG_INFO("GetTypeName Failed, Not exist: Pointer: %d", ptr);
	}

	return std::string();
}

#ifdef USE_HOOPSVIEW
void MarkNum::ShowEdgeMark(HoopsView* hv)
{
	if (hv == nullptr) {
		throw std::invalid_argument("hv is nullptr.");
	}

	auto get_mid_point = [&](SPAposition const &a, SPAposition const &b) {
		return SPAposition(
			(a.x() + b.x()) / 2,
			(a.y() + b.y()) / 2,
			(a.z() + b.z()) / 2
		);
	};

	std::vector<SPAposition> avoid_text_collision_position_vec;
	const double avoid_text_collision_thereshold = 0.0001;
	const double avoid_text_collision_move_distance = 0.01;
	auto check_text_collision = [&](const SPAposition& mid_pos) -> bool {
		for (int i = 0; i < avoid_text_collision_position_vec.size(); i++) {
			if (std::abs(mid_pos.x() - avoid_text_collision_position_vec[i].x()) < avoid_text_collision_thereshold &&
				std::abs(mid_pos.y() - avoid_text_collision_position_vec[i].y()) < avoid_text_collision_thereshold &&
				std::abs(mid_pos.z() - avoid_text_collision_position_vec[i].z()) < avoid_text_collision_thereshold
				) {
				return true;
			}
		}

		return false;
	};
	

	for (auto& it = MarkNum::Singleton::marknum_map.begin(); it != MarkNum::Singleton::marknum_map.end(); it++)
	{
		auto& key_pointer = it->first;

		auto &mark_pair = it->second;
		auto &type = mark_pair.first;
		auto &mark_num = mark_pair.second;

		auto mark_num_str = std::to_string(static_cast<long long>(mark_num));

		if (type == "edge") {
			EDGE* edge_key_pointer = dynamic_cast<EDGE*>(key_pointer);

			SPAposition start_vertex_coords = edge_key_pointer->start()->geometry()->coords();
			SPAposition end_vertex_coords = edge_key_pointer->end()->geometry()->coords();
			SPAposition mid_vertex_coords = get_mid_point(start_vertex_coords, end_vertex_coords);

			LOG_DEBUG("edge: %d, st: (%.5lf, %.5lf, %.5lf), ed: (%.5lf, %.5lf, %.5lf), mid: (%.5lf, %.5lf, %.5lf)",
				mark_num,
				start_vertex_coords.x(),
				start_vertex_coords.y(),
				start_vertex_coords.z(),
				end_vertex_coords.x(),
				end_vertex_coords.y(),
				end_vertex_coords.z(),
				mid_vertex_coords.x(),
				mid_vertex_coords.y(),
				mid_vertex_coords.z()
			);

			// 在中间点坐标处渲染
			// 暂时用一个粗暴的，基于n^2遍历的防碰撞机制（反正我只是为了调试的时候不让编号挤到一坨用的）

			if (check_text_collision(mid_vertex_coords)) {
				mid_vertex_coords.z() += 0.01;
			}

			hv->render_text(mid_vertex_coords, &mark_num_str[0], "blue");

			avoid_text_collision_position_vec.emplace_back(mid_vertex_coords);
		}
	}
}

void MarkNum::ShowEdgeMark(HoopsView* hv, std::set<int>& show_edge_marknum_set)
{
	if (hv == nullptr) {
		throw std::invalid_argument("hv is nullptr.");
	}

	auto get_mid_point = [&](SPAposition const &a, SPAposition const &b) {
		return SPAposition(
			(a.x() + b.x()) / 2,
			(a.y() + b.y()) / 2,
			(a.z() + b.z()) / 2
		);
	};

	std::vector<SPAposition> avoid_text_collision_position_vec;
	const double avoid_text_collision_thereshold = 0.0001;

	for (auto& it = MarkNum::Singleton::marknum_map.begin(); it != MarkNum::Singleton::marknum_map.end(); it++)
	{
		auto& key_pointer = it->first;

		auto &mark_pair = it->second;
		auto &type = mark_pair.first;
		auto &mark_num = mark_pair.second;

		auto mark_num_str = std::to_string(static_cast<long long>(mark_num));

		if (type == "edge" && show_edge_marknum_set.count(mark_num)) {
			EDGE* edge_key_pointer = dynamic_cast<EDGE*>(key_pointer);

			SPAposition start_vertex_coords = edge_key_pointer->start()->geometry()->coords();
			SPAposition end_vertex_coords = edge_key_pointer->end()->geometry()->coords();
			SPAposition mid_vertex_coords = get_mid_point(start_vertex_coords, end_vertex_coords);

			// 在中间点坐标处渲染
			// 暂时用一个粗暴的，基于n^2遍历的防碰撞机制（反正我只是为了调试的时候不让编号挤到一坨用的）

			for (int i = 0; i < avoid_text_collision_position_vec.size(); i++) {
				if (
					abs(avoid_text_collision_position_vec[i].x() - mid_vertex_coords.x()) < avoid_text_collision_thereshold
					&& abs(avoid_text_collision_position_vec[i].y() - mid_vertex_coords.y()) < avoid_text_collision_thereshold
					&& abs(avoid_text_collision_position_vec[i].z() - mid_vertex_coords.z()) < avoid_text_collision_thereshold
					) {
					mid_vertex_coords.z() += 0.01;
				}
			}

			hv->render_text(mid_vertex_coords, &mark_num_str[0], "blue");

			avoid_text_collision_position_vec.emplace_back(mid_vertex_coords);
		}
	}
}

void MarkNum::ShowFaceMark(HoopsView* hv) {
	if (hv == nullptr) {
		throw std::invalid_argument("hv is nullptr.");
	}

	std::vector<SPAposition> avoid_text_collision_position_vec;
	const double avoid_text_collision_thereshold = 0.0001;
	const double avoid_text_collision_move_distance = 0.01;
	auto check_text_collision = [&](const SPAposition& mid_pos) -> bool {
		for (int i = 0; i < avoid_text_collision_position_vec.size(); i++) {
			if (std::abs(mid_pos.x() - avoid_text_collision_position_vec[i].x())< avoid_text_collision_thereshold &&
				std::abs(mid_pos.y() - avoid_text_collision_position_vec[i].y())< avoid_text_collision_thereshold &&
				std::abs(mid_pos.z() - avoid_text_collision_position_vec[i].z()) < avoid_text_collision_thereshold
			) {
				return true;
			}
		}

		return false;
	};

	for (auto& it = MarkNum::Singleton::marknum_map.begin(); it != MarkNum::Singleton::marknum_map.end(); it++)
	{
		auto& key_pointer = it->first;

		auto &mark_pair = it->second;
		auto &type = mark_pair.first;
		auto &mark_num = mark_pair.second;

		auto mark_num_str = std::to_string(static_cast<long long>(mark_num));

		if (type == "face") {
			FACE* face_key_pointer = dynamic_cast<FACE*>(key_pointer);

			SURFACE* face_surface = face_key_pointer->geometry();
			const char* face_surface_type = face_surface->type_name();

			// CONE
			if (strcmp(face_surface_type, "cone") == 0 ) {
				CONE* face_surface_cone = dynamic_cast<CONE*>(face_surface);

				SPAposition mid_pos = face_surface_cone->root_point();
				if (check_text_collision(mid_pos)) {
					mid_pos.z() += avoid_text_collision_move_distance;
					LOG_DEBUG("face text collision: %d", mark_num);
				}

				mark_num_str = "c" + mark_num_str;
				hv->render_text(mid_pos, &mark_num_str[0], "red");
				LOG_DEBUG("face: %d, mid_pos: (%.5lf, %.5lf, %.5lf)", mark_num, mid_pos.x(), mid_pos.y(), mid_pos.z());

				avoid_text_collision_position_vec.emplace_back(mid_pos);

			} // PLANE
			else if (strcmp(face_surface_type, "plane") == 0) {
				//PLANE* face_surface_cone = dynamic_cast<PLANE*>(face_surface);

				//SPAposition mid_pos = face_surface_cone->root_point();
				//if (check_text_collision(mid_pos)) {
				//	mid_pos.z() += avoid_text_collision_move_distance;
				//	LOG_DEBUG("face text collision: %d", mark_num);
				//}

				//mark_num_str = "p" + mark_num_str;
				//hv->render_text(mid_pos, &mark_num_str[0], "red");
				//LOG_DEBUG("face: %d, mid_pos: (%.5lf, %.5lf, %.5lf)", mark_num, mid_pos.x(), mid_pos.y(), mid_pos.z());

				//avoid_text_collision_position_vec.emplace_back(mid_pos);
			}
		}
	}
}

#endif

void MarkNum::Clear()
{
	LOG_INFO("MarkNum Clear start.");

	MarkNum::Singleton::marknum_map.clear();
	MarkNum::Singleton::marknum_body = 0;
	MarkNum::Singleton::marknum_lump = 0;
	MarkNum::Singleton::marknum_shell = 0;
	MarkNum::Singleton::marknum_wire = 0;
	MarkNum::Singleton::marknum_face = 0;
	MarkNum::Singleton::marknum_loop = 0;
	MarkNum::Singleton::marknum_coedge = 0;
	MarkNum::Singleton::marknum_edge = 0;
	MarkNum::Singleton::marknum_vertex = 0;

	LOG_INFO("MarkNum Clear end.");
}

void MarkNum::Debug::PrintMap()
{
	LOG_INFO("PrintMap start.");

	int cnt = 0;
	static const unsigned int BUF_SIZE = 5000;
	static char buf[BUF_SIZE];
	for (auto& it = MarkNum::Singleton::marknum_map.begin(); it != MarkNum::Singleton::marknum_map.end(); it++) 
	{
		auto &mark_pair = it->second;
		auto &type = mark_pair.first;
		auto &mark_num = mark_pair.second;
		std::string final_line_contents;

		snprintf(buf, BUF_SIZE, "type: %s,\t mark_num: %d,\t pointer: %d; \t", type.c_str(), mark_num, it->first);
		final_line_contents += buf;

		// 如果type是coedge，打印一下对应的edge
		if (type == "coedge")
		{
			EDGE* coedge_edge_pointer = dynamic_cast<COEDGE*>(it->first)->edge();

			snprintf(buf, BUF_SIZE, "COEDGE->EDGE() Pointer: %d %d; \t", coedge_edge_pointer, MarkNum::GetId(coedge_edge_pointer));
			final_line_contents += buf;
		}
		else if (type == "edge") { // 如果type是edge
			EDGE* edge_pointer = dynamic_cast<EDGE*>(it->first);

			// 打印顶点
			snprintf(buf, BUF_SIZE, "EDGE->VERTEX: (start: %d, end: %d); \t", MarkNum::GetId(edge_pointer->start()), MarkNum::GetId(edge_pointer->end()));
			final_line_contents += buf;

			// 打印其所属coedge
			auto coedge_vec = Utils::CoedgeOfEdge(edge_pointer);
			snprintf(buf, BUF_SIZE, "EDGE->coedge_vec (size: %d): ", static_cast<int>(coedge_vec.size()));
			final_line_contents += buf;
			for (int i = 0; i < coedge_vec.size(); i++) {
				//LOG_DEBUG("%d, ", MarkNum::GetId(coedge_vec[i]));
				snprintf(buf, BUF_SIZE, "%d, ", MarkNum::GetId(coedge_vec[i]));
				final_line_contents += buf;
			}
			final_line_contents += ";";
		}

		LOG_DEBUG(final_line_contents.c_str());
	}

	LOG_INFO("PrintMap end.");
}