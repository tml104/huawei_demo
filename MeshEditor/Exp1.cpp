#include "StdAfx.h"
#include "Exp1.h"

/*
	更新缺边集合（这个的逻辑类似于NonManifold::FindNonManifold）
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

			// 注意判定条件的不同
			if (coedge_count < 2){ // 破边
				LOG_INFO("Bad edge found: iedge: %d, iedge_coedge: %d, iedge_loop: %d, coedge_cnt: %d",
					MarkNum::GetId(iedge),
					MarkNum::GetId(iedge->coedge()),
					MarkNum::GetId(iedge->coedge()->loop()),
					coedge_count
				);

				bad_edge_set.insert(iedge);

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
void Exp1::Exp1::TraverseLoops(LOOP * iloop, bool traverse_incident_loops, COEDGE* begin_coedge)
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

		GeometryUtils::PrintEdgeGeometry(jedge);

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
	GeometryUtils::PrintFaceGeometry(iface);

	// 如果traverse_incident_loops为true则遍历相邻loop的信息
	// （先把这个去掉，否则信息过于杂乱）
	//if (traverse_incident_loops) {
	//	for (auto& incident_loop_it = incident_loops_vec.begin(); incident_loop_it != incident_loops_vec.end(); incident_loop_it++){
	//		TraverseLoops(*incident_loop_it, false); // 递归（但是接下来就不递归了）
	//	}
	//}

	LOG_INFO("end.");
}

/*
	实验流程
*/
void Exp1::Exp1::StartExperiment(ENTITY_LIST & bodies)
{
	LOG_INFO("start.");

	UpdateBadCoedgeSet(bodies);

	for (auto& bad_edge_it = bad_edge_set.begin(); bad_edge_it != bad_edge_set.end(); bad_edge_it++) {
		EDGE* iedge = (*bad_edge_it);
		COEDGE* icoedge = iedge->coedge();
		LOOP* iloop = icoedge->loop(); // 对坏边来说这个反正也只有一个吧

		TraverseLoops(iloop, true, icoedge);
	}

	LOG_INFO("end.");
}

void Exp1::Exp1::Init(ENTITY_LIST & bodies)
{
	api_initialize_constructors();
	api_initialize_booleans();

	// 实验1
	StartExperiment(bodies);

	api_terminate_constructors();
	api_terminate_booleans();
}
