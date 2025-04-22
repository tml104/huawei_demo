#include "StdAfx.h"
#include "GeometryExporter.h"

void GeometryExporter::SaveJson(const std::string & file_name, const Json::Value & root)
{
	std::fstream f;
	f.open(file_name, std::ios::out | std::ios::trunc);
	if (f.is_open() == false) {
		LOG_DEBUG("Save Json: Open file failed.");
	}

	std::string res = root.toStyledString();
	f << res << std::endl;

	LOG_INFO("Save Json: %s", file_name.c_str());
}

Json::Value GeometryExporter::Exporter::GetGeometryJson(int ibody_marknum)
{
	Json::Value root;

	Json::Value basic_statistics;
	Json::Value root_vertices;
	Json::Value root_edges;
	Json::Value root_coedges;
	Json::Value root_loops;
	Json::Value root_faces;
	Json::Value root_shells;
	Json::Value root_lumps;
	Json::Value root_bodies; // ���body���ܷ�������Ϊbody��ʵֻ��Ҫ���ж��ٸ�����Ϣ�����ӣ���������Ԥ������� 


	// ͳ��
	basic_statistics["marknum_body"] = MarkNum::Singleton::marknum_body;
	basic_statistics["marknum_lump"] = MarkNum::Singleton::marknum_lump;
	basic_statistics["marknum_shell"] = MarkNum::Singleton::marknum_shell;
	basic_statistics["marknum_wire"] = MarkNum::Singleton::marknum_wire;
	basic_statistics["marknum_face"] = MarkNum::Singleton::marknum_face;
	basic_statistics["marknum_loop"] = MarkNum::Singleton::marknum_loop;
	basic_statistics["marknum_coedge"] = MarkNum::Singleton::marknum_coedge;
	basic_statistics["marknum_edge"] = MarkNum::Singleton::marknum_edge;
	basic_statistics["marknum_vertex"] = MarkNum::Singleton::marknum_vertex;


	// ʵ����Ϣ
	for (auto it = MarkNum::Singleton::marknum_map.begin(); it != MarkNum::Singleton::marknum_map.end(); it++) {
		auto &mark_pair = it->second;
		auto &type = mark_pair.first;
		auto &mark_num = mark_pair.second;

		// TODO: ������Ϣ
		if (type == "vertex") {
			VERTEX* ptr = dynamic_cast<VERTEX*>(it->first);
			int body_marknum = MarkNum::GetBody(ptr);

			if (body_marknum == ibody_marknum)
			{
				root_vertices.append(VertexToJson(ptr, mark_num));
			}

		}
		else if (type == "edge") {
			EDGE* ptr = dynamic_cast<EDGE*>(it->first);
			int body_marknum = MarkNum::GetBody(ptr);

			if (body_marknum == ibody_marknum)
			{
				root_edges.append(EdgeToJson(ptr, mark_num));
			}
		}
		else if (type == "coedge")
		{
			COEDGE* ptr = dynamic_cast<COEDGE*>(it->first);
			int body_marknum = MarkNum::GetBody(ptr);

			if (body_marknum == ibody_marknum)
			{
				root_coedges.append(CoedgeToJson(ptr, mark_num));
			}
		}
		else if (type == "face")
		{
			FACE* ptr = dynamic_cast<FACE*>(it->first);
			int body_marknum = MarkNum::GetBody(ptr);

			if (body_marknum == ibody_marknum)
			{
				root_faces.append(FaceToJson(ptr, mark_num));
			}
		}
		else if (type == "loop")
		{
			LOOP* ptr = dynamic_cast<LOOP*>(it->first);
			int body_marknum = MarkNum::GetBody(ptr);

			if (body_marknum == ibody_marknum)
			{
				root_loops.append(LoopToJson(ptr, mark_num));
			}
		}

		
	}

	root["basic_statistics"] = basic_statistics;
	root["root_vertices"] = root_vertices;
	root["root_edges"] = root_edges;
	root["root_coedges"] = root_coedges;
	root["root_loops"] = root_loops;
	root["root_faces"] = root_faces;
	root["root_shells"] = root_shells;
	root["root_lumps"] = root_lumps;
	root["root_bodies"] = root_bodies;

	return root;
}

void GeometryExporter::Exporter::Start(const std::tuple<std::string, std::string, std::string>& split_path_tuple, const std::set<int>& selected_bodies)
{
	std::string path = std::get<0>(split_path_tuple);
	std::string file_name_first = std::get<1>(split_path_tuple);
	std::string file_name_second = std::get<2>(split_path_tuple);

	Utils::SAT2STL(split_path_tuple, bodies, selected_bodies);

	for (int i = 0; i < bodies.count(); i++)
	{
		BODY* ibody = dynamic_cast<BODY*>(bodies[i]);
		int ibody_marknum = MarkNum::GetId(ibody);

		if (selected_bodies.count(ibody_marknum) == 0) {
			continue;
		}

		std::string file_path_string_to_be_saved = path + "/" + file_name_first + "_geometry_json_" + std::to_string(static_cast<long long>(i)) + ".json";

		Json::Value geometry_json = GetGeometryJson(ibody_marknum);
		SaveJson(file_path_string_to_be_saved, geometry_json);
	}
}

void GeometryExporter::Exporter::Start(const std::tuple<std::string, std::string, std::string>& split_path_tuple)
{
	std::string path = std::get<0>(split_path_tuple);
	std::string file_name_first = std::get<1>(split_path_tuple);
	std::string file_name_second = std::get<2>(split_path_tuple);

	// �ڴ˴�����stl��������Ҫ�޸ģ�
	Utils::SAT2STL(split_path_tuple, bodies);

	for (int i = 0; i < bodies.count(); i++)
	{
		BODY* ibody = dynamic_cast<BODY*>(bodies[i]);
		int ibody_marknum = MarkNum::GetId(ibody);

		std::string file_path_string_to_be_saved = path + "/" + file_name_first + "_geometry_json_" + std::to_string(static_cast<long long>(i)) + ".json";

		Json::Value geometry_json = GetGeometryJson(ibody_marknum);
		SaveJson(file_path_string_to_be_saved, geometry_json);
	}
}

Json::Value GeometryExporter::Exporter::GetDebugShowPointJson()
{
	Json::Value root;

	const auto& debug_points_map = DebugShow::Singleton::debug_points_map;

	for (auto it = debug_points_map.begin(); it != debug_points_map.end(); it++) {

		const std::string& name = it->first;
		const SPAposition& pos = it->second;

		Json::Value point_json;

		point_json["name"] = name;
		point_json["x"] = pos.x();
		point_json["y"] = pos.y();
		point_json["z"] = pos.z();

		root.append(point_json);
	}

	return root;
}

Json::Value GeometryExporter::Exporter::GetDebugShowJson()
{
	Json::Value root;

	root["debug_points"] = GetDebugShowPointJson();

	return root;
}

void GeometryExporter::Exporter::ExportDebugPoints(const std::tuple<std::string, std::string, std::string>& split_path_tuple)
{
	std::string path = std::get<0>(split_path_tuple);
	std::string file_name_first = std::get<1>(split_path_tuple);
	std::string file_name_second = std::get<2>(split_path_tuple);

	std::string file_path_string_to_be_saved = path + "/" + file_name_first + "_debugshow.json";

	Json::Value debugshow_json = GetDebugShowJson();

	SaveJson(file_path_string_to_be_saved, debugshow_json);
}

Json::Value GeometryExporter::Exporter::SPApositionToJson(const SPAposition & pos)
{
	Json::Value root;

	root["x"] = pos.x();
	root["y"] = pos.y();
	root["z"] = pos.z();

	return root;
}

Json::Value GeometryExporter::Exporter::SPAparposToJson(const SPApar_pos & pos)
{
	Json::Value root;

	SPAparameter u = pos.u;
	double uu = u.operator double(); // static_cast<double>(u);

	SPAparameter v = pos.v;
	double vv = v.operator double();

	root["u"] = uu;
	root["v"] = vv;

	return root;
}

Json::Value GeometryExporter::Exporter::SPAunitvectorToJson(const SPAunit_vector & unit_vec)
{
	Json::Value root;
	
	root["x"] = unit_vec.x();
	root["y"] = unit_vec.y();
	root["z"] = unit_vec.z();

	return root;
}

Json::Value GeometryExporter::Exporter::SPAvectorToJson(const SPAvector& vec)
{
	Json::Value root;

	root["x"] = vec.x();
	root["y"] = vec.y();
	root["z"] = vec.z();

	return root;
}

Json::Value GeometryExporter::Exporter::VertexToJson(VERTEX * vertex, int marknum)
{
	Json::Value root;
	root["marknum"] = marknum;
	root["body"] = MarkNum::GetBody(vertex);

	APOINT* p = vertex->geometry();

	if (p != nullptr) {

		SPAposition coords = p->coords();
		root["point"] = SPApositionToJson(coords);
	}

	return root;
}

Json::Value GeometryExporter::Exporter::EdgeToJson(EDGE * edge, int marknum)
{
	Json::Value root;
	root["marknum"] = marknum;
	root["body"] = MarkNum::GetBody(edge);

	int nonmanifold_count = Utils::CoedgeCount(edge);
	root["nonmanifold_count"] = nonmanifold_count;

	VERTEX* st = edge->start();
	VERTEX* ed = edge->end();
	int st_marknum = (st == nullptr) ? (-1) : (MarkNum::GetId(st));
	int ed_marknum = (ed == nullptr) ? (-1) : (MarkNum::GetId(ed));

	root["st_marknum"] = st_marknum;
	root["ed_marknum"] = ed_marknum;

	CURVE* cur = edge->geometry();

	// �������������ֱ�߿��Բ��ò���
	// TODO: ����Ӧ��������
	if (cur != nullptr) {
		root["curve_type"] = cur->type_name();

		std::vector<SPAposition> sampled_points = GeometryUtils::SampleEdge(edge);

		Json::Value root_sampled_points;
		for (int h = 0; h < sampled_points.size(); h++) {
			root_sampled_points.append(SPApositionToJson(sampled_points[h]));
		}
		root["sampled_points"] = root_sampled_points;

		Json::Value root_property;

		if (strcmp(cur->type_name(), "intcurve") == 0) {
			INTCURVE* intcur = dynamic_cast<INTCURVE*> (cur);
			bs3_curve bc3 = intcur->def.cur(); // �ռ�����
			bs2_curve pc1 = intcur->def.pcur1(); // �潻�ߣ��������ߣ�
			bs2_curve pc2 = intcur->def.pcur2(); // �潻�ߣ��������ߣ�

			int num_pts = 0;
			int max_points = 9999;

			// bc3: ȡ�ÿ��Ƶ�
			SPAposition* ctrlpts = new SPAposition[max_points];
			bs3_curve_control_points(bc3, num_pts, ctrlpts);

			if (num_pts > max_points) {
				throw std::runtime_error("num_pts > max_points"); // Ǳ�ڵ��ڴ�й©���ա���
			}

			Json::Value root_ctrlpts;
			for (int i = 0; i < num_pts; i++) {
				root_ctrlpts.append(SPApositionToJson(ctrlpts[i]));
			}
			root_property["ctrlpts"] = root_ctrlpts;

			delete[] ctrlpts;

			// pc1

			if (pc1)
			{
				root_property["pc1"] = true;
				LOG_DEBUG("pc1 exist");
			}
			else
			{
				LOG_DEBUG("pc1 noexist");
			}

			if (pc2)
			{
				root_property["pc2"] = true;
				LOG_DEBUG("pc2 exist");
			}
			else
			{
				LOG_DEBUG("pc2 noexist");
			}

		}
		else if (strcmp(cur->type_name(), "ellipse") == 0) {
			ELLIPSE* edge_curve_ellipse = dynamic_cast<ELLIPSE*> (cur);
			ellipse edge_curve_ellipse_def = edge_curve_ellipse->def;

			root_property["centre_x"] = edge_curve_ellipse_def.centre.x();
			root_property["centre_y"] = edge_curve_ellipse_def.centre.y();
			root_property["centre_z"] = edge_curve_ellipse_def.centre.z();

			root_property["normal_x"] = edge_curve_ellipse_def.normal.x();
			root_property["normal_y"] = edge_curve_ellipse_def.normal.y();
			root_property["normal_z"] = edge_curve_ellipse_def.normal.z();

			root_property["major_axis_x"] = edge_curve_ellipse_def.major_axis.x();
			root_property["major_axis_y"] = edge_curve_ellipse_def.major_axis.y();
			root_property["major_axis_z"] = edge_curve_ellipse_def.major_axis.z();

			root_property["major_length"] = edge_curve_ellipse_def.major_axis_length;
			root_property["minor_length"] = edge_curve_ellipse_def.major_axis_length * edge_curve_ellipse_def.radius_ratio;

		}
		else if (strcmp(cur->type_name(), "straight") == 0) {
			STRAIGHT* edge_curve_straight = dynamic_cast<STRAIGHT*>(cur);
			straight edge_curve_straight_def = edge_curve_straight->def;

			root_property["root_point_x"] = edge_curve_straight->root_point().x();
			root_property["root_point_y"] = edge_curve_straight->root_point().y();
			root_property["root_point_z"] = edge_curve_straight->root_point().z();

			root_property["direction_x"] = edge_curve_straight->direction().x();
			root_property["direction_y"] = edge_curve_straight->direction().y();
			root_property["direction_z"] = edge_curve_straight->direction().z();
		}
		else if (strcmp(cur->type_name(), "helix") == 0) {
			// TODO:
			LOG_INFO("cur type: HELIX");
		}
		else if (strcmp(cur->type_name(), "undefc") == 0) {
			// TODO:
			LOG_INFO("cur type: UNDEFC");
		}
		else {
			LOG_INFO("cur type: UNKNOWN");
		}

		root["property"] = root_property;
	}

	return root;
}

Json::Value GeometryExporter::Exporter::CoedgeToJson(COEDGE * coedge, int marknum)
{
	Json::Value root;
	root["marknum"] = marknum;
	root["body"] = MarkNum::GetBody(coedge);

	EDGE* edge = coedge->edge();
	int edge_marknum = MarkNum::GetId(edge); // return 0 if edge == nullptr
	root["edge_marknum"] = edge_marknum;

	LOOP* loop = coedge->loop();
	int loop_marknum = MarkNum::GetId(loop);
	root["loop_marknum"] = loop_marknum;

	if (loop)
	{
		FACE* face = loop->face();
		int face_marknum = MarkNum::GetId(face);
		root["face_marknum"] = face_marknum;
	}
	else
	{
		root["face_marknum"] = 0;
	}

	root["sense"] = coedge->sense(); // reverse is true, forward is false

	Json::Value root_property;

	// ע���������ȡСд��ģ���д�������пհ�
	PCURVE* pc = coedge->geometry();

	if (pc != nullptr)
	{
		root_property["curve_name"] = pc->type_name();// �ƺ�һ����pcurve
		
		if (strcmp(pc->type_name(), "pcurve") == 0) {
			bs2_curve bs2 = pc->equation().cur();

			int bs2_deg = bs2_curve_degree(bs2);
			root_property["curve_degree"] = bs2_deg;

			// ���Ƶ�
			int num_pts;
			int max_points = 999;
			SPApar_pos* ctrlpts = new SPApar_pos[max_points];

			bs2_curve_control_points(bs2, num_pts, ctrlpts);
			if (num_pts > max_points)
			{
				throw std::runtime_error("num_pts > max_points");
			}


			Json::Value root_ctrlpts;
			for (int i = 0; i < num_pts; i++)
			{
				root_ctrlpts.append(SPAparposToJson(ctrlpts[i]));
			}

			delete[] ctrlpts;

			root_property["ctrlpts"] = root_ctrlpts;
		
			// ���Ƶ� end

			// �ڵ�knots
			int num_kts;
			int max_kts = 999;
			double* knots = new double[max_kts];

			bs2_curve_knots(bs2, num_kts, knots);

			if (num_kts > max_kts)
			{
				throw std::runtime_error("num_kts > max_kts");
			}

			Json::Value root_knots;
			for (int i=0;i<num_kts;i++)
			{
				root_knots.append(knots[i]);
			}

			delete[] knots;

			root_property["knots"] = root_knots;
			// �ڵ�knots end
		}

	}
	else
	{
		LOG_ERROR("coedge's geometry is null: coedge: %d", marknum);
	}

	root["property"] = root_property;

	return root;
}

Json::Value GeometryExporter::Exporter::LoopToJson(LOOP * loop, int marknum)
{
	Json::Value root;
	root["marknum"] = marknum;
	root["body"] = MarkNum::GetBody(loop);

	//root["start_coedge_marknum"] = MarkNum::GetId(loop->start());
	root["face_marknum"] = MarkNum::GetId(loop->start());

	COEDGE* icoedge = loop->start();

	Json::Value root_coedge_list;

	// ��loop�е�����edge������
	do {

		if (icoedge == nullptr) {
			break;
		}

		EDGE* iedge = icoedge->edge();

		int coedge_marknum = MarkNum::GetId(icoedge);
		int edge_marknum = MarkNum::GetId(iedge);

		Json::Value coedge_info;
		coedge_info["coedge_marknum"] = coedge_marknum;
		coedge_info["edge_marknum"] = edge_marknum;

		root_coedge_list.append(coedge_info);

		icoedge = icoedge->next();
	} while (icoedge != nullptr && icoedge != loop->start());

	root["coedge_list"] = root_coedge_list;

	return root;
}

Json::Value GeometryExporter::Exporter::FaceToJson(FACE * face, int marknum)
{
	Json::Value root;
	root["marknum"] = marknum;
	root["body"] = MarkNum::GetBody(face);

	root["loop_marknum"] = MarkNum::GetId(face->loop());
	root["sense"] = face->sense();

	SURFACE* face_geometry = face->geometry();
	
	Json::Value geo_root;
	if (face_geometry) {
		const char* face_type_name = face_geometry->type_name();
		geo_root["face_type"] = face_type_name; // cone, meshsurf, plane, sphere, spline, and torus.

		if (strcmp(face_type_name, "cone") == 0) {
			CONE* cone_geometry = dynamic_cast<CONE*>(face_geometry);

			SPAvector major_axis = cone_geometry->major_axis();
			double ratio = cone_geometry->radius_ratio();
			SPAposition root_point = cone_geometry->root_point();
			SPAunit_vector direction = cone_geometry->direction();
			double cos = cone_geometry->cosine_angle();
			double sin = cone_geometry->sine_angle();

			geo_root["root_point"] = SPApositionToJson(root_point);
			geo_root["major_axis"] = SPAvectorToJson(major_axis);
			geo_root["radius_ratio"] = ratio;
			geo_root["direction"] = SPAunitvectorToJson(direction);

			geo_root["cos"] = cos;
			geo_root["sin"] = sin;
			geo_root["angle"] = asin(sin) * 180 / M_PI;
		}
		else if (strcmp(face_type_name, "meshsurf") == 0) {
			LOG_ERROR("meshsurf is not support face geometry type!");
		}
		else if (strcmp(face_type_name, "plane") == 0) {
			PLANE* plane_geometry = dynamic_cast<PLANE*>(face_geometry);

			SPAunit_vector normal = plane_geometry->normal();
			SPAposition root_point = plane_geometry->root_point();

			geo_root["root_point"] = SPApositionToJson(root_point);
			geo_root["normal"] = SPAunitvectorToJson(normal);
		}
		else if (strcmp(face_type_name, "sphere") == 0) {
			SPHERE* sphere_geometry = dynamic_cast<SPHERE*>(face_geometry);
			
			SPAposition centre = sphere_geometry->centre();
			double radius = sphere_geometry->radius();

			geo_root["centre"] = SPApositionToJson(centre);
			geo_root["radius"] = radius;
		}
		else if (strcmp(face_type_name, "spline") == 0) {
			SPLINE* spline_geometry = dynamic_cast<SPLINE*>(face_geometry);
			bs3_surface bs3 = spline_geometry->def.sur();

			// degree
			geo_root["degree_u"] = bs3_surface_degree_u(bs3);
			geo_root["degree_v"] = bs3_surface_degree_v(bs3);

			// control points
			Json::Value control_points;

			int num_u = 0, num_v = 0;
			const int MAX_POINTS_NUM = 999;

			SPAposition* ctrlpts = new SPAposition[MAX_POINTS_NUM];
			bs3_surface_control_points(bs3, num_u, num_v, ctrlpts);
			geo_root["num_u"] = num_u;
			geo_root["num_v"] = num_v;

			Json::Value points;
			for (int i = 0; i < num_u*num_v; i++) {
				points.append(SPApositionToJson(ctrlpts[i]));
			}
			geo_root["control_points"] = points;

			// knots
			int num_knots_u = 0, num_knots_v = 0;
			double* uknots = new double[MAX_POINTS_NUM];
			double* vknots = new double[MAX_POINTS_NUM];
			bs3_surface_knots_u(bs3, num_knots_u, uknots);
			bs3_surface_knots_v(bs3, num_knots_v, vknots);

			geo_root["num_knots_u"] = num_knots_u;
			geo_root["num_knots_v"] = num_knots_v;

			Json::Value knots_u, knots_v;
			for (int i = 0; i < num_knots_u; ++i) {
				knots_u.append(uknots[i]);
			}
			for (int i = 0; i < num_knots_v; ++i) {
				knots_v.append(vknots[i]);
			}
			
			geo_root["knots_u"] = knots_u;
			geo_root["knots_v"] = knots_v;

			// weight
			int weight_num_u = 0, weight_num_v = 0;
			double* weights = new double[MAX_POINTS_NUM];
			bs3_surface_weights(bs3, weight_num_u, weight_num_v, weights);

			geo_root["weight_num_u"] = weight_num_u;
			geo_root["weight_num_v"] = weight_num_v;

			Json::Value weights_json;
			for (int i = 0; i < weight_num_u*weight_num_v; i++) {
				weights_json.append(weights[i]);
			}
			geo_root["weights"] = weights_json;

			delete[] ctrlpts;
			delete[] uknots;
			delete[] vknots;
			delete[] weights;
		}
		else if (strcmp(face_type_name, "torus") == 0) {
			TORUS* trs_geometry = dynamic_cast<TORUS*>(face_geometry);

			SPAposition centre = trs_geometry->centre(); // ����
			double minor_radius = trs_geometry->minor_radius();
			double major_radius = trs_geometry->major_radius();
			SPAunit_vector normal = trs_geometry->normal();

			geo_root["centre"] = SPApositionToJson(centre);
			geo_root["normal"] = SPAunitvectorToJson(normal);
			geo_root["minor_radius"] = minor_radius;
			geo_root["major_radius"] = major_radius;
		}
		else {
			LOG_ERROR("face_type is unknown!");
		}

	}
	root["geometry_info"] = geo_root;

	return root;
}
