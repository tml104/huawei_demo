#include "StdAfx.h"
#include "Exp1.h"

/*
	����ȱ�߼��ϣ�������߼�������NonManifold::FindNonManifold��
*/
void Exp1::Exp1::UpdateBadCoedgeSet(ENTITY_LIST & bodies)
{
	LOG_INFO("start.");

	for (int i = 0; i < bodies.count(); i++) {
		ENTITY* ibody = (bodies[i]);

		ENTITY_LIST edge_list;
		api_get_edges(ibody, edge_list);

		for (int j = 0; j < edge_list.count(); j++) {

			EDGE* iedge = dynamic_cast<EDGE*>(edge_list[j]);
			int coedge_count = Utils::CoedgeCount(iedge);

			// ע���ж������Ĳ�ͬ
			if (coedge_count < 2){ // �Ʊ�
				LOG_INFO("Bad edge found: iedge: %d, iedge_coedge: %d, iedge_loop: %d, coedge_cnt: %d",
					MarkNum::GetId(iedge),
					MarkNum::GetId(iedge->coedge()),
					MarkNum::GetId(iedge->coedge()->loop()),
					coedge_count
				);

				bad_edge_set.insert(iedge);

			}
			else if (coedge_count > 2) { // �����Σ�������
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
	����iloop�����б߲���������ߵļ�����Ϣ���Ӹ�����begin_coedge��ʼ�������������ֱ�Ӵ�iloop->start()ȡ�������traverse_incident_loopsΪtrue������Ժ�ݹ���ô˺�������������loop����Ϣ��
*/
void Exp1::Exp1::TraverseLoops(LOOP * iloop, bool traverse_incident_loops, COEDGE* begin_coedge)
{
	LOG_INFO("start.");

	std::vector<LOOP*> incident_loops_vec;

	// ���ָ������ʼcoedge�����ڣ������ӵ�ǰ������iloop������һ��coedge
	if (begin_coedge == nullptr) {
		begin_coedge = iloop->start(); 
	}

	LOG_INFO("iloop: %d, traverse_incident_loops: %s, begin_coedge: %d ",
			MarkNum::GetId(iloop),
			traverse_incident_loops ? "true" : "false",
			MarkNum::GetId(begin_coedge)
		);

	// �������ϵ�coedge����ȡedge�ļ�����Ϣ����
	COEDGE* jcoedge = begin_coedge;
	do {
		if (jcoedge == nullptr) {
			break;
		}

		// ��ӡ��ǰ������coedge��edge�ļ�����Ϣ
		EDGE* jedge = jcoedge->edge();

		LOG_INFO("jcoedge: %d, jedge: %d ",
				MarkNum::GetId(jcoedge),
				MarkNum::GetId(jedge)
			);

		GeometryUtils::PrintEdgeGeometry(jedge);

		// ���traverse_incident_loopsΪtrue���������loop,�����ǵ�ָ������incident_loops_vec��
		if (traverse_incident_loops){
			COEDGE* kcoedge = jcoedge->partner();
			do {
				// ע���ų�����ѭ�������
				if (kcoedge == nullptr || kcoedge == jcoedge) {
					break;
				}

				LOOP* kloop = kcoedge->loop();

				if (kloop != iloop) // ���ų�һ�Σ��Ǳ�Ҫ��
				{
					incident_loops_vec.emplace_back(kloop);
				}

				kcoedge = kcoedge->partner();
			} while (kcoedge != nullptr && kcoedge != jcoedge->partner());
		}

		jcoedge = jcoedge->next();
	} while (jcoedge != nullptr && jcoedge != begin_coedge);


	// ��ȡ��ǰiloop��face�ļ�����Ϣ
	FACE* iface = iloop->face();
	GeometryUtils::PrintFaceGeometry(iface);

	// ���traverse_incident_loopsΪtrue���������loop����Ϣ
	// ���Ȱ����ȥ����������Ϣ�������ң�
	//if (traverse_incident_loops) {
	//	for (auto& incident_loop_it = incident_loops_vec.begin(); incident_loop_it != incident_loops_vec.end(); incident_loop_it++){
	//		TraverseLoops(*incident_loop_it, false); // �ݹ飨���ǽ������Ͳ��ݹ��ˣ�
	//	}
	//}

	LOG_INFO("end.");
}

/*
	ʵ������
*/
void Exp1::Exp1::StartExperiment(ENTITY_LIST & bodies)
{
	LOG_INFO("start.");

	UpdateBadCoedgeSet(bodies);

	for (auto& bad_edge_it = bad_edge_set.begin(); bad_edge_it != bad_edge_set.end(); bad_edge_it++) {
		EDGE* iedge = (*bad_edge_it);
		COEDGE* icoedge = iedge->coedge();
		LOOP* iloop = icoedge->loop(); // �Ի�����˵�������Ҳֻ��һ����

		TraverseLoops(iloop, true, icoedge);
	}

	LOG_INFO("end.");
}

void Exp1::Exp1::Init(ENTITY_LIST & bodies)
{
	api_initialize_constructors();
	api_initialize_booleans();

	// ʵ��1
	StartExperiment(bodies);

	api_terminate_constructors();
	api_terminate_booleans();
}
