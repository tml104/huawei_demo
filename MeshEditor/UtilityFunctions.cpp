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

void Utils::SaveToSAT(const std::string& file_path, ENTITY_LIST & bodies)
{
	Utils::SaveToSAT(QString(file_path.c_str()), bodies);
}

void Utils::SaveToSATBody(QString file_path, BODY * body) {
	ENTITY_LIST save_list;
	save_list.add(body);

	Utils::SaveToSAT(
		file_path,
		save_list
	);
}

void Utils::SaveToSATBody(const std::string& file_path, BODY * body)
{
	Utils::SaveToSATBody(QString(file_path.c_str()), body);
}

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


void Utils::SaveModifiedBodies(const std::tuple<std::string, std::string, std::string>& split_path_tuple, ENTITY_LIST& bodies)
{
	std::string path = std::get<0>(split_path_tuple);
	std::string file_name_first = std::get<1>(split_path_tuple);
	std::string file_name_second = std::get<2>(split_path_tuple);

	std::string file_path_string_to_be_saved = path + "/" + file_name_first + "_mod." + file_name_second;
	Utils::SaveToSAT(QString((file_path_string_to_be_saved).c_str()), bodies);
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

		Utils::SaveToSATBody(QString(file_path_string_to_be_saved.c_str()), dynamic_cast<BODY*>(bodies[i]));

		LOG_INFO("file_path_string_to_be_saved: %s", file_path_string_to_be_saved.c_str());
	}
}