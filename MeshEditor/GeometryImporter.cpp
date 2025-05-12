#include "StdAfx.h"
#include "GeometryImporter.h"

Json::Value GeometryImporter::LoadJsonFile(const std::string & json_path)
{
	std::fstream f;
	f.open(json_path, std::ios::in);

	if (!f.is_open()) {
		LOG_ERROR("json_path open failed.");
		throw std::runtime_error("json_path open failed.");
	}

	Json::Reader reader;
	Json::Value root;
	reader.parse(f, root, false);
	f.close();


	LOG_INFO("End.");
	return root;
}


// TEMP: only load nurbs face
void GeometryImporter::Importer::ModifyFaces(const Json::Value & root)
{
	Json::Value root_faces = root["root_faces"];

	for (int i = 0; i < root_faces.size(); i++) {

		Json::Value face_data = root_faces[i];

		int body_id = face_data["body"].asInt();
		int face_marknum = face_data["marknum"].asInt();
		Json::Value geometry_info_data = face_data["geometry_info"];
		Json::Value control_points_data = geometry_info_data["control_points"];

		FACE* f_ptr = MarkNum::GetPtr<FACE*>(std::make_pair("face", face_marknum));

		if (f_ptr != nullptr) {

			SURFACE* face_geometry = f_ptr->geometry();
			if (face_geometry) {
				const char* face_type_name = face_geometry->type_name();
				if (strcmp(face_type_name, "spline") == 0) {
					SPLINE* spline_geometry = dynamic_cast<SPLINE*>(face_geometry);

					bs3_surface bs3 = spline_geometry->def.sur();

					int num_u = 0, num_v = 0;
					const int MAX_POINTS_NUM = 999;

					// 可能可以跳过获取ctrlpts
					SPAposition* ctrlpts = new SPAposition[MAX_POINTS_NUM];
					bs3_surface_control_points(bs3, num_u, num_v, ctrlpts);

					std::vector<double> new_control_points;
					for (int j = 0; j < control_points_data.size(); j++) {
						Json::Value point_data = control_points_data[j];
						double x = point_data["x"].asDouble();
						double y = point_data["y"].asDouble();
						double z = point_data["z"].asDouble();
						
						new_control_points.push_back(x);
						new_control_points.push_back(y);
						new_control_points.push_back(z);
					}

					int weight_num_u = 0, weight_num_v = 0;
					double* weights = new double[MAX_POINTS_NUM];
					bs3_surface_weights(bs3, weight_num_u, weight_num_v, weights);

					bs3_surface_set_ctrlpts(bs3, num_u, num_v, new_control_points.data(), weights);

					delete[] ctrlpts;
					delete[] weights;
				}
			}
		}

	}

	LOG_INFO("End.");
}

void GeometryImporter::Importer::Start(const std::string & json_path)
{
	Json::Value root = LoadJsonFile(json_path);

	ModifyFaces(root);
}

