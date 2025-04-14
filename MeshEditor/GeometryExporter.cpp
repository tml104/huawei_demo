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

Json::Value GeometryExporter::Exporter::ExportGeometryInfo(int ibody_marknum)
{
	Json::Value root;

	Json::Value root_vertices;
	Json::Value root_edges;
	Json::Value root_coedges;
	Json::Value root_loops;
	Json::Value root_shells;
	Json::Value root_lumps;
	Json::Value root_bodies;

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

	}

	root["root_vertices"] = root_vertices;
	root["root_edges"] = root_edges;
	root["root_coedges"] = root_coedges;
	root["root_loops"] = root_loops;
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

		Json::Value geometry_json = ExportGeometryInfo(ibody_marknum);
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

		Json::Value geometry_json = ExportGeometryInfo(ibody_marknum);
		SaveJson(file_path_string_to_be_saved, geometry_json);
	}
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
	else
	{
		LOG_ERROR("coedge's geometry is null: coedge: %d", marknum);
	}

	root["property"] = root_property;

	return root;
}
