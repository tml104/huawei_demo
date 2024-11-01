#include "StdAfx.h"
#include "UtilityFunctions.h"

int Utils::CoedgeCount(EDGE* iedge) {
	COEDGE* icoedge = iedge->coedge();
	int cnt = 0;
	do {
		if (icoedge == nullptr) {
			LOG_ERROR("icoedge is nullptr: iedge:%d, icoedge:%d", MarkNum::GetId(iedge), MarkNum::GetId(icoedge));
			cnt = -1;
			break;
		}
		cnt++;

		icoedge = icoedge->partner();
	} while (icoedge != nullptr && icoedge != iedge->coedge());

	return cnt;
}

int Utils::PartnerCount(COEDGE * icoedge_init)
{
	COEDGE* icoedge = icoedge_init;
	int cnt = 0;
	do {
		if (icoedge == nullptr) {
			LOG_ERROR("icoedge is nullptr: iedge:%d, icoedge : %d", MarkNum::GetId(icoedge->edge()), MarkNum::GetId(icoedge));
			cnt = -1;
			break;
		}
		cnt++;

		icoedge = icoedge->partner();
	} while (icoedge != nullptr && icoedge != icoedge_init);

	return cnt;
}

int Utils::LoopLength(LOOP * lp)
{
	COEDGE* icoedge = lp->start();
	int length = 0;
	do {
		if (icoedge == nullptr)
		{
			LOG_ERROR("icoedge == nullptr");
			return -1;
		}

		++length;

		icoedge = icoedge->next();
	} while (icoedge && icoedge!=lp->start());

	return length;
}

std::vector<COEDGE*> Utils::CoedgeOfEdge(EDGE * iedge)
{
	std::vector<COEDGE*> coedge_vec;

	COEDGE* icoedge = iedge->coedge();
	do {
		if (icoedge == nullptr) {
			LOG_ERROR("icoedge is nullptr: iedge:%d, icoedge:%d", MarkNum::GetId(iedge), MarkNum::GetId(icoedge));
			break;
		}

		coedge_vec.emplace_back(icoedge);

		icoedge = icoedge->partner();
	} while (icoedge != nullptr && icoedge != iedge->coedge());

	return coedge_vec;
}

EDGE* Utils::CopyEdge(EDGE * in_edge)
{
	EDGE* new_edge = nullptr;

	auto res_outcome = api_edge(in_edge, new_edge);
	check_outcome(res_outcome);
	auto res_logical = res_outcome.ok();
	if (res_logical == false) {
		LOG_ERROR("api_edge -> res_logical: false");

		err_mess_type err_number = res_outcome.error_number();

		LOG_ERROR("Error: %s", find_err_mess(err_number));
		throw std::runtime_error("Create new edge error :res_logical == false");

		return nullptr;
	}
	return new_edge;
}

#ifdef USE_QSTRING
void Utils::SaveToSAT(QString file_path, ENTITY_LIST &bodies) {
	FileInfo info;
	info.set_units(1.0);
	info.set_product_id("HQH1");
	outcome result = api_set_file_info((FileId_ | FileUnits), info);
	check_outcome(result);

	FILE *fp = fopen(file_path.toAscii().data(), "w");
	if (fp != NULL) {
		api_save_entity_list(fp, TRUE, bodies);
		LOG_INFO("file saved: %s", file_path.toAscii().data());
	}
	else {
		LOG_ERROR("file open failed.");
	}
	fclose(fp);
}
#endif

void Utils::SaveToSAT(const std::string& file_path, ENTITY_LIST & bodies)
{
	//Utils::SaveToSAT(QString(file_path.c_str()), bodies);

	FileInfo info;
	info.set_units(1.0);
	info.set_product_id("HQH1");

#ifdef IN_HUAWEI
	outcome result = api_set_file_info((FileIdent | FileUnits), info);
#else
	outcome result = api_set_file_info((FileId_ | FileUnits), info);
#endif
	check_outcome(result);

	FILE *fp = fopen(file_path.c_str(), "w");
	if (fp != NULL) {
		api_save_entity_list(fp, TRUE, bodies);
		LOG_INFO("file saved: %s", file_path.c_str());
	}
	else {
		LOG_ERROR("file open failed.");
	}
	fclose(fp);
}

#ifdef USE_QSTRING
void Utils::SaveToSATBody(QString file_path, BODY * body) {
	ENTITY_LIST save_list;
	save_list.add(body);

	Utils::SaveToSAT(
		file_path,
		save_list
	);
}
#endif

void Utils::SaveToSATBody(const std::string& file_path, BODY * body)
{
	//Utils::SaveToSATBody(QString(file_path.c_str()), body);

	ENTITY_LIST save_list;
	save_list.add(body);

	Utils::SaveToSAT(
		file_path,
		save_list
	);
}

#ifdef USE_QSTRING
std::tuple<std::string, std::string, std::string> Utils::SplitPath(QString file_path)
{
	std::string file_path_string(file_path.toAscii().data());
	std::size_t bot_dir_pos = file_path_string.find_last_of("/\\");
	std::string path = file_path_string.substr(0, bot_dir_pos);
	std::string file_name = file_path_string.substr(bot_dir_pos + 1);
	std::size_t dot_pos = file_name.find_last_of(".");
	std::string file_name_first = file_name.substr(0, dot_pos);
	std::string file_name_second = file_name.substr(dot_pos + 1);

	return std::make_tuple(path, file_name_first, file_name_second);
}
#endif

std::tuple<std::string, std::string, std::string> Utils::SplitPath(std::string file_path)
{
	std::size_t bot_dir_pos = file_path.find_last_of("/\\");
	std::string path = file_path.substr(0, bot_dir_pos);
	std::string file_name = file_path.substr(bot_dir_pos + 1);
	std::size_t dot_pos = file_name.find_last_of(".");
	std::string file_name_first = file_name.substr(0, dot_pos);
	std::string file_name_second = file_name.substr(dot_pos + 1);

	return std::make_tuple(path, file_name_first, file_name_second);
}

void Utils::SaveModifiedBodies(const std::tuple<std::string, std::string, std::string>& split_path_tuple, ENTITY_LIST& bodies)
{
	std::string path = std::get<0>(split_path_tuple);
	std::string file_name_first = std::get<1>(split_path_tuple);
	std::string file_name_second = std::get<2>(split_path_tuple);

	std::string file_path_string_to_be_saved = path + "/" + file_name_first + "_mod." + file_name_second;
	Utils::SaveToSAT((file_path_string_to_be_saved), bodies);
	LOG_INFO("file_path_string_to_be_saved: %s", file_path_string_to_be_saved.c_str());
}

void Utils::SaveModifiedBodiesRespectly(const std::tuple<std::string, std::string, std::string>& split_path_tuple, ENTITY_LIST& bodies)
{
	std::string path = std::get<0>(split_path_tuple);
	std::string file_name_first = std::get<1>(split_path_tuple);
	std::string file_name_second = std::get<2>(split_path_tuple);

	for (int i = 0; i < bodies.count(); i++)
	{
		std::string file_path_string_to_be_saved = path + "/" + file_name_first + "_mod_body_" + std::to_string(static_cast<long long>(i)) + "." + file_name_second;

		Utils::SaveToSATBody(file_path_string_to_be_saved, dynamic_cast<BODY*>(bodies[i]));

		LOG_INFO("file_path_string_to_be_saved: %s", file_path_string_to_be_saved.c_str());
	}
}

SPAvector Utils::GetNormalFromPoints(SPAposition p1, SPAposition p2, SPAposition p3)
{
	SPAvector u( p2.x() - p1.x() , p2.y() - p1.y(), p2.z() - p1.z() );
	SPAvector v( p3.x() - p1.x() , p3.y() - p1.y(), p3.z() - p1.z() );

	SPAvector ans(
		u.y() * v.z() - u.z() * v.y(),
		u.z() * v.x() - u.x() * v.z(),
		u.x() * v.y() - u.y() * v.x()
	);
	
	return ans;
}

void Utils::SaveSTL(const std::string & stl_file_path, std::vector<SPAposition>& out_mesh_points, std::vector<SPAunit_vector>& out_mesh_normals, std::vector<ENTITY*>& out_faces)
{
	std::fstream f;
	f.open(stl_file_path, std::ios::out | std::ios::trunc);

	if (!f.is_open()) {
		throw std::runtime_error("Open stl_file_path failed.");
	}

	f << "solid default:import_1\n";

	for (int i = 2; i < out_mesh_points.size(); i+=3) {
		SPAposition p1 = out_mesh_points[i];
		SPAposition p2 = out_mesh_points[i - 1];
		SPAposition p3 = out_mesh_points[i - 2];

		SPAvector normal = GetNormalFromPoints(p1, p2, p3);

		ENTITY* owner = nullptr;
		if (out_faces[i] == out_faces[i - 1] && out_faces[i] == out_faces[i - 2]) {
			owner = out_faces[i];
		}
		else {
			LOG_ERROR("vertex owner not consistent!");
		}

		f << "\tfacet normal " << normal.x() << " " << normal.y() << " " << normal.z() << "\n";
		f << "\t\touter loop\n";
		f << "\t\t\tvertex " << p1.x() << " " << p1.y() << " " << p1.z() << "\n";
		f << "\t\t\tvertex " << p2.x() << " " << p2.y() << " " << p2.z() << "\n";
		f << "\t\t\tvertex " << p3.x() << " " << p3.y() << " " << p3.z() << "\n";
		f << "\t\tendloop\n";
		f << "\tendfacet\n";
	}

	f << "endsolid default:import_1\n";
	f.close();
}

void Utils::SAT2STL(const std::tuple<std::string, std::string, std::string>& split_path_tuple, ENTITY_LIST& bodies) 
{
	LOG_INFO("Start.");

	for (int i = 0; i < bodies.count(); i++) {
		BODY* ibody = dynamic_cast<BODY*> (bodies[i]);

		std::vector<SPAposition> out_mesh_points; 
		std::vector<SPAunit_vector> out_mesh_normals;
		std::vector<ENTITY*> out_faces;

		MyMeshManager::MyFacet(ibody, out_mesh_points, out_mesh_normals, out_faces);

		std::string path = std::get<0>(split_path_tuple);
		std::string file_name_first = std::get<1>(split_path_tuple);
		std::string file_name_second = std::get<2>(split_path_tuple);
		std::string file_path_string_to_be_saved = path + "/" + file_name_first + "_stl_" + std::to_string(static_cast<long long>(i)) + ".stl";

		SaveSTL(file_path_string_to_be_saved, out_mesh_points, out_mesh_normals, out_faces);

		LOG_INFO("Points count: %d", out_mesh_points.size());
		LOG_INFO("Faces count: %d", out_faces.size());

		LOG_INFO("Saved STL file for body[%d]: %s", i, file_path_string_to_be_saved.c_str());
	}

	LOG_INFO("End.");
}

void Utils::SAT2STL(const std::tuple<std::string, std::string, std::string>& split_path_tuple, ENTITY_LIST& bodies, const std::set<int>& selected_bodies)
{
	LOG_INFO("Start.");

	for (int i = 0; i < bodies.count(); i++) {
		BODY* ibody = dynamic_cast<BODY*> (bodies[i]);

		if (selected_bodies.count(MarkNum::GetId(ibody)) == 0) {
			continue;
		}

		std::vector<SPAposition> out_mesh_points;
		std::vector<SPAunit_vector> out_mesh_normals;
		std::vector<ENTITY*> out_faces;

		MyMeshManager::MyFacet(ibody, out_mesh_points, out_mesh_normals, out_faces);

		std::string path = std::get<0>(split_path_tuple);
		std::string file_name_first = std::get<1>(split_path_tuple);
		std::string file_name_second = std::get<2>(split_path_tuple);
		std::string file_path_string_to_be_saved = path + "/" + file_name_first + "_stl_" + std::to_string(static_cast<long long>(i)) + ".stl";

		SaveSTL(file_path_string_to_be_saved, out_mesh_points, out_mesh_normals, out_faces);

		LOG_INFO("Points count: %d", out_mesh_points.size());
		LOG_INFO("Faces count: %d", out_faces.size());

		LOG_INFO("Saved STL file for body[%d]: %s", i, file_path_string_to_be_saved.c_str());
	}

	LOG_INFO("End.");
}

/*
	DEPRECATE: 没用
*/
void Utils::EntityList2STL(const std::tuple<std::string, std::string, std::string>& split_path_tuple, ENTITY_LIST & bodies)
{
	LOG_INFO("Start.");

	std::vector<SPAposition> out_mesh_points;
	std::vector<SPAunit_vector> out_mesh_normals;
	std::vector<ENTITY*> out_faces;

	MyMeshManager::MyFacetEntityList(bodies[0]->owner(), bodies, out_mesh_points, out_mesh_normals, out_faces); // 这能行吗……好的并不可以

	std::string path = std::get<0>(split_path_tuple);
	std::string file_name_first = std::get<1>(split_path_tuple);
	std::string file_name_second = std::get<2>(split_path_tuple);
	std::string file_path_string_to_be_saved = path + "/" + file_name_first + "_stl_entity_list.stl";

	SaveSTL(file_path_string_to_be_saved, out_mesh_points, out_mesh_normals, out_faces);

	LOG_INFO("Points count: %d", out_mesh_points.size());
	LOG_INFO("Faces count: %d", out_faces.size());

	LOG_INFO("Saved STL file for body entity list: %s", file_path_string_to_be_saved.c_str());

	LOG_INFO("End.");
}
