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
	������������PoorCoedge��ƥ�����
	����MatchTree::Match�л��õ���

	Ŀǰ�޸�Ϊֻ�;����йأ��Ƕ�ֻ�����ڸ�����һЩ��������ų�
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

	// �Ծ��룺��һ�������ƽ����������ʱӦ����һ��С��1����ֵ����Ȼ���Ȩ
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
	����˳��1
	���Ʊߣ������浽poor_coedge_vec��
*/
void Stitch::StitchGapFixer::FindPoorCoedge(ENTITY_LIST & bodies)
{
	LOG_INFO("start.");

	for (int i = 0; i < bodies.count(); i++) {
		ENTITY* ibody = (bodies[i]);

		// �������body��coedge list
		ENTITY_LIST coedge_list;
		api_get_coedges(ibody, coedge_list);

		// for coedge
		for (int j = 0; j < coedge_list.count(); j++) {
			COEDGE* coedge_ptr = static_cast<COEDGE*>(coedge_list[j]);

			int partner_count = Utils::PartnerCount(coedge_ptr);

			// ���coedge��partnet������1����ζ���ҵ�poor coedge�����浽vec��
			if (partner_count == 1) {

				// [��Ч�Լ��] START

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

				// ����޼��ε����				
				if (coedge_ptr->geometry() == nullptr) // ע�������Ҫ�ų�����Ϊ����ȷʵ��coedge_ptr���β����ڵ��Ƕ�Ӧ��edgeȴ�е������
				{
					LOG_ERROR("coedge_ptr NO GEOMETRY: coedge_ptr: %d (edge: %d), vertex: %d %d",
						MarkNum::GetId(coedge_ptr),
						MarkNum::GetId(coedge_ptr->edge()),
						MarkNum::GetId(coedge_ptr->start()),
						MarkNum::GetId(coedge_ptr->end()));
				}

				if (coedge_ptr->start()->geometry() == nullptr)
				{
					LOG_ERROR("coedge_ptr->start() NO GEOMETRY: coedge_ptr: %d (edge: %d), vertex: %d %d",
						MarkNum::GetId(coedge_ptr),
						MarkNum::GetId(coedge_ptr->edge()),
						MarkNum::GetId(coedge_ptr->start()),
						MarkNum::GetId(coedge_ptr->end()));
				}

				if (coedge_ptr->end()->geometry() == nullptr)
				{
					LOG_ERROR("coedge_ptr->end() NO GEOMETRY: coedge_ptr: %d (edge: %d), vertex: %d %d",
						MarkNum::GetId(coedge_ptr),
						MarkNum::GetId(coedge_ptr->edge()),
						MarkNum::GetId(coedge_ptr->start()),
						MarkNum::GetId(coedge_ptr->end()));
				}

				// [��Ч�Լ��] END

				LOG_DEBUG("poor coedge found: %d (edge: %d), vertex:%d %d",
					MarkNum::GetId(coedge_ptr),
					MarkNum::GetId(coedge_ptr->edge()),
					MarkNum::GetId(coedge_ptr->start()),
					MarkNum::GetId(coedge_ptr->end())
				);


				// ���㣨Ĭ��5�����м�����㣬����0��4�Ƕ˵㣬2�����е�
				Stitch::PoorCoedge temp_poor_coedge;
				temp_poor_coedge.coedge = coedge_ptr;

				auto edge_range = coedge_ptr->edge()->param_range();
				auto coedge_range = (coedge_ptr->sense() == FORWARD) ? edge_range : -edge_range;

				for (int k = 0; k < MIDPOINT_CNT; k++) {
					// ��ʼ��
					if (k == 0) {
						temp_poor_coedge.midpoint_coords[k][0] = coedge_ptr->start()->geometry()->coords().x();
						temp_poor_coedge.midpoint_coords[k][1] = coedge_ptr->start()->geometry()->coords().y();
						temp_poor_coedge.midpoint_coords[k][2] = coedge_ptr->start()->geometry()->coords().z();
						continue;
					}
					// �յ�
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

				// [DEBUG] ��ӡ�м��ֵ�����������ֵ

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
	 ����˳��2
	 ƥ��ÿ���ߣ����ҹ���KDTree���м���
	 ���ƥ����̰�ĵ�ƥ��ģ�Ҳ����˵һ��ƥ��ɹ��Ͱ����Ž���Ӧ��found_coedge_set���������ٲ���ƥ�䣩������֮��Ҫ��һ��
*/
void Stitch::StitchGapFixer::MatchPoorCoedge()
{
	LOG_INFO("start.");

	// �����������е㣩
	match_tree.ConstructTree(poor_coedge_vec);

	for (int i = 0; i < poor_coedge_vec.size(); i++) {
		auto &e = poor_coedge_vec[i];

		if (found_coedge_set.count(e.coedge)) {
			LOG_DEBUG("Skip matching: %d (edge: %d)", MarkNum::GetId(e.coedge), MarkNum::GetId(e.coedge->edge()));
			continue;
		}

		auto match_poor_coedge_vec = match_tree.Match(e, found_coedge_set, dont_stitch_coincident);

		if (match_poor_coedge_vec.size()) {
			// ȡ������С��<PoorCoedge, score>��
			auto first_match = match_poor_coedge_vec.front();

			// [debug]��ӡ����
			LOG_DEBUG("Best match id: %d (edge: %d) %d (edge: %d), score: %.5lf",
				MarkNum::GetId(e.coedge),
				MarkNum::GetId(e.coedge->edge()),
				MarkNum::GetId(first_match.first.coedge),
				MarkNum::GetId(first_match.first.coedge->edge()),
				first_match.second
			);

			poor_coedge_pair_vec.emplace_back(std::make_pair(e, first_match.first));

			// ά��found_coedge_set
			found_coedge_set.insert(e.coedge);
			found_coedge_set.insert(first_match.first.coedge);
		}


	}

	// [debug] ��ӡ��Ա���Ϣ
	for (int i = 0; i < poor_coedge_pair_vec.size(); i++) {
		auto &coedge_pair = poor_coedge_pair_vec[i];

		LOG_DEBUG("Match pair (coedge id pair) (edge id pair): (%d, %d) (%d, %d)",
			MarkNum::GetId(coedge_pair.first.coedge),
			MarkNum::GetId(coedge_pair.second.coedge),
			MarkNum::GetId(coedge_pair.first.coedge->edge()),
			MarkNum::GetId(coedge_pair.second.coedge->edge())
		);
	}

	LOG_INFO("end.");
}


/*
	����˳��3
	������
*/
void Stitch::StitchGapFixer::RearrangePoorCoedge()
{
	LOG_INFO("start.");

	PreProcess(); // Ԥ����

	// 2. ��poor_coedge_pair_vec������ʹ��������޸�˳��
	/*
		��������˳������Ȼ�����poor_coedge_pair_vec2��
		(1) �ȷ�ϲ���ɾ�ߵ�
		(2) �ٷ�ϻ�ɾ���Ѿ�ƥ��ߣ������ߣ���ƥ��Ķ���ȱ�ߣ���
		(3) ���ȫ�����
	*/
	std::vector<std::pair<Stitch::PoorCoedge, Stitch::PoorCoedge>> poor_coedge_pair_vec2; // ������������Ա߼���

	// ��poor_coedge_pair_vec2����������poor_coedge_pair_vec2�б�����ǣ���һ�׶����������ģ����ɶ�(pair)�Ѿ���Ե��Ʊߣ����ڸ����Ʊߵĳ��ȺͶԵ�һ�׶����������и������ֵ�����Ʊ����ڶ��׶�����
	// ��ע��lr�������ұգ�
	auto partial_sort_for_poor_coedge_pair_vec2 = [&](std::vector<std::pair<Stitch::PoorCoedge, Stitch::PoorCoedge>>& poor_coedge_pair_vec2, int l, int r) {
		std::sort(
			poor_coedge_pair_vec2.begin() + l,
			poor_coedge_pair_vec2.begin() + r,
			[&](const std::pair<Stitch::PoorCoedge, Stitch::PoorCoedge>& pair_a, const std::pair<Stitch::PoorCoedge, Stitch::PoorCoedge>& pair_b) -> bool {
				double pair_a_score = Stitch::CalculatePoorCoedgeScore(pair_a.first, pair_a.second);
				double pair_b_score = Stitch::CalculatePoorCoedgeScore(pair_b.first, pair_b.second);

				return pair_a_score < pair_b_score; // Ŀǰ�������ֻ�;����йأ�����ԽСԽ���ȣ���������������þ���С�����ȴ���
			}
		);
	};

	auto rearrange_poor_coedge_pairs = [&](std::vector<std::pair<Stitch::PoorCoedge, Stitch::PoorCoedge>>& poor_coedge_pair_vec2)
	{
		LOG_DEBUG("Rearrange start.");

		std::vector<bool> poor_coedge_pair_vec_flag(poor_coedge_pair_vec.size()); // ��������������Ӧ��Ա���ϱ����Ϊtrue�����������ǣ����ҽ����Ǵ�found_coedge_set�������Ƴ�
		// ���Ĵ�����ο�stitch�л���Ȧ���Ǹ�ͼ��v0, v1������Ʋ�Ǹ����ϵĶ��㣬v01,v11���Ǵ�Ʋ���Ǹ�

		// (2.0) ~~�ų�~~ ���潻�����ӵ����

		auto f2_0 = [&](std::pair<Stitch::PoorCoedge, Stitch::PoorCoedge>& poor_coedge_pair, VERTEX* v1, VERTEX* v2) {
			std::vector<EDGE*> edges = edges_data.FindEdgesBetweenVertices(v1, v2);

			for (int j = 0; j < edges.size(); j++) {
				EDGE* e = edges[j];
				if (e != poor_coedge_pair.first.coedge->edge() && e != poor_coedge_pair.second.coedge->edge()) {// ���ж��ų��������ߺ�����Ա���ͬ������������ɶ��Ҫ�𣿣�
					LOG_WARN("(2.0) Warn: Connect between v0 v01 or v1 v11: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d) (v1: %d, v2: %d, e: %d)",
						MarkNum::GetId(poor_coedge_pair.first.coedge),
						MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
						MarkNum::GetId(poor_coedge_pair.second.coedge),
						MarkNum::GetId(poor_coedge_pair.second.coedge->edge()),
						MarkNum::GetId(v1),
						MarkNum::GetId(v2),
						MarkNum::GetId(e)
					);

					/*found_coedge_set.erase(poor_coedge_pair.first.coedge);
					found_coedge_set.erase(poor_coedge_pair.second.coedge);
					poor_coedge_pair_vec_flag[i] = true;*/ // ���ڲ����ų��������
				}
			}
		};

		for (int i = 0; i < poor_coedge_pair_vec.size(); i++) {

			if (poor_coedge_pair_vec_flag[i]) {
				continue;
			}

			auto &poor_coedge_pair = poor_coedge_pair_vec[i]; // ȡ���Ѿ���Ե�һ��

			auto v0 = poor_coedge_pair.first.coedge->start();
			auto v1 = poor_coedge_pair.first.coedge->end();
			auto v01 = poor_coedge_pair.second.coedge->start();
			auto v11 = poor_coedge_pair.second.coedge->end();

			f2_0(poor_coedge_pair, v0, v01);
			f2_0(poor_coedge_pair, v1, v11);
		}

		// (2.1) ˳��1��������
		for (int i = 0; i < poor_coedge_pair_vec.size(); i++) {

			if (poor_coedge_pair_vec_flag[i]) {
				continue;
			}

			auto &poor_coedge_pair = poor_coedge_pair_vec[i];

			auto v0 = poor_coedge_pair.first.coedge->start();
			auto v1 = poor_coedge_pair.first.coedge->end();
			auto v01 = poor_coedge_pair.second.coedge->start();
			auto v11 = poor_coedge_pair.second.coedge->end();

			if( edges_data.FindEdgesBetweenVertices(v0, v11).size() == 0
				&& edges_data.FindEdgesBetweenVertices(v1, v01).size() == 0
				&& edges_data.FindEdgesBetweenVertices(v1, v11).size() == 0
				&& edges_data.FindEdgesBetweenVertices(v0, v01).size() == 0
				)
			{
				LOG_DEBUG("(2.1) Not connect: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d)",
					MarkNum::GetId(poor_coedge_pair.first.coedge),
					MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
					MarkNum::GetId(poor_coedge_pair.second.coedge),
					MarkNum::GetId(poor_coedge_pair.second.coedge->edge())
				);

				poor_coedge_pair_vec2.emplace_back(poor_coedge_pair);
				poor_coedge_pair_vec_flag[i] = true;
			}
		}
		// -> �ֲ�������
		int poor_coedge_pair_vec2_size_2_1 = poor_coedge_pair_vec2.size();
		partial_sort_for_poor_coedge_pair_vec2(poor_coedge_pair_vec2, 0, poor_coedge_pair_vec2_size_2_1);

		// (2.2) ˳��2�����ӷ��Ѿ�ƥ���

		auto f2_2 = [&](std::pair<Stitch::PoorCoedge, Stitch::PoorCoedge>& poor_coedge_pair, VERTEX* v1, VERTEX* v2, bool& flag) {
			std::vector<EDGE*> edges = edges_data.FindEdgesBetweenVertices(v1, v2);

			// v0, v11��v1, v2�� ֮���б����� 
			for (int j = 0; j < edges.size(); j++) {
				EDGE* e = edges[j];

				// ��������ߵ�����coedge���������coedge��Stitch::Singleton::found_coedge_set����˵���ⲻ��һ������ƥ��
				// ��ʵ�ϣ�����������жϳ�v0_v11_edge���edge��coedge����found_coedge_set�У�����ζ���������ʵͬ��Ҳ���Ʊߡ��������Ʊߣ�v0_v11_edge��Ҳ������ı���ƥ�䣬�򽫵�ǰ�����Ժ��á��������������һ��
				COEDGE* icoedge = e->coedge();
				do {
					if (icoedge == nullptr) {
						break;
					}

					if (found_coedge_set.count(icoedge)) {
						LOG_DEBUG("(2.2) Matched coedge (���Ƿ��Ѿ�ƥ������ӣ�Ҳ��������ƥ�������) connect v0 v11 or v1 v01: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d), (v1, v2: %d, %d), (this matched edge, this matched coedge: %d, %d)",
							MarkNum::GetId(poor_coedge_pair.first.coedge),
							MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
							MarkNum::GetId(poor_coedge_pair.second.coedge),
							MarkNum::GetId(poor_coedge_pair.second.coedge->edge()),
							MarkNum::GetId(v1),
							MarkNum::GetId(v2),
							MarkNum::GetId(e),
							MarkNum::GetId(icoedge)
						);

						flag = false;
						break;
					}

					icoedge = icoedge->partner();
				} while (icoedge != nullptr && icoedge != e->coedge());
			}
		};

		for (int i = 0; i < poor_coedge_pair_vec.size(); i++) {
			if (poor_coedge_pair_vec_flag[i]) {
				continue;
			}
			auto &poor_coedge_pair = poor_coedge_pair_vec[i];

			auto v0 = poor_coedge_pair.first.coedge->start();
			auto v1 = poor_coedge_pair.first.coedge->end();
			auto v01 = poor_coedge_pair.second.coedge->start();
			auto v11 = poor_coedge_pair.second.coedge->end();

			bool flag = true; // ��������flag��������ж�v0, v11 ���ȣ�֮��û�б������������ȷ���poor_coedge_pair_vec2�У�ͬʱ�޸�poor_coedge_pair_vec_flag���vector��

			// v0 v11
			f2_2(poor_coedge_pair, v0, v11, flag);

			// v1 v01
			f2_2(poor_coedge_pair, v1, v01, flag);

			if (flag) {
				LOG_DEBUG("(2.2) Rearrange Flag: True. Not unmatched coedge connect: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d)",
					MarkNum::GetId(poor_coedge_pair.first.coedge),
					MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
					MarkNum::GetId(poor_coedge_pair.second.coedge),
					MarkNum::GetId(poor_coedge_pair.second.coedge->edge())
				);

				poor_coedge_pair_vec2.emplace_back(poor_coedge_pair);
				poor_coedge_pair_vec_flag[i] = true;

			}

		}
		// -> �ֲ�������
		int poor_coedge_pair_vec2_size_2_2 = poor_coedge_pair_vec2.size();
		partial_sort_for_poor_coedge_pair_vec2(poor_coedge_pair_vec2, poor_coedge_pair_vec2_size_2_1, poor_coedge_pair_vec2_size_2_2);

		// (2.3) ˳��3��ʣ�µ����
		for (int i = 0; i < poor_coedge_pair_vec.size(); i++) {
			if (poor_coedge_pair_vec_flag[i]) {
				continue;
			}

			auto &poor_coedge_pair = poor_coedge_pair_vec[i];

			poor_coedge_pair_vec2.emplace_back(poor_coedge_pair);
			//poor_coedge_pair_vec_flag[i] = true;

			LOG_DEBUG("(2.3) Remaining case: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d)",
				MarkNum::GetId(poor_coedge_pair.first.coedge),
				MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
				MarkNum::GetId(poor_coedge_pair.second.coedge),
				MarkNum::GetId(poor_coedge_pair.second.coedge->edge())
			);

		}
		// -> �ֲ�������
		int poor_coedge_pair_vec2_size_2_3 = poor_coedge_pair_vec2.size();
		partial_sort_for_poor_coedge_pair_vec2(poor_coedge_pair_vec2, poor_coedge_pair_vec2_size_2_2, poor_coedge_pair_vec2_size_2_3);

		LOG_DEBUG("Rearrange end.");
	};

	rearrange_poor_coedge_pairs(poor_coedge_pair_vec2);

	poor_coedge_pair_vec = std::move(poor_coedge_pair_vec2);

	//[debug] ����������Ա���Ϣ
	for (int i = 0; i < poor_coedge_pair_vec.size(); i++) {
		auto &coedge_pair = poor_coedge_pair_vec[i];

		LOG_DEBUG("Match pair AFTER REARRANGE: (coedge id pair) (edge id pair): (%d, %d) (%d, %d)",
			MarkNum::GetId(coedge_pair.first.coedge),
			MarkNum::GetId(coedge_pair.second.coedge),
			MarkNum::GetId(coedge_pair.first.coedge->edge()),
			MarkNum::GetId(coedge_pair.second.coedge->edge())
		);
	}


	LOG_INFO("end.");
}

/*
	����˳��4
	��ƥ���poor coedge���޸�����
	���룺Ŀǰpoor_coedge_pair_vec�Ѿ�ά�����
*/
void Stitch::StitchGapFixer::StitchPoorCoedge(ENTITY_LIST &bodies)
{
	// 3. ��ʼ�޸�������poor_coedge_pair_vec2���������ĺ�ѡ�Ʊ߶Լ��ϣ���Ȼ�����γ����޸Ķ�Ӧ�Ʊߵĸ���ָ���Լ�����ָ��
	// �涨v0 v1���ڱ��Ǳ�����һ���ϲ���
	// ��240401:��ԭ������Ҳ������涨�ġ����š���Ҳ����˵����ʹ���һ���Ż��ռ��ϵ������ˣ�
	LOG_INFO("Stitch Fix start.");

	for (int i = 0; i < poor_coedge_pair_vec.size(); i++) {
		auto &poor_coedge_pair = poor_coedge_pair_vec[i];

		// [debug] ��ӡ��ǰ�޸���poorcoedge pair����Ϣ
		LOG_DEBUG("Stitch Fix for poor_coedge_pair[%d]: (poor_coedge_pair.first.coedge: %d, edge: %d), (poor_coedge_pair.second.coedge: %d, edge: %d)",
			i,
			MarkNum::GetId(poor_coedge_pair.first.coedge),
			MarkNum::GetId(poor_coedge_pair.first.coedge->edge()),
			MarkNum::GetId(poor_coedge_pair.second.coedge),
			MarkNum::GetId(poor_coedge_pair.second.coedge->edge())
		);

		// ע����������ж�Ҫ�޸��ı��ǲ��ǻ���found_coedge_set���档�����Ϊ��ǰ�Ĳ��豻ɾ����Ӧ���˾Ͳ�����������޸���
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

		// 3.1 �ȸ�ָ��
		poor_coedge_pair.first.coedge->set_partner(poor_coedge_pair.second.coedge);
		poor_coedge_pair.second.coedge->set_partner(poor_coedge_pair.first.coedge);
		poor_coedge_pair.second.coedge->set_edge(poor_coedge_pair.first.coedge->edge());
		poor_coedge_pair.second.coedge->set_sense(!(poor_coedge_pair.first.coedge->sense()));

		// 3.2 �޸����бߵ�v11, v01Ϊv0, v1�����ԭ��v0==v11���Ǿ�������
		for (int j = 0; j < edges_data.all_edge_vector.size(); j++) {
			auto &iedge = edges_data.all_edge_vector[j];
			if (v0 != v11) {
				if (iedge->start() == v11) {

					LOG_DEBUG("(3.2) set iedge->start() from v11 to v0: %d, %d, %d",
						MarkNum::GetId(iedge),
						MarkNum::GetId(v11),
						MarkNum::GetId(v0)
					);

					iedge->set_start(v0);
				}
				if (iedge->end() == v11) {

					LOG_DEBUG("(3.2) set iedge->end() from v11 to v0: %d, %d, %d",
						MarkNum::GetId(iedge),
						MarkNum::GetId(v11),
						MarkNum::GetId(v0)
					);

					iedge->set_end(v0);
				}
			}
			if (v1 != v01) {
				if (iedge->start() == v01) {
					LOG_DEBUG("(3.2) set iedge->start() from v01 to v1: %d, %d, %d",
						MarkNum::GetId(iedge),
						MarkNum::GetId(v01),
						MarkNum::GetId(v1)
					);

					iedge->set_start(v1);
				}
				if (iedge->end() == v01) {
					LOG_DEBUG("(3.2) set iedge->end() from v01 to v1: %d, %d, %d",
						MarkNum::GetId(iedge),
						MarkNum::GetId(v01),
						MarkNum::GetId(v1)
					);

					iedge->set_end(v1);
				}
			}
		}

		// 3.3.1 ���v0 v11����������������������ɾ����Ӧ�ߣ��޸������൱�ڴ�loop��ɾ����Ӧ�ߣ�
		// �����ɾ�߿���ȥ��û�п��ǵ���ɾ���ߵ����ü����������Ҳ����˵û���ж��Ƿ����Ʊߣ���
		// 240813: ~~���ڸĳ����������Ҳ���࿼�����ֽ�������ӵ�������������ҲҪɾ������(v0, v11) (v1, v01) (v0, v01) (v1, v11)~~ ���ԣ�������ǲ��ܸģ�
		auto f3_3 = [&](VERTEX* v1, VERTEX* v2) {
			std::vector<EDGE*> edges = edges_data.FindEdgesBetweenVertices(v1, v2);

			for (int j = 0; j < edges.size(); j++) {
				EDGE* e = edges[j];

				// �����˱�������coedge���ҵ�ǰ��coedge,�޸�prev, next��ϵ
				COEDGE* icoedge = e->coedge();

				do {
					if (icoedge == nullptr) {
						break;
					}

					COEDGE* icoedge_prev = icoedge->previous();
					COEDGE* icoedge_next = icoedge->next();

					if (icoedge_prev != nullptr && icoedge_next != nullptr) {
						//�����ˣ���ɾcoedge��ʱ�򣬼ǵ�check���޸Ķ�Ӧloop��start����
						if (icoedge->loop()->start() == icoedge) {
							icoedge->loop()->set_start(icoedge_next); // �������һ��loop��start�ɣ���������Ϊ�գ�����������ó�icoedge_next
						}

						icoedge_next->set_previous(icoedge_prev);
						icoedge_prev->set_next(icoedge_next);

						LOG_DEBUG("(3.3) change coedge prev and next: coedge:%d, prev:%d, next:%d",
							MarkNum::GetId(icoedge),
							MarkNum::GetId(icoedge_prev),
							MarkNum::GetId(icoedge_next)
						);
					}
					else {
						LOG_ERROR("(3.3) change coedge prev and next: FAILED");
					}
					// fixʱɾ���ˣ�ά��found_coedge_set
					found_coedge_set.erase(icoedge);
					icoedge = icoedge->partner();
				} while (icoedge != nullptr && icoedge != e->coedge());
			}
		};

		// 3.3.2 ���v1 v01����������������������ɾ����Ӧ�ߣ��޸������൱�ڴ�loop��ɾ����Ӧ�ߣ�
		f3_3(v0, v11);
		f3_3(v1, v01);
		//f3_3(v0, v01);
		//f3_3(v1, v11);

		// TODO: 3.4 �޸ı߼���

		// TODO: 3.5 �޸��漸��
	}

	LOG_INFO("Stitch Fix end.");
}

void Stitch::StitchGapFixer::PreProcess()
{
	edges_data.Init(bodies);
}

Stitch::MatchTree::MatchTree() : root(nullptr)
{
}

/*
	�Ƴ�rootָ���ڴ�
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
	����KDTree
	���룺����poor coedge��������coedge���м������������Ϣ�����б�
*/
void Stitch::MatchTree::ConstructTree(std::vector<Stitch::PoorCoedge>& poor_coedge_vec)
{
	/*
		Ҷ�ӽڵ�
	*/
	auto update_leaf_node = [&](Stitch::PoorCoedge leaf_coedge, int now_dim)->Stitch::MatchTree::MatchTreeNode * {
		Stitch::MatchTree::MatchTreeNode *node = new Stitch::MatchTree::MatchTreeNode();

		node->is_leaf = true;
		node->leaf_poor_coedge = leaf_coedge; // ����poorCoedge
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
		��Ҷ�ӽڵ�
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
		�ݹ鹹��
		������poor_coedge_vec, now_dim�������ã�0��, l, r��l,rֵ��0~poor_coedge_vec.size()-1��
	*/
	std::function<Stitch::MatchTree::MatchTreeNode*(std::vector<Stitch::PoorCoedge>&, int, int, int)> recursive_construct = [&](std::vector<Stitch::PoorCoedge>& poor_coedge_vec, int now_dim, int l, int r)->Stitch::MatchTree::MatchTreeNode* {
		if (l == r) {
			return update_leaf_node(poor_coedge_vec[l], now_dim);
		}

		if (l > r) {
			return nullptr;
		}

		int mid = (l + r) >> 1;

		// ����nth_element����poor_coedge_vec
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
	��KDTree����ƥ�䣨����ĳ���߽���ƥ�䣩
	������������ĳ����ƥ��poor coedge����
	����ֵ��һ������<poor coedge����,����>���б�
	ע�������������֦���������EPSLION1����һЩ
*/
std::vector<std::pair<Stitch::PoorCoedge, double>> Stitch::MatchTree::Match(Stitch::PoorCoedge poor_coedge1, const std::set<COEDGE*>& found_coedge_set, bool dont_stitch_coincident)
{
	LOG_INFO("start.");

	LOG_DEBUG("Now Matching: %d (edge: %d)", MarkNum::GetId(poor_coedge1.coedge), MarkNum::GetId(poor_coedge1.coedge->edge()));

	std::vector<std::pair<Stitch::PoorCoedge, double>> match_res_vec;

	/*
		��������Ƿ���ϼ�֦���������Ӧ�ü�֦Ӧ�÷���false
		ע�������������֦���������EPSLION1����һЩ
	*/
	auto check_subtree_valid = [&](Stitch::MatchTree::MatchTreeNode *now_root)->bool {

		// ������ά����
		for (int i = 0; i < 3; i++) {
			if (
				!( // ȡ���������Ƿ�������������
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
		ȡ��ƥ�����
		ƥ��� 5����ľ��� �õ�
		����κ�һ���������󣬻��߼нǹ�������������Ϊ��ƥ�䣨���󷵻�һ������������
		����������߼��ĳ���CalculatePoorCoedgeScore��ʵ�֣�
	*/
	auto get_match_score = [&](Stitch::MatchTree::MatchTreeNode *now_root)->double {
		Stitch::PoorCoedge &poor_coedge2 = now_root->leaf_poor_coedge;

		return Stitch::CalculatePoorCoedgeScore(poor_coedge1, poor_coedge2);
	};

	std::function<void(Stitch::MatchTree::MatchTreeNode *)> recursive_match = [&](Stitch::MatchTree::MatchTreeNode *now_root) {

		if (now_root->is_leaf) {

			// �ų����Լ����Լ�ƥ�䣬�Լ����Ѿ�ƥ�����poor coedgeƥ����������
			if (now_root->leaf_poor_coedge.coedge == poor_coedge1.coedge) { // ָ����ͬ�ų�
				LOG_DEBUG("Skip self match.");
				return;
			}

			if (found_coedge_set.count(poor_coedge1.coedge)) {
				LOG_DEBUG("Skip matched coedge.");
				return;
			}

			// �ų�����ȫ��ͬ�ıߵļ���ƥ�����������ѡ�����ã�
			if (dont_stitch_coincident && GeometryUtils::GeometryCoincidentEdge(poor_coedge1.coedge->edge(), now_root->leaf_poor_coedge.coedge->edge())) {
				LOG_DEBUG("Skip coincident coedge.");
				return;
			}

			// ȷʵ���ƥ�����������ƥ�����
			LOG_DEBUG("Score calculating: %d (edge: %d) %d (edge: %d)",
				MarkNum::GetId(poor_coedge1.coedge),
				MarkNum::GetId(poor_coedge1.coedge->edge()),
				MarkNum::GetId(now_root->leaf_poor_coedge.coedge),
				MarkNum::GetId(now_root->leaf_poor_coedge.coedge->edge())
			);

			double score = get_match_score(now_root);

			LOG_DEBUG("Score get: %d (edge: %d) %d (edge: %d), score:%.5lf\n",
				MarkNum::GetId(poor_coedge1.coedge),
				MarkNum::GetId(poor_coedge1.coedge->edge()),
				MarkNum::GetId(now_root->leaf_poor_coedge.coedge),
				MarkNum::GetId(now_root->leaf_poor_coedge.coedge->edge()),
				score
			);

			if (score > 0) { // ע���ų�����Ϊ�������������ʱ����������߼нǹ����²��Ϸ��������
				match_res_vec.emplace_back(std::make_pair(now_root->leaf_poor_coedge, score));
			}

			return;
		}

		if (!check_subtree_valid(now_root)) {
			//LOG_DEBUG("check_subtree_valid==false: %d\n", now_root);
			return;
		}

		if (now_root->left_node != nullptr) {
			recursive_match(now_root->left_node);
		}

		if (now_root->right_node != nullptr) {
			recursive_match(now_root->right_node);
		}
	};

	// ƥ�䵥���ߵ����̣����շ���Сto������
	recursive_match(this->root);
	std::sort(match_res_vec.begin(), match_res_vec.end(), [&](const std::pair<Stitch::PoorCoedge, double>& a, const std::pair<Stitch::PoorCoedge, double>& b) {
		return a.second < b.second;
		});
	return match_res_vec;

	LOG_DEBUG("Matching end: %d\n", MarkNum::GetId(poor_coedge1.coedge));

	LOG_INFO("end.");
}

Stitch::MatchTree::MatchTreeNode::MatchTreeNode() :
	//min_range_point{ Stitch::MAXVAL, Stitch::MAXVAL, Stitch::MAXVAL}, // �汾��֧����ôд
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
	�Ƴ��˽ڵ���ָ���ڴ�
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
	���
	���Ʊ߲��޸� ��ע��Ŀǰ���ֻ����bodies��ֻ��һ��body�������������ܻ�����⣩
*/
void Stitch::StitchGapFixer::Start(bool call_fix, bool dont_stitch_coincident)
{
	this->dont_stitch_coincident = dont_stitch_coincident;

	LOG_INFO("EPSLION1: %.5lf;\t EPSLION2: %.5lf;\t EPSLION2_SIN: %.5lf;\t call_fix: %s\t, dont_stitch_coincident: %s\t", EPSLION1, EPSLION2, EPSLION2_SIN, call_fix ? "True" : "False", this->dont_stitch_coincident?"True":"False");

	// ��ʱ��ʼ
	clock_t Stitch_start_clock = std::clock();

	api_initialize_constructors();
	api_initialize_booleans();

	FindPoorCoedge(bodies); // 1
	clock_t Stitch_end_clock_find = std::clock(); // ��ʱ���� find

	MatchPoorCoedge(); // 2
	clock_t Stitch_end_clock_match = std::clock(); // ��ʱ���� match

	RearrangePoorCoedge(); // 3

	if (call_fix) { StitchPoorCoedge(bodies); } // 4
	clock_t Stitch_end_clock_stitch = std::clock(); // ��ʱ���� stitch

	match_tree.DeleteTree(); // ɾ��KDTree

	api_terminate_constructors();
	api_terminate_booleans();

	// ��ӡʱ��
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

	edges_data.Clear();

	match_tree.DeleteTree();

	LOG_INFO("end.");
}

void Stitch::EdgesData::Init(ENTITY_LIST & bodies)
{
	// Ԥ����: a. <vertex,vertex> to edge��map; b. ����ʵ���ȫ���ߵ�vector
	for (int i = 0; i < bodies.count(); i++) {
		ENTITY *ibody = bodies[i];
		ENTITY_LIST edge_list;
		api_get_edges(ibody, edge_list);

		for (int j = 0; j < edge_list.count(); j++) {
			EDGE* iedge = static_cast<EDGE*>(edge_list[j]);
			all_edge_vector.emplace_back(iedge);
			vertex_pair_to_edge_map[std::make_pair(iedge->start(), iedge->end())].emplace_back(iedge);
		}
	}
}

void Stitch::EdgesData::Clear()
{
	vertex_pair_to_edge_map.clear();
	all_edge_vector.clear();
}

std::vector<EDGE*> Stitch::EdgesData::FindEdgesBetweenVertices(VERTEX * v1, VERTEX * v2)
{
	std::vector<EDGE*> res;

	auto v1_v2_edges_it = vertex_pair_to_edge_map.find(std::make_pair(v1, v2));
	auto v2_v1_edges_it = vertex_pair_to_edge_map.find(std::make_pair(v2, v1));

	if (v1_v2_edges_it != vertex_pair_to_edge_map.end()) {
		res.insert(res.end(), v1_v2_edges_it->second.begin(), v1_v2_edges_it->second.end());
	}

	if (v2_v1_edges_it != vertex_pair_to_edge_map.end()) {
		res.insert(res.end(), v2_v1_edges_it->second.begin(), v2_v1_edges_it->second.end());
	}

	return res;
}
