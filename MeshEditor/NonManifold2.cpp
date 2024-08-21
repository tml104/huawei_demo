#include "StdAfx.h"
#include "NonManifold2.h"

/*
	����˳��1
	Ѱ��NonManifold�ıߣ�ͬʱά�����鼯
*/
void NonManifold::NonManifoldFixer::FindNonManifold()
{
	LOG_INFO("start.");

	// 1. 2. �ҳ����з����αߣ�ά��(vertex->edge)map
	for (int i = 0; i < bodies.count(); i++) {
		ENTITY* ibody = (bodies[i]);

		// �������body��edge list
		ENTITY_LIST edge_list;
		api_get_edges(ibody, edge_list);

		for (int j = 0; j < edge_list.count(); j++) {
			EDGE* iedge = dynamic_cast<EDGE*>(edge_list[j]);
			int coedge_count = Utils::CoedgeCount(iedge);

			// �ҵ������αߣ�����nonmanifold_edge_set��
			if (coedge_count > 2) {
				LOG_DEBUG("NonManifold edge found: iedge: %d, coedge_cnt: %d", MarkNum::GetId(iedge), coedge_count);
				nonmanifold_edge_set.insert(iedge);
			}

			// ά��(vertex->edge)map
			if (iedge->start() != nullptr) {
				vertex_to_edge_map[iedge->start()].insert(iedge);
			}

			if (iedge->end() != nullptr) {
				vertex_to_edge_map[iedge->end()].insert(iedge);
			}

			// ά��old_start_vertex_map, old_end_vertex_map�� Ҳ��ά��ԭ�����бߵĳ�ʼ��ʼ���㡢�������㼯��
			old_start_vertex_map[iedge] = iedge->start();
			old_end_vertex_map[iedge] = iedge->end();
		}
	}

	LOG_INFO("end.");
}

/* 
	����˳��2
	���У������ĳ�����������㻷�����ظ��ķ����αߵ��Ǹ�������xloop��������special_nonmanifold_edge_set�� 
*/
void NonManifold::NonManifoldFixer::SpecialCheckNonManifold()
{
	LOG_INFO("start.");

	// �˺�����������ĳ�����α����Ƿ����һ��loop����loop������������ͬ��coedge�������Ӧ��edge���Ǹø����ķ����αߵ�
	auto check_nonmanifold_edge_in_xloop = [&](EDGE* const &iedge_nonmanifold) -> bool {

		// �����÷����α��е�coedge
		COEDGE* icoedge = iedge_nonmanifold->coedge();
		do {
			if (icoedge == nullptr) {
				break;
			}

			// ȡ�ø�coedge��loop��Ȼ��������е�coedge�������edge����������αߣ�iedge_nonmanifold����ͬ����Ŀ
			LOOP* iloop = icoedge->loop();
			int identical_edge_nonmanifold_count = 0;
			COEDGE* jcoedge = iloop->start();
			do {
				if (jcoedge == nullptr) {
					break;
				}

				// �����ͬ����+1
				identical_edge_nonmanifold_count += (jcoedge->edge() == iedge_nonmanifold);

				jcoedge = jcoedge->next();
			} while (jcoedge != nullptr && jcoedge != iloop->start());

			// [debug] ��ӡһ��identical_edge_nonmanifold_count
			//LOG_DEBUG("iedge_nonmanifold:%d, icoedge:%d, iloop:%d, identical_edge_nonmanifold_count: %d.",
			//	MarkNum::GetId(iedge_nonmanifold),
			//	MarkNum::GetId(icoedge),
			//	MarkNum::GetId(iloop),
			//	identical_edge_nonmanifold_count
			//);

			// ���ɹ�����Ӵ�xloop�߽���special_nonmanifold_edge_set�����У�֮����ݳ�����Ȼ�Ϳ������ˣ�
			if (identical_edge_nonmanifold_count == 2) {
				special_nonmanifold_edge_map[iedge_nonmanifold] = iloop;
				return true;
			}
			else if (identical_edge_nonmanifold_count > 2)// �����쳣��������������2�Ļ��Ҳ��ᴦ��
			{
				LOG_ERROR("identical_edge_nonmanifold_count > 2.");
				throw std::runtime_error("identical_edge_nonmanifold_count >2");
			}

			icoedge = icoedge->partner();
		} while (icoedge != nullptr && icoedge != iedge_nonmanifold->coedge());

		return false;
	};

	// ������һ���ҵ������з����αߣ��������Ƿ�����xloop������
	for (auto iedge_it = nonmanifold_edge_set.begin(); iedge_it != nonmanifold_edge_set.end(); iedge_it++) {
		EDGE* iedge_nonmanifold = (*iedge_it);
		bool check_nonmanifold_edge_in_xloop_res = check_nonmanifold_edge_in_xloop(iedge_nonmanifold);
		LOG_DEBUG("iedge_nonmanifold: %d, check_nonmanifold_edge_in_xloop_res: %d", MarkNum::GetId(iedge_nonmanifold), check_nonmanifold_edge_in_xloop_res);
	}

	LOG_INFO("end.");
}

/*
	����˳��3
	ר�Ÿ�special_nonmanifold_edge_set�е����
*/
void NonManifold::NonManifoldFixer::SolveSpecialNonManifold()
{
	LOG_INFO("start.");

	// ����iedge_nonmanifold�е�coedge����������4����������xloop�ϵĵ����ó����γ�һ������
	// Ȼ�����������е�iedge_nonmanifold��coedge�����ݳ����xloop�ϵ�������
	// ����޸Ķ���Ĳ����ǲ���Ŷ
	auto solve_special = [&](EDGE* const &iedge_nonmanifold, LOOP* const &xloop) {
		LOG_DEBUG("solve_special start: iedge_nonmanifold: %d, xloop: %d", MarkNum::GetId(iedge_nonmanifold), MarkNum::GetId(xloop));

		std::vector<COEDGE*> not_in_xloop_vec, in_xloop_vec;
		int coedge_cnt = 0;

		COEDGE* jcoedge = iedge_nonmanifold->coedge();
		//����iedge_nonmanifold�е�coedge����������4����������xloop�ϵĵ����ó����γ�һ������
		do {
			if (jcoedge == nullptr) {
				break;
			}

			LOOP* jloop = jcoedge->loop();
			if (jloop == xloop) {
				in_xloop_vec.emplace_back(jcoedge);
			}
			else {
				not_in_xloop_vec.emplace_back(jcoedge);
			}

			coedge_cnt++;
			jcoedge = jcoedge->partner();
		} while (jcoedge != nullptr && jcoedge != iedge_nonmanifold->coedge());

		if (coedge_cnt != 4) {
			LOG_ERROR("coedge_cnt != 4");
			throw std::runtime_error("coedge_cnt != 4");
		}

		// ��ԵĲ���ͬһ�����ϵ�coedge���ϡ���һ������xloop�ϵ�coedge���ڶ����ǲ���xloop�ϵ�coedge
		std::vector<std::pair<COEDGE*, COEDGE*>> pair_coedge_vec;

		// �ȱ�����xloop�ϵ�coedge��Ȼ�������鲻��xloop�ϵ�coedge�ķ����Ƿ�����һ���¡���һ�µ����Ϊһ��
		// ������Ժ�Ҫ����6��coedgeͬһ�ߵ�����Ļ���������¿��ǣ���Ȼ�Ҳ����û�����ôһ�죩
		for (unsigned int i = 0; i < in_xloop_vec.size(); i++) {
			COEDGE* icoedge_in_xloop = in_xloop_vec[i];

			for (unsigned int j = 0; j < not_in_xloop_vec.size(); j++) {
				COEDGE* jcoedge_not_in_xloop = not_in_xloop_vec[j];
				if (icoedge_in_xloop->sense() != jcoedge_not_in_xloop->sense()) {
					pair_coedge_vec.emplace_back(std::make_pair(icoedge_in_xloop, jcoedge_not_in_xloop));
					LOG_DEBUG("Pair Get: icoedge_in_xloop:%d, jcoedge_not_in_xloop:%d (loop1: %d, loop2:%d)",
						MarkNum::GetId(icoedge_in_xloop),
						MarkNum::GetId(jcoedge_not_in_xloop),
						MarkNum::GetId(icoedge_in_xloop->loop()),
						MarkNum::GetId(jcoedge_not_in_xloop->loop())
					);
				}
			}
		}

		// ����Ϊֹ������Ӧ���������˰�
		// �����±߲��ݴ棬��Ŀ��������������ΪֹӦ����2�Σ�
		std::vector<EDGE*> new_edge_vec;

		for (unsigned int i = 0; i < pair_coedge_vec.size(); i++) {

			// �����±�
			EDGE *new_edge = Utils::CopyEdge(iedge_nonmanifold);

			// ���Ƴɹ�
			new_edge_vec.emplace_back(new_edge);
		}

		// �ٴα���ÿ���ѷ���coedge�ԣ����������±߲����ж����滻�Ȳ���
		for (unsigned int i = 0; i < pair_coedge_vec.size(); i++) {

			COEDGE* coedge1 = pair_coedge_vec[i].first; // xloop�ϵķ����αߵ�coedge
			COEDGE* coedge2 = pair_coedge_vec[i].second; // ����xloop�ϵķ����αߵ�coedge
			EDGE* new_edge = new_edge_vec[i];

			// �޸��±����˺�����partner
			new_edge->set_coedge(coedge1);
			coedge1->set_edge(new_edge);
			coedge2->set_edge(new_edge);

			coedge1->set_partner(coedge2);
			coedge2->set_partner(coedge1);

			// �ھɵı�->���㼯����ά���±ߵĶ���
			old_start_vertex_map[new_edge] = iedge_nonmanifold->start();
			old_end_vertex_map[new_edge] = iedge_nonmanifold->end();

			// �޸Ķ��������㵽�����Ҳŷ����Ǹ��Ȳ����������ǱȽ������
			// ��������Ӳ��ͷƤ��һ���ְɣ��޸�xloop�Լ�����xloop����һ��coedge����loop�ϵ�������ɫ�ߣ���������·����Ρ��е�ͼ����������loop����ʱ����
			// ���������ϣ�����loop������нӵ��ⲿ���ϵģ�Ҳ��Ҫ����һ�����鼯���ⲿ��������loops�ҳ�����Ȼ�󵥶������Ǵ����¶��㣬���������ǶԵģ���������·����Ρ��е��лƱߵ�ͼ���Ҳ��Ǹ���ɫ������ӵ�����������

			// ����xloop�ϵķ����αߵ�coedge ��Ӧloop�Ķ����޸�
			LOOP* loop_coedge2 = coedge2->loop();
			COEDGE* jcoedge = loop_coedge2->start();
			do {
				if (jcoedge == nullptr) {
					break;
				}
				// ������xloop�ϵķ����αߵ�coedge��Ҳ��coedge2����coedge2�����Ѿ���Ϊset_edge�����ú��ˣ�
				if (jcoedge == coedge2) {
					jcoedge = jcoedge->next();
					continue;
				}

				// �ж϶��㲢�޸�
				EDGE* jedge = jcoedge->edge();
				// [debug] �鿴�ļ�����֧��������
				int _set_start_vertex_flag = 0;
				int _set_end_vertex_flag = 0;

				if (old_start_vertex_map[jedge] == iedge_nonmanifold->start()) {
					jedge->set_start(new_edge->start());
					_set_start_vertex_flag = 1;
				}
				else if (old_start_vertex_map[jedge] == iedge_nonmanifold->end()) {
					jedge->set_start(new_edge->end());
					_set_start_vertex_flag = 2;
				}

				if (old_end_vertex_map[jedge] == iedge_nonmanifold->start()) {
					jedge->set_end(new_edge->start());
					_set_end_vertex_flag = 3;
				}
				else if (old_end_vertex_map[jedge] == iedge_nonmanifold->end()) {
					jedge->set_end(new_edge->end());
					_set_end_vertex_flag = 4;
				}

				LOG_DEBUG("solve_special A: loop_coedge2: %d, jcoedge: %d, jedge: %d, _set_start_vertex_flag: %d, _set_end_vertex_flag: %d",
					MarkNum::GetId(loop_coedge2),
					MarkNum::GetId(jcoedge),
					MarkNum::GetId(jedge),
					_set_start_vertex_flag,
					_set_end_vertex_flag
				);

				jcoedge = jcoedge->next();
			} while (jcoedge != nullptr && jcoedge != loop_coedge2->start());

			// ��xloop�ϵģ�����ͱȽ��鷳�������coedge��next��start�Լ�prev��end��
			LOOP* xloop = coedge1->loop();
			//jcoedge = xloop->start(); // �������������������������
			jcoedge = coedge1;

			// [debug] �鿴�ĸ���֧��������
			int _jcoedge_next_set_start_vertex_flag = 0;
			int _jcoedge_prev_set_end_vertex_flag = 0;

			// �޸�next��start
			if (jcoedge->next()->sense() == FORWARD) {
				if (old_start_vertex_map[jcoedge->next()->edge()] == iedge_nonmanifold->start()) {
					jcoedge->next()->edge()->set_start(new_edge->start());
					_jcoedge_next_set_start_vertex_flag = 1;
				}
				else if (old_start_vertex_map[jcoedge->next()->edge()] == iedge_nonmanifold->end()) {
					jcoedge->next()->edge()->set_start(new_edge->end());
					_jcoedge_next_set_start_vertex_flag = 2;
				}
			}
			else { // ��ת�޸�next��end
				if (old_end_vertex_map[jcoedge->next()->edge()] == iedge_nonmanifold->start()) {
					jcoedge->next()->edge()->set_end(new_edge->start());
					_jcoedge_next_set_start_vertex_flag = 3;
				}
				else if (old_end_vertex_map[jcoedge->next()->edge()] == iedge_nonmanifold->end()) {
					jcoedge->next()->edge()->set_end(new_edge->end());
					_jcoedge_next_set_start_vertex_flag = 4;
				}
			}

			// �޸�prev��end
			if (jcoedge->previous()->sense() == FORWARD) {
				if (old_end_vertex_map[jcoedge->previous()->edge()] == iedge_nonmanifold->start()) {
					jcoedge->previous()->edge()->set_end(new_edge->start());
					_jcoedge_prev_set_end_vertex_flag = 1;
				}
				else if (old_end_vertex_map[jcoedge->previous()->edge()] == iedge_nonmanifold->end()) {
					jcoedge->previous()->edge()->set_end(new_edge->end());
					_jcoedge_prev_set_end_vertex_flag = 2;
				}
			}
			else { // ��ת�޸�prev��start
				if (old_start_vertex_map[jcoedge->previous()->edge()] == iedge_nonmanifold->start()) {
					jcoedge->previous()->edge()->set_start(new_edge->start());
					_jcoedge_prev_set_end_vertex_flag = 3;
				}
				else if (old_start_vertex_map[jcoedge->previous()->edge()] == iedge_nonmanifold->end()) {
					jcoedge->previous()->edge()->set_start(new_edge->end());
					_jcoedge_prev_set_end_vertex_flag = 4;
				}
			}

			LOG_DEBUG("solve_special B: xloop: %d, jcoedge: %d, _jcoedge_next_set_start_vertex_flag: %d, _jcoedge_prev_set_end_vertex_flag: %d",
				MarkNum::GetId(xloop),
				MarkNum::GetId(jcoedge),
				_jcoedge_next_set_start_vertex_flag,
				_jcoedge_prev_set_end_vertex_flag
			);
		}

		LOG_DEBUG("solve_special end.");
	};

	for (auto it = special_nonmanifold_edge_map.begin(); it != special_nonmanifold_edge_map.end(); it++) {
		EDGE* iedge_nomanifold = it->first;
		LOOP* xloop = it->second;

		solve_special(iedge_nomanifold, xloop);
	}

	LOG_INFO("end.");
}


// [Deprecated] �˺����Ѿ������ڵڶ���SolveNonManifold2������
void NonManifold::NonManifoldFixer::MaintainFindUnionSet()
{
	LOG_INFO("start.");

	// 3. ά�����鼯
	// �������ͨ������ĳ�������αߵĶ������ڵı߼��ϣ�������������е�������edge��Ȼ�󽫶�Ӧedge�������loop����һ��unite
	auto traverse_vertex_incident_edge_set = [&](std::set<EDGE*>& vertex_incident_edge_set) {
		LOG_DEBUG("start.");

		for (auto it2 = vertex_incident_edge_set.begin(); it2 != vertex_incident_edge_set.end(); it2++) { // *it2: �����α߶������ڱ߼����е�һ����
			int coedge_cnt = Utils::CoedgeCount(*it2);

			// �ų���(*it2)�������nonmanifold_edge�����
			if (nonmanifold_edge_set.count(*it2)) {
				LOG_DEBUG(
					"Maintain LoopFindUnionSet, nonmanifold_edge_set.count(*it2)>0: *it2:%d, coedge_cnt:%d",
					MarkNum::GetId(*it2),
					coedge_cnt
				);
				continue;
			}

			// �ų����쳣�ߣ�coedge�������������αߣ��������������ʵ����������һ���Ѿ��ų��ò���ˣ�
			if (coedge_cnt != 2) {
				LOG_DEBUG("Maintain LoopFindUnionSet, coedge_cnt !=2: *it2:%d, coedge_cnt:%d", MarkNum::GetId(*it2), coedge_cnt);

				continue;
			}

			LOG_DEBUG("Maintain LoopFindUnionSet, check passed : *it2:%d, coedge_cnt: %d", MarkNum::GetId(*it2),
				coedge_cnt);

			// ���ͨ���󣬽������������αߵ�coedge��Ȼ����Щcoedge����loopȫ������icoedge_loop_vec�У��Ժ���Щloopȫ������ͬһ�����鼯
			std::vector<LOOP*> icoedge_loop_vec;

			auto icoedge = (*it2)->coedge();
			do {
				if (icoedge == nullptr) {
					break;
				}

				// ������loop����icoedge_loop_vec�У��Ժ�ʹ�ò��鼯unite
				icoedge_loop_vec.emplace_back(icoedge->loop());

				icoedge = icoedge->partner();
			} while (icoedge != nullptr && icoedge != (*it2)->coedge());

			// TODO: ���ұߵ��������
			LOG_INFO("icoedge_loop_vec size: %d", icoedge_loop_vec.size());

			// �����α�������loopȫ��unite����
			for (int k = 1; k < icoedge_loop_vec.size(); k++) {
				auto &loop1 = icoedge_loop_vec[k - 1];
				auto &loop2 = icoedge_loop_vec[k];

				loop_findunionset.unite(loop1, loop2);

				LOG_DEBUG("Maintain LoopFindUnionSet, unite: %d, %d",
					MarkNum::GetId(loop1),
					MarkNum::GetId(loop2)
				);

			}
		}

		LOG_DEBUG("end.");
	};

	// ���������α߼���nonmanifold_edge_set��Ȼ����Щ�ߵ�begin��end��vertex��Ӧ�����ڱ߼�������traverse_vertex_incident_edge_set�У��õ�loop���鼯
	for (auto it = nonmanifold_edge_set.begin(); it != nonmanifold_edge_set.end(); it++) {
		auto& start_vertex_incident_edge_set = vertex_to_edge_map[(*it)->start()];
		traverse_vertex_incident_edge_set(start_vertex_incident_edge_set);

		auto& end_vertex_incident_edge_set = vertex_to_edge_map[(*it)->end()];
		traverse_vertex_incident_edge_set(end_vertex_incident_edge_set);
	}

	LOG_INFO("end.");
}


// Ŀǰ����㷨ֻ�ܽ�����д��ֽ����ȫ����һ�������αߵ���������Ҫ�ǻ��б�ĵ������ڷ����αߵĶ����ϵ������������������ɣ���Ϊÿ�������ε�ӵ�֮����ȥ�����ߵķ�ʽ����Ҫ�ٿ��ǵġ�
// ��������Ŀǰû������������ʱ�Ȳ����˰� ���� 2024��3��5�� 14:42:28
// [Deprecated] �����еڶ���ʵ��������������ʱ����
void NonManifold::NonManifoldFixer::SolveNonManifold()
{
	LOG_INFO("start.");

	// 4. ����ÿ�������αߣ�����coedge����
	// TODO��������д���е����⣬coedge����Ӧ�û���Ҫ���з����α���˭����Ϣ����������ֱ���ж�nonmanifold_coedge_map�е�vector��coedge��������2���ܵ�֪�ǲ���ͬһ�������α�����ģ���Ŀǰ�Ĵ�����жϻᵼ�±����ܺϲ���������ܺϲ���
	//����ΪĿǰ����������ÿ��group����ֻ����������coedge������������������������������鼯�ϲ����ٷֱ�Ҳ�����Ǹ���һ����һ�鲢�鼯������coedge���������п��ܻ����һ�����鼯������4������coedge�������������߼�Ҫ�İ���
	//��Ҳ����˵����Ҫ�ĳ� ����ĳ�ض������α�->���ɹ��ڴ˷����αߵĲ��鼯->���ݴ˲��鼯������ ���ģʽ����
	auto group_map = loop_findunionset.get_group_map(); // ���get_group_map����(group father->group set)��ӳ��
	std::map<LOOP*, std::vector<COEDGE*>> nonmanifold_coedge_map; //(group loop father->�����α�coedge����)��map��������ÿ��groupӦ��ֻ������

	//[debug] ���group_map������һ��group map
	LOG_DEBUG("Checking group_map start.");
	for (auto it = group_map.begin(); it != group_map.end(); it++) {
		LOG_DEBUG("group map info: group father: %d, size: %d", MarkNum::GetId(it->first), it->second.size());
	}
	LOG_DEBUG("Checking group_map end.");


	// ����ÿ�������αߣ�����coedge����
	for (auto it = nonmanifold_edge_set.begin(); it != nonmanifold_edge_set.end(); it++) {
		auto &iedge_nonmanifold = (*it);

		LOG_DEBUG("Solving iedge_nonmanifold: %d", MarkNum::GetId(iedge_nonmanifold));

		// ����coedge��һ���ǻ���4�����������๤��
		auto icoedge = iedge_nonmanifold->coedge();
		do {

			if (icoedge == nullptr) {
				LOG_ERROR(
					"icoedge is nullptr: iedge_nonmanifold:%d, icoedge : %d",
					MarkNum::GetId(iedge_nonmanifold),
					MarkNum::GetId(icoedge)
				);
				break;
			}

			auto iloop = icoedge->loop();
			auto iloop_group_father = loop_findunionset.get_father(iloop); //ȡ��coedge��loop�����ĸ�group��
			nonmanifold_coedge_map[iloop_group_father].emplace_back(icoedge); //����group����coedge

			icoedge = icoedge->partner();
		} while (icoedge != nullptr && icoedge != iedge_nonmanifold->coedge());

		// 5.  ������ɣ������±߲��޸�ָ��
		// ���ڷֳ������� �ȸ��ƶ�Ӧ������edgeָ�벢���棬֮���޸ĵ�ʱ���ٴ�����ȡ����

		std::vector<EDGE*> new_edge_vec;

		// 5.1 ������һ�����漰�������ڵ�group��Ȼ��Ϊÿ��group�����µı߲�������new_edge_vec��
		// nonmanifold_coedge_map: (group loop father->�����α�coedge����)��map��������ÿ��groupӦ��ֻ������
		// �˴���nonmanifold_coedge_map�Ĵ�СӦ����group����һ�£�Ҳ����ѭ����ִ�д�����
		for (auto it2 = nonmanifold_coedge_map.begin(); it2 != nonmanifold_coedge_map.end(); it2++) {
			auto & this_group_father = it2->first;
			auto &nonmanifold_coedge_vec = it2->second;

			LOG_DEBUG("5.1: Solving group (father): %d", MarkNum::GetId(this_group_father));

			// ����ǲ���ֻ��������Ӧcoedge
			if (nonmanifold_coedge_vec.size() != 2) {
				LOG_ERROR("5.1: nonmanifold_coedge_set.size() != 2, continue will create nonmanifold edge!");
				continue;
			}

			// ���ͨ����Ĭ�����group��Ӧֻ������coedge���������漰���Ǹ�ԲͲ��һƬ������еĻ�������


			// �����±�
			EDGE* new_edge = Utils::CopyEdge(iedge_nonmanifold);

			// ���Ƴɹ�����������new_edge_vec���Թ���һ��ʹ��
			LOG_DEBUG("Copy succeed. old: %d, new_edge_address: %d", MarkNum::GetId(iedge_nonmanifold), new_edge);

			new_edge_vec.emplace_back(new_edge);
		}

		// 5.2 �ٴα�����һ�����漰�������ڵ�group��Ȼ��Ϊ���group�е�loop���������±ߣ��߽����滻����Ȳ���
		int new_edge_pick_index = 0;
		for (auto it2 = nonmanifold_coedge_map.begin(); it2 != nonmanifold_coedge_map.end(); it2++) {
			auto & this_group_father = it2->first;
			auto & nonmanifold_coedge_vec = it2->second;

			// ����ǲ���ֻ��������Ӧcoedge
			if (nonmanifold_coedge_vec.size() != 2) {
				LOG_ERROR("nonmanifold_coedge_set.size() != 2, continue will create nonmanifold edge!");
				continue;
			}

			// ���ͨ����Ĭ�����group��Ӧֻ������coedge���������漰���Ǹ�ԲͲ��һƬ������еĻ�������
			auto &coedge1 = nonmanifold_coedge_vec[0];
			auto &coedge2 = nonmanifold_coedge_vec[1];

			// ��new_edge vector���ó��ߣ����桰�����±ߡ����������ע��˳��Ҫ�����洴����ʱ���ϸ�һ�£�
			EDGE* new_edge = new_edge_vec[new_edge_pick_index++];

			// �޸��±�����
			new_edge->set_coedge(coedge1); // ��㶨��
			coedge1->set_edge(new_edge);
			coedge2->set_edge(new_edge);

			// ����partner
			coedge1->set_partner(coedge2);
			coedge2->set_partner(coedge1);

			LOG_DEBUG(
				"Coedge partner set:%d %d",
				MarkNum::GetId(coedge1->partner()),
				MarkNum::GetId(coedge2->partner())
			);

			// �ھɵı�->���㼯����ά���±ߵĶ���
			old_start_vertex_map[new_edge] = iedge_nonmanifold->start();
			old_end_vertex_map[new_edge] = iedge_nonmanifold->end();

			// �޸Ķ��㣺����group��������loop���޸ıߵĶ���
			auto &this_part_group_loops = group_map[this_group_father];

			for (auto it3 = this_part_group_loops.begin(); it3 != this_part_group_loops.end(); it3++) {
				auto jloop = (*it3);
				auto jcoedge = jloop->start();

				LOG_DEBUG("Solving jloop: %d", MarkNum::GetId(jloop));

				//����loop�е�coedge
				do {
					if (jcoedge == nullptr) {
						break;
					}
					auto jedge = jcoedge->edge();

					LOG_DEBUG(
						"Solving jcoedge, jcoedge: %d, %d",
						MarkNum::GetId(jcoedge),
						MarkNum::GetId(jedge)
					);

					// ���������αߣ������α��ϵ�coedge��edge�ᱻֱ�����ã���˲����޸Ķ��㣩
					if (nonmanifold_edge_set.count(jedge)) {
						LOG_DEBUG("Modify vertex SKIP nonmanifold_edge: jedge: %d", MarkNum::GetId(jedge));

						jcoedge = jcoedge->next();

						if (!(jcoedge != nullptr && jcoedge != jloop->start())) {
							break;
						}
						continue;
					}

					// ֮ǰ����ֻ�ж��˵�ǰedge����뵱ǰ�����α������ͬ���������
					// ����Ӧ�øĳ�4����ϰ�

					// [debug] �鿴�ļ�����֧��������
					int _set_start_vertex_flag = 0;
					int _set_end_vertex_flag = 0;

					if (old_start_vertex_map[jedge] == iedge_nonmanifold->start()) {
						jedge->set_start(new_edge->start());
						_set_start_vertex_flag = 1;
					}
					else if (old_start_vertex_map[jedge] == iedge_nonmanifold->end()) {
						jedge->set_start(new_edge->end());
						_set_start_vertex_flag = 2;
					}

					if (old_end_vertex_map[jedge] == iedge_nonmanifold->start()) {
						jedge->set_end(new_edge->start());
						_set_end_vertex_flag = 3;
					}
					else if (old_end_vertex_map[jedge] == iedge_nonmanifold->end()) {
						jedge->set_end(new_edge->end());
						_set_end_vertex_flag = 4;
					}

					// [debug] �鿴�ļ�����֧��������
					LOG_DEBUG(
						"Modify vertex FLAG: jedge: %d, flag: (_set_start_vertex_flag: %d, _set_end_vertex_flag: %d)",
						MarkNum::GetId(jedge),
						_set_start_vertex_flag,
						_set_end_vertex_flag
					);

					jcoedge = jcoedge->next();
				} while (jcoedge != nullptr && jcoedge != jloop->start());

			}
		}

		// ������ڱ���(group loop father->�����α�coedge����)��map��������ÿ��groupӦ��ֻ������
		nonmanifold_coedge_map.clear();
	}

	LOG_INFO("end.");
}

/*
	�ڶ���SolveNonManifoldʵ�֣���ԭMaintainFindUnionSet��SolveNonManifold�ϲ����佫��ÿ�������α�Ϊ��λ����������
*/
void NonManifold::NonManifoldFixer::SolveNonManifold2()
{
	LOG_INFO("start.");

	// �·���ԭMaintainFindUnionSet�Ĳ���

	// 3. ά�����鼯
	// �������ͨ������ĳ�������αߵĶ������ڵı߼��ϣ�������������е�������edge��Ȼ�󽫶�Ӧedge�������loop����һ��unite
	auto traverse_vertex_incident_edge_set = [&](std::set<EDGE*>& vertex_incident_edge_set) {
		LOG_INFO("traverse_vertex_incident_edge_set start.");

		for (auto it2 = vertex_incident_edge_set.begin(); it2 != vertex_incident_edge_set.end(); it2++) { // *it2: �����α߶������ڱ߼����е�һ����
			int coedge_cnt = Utils::CoedgeCount(*it2);

			// �ų���(*it2)�������nonmanifold_edge�����
			if (nonmanifold_edge_set.count(*it2)) {
				LOG_DEBUG(
					"Maintain LoopFindUnionSet, nonmanifold_edge_set.count(*it2)>0: *it2: %d, coedge_cnt:%d",
					MarkNum::GetId(*it2),
					coedge_cnt
				);
				continue;
			}

			// �ų����쳣�ߣ�coedge�������������αߣ��������������ʵ����������һ���Ѿ��ų��ò���ˣ�
			if (coedge_cnt != 2) {
				LOG_DEBUG(
					"Maintain LoopFindUnionSet, coedge_cnt !=2: *it2: %d, coedge_cnt:%d",
					MarkNum::GetId(*it2),
					coedge_cnt
				);

				continue;
			}

			// ���ͨ��
			LOG_DEBUG(
				"Maintain LoopFindUnionSet, check passed : *it2: %d, coedge_cnt: %d",
				MarkNum::GetId(*it2),
				coedge_cnt
			);

			// ���ͨ���󣬽����������ߵ�coedge��Ȼ����Щcoedge����loopȫ������icoedge_loop_vec�У��Ժ���Щloopȫ������ͬһ�����鼯
			std::vector<LOOP*> icoedge_loop_vec;

			auto icoedge = (*it2)->coedge();
			do {
				if (icoedge == nullptr) {
					break;
				}

				// ������loop����icoedge_loop_vec�У��Ժ�ʹ�ò��鼯unite
				icoedge_loop_vec.emplace_back(icoedge->loop());

				icoedge = icoedge->partner();
			} while (icoedge != nullptr && icoedge != (*it2)->coedge());

			// TODO: ���ұߵ��������
			LOG_DEBUG("icoedge_loop_vec size: %d", icoedge_loop_vec.size());


			for (int k = 1; k < icoedge_loop_vec.size(); k++) {
				auto &loop1 = icoedge_loop_vec[k - 1];
				auto &loop2 = icoedge_loop_vec[k];

				loop_findunionset.unite(loop1, loop2);

				LOG_DEBUG(
					"Maintain LoopFindUnionSet, unite: %d, %d",
					MarkNum::GetId(loop1),
					MarkNum::GetId(loop2)
				);
			}
		}

		LOG_DEBUG("traverse_vertex_incident_edge_set end.");
	};

	auto solve_single_nonmanifold_edge = [&](EDGE* iedge_nonmanifold) {
		LOG_INFO("solve_single_nonmanifold_edge start.");

		// 4. ����ÿ�������αߣ�����coedge����
		// TODO:����֪�����費��Ҫ��coedge���ಽ����ά�������α���˭�����Ϣ���д��º���飩������Ŀǰ����ȷ�����Ƕ�����������������ŵ������������ǲ��������ģ�
		auto group_map = loop_findunionset.get_group_map(); // ���get_group_map����(group father->group set)��ӳ��
		std::map<LOOP*, std::vector<COEDGE*>> nonmanifold_coedge_map; //(group loop father->�����α�coedge����)��map��������ÿ��groupӦ��ֻ������

		//[debug] ���group_map������һ��group map
		LOG_DEBUG("Check group_map start.");

		for (auto it = group_map.begin(); it != group_map.end(); it++) {
			auto& group = it->second;

			LOG_DEBUG(
				"group map info: group father:%d, size:%d",
				MarkNum::GetId(it->first),
				group.size()
			);

			for (auto group_it = group.begin(); group_it != group.end(); group_it++) {
				LOG_DEBUG(" -> loop: %d", MarkNum::GetId(*group_it));
			}
		}

		LOG_DEBUG("Check group_map end.");

		// ���ڵ�ǰ�����α�iedge_nonmanifold������coedge����
		COEDGE* icoedge = iedge_nonmanifold->coedge();
		do {
			if (icoedge == nullptr) {
				break;
			}

			LOOP* iloop = icoedge->loop();
			LOOP* iloop_group_father = loop_findunionset.get_father(iloop); //ȡ��coedge��loop�����ĸ�group��
			nonmanifold_coedge_map[iloop_group_father].emplace_back(icoedge); //����group����coedge

			icoedge = icoedge->partner();
		} while (icoedge != nullptr && icoedge != iedge_nonmanifold->coedge());

		// 5.  ������ɣ������±߲��޸�ָ��
		// ���ڷֳ������� �ȸ��ƶ�Ӧ������edgeָ�벢���棬֮���޸ĵ�ʱ���ٴ�����ȡ����

		std::vector<EDGE*> new_edge_vec;
		// 5.1 ������һ�����漰�������ڵ�group��Ȼ��Ϊÿ��group�����µı߲�������new_edge_vec��
		// nonmanifold_coedge_map: (group loop father->�����α�coedge����)��map��������ÿ��groupӦ��ֻ������
		// �˴���nonmanifold_coedge_map�Ĵ�СӦ����group����һ�£�Ҳ����ѭ����ִ�д�����
		for (auto it2 = nonmanifold_coedge_map.begin(); it2 != nonmanifold_coedge_map.end(); it2++) {
			auto& this_group_father = it2->first;
			auto& nonmanifold_coedge_vec = it2->second;
			LOG_DEBUG("5.1: Solving group (father): %d", MarkNum::GetId(this_group_father));

			// ����ǲ���ֻ��������Ӧcoedge
			if (nonmanifold_coedge_vec.size() != 2) {
				LOG_ERROR("5.1: nonmanifold_coedge_set.size() != 2, continue will create nonmanifold edge!");
				continue;
			}

			// ���ͨ����Ĭ�����group��Ӧֻ������coedge
			// �����±ߣ���Ԥ����new_edge_vec��
			EDGE* new_edge = Utils::CopyEdge(iedge_nonmanifold);
			new_edge_vec.emplace_back(new_edge);
		}

		//5.2 �ٴα�����һ�����漰�������ڵ�group��Ȼ��Ϊ���group�е�loop���������±ߣ��߽����滻����Ȳ���
		// ע��bug�����˴�����Ժ���Ҫ�ع��Ļ������С�ģ����ѭ�����ϸ�ѭ��һ���Ƿֿ��Ķ���Ƕ�׹�ϵ��
		int new_edge_pick_index = 0;
		for (auto it2 = nonmanifold_coedge_map.begin(); it2 != nonmanifold_coedge_map.end(); it2++) {
			auto & this_group_father = it2->first;
			auto & nonmanifold_coedge_vec = it2->second;

			// ����ǲ���ֻ��������Ӧcoedge
			if (nonmanifold_coedge_vec.size() != 2) {
				LOG_ERROR("5.2: nonmanifold_coedge_set.size() != 2, continue will create nonmanifold edge!");
				continue;
			}

			// ���ͨ����Ĭ�����group��Ӧֻ������coedge���������漰���Ǹ�ԲͲ��һƬ������еĻ�������
			auto &coedge1 = nonmanifold_coedge_vec[0];
			auto &coedge2 = nonmanifold_coedge_vec[1];

			// ��new_edge vector���ó��ߣ����桰�����±ߡ����������ע��˳��Ҫ�����洴����ʱ���ϸ�һ�£�
			EDGE* new_edge = new_edge_vec[new_edge_pick_index++];

			// �޸��±�����
			new_edge->set_coedge(coedge1); // ��㶨��
			coedge1->set_edge(new_edge);
			coedge2->set_edge(new_edge);

			// ����partner
			coedge1->set_partner(coedge2);
			coedge2->set_partner(coedge1);

			LOG_DEBUG(
				"5.2: Coedge partner set: %d %d",
				MarkNum::GetId(coedge1->partner()),
				MarkNum::GetId(coedge2->partner())
			);

			// �ھɵı�->���㼯����ά���±ߵĶ���
			old_start_vertex_map[new_edge] = iedge_nonmanifold->start();
			old_end_vertex_map[new_edge] = iedge_nonmanifold->end();

			// �޸Ķ��㣺����group��������loop���޸ıߵĶ���
			auto &this_part_group_loops = group_map[this_group_father];

			for (auto it3 = this_part_group_loops.begin(); it3 != this_part_group_loops.end(); it3++) {
				LOOP* jloop = (*it3);
				COEDGE* jcoedge = jloop->start();

				LOG_DEBUG("5.2: Modifying jloop: %d", MarkNum::GetId(jloop));

				do {
					if (jcoedge == nullptr) {
						break;
					}
					EDGE* jedge = jcoedge->edge();

					LOG_DEBUG(
						"5.2: Modifying jcoedge: jcoedge :%d, jedge: %d",
						MarkNum::GetId(jcoedge),
						MarkNum::GetId(jedge)
					);

					// ���������αߣ������α��ϵ�coedge��edge�ᱻֱ�����ã���˲����޸Ķ��㣩
					if (nonmanifold_edge_set.count(jedge)) {
						LOG_DEBUG("Modify vertex SKIP nonmanifold_edge: jedge: %d", MarkNum::GetId(jedge));

						jcoedge = jcoedge->next();

						if (!(jcoedge != nullptr && jcoedge != jloop->start())) {
							break;
						}
						continue;
					}

					// ֮ǰ����ֻ�ж��˵�ǰedge����뵱ǰ�����α������ͬ���������
					// ����Ӧ�øĳ�4����ϰ�

					// [debug] �鿴�ļ�����֧��������
					int _set_start_vertex_flag = 0;
					int _set_end_vertex_flag = 0;

					if (old_start_vertex_map[jedge] == iedge_nonmanifold->start()) {
						jedge->set_start(new_edge->start());
						_set_start_vertex_flag = 1;
					}
					else if (old_start_vertex_map[jedge] == iedge_nonmanifold->end()) {
						jedge->set_start(new_edge->end());
						_set_start_vertex_flag = 2;
					}

					if (old_end_vertex_map[jedge] == iedge_nonmanifold->start()) {
						jedge->set_end(new_edge->start());
						_set_end_vertex_flag = 3;
					}
					else if (old_end_vertex_map[jedge] == iedge_nonmanifold->end()) {
						jedge->set_end(new_edge->end());
						_set_end_vertex_flag = 4;
					}

					// [debug] �鿴�ļ�����֧��������
					LOG_DEBUG(
						"Modify vertex FLAG: jcoedge: %d, jedge: %d, flag: (_set_start_vertex_flag: %d, _set_end_vertex_flag: %d)",
						MarkNum::GetId(jcoedge),
						MarkNum::GetId(jedge),
						_set_start_vertex_flag,
						_set_end_vertex_flag);

					jcoedge = jcoedge->next();
				} while (jcoedge != nullptr && jcoedge != jloop->start());
			}

		}

		LOG_DEBUG("solve_single_nonmanifold_edge end.");
	};

	// ���������α߼���nonmanifold_edge_set��Ȼ����Щ�ߵ�begin��end��vertex��Ӧ�����ڱ߼�������traverse_vertex_incident_edge_set�У��õ�loop���鼯
	for (auto it = nonmanifold_edge_set.begin(); it != nonmanifold_edge_set.end(); it++) {

		// ���������it����ǰ�����ķ�����edge����special_nonmanifold_edge_map��������
		if (special_nonmanifold_edge_map.count(*it)) {
			LOG_DEBUG("Skip special_nonmanifold_edge: edge: %d", MarkNum::GetId(*it));
			continue;
		}

		auto& start_vertex_incident_edge_set = vertex_to_edge_map[(*it)->start()];
		traverse_vertex_incident_edge_set(start_vertex_incident_edge_set);

		auto& end_vertex_incident_edge_set = vertex_to_edge_map[(*it)->end()];
		traverse_vertex_incident_edge_set(end_vertex_incident_edge_set);

		// �·��Ƿ��ԭSolveNonManifold�Ĳ���
		solve_single_nonmanifold_edge((*it));

		// �£�������鼯
		loop_findunionset.clear();
	}

	LOG_INFO("end.");
}

void NonManifold::NonManifoldFixer::Start()
{
	// ��ʱ��ʼ
	clock_t NonManifold_start_clock = std::clock();

	api_initialize_constructors();
	api_initialize_booleans();

	NonManifold::Debug::PrintLoopsInfo(bodies);

	// ������������Ӱ���С���ȵ��ã�
	FindNonManifold();
	SpecialCheckNonManifold();

	clock_t NonManifold_end_clock_find = std::clock(); // ��ʱ���� find

	SolveSpecialNonManifold(); // ��������������
	SolveNonManifold2(); // ���������õڶ���ͳһʵ��

	clock_t NonManifold_end_clock_solve = std::clock(); // ��ʱ���� solve

	// ��ӡʱ��
	double nonmanifold_end_clock_find_cost = static_cast<double>(NonManifold_end_clock_find - NonManifold_start_clock) / CLOCKS_PER_SEC;
	double nonmanifold_end_clock_find_solve = static_cast<double>(NonManifold_end_clock_solve - NonManifold_start_clock) / CLOCKS_PER_SEC;
	LOG_INFO("nonmanifold_end_clock_find_cost: %.5lf sec ", nonmanifold_end_clock_find_cost);
	LOG_INFO("nonmanifold_end_clock_find_solve: %.5lf sec ", nonmanifold_end_clock_find_solve);

	api_terminate_constructors();
	api_terminate_booleans();
}

void NonManifold::NonManifoldFixer::Clear()
{
	nonmanifold_edge_set.clear();
	special_nonmanifold_edge_map.clear();
	old_start_vertex_map.clear();
	old_end_vertex_map.clear();
	vertex_to_edge_map.clear();
	loop_findunionset.clear();
}

/*
	���鼯��get father
*/
LOOP * NonManifold::LoopFindUnionSet::get_father(LOOP * loop1)
{
	auto it = father_map.find(loop1);
	if (it == father_map.end()) {
		father_map[loop1] = loop1;
		return loop1;
	}

	if (it->second == loop1) {
		return loop1;
	}

	// ·��ѹ��
	return father_map[loop1] = get_father(it->second);
}

/*
	���鼯�� unite ����
	���룺����loop�����Բ��Ƕ�Ӧgroup��father��
*/
void NonManifold::LoopFindUnionSet::unite(LOOP * loop1, LOOP * loop2)
{
	auto loop1_father = get_father(loop1);
	auto loop2_father = get_father(loop2);

	if (loop1_father == loop2_father) {
		return;
	}

	father_map[loop1_father] = loop2_father; //δ·��ѹ��
}

/*
	ȡ�� (group father -> group����set����һϵ��ͬһgroup�µ�loop��)��map
*/
std::map<LOOP*, std::set<LOOP*>> NonManifold::LoopFindUnionSet::get_group_map()
{
	std::map<LOOP*, std::set<LOOP*>> group;

	for (auto it = father_map.begin(); it != father_map.end(); it++) {
		group[get_father(it->first)].insert(it->first); // ȷ��·��ѹ��
	}

	return group;
}

/*
	���
*/
void NonManifold::LoopFindUnionSet::clear()
{
	father_map.clear();
}

void NonManifold::Debug::PrintLoopsInfo(ENTITY_LIST & bodies)
{
	//LOG_INFO("start.");

	for (int i = 0; i < bodies.count(); i++) {
		ENTITY* ibody = (bodies[i]);

		// ȡ��body�ڵ�����loop
		ENTITY_LIST loops_list;
		api_get_loops(ibody, loops_list);

		for (int j = 0; j < loops_list.count(); j++) {
			LOOP* jloop = dynamic_cast<LOOP*>(loops_list[j]);

			// ������loop�������coedge��edge���
			COEDGE* jcoedge = jloop->start();
			do {
				if (jcoedge == nullptr) {
					break;
				}

				EDGE* jedge = jcoedge->edge();

				// ���
				LOG_DEBUG(
					"loop: %d, coedge:%d, edge:%d.",
					MarkNum::GetId(jloop),
					MarkNum::GetId(jcoedge),
					MarkNum::GetId(jedge)
				);

				jcoedge = jcoedge->next();
			} while (jcoedge != nullptr && jcoedge != jloop->start());
		}

	}

	//LOG_INFO("end.");
}
