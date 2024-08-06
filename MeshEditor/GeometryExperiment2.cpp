#include "StdAfx.h"
#include "GeometryExperiment2.h"

std::vector<LOOP*> GeometryExperiment2::Singleton::bad_loop_vec;

void GeometryExperiment2::GeometryExperiment2(ENTITY_LIST & bodies)
{
	LOG_INFO("start.");

	auto get_sorted_vertex_pair = [&](EDGE* e) -> std::pair<VERTEX*, VERTEX*>
	{
		VERTEX* st = e->start();
		VERTEX* ed = e->end();
		if (MarkNum::GetId(st) > MarkNum::GetId(ed))
		{
			std::swap(st, ed);
		}

		return std::make_pair(st, ed);
	};

	// 检查函数：输入loop，
	auto check_has_geometry_coincident = [&](LOOP* lp) -> bool
	{
		//std::set<CURVE*> gset;
		std::set<std::pair<VERTEX*, VERTEX*>> vset;

		COEDGE* icoedge = lp->start();
		do {
			if (icoedge == nullptr) {
				LOG_ERROR("icoedge == nullptr");
				return false;
			}

			EDGE* iedge = icoedge->edge();
			CURVE* g = iedge->geometry();
			if (g != nullptr)
			{
				auto vp = get_sorted_vertex_pair(iedge);
				if (vset.count(vp))
				{
					LOG_DEBUG("geometry coincident Yes: icoedge: %d, edge: %d, loop: %d", MarkNum::GetId(icoedge), MarkNum::GetId(icoedge->edge()), MarkNum::GetId(lp));
					return true;
				}
				vset.insert(vp);
			}
			else {
				LOG_ERROR("no geometry: icoedge: %d, edge: %d, loop: %d", MarkNum::GetId(icoedge), MarkNum::GetId(icoedge->edge()), MarkNum::GetId(lp));
			}

			icoedge = icoedge->next();
		} while (icoedge != nullptr && icoedge != lp->start());

		return false;

	};

	for (int i = 0; i < bodies.count(); i++)
	{
		ENTITY_LIST loop_list;

		BODY* ibody = dynamic_cast<BODY*>(bodies[i]); // current body; （当然对于 C_ent(0)_body_0 来说只有一个body）
		api_get_loops(ibody, loop_list);

		for (int j = 0; j < loop_list.count(); j++)
		{
			LOOP* iloop = dynamic_cast<LOOP*>(loop_list[j]);
			if (check_has_geometry_coincident(iloop))
			{
				GeometryExperiment2::Singleton::bad_loop_vec.emplace_back(iloop);
			}
		}
	}

	LOG_INFO("end.");
}


/*
	打印bad_loop_vec中的内容
*/
void GeometryExperiment2::PrintBadLoopVec()
{
	LOG_INFO("start.");

	int cnt = GeometryExperiment2::Singleton::bad_loop_vec.size();
	LOG_DEBUG("bad loop count: %d", cnt);
	for (int i = 0; i < cnt; i++)
	{
		LOOP* lp = GeometryExperiment2::Singleton::bad_loop_vec[i];
		LOG_DEBUG("bad loop: %d, length: %d", MarkNum::GetId(lp), Utils::LoopLength(lp));

		COEDGE* icoedge = lp->start();
		do {
			if (icoedge == nullptr) {
				LOG_ERROR("icoedge == nullptr");
				return;
			}

			EDGE* iedge = icoedge->edge();

			LOG_DEBUG("icoedge in bad loop: %d, edge in bad loop: %d", MarkNum::GetId(icoedge), MarkNum::GetId(iedge));
			GeometryUtils::PrintEdgeGeometry(iedge);

			icoedge = icoedge->next();
		} while (icoedge && icoedge != lp->start());

	}

	LOG_INFO("end.");
}

#ifdef USE_HOOPSVIEW

void GeometryExperiment2::ShowBadLoopEdgeMark(HoopsView * hv)
{
	std::set<int> show_edge_marknum_set;
	for (int i = 0; i < GeometryExperiment2::Singleton::bad_loop_vec.size(); i++)
	{
		//show_edge_marknum_set.insert(MarkNum::GetId(GeometryExperiment2::Singleton::bad_loop_vec[i]));
		LOOP* lp = GeometryExperiment2::Singleton::bad_loop_vec[i];
		int lp_length = Utils::LoopLength(lp);
		if (lp_length == 2)
		{
			continue;
		}

		COEDGE* icoedge = lp->start();
		do {
			if (icoedge == nullptr) {
				LOG_ERROR("icoedge == nullptr");
				return;
			}

			EDGE* iedge = icoedge->edge();

			show_edge_marknum_set.insert(MarkNum::GetId(iedge));

			icoedge = icoedge->next();
		} while (icoedge && icoedge != lp->start());
	}

	//[debug] print show_edge_marknum_set
	LOG_DEBUG("show_edge_marknum_set size: %d", show_edge_marknum_set.size());

	for (auto&& it = show_edge_marknum_set.begin(); it != show_edge_marknum_set.end(); it++)
	{
		LOG_DEBUG("Marknum in show_edge_marknum_set: %d", *(it));
	}

	MarkNum::ShowEdgeMark(hv, show_edge_marknum_set);
}

#endif


void GeometryExperiment2::Init(ENTITY_LIST & bodies)
{
	LOG_INFO("start.");

	GeometryExperiment2(bodies);
	PrintBadLoopVec();

	LOG_INFO("end.");
}
