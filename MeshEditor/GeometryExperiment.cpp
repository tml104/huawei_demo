#include "StdAfx.h"
#include "GeometryExperiment.h"

std::set<EDGE*> GeometryExperiment::Singleton::bad_edge_set;

/*
	打印边的几何（curve）信息
*/
void GeometryExperiment::PrintEdgeGeometry(EDGE * e)
{
	LOG_INFO("start.");

	CURVE* edge_curve = e->geometry();
	const char* edge_curve_type = edge_curve->type_name();
	
	// 打印边与其类型
	LOG_INFO(
		"edge: %d, type: %s",
		MarkNum::GetId(e),
		edge_curve_type
	);

	// 打印坐标
	SPAposition start_vertex_coords = e->start()->geometry()->coords();
	SPAposition end_vertex_coords = e->end()->geometry()->coords();


	LOG_INFO(
		"edge: %d, start_vertex: (%.6lf %.6lf %.6lf) ",
		MarkNum::GetId(e),
		start_vertex_coords.x(),
		start_vertex_coords.y(),
		start_vertex_coords.z()
	);

	LOG_INFO("edge: %d, end_vertex: (%.6lf %.6lf %.6lf) ",
		MarkNum::GetId(e),
		end_vertex_coords.x(),
		end_vertex_coords.y(),
		end_vertex_coords.z()
	);

	// 打印几何
	if (strcmp(edge_curve_type, "intcurve") == 0){
		INTCURVE* intcur = dynamic_cast<INTCURVE*> (edge_curve);
		bs3_curve bc3 = intcur->def.cur(); // 空间曲线
		bs2_curve pc1 = intcur->def.pcur1(); // 面交线（参数曲线）
		bs2_curve pc2 = intcur->def.pcur2(); // 面交线（参数曲线）

		int num_pts = 0;
		int max_points = 100;

		// bc3: 取得控制点
		SPAposition* ctrlpts = new SPAposition[max_points];
		bs3_curve_control_points(bc3, num_pts, ctrlpts);

		// -> 打印控制点的数量
		
		LOG_INFO("edge: %d, type: %s, bc3_num_pts: %d",
			MarkNum::GetId(e),
			edge_curve_type,
			num_pts
		);

		if (num_pts > max_points) {
			throw std::runtime_error("num_pts > max_points"); // 潜在的内存泄漏风险……
		}

		// -> 打印控制点坐标
		for (int i = 0; i < num_pts; i++) {
			SPAposition& pt = ctrlpts[i];

			LOG_INFO("edge: %d, ctrlpts[%d]: (%.6lf, %.6lf, %.6lf) ", 
					MarkNum::GetId(e),
					i,
					pt.x(),
					pt.y(),
					pt.z()
			);
		}

		delete[] ctrlpts;

		// pc1: 取得控制点（uv坐标）
		SPApar_pos* ctrlpts1 = new SPApar_pos[max_points];
		bs2_curve_control_points(pc1, num_pts, ctrlpts1);

		// -> 打印控制点（uv）的数量
		LOG_INFO("edge: %d, type: %s, bc2_1_num_pts: %d",
				MarkNum::GetId(e),
				edge_curve_type,
				num_pts
			);

		if (num_pts > max_points) {
			throw std::runtime_error("num_pts > max_points");// 潜在的内存泄漏风险……
		}

		// -> 打印控制点（uv）坐标
		for (int i = 0; i < num_pts; i++) {
			SPApar_pos& pt = ctrlpts1[i];
			LOG_INFO(
				"edge: %d, ctrlpts1[%d] : (%.6lf, %.6lf, -- - )",
				MarkNum::GetId(e),
				i,
				static_cast<double>(pt.u),
				static_cast<double>(pt.v)
			);
		}

		delete[] ctrlpts1;

		// pc2: 取得控制点（uv坐标）
		SPApar_pos* ctrlpts2 = new SPApar_pos[max_points];
		bs2_curve_control_points(pc2, num_pts, ctrlpts2);

		// -> 打印控制点（uv）的数量
		LOG_INFO(
			"edge: %d, type: %s, bc2_2_num_pts: %d",
				MarkNum::GetId(e),
				edge_curve_type,
				num_pts
			);

		if (num_pts > max_points) {
			throw std::runtime_error("num_pts > max_points"); // 潜在的内存泄漏风险……
		}

		// -> 打印控制点（uv）坐标
		for (int i = 0; i < num_pts; i++) {
			SPApar_pos& pt = ctrlpts2[i];
			LOG_INFO("edge: %d, ctrlpts2[%d]: (%.6lf, %.6lf, ---) ",
					MarkNum::GetId(e),
					i,
					static_cast<double>(pt.u),
					static_cast<double>(pt.v)
				);
		}

		delete[] ctrlpts2;
	}
	else if (strcmp(edge_curve_type, "ellipse") == 0) {
		ELLIPSE* edge_curve_ellipse = dynamic_cast<ELLIPSE*> (edge_curve);
		ellipse edge_curve_ellipse_def = edge_curve_ellipse->def;

		// TODO:
		LOG_INFO("edge: %d, type: %s, centre: (%.6lf, %.6lf, %.6lf), normal: (%.6lf, %.6lf, %.6lf), major axis: (%.6lf, %.6lf, %.6lf), major length: %.6lf, minor length: %.6lf", 
				MarkNum::GetId(e),
				edge_curve_type,
				edge_curve_ellipse_def.centre.x(),
				edge_curve_ellipse_def.centre.y(),
				edge_curve_ellipse_def.centre.z(),
				edge_curve_ellipse_def.normal.x(),
				edge_curve_ellipse_def.normal.y(),
				edge_curve_ellipse_def.normal.z(),
				edge_curve_ellipse_def.major_axis.x(),
				edge_curve_ellipse_def.major_axis.y(),
				edge_curve_ellipse_def.major_axis.z(),
				edge_curve_ellipse_def.major_axis_length,
				edge_curve_ellipse_def.major_axis_length * edge_curve_ellipse_def.radius_ratio
			);
	}
	else if (strcmp(edge_curve_type, "straight") == 0) {
		STRAIGHT* edge_curve_straight = dynamic_cast<STRAIGHT*>(edge_curve);
		straight edge_curve_straight_def = edge_curve_straight->def; // 我这不是只能含泪修改straight.hxx中的权限？

		LOG_INFO("edge: %d, type: %s, root_point: (%.6lf, %.6lf, %.6lf), direction: (%.6lf, %.6lf, %.6lf)",
				MarkNum::GetId(e),
				edge_curve_type,
				edge_curve_straight->root_point().x(),
				edge_curve_straight->root_point().y(),
				edge_curve_straight->root_point().z(),
				edge_curve_straight->direction().x(),
				edge_curve_straight->direction().y(),
				edge_curve_straight->direction().z()
			);
	}
	else if (strcmp(edge_curve_type, "helix") == 0) {
		LOG_INFO("edge: %d, type: %s, HELIX! ",
				MarkNum::GetId(e),
				edge_curve_type
			);
	}
	else if (strcmp(edge_curve_type, "undefc") == 0) {
		LOG_INFO("edge: %d, type:%s, UNDEFC! ", MarkNum::GetId(e), edge_curve_type);
	}
	else {
		LOG_INFO("edge: %d, tpye:%s, UNKNOWN! ", MarkNum::GetId(e), edge_curve_type);
	}


	LOG_INFO("end.");
}

void GeometryExperiment::PrintFaceGeometry(FACE * f)
{
	LOG_INFO("start.");

	SURFACE* face_surface = f->geometry();
	const char* face_surface_type = face_surface->type_name();

	// 打印面与其类型
	LOG_INFO("face: %d, type: %s ",
		MarkNum::GetId(f),
		face_surface_type
	);

	// 打印几何
	if (strcmp(face_surface_type, "cone") == 0) {
		CONE* face_surface_cone = dynamic_cast<CONE*>(face_surface);

		LOG_INFO(
			"face: %d, type: %s, root_point: (%.6lf, %.6lf, %.6lf), major_axis: (%.6lf, %.6lf, %.6lf), radius_ratio: %.6lf, direction: (%.6lf, %.6lf, %.6lf), angle: (cos: %.6lf, sin: %.6lf, angle: %.6lf) ",
				MarkNum::GetId(f),
				face_surface_type,

				face_surface_cone->root_point().x(),
				face_surface_cone->root_point().y(),
				face_surface_cone->root_point().z(),

				face_surface_cone->major_axis().x(),
				face_surface_cone->major_axis().y(),
				face_surface_cone->major_axis().z(),

				face_surface_cone->radius_ratio(),

				face_surface_cone->direction().x(),
				face_surface_cone->direction().y(),
				face_surface_cone->direction().z(),

				face_surface_cone->cosine_angle(),
				face_surface_cone->sine_angle(),
				asin(face_surface_cone->sine_angle()) * 180 / MYPI
			);
	}
	else if (strcmp(face_surface_type, "meshsurf") == 0) {
		LOG_INFO(
			"face: %d, type: %s, MESHSURF! ",
				MarkNum::GetId(f),
				face_surface_type
			);
	}
	else if (strcmp(face_surface_type, "plane") == 0) {
		PLANE* face_surface_plane = dynamic_cast<PLANE*>(face_surface);

		LOG_INFO("face: %d, type: %s, root_point: (%.6lf, %.6lf, %.6lf), normal: (%.6lf, %.6lf, %.6lf) ",
				MarkNum::GetId(f),
				face_surface_type,

				face_surface_plane->root_point().x(),
				face_surface_plane->root_point().y(),
				face_surface_plane->root_point().z(),

				face_surface_plane->normal().x(),
				face_surface_plane->normal().y(),
				face_surface_plane->normal().z()
			);
	}
	else if (strcmp(face_surface_type, "sphere") == 0) {
		LOG_INFO("face: %d, type: %s, SPHERE! ",
				MarkNum::GetId(f),
				face_surface_type
			);
	}
	else if (strcmp(face_surface_type, "spline") == 0) {
		LOG_INFO("face: %d, type: %s, SPLINE! ",
				MarkNum::GetId(f),
				face_surface_type
			);
	}
	else if (strcmp(face_surface_type, "torus") == 0) {
		TORUS* face_surface_torus = dynamic_cast<TORUS*> (face_surface);

		LOG_INFO("face: %d, type: %s, centre: (%.6lf, %.6lf, %.6lf), normal: (%.6lf, %.6lf, %.6lf), radius: (major: %.6lf, minor: %.6lf) ",
				MarkNum::GetId(f),
				face_surface_type,

				face_surface_torus->centre().x(),
				face_surface_torus->centre().y(),
				face_surface_torus->centre().z(),

				face_surface_torus->normal().x(),
				face_surface_torus->normal().y(),
				face_surface_torus->normal().z(),

				face_surface_torus->major_radius(),
				face_surface_torus->minor_radius()
			);
	}
	else {
		LOG_INFO("face: %d, type: %s, UNKNOWN! ",
				MarkNum::GetId(f),
				face_surface_type
			);
	}

	LOG_INFO("end.");
}

/*
	更新缺边集合（这个的逻辑类似于NonManifold::FindNonManifold）
*/
void GeometryExperiment::UpdateBadCoedgeSet(ENTITY_LIST & bodies)
{
	LOG_INFO("start.");

	for (int i = 0; i < bodies.count(); i++) {
		ENTITY* ibody = (bodies[i]);

		ENTITY_LIST edge_list;
		api_get_edges(ibody, edge_list);

		for (int j = 0; j < edge_list.count(); j++) {

			EDGE* iedge = dynamic_cast<EDGE*>(edge_list[j]);
			int coedge_count = Utils::CoedgeCount(iedge);

			// 注意判定条件的不同
			if (coedge_count < 2){ // 破边
				LOG_INFO("Bad edge found: iedge: %d, iedge_coedge: %d, iedge_loop: %d, coedge_cnt: %d",
					MarkNum::GetId(iedge),
					MarkNum::GetId(iedge->coedge()),
					MarkNum::GetId(iedge->coedge()->loop()),
					coedge_count
				);

				GeometryExperiment::Singleton::bad_edge_set.insert(iedge);

			}
			else if (coedge_count > 2) { // 非流形（跳过）
				LOG_INFO("NonManifold edge found, skipped: iedge: %d, coedge_cnt: %d",
					MarkNum::GetId(iedge),
					coedge_count
				);
			}
		}
	}

	LOG_INFO("end.");
}


/*
	遍历iloop上所有边并依次输出边的几何信息（从给定的begin_coedge开始，如果不存在则直接从iloop->start()取）。如果traverse_incident_loops为true则会在稍后递归调用此函数，遍历相邻loop的信息。
*/
void GeometryExperiment::TraverseLoops(LOOP * iloop, bool traverse_incident_loops, COEDGE* begin_coedge)
{
	LOG_INFO("start.");

	std::vector<LOOP*> incident_loops_vec;

	// 如果指定的起始coedge不存在，则随便从当前待遍历iloop上面挑一个coedge
	if (begin_coedge == nullptr) {
		begin_coedge = iloop->start(); 
	}

	LOG_INFO("iloop: %d, traverse_incident_loops: %s, begin_coedge: %d ",
			MarkNum::GetId(iloop),
			traverse_incident_loops ? "true" : "false",
			MarkNum::GetId(begin_coedge)
		);

	// 遍历环上的coedge，获取edge的几何信息……
	COEDGE* jcoedge = begin_coedge;
	do {
		if (jcoedge == nullptr) {
			break;
		}

		// 打印当前遍历的coedge的edge的几何信息
		EDGE* jedge = jcoedge->edge();

		LOG_INFO("jcoedge: %d, jedge: %d ",
				MarkNum::GetId(jcoedge),
				MarkNum::GetId(jedge)
			);

		GeometryExperiment::PrintEdgeGeometry(jedge);

		// 如果traverse_incident_loops为true则遍历相邻loop,将他们的指针塞入incident_loops_vec中
		if (traverse_incident_loops){
			COEDGE* kcoedge = jcoedge->partner();
			do {
				// 注意排除自身循环的情况
				if (kcoedge == nullptr || kcoedge == jcoedge) {
					break;
				}

				LOOP* kloop = kcoedge->loop();

				if (kloop != iloop) // 再排除一次（非必要）
				{
					incident_loops_vec.emplace_back(kloop);
				}

				kcoedge = kcoedge->partner();
			} while (kcoedge != nullptr && kcoedge != jcoedge->partner());
		}

		jcoedge = jcoedge->next();
	} while (jcoedge != nullptr && jcoedge != begin_coedge);


	// 获取当前iloop的face的几何信息
	FACE* iface = iloop->face();
	GeometryExperiment::PrintFaceGeometry(iface);

	// 如果traverse_incident_loops为true则遍历相邻loop的信息
	// （先把这个去掉，否则信息过于杂乱）
	//if (traverse_incident_loops) {
	//	for (auto& incident_loop_it = incident_loops_vec.begin(); incident_loop_it != incident_loops_vec.end(); incident_loop_it++){
	//		GeometryExperiment::TraverseLoops(*incident_loop_it, false); // 递归（但是接下来就不递归了）
	//	}
	//}

	LOG_INFO("end.");
}

/*
	实验流程
*/
void GeometryExperiment::GeometryExperiment1(ENTITY_LIST & bodies, HoopsView* hv)
{
	LOG_INFO("start.");

	UpdateBadCoedgeSet(bodies);

	for (auto& bad_edge_it = GeometryExperiment::Singleton::bad_edge_set.begin(); bad_edge_it != GeometryExperiment::Singleton::bad_edge_set.end(); bad_edge_it++) {
		EDGE* iedge = (*bad_edge_it);
		COEDGE* icoedge = iedge->coedge();
		LOOP* iloop = icoedge->loop(); // 对坏边来说这个反正也只有一个吧

		TraverseLoops(iloop, true, icoedge);
	}

	LOG_INFO("end.");
}

void GeometryExperiment::Init(ENTITY_LIST & bodies, HoopsView* hv)
{
	api_initialize_constructors();
	api_initialize_booleans();

	// 实验1
	GeometryExperiment::GeometryExperiment1(bodies);

	api_terminate_constructors();
	api_terminate_booleans();
}
