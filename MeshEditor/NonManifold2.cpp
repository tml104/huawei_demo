#include "StdAfx.h"
#include "NonManifold2.h"

/*
	调用顺序：1
	寻找NonManifold的边，同时维护并查集
*/
void NonManifold::NonManifoldFixer::FindNonManifold()
{
	LOG_INFO("start.");

	// 1. 2. 找出所有非流形边，维护(vertex->edge)map
	for (int i = 0; i < bodies.count(); i++) {
		ENTITY* ibody = (bodies[i]);

		// 遍历这个body的edge list
		ENTITY_LIST edge_list;
		api_get_edges(ibody, edge_list);

		for (int j = 0; j < edge_list.count(); j++) {
			EDGE* iedge = dynamic_cast<EDGE*>(edge_list[j]);
			int coedge_count = Utils::CoedgeCount(iedge);

			// 找到非流形边，加入nonmanifold_edge_set中
			if (coedge_count > 2) {
				LOG_DEBUG("NonManifold edge found: iedge: %d, coedge_cnt: %d", MarkNum::GetId(iedge), coedge_count);
				nonmanifold_edge_set.insert(iedge);
			}

			// 维护(vertex->edge)map
			if (iedge->start() != nullptr) {
				vertex_to_edge_map[iedge->start()].insert(iedge);
			}

			if (iedge->end() != nullptr) {
				vertex_to_edge_map[iedge->end()].insert(iedge);
			}

			// 维护old_start_vertex_map, old_end_vertex_map： 也即维护原来所有边的初始开始顶点、结束顶点集合
			old_start_vertex_map[iedge] = iedge->start();
			old_end_vertex_map[iedge] = iedge->end();
		}
	}

	LOG_INFO("end.");
}

/* 
	调用顺序：2
	特判：如果有某个非流形满足环中有重复的非流形边的那个特征（xloop），加入special_nonmanifold_edge_set中 
*/
void NonManifold::NonManifoldFixer::SpecialCheckNonManifold()
{
	LOG_INFO("start.");

	// 此函数检查给定的某非流形边中是否存在一个loop，其loop上是有两个不同的coedge但是其对应的edge都是该给定的非流形边的
	auto check_nonmanifold_edge_in_xloop = [&](EDGE* const &iedge_nonmanifold) -> bool {

		// 遍历该非流形边中的coedge
		COEDGE* icoedge = iedge_nonmanifold->coedge();
		do {
			if (icoedge == nullptr) {
				break;
			}

			// 取得该coedge的loop，然后遍历其中的coedge，检查其edge与给定非流形边（iedge_nonmanifold）相同的数目
			LOOP* iloop = icoedge->loop();
			int identical_edge_nonmanifold_count = 0;
			COEDGE* jcoedge = iloop->start();
			do {
				if (jcoedge == nullptr) {
					break;
				}

				// 如果相同则贡献+1
				identical_edge_nonmanifold_count += (jcoedge->edge() == iedge_nonmanifold);

				jcoedge = jcoedge->next();
			} while (jcoedge != nullptr && jcoedge != iloop->start());

			// [debug] 打印一下identical_edge_nonmanifold_count
			//LOG_DEBUG("iedge_nonmanifold:%d, icoedge:%d, iloop:%d, identical_edge_nonmanifold_count: %d.",
			//	MarkNum::GetId(iedge_nonmanifold),
			//	MarkNum::GetId(icoedge),
			//	MarkNum::GetId(iloop),
			//	identical_edge_nonmanifold_count
			//);

			// 检查成功：添加此xloop边进入special_nonmanifold_edge_set集合中（之后根据朝向自然就可区分了）
			if (identical_edge_nonmanifold_count == 2) {
				special_nonmanifold_edge_map[iedge_nonmanifold] = iloop;
				return true;
			}
			else if (identical_edge_nonmanifold_count > 2)// 其他异常情况：这玩意大于2的话我不会处理
			{
				LOG_ERROR("identical_edge_nonmanifold_count > 2.");
				throw std::runtime_error("identical_edge_nonmanifold_count >2");
			}

			icoedge = icoedge->partner();
		} while (icoedge != nullptr && icoedge != iedge_nonmanifold->coedge());

		return false;
	};

	// 遍历上一步找到的所有非流形边，逐个检查是否满足xloop的条件
	for (auto iedge_it = nonmanifold_edge_set.begin(); iedge_it != nonmanifold_edge_set.end(); iedge_it++) {
		EDGE* iedge_nonmanifold = (*iedge_it);
		bool check_nonmanifold_edge_in_xloop_res = check_nonmanifold_edge_in_xloop(iedge_nonmanifold);
		LOG_DEBUG("iedge_nonmanifold: %d, check_nonmanifold_edge_in_xloop_res: %d", MarkNum::GetId(iedge_nonmanifold), check_nonmanifold_edge_in_xloop_res);
	}

	LOG_INFO("end.");
}

/*
	调用顺序：3
	专门搞special_nonmanifold_edge_set中的情况
*/
void NonManifold::NonManifoldFixer::SolveSpecialNonManifold()
{
	LOG_INFO("start.");

	// 遍历iedge_nonmanifold中的coedge（理论上有4个），不在xloop上的单独拿出来形成一个集合
	// 然后对这个集合中的iedge_nonmanifold的coedge，根据朝向和xloop上的做分类
	// 最后修改顶点的步骤是差不多的哦
	auto solve_special = [&](EDGE* const &iedge_nonmanifold, LOOP* const &xloop) {
		LOG_DEBUG("solve_special start: iedge_nonmanifold: %d, xloop: %d", MarkNum::GetId(iedge_nonmanifold), MarkNum::GetId(xloop));

		std::vector<COEDGE*> not_in_xloop_vec, in_xloop_vec;
		int coedge_cnt = 0;

		COEDGE* jcoedge = iedge_nonmanifold->coedge();
		//遍历iedge_nonmanifold中的coedge（理论上有4个），不在xloop上的单独拿出来形成一个集合
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

		// 配对的不在同一个环上的coedge集合。第一个是在xloop上的coedge，第二个是不在xloop上的coedge
		std::vector<std::pair<COEDGE*, COEDGE*>> pair_coedge_vec;

		// 先遍历在xloop上的coedge，然后逐个检查不在xloop上的coedge的方向是否与其一不致。不一致的配对为一组
		// （如果以后要考虑6个coedge同一边的情况的话这里得重新考虑，虽然我不觉得会有这么一天）
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

		// 至此为止理论上应该有两组了吧
		// 创建新边并暂存，数目等于组数（至此为止应该是2次）
		std::vector<EDGE*> new_edge_vec;

		for (unsigned int i = 0; i < pair_coedge_vec.size(); i++) {

			// 创建新边
			EDGE *new_edge = Utils::CopyEdge(iedge_nonmanifold);

			// 复制成功
			new_edge_vec.emplace_back(new_edge);
		}

		// 再次遍历每组已分类coedge对，设现在置新边并进行顶点替换等操作
		for (unsigned int i = 0; i < pair_coedge_vec.size(); i++) {

			COEDGE* coedge1 = pair_coedge_vec[i].first; // xloop上的非流形边的coedge
			COEDGE* coedge2 = pair_coedge_vec[i].second; // 不在xloop上的非流形边的coedge
			EDGE* new_edge = new_edge_vec[i];

			// 修改新边拓扑和设置partner
			new_edge->set_coedge(coedge1);
			coedge1->set_edge(new_edge);
			coedge2->set_edge(new_edge);

			coedge1->set_partner(coedge2);
			coedge2->set_partner(coedge1);

			// 在旧的边->顶点集合中维护新边的顶点
			old_start_vertex_map[new_edge] = iedge_nonmanifold->start();
			old_end_vertex_map[new_edge] = iedge_nonmanifold->end();

			// 修改顶点的情况搞到这里我才发现那个先拆点的做法才是比较正规的
			// 但这里先硬着头皮改一部分吧，修改xloop以及不在xloop的另一个coedge所在loop上的两个黄色边（具体见“新非流形”中的图），其他的loop都暂时不动
			// 但是理论上，其他loop如果还有接到这部分上的，也需要再用一个并查集把这部分两连的loops找出来，然后单独给它们创建新顶点，这样做才是对的（具体见“新非流形”中的有黄边的图的右侧那个蓝色四面体接到上面的情况）

			// 不在xloop上的非流形边的coedge 对应loop的顶点修改
			LOOP* loop_coedge2 = coedge2->loop();
			COEDGE* jcoedge = loop_coedge2->start();
			do {
				if (jcoedge == nullptr) {
					break;
				}
				// 跳过在xloop上的非流形边的coedge，也即coedge2本身（coedge2本身已经因为set_edge而设置好了）
				if (jcoedge == coedge2) {
					jcoedge = jcoedge->next();
					continue;
				}

				// 判断顶点并修改
				EDGE* jedge = jcoedge->edge();
				// [debug] 查看哪几个分支被调用了
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

			// 在xloop上的，这个就比较麻烦：改这个coedge的next的start以及prev的end？
			LOOP* xloop = coedge1->loop();
			//jcoedge = xloop->start(); // 啊啊啊！！！这就是问题所在
			jcoedge = coedge1;

			// [debug] 查看哪个分支被调用了
			int _jcoedge_next_set_start_vertex_flag = 0;
			int _jcoedge_prev_set_end_vertex_flag = 0;

			// 修改next的start
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
			else { // 反转修改next的end
				if (old_end_vertex_map[jcoedge->next()->edge()] == iedge_nonmanifold->start()) {
					jcoedge->next()->edge()->set_end(new_edge->start());
					_jcoedge_next_set_start_vertex_flag = 3;
				}
				else if (old_end_vertex_map[jcoedge->next()->edge()] == iedge_nonmanifold->end()) {
					jcoedge->next()->edge()->set_end(new_edge->end());
					_jcoedge_next_set_start_vertex_flag = 4;
				}
			}

			// 修改prev的end
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
			else { // 反转修改prev的start
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


// [Deprecated] 此函数已经集成在第二版SolveNonManifold2函数中
void NonManifold::NonManifoldFixer::MaintainFindUnionSet()
{
	LOG_INFO("start.");

	// 3. 维护并查集
	// 这个函数通过输入某个非流形边的顶点相邻的边集合，遍历这个集合中的正常边edge，然后将对应edge的两侧的loop进行一个unite
	auto traverse_vertex_incident_edge_set = [&](std::set<EDGE*>& vertex_incident_edge_set) {
		LOG_DEBUG("start.");

		for (auto it2 = vertex_incident_edge_set.begin(); it2 != vertex_incident_edge_set.end(); it2++) { // *it2: 非流形边顶点相邻边集合中的一个边
			int coedge_cnt = Utils::CoedgeCount(*it2);

			// 排除掉(*it2)本身就是nonmanifold_edge的情况
			if (nonmanifold_edge_set.count(*it2)) {
				LOG_DEBUG(
					"Maintain LoopFindUnionSet, nonmanifold_edge_set.count(*it2)>0: *it2:%d, coedge_cnt:%d",
					MarkNum::GetId(*it2),
					coedge_cnt
				);
				continue;
			}

			// 排除掉异常边（coedge数量不符合流形边）的情况（不过事实上理论上上一步已经排除得差不多了）
			if (coedge_cnt != 2) {
				LOG_DEBUG("Maintain LoopFindUnionSet, coedge_cnt !=2: *it2:%d, coedge_cnt:%d", MarkNum::GetId(*it2), coedge_cnt);

				continue;
			}

			LOG_DEBUG("Maintain LoopFindUnionSet, check passed : *it2:%d, coedge_cnt: %d", MarkNum::GetId(*it2),
				coedge_cnt);

			// 检查通过后，接下来遍历流形边的coedge，然后将这些coedge所在loop全部加入icoedge_loop_vec中，稍后将这些loop全部加入同一个并查集
			std::vector<LOOP*> icoedge_loop_vec;

			auto icoedge = (*it2)->coedge();
			do {
				if (icoedge == nullptr) {
					break;
				}

				// 将所属loop加入icoedge_loop_vec中，稍后使用并查集unite
				icoedge_loop_vec.emplace_back(icoedge->loop());

				icoedge = icoedge->partner();
			} while (icoedge != nullptr && icoedge != (*it2)->coedge());

			// TODO: 悬挂边的情况……
			LOG_INFO("icoedge_loop_vec size: %d", icoedge_loop_vec.size());

			// 将流形边中相邻loop全部unite起来
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

	// 遍历非流形边集合nonmanifold_edge_set，然后将这些边的begin、end的vertex对应的相邻边集合输入traverse_vertex_incident_edge_set中，得到loop并查集
	for (auto it = nonmanifold_edge_set.begin(); it != nonmanifold_edge_set.end(); it++) {
		auto& start_vertex_incident_edge_set = vertex_to_edge_map[(*it)->start()];
		traverse_vertex_incident_edge_set(start_vertex_incident_edge_set);

		auto& end_vertex_incident_edge_set = vertex_to_edge_map[(*it)->end()];
		traverse_vertex_incident_edge_set(end_vertex_incident_edge_set);
	}

	LOG_INFO("end.");
}


// 目前这个算法只能解决所有待分解的面全挤在一条非流形边的情况，如果要是还有别的点吸附在非流形边的顶点上的情况（这算奇异情况吧），为每个非流形点加点之后再去创建边的方式是需要再考虑的。
// 但是由于目前没有例子所以暂时先不管了吧 —— 2024年3月5日 14:42:28
// [Deprecated] 由于有第二版实现因此这个函数暂时废弃
void NonManifold::NonManifoldFixer::SolveNonManifold()
{
	LOG_INFO("start.");

	// 4. 遍历每个非流形边，将其coedge分类
	// TODO：（这里写的有点问题，coedge分类应该还是要带有非流形边是谁的信息，否则这里直接判断nonmanifold_coedge_map中的vector中coedge数量等于2不能得知是不是同一个非流形边引起的，以目前的代码的判断会导致本来能合并的情况不能合并）
	//（因为目前这个代码假设每个group里面只有两个分类coedge，但是由于这个代码是先整体做并查集合并后再分别（也即不是根据一条边一组并查集来）做coedge分类的因此有可能会出现一个并查集里面有4个以上coedge的情况？这代码逻辑要改啊）
	//（也就是说这里要改成 输入某特定非流形边->生成关于此非流形边的并查集->根据此并查集做分类 这个模式？）
	auto group_map = loop_findunionset.get_group_map(); // 这个get_group_map返回(group father->group set)的映射
	std::map<LOOP*, std::vector<COEDGE*>> nonmanifold_coedge_map; //(group loop father->非流形边coedge集合)的map，理论上每个group应该只有两个

	//[debug] 检查group_map：遍历一下group map
	LOG_DEBUG("Checking group_map start.");
	for (auto it = group_map.begin(); it != group_map.end(); it++) {
		LOG_DEBUG("group map info: group father: %d, size: %d", MarkNum::GetId(it->first), it->second.size());
	}
	LOG_DEBUG("Checking group_map end.");


	// 遍历每个非流形边，将其coedge分类
	for (auto it = nonmanifold_edge_set.begin(); it != nonmanifold_edge_set.end(); it++) {
		auto &iedge_nonmanifold = (*it);

		LOG_DEBUG("Solving iedge_nonmanifold: %d", MarkNum::GetId(iedge_nonmanifold));

		// 遍历coedge（一般是会有4个），做分类工作
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
			auto iloop_group_father = loop_findunionset.get_father(iloop); //取得coedge的loop是在哪个group里
			nonmanifold_coedge_map[iloop_group_father].emplace_back(icoedge); //根据group放入coedge

			icoedge = icoedge->partner();
		} while (icoedge != nullptr && icoedge != iedge_nonmanifold->coedge());

		// 5.  分类完成，创建新边并修改指针
		// 现在分成两步： 先复制对应数量的edge指针并保存，之后修改的时候再从里面取出来

		std::vector<EDGE*> new_edge_vec;

		// 5.1 遍历上一步中涉及到的相邻的group，然后为每个group创建新的边并保存在new_edge_vec中
		// nonmanifold_coedge_map: (group loop father->非流形边coedge集合)的map，理论上每个group应该只有两个
		// 此处，nonmanifold_coedge_map的大小应该与group数量一致（也即该循环的执行次数）
		for (auto it2 = nonmanifold_coedge_map.begin(); it2 != nonmanifold_coedge_map.end(); it2++) {
			auto & this_group_father = it2->first;
			auto &nonmanifold_coedge_vec = it2->second;

			LOG_DEBUG("5.1: Solving group (father): %d", MarkNum::GetId(this_group_father));

			// 检查是不是只有两个对应coedge
			if (nonmanifold_coedge_vec.size() != 2) {
				LOG_ERROR("5.1: nonmanifold_coedge_set.size() != 2, continue will create nonmanifold edge!");
				continue;
			}

			// 检查通过，默认这个group对应只有两个coedge（这块如果涉及到那个圆筒切一片体的特判的话……）


			// 创建新边
			EDGE* new_edge = Utils::CopyEdge(iedge_nonmanifold);

			// 复制成功，把它放入new_edge_vec中以供下一步使用
			LOG_DEBUG("Copy succeed. old: %d, new_edge_address: %d", MarkNum::GetId(iedge_nonmanifold), new_edge);

			new_edge_vec.emplace_back(new_edge);
		}

		// 5.2 再次遍历上一步中涉及到的相邻的group，然后为这个group中的loop进行设置新边，边进行替换顶点等操作
		int new_edge_pick_index = 0;
		for (auto it2 = nonmanifold_coedge_map.begin(); it2 != nonmanifold_coedge_map.end(); it2++) {
			auto & this_group_father = it2->first;
			auto & nonmanifold_coedge_vec = it2->second;

			// 检查是不是只有两个对应coedge
			if (nonmanifold_coedge_vec.size() != 2) {
				LOG_ERROR("nonmanifold_coedge_set.size() != 2, continue will create nonmanifold edge!");
				continue;
			}

			// 检查通过，默认这个group对应只有两个coedge（这块如果涉及到那个圆筒切一片体的特判的话……）
			auto &coedge1 = nonmanifold_coedge_vec[0];
			auto &coedge2 = nonmanifold_coedge_vec[1];

			// 从new_edge vector中拿出边，代替“创建新边”这个操作（注意顺序要和上面创建的时候严格一致）
			EDGE* new_edge = new_edge_vec[new_edge_pick_index++];

			// 修改新边拓扑
			new_edge->set_coedge(coedge1); // 随便定的
			coedge1->set_edge(new_edge);
			coedge2->set_edge(new_edge);

			// 设置partner
			coedge1->set_partner(coedge2);
			coedge2->set_partner(coedge1);

			LOG_DEBUG(
				"Coedge partner set:%d %d",
				MarkNum::GetId(coedge1->partner()),
				MarkNum::GetId(coedge2->partner())
			);

			// 在旧的边->顶点集合中维护新边的顶点
			old_start_vertex_map[new_edge] = iedge_nonmanifold->start();
			old_end_vertex_map[new_edge] = iedge_nonmanifold->end();

			// 修改顶点：遍历group底下所有loop，修改边的顶点
			auto &this_part_group_loops = group_map[this_group_father];

			for (auto it3 = this_part_group_loops.begin(); it3 != this_part_group_loops.end(); it3++) {
				auto jloop = (*it3);
				auto jcoedge = jloop->start();

				LOG_DEBUG("Solving jloop: %d", MarkNum::GetId(jloop));

				//遍历loop中的coedge
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

					// 跳过非流形边（非流形边上的coedge的edge会被直接设置，因此不用修改顶点）
					if (nonmanifold_edge_set.count(jedge)) {
						LOG_DEBUG("Modify vertex SKIP nonmanifold_edge: jedge: %d", MarkNum::GetId(jedge));

						jcoedge = jcoedge->next();

						if (!(jcoedge != nullptr && jcoedge != jloop->start())) {
							break;
						}
						continue;
					}

					// 之前这里只判断了当前edge起点与当前非流形边起点相同的情况……
					// 现在应该改成4种组合吧

					// [debug] 查看哪几个分支被调用了
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

					// [debug] 查看哪几个分支被调用了
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

		// 清空用于保存(group loop father->非流形边coedge集合)的map，理论上每个group应该只有两个
		nonmanifold_coedge_map.clear();
	}

	LOG_INFO("end.");
}

/*
	第二版SolveNonManifold实现，将原MaintainFindUnionSet、SolveNonManifold合并，其将以每个非流形边为点位来处理问题
*/
void NonManifold::NonManifoldFixer::SolveNonManifold2()
{
	LOG_INFO("start.");

	// 下方是原MaintainFindUnionSet的部分

	// 3. 维护并查集
	// 这个函数通过输入某个非流形边的顶点相邻的边集合，遍历这个集合中的正常边edge，然后将对应edge的两侧的loop进行一个unite
	auto traverse_vertex_incident_edge_set = [&](std::set<EDGE*>& vertex_incident_edge_set) {
		LOG_INFO("traverse_vertex_incident_edge_set start.");

		for (auto it2 = vertex_incident_edge_set.begin(); it2 != vertex_incident_edge_set.end(); it2++) { // *it2: 非流形边顶点相邻边集合中的一个边
			int coedge_cnt = Utils::CoedgeCount(*it2);

			// 排除掉(*it2)本身就是nonmanifold_edge的情况
			if (nonmanifold_edge_set.count(*it2)) {
				LOG_DEBUG(
					"Maintain LoopFindUnionSet, nonmanifold_edge_set.count(*it2)>0: *it2: %d, coedge_cnt:%d",
					MarkNum::GetId(*it2),
					coedge_cnt
				);
				continue;
			}

			// 排除掉异常边（coedge数量不符合流形边）的情况（不过事实上理论上上一步已经排除得差不多了）
			if (coedge_cnt != 2) {
				LOG_DEBUG(
					"Maintain LoopFindUnionSet, coedge_cnt !=2: *it2: %d, coedge_cnt:%d",
					MarkNum::GetId(*it2),
					coedge_cnt
				);

				continue;
			}

			// 检查通过
			LOG_DEBUG(
				"Maintain LoopFindUnionSet, check passed : *it2: %d, coedge_cnt: %d",
				MarkNum::GetId(*it2),
				coedge_cnt
			);

			// 检查通过后，接下来遍历边的coedge，然后将这些coedge所在loop全部加入icoedge_loop_vec中，稍后将这些loop全部加入同一个并查集
			std::vector<LOOP*> icoedge_loop_vec;

			auto icoedge = (*it2)->coedge();
			do {
				if (icoedge == nullptr) {
					break;
				}

				// 将所属loop加入icoedge_loop_vec中，稍后使用并查集unite
				icoedge_loop_vec.emplace_back(icoedge->loop());

				icoedge = icoedge->partner();
			} while (icoedge != nullptr && icoedge != (*it2)->coedge());

			// TODO: 悬挂边的情况……
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

		// 4. 遍历每个非流形边，将其coedge分类
		// TODO:（不知道还需不需要在coedge分类步骤中维护非流形边是谁这个信息，有待事后调查）（不过目前可以确定的是对于那种有奇异点连着的情况这个代码是不能消除的）
		auto group_map = loop_findunionset.get_group_map(); // 这个get_group_map返回(group father->group set)的映射
		std::map<LOOP*, std::vector<COEDGE*>> nonmanifold_coedge_map; //(group loop father->非流形边coedge集合)的map，理论上每个group应该只有两个

		//[debug] 检查group_map：遍历一下group map
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

		// 对于当前非流形边iedge_nonmanifold，将其coedge分类
		COEDGE* icoedge = iedge_nonmanifold->coedge();
		do {
			if (icoedge == nullptr) {
				break;
			}

			LOOP* iloop = icoedge->loop();
			LOOP* iloop_group_father = loop_findunionset.get_father(iloop); //取得coedge的loop是在哪个group里
			nonmanifold_coedge_map[iloop_group_father].emplace_back(icoedge); //根据group放入coedge

			icoedge = icoedge->partner();
		} while (icoedge != nullptr && icoedge != iedge_nonmanifold->coedge());

		// 5.  分类完成，创建新边并修改指针
		// 现在分成两步： 先复制对应数量的edge指针并保存，之后修改的时候再从里面取出来

		std::vector<EDGE*> new_edge_vec;
		// 5.1 遍历上一步中涉及到的相邻的group，然后为每个group创建新的边并保存在new_edge_vec中
		// nonmanifold_coedge_map: (group loop father->非流形边coedge集合)的map，理论上每个group应该只有两个
		// 此处，nonmanifold_coedge_map的大小应该与group数量一致（也即该循环的执行次数）
		for (auto it2 = nonmanifold_coedge_map.begin(); it2 != nonmanifold_coedge_map.end(); it2++) {
			auto& this_group_father = it2->first;
			auto& nonmanifold_coedge_vec = it2->second;
			LOG_DEBUG("5.1: Solving group (father): %d", MarkNum::GetId(this_group_father));

			// 检查是不是只有两个对应coedge
			if (nonmanifold_coedge_vec.size() != 2) {
				LOG_ERROR("5.1: nonmanifold_coedge_set.size() != 2, continue will create nonmanifold edge!");
				continue;
			}

			// 检查通过，默认这个group对应只有两个coedge
			// 创建新边，并预存在new_edge_vec中
			EDGE* new_edge = Utils::CopyEdge(iedge_nonmanifold);
			new_edge_vec.emplace_back(new_edge);
		}

		//5.2 再次遍历上一步中涉及到的相邻的group，然后为这个group中的loop进行设置新边，边进行替换顶点等操作
		// 注意bug！：此处如果以后需要重构的话请务必小心，这个循环和上个循环一定是分开的而非嵌套关系！
		int new_edge_pick_index = 0;
		for (auto it2 = nonmanifold_coedge_map.begin(); it2 != nonmanifold_coedge_map.end(); it2++) {
			auto & this_group_father = it2->first;
			auto & nonmanifold_coedge_vec = it2->second;

			// 检查是不是只有两个对应coedge
			if (nonmanifold_coedge_vec.size() != 2) {
				LOG_ERROR("5.2: nonmanifold_coedge_set.size() != 2, continue will create nonmanifold edge!");
				continue;
			}

			// 检查通过，默认这个group对应只有两个coedge（这块如果涉及到那个圆筒切一片体的特判的话……）
			auto &coedge1 = nonmanifold_coedge_vec[0];
			auto &coedge2 = nonmanifold_coedge_vec[1];

			// 从new_edge vector中拿出边，代替“创建新边”这个操作（注意顺序要和上面创建的时候严格一致）
			EDGE* new_edge = new_edge_vec[new_edge_pick_index++];

			// 修改新边拓扑
			new_edge->set_coedge(coedge1); // 随便定的
			coedge1->set_edge(new_edge);
			coedge2->set_edge(new_edge);

			// 设置partner
			coedge1->set_partner(coedge2);
			coedge2->set_partner(coedge1);

			LOG_DEBUG(
				"5.2: Coedge partner set: %d %d",
				MarkNum::GetId(coedge1->partner()),
				MarkNum::GetId(coedge2->partner())
			);

			// 在旧的边->顶点集合中维护新边的顶点
			old_start_vertex_map[new_edge] = iedge_nonmanifold->start();
			old_end_vertex_map[new_edge] = iedge_nonmanifold->end();

			// 修改顶点：遍历group底下所有loop，修改边的顶点
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

					// 跳过非流形边（非流形边上的coedge的edge会被直接设置，因此不用修改顶点）
					if (nonmanifold_edge_set.count(jedge)) {
						LOG_DEBUG("Modify vertex SKIP nonmanifold_edge: jedge: %d", MarkNum::GetId(jedge));

						jcoedge = jcoedge->next();

						if (!(jcoedge != nullptr && jcoedge != jloop->start())) {
							break;
						}
						continue;
					}

					// 之前这里只判断了当前edge起点与当前非流形边起点相同的情况……
					// 现在应该改成4种组合吧

					// [debug] 查看哪几个分支被调用了
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

					// [debug] 查看哪几个分支被调用了
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

	// 遍历非流形边集合nonmanifold_edge_set，然后将这些边的begin、end的vertex对应的相邻边集合输入traverse_vertex_incident_edge_set中，得到loop并查集
	for (auto it = nonmanifold_edge_set.begin(); it != nonmanifold_edge_set.end(); it++) {

		// 跳过：如果it（当前迭代的非流形edge）在special_nonmanifold_edge_map中则跳过
		if (special_nonmanifold_edge_map.count(*it)) {
			LOG_DEBUG("Skip special_nonmanifold_edge: edge: %d", MarkNum::GetId(*it));
			continue;
		}

		auto& start_vertex_incident_edge_set = vertex_to_edge_map[(*it)->start()];
		traverse_vertex_incident_edge_set(start_vertex_incident_edge_set);

		auto& end_vertex_incident_edge_set = vertex_to_edge_map[(*it)->end()];
		traverse_vertex_incident_edge_set(end_vertex_incident_edge_set);

		// 下方是缝合原SolveNonManifold的部分
		solve_single_nonmanifold_edge((*it));

		// 新：清除并查集
		loop_findunionset.clear();
	}

	LOG_INFO("end.");
}

void NonManifold::NonManifoldFixer::Start()
{
	// 计时开始
	clock_t NonManifold_start_clock = std::clock();

	api_initialize_constructors();
	api_initialize_booleans();

	NonManifold::Debug::PrintLoopsInfo(bodies);

	// （这两个函数影响较小，先调用）
	FindNonManifold();
	SpecialCheckNonManifold();

	clock_t NonManifold_end_clock_find = std::clock(); // 计时结束 find

	SolveSpecialNonManifold(); // 单独求解特殊情况
	SolveNonManifold2(); // 接下来调用第二版统一实现

	clock_t NonManifold_end_clock_solve = std::clock(); // 计时结束 solve

	// 打印时间
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
	并查集的get father
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

	// 路径压缩
	return father_map[loop1] = get_father(it->second);
}

/*
	并查集的 unite 操作
	输入：两个loop（可以不是对应group的father）
*/
void NonManifold::LoopFindUnionSet::unite(LOOP * loop1, LOOP * loop2)
{
	auto loop1_father = get_father(loop1);
	auto loop2_father = get_father(loop2);

	if (loop1_father == loop2_father) {
		return;
	}

	father_map[loop1_father] = loop2_father; //未路径压缩
}

/*
	取得 (group father -> group（用set保存一系列同一group下的loop）)的map
*/
std::map<LOOP*, std::set<LOOP*>> NonManifold::LoopFindUnionSet::get_group_map()
{
	std::map<LOOP*, std::set<LOOP*>> group;

	for (auto it = father_map.begin(); it != father_map.end(); it++) {
		group[get_father(it->first)].insert(it->first); // 确保路径压缩
	}

	return group;
}

/*
	清空
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

		// 取得body内的所有loop
		ENTITY_LIST loops_list;
		api_get_loops(ibody, loops_list);

		for (int j = 0; j < loops_list.count(); j++) {
			LOOP* jloop = dynamic_cast<LOOP*>(loops_list[j]);

			// 遍历此loop并输出其coedge、edge编号
			COEDGE* jcoedge = jloop->start();
			do {
				if (jcoedge == nullptr) {
					break;
				}

				EDGE* jedge = jcoedge->edge();

				// 输出
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
