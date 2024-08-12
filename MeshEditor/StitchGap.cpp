#include "StdAfx.h"
#include "StitchGap.h"

Stitch::MyVector Stitch::GetMyVector(Stitch::MyPoint a, Stitch::MyPoint b)
{
	Stitch::MyVector temp;
	for (int i = 0; i < 3; i++) {
		temp[i] = b[i] - a[i];
	}

	return temp;
}

double Stitch::Distance(Stitch::MyPoint a, Stitch::MyPoint b)
{
	double distance = 0.0;
	for (int i = 0; i < 3; i++) {
		distance += (b[i] - a[i])*(b[i] - a[i]);
	}
	return std::sqrt(distance);
}

double Stitch::Distance(Stitch::MyVector v)
{
	double distance = 0.0;
	for (int i = 0; i < 3; i++) {
		distance += v[i] * v[i];
	}
	return  std::sqrt(distance);
}

Stitch::MyVector Stitch::Normalize(Stitch::MyVector v)
{
	double distance = Stitch::Distance(v);
	Stitch::MyVector temp;
	for (int i = 0; i < 3; i++) {
		temp[i] = v[i] / distance;
	}

	return temp;
}

double Stitch::Dot(Stitch::MyVector v1, Stitch::MyVector v2)
{
	double dot_result = 0.0;
	for (int i = 0; i < 3; i++) {
		dot_result += (v1[i] * v2[i]);
	}
	return dot_result;
}

/*
	计算两个给定PoorCoedge的匹配分数
	（在MatchTree::Match中会用到）

	目前修改为只和距离有关，角度只是用于辅助将一些其他情况排除
*/
double Stitch::CalculatePoorCoedgeScore(const PoorCoedge & poor_coedge1, const PoorCoedge & poor_coedge2)
{
	double score1 = 0.0;

	bool vaild_flag = true;

	for (int i = 0, j = Stitch::MIDPOINT_CNT - 1; i < Stitch::MIDPOINT_CNT; i++, j--) {
		double distance = Stitch::Distance(poor_coedge1.midpoint_coords[i], poor_coedge2.midpoint_coords[j]);

		LOG_DEBUG("distance of (i,j = %d, %d) : %.5lf", i, j, distance);

		score1 += distance;
		if (distance > Stitch::EPSLION1) {
			vaild_flag = false;
		}
	}

	// 对距离：归一化后计算平均分数（此时应该是一个小于1的数值），然后加权
	score1 /= Stitch::EPSLION1;
	score1 /= Stitch::MIDPOINT_CNT;

	Stitch::MyVector vec1 = Stitch::Normalize(Stitch::GetMyVector(poor_coedge1.midpoint_coords[0], poor_coedge1.midpoint_coords[Stitch::MIDPOINT_CNT - 1]));
	Stitch::MyVector vec2 = Stitch::Normalize(Stitch::GetMyVector(poor_coedge2.midpoint_coords[0], poor_coedge2.midpoint_coords[Stitch::MIDPOINT_CNT - 1]));
	double dot_result = Stitch::Dot(vec1, vec2);

	LOG_DEBUG("dot result: %.5lf", dot_result);

	if (dot_result > 0) {
		vaild_flag = false;
	}
	else {
		dot_result = -dot_result;
		if (dot_result < EPSLION2)
		{
			vaild_flag = false;
		}
	}

	if (vaild_flag) {
		LOG_DEBUG("vaild score. score1: %.5lf ", score1);
		return score1;
	}

	LOG_DEBUG("invaild score. score: -1.0");
	return -1.0;
}

/*
	调用顺序：1
	找破边，并保存到poor_coedge_vec中
*/
void Stitch::StitchGapFixer::FindPoorCoedge(ENTITY_LIST & bodies)
{
	LOG_INFO("start.");

	for (int i = 0; i < bodies.count(); i++) {
		ENTITY* ibody = (bodies[i]);

		// 遍历这个body的coedge list
		ENTITY_LIST coedge_list;
		api_get_coedges(ibody, coedge_list);

		// for coedge
		for (int j = 0; j < coedge_list.count(); j++) {
			COEDGE* coedge_ptr = static_cast<COEDGE*>(coedge_list[j]);

			int partner_count = Utils::PartnerCount(coedge_ptr);

			// 如果coedge的partnet数量是1，意味着找到poor coedge，保存到vec中
			if (partner_count == 1) {

				// [有效性检查] START

				if (coedge_ptr->start() == nullptr)
				{
					LOG_ERROR("poor coedge found, but NO START.");
					continue;
				}

				if (coedge_ptr->end() == nullptr)
				{
					LOG_ERROR("poor coedge found, but NO END.");
					continue;
				}

				if (coedge_ptr->edge() == nullptr)
				{
					LOG_ERROR("poor coedge found, but NO EDGE.");
					continue;
				}

				// ~~排除~~ 输出无几何的情况				
				if (coedge_ptr->geometry() == nullptr)
				{
					LOG_ERROR("coedge_ptr NO GEOMETRY: coedge_ptr: %d (edge: %d), vertex: %d %d",
						MarkNum::GetId(coedge_ptr),
						MarkNum::GetId(coedge_ptr->edge()),
						MarkNum::GetId(coedge_ptr->start()),
						MarkNum::GetId(coedge_ptr->end()));
					continue;
				}

				if (coedge_ptr->start()->geometry() == nullptr)
				{
					LOG_ERROR("coedge_ptr->start() NO GEOMETRY: coedge_ptr: %d (edge: %d), vertex: %d %d",
						MarkNum::GetId(coedge_ptr),
						MarkNum::GetId(coedge_ptr->edge()),
						MarkNum::GetId(coedge_ptr->start()),
						MarkNum::GetId(coedge_ptr->end()));
					continue;
				}

				if (coedge_ptr->end()->geometry() == nullptr)
				{
					LOG_ERROR("coedge_ptr->end() NO GEOMETRY: coedge_ptr: %d (edge: %d), vertex: %d %d",
						MarkNum::GetId(coedge_ptr),
						MarkNum::GetId(coedge_ptr->edge()),
						MarkNum::GetId(coedge_ptr->start()),
						MarkNum::GetId(coedge_ptr->end()));
					continue;
				}

				// [有效性检查] END

				LOG_DEBUG("poor coedge found: %d (edge: %d), vertex:%d %d",
					MarkNum::GetId(coedge_ptr),
					MarkNum::GetId(coedge_ptr->edge()),
					MarkNum::GetId(coedge_ptr->start()),
					MarkNum::GetId(coedge_ptr->end())
				);


				// 计算（默认5个）中间采样点，其中0、4是端点，2是真中点
				Stitch::PoorCoedge temp_poor_coedge;
				temp_poor_coedge.coedge = coedge_ptr;

				auto edge_range = coedge_ptr->edge()->param_range();
				auto coedge_range = (coedge_ptr->sense() == FORWARD) ? edge_range : -edge_range;

				for (int k = 0; k < MIDPOINT_CNT; k++) {
					// 起始点
					if (k == 0) {
						temp_poor_coedge.midpoint_coords[k][0] = coedge_ptr->start()->geometry()->coords().x();
						temp_poor_coedge.midpoint_coords[k][1] = coedge_ptr->start()->geometry()->coords().y();
						temp_poor_coedge.midpoint_coords[k][2] = coedge_ptr->start()->geometry()->coords().z();
						continue;
					}
					// 终点
					if (k == MIDPOINT_CNT - 1) {
						temp_poor_coedge.midpoint_coords[k][0] = coedge_ptr->end()->geometry()->coords().x();
						temp_poor_coedge.midpoint_coords[k][1] = coedge_ptr->end()->geometry()->coords().y();
						temp_poor_coedge.midpoint_coords[k][2] = coedge_ptr->end()->geometry()->coords().z();
						continue;
					}

					double interpolate_param = k * 1.0 / MIDPOINT_CNT;
					auto coedge_param = coedge_range.interpolate(interpolate_param);
					auto curve_param = (coedge_ptr->sense() == coedge_ptr->edge()->sense()) ? coedge_param : -coedge_param;
					auto interpolate_point = coedge_ptr->edge()->geometry()->equation().eval_position(curve_param);

					temp_poor_coedge.midpoint_coords[k][0] = interpolate_point.x();
					temp_poor_coedge.midpoint_coords[k][1] = interpolate_point.y();
					temp_poor_coedge.midpoint_coords[k][2] = interpolate_point.z();
				}

				// [DEBUG] 打印中间插值采样点的坐标值

				for (int k = 0; k < MIDPOINT_CNT; k++) {
					LOG_DEBUG("sample_point[%d]: %.5lf, %.5lf, %.5lf",
						k,
						temp_poor_coedge.midpoint_coords[k][0],
						temp_poor_coedge.midpoint_coords[k][1],
						temp_poor_coedge.midpoint_coords[k][2]
					);
				}


				poor_coedge_vec.emplace_back(temp_poor_coedge);

			}
		}
	}
	LOG_DEBUG("poor coedge total num: %d", static_cast<int>(poor_coedge_vec.size()));

	LOG_INFO("end.");
}

/*
	 调用顺序：2
	 匹配每条边，并且构建KDTree进行加速
	 这个匹配是贪心地匹配的（也就是说一旦匹配成功就把它放进对应的found_coedge_set，后续不再参与匹配），可能之后要改一下
*/
void Stitch::StitchGapFixer::MatchPoorCoedge()
{
	LOG_INFO("start.");

	// 建树（基于中点）
	match_tree.ConstructTree(poor_coedge_vec);

	for (int i = 0; i < poor_coedge_vec.size(); i++) {
		auto &e = poor_coedge_vec[i];

		if (found_coedge_set.count(e.coedge)) {
			continue;
		}

		auto match_poor_coedge_vec = match_tree.Match(e, found_coedge_set);

		if (match_poor_coedge_vec.size()) {
			// 取分数最小的<PoorCoedge, score>对
			auto first_match = match_poor_coedge_vec.front();

			// [debug]打印分数
			LOG_DEBUG("Best match id: %d %d, score: %.5lf",
				MarkNum::GetId(e.coedge),
				MarkNum::GetId(first_match.first.coedge),
				first_match.second
			);

			poor_coedge_pair_vec.emplace_back(std::make_pair(e, first_match.first));

			// 维护found_coedge_set
			found_coedge_set.insert(e.coedge);
			found_coedge_set.insert(first_match.first.coedge);
		}


	}

	// [debug] 打印配对边信息
	for (int i = 0; i < poor_coedge_pair_vec.size(); i++) {
		auto &coedge_pair = poor_coedge_pair_vec[i];

		LOG_DEBUG("Match pair (coedge id) (edge id): (%d, %d) (%d, %d)",
			MarkNum::GetId(coedge_pair.first.coedge),
			MarkNum::GetId(coedge_pair.second.coedge),
			MarkNum::GetId(coedge_pair.first.coedge->edge()),
			MarkNum::GetId(coedge_pair.second.coedge->edge())
		);
	}

	LOG_INFO("end.");
}

/*
	调用顺序：3
	对匹配的poor coedge对修改拓扑
	输入：目前poor_coedge_pair_vec已经维护完成
*/
void Stitch::StitchGapFixer::StitchPoorCoedge(ENTITY_LIST &bodies)
{
	LOG_INFO("start.");

	// 1. 准备<vertex,vertex> to edge的map
	std::map<std::pair<VERTEX*, VERTEX*>, EDGE*> vertex_pair_to_edge_map;
	std::vector<EDGE*> all_edge_vector; // 顺带维护一下整个实体的全部边的vector给第三步用

	// 预处理刚刚定义的两个东西
	for (int i = 0; i < bodies.count(); i++) {
		ENTITY *ibody = bodies[i];
		ENTITY_LIST edge_list;
		api_get_edges(ibody, edge_list);

		for (int j = 0; j < edge_list.count(); j++) {
			EDGE* iedge = static_cast<EDGE*>(edge_list[j]);
			all_edge_vector.emplace_back(iedge);
			vertex_pair_to_edge_map[std::make_pair(iedge->start(), iedge->end())] = iedge;
		}
	}

	// 2. 对poor_coedge_pair_vec重排序使得其符合修复顺序
	/*
		按照如下顺序排序，然后放入poor_coedge_pair_vec2中
		(1) 先缝合不会删边的
		(2) 再缝合会删非已经匹配边（正常边，非匹配的独立缺边）的
		(3) 最后全部缝合
	*/

	std::vector<std::pair<Stitch::PoorCoedge, Stitch::PoorCoedge>> poor_coedge_pair_vec2; // 保存重排序配对边集合

	// 对poor_coedge_pair_vec2部分重排序：poor_coedge_pair_vec2中保存的是（第一阶段重新排序后的）若干对(pair)已经配对的破边，现在根据破边的长度和对第一阶段重新排序中各个部分的配对破边做第二阶段排序
	// （注意lr满足左开右闭）
	auto partial_sort_for_poor_coedge_pair_vec2 = [&](std::vector<std::pair<Stitch::PoorCoedge, Stitch::PoorCoedge>>& poor_coedge_pair_vec2, int l, int r) {
		std::sort(
			poor_coedge_pair_vec2.begin() + l,
			poor_coedge_pair_vec2.begin() + r,
			[&](const std::pair<Stitch::PoorCoedge, Stitch::PoorCoedge>& pair_a, const std::pair<Stitch::PoorCoedge, Stitch::PoorCoedge>& pair_b) -> bool {
				double pair_a_score = Stitch::CalculatePoorCoedgeScore(pair_a.first, pair_a.second);
				double pair_b_score = Stitch::CalculatePoorCoedgeScore(pair_b.first, pair_b.second);

				return pair_a_score < pair_b_score; // 目前这个分数只和距离有关（距离越小越优先），因此这样就是让距离小的优先处理
			}
		);
	};

	auto rearrange_poor_coedge_pairs = [&](std::vector<std::pair<Stitch::PoorCoedge, Stitch::PoorCoedge>>& poor_coedge_pair_vec2)
	{
		LOG_DEBUG("Rearrange start.");

		std::vector<bool> poor_coedge_pair_vec_flag(poor_coedge_pair_vec.size()); // 标记向量：如果对应配对边组合被标记为true，则跳过它们，并且将它们从found_coedge_set集合中移除
		// 下文代码请参考stitch中画蓝圈的那个图，v0, v1代表不带撇那个边上的顶点，v01,v11则是带撇的那个

		// (2.0) 排除掉交叉连接的情况
		for (int i = 0; i < poor_coedge_pair_vec.size(); i++) {

			if (poor_coedge_pair_vec_flag[i]) {
				continue;
			}

			auto &poor_coedge_pair = poor_coedge_pair_vec[i]; // 取得已经配对的一对

			auto v0 = poor_coedge_pair.first.coedge->start();
			auto v1 = poor_coedge_pair.first.coedge->end();
			auto v01 = poor_coedge_pair.second.coedge->start();
			auto v11 = poor_coedge_pair.second.coedge->end();

			auto v0_v01_edge_it = vertex_pair_to_edge_map.find(std::make_pair(v0, v01)); // 寻找v0, v01是否有连接，也即交叉连接
			if (v0_v01_edge_it == vertex_pair_to_edge_map.end()) { // 找不到就反着找一遍确保没有
				v0_v01_edge_it = vertex_pair_to_edge_map.find(std::make_pair(v01, v0));
			}

			if (v0_v01_edge_it != vertex_pair_to_edge_map.end()) { //如果能找到
				auto iedge = v0_v01_edge_it->second; // 取得找到的这个交叉边
				if (iedge != poor_coedge_pair.first.coedge->edge() && iedge != poor_coedge_pair.second.coedge->edge()) { // 排除这个交叉边和已配对边相同的情况（这个有啥必要吗？）
					LOG_DEBUG("Connect between v0 v01: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d)", 
						MarkNum::GetId(poor_coedge_pair.first.coedge),
						MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
						MarkNum::GetId(poor_coedge_pair.second.coedge),
						MarkNum::GetId(poor_coedge_pair.second.coedge->edge())
					);

					found_coedge_set.erase(poor_coedge_pair.first.coedge);
					found_coedge_set.erase(poor_coedge_pair.second.coedge);
					poor_coedge_pair_vec_flag[i] = true;
					continue;
				}
			}

			auto v1_v11_edge_it = vertex_pair_to_edge_map.find(std::make_pair(v1, v11)); // 寻找v1, v11是否有连接，也即交叉连接
			if (v1_v11_edge_it == vertex_pair_to_edge_map.end()) {
				v1_v11_edge_it = vertex_pair_to_edge_map.find(std::make_pair(v11, v1));
			}

			if (v1_v11_edge_it != vertex_pair_to_edge_map.end()) { // 如果能找到
				auto iedge = v1_v11_edge_it->second; // 取得找到的交叉边
				if (iedge != poor_coedge_pair.first.coedge->edge() && iedge != poor_coedge_pair.second.coedge->edge()) { // 排除这个交叉边和已配对边相同的情况（这个有啥必要吗？）
					LOG_DEBUG("Connect between v1 v11: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d)",
						MarkNum::GetId(poor_coedge_pair.first.coedge),
						MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
						MarkNum::GetId(poor_coedge_pair.second.coedge),
						MarkNum::GetId(poor_coedge_pair.second.coedge->edge())
					);

					found_coedge_set.erase(poor_coedge_pair.first.coedge);
					found_coedge_set.erase(poor_coedge_pair.second.coedge);
					poor_coedge_pair_vec_flag[i] = true;
				}
			}

		}

		// (2.1) 顺序1：无连接
		for (int i = 0; i < poor_coedge_pair_vec.size(); i++) {

			if (poor_coedge_pair_vec_flag[i]) {
				continue;
			}

			auto &poor_coedge_pair = poor_coedge_pair_vec[i];

			auto v0 = poor_coedge_pair.first.coedge->start();
			auto v1 = poor_coedge_pair.first.coedge->end();
			auto v01 = poor_coedge_pair.second.coedge->start();
			auto v11 = poor_coedge_pair.second.coedge->end();

			if (vertex_pair_to_edge_map.count(std::make_pair(v0, v11)) == 0
				&& vertex_pair_to_edge_map.count(std::make_pair(v11, v0)) == 0
				&& vertex_pair_to_edge_map.count(std::make_pair(v1, v01)) == 0
				&& vertex_pair_to_edge_map.count(std::make_pair(v01, v1)) == 0
				) {

				LOG_DEBUG("Not connect: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d)",
					MarkNum::GetId(poor_coedge_pair.first.coedge),
					MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
					MarkNum::GetId(poor_coedge_pair.second.coedge),
					MarkNum::GetId(poor_coedge_pair.second.coedge->edge())
				);

				poor_coedge_pair_vec2.emplace_back(poor_coedge_pair);
				poor_coedge_pair_vec_flag[i] = true;
			}
		}
		// -> 局部重排序
		int poor_coedge_pair_vec2_size_2_1 = poor_coedge_pair_vec2.size();
		partial_sort_for_poor_coedge_pair_vec2(poor_coedge_pair_vec2, 0, poor_coedge_pair_vec2_size_2_1);

		// (2.2) 顺序2：连接非已经匹配边
		for (int i = 0; i < poor_coedge_pair_vec.size(); i++) {
			if (poor_coedge_pair_vec_flag[i]) {
				continue;
			}
			auto &poor_coedge_pair = poor_coedge_pair_vec[i];

			auto v0 = poor_coedge_pair.first.coedge->start();
			auto v1 = poor_coedge_pair.first.coedge->end();
			auto v01 = poor_coedge_pair.second.coedge->start();
			auto v11 = poor_coedge_pair.second.coedge->end();

			bool flag = true; // 优先排序flag，如果在判断v0, v11 （等）之间没有边相连则将其优先放入poor_coedge_pair_vec2中（同时修改poor_coedge_pair_vec_flag这个vector）

			// v0 v11
			auto v0_v11_map_it = vertex_pair_to_edge_map.find(std::make_pair(v0, v11));
			if (v0_v11_map_it == vertex_pair_to_edge_map.end()) {
				v0_v11_map_it = vertex_pair_to_edge_map.find(std::make_pair(v11, v0));
			}
			// v0, v11 之间有边相连 
			if (v0_v11_map_it != vertex_pair_to_edge_map.end()) {
				EDGE* v0_v11_edge = v0_v11_map_it->second;
				// 遍历这个边的所有coedge，如果存在coedge在Stitch::Singleton::found_coedge_set中则说明这不是一个非已匹边
				// 事实上，这里如果能判断出v0_v11_edge这个edge的coedge存在found_coedge_set中，就意味着这个边其实同样也是破边。如果这个破边（v0_v11_edge）也和另外的边有匹配，则将当前这个配对后置
				COEDGE* icoedge = v0_v11_edge->coedge();
				do {
					if (icoedge == nullptr) {
						break;
					}

					LOG_DEBUG("Pre Not unmatched coedge connect v0 v11: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d), (v0, v11: %d, %d), (edge, coedge: %d, %d)",
						MarkNum::GetId(poor_coedge_pair.first.coedge),
						MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
						MarkNum::GetId(poor_coedge_pair.second.coedge),
						MarkNum::GetId(poor_coedge_pair.second.coedge->edge()),
						MarkNum::GetId(v0),
						MarkNum::GetId(v11),
						MarkNum::GetId(v0_v11_edge),
						MarkNum::GetId(icoedge)
					);

					if (found_coedge_set.count(icoedge)) {
						LOG_DEBUG("Not unmatched coedge connect v0 v11: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d), (v0, v11: %d, %d), (edge, coedge: %d, %d)",
							MarkNum::GetId(poor_coedge_pair.first.coedge),
							MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
							MarkNum::GetId(poor_coedge_pair.second.coedge),
							MarkNum::GetId(poor_coedge_pair.second.coedge->edge()),
							MarkNum::GetId(v0),
							MarkNum::GetId(v11),
							MarkNum::GetId(v0_v11_edge),
							MarkNum::GetId(icoedge)
						);

						flag = false;
						break;
					}

					icoedge = icoedge->partner();
				} while (icoedge != nullptr && icoedge != v0_v11_edge->coedge());

			}

			// v1 v01
			auto v1_v01_map_it = vertex_pair_to_edge_map.find(std::make_pair(v1, v01));
			if (v1_v01_map_it == vertex_pair_to_edge_map.end()) {
				v1_v01_map_it = vertex_pair_to_edge_map.find(std::make_pair(v01, v1));
			}
			// v1,v01 之间有边相连
			if (v1_v01_map_it != vertex_pair_to_edge_map.end()) {
				EDGE* v1_v01_edge = v1_v01_map_it->second;
				// 遍历所有coedge，如果存在coedge在Stitch::Singleton::found_coedge_set中则说明这不是一个非已匹边
				COEDGE* icoedge = v1_v01_edge->coedge();
				do {
					if (icoedge == nullptr) {
						break;
					}

					LOG_DEBUG("Pre Not unmatched coedge connect v1 v01: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d), (v1, v01: %d, %d), (edge, coedge: %d, %d)",
						MarkNum::GetId(poor_coedge_pair.first.coedge),
						MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
						MarkNum::GetId(poor_coedge_pair.second.coedge),
						MarkNum::GetId(poor_coedge_pair.second.coedge->edge()),
						MarkNum::GetId(v1),
						MarkNum::GetId(v01),
						MarkNum::GetId(v1_v01_edge),
						MarkNum::GetId(icoedge)
					);

					if (found_coedge_set.count(icoedge)) {
						LOG_DEBUG("Not unmatched coedge connect v1 v01: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d), (v1, v01: %d, %d), (edge, coedge: %d, %d)",
							MarkNum::GetId(poor_coedge_pair.first.coedge),
							MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
							MarkNum::GetId(poor_coedge_pair.second.coedge),
							MarkNum::GetId(poor_coedge_pair.second.coedge->edge()),
							MarkNum::GetId(v1),
							MarkNum::GetId(v01),
							MarkNum::GetId(v1_v01_edge),
							MarkNum::GetId(icoedge)
						);

						flag = false;
						break;
					}

					icoedge = icoedge->partner();
				} while (icoedge != nullptr && icoedge != v1_v01_edge->coedge());

			}

			if (flag) {
				LOG_DEBUG("Rearrange Flag: True. Not unmatched coedge connect: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d)",
					MarkNum::GetId(poor_coedge_pair.first.coedge),
					MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
					MarkNum::GetId(poor_coedge_pair.second.coedge),
					MarkNum::GetId(poor_coedge_pair.second.coedge->edge())
				);

				poor_coedge_pair_vec2.emplace_back(poor_coedge_pair);
				poor_coedge_pair_vec_flag[i] = true;

			}


		}
		// -> 局部重排序
		int poor_coedge_pair_vec2_size_2_2 = poor_coedge_pair_vec2.size();
		partial_sort_for_poor_coedge_pair_vec2(poor_coedge_pair_vec2, poor_coedge_pair_vec2_size_2_1, poor_coedge_pair_vec2_size_2_2);

		// (2.3) 顺序3：剩下的情况
		for (int i = 0; i < poor_coedge_pair_vec.size(); i++) {
			if (poor_coedge_pair_vec_flag[i]) {
				continue;
			}

			auto &poor_coedge_pair = poor_coedge_pair_vec[i];

			poor_coedge_pair_vec2.emplace_back(poor_coedge_pair);
			//poor_coedge_pair_vec_flag[i] = true;

			LOG_DEBUG("Remaining case: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d)",
				MarkNum::GetId(poor_coedge_pair.first.coedge),
				MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
				MarkNum::GetId(poor_coedge_pair.second.coedge),
				MarkNum::GetId(poor_coedge_pair.second.coedge->edge())
			);

		}
		// -> 局部重排序
		int poor_coedge_pair_vec2_size_2_3 = poor_coedge_pair_vec2.size();
		partial_sort_for_poor_coedge_pair_vec2(poor_coedge_pair_vec2, poor_coedge_pair_vec2_size_2_2, poor_coedge_pair_vec2_size_2_3);

		LOG_DEBUG("Rearrange end.");
	};

	rearrange_poor_coedge_pairs(poor_coedge_pair_vec2);

	// 3. 开始修复：遍历poor_coedge_pair_vec2（重排序后的候选破边对集合），然后依次尝试修改对应破边的各种指针以及顶点指针
	// 规定v0 v1所在边是被与另一个合并的
	// （240401:啊原来这里也是随意规定的……嗯……也就是说这里就存在一个优化空间上的问题了）
	LOG_INFO("Stitch Fix start.");

	for (int i = 0; i < poor_coedge_pair_vec2.size(); i++) {
		auto &poor_coedge_pair = poor_coedge_pair_vec2[i];

		// [debug] 打印当前修复的poorcoedge pair的信息
		LOG_DEBUG("Stitch Fix for poor_coedge_pair[%d]: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d)",
			i,
			MarkNum::GetId(poor_coedge_pair.first.coedge),
			MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
			MarkNum::GetId(poor_coedge_pair.second.coedge),
			MarkNum::GetId(poor_coedge_pair.second.coedge->edge())
		);

		// 注意这里必须判断要修复的边是不是还在found_coedge_set里面。如果因为先前的步骤被删掉对应边了就不能再做这个修复了
		if (found_coedge_set.count(poor_coedge_pair.first.coedge) == 0
			|| found_coedge_set.count(poor_coedge_pair.second.coedge) == 0
			) {

			LOG_DEBUG("poor_coedge_pair SKIP fixing, as they not longer in found_coedge_set: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d)",
				MarkNum::GetId(poor_coedge_pair.first.coedge),
				MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
				MarkNum::GetId(poor_coedge_pair.second.coedge),
				MarkNum::GetId(poor_coedge_pair.second.coedge->edge())
			);

			continue;
		}

		auto v0 = poor_coedge_pair.first.coedge->start();
		auto v1 = poor_coedge_pair.first.coedge->end();
		auto v01 = poor_coedge_pair.second.coedge->start();
		auto v11 = poor_coedge_pair.second.coedge->end();

		// 3.1 先改指针
		poor_coedge_pair.first.coedge->set_partner(poor_coedge_pair.second.coedge);
		poor_coedge_pair.second.coedge->set_partner(poor_coedge_pair.first.coedge);
		poor_coedge_pair.second.coedge->set_edge(poor_coedge_pair.first.coedge->edge());
		poor_coedge_pair.second.coedge->set_sense(!(poor_coedge_pair.first.coedge->sense()));

		// 3.2 修改所有边的v11, v01为v0,v1（如果原本v0==v11等那就跳过）
		for (int j = 0; j < all_edge_vector.size(); j++) {
			auto &iedge = all_edge_vector[j];
			if (v0 != v11) {
				if (iedge->start() == v11) {

					LOG_DEBUG("set iedge->start() from v11 to v0: %d, %d, %d",
						MarkNum::GetId(iedge),
						MarkNum::GetId(v11),
						MarkNum::GetId(v0)
					);

					iedge->set_start(v0);
				}
				if (iedge->end() == v11) {

					LOG_DEBUG("set iedge->end() from v11 to v0: %d, %d, %d",
						MarkNum::GetId(iedge),
						MarkNum::GetId(v11),
						MarkNum::GetId(v0)
					);

					iedge->set_end(v0);
				}
			}
			if (v1 != v01) {
				if (iedge->start() == v01) {
					LOG_DEBUG("set iedge->start() from v01 to v1: %d, %d, %d",
						MarkNum::GetId(iedge),
						MarkNum::GetId(v01),
						MarkNum::GetId(v1)
					);

					iedge->set_start(v1);
				}
				if (iedge->end() == v01) {
					LOG_DEBUG("set iedge->end() from v01 to v1: %d, %d, %d",
						MarkNum::GetId(iedge),
						MarkNum::GetId(v01),
						MarkNum::GetId(v1)
					);

					iedge->set_end(v1);
				}
			}
		}

		// 3.3 检查v0 v11的相连情况，如果有相连就删除对应边（修改链表，相当于从loop中删除对应边）
		// （这个删边看上去并没有考虑到被删除边的引用计数情况啊（也就是说没有判断是否是破边））
		auto v0_v11_map_it = vertex_pair_to_edge_map.find(std::make_pair(v0, v11));
		if (v0_v11_map_it == vertex_pair_to_edge_map.end()) {
			v0_v11_map_it = vertex_pair_to_edge_map.find(std::make_pair(v11, v0));
		}
		// v0, v11 之间有边相连
		if (v0_v11_map_it != vertex_pair_to_edge_map.end()) {
			EDGE* v0_v11_edge = v0_v11_map_it->second;
			// 遍历此边下所有coedge，找到前后coedge,修改prev, next关系
			COEDGE* icoedge = v0_v11_edge->coedge();
			do {
				if (icoedge == nullptr) {
					break;
				}

				COEDGE* icoedge_prev = icoedge->previous();
				COEDGE* icoedge_next = icoedge->next();

				if (icoedge_prev != nullptr && icoedge_next != nullptr) {
					//别忘了：在删coedge的时候，记得check并修改对应loop的start（）
					if (icoedge->loop()->start() == icoedge) {
						icoedge->loop()->set_start(icoedge_next); // 随便设置一个loop的start吧（反正都不为空），这里就设置成icoedge_next
					}

					icoedge_next->set_previous(icoedge_prev);
					icoedge_prev->set_next(icoedge_next);

					LOG_DEBUG("change coedge prev and next: coedge:%d, prev:%d, next:%d",
						MarkNum::GetId(icoedge),
						MarkNum::GetId(icoedge_prev),
						MarkNum::GetId(icoedge_next)
					);
				}
				else {
					LOG_ERROR("change coedge prev and next: FAILED");
				}
				// fix时删边了，维护found_coedge_set
				found_coedge_set.erase(icoedge);
				icoedge = icoedge->partner();
			} while (icoedge != nullptr && icoedge != v0_v11_edge->coedge());
		}

		// 3.3 检查v1 v01的相连情况，如果有相连就删除对应边（修改链表，相当于从loop中删除对应边）
		auto v1_v01_map_it = vertex_pair_to_edge_map.find(std::make_pair(v1, v01));
		if (v1_v01_map_it == vertex_pair_to_edge_map.end()) {
			v1_v01_map_it = vertex_pair_to_edge_map.find(std::make_pair(v01, v1));
		}
		// v1, v01 之间有边相连
		if (v1_v01_map_it != vertex_pair_to_edge_map.end()) {
			EDGE* v1_v01_edge = v1_v01_map_it->second;
			// 遍历此边下所有coedge，找到前后coedge,修改prev, next关系
			COEDGE* icoedge = v1_v01_edge->coedge();
			do {
				if (icoedge == nullptr) {
					break;
				}

				COEDGE* icoedge_prev = icoedge->previous();
				COEDGE* icoedge_next = icoedge->next();

				if (icoedge_prev != nullptr && icoedge_next != nullptr) {
					//别忘了：在删coedge的时候，记得check并修改对应loop的start（）
					if (icoedge->loop()->start() == icoedge) {
						icoedge->loop()->set_start(icoedge_next); // 随便设置一个loop的start吧（反正都不为空），这里就设置成icoedge_next
					}

					icoedge_prev->set_next(icoedge_next);
					icoedge_next->set_previous(icoedge_prev);

					LOG_DEBUG(
						"change coedge prev and next: edge:%d, coedge:%d, prev:%d, next:%d",
						MarkNum::GetId(v1_v01_edge),
						MarkNum::GetId(icoedge),
						MarkNum::GetId(icoedge_prev),
						MarkNum::GetId(icoedge_next)
					);

				}
				else {
					LOG_ERROR("change coedge prev and next: FAILED");
				}
				// fix时删边了，维护found_coedge_set
				found_coedge_set.erase(icoedge);
				icoedge = icoedge->partner();
			} while (icoedge != nullptr && icoedge != v1_v01_edge->coedge());
		}
	}

	LOG_INFO("Stitch Fix end.");

	LOG_INFO("end.");
}



Stitch::MatchTree::MatchTree() : root(nullptr)
{
}

/*
	移除root指针内存
*/
Stitch::MatchTree::~MatchTree()
{
	if (this->root != nullptr) {
		delete root;
	}

	root = nullptr;

	LOG_INFO("~MatchTree Done.");
}


/*
	构造KDTree
	输入：含有poor coedge（其中有coedge和中间采样点坐标信息）的列表
*/
void Stitch::MatchTree::ConstructTree(std::vector<Stitch::PoorCoedge>& poor_coedge_vec)
{
	/*
		叶子节点
	*/
	auto update_leaf_node = [&](Stitch::PoorCoedge leaf_coedge, int now_dim)->Stitch::MatchTree::MatchTreeNode * {
		Stitch::MatchTree::MatchTreeNode *node = new Stitch::MatchTree::MatchTreeNode();

		node->is_leaf = true;
		node->leaf_poor_coedge = leaf_coedge; // 复制poorCoedge
		node->split_dim = now_dim;

		// boundary is set to midpoint
		for (int i = 0; i < 3; i++)
		{
			node->min_range_point[i] = leaf_coedge.midpoint_coords[Stitch::MIDPOINT_CNT >> 1][i];
			node->max_range_point[i] = leaf_coedge.midpoint_coords[Stitch::MIDPOINT_CNT >> 1][i];
		}

		return node;
	};

	/*
		非叶子节点
	*/
	auto update_now_node = [&](Stitch::MatchTree::MatchTreeNode *left, Stitch::MatchTree::MatchTreeNode *right, int now_dim)-> Stitch::MatchTree::MatchTreeNode * {
		Stitch::MatchTree::MatchTreeNode *node = new Stitch::MatchTree::MatchTreeNode();

		node->split_dim = now_dim;

		if (left != nullptr) {
			for (int i = 0; i < 3; i++) {
				node->min_range_point[i] = std::min(node->min_range_point[i], left->min_range_point[i]);
				node->max_range_point[i] = std::max(node->max_range_point[i], left->max_range_point[i]);
			}

			node->left_node = left;
		}

		if (right != nullptr) {
			for (int i = 0; i < 3; i++) {
				node->min_range_point[i] = std::min(node->min_range_point[i], right->min_range_point[i]);
				node->max_range_point[i] = std::max(node->max_range_point[i], right->max_range_point[i]);
			}

			node->right_node = right;
		}

		return node;
	};

	/*
		递归构造
		参数：poor_coedge_vec, now_dim（初调用：0）, l, r（l,r值域：0~poor_coedge_vec.size()-1）
	*/
	std::function<Stitch::MatchTree::MatchTreeNode*(std::vector<Stitch::PoorCoedge>&, int, int, int)> recursive_construct = [&](std::vector<Stitch::PoorCoedge>& poor_coedge_vec, int now_dim, int l, int r)->Stitch::MatchTree::MatchTreeNode* {
		if (l == r) {
			return update_leaf_node(poor_coedge_vec[l], now_dim);
		}

		if (l > r) {
			return nullptr;
		}

		int mid = (l + r) >> 1;

		// 利用nth_element划分poor_coedge_vec
		std::nth_element(poor_coedge_vec.begin() + l, poor_coedge_vec.begin() + mid, poor_coedge_vec.begin() + (r + 1),
			[&](const Stitch::PoorCoedge& a, const Stitch::PoorCoedge& b) {
				return a.midpoint_coords[Stitch::MIDPOINT_CNT >> 1][now_dim] < b.midpoint_coords[Stitch::MIDPOINT_CNT >> 1][now_dim];
			});

		Stitch::MatchTree::MatchTreeNode* left_node = recursive_construct(poor_coedge_vec, (now_dim + 1) % 3, l, mid);
		Stitch::MatchTree::MatchTreeNode* right_node = recursive_construct(poor_coedge_vec, (now_dim + 1) % 3, mid + 1, r);

		//update this node...
		return update_now_node(left_node, right_node, now_dim);
	};

	this->root = recursive_construct(poor_coedge_vec, 0, 0, static_cast<int>(poor_coedge_vec.size()) - 1);
}

void Stitch::MatchTree::DeleteTree()
{
	if (this->root != nullptr)
		delete this->root;
	root = nullptr;



	LOG_INFO("DeleteTree Done.\n");
}

/*
	在KDTree上做匹配（给定某条边进行匹配）
	参数：给定的某个待匹配poor coedge对象
	返回值：一个含有<poor coedge对象,分数>的列表
	注意这里的子树剪枝条件会根据EPSLION1放松一些
*/
std::vector<std::pair<Stitch::PoorCoedge, double>> Stitch::MatchTree::Match(Stitch::PoorCoedge poor_coedge1, const std::set<COEDGE*>& found_coedge_set)
{

	LOG_INFO("start.");

	LOG_DEBUG("Now Matching: %d", MarkNum::GetId(poor_coedge1.coedge));

	std::vector<std::pair<Stitch::PoorCoedge, double>> match_res_vec;

	/*
		检查子树是否符合剪枝条件，如果应该剪枝应该返回false
		注意这里的子树剪枝条件会根据EPSLION1放松一些
	*/
	auto check_subtree_valid = [&](Stitch::MatchTree::MatchTreeNode *now_root)->bool {

		// 遍历三维内容
		for (int i = 0; i < 3; i++) {
			if (
				!( // 取反：下面是符合条件的内容
				(now_root->min_range_point[i] - Stitch::EPSLION1 < poor_coedge1.midpoint_coords[Stitch::MIDPOINT_CNT >> 1][i])
					&&
					(poor_coedge1.midpoint_coords[Stitch::MIDPOINT_CNT >> 1][i] < now_root->max_range_point[i] + Stitch::EPSLION1)
					)
				) {
				return false;
			}
		}

		return true;
	};

	/*
		取得匹配分数
		匹配从 5个点的距离 得到
		如果任何一个点距离过大，或者夹角过大，则无条件认为不匹配（过大返回一个负数分数）
		（现在这块逻辑改成在CalculatePoorCoedgeScore中实现）
	*/
	auto get_match_score = [&](Stitch::MatchTree::MatchTreeNode *now_root)->double {
		Stitch::PoorCoedge &poor_coedge2 = now_root->leaf_poor_coedge;

		return Stitch::CalculatePoorCoedgeScore(poor_coedge1, poor_coedge2);
	};

	std::function<void(Stitch::MatchTree::MatchTreeNode *)> recursive_match = [&](Stitch::MatchTree::MatchTreeNode *now_root) {

		if (now_root->is_leaf) {

			// 排除掉自己与自己匹配，以及与已经匹配过的poor coedge匹配的两种情况
			if (now_root->leaf_poor_coedge.coedge == poor_coedge1.coedge) { // 指针相同排除
				LOG_DEBUG("Skip self match.");
				return;
			}

			if (found_coedge_set.count(poor_coedge1.coedge)) {
				LOG_DEBUG("Skip matched coedge.");
				return;
			}

			// 确实检查匹配条件并获得匹配分数
			LOG_DEBUG("Score calculating: %d %d",
				MarkNum::GetId(poor_coedge1.coedge),
				MarkNum::GetId(now_root->leaf_poor_coedge.coedge)
			);

			double score = get_match_score(now_root);

			LOG_DEBUG("Score get: %d %d, score:%.5lf\n",
				MarkNum::GetId(poor_coedge1.coedge),
				MarkNum::GetId(now_root->leaf_poor_coedge.coedge),
				score
			);

			if (score > 0) { // 注意排除分数为负数（计算分数时点距离过大或者夹角过大导致不合法）的情况
				match_res_vec.emplace_back(std::make_pair(now_root->leaf_poor_coedge, score));
			}

			return;
		}

		if (!check_subtree_valid(now_root)) {
			LOG_DEBUG("check_subtree_valid==false: %d\n", now_root);
			return;
		}

		if (now_root->left_node != nullptr) {
			recursive_match(now_root->left_node);
		}

		if (now_root->right_node != nullptr) {
			recursive_match(now_root->right_node);
		}
	};

	// 匹配单个边的流程（按照分数小to大排序）
	recursive_match(this->root);
	std::sort(match_res_vec.begin(), match_res_vec.end(), [&](const std::pair<Stitch::PoorCoedge, double>& a, const std::pair<Stitch::PoorCoedge, double>& b) {
		return a.second < b.second;
		});
	return match_res_vec;

	LOG_DEBUG("Matching end: %d\n", MarkNum::GetId(poor_coedge1.coedge));

	LOG_INFO("end.");
}

Stitch::MatchTree::MatchTreeNode::MatchTreeNode() :
	//min_range_point{ Stitch::MAXVAL, Stitch::MAXVAL, Stitch::MAXVAL}, // 版本不支持这么写
	//max_range_point{ Stitch::MINVAL,Stitch::MINVAL,Stitch::MINVAL },
	left_node(nullptr),
	right_node(nullptr),
	split_dim(-1),
	is_leaf(false),
	leaf_poor_coedge()
{
	for (int i = 0; i < 3; i++) {
		min_range_point[i] = Stitch::MAXVAL;
		max_range_point[i] = Stitch::MINVAL;
	}
}

/*
	移除此节点下指针内存
*/
Stitch::MatchTree::MatchTreeNode::~MatchTreeNode()
{
	if (left_node != nullptr) {
		delete left_node;
	}

	if (right_node != nullptr) {
		delete right_node;
	}

	LOG_INFO("~MatchTreeNode Done.");
}

Stitch::PoorCoedge::PoorCoedge() : coedge(nullptr)
{
}


/*
	入口
	找破边并修复 （注意目前这个只接受bodies下只有一个body的情况，否则可能会出问题）
*/
void Stitch::StitchGapFixer::Init(ENTITY_LIST &bodies, bool call_fix)
{
	LOG_INFO("EPSLION1: %.5lf;\t EPSLION2: %.5lf;\t EPSLION2_SIN: %.5lf;\t call_fix: %s\t", EPSLION1, EPSLION2, EPSLION2_SIN, call_fix ? "True" : "False");


	// 计时开始
	clock_t Stitch_start_clock = std::clock();

	api_initialize_constructors();
	api_initialize_booleans();

	FindPoorCoedge(bodies); // 1
	clock_t Stitch_end_clock_find = std::clock(); // 计时结束 find

	MatchPoorCoedge(); // 2
	clock_t Stitch_end_clock_match = std::clock(); // 计时结束 match

	if (call_fix) { StitchPoorCoedge(bodies); } // 3
	clock_t Stitch_end_clock_stitch = std::clock(); // 计时结束 stitch

	match_tree.DeleteTree(); // 删除KDTree

	api_terminate_constructors();
	api_terminate_booleans();

	// 打印时间
	double stitch_end_clock_find_cost = static_cast<double>(Stitch_end_clock_find - Stitch_start_clock) / CLOCKS_PER_SEC;
	double stitch_end_clock_match_cost = static_cast<double>(Stitch_end_clock_match - Stitch_start_clock) / CLOCKS_PER_SEC;
	double stitch_end_clock_stitch_cost = static_cast<double>(Stitch_end_clock_stitch - Stitch_start_clock) / CLOCKS_PER_SEC;

	LOG_INFO("Stitch_end_clock_find: %.5lf sec", Stitch_end_clock_find);
	LOG_INFO("Stitch_end_clock_match: %.5lf sec", Stitch_end_clock_match);
	if (call_fix) {
		LOG_INFO("Stitch_end_clock_stitch: %.5lf sec", Stitch_end_clock_stitch);
	}
}

void Stitch::StitchGapFixer::Clear()
{
	LOG_INFO("start.");

	poor_coedge_vec.clear();
	found_coedge_set.clear();
	poor_coedge_pair_vec.clear();
	match_tree.DeleteTree();

	LOG_INFO("end.");
}
