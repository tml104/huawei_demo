#include "StdAfx.h"
#include "mesh_min_cut.h"
#include "max_flow_min_cut.h"
//#include "max_flow_min_cut_copy.h"
#include <QWidget>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QTabWidget>
#include <queue>
#include "ui_topologyoptwidget.h"
#include "hoopsview.h"
#include "meshrendercontrolwidget.h"
#include "mousecontrolwidget.h"
#include "filecontrolwidget.h"
#include "groupcontrolwidget.h"
#include "OneSimpleSheetInflation.h"
#include "OneSimpleSheetExtraction.h"
#include "MeshDefs.h"
#include "FuncDefs.h"
#include "DualDefs.h"
#include "DualOperations.h"
#include "GeometryFuncs.h"
#include "TopologyQualityPredict.h"
#include "PrioritySetManager.h"

double alpha = 100;

void rate_inner_vhs (VolumeMesh *mesh, std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs, 
	std::vector<std::unordered_set<OvmVeH> > &layered_vhs)
{

	if (!mesh->vertex_property_exists<int> ("node level")){
		auto tmp = mesh->request_vertex_property<int> ("node level", 0);
		mesh->set_persistent (tmp);
	}
	if (!mesh->mesh_property_exists<int> ("max node level")){
		auto tmp = mesh->request_mesh_property<int> ("max node level", 1);
		mesh->set_persistent (tmp);
	}
	auto V_LEVEL = mesh->request_vertex_property<int> ("node level");
	auto V_MAX_LEVEL = mesh->request_mesh_property<int> ("max node level");
	std::unordered_set<OvmVeH> s_bnd_vhs, t_bnd_vhs;
	std::unordered_set<OvmFaH> tmp, s_bnd_fhs, t_bnd_fhs;
	collect_boundary_element (mesh, s_chs, NULL, NULL, &tmp);

	foreach (auto &fh, tmp){
		if (!mesh->is_boundary (fh))
			s_bnd_fhs.insert (fh);
	}
	foreach (auto &fh,tmp)
	{
		for (auto hfv_it = mesh->hfv_iter(mesh->halfface_handle(fh,0));hfv_it;hfv_it++)
		{
			t_bnd_vhs.insert(*hfv_it);
		}
	}
	tmp.clear ();
	collect_boundary_element (mesh, t_chs, NULL, NULL, &tmp);
	foreach (auto &fh, tmp){
		if (!mesh->is_boundary (fh))
			t_bnd_fhs.insert (fh);
	}
	foreach (auto &fh,tmp)
	{
		for (auto hfv_it = mesh->hfv_iter(mesh->halfface_handle(fh,0));hfv_it;hfv_it++)
		{
			t_bnd_vhs.insert(*hfv_it);
		}
	}

	std::unordered_set<OvmVeH> vhs_visited;
	std::queue<OvmVeH> q;
	auto fGetFront = [&] (std::unordered_set<OvmCeH> &chs, std::unordered_set<OvmVeH> &bnd_vhs){
		foreach (auto &ch, chs){
			for (auto cv_it = mesh->cv_iter(ch);cv_it;cv_it++)
			{
				vhs_visited.insert(*cv_it);
				V_LEVEL[*cv_it] = 0;
				if (bnd_vhs.find(*cv_it) != bnd_vhs.end(*cv_it))
				{
					q.push(*cv_it);
				}
			}
		}
	};
	fGetFront (s_chs, s_bnd_vhs);
	fGetFront (t_chs, t_bnd_vhs);

	int layer_idx = 0;
	clock_t s4 = clock();

	OvmVeH last = q.back();
	OvmVeH q_top;
	while (!q.empty()) 
	{
		q_top = q.front();
		q.pop();
		if (q_top == last && !q.empty())
		{
			layer_idx++;
			last = q.back();
		}
		for (auto v_it = mesh->voh_iter(q_top);v_it;v_it++)
		{
			OvmVeH target_vh = mesh->halfedge (*v_it).to_vertex ();
			if (vhs_visited.find(target_vh) == vhs_visited.end())
			{
				vhs_visited.insert(target_vh);
				q.push(target_vh);
				V_LEVEL[target_vh] = layer_idx;
			}
		}
	}

	V_MAX_LEVEL[0] = layer_idx + 1;
}

std::unordered_set<OvmFaH> get_volume_mesh_min_cut (VolumeMesh *mesh, std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs)
{
	std::cout<<"Enter function get"<<std::endl;
	std::vector<std::unordered_set<OvmVeH> > layered_vhs;
	rate_inner_vhs (mesh, s_chs, t_chs, layered_vhs);

	auto V_LEVEL = mesh->request_vertex_property<int> ("node level");
	auto V_MAX_LEVEL = mesh->request_mesh_property<int> ("max node level");
	const int INFINITY_LEVEL = std::numeric_limits<int>::max () / 1000;
	const int MAX_LEVEL = V_MAX_LEVEL[0];

	//建立网格面句柄和无向图中节点序号之间的映射关系
	std::hash_map<OvmCeH, int> c_n_mapping;
	std::hash_map<int, OvmCeH> n_c_mapping;
	std::unordered_set<OvmCeH> normal_chs;
	std::cout<<"Step 0"<<std::endl;
	for (auto c_it = mesh->cells_begin (); c_it != mesh->cells_end (); ++c_it){
		auto ch = *c_it;
		if (!contains (s_chs, ch) && !contains (t_chs, ch)){
			normal_chs.insert (ch);
			int cur_idx = normal_chs.size ();
			c_n_mapping.insert (std::make_pair(ch, cur_idx));
			n_c_mapping.insert (std::make_pair(cur_idx, ch));
		}
	}
	std::cout<<"Before construct graph"<<std::endl;
	//构建有向图
	int N_node = normal_chs.size () + 2;
	int S_idx = 0, T_idx = N_node - 1;
	std::cout<<"Nnode "<<N_node<<std::endl;
	CoreSolver<int> graph (N_node, S_idx, T_idx);
	std::cout<<"AFter construct graph"<<std::endl;
	auto fRateFh = [&] (OvmFaH fh) -> int {
		auto adj_vhs = get_adj_vertices_around_face (mesh, fh);
		int all_vh_level = 0;
		foreach (auto &adj_vh, adj_vhs){
			int vh_level = V_LEVEL[adj_vh];
			all_vh_level += MAX_LEVEL - vh_level;
		}
		return (all_vh_level) / adj_vhs.size () * 4;
		//return 1;
	};

	auto fInsertGraphEdges = [&] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2,
		int idx1, int idx2){
			int all_level = 0;
			if (fhs1.size () < fhs2.size ()){
				foreach (auto &fh1, fhs1){
					if (contains (fhs2, fh1)){
						auto rate_level = fRateFh (fh1);
						all_level += rate_level;
					}
				}
			}else{
				foreach (auto &fh2, fhs2){
					if (contains (fhs1, fh2)){
						auto rate_level = fRateFh (fh2);
						all_level += rate_level;
					}
				}
			}

			if (all_level > 0){
				graph.insert_edge (idx1, idx2, all_level);
			}
	};

	std::cout<<"Step 1"<<std::endl;
	//连接s和t
	clock_t s1 = clock();
	std::unordered_set<OvmFaH> s_bnd_fhs, t_bnd_fhs;
	collect_boundary_element (mesh, s_chs, NULL, NULL, &s_bnd_fhs);
	collect_boundary_element (mesh, t_chs, NULL, NULL, &t_bnd_fhs);
	fInsertGraphEdges (s_bnd_fhs, t_bnd_fhs, S_idx, T_idx);
	clock_t e1 = clock();
	double tcost1 = (double)(e1-s1)/CLOCKS_PER_SEC;
	std::cout<<"Time cost1 is "<<tcost1<<std::endl;
	//graph.test_fun();
	std::cout<<"Step2 "<<std::endl;
	foreach (auto &nor_ch, normal_chs){
		std::unordered_set<OvmCeH> adj_chs;
		get_adj_hexas_around_hexa (mesh, nor_ch, adj_chs);
		std::unordered_set<OvmFaH> adj_fhs;
		get_adj_faces_around_hexa (mesh, nor_ch, adj_fhs);

		int nor_ch_idx = c_n_mapping[nor_ch];
		bool adj_to_s = false, adj_to_t = false;
		std::unordered_set<OvmCeH> adj_nor_chs;
		foreach (auto &adj_ch, adj_chs){
			if (contains (s_chs, adj_ch))
				adj_to_s = true;
			else if (contains (t_chs, adj_ch))                                                                                  
				adj_to_t = true;
			else
				adj_nor_chs.insert (adj_ch);
		}
		if (adj_to_s)
			fInsertGraphEdges (s_bnd_fhs, adj_fhs, S_idx, nor_ch_idx);
		if (adj_to_t)
			fInsertGraphEdges (adj_fhs, t_bnd_fhs, nor_ch_idx, T_idx);
		foreach (auto &adj_nor_ch, adj_nor_chs){
			int adj_nor_ch_idx = c_n_mapping[adj_nor_ch];
			std::unordered_set<OvmFaH> adj_nor_ch_adj_fhs;
			get_adj_faces_around_hexa (mesh, adj_nor_ch, adj_nor_ch_adj_fhs);
			fInsertGraphEdges (adj_fhs, adj_nor_ch_adj_fhs, nor_ch_idx, adj_nor_ch_idx);
		}
	}

	std::cout<<"Step 3"<<std::endl;
	QString max_flow_str = QString ("Max flow is %1").arg (graph.ford_fulkerson());
	auto min_cut = graph.mincut ();

	//给定一个图中的节点序号，求得它的外围面，要分四种情况
	auto fGetTmpBndFhs = [&] (int tmp_idx) -> std::unordered_set<OvmFaH>{
		std::unordered_set<OvmFaH> bnd_fhs;
		if (tmp_idx == S_idx)
			bnd_fhs = s_bnd_fhs;
		else if (tmp_idx == T_idx)
			bnd_fhs = t_bnd_fhs;
		else{
			auto nor_ch = n_c_mapping[tmp_idx];
			get_adj_faces_around_hexa (mesh, nor_ch, bnd_fhs);
		}
		return bnd_fhs;
	};

	auto fGetSharedFhs = [] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2)->std::unordered_set<OvmFaH>{
		std::unordered_set<OvmFaH> shared_fhs;
		foreach (auto &fh, fhs1){
			if (contains (fhs2, fh))
				shared_fhs.insert (fh);
		}
		return shared_fhs;
	};

	std::cout<<"Step 4"<<std::endl;
	std::unordered_set<OvmFaH> min_cut_fhs;
	for (int i = 0; i != min_cut.size (); ++i){
		auto bnd_fhs1 = fGetTmpBndFhs (min_cut[i].first), 
			bnd_fhs2 = fGetTmpBndFhs (min_cut[i].second);

		auto shared_fhs = fGetSharedFhs (bnd_fhs1, bnd_fhs2);
		foreach (auto &fh, shared_fhs)
			min_cut_fhs.insert (fh);
	}

	return min_cut_fhs;
}
//std::unordered_set<OvmFaH> get_volume_mesh_min_cut_new_version (VolumeMesh *mesh, std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs)
//{
//	//std::cout<<"Enter function get"<<std::endl;
//	std::vector<std::unordered_set<OvmVeH> > layered_vhs;
//	rate_inner_vhs (mesh, s_chs, t_chs, layered_vhs);
//
//	auto V_LEVEL = mesh->request_vertex_property<int> ("node level");
//	auto V_MAX_LEVEL = mesh->request_mesh_property<int> ("max node level");
//	const int INFINITY_LEVEL = std::numeric_limits<int>::max () / 1000;
//	const int MAX_LEVEL = V_MAX_LEVEL[0];
//	clock_t s1 = clock();
//	//建立网格面句柄和无向图中节点序号之间的映射关系
//	std::hash_map<OvmCeH, int> c_n_mapping;
//	std::hash_map<int, OvmCeH> n_c_mapping;
//	std::unordered_set<OvmCeH> normal_chs;
//	for (auto c_it = mesh->cells_begin (); c_it != mesh->cells_end (); ++c_it){
//		auto ch = *c_it;
//		if (!contains (s_chs, ch) && !contains (t_chs, ch)){
//			normal_chs.insert (ch);
//			int cur_idx = normal_chs.size ();
//			c_n_mapping.insert (std::make_pair(ch, cur_idx));
//			n_c_mapping.insert (std::make_pair(cur_idx, ch));
//		}
//	}
//	//构建有向图
//	int N_node = normal_chs.size () + 2;
//	int S_idx = 0, T_idx = N_node - 1;
//	Isap graph;graph.init(N_node,6*N_node);
//	clock_t e1 = clock();
//	double tcost1 =(e1-s1)/CLOCKS_PER_SEC;
//	std::cout<<"Time cost1 is "<<tcost1<<std::endl;
//	auto fRateFh = [&] (OvmFaH fh) -> int {
//		auto adj_vhs = get_adj_vertices_around_face (mesh, fh);
//		int all_vh_level = 0;
//		foreach (auto &adj_vh, adj_vhs){
//			int vh_level = V_LEVEL[adj_vh];
//			all_vh_level += MAX_LEVEL - vh_level;
//		}
//		return (all_vh_level) / adj_vhs.size () * 4;
//		//return 1;
//	};
//
//	auto fInsertGraphEdges = [&] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2,
//		int idx1, int idx2){
//			int all_level = 0;
//			if (fhs1.size () < fhs2.size ()){
//				foreach (auto &fh1, fhs1){
//					if (contains (fhs2, fh1)){
//						auto rate_level = fRateFh (fh1);
//						all_level += rate_level;
//					}
//				}
//			}else{
//				foreach (auto &fh2, fhs2){
//					if (contains (fhs1, fh2)){
//						auto rate_level = fRateFh (fh2);
//						all_level += rate_level;
//					}
//				}
//			}
//
//			if (all_level > 0){
//				graph.add_edge (idx1, idx2, all_level);
//			}
//	};
//
//	//连接s和t
//	clock_t s2 = clock();
//	std::unordered_set<OvmFaH> s_bnd_fhs, t_bnd_fhs;
//	collect_boundary_element (mesh, s_chs, NULL, NULL, &s_bnd_fhs);
//	collect_boundary_element (mesh, t_chs, NULL, NULL, &t_bnd_fhs);
//	fInsertGraphEdges (s_bnd_fhs, t_bnd_fhs, S_idx, T_idx);
//	clock_t s_2 = clock();
//	foreach (auto &nor_ch, normal_chs){
//		std::unordered_set<OvmCeH> adj_chs;
//		get_adj_hexas_around_hexa (mesh, nor_ch, adj_chs);
//		std::unordered_set<OvmFaH> adj_fhs;
//		get_adj_faces_around_hexa (mesh, nor_ch, adj_fhs);
//
//		int nor_ch_idx = c_n_mapping[nor_ch];
//		bool adj_to_s = false, adj_to_t = false;
//		std::unordered_set<OvmCeH> adj_nor_chs;
//		foreach (auto &adj_ch, adj_chs){
//			if (contains (s_chs, adj_ch))
//				adj_to_s = true;
//			else if (contains (t_chs, adj_ch))                                                                                  
//				adj_to_t = true;
//			else
//				adj_nor_chs.insert (adj_ch);
//		}
//		if (adj_to_s)
//			fInsertGraphEdges (s_bnd_fhs, adj_fhs, S_idx, nor_ch_idx);
//		if (adj_to_t)
//			fInsertGraphEdges (adj_fhs, t_bnd_fhs, nor_ch_idx, T_idx);
//		foreach (auto &adj_nor_ch, adj_nor_chs){
//			int adj_nor_ch_idx = c_n_mapping[adj_nor_ch];
//			std::unordered_set<OvmFaH> adj_nor_ch_adj_fhs;
//			get_adj_faces_around_hexa (mesh, adj_nor_ch, adj_nor_ch_adj_fhs);
//			fInsertGraphEdges (adj_fhs, adj_nor_ch_adj_fhs, nor_ch_idx, adj_nor_ch_idx);
//		}
//	}
//	clock_t e2 = clock();
//	double tcost21 = (double)(s_2-s2)/CLOCKS_PER_SEC;
//	double tcost22 = (double)(e2-s_2)/CLOCKS_PER_SEC;
//	std::cout<<"Time cost21 and cost22 is "<<tcost21<<" "<<tcost22<<std::endl;
//	//QString max_flow_str = QString ("Max flow is %1").arg (graph.ford_fulkerson());
//	auto min_cut = graph.min_cut(S_idx,T_idx,N_node);
//	double e3 = clock();
//	double tcost3 = (double)(e3-e2)/CLOCKS_PER_SEC;
//	std::cout<<"Time cost3 is "<<tcost3<<std::endl;
//	//给定一个图中的节点序号，求得它的外围面，要分四种情况
//	auto fGetTmpBndFhs = [&] (int tmp_idx) -> std::unordered_set<OvmFaH>{
//		std::unordered_set<OvmFaH> bnd_fhs;
//		if (tmp_idx == S_idx)
//			bnd_fhs = s_bnd_fhs;
//		else if (tmp_idx == T_idx)
//			bnd_fhs = t_bnd_fhs;
//		else{
//			auto nor_ch = n_c_mapping[tmp_idx];
//			get_adj_faces_around_hexa (mesh, nor_ch, bnd_fhs);
//		}
//		return bnd_fhs;
//	};
//
//	auto fGetSharedFhs = [] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2)->std::unordered_set<OvmFaH>{
//		std::unordered_set<OvmFaH> shared_fhs;
//		foreach (auto &fh, fhs1){
//			if (contains (fhs2, fh))
//				shared_fhs.insert (fh);
//		}
//		return shared_fhs;
//	};
//	clock_t s4 = clock();
//	std::unordered_set<OvmFaH> min_cut_fhs;
//	for (int i = 0; i != min_cut.size (); ++i){
//		auto bnd_fhs1 = fGetTmpBndFhs (min_cut[i].first), 
//			bnd_fhs2 = fGetTmpBndFhs (min_cut[i].second);
//
//		auto shared_fhs = fGetSharedFhs (bnd_fhs1, bnd_fhs2);
//		foreach (auto &fh, shared_fhs)
//			min_cut_fhs.insert (fh);
//	}
//	clock_t e4 = clock();
//	double tcost4 = (double)(e4-s4)/CLOCKS_PER_SEC;
//	std::cout<<"Time cost4 is "<<tcost4<<std::endl;
//	return min_cut_fhs;
//}
std::unordered_set<OvmFaH> get_volume_mesh_min_cut_new_version (VolumeMesh *mesh, std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs)
{
	//std::cout<<"Enter function get"<<std::endl;
	std::vector<std::unordered_set<OvmVeH> > layered_vhs;
	rate_inner_vhs (mesh, s_chs, t_chs, layered_vhs);

	auto V_LEVEL = mesh->request_vertex_property<int> ("node level");
	auto V_MAX_LEVEL = mesh->request_mesh_property<int> ("max node level");
	const int INFINITY_LEVEL = std::numeric_limits<int>::max () / 1000;
	const int MAX_LEVEL = V_MAX_LEVEL[0];
	//建立网格面句柄和无向图中节点序号之间的映射关系
	std::hash_map<OvmCeH, int> c_n_mapping;
	std::hash_map<int, OvmCeH> n_c_mapping;
	std::unordered_set<OvmCeH> normal_chs;
	for (auto c_it = mesh->cells_begin (); c_it != mesh->cells_end (); ++c_it){
		auto ch = *c_it;
		if (s_chs.find(ch) == s_chs.end() && t_chs.find(ch) == t_chs.end()){
			normal_chs.insert (ch);
			int cur_idx = normal_chs.size ();
			c_n_mapping.insert (std::make_pair(ch, cur_idx));
			n_c_mapping.insert (std::make_pair(cur_idx, ch));
		}
	}
	//构建有向图
	int N_node = normal_chs.size () + 2;
	int S_idx = 0, T_idx = N_node - 1;
	Isap<int> graph;graph.init(N_node,6*N_node);

	std::hash_map<OvmFaH,int> fRateFh;
	for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
	{
		int all_vh_level = 0;int adj_num = 0;
		for (auto hfv_it = mesh->hfv_iter(mesh->halfface_handle(*f_it,0));hfv_it;hfv_it++)
		{
			all_vh_level += MAX_LEVEL - V_LEVEL[*hfv_it];
			adj_num++;
		}
		fRateFh[*f_it] = all_vh_level /adj_num * 4; 
	}
	auto fInsertGraphEdges = [&] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2,
		int idx1, int idx2){
			int all_level = 0;
			if (fhs1.size () < fhs2.size ()){
				foreach (auto &fh1, fhs1){
					if (fhs2.find(fh1) != fhs2.end()){
						auto rate_level = fRateFh[fh1];
						all_level += rate_level;
					}
				}
			}else{
				foreach (auto &fh2, fhs2){
					if (fhs1.find(fh2) != fhs1.end()){
						auto rate_level = fRateFh[fh2];
						all_level += rate_level;
					}
				}
			}

			if (all_level > 0){
				graph.add_edge (idx1, idx2, all_level);
			}
	};
	//插入两个边
	auto fInsertGraphEdges_double = [&] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2,
		int idx1, int idx2){
			int all_level = 0;
			if (fhs1.size () < fhs2.size ()){
				foreach (auto &fh1, fhs1){
					if (fhs2.find(fh1) != fhs2.end()){
						auto rate_level = fRateFh[fh1];
						all_level += rate_level;
					}
				}
			}else{
				foreach (auto &fh2, fhs2){
					if (fhs1.find(fh2) != fhs1.end()){
						auto rate_level = fRateFh[fh2];
						all_level += rate_level;
					}
				}
			}

			if (all_level > 0){
				graph.add_edge (idx1, idx2, all_level);
				graph.add_edge (idx2, idx1, all_level);
			}
	};

	//连接s和t
	std::unordered_set<OvmFaH> s_bnd_fhs, t_bnd_fhs;
	collect_boundary_element (mesh, s_chs, NULL, NULL, &s_bnd_fhs);
	collect_boundary_element (mesh, t_chs, NULL, NULL, &t_bnd_fhs);
	fInsertGraphEdges (s_bnd_fhs, t_bnd_fhs, S_idx, T_idx);

	for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
	{
		//std::cout<<f_it->idx()<<std::endl;
		OvmCeH ch1 = mesh->incident_cell(mesh->halfface_handle(*f_it,0));
		OvmCeH ch2 = mesh->incident_cell(mesh->halfface_handle(*f_it,1));
		//排除ch1 ch2都为 s_ch t_ch 集合中的元素
		if ((s_chs.find(ch1) != s_chs.end() || t_chs.find(ch1) != t_chs.end()) && (s_chs.find(ch2) != s_chs.end() || t_chs.find(ch2) != t_chs.end()))
		{
			continue;
		}
		else if (s_chs.find(ch1) != s_chs.end())
		{
			std::unordered_set<OvmFaH> adj_fhs;
			get_adj_faces_around_hexa(mesh, ch2,adj_fhs);
			fInsertGraphEdges (s_bnd_fhs, adj_fhs, S_idx, c_n_mapping[ch2]);
		}
		else if (t_chs.find(ch1) != t_chs.end())
		{
			std::unordered_set<OvmFaH> adj_fhs;
			get_adj_faces_around_hexa(mesh, ch2, adj_fhs);
			fInsertGraphEdges (adj_fhs, t_bnd_fhs, c_n_mapping[ch2], T_idx);
		}
		else if (s_chs.find(ch2) != s_chs.end())
		{
			std::unordered_set<OvmFaH> adj_fhs;
			get_adj_faces_around_hexa(mesh, ch1, adj_fhs);
			fInsertGraphEdges (s_bnd_fhs, adj_fhs, S_idx, c_n_mapping[ch1]);
		}
		else if (t_chs.find(ch2) != t_chs.end())
		{
			std::unordered_set<OvmFaH> adj_fhs;
			get_adj_faces_around_hexa(mesh, ch1, adj_fhs);
			fInsertGraphEdges (adj_fhs, t_bnd_fhs, c_n_mapping[ch1], T_idx);
		}
		else
		{
			std::unordered_set<OvmFaH> adj_fhs1,adj_fhs2;
			get_adj_faces_around_hexa(mesh, ch1, adj_fhs1);
			get_adj_faces_around_hexa(mesh, ch2, adj_fhs2);
			fInsertGraphEdges_double (adj_fhs1,adj_fhs2,c_n_mapping[ch1],c_n_mapping[ch2]);
		}
	}

	//QString max_flow_str = QString ("Max flow is %1").arg (graph.ford_fulkerson());
	auto min_cut = graph.min_cut(S_idx,T_idx,N_node);

	//给定一个图中的节点序号，求得它的外围面，要分四种情况
	auto fGetTmpBndFhs = [&] (int tmp_idx) -> std::unordered_set<OvmFaH>{
		std::unordered_set<OvmFaH> bnd_fhs;
		if (tmp_idx == S_idx)
			bnd_fhs = s_bnd_fhs;
		else if (tmp_idx == T_idx)
			bnd_fhs = t_bnd_fhs;
		else{
			auto nor_ch = n_c_mapping[tmp_idx];
			get_adj_faces_around_hexa (mesh, nor_ch, bnd_fhs);
		}
		return bnd_fhs;
	};

	auto fGetSharedFhs = [] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2)->std::unordered_set<OvmFaH>{
		std::unordered_set<OvmFaH> shared_fhs;
		foreach (auto &fh, fhs1){
			if (fhs2.find(fh) != fhs2.end()/*contains (fhs2, fh)*/)
				shared_fhs.insert (fh);
		}
		return shared_fhs;
	};
	std::unordered_set<OvmFaH> min_cut_fhs;
	for (int i = 0; i != min_cut.size (); ++i){
		auto bnd_fhs1 = fGetTmpBndFhs (min_cut[i].first), 
			bnd_fhs2 = fGetTmpBndFhs (min_cut[i].second);

		auto shared_fhs = fGetSharedFhs (bnd_fhs1, bnd_fhs2);
		foreach (auto &fh, shared_fhs)
			min_cut_fhs.insert (fh);
	}
	//get_common_face_handle()
	return min_cut_fhs;
}

std::unordered_set<OvmFaH> get_volume_mesh_min_cut_considering_area (VolumeMesh *mesh, std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs)
{
	clock_t time1 = clock();
	//double alpha = 100;
	//std::cout<<"Enter function get"<<std::endl;
	std::vector<std::unordered_set<OvmVeH> > layered_vhs;
	rate_inner_vhs (mesh, s_chs, t_chs, layered_vhs);

	auto V_LEVEL = mesh->request_vertex_property<int> ("node level");
	auto V_MAX_LEVEL = mesh->request_mesh_property<int> ("max node level");
	const int INFINITY_LEVEL = std::numeric_limits<int>::max () / 1000;
	const int MAX_LEVEL = V_MAX_LEVEL[0];
	//建立网格面句柄和无向图中节点序号之间的映射关系
	std::hash_map<OvmCeH, int> c_n_mapping;
	std::hash_map<int, OvmCeH> n_c_mapping;
	std::unordered_set<OvmCeH> normal_chs;
	for (auto c_it = mesh->cells_begin (); c_it != mesh->cells_end (); ++c_it){
		auto ch = *c_it;
		if (s_chs.find(ch) == s_chs.end() && t_chs.find(ch) == t_chs.end()){
			normal_chs.insert (ch);
			int cur_idx = normal_chs.size ();
			c_n_mapping.insert (std::make_pair(ch, cur_idx));
			n_c_mapping.insert (std::make_pair(cur_idx, ch));
		}
	}
	//构建有向图
	int N_node = normal_chs.size () + 2;
	int S_idx = 0, T_idx = N_node - 1;
	Isap<double> graph;
	//graph.init(N_node,6*N_node);
	
	graph.init(N_node,6*N_node);
	
	std::hash_map<OvmFaH,int> fRateFh;
	for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
	{
		int all_vh_level = 0;int adj_num = 0;
		for (auto hfv_it = mesh->hfv_iter(mesh->halfface_handle(*f_it,0));hfv_it;hfv_it++)
		{
			all_vh_level += MAX_LEVEL - V_LEVEL[*hfv_it];
			adj_num++;
		}
		fRateFh[*f_it] = all_vh_level /adj_num * 4; 
	}
	
	auto fInsertGraphEdges = [&] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2,
		int idx1, int idx2,double _area, bool if_double){
			int all_level = 0;
			if (fhs1.size () < fhs2.size ()){
				foreach (auto &fh1, fhs1){
					if (fhs2.find(fh1) != fhs2.end()){
						auto rate_level = fRateFh[fh1];
						all_level += rate_level;
					}
				}
			}else{
				foreach (auto &fh2, fhs2){
					if (fhs1.find(fh2) != fhs1.end()){
						auto rate_level = fRateFh[fh2];
						all_level += rate_level;
					}
				}
			}
			if (all_level > 0){
				//std::cout<<__LINE__<<idx1<<" "<<idx2<<" "<<_area<<std::endl;
				graph.add_edge (idx1, idx2, /*all_level+*/alpha*_area);
				//graph.add_edge(idx1, idx2, _area);
				if (if_double)
				{
					//graph.add_edge(idx2, idx1, _area);
					graph.add_edge(idx2,idx1,/*all_level+*/alpha*_area);
				}
			}
	};
	auto fComputeQuadArea = [&](OvmFaH fh) ->double{
		auto adj_hehs = mesh->face(fh).halfedges();
		OvmVec3d dire[4];
		for (int hei = 0;hei < 4;hei++)
		{
			dire[hei] = (mesh->vertex(mesh->halfedge(adj_hehs[hei]).to_vertex()) - mesh->vertex(mesh->halfedge(adj_hehs[hei]).from_vertex()));
		}
		OvmVec3d cr1 = cross(dire[0], dire[1]);
		OvmVec3d cr2 = cross(dire[2], dire[3]);
		double ans = (dot(cr1,cr2) > 0) ? cr1.length() + cr2.length() : cr1.length() - cr2.length();
		ans = abs(ans)/2.0;
		return ans;
	};
	
	//连接s和t
	std::unordered_set<OvmFaH> s_bnd_fhs, t_bnd_fhs;
	collect_boundary_element (mesh, s_chs, NULL, NULL, &s_bnd_fhs);
	collect_boundary_element (mesh, t_chs, NULL, NULL, &t_bnd_fhs);
	double _temp_area;
	_temp_area = 0;
	for (auto sf_it = s_bnd_fhs.begin();sf_it != s_bnd_fhs.end();sf_it++)
	{
		if (t_bnd_fhs.find(*sf_it) != t_bnd_fhs.end())
		{
			_temp_area += fComputeQuadArea(*sf_it);
		}
	}
	
	fInsertGraphEdges (s_bnd_fhs, t_bnd_fhs, S_idx, T_idx,_temp_area,false);
	
	for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
	{
		if (mesh->is_boundary(*f_it))
			continue;
		//std::cout<<f_it->idx()<<std::endl;
		_temp_area = fComputeQuadArea(*f_it);
		OvmCeH ch1 = mesh->incident_cell(mesh->halfface_handle(*f_it,0));
		OvmCeH ch2 = mesh->incident_cell(mesh->halfface_handle(*f_it,1));
		//排除ch1 ch2都为 s_ch t_ch 集合中的元素
		if ((s_chs.find(ch1) != s_chs.end() || t_chs.find(ch1) != t_chs.end()) && (s_chs.find(ch2) != s_chs.end() || t_chs.find(ch2) != t_chs.end()))
		{
			continue;
		}
		else if (s_chs.find(ch1) != s_chs.end())
		{
			std::unordered_set<OvmFaH> adj_fhs;
			get_adj_faces_around_hexa(mesh, ch2,adj_fhs);
			fInsertGraphEdges (s_bnd_fhs, adj_fhs, S_idx, c_n_mapping[ch2],_temp_area,false);
		}
		else if (t_chs.find(ch1) != t_chs.end())
		{
			std::unordered_set<OvmFaH> adj_fhs;
			get_adj_faces_around_hexa(mesh, ch2, adj_fhs);		
			fInsertGraphEdges (adj_fhs, t_bnd_fhs, c_n_mapping[ch2], T_idx,_temp_area,false);
		}
		else if (s_chs.find(ch2) != s_chs.end())
		{
			std::unordered_set<OvmFaH> adj_fhs;
			get_adj_faces_around_hexa(mesh, ch1, adj_fhs);
			fInsertGraphEdges (s_bnd_fhs, adj_fhs, S_idx, c_n_mapping[ch1],_temp_area,false);
		}
		else if (t_chs.find(ch2) != t_chs.end())
		{
			std::unordered_set<OvmFaH> adj_fhs;
			get_adj_faces_around_hexa(mesh, ch1, adj_fhs);
			fInsertGraphEdges (adj_fhs, t_bnd_fhs, c_n_mapping[ch1], T_idx,_temp_area,false);
		}
		else
		{
			std::unordered_set<OvmFaH> adj_fhs1,adj_fhs2;
			get_adj_faces_around_hexa(mesh, ch1, adj_fhs1);
			get_adj_faces_around_hexa(mesh, ch2, adj_fhs2);
			fInsertGraphEdges(adj_fhs1,adj_fhs2,c_n_mapping[ch1],c_n_mapping[ch2],_temp_area,true);
			//fInsertGraphEdges_double (adj_fhs1,adj_fhs2,c_n_mapping[ch1],c_n_mapping[ch2]);
		}
	}
	//std::cout<<__LINE__<<std::endl;
	//QString max_flow_str = QString ("Max flow is %1").arg (graph.ford_fulkerson());
	auto min_cut = graph.min_cut(S_idx,T_idx,N_node);
	//std::cout<<__LINE__<<std::endl;
	//给定一个图中的节点序号，求得它的外围面，要分四种情况
	auto fGetTmpBndFhs = [&] (int tmp_idx) -> std::unordered_set<OvmFaH>{
		std::unordered_set<OvmFaH> bnd_fhs;
		if (tmp_idx == S_idx)
			bnd_fhs = s_bnd_fhs;
		else if (tmp_idx == T_idx)
			bnd_fhs = t_bnd_fhs;
		else{
			auto nor_ch = n_c_mapping[tmp_idx];
			get_adj_faces_around_hexa (mesh, nor_ch, bnd_fhs);
		}
		return bnd_fhs;
	};

	auto fGetSharedFhs = [] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2)->std::unordered_set<OvmFaH>{
		std::unordered_set<OvmFaH> shared_fhs;
		foreach (auto &fh, fhs1){
			if (fhs2.find(fh) != fhs2.end()/*contains (fhs2, fh)*/)
				shared_fhs.insert (fh);
		}
		return shared_fhs;
	};
	std::unordered_set<OvmFaH> min_cut_fhs;
	for (int i = 0; i != min_cut.size (); ++i){
		auto bnd_fhs1 = fGetTmpBndFhs (min_cut[i].first), 
			bnd_fhs2 = fGetTmpBndFhs (min_cut[i].second);

		auto shared_fhs = fGetSharedFhs (bnd_fhs1, bnd_fhs2);
		foreach (auto &fh, shared_fhs)
			min_cut_fhs.insert (fh);
	}
	clock_t time2 = clock();
	double tcost22 = (double)(time2- time1)/CLOCKS_PER_SEC;
	std::cout<<"Mesh size is Cell "<<mesh->n_cells()<<"Time cost is "<<tcost22<<std::endl;

	//std::cout<<__LINE__<<std::endl;
	//get_common_face_handle()
	return min_cut_fhs;
}
//
//std::unordered_set<OvmFaH> get_volume_mesh_min_cut_boost_version (VolumeMesh *mesh, std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs)
//{
//	//std::cout<<"Enter function get"<<std::endl;
//	std::vector<std::unordered_set<OvmVeH> > layered_vhs;
//	rate_inner_vhs (mesh, s_chs, t_chs, layered_vhs);
//
//	auto V_LEVEL = mesh->request_vertex_property<int> ("node level");
//	auto V_MAX_LEVEL = mesh->request_mesh_property<int> ("max node level");
//	const int INFINITY_LEVEL = std::numeric_limits<int>::max () / 1000;
//	const int MAX_LEVEL = V_MAX_LEVEL[0];
//	clock_t s1 = clock();
//	//建立网格面句柄和无向图中节点序号之间的映射关系
//	std::hash_map<OvmCeH, int> c_n_mapping;
//	std::hash_map<int, OvmCeH> n_c_mapping;
//	std::unordered_set<OvmCeH> normal_chs;
//	for (auto c_it = mesh->cells_begin (); c_it != mesh->cells_end (); ++c_it){
//		auto ch = *c_it;
//		if (s_chs.find(ch) == s_chs.end() && t_chs.find(ch) == t_chs.end()){
//			normal_chs.insert (ch);
//			int cur_idx = normal_chs.size ();
//			c_n_mapping.insert (std::make_pair(ch, cur_idx));
//			n_c_mapping.insert (std::make_pair(cur_idx, ch));
//		}
//	}
//
//	//构建有向图
//	int N_node = normal_chs.size () + 2;
//	int S_idx = 0, T_idx = N_node - 1;
//	clock_t e11 = clock();
//	std::vector<std::pair<int, int>> arcs;
//	std::vector<int> weights;
//	std::vector<OvmHaFaH> undirected_arc2hfh; 
//	std::hash_map<OvmFaH,int> fRateFh;
//	for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
//	{
//		int all_vh_level = 0;int adj_num = 0;
//		for (auto hfv_it = mesh->hfv_iter(mesh->halfface_handle(*f_it,0));hfv_it;hfv_it++)
//		{
//			all_vh_level += MAX_LEVEL - V_LEVEL[*hfv_it];
//			adj_num++;
//		}
//		fRateFh[*f_it] = all_vh_level /adj_num * 4; 
//	}
//
//	auto fInsertGraphEdges = [&] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2,
//		int idx1, int idx2){
//			int all_level = 0;
//			if (fhs1.size () < fhs2.size ()){
//				foreach (auto &fh1, fhs1){
//					if (fhs2.find(fh1) != fhs2.end()){
//						auto rate_level = fRateFh[fh1];
//						all_level += rate_level;
//					}
//				}
//			}else{
//				foreach (auto &fh2, fhs2){
//					if (fhs1.find(fh2) != fhs1.end()){
//						auto rate_level = fRateFh[fh2];
//						all_level += rate_level;
//					}
//				}
//			}
//
//			if (all_level > 0)
//			{
//				std::pair<int, int> tmp = std::make_pair(idx1, idx2);
//				arcs.push_back(tmp);
//				weights.push_back(all_level);
//				//这里的插面放在外部处理
//			}
//	};
//	//插入两个边
//	auto fInsertGraphEdges_double = [&] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2,
//		int idx1, int idx2){
//			int all_level = 0;
//			if (fhs1.size () < fhs2.size ()){
//				foreach (auto &fh1, fhs1){
//					if (fhs2.find(fh1) != fhs2.end()){
//						auto rate_level = fRateFh[fh1];
//						all_level += rate_level;
//					}
//				}
//			}else{
//				foreach (auto &fh2, fhs2){
//					if (fhs1.find(fh2) != fhs1.end()){
//						auto rate_level = fRateFh[fh2];
//						all_level += rate_level;
//					}
//				}
//			}
//
//			if (all_level > 0)
//			{
//				//graph.add_edge (idx1, idx2, all_level);
//				//graph.add_edge (idx2, idx1, all_level);
//				std::pair<int, int> tmp1 = std::make_pair(idx1,idx2);
//				std::pair<int, int> tmp2 = std::make_pair(idx2,idx1);
//				arcs.push_back(tmp1);
//				//arcs.push_back(tmp2);
//				weights.push_back(all_level);
//				//weights.push_back(all_level);
//			}
//	};
//
//
//	//连接s和t
//	std::unordered_set<OvmFaH> s_bnd_fhs, t_bnd_fhs;
//	collect_boundary_element (mesh, s_chs, NULL, NULL, &s_bnd_fhs);
//	collect_boundary_element (mesh, t_chs, NULL, NULL, &t_bnd_fhs);
//	fInsertGraphEdges (s_bnd_fhs, t_bnd_fhs, S_idx, T_idx);
//	undirected_arc2hfh.push_back(mesh->InvalidHalfFaceHandle);
//	clock_t s_2 = clock();
//
//	for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
//	{
//		OvmCeH ch1 = mesh->incident_cell(mesh->halfface_handle(*f_it,0));
//		OvmCeH ch2 = mesh->incident_cell(mesh->halfface_handle(*f_it,1));
//		//排除ch1 ch2都为 s_ch t_ch 集合中的元素
//		if ((s_chs.find(ch1) != s_chs.end() || t_chs.find(ch1) != t_chs.end()) && (s_chs.find(ch2) != s_chs.end() || t_chs.find(ch2) != t_chs.end()))
//		{
//			continue;
//		}
//		else if (s_chs.find(ch1) != s_chs.end())
//		{
//			std::unordered_set<OvmFaH> adj_fhs;
//			get_adj_faces_around_hexa(mesh, ch2,adj_fhs);
//			fInsertGraphEdges (s_bnd_fhs, adj_fhs, S_idx, c_n_mapping[ch2]);
//			undirected_arc2hfh.push_back(mesh->halfface_handle(*f_it,1));
//		}
//		else if (t_chs.find(ch1) != t_chs.end())
//		{
//			std::unordered_set<OvmFaH> adj_fhs;
//			get_adj_faces_around_hexa(mesh, ch2, adj_fhs);
//			fInsertGraphEdges (adj_fhs, t_bnd_fhs, c_n_mapping[ch2], T_idx);
//			undirected_arc2hfh.push_back(mesh->halfface_handle(*f_it,1));
//		}
//		else if (s_chs.find(ch2) != s_chs.end())
//		{
//			std::unordered_set<OvmFaH> adj_fhs;
//			get_adj_faces_around_hexa(mesh, ch1, adj_fhs);
//			fInsertGraphEdges (s_bnd_fhs, adj_fhs, S_idx, c_n_mapping[ch1]);
//			undirected_arc2hfh.push_back(mesh->halfface_handle(*f_it,0));
//		}
//		else if (t_chs.find(ch2) != t_chs.end())
//		{
//			std::unordered_set<OvmFaH> adj_fhs;
//			get_adj_faces_around_hexa(mesh, ch1, adj_fhs);
//			fInsertGraphEdges (adj_fhs, t_bnd_fhs, c_n_mapping[ch1], T_idx);
//			undirected_arc2hfh.push_back(mesh->halfface_handle(*f_it,0));
//		}
//		else
//		{
//			std::unordered_set<OvmFaH> adj_fhs1,adj_fhs2;
//			get_adj_faces_around_hexa(mesh, ch1, adj_fhs1);
//			get_adj_faces_around_hexa(mesh, ch2, adj_fhs2);
//			fInsertGraphEdges_double (adj_fhs1,adj_fhs2,c_n_mapping[ch1],c_n_mapping[ch2]);
//			undirected_arc2hfh.push_back(mesh->halfface_handle(*f_it,0));
//			//undirected_arc2hfh.push_back(mesh->halfface_handle(*f_it,1));
//		}
//	}
//	//用boost 构图求解
//	clock_t ss1 = clock();
//	undirected_graph g;
//	g.clear();
//	boost::property_map<undirected_graph, boost::edge_weight_t>::type pmpWeightmap = boost::get(boost::edge_weight, g);      
//	for(int i=0;i<arcs.size();i++)  
//	{  
//		edgedescriptor edEdge;  
//		bool bInserted;  
//		//加入Link的起始和终止NodeID  
//		boost::tie(edEdge,bInserted)=boost::add_edge(arcs[i].first,arcs[i].second,g);  
//		//加入Link的长度  
//		pmpWeightmap[edEdge]=weights[i];  
//	}  
//	clock_t ee11 = clock();
//	double tcost11 = (double)(ee11-ss1)/CLOCKS_PER_SEC;
//	std::cout<<"Time cost 11 "<<tcost11<<std::endl;
//	BOOST_AUTO(parities, boost::make_one_bit_color_map(num_vertices(g), get(boost::vertex_index, g)));
//	boost::stoer_wagner_min_cut(g, boost::get(boost::edge_weight, g), boost::parity_map(parities));
//	clock_t ee12 = clock();
//	double tcost12 = (double)(ee12-ee11)/CLOCKS_PER_SEC;
//	std::cout<<"Time cost 12 "<<tcost12<<std::endl;
//	//////////////////////////////////////////////////////////////////////////
//	//求最小割
//	std::unordered_set<OvmFaH> min_cut_fhs;
//	for (auto s_fh_it = s_bnd_fhs.begin();s_fh_it != s_bnd_fhs.end();s_fh_it++)
//	{
//		if (t_bnd_fhs.find(*s_fh_it) != t_bnd_fhs.end())
//		{
//			min_cut_fhs.insert(*s_fh_it);
//		}
//	}
//	clock_t ee21 = clock();
//	double tcost21 = (double)(ee21-ee12)/CLOCKS_PER_SEC;
//	std::cout<<"Time cost 21 "<<tcost21<<std::endl;
//	for (int ai = 0;ai < arcs.size();ai++)
//	{
//		bool t_1 = boost::get(parities,arcs[ai].first);
//		bool t_2 = boost::get(parities,arcs[ai].second);
//		if ((t_1 && !t_2) || (!t_1 && t_2))
//		{
//			min_cut_fhs.insert(mesh->face_handle(undirected_arc2hfh[ai]));
//		}
//	}
//	clock_t ee22 = clock();
//
//	double tcost22 = (double)(ee22- ee21)/CLOCKS_PER_SEC;
//	std::cout<<"Time cost is "<<tcost11<<" "<<tcost12<<" "<<tcost21<<" "<<tcost22<<std::endl;
//	return min_cut_fhs;
//}

std::unordered_set<OvmFaH> get_local_mesh_min_cut (VolumeMesh *mesh, std::unordered_set<OvmCeH> &all_chs, std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs)
{
	//建立网格面句柄和无向图中节点序号之间的映射关系
	std::hash_map<OvmCeH, int> c_n_mapping;
	std::hash_map<int, OvmCeH> n_c_mapping;
	std::unordered_set<OvmCeH> normal_chs;

	for (auto c_it = mesh->cells_begin (); c_it != mesh->cells_end (); ++c_it){
		auto ch = *c_it;
		if (!contains (s_chs, ch) && !contains (t_chs, ch)){
			normal_chs.insert (ch);
			int cur_idx = normal_chs.size ();
			c_n_mapping.insert (std::make_pair(ch, cur_idx));
			n_c_mapping.insert (std::make_pair(cur_idx, ch));
		}
	}

	//构建有向图
	int N_node = normal_chs.size () + 2;
	int S_idx = 0, T_idx = N_node - 1;
	CoreSolver<int> graph (N_node, S_idx, T_idx);

	auto fInsertGraphEdges = [&] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2,
		int idx1, int idx2){
			int all_level = 0;
			if (fhs1.size () < fhs2.size ()){
				foreach (auto &fh1, fhs1){
					if (contains (fhs2, fh1)){
						auto rate_level = 1;//fRateFh (fh1);
						all_level += rate_level;
					}
				}
			}else{
				foreach (auto &fh2, fhs2){
					if (contains (fhs1, fh2)){
						auto rate_level = 1;//fRateFh (fh2);
						all_level += rate_level;
					}
				}
			}

			if (all_level > 0){
				graph.insert_edge (idx1, idx2, all_level);
			}
	};

	//连接s和t
	std::unordered_set<OvmFaH> s_bnd_fhs, t_bnd_fhs;
	collect_boundary_element (mesh, s_chs, NULL, NULL, &s_bnd_fhs);
	collect_boundary_element (mesh, t_chs, NULL, NULL, &t_bnd_fhs);
	fInsertGraphEdges (s_bnd_fhs, t_bnd_fhs, S_idx, T_idx);

	foreach (auto &nor_ch, normal_chs){
		std::unordered_set<OvmCeH> adj_chs;
		get_adj_hexas_around_hexa (mesh, nor_ch, adj_chs);
		std::unordered_set<OvmFaH> adj_fhs;
		get_adj_faces_around_hexa (mesh, nor_ch, adj_fhs);

		int nor_ch_idx = c_n_mapping[nor_ch];
		bool adj_to_s = false, adj_to_t = false;
		std::unordered_set<OvmCeH> adj_nor_chs;
		foreach (auto &adj_ch, adj_chs){
			if (contains (s_chs, adj_ch))
				adj_to_s = true;
			else if (contains (t_chs, adj_ch))                                                                                               
				adj_to_t = true;
			else
				adj_nor_chs.insert (adj_ch);
		}
		if (adj_to_s)
			fInsertGraphEdges (s_bnd_fhs, adj_fhs, S_idx, nor_ch_idx);
		if (adj_to_t)
			fInsertGraphEdges (adj_fhs, t_bnd_fhs, nor_ch_idx, T_idx);
		foreach (auto &adj_nor_ch, adj_nor_chs){
			int adj_nor_ch_idx = c_n_mapping[adj_nor_ch];
			std::unordered_set<OvmFaH> adj_nor_ch_adj_fhs;
			get_adj_faces_around_hexa (mesh, adj_nor_ch, adj_nor_ch_adj_fhs);
			fInsertGraphEdges (adj_fhs, adj_nor_ch_adj_fhs, nor_ch_idx, adj_nor_ch_idx);
		}
	}

	QString max_flow_str = QString ("Max flow is %1").arg (graph.ford_fulkerson());
	auto min_cut = graph.mincut ();

	//给定一个图中的节点序号，求得它的外围面，要分四种情况
	auto fGetTmpBndFhs = [&] (int tmp_idx) -> std::unordered_set<OvmFaH>{
		std::unordered_set<OvmFaH> bnd_fhs;
		if (tmp_idx == S_idx)
			bnd_fhs = s_bnd_fhs;
		else if (tmp_idx == T_idx)
			bnd_fhs = t_bnd_fhs;
		else{
			auto nor_ch = n_c_mapping[tmp_idx];
			get_adj_faces_around_hexa (mesh, nor_ch, bnd_fhs);
		}
		return bnd_fhs;
	};

	auto fGetSharedFhs = [] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2)->std::unordered_set<OvmFaH>{
		std::unordered_set<OvmFaH> shared_fhs;
		foreach (auto &fh, fhs1){
			if (contains (fhs2, fh))
				shared_fhs.insert (fh);
		}
		return shared_fhs;
	};

	std::unordered_set<OvmFaH> min_cut_fhs;
	for (int i = 0; i != min_cut.size (); ++i){
		auto bnd_fhs1 = fGetTmpBndFhs (min_cut[i].first), 
			bnd_fhs2 = fGetTmpBndFhs (min_cut[i].second);

		auto shared_fhs = fGetSharedFhs (bnd_fhs1, bnd_fhs2);
		foreach (auto &fh, shared_fhs)
			min_cut_fhs.insert (fh);
	}

	return min_cut_fhs;
}

std::unordered_set<OvmFaH> get_local_mesh_min_cut_new_version (VolumeMesh *mesh, std::unordered_set<OvmCeH> &all_chs, std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs)
{
	//建立网格面句柄和无向图中节点序号之间的映射关系
	std::hash_map<OvmCeH, int> c_n_mapping;
	std::hash_map<int, OvmCeH> n_c_mapping;
	std::unordered_set<OvmCeH> normal_chs;

	for (auto c_it = mesh->cells_begin (); c_it != mesh->cells_end (); ++c_it){
		auto ch = *c_it;
		if (!contains (s_chs, ch) && !contains (t_chs, ch)){
			normal_chs.insert (ch);
			int cur_idx = normal_chs.size ();
			c_n_mapping.insert (std::make_pair(ch, cur_idx));
			n_c_mapping.insert (std::make_pair(cur_idx, ch));
		}
	}

	//构建有向图
	int N_node = normal_chs.size () + 2;
	int S_idx = 0, T_idx = N_node - 1;
	//CoreSolver<int> graph (N_node, S_idx, T_idx);
	Isap<int> graph;graph.init(N_node,6*N_node);
	auto fInsertGraphEdges = [&] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2,
		int idx1, int idx2){
			int all_level = 0;
			if (fhs1.size () < fhs2.size ()){
				foreach (auto &fh1, fhs1){
					if (contains (fhs2, fh1)){
						auto rate_level = 1;//fRateFh (fh1);
						all_level += rate_level;
					}
				}
			}else{
				foreach (auto &fh2, fhs2){
					if (contains (fhs1, fh2)){
						auto rate_level = 1;//fRateFh (fh2);
						all_level += rate_level;
					}
				}
			}

			if (all_level > 0){
				graph.add_edge(idx1, idx2, all_level);//graph.insert_edge (idx1, idx2, all_level);
			}
	};

	//连接s和t
	std::unordered_set<OvmFaH> s_bnd_fhs, t_bnd_fhs;
	collect_boundary_element (mesh, s_chs, NULL, NULL, &s_bnd_fhs);
	collect_boundary_element (mesh, t_chs, NULL, NULL, &t_bnd_fhs);
	fInsertGraphEdges (s_bnd_fhs, t_bnd_fhs, S_idx, T_idx);

	foreach (auto &nor_ch, normal_chs){
		std::unordered_set<OvmCeH> adj_chs;
		get_adj_hexas_around_hexa (mesh, nor_ch, adj_chs);
		std::unordered_set<OvmFaH> adj_fhs;
		get_adj_faces_around_hexa (mesh, nor_ch, adj_fhs);

		int nor_ch_idx = c_n_mapping[nor_ch];
		bool adj_to_s = false, adj_to_t = false;
		std::unordered_set<OvmCeH> adj_nor_chs;
		foreach (auto &adj_ch, adj_chs){
			if (contains (s_chs, adj_ch))
				adj_to_s = true;
			else if (contains (t_chs, adj_ch))                                                                                               
				adj_to_t = true;
			else
				adj_nor_chs.insert (adj_ch);
		}
		if (adj_to_s)
			fInsertGraphEdges (s_bnd_fhs, adj_fhs, S_idx, nor_ch_idx);
		if (adj_to_t)
			fInsertGraphEdges (adj_fhs, t_bnd_fhs, nor_ch_idx, T_idx);
		foreach (auto &adj_nor_ch, adj_nor_chs){
			int adj_nor_ch_idx = c_n_mapping[adj_nor_ch];
			std::unordered_set<OvmFaH> adj_nor_ch_adj_fhs;
			get_adj_faces_around_hexa (mesh, adj_nor_ch, adj_nor_ch_adj_fhs);
			fInsertGraphEdges (adj_fhs, adj_nor_ch_adj_fhs, nor_ch_idx, adj_nor_ch_idx);
		}
	}

	//QString max_flow_str = QString ("Max flow is %1").arg (graph.ford_fulkerson());
	auto min_cut = graph.min_cut(S_idx,T_idx,N_node);

	//给定一个图中的节点序号，求得它的外围面，要分四种情况
	auto fGetTmpBndFhs = [&] (int tmp_idx) -> std::unordered_set<OvmFaH>{
		std::unordered_set<OvmFaH> bnd_fhs;
		if (tmp_idx == S_idx)
			bnd_fhs = s_bnd_fhs;
		else if (tmp_idx == T_idx)
			bnd_fhs = t_bnd_fhs;
		else{
			auto nor_ch = n_c_mapping[tmp_idx];
			get_adj_faces_around_hexa (mesh, nor_ch, bnd_fhs);
		}
		return bnd_fhs;
	};

	auto fGetSharedFhs = [] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2)->std::unordered_set<OvmFaH>{
		std::unordered_set<OvmFaH> shared_fhs;
		foreach (auto &fh, fhs1){
			if (contains (fhs2, fh))
				shared_fhs.insert (fh);
		}
		return shared_fhs;
	};

	std::unordered_set<OvmFaH> min_cut_fhs;
	for (int i = 0; i != min_cut.size (); ++i){
		auto bnd_fhs1 = fGetTmpBndFhs (min_cut[i].first), 
			bnd_fhs2 = fGetTmpBndFhs (min_cut[i].second);

		auto shared_fhs = fGetSharedFhs (bnd_fhs1, bnd_fhs2);
		foreach (auto &fh, shared_fhs)
			min_cut_fhs.insert (fh);
	}

	return min_cut_fhs;
}

std::unordered_set<OvmEgH> get_quad_mesh_min_cut (VolumeMesh *mesh, 
	std::unordered_set<OvmFaH> &s_fhs, std::unordered_set<OvmFaH> &t_fhs,
	std::vector<std::unordered_set<OvmFaH> > &obstacles)
{
	auto V_LEVEL = mesh->request_vertex_property<int> ("node level");
	auto V_MAX_LEVEL = mesh->request_mesh_property<int> ("max node level");
	const int INFINITY_LEVEL = std::numeric_limits<int>::max () / 1000;
	const int MAX_LEVEL = V_MAX_LEVEL[0];
	//构建有向图
	std::unordered_set<OvmFaH> normal_fhs;
	for (auto bf_it = mesh->bf_iter (); bf_it; ++bf_it){
		auto fh = *bf_it;
		if (contains (s_fhs, fh) || contains (t_fhs, fh)) continue;
		bool in_ob = false;
		foreach (auto &ob, obstacles){
			if (contains (ob, fh)){
				in_ob = true; break;
			}
		}
		if (!in_ob)
			normal_fhs.insert (fh);
	}

	int N_node = normal_fhs.size () + 2 + obstacles.size ();
	int S_idx = 0, T_idx = N_node - 1;
	CoreSolver<int> graph (N_node, S_idx, T_idx);

	auto fWhichOb = [&] (OvmFaH test_fh, int &ob_idx)->bool{
		for (int i = 0; i != obstacles.size (); ++i){
			if (contains (obstacles[i], test_fh)){
				ob_idx = i;
				return true;
			}
		}
		return false;
	};

	//0号节点是s节点，最后一个节点是t节点，ob节点排布在1~ ob.size
	std::hash_map<OvmFaH, int> f_n_mapping;
	std::hash_map<int, OvmFaH> n_f_mapping;
	int cur_idx = 1 + obstacles.size ();

	foreach (auto &fh, normal_fhs){
		f_n_mapping.insert (std::make_pair (fh, cur_idx));
		n_f_mapping.insert (std::make_pair (cur_idx, fh));
		cur_idx++;
	}

	auto fGetBndEhs = [&] (std::unordered_set<OvmFaH> &fhs)->std::unordered_set<OvmEgH>{
		std::unordered_set<OvmEgH> bnd_ehs;
		foreach (auto &fh, fhs){
			auto hehs = mesh->face (fh).halfedges ();
			foreach (auto &heh, hehs){
				auto eh = mesh->edge_handle (heh);
				if (contains (bnd_ehs, eh))	bnd_ehs.erase (eh);
				else bnd_ehs.insert (eh);
			}
		}
		return bnd_ehs;
	};

	auto fRateEh = [&] (OvmEgH eh) -> int {
		auto vh1 = mesh->edge (eh).from_vertex (),
			vh2 = mesh->edge (eh).to_vertex ();
		auto vh1_level = V_LEVEL[vh1],
			vh2_level = V_LEVEL[vh2];
		if (vh1_level == 0 && vh2_level == 0)
			return INFINITY_LEVEL;
		return ((MAX_LEVEL - vh1_level) + (MAX_LEVEL - vh2_level)) / 2;
	};

	auto fInsertGraphEdges = [&] (std::unordered_set<OvmEgH> &ehs1, std::unordered_set<OvmEgH> &ehs2,
		int idx1, int idx2, bool mutual){
			int all_level = 0;
			if (ehs1.size () < ehs2.size ()){
				foreach (auto &eh1, ehs1){
					if (contains (ehs2, eh1)){
						auto rate_level = fRateEh (eh1);
						all_level += rate_level;
					}
				}
			}else{
				foreach (auto &eh2, ehs2){
					if (contains (ehs1, eh2)){
						auto rate_level = fRateEh (eh2);
						all_level += rate_level;
					}
				}
			}

			if (all_level > 0){
				graph.insert_edge (idx1, idx2, all_level);
				if (mutual)
					graph.insert_edge (idx2, idx1, all_level);
			}
	};

	//首先处理s、t和ob之间的连接关系
	auto s_bnd_ehs = fGetBndEhs (s_fhs),
		t_bnd_ehs = fGetBndEhs (t_fhs);

	//s和t之间
	//if (JC::intersects (s_bnd_ehs, t_bnd_ehs))
	//	graph.insert_edge (S_idx, T_idx, 1);

	fInsertGraphEdges (s_bnd_ehs, t_bnd_ehs, S_idx, T_idx, false);

	//s和ob之间 ob和t之间
	std::vector<std::unordered_set<OvmEgH> > ob_bnd_ehs;
	for (int i = 0; i != obstacles.size (); ++i){
		auto one_ob_bnd_ehs = fGetBndEhs (obstacles[i]);
		//if (JC::intersects (s_bnd_ehs, one_ob_bnd_ehs))
		//	graph.insert_edge (S_idx, i, 1);
		//if (JC::intersects (one_ob_bnd_ehs, t_bnd_ehs))
		//	graph.insert_edge (i, T_idx, 1);

		fInsertGraphEdges (s_bnd_ehs, one_ob_bnd_ehs, S_idx, i, false);
		fInsertGraphEdges (one_ob_bnd_ehs, t_bnd_ehs, i, T_idx, false);
		ob_bnd_ehs.push_back (one_ob_bnd_ehs);
	}

	//ob之间
	for (int i = 0; i != obstacles.size () - 1; ++i){
		auto ob_bnd_ehs_i = ob_bnd_ehs[i];
		for (int j = i + 1; j != obstacles.size (); ++j){
			auto ob_bnd_ehs_j = ob_bnd_ehs[j];
			//if (JC::intersects (ob_bnd_ehs_i, ob_bnd_ehs_j)){
			//	graph.insert_edge (1 + i, 1 + j, 1);
			//	graph.insert_edge (1 + j, 1 + i, 1);
			//}
			fInsertGraphEdges (ob_bnd_ehs_i, ob_bnd_ehs_j, i, j, true);
		}
	}

	//普通节点之间
	foreach (auto &fh, normal_fhs){
		auto fh_idx = f_n_mapping[fh];
		std::unordered_set<OvmFaH> adj_fhs;
		get_adj_boundary_faces_around_face (mesh, fh, adj_fhs);
		std::unordered_set<OvmEgH> adj_ehs;
		get_adj_edges_around_face (mesh, fh, adj_ehs);

		bool adj_to_S = false, adj_to_T = false;
		std::set<int> ob_indices;
		std::set<OvmFaH> nor_fhs;
		foreach (auto &adj_fh, adj_fhs){
			int ob_idx = -1;
			if (contains (s_fhs, adj_fh))
				adj_to_S = true;
			else if (contains (t_fhs, adj_fh))
				adj_to_T = true;
			else if (fWhichOb (adj_fh, ob_idx))
				ob_indices.insert (ob_idx);
			else{

				nor_fhs.insert (adj_fh);
			}
		}
		if (adj_to_S)
			//graph.insert_edge (S_idx, fh_idx, 1);
			fInsertGraphEdges (s_bnd_ehs, adj_ehs, S_idx, fh_idx, false);
		if (adj_to_T)
			//graph.insert_edge (fh_idx, T_idx, 1);
			fInsertGraphEdges (adj_ehs, t_bnd_ehs, fh_idx, T_idx, false);
		foreach (auto &ob_idx, ob_indices){
			//graph.insert_edge (ob_idx, fh_idx, 1);
			//graph.insert_edge (fh_idx, ob_idx, 1);
			fInsertGraphEdges (ob_bnd_ehs[ob_idx], adj_ehs, ob_idx, fh_idx, true);
		}
		//foreach (auto &adj_fh_idx, nor_indices){
		foreach (auto &adj_fh, nor_fhs){
			//graph.insert_edge (fh_idx, adj_fh_idx, 1);
			auto adj_fh_idx = f_n_mapping[adj_fh];
			std::unordered_set<OvmEgH> adj_ehs2;
			get_adj_edges_around_face (mesh, adj_fh, adj_ehs2);
			fInsertGraphEdges (adj_ehs, adj_ehs2, fh_idx, adj_fh_idx, false);
		}
	}

	//下面求最大流最小割
	QString max_flow_str = QString ("Max flow is %1").arg (graph.ford_fulkerson());
	auto min_cut = graph.mincut ();

	std::unordered_set<OvmEgH> min_cut_ehs;

	//给定一个图中的节点序号，求得它的外围边界边，要分四种情况
	auto fGetTmpBndEhs = [&] (int tmp_idx) -> std::unordered_set<OvmEgH>{
		std::unordered_set<OvmEgH> bnd_ehs;
		if (tmp_idx == S_idx)
			bnd_ehs = s_bnd_ehs;
		else if (tmp_idx == T_idx)
			bnd_ehs = t_bnd_ehs;
		else if (1<= tmp_idx && tmp_idx <= obstacles.size ())
			bnd_ehs = ob_bnd_ehs[tmp_idx];
		else{
			auto nor_fh = n_f_mapping[tmp_idx];
			auto hehs = mesh->face (nor_fh).halfedges ();
			foreach (auto &heh, hehs){
				auto eh = mesh->edge_handle (heh);
				bnd_ehs.insert (eh);
			}
		}
		return bnd_ehs;
	};

	auto fGetSharedEhs = [] (std::unordered_set<OvmEgH> &ehs1, std::unordered_set<OvmEgH> &ehs2)->std::unordered_set<OvmEgH>{
		std::unordered_set<OvmEgH> shared_ehs;
		foreach (auto &eh, ehs1){
			if (contains (ehs2, eh))
				shared_ehs.insert (eh);
		}
		return shared_ehs;
	};

	for (int i = 0; i != min_cut.size (); ++i){
		auto bnd_ehs1 = fGetTmpBndEhs (min_cut[i].first), 
			bnd_ehs2 = fGetTmpBndEhs (min_cut[i].second);

		auto shared_ehs = fGetSharedEhs (bnd_ehs1, bnd_ehs2);
		foreach (auto &eh, shared_ehs)
			min_cut_ehs.insert (eh);
	}
	return min_cut_ehs;
}   

std::unordered_set<OvmEgH> get_quad_mesh_min_cut_new_version (VolumeMesh *mesh, 
	std::unordered_set<OvmFaH> &s_fhs, std::unordered_set<OvmFaH> &t_fhs,
	std::vector<std::unordered_set<OvmFaH> > &obstacles)
{
	auto V_LEVEL = mesh->request_vertex_property<int> ("node level");
	auto V_MAX_LEVEL = mesh->request_mesh_property<int> ("max node level");
	const int INFINITY_LEVEL = std::numeric_limits<int>::max () / 1000;
	const int MAX_LEVEL = V_MAX_LEVEL[0];
	//构建有向图
	std::unordered_set<OvmFaH> normal_fhs;
	for (auto bf_it = mesh->bf_iter (); bf_it; ++bf_it){
		auto fh = *bf_it;
		if (contains (s_fhs, fh) || contains (t_fhs, fh)) continue;
		bool in_ob = false;
		foreach (auto &ob, obstacles){
			if (contains (ob, fh)){
				in_ob = true; break;
			}
		}
		if (!in_ob)
			normal_fhs.insert (fh);
	}

	int N_node = normal_fhs.size () + 2 + obstacles.size ();
	int S_idx = 0, T_idx = N_node - 1;
	Isap<int> graph;graph.init(N_node,6*N_node);

	auto fWhichOb = [&] (OvmFaH test_fh, int &ob_idx)->bool{
		for (int i = 0; i != obstacles.size (); ++i){
			if (contains (obstacles[i], test_fh)){
				ob_idx = i;
				return true;
			}
		}
		return false;
	};

	//0号节点是s节点，最后一个节点是t节点，ob节点排布在1~ ob.size
	std::hash_map<OvmFaH, int> f_n_mapping;
	std::hash_map<int, OvmFaH> n_f_mapping;
	int cur_idx = 1 + obstacles.size ();

	foreach (auto &fh, normal_fhs){
		f_n_mapping.insert (std::make_pair (fh, cur_idx));
		n_f_mapping.insert (std::make_pair (cur_idx, fh));
		cur_idx++;
	}

	auto fGetBndEhs = [&] (std::unordered_set<OvmFaH> &fhs)->std::unordered_set<OvmEgH>{
		std::unordered_set<OvmEgH> bnd_ehs;
		foreach (auto &fh, fhs){
			auto hehs = mesh->face (fh).halfedges ();
			foreach (auto &heh, hehs){
				auto eh = mesh->edge_handle (heh);
				if (contains (bnd_ehs, eh))	bnd_ehs.erase (eh);
				else bnd_ehs.insert (eh);
			}
		}
		return bnd_ehs;
	};

	auto fRateEh = [&] (OvmEgH eh) -> int {
		auto vh1 = mesh->edge (eh).from_vertex (),
			vh2 = mesh->edge (eh).to_vertex ();
		auto vh1_level = V_LEVEL[vh1],
			vh2_level = V_LEVEL[vh2];
		if (vh1_level == 0 && vh2_level == 0)
			return INFINITY_LEVEL;
		return ((MAX_LEVEL - vh1_level) + (MAX_LEVEL - vh2_level)) / 2;
	};

	auto fInsertGraphEdges = [&] (std::unordered_set<OvmEgH> &ehs1, std::unordered_set<OvmEgH> &ehs2,
		int idx1, int idx2, bool mutual){
			int all_level = 0;
			if (ehs1.size () < ehs2.size ()){
				foreach (auto &eh1, ehs1){
					if (contains (ehs2, eh1)){
						auto rate_level = fRateEh (eh1);
						all_level += rate_level;
					}
				}
			}else{
				foreach (auto &eh2, ehs2){
					if (contains (ehs1, eh2)){
						auto rate_level = fRateEh (eh2);
						all_level += rate_level;
					}
				}
			}

			if (all_level > 0){
				//graph.insert_edge (idx1, idx2, all_level);
				graph.add_edge(idx1, idx2, all_level);
				if (mutual)
					//graph.insert_edge (idx2, idx1, all_level);
					graph.add_edge(idx2, idx1, all_level);
			}
	};

	//首先处理s、t和ob之间的连接关系
	auto s_bnd_ehs = fGetBndEhs (s_fhs),
		t_bnd_ehs = fGetBndEhs (t_fhs);

	//s和t之间
	//if (JC::intersects (s_bnd_ehs, t_bnd_ehs))
	//	graph.insert_edge (S_idx, T_idx, 1);

	fInsertGraphEdges (s_bnd_ehs, t_bnd_ehs, S_idx, T_idx, false);

	//s和ob之间 ob和t之间
	std::vector<std::unordered_set<OvmEgH> > ob_bnd_ehs;
	for (int i = 0; i != obstacles.size (); ++i){
		auto one_ob_bnd_ehs = fGetBndEhs (obstacles[i]);
		//if (JC::intersects (s_bnd_ehs, one_ob_bnd_ehs))
		//	graph.insert_edge (S_idx, i, 1);
		//if (JC::intersects (one_ob_bnd_ehs, t_bnd_ehs))
		//	graph.insert_edge (i, T_idx, 1);

		fInsertGraphEdges (s_bnd_ehs, one_ob_bnd_ehs, S_idx, i, false);
		fInsertGraphEdges (one_ob_bnd_ehs, t_bnd_ehs, i, T_idx, false);
		ob_bnd_ehs.push_back (one_ob_bnd_ehs);
	}

	//ob之间
	for (int i = 0; i != obstacles.size () - 1; ++i){
		if (obstacles.size() == 0)
		{
			break;
		}
		auto ob_bnd_ehs_i = ob_bnd_ehs[i];
		for (int j = i + 1; j != obstacles.size (); ++j){
			auto ob_bnd_ehs_j = ob_bnd_ehs[j];
			//if (JC::intersects (ob_bnd_ehs_i, ob_bnd_ehs_j)){
			//	graph.insert_edge (1 + i, 1 + j, 1);
			//	graph.insert_edge (1 + j, 1 + i, 1);
			//}
			fInsertGraphEdges (ob_bnd_ehs_i, ob_bnd_ehs_j, i, j, true);
		}
	}
	//普通节点之间
	foreach (auto &fh, normal_fhs){
		auto fh_idx = f_n_mapping[fh];
		std::unordered_set<OvmFaH> adj_fhs;
		get_adj_boundary_faces_around_face (mesh, fh, adj_fhs);
		std::unordered_set<OvmEgH> adj_ehs;
		get_adj_edges_around_face (mesh, fh, adj_ehs);

		bool adj_to_S = false, adj_to_T = false;
		std::set<int> ob_indices;
		std::set<OvmFaH> nor_fhs;
		foreach (auto &adj_fh, adj_fhs){
			int ob_idx = -1;
			if (contains (s_fhs, adj_fh))
				adj_to_S = true;
			else if (contains (t_fhs, adj_fh))
				adj_to_T = true;
			else if (fWhichOb (adj_fh, ob_idx))
				ob_indices.insert (ob_idx);
			else{

				nor_fhs.insert (adj_fh);
			}
		}
		if (adj_to_S)
			//graph.insert_edge (S_idx, fh_idx, 1);
			fInsertGraphEdges (s_bnd_ehs, adj_ehs, S_idx, fh_idx, false);
		if (adj_to_T)
			//graph.insert_edge (fh_idx, T_idx, 1);
			fInsertGraphEdges (adj_ehs, t_bnd_ehs, fh_idx, T_idx, false);
		foreach (auto &ob_idx, ob_indices){
			//graph.insert_edge (ob_idx, fh_idx, 1);
			//graph.insert_edge (fh_idx, ob_idx, 1);
			fInsertGraphEdges (ob_bnd_ehs[ob_idx], adj_ehs, ob_idx, fh_idx, true);
		}
		//foreach (auto &adj_fh_idx, nor_indices){
		foreach (auto &adj_fh, nor_fhs){
			//graph.insert_edge (fh_idx, adj_fh_idx, 1);
			auto adj_fh_idx = f_n_mapping[adj_fh];
			std::unordered_set<OvmEgH> adj_ehs2;
			get_adj_edges_around_face (mesh, adj_fh, adj_ehs2);
			fInsertGraphEdges (adj_ehs, adj_ehs2, fh_idx, adj_fh_idx, false);
		}
	}
	//下面求最大流最小割
	//QString max_flow_str = QString ("Max flow is %1").arg (graph.ford_fulkerson());
	auto min_cut = graph.min_cut(S_idx,T_idx,N_node);//graph.mincut ();

	std::unordered_set<OvmEgH> min_cut_ehs;

	//给定一个图中的节点序号，求得它的外围边界边，要分四种情况
	auto fGetTmpBndEhs = [&] (int tmp_idx) -> std::unordered_set<OvmEgH>{
		std::unordered_set<OvmEgH> bnd_ehs;
		if (tmp_idx == S_idx)
			bnd_ehs = s_bnd_ehs;
		else if (tmp_idx == T_idx)
			bnd_ehs = t_bnd_ehs;
		else if (1<= tmp_idx && tmp_idx <= obstacles.size ())
			bnd_ehs = ob_bnd_ehs[tmp_idx];
		else{
			auto nor_fh = n_f_mapping[tmp_idx];
			auto hehs = mesh->face (nor_fh).halfedges ();
			foreach (auto &heh, hehs){
				auto eh = mesh->edge_handle (heh);
				bnd_ehs.insert (eh);
			}
		}
		return bnd_ehs;
	};

	auto fGetSharedEhs = [] (std::unordered_set<OvmEgH> &ehs1, std::unordered_set<OvmEgH> &ehs2)->std::unordered_set<OvmEgH>{
		std::unordered_set<OvmEgH> shared_ehs;
		foreach (auto &eh, ehs1){
			if (contains (ehs2, eh))
				shared_ehs.insert (eh);
		}
		return shared_ehs;
	};

	for (int i = 0; i != min_cut.size (); ++i){
		auto bnd_ehs1 = fGetTmpBndEhs (min_cut[i].first), 
			bnd_ehs2 = fGetTmpBndEhs (min_cut[i].second);

		auto shared_ehs = fGetSharedEhs (bnd_ehs1, bnd_ehs2);
		foreach (auto &eh, shared_ehs)
			min_cut_ehs.insert (eh);
	}
	return min_cut_ehs;
}   

std::unordered_set<OvmEgH> get_quad_mesh_min_cut_considering_area(VolumeMesh *mesh,
	std::unordered_set<OvmFaH> &s_fhs, std::unordered_set<OvmFaH> &t_fhs,
	std::vector<std::unordered_set<OvmFaH> > &obstacles)
{
	auto V_LEVEL = mesh->request_vertex_property<int> ("node level");
	auto V_MAX_LEVEL = mesh->request_mesh_property<int> ("max node level");
	const int INFINITY_LEVEL = std::numeric_limits<int>::max () / 1000;
	const int MAX_LEVEL = V_MAX_LEVEL[0];
	//构建有向图
	std::unordered_set<OvmFaH> normal_fhs;
	for (auto bf_it = mesh->bf_iter (); bf_it; ++bf_it){
		auto fh = *bf_it;
		if (contains (s_fhs, fh) || contains (t_fhs, fh)) continue;
		bool in_ob = false;
		foreach (auto &ob, obstacles){
			if (contains (ob, fh)){
				in_ob = true; break;
			}
		}
		if (!in_ob)
			normal_fhs.insert (fh);
	}

	int N_node = normal_fhs.size () + 2 + obstacles.size ();
	int S_idx = 0, T_idx = N_node - 1;
	Isap<double> graph;graph.init(N_node,6*N_node);

	auto fWhichOb = [&] (OvmFaH test_fh, int &ob_idx)->bool{
		for (int i = 0; i != obstacles.size (); ++i){
			if (contains (obstacles[i], test_fh)){
				ob_idx = i;
				return true;
			}
		}
		return false;
	};

	//0号节点是s节点，最后一个节点是t节点，ob节点排布在1~ ob.size
	std::hash_map<OvmFaH, int> f_n_mapping;
	std::hash_map<int, OvmFaH> n_f_mapping;
	int cur_idx = 1 + obstacles.size ();

	foreach (auto &fh, normal_fhs){
		f_n_mapping.insert (std::make_pair (fh, cur_idx));
		n_f_mapping.insert (std::make_pair (cur_idx, fh));
		cur_idx++;
	}

	auto fGetBndEhs = [&] (std::unordered_set<OvmFaH> &fhs)->std::unordered_set<OvmEgH>{
		std::unordered_set<OvmEgH> bnd_ehs;
		foreach (auto &fh, fhs){
			auto hehs = mesh->face (fh).halfedges ();
			foreach (auto &heh, hehs){
				auto eh = mesh->edge_handle (heh);
				if (contains (bnd_ehs, eh))	bnd_ehs.erase (eh);
				else bnd_ehs.insert (eh);
			}
		}
		return bnd_ehs;
	};

	auto fRateEh = [&] (OvmEgH eh) -> int {
		auto vh1 = mesh->edge (eh).from_vertex (),
			vh2 = mesh->edge (eh).to_vertex ();
		auto vh1_level = V_LEVEL[vh1],
			vh2_level = V_LEVEL[vh2];
		if (vh1_level == 0 && vh2_level == 0)
			return INFINITY_LEVEL;
		return ((MAX_LEVEL - vh1_level) + (MAX_LEVEL - vh2_level)) / 2;
	};

	auto fInsertGraphEdges = [&] (std::unordered_set<OvmEgH> &ehs1, std::unordered_set<OvmEgH> &ehs2,
		int idx1, int idx2, double _area, bool mutual){
			int all_level = 0;
			if (ehs1.size () < ehs2.size ()){
				foreach (auto &eh1, ehs1){
					if (contains (ehs2, eh1)){
						auto rate_level = fRateEh (eh1);
						all_level += rate_level;
					}
				}
			}else{
				foreach (auto &eh2, ehs2){
					if (contains (ehs1, eh2)){
						auto rate_level = fRateEh (eh2);
						all_level += rate_level;
					}
				}
			}

			if (all_level > 0){
				//graph.insert_edge (idx1, idx2, all_level);
				graph.add_edge(idx1, idx2, _area);
				if (mutual)
					//graph.insert_edge (idx2, idx1, all_level);
					graph.add_edge(idx2, idx1, _area);
			}
	};
	auto fGetEdgeLength = [&](OvmEgH _eh)->double {
		return (mesh->vertex(mesh->edge(_eh).from_vertex())-mesh->vertex(mesh->edge(_eh).to_vertex())).length();
	};
	//首先处理s、t和ob之间的连接关系
	auto s_bnd_ehs = fGetBndEhs (s_fhs),
		t_bnd_ehs = fGetBndEhs (t_fhs);

	//s和t之间
	//if (JC::intersects (s_bnd_ehs, t_bnd_ehs))
	//	graph.insert_edge (S_idx, T_idx, 1);

	fInsertGraphEdges (s_bnd_ehs, t_bnd_ehs, S_idx, T_idx, 0, false);

	//s和ob之间 ob和t之间
	std::vector<std::unordered_set<OvmEgH> > ob_bnd_ehs;
	for (int i = 0; i != obstacles.size (); ++i){
		auto one_ob_bnd_ehs = fGetBndEhs (obstacles[i]);
		//if (JC::intersects (s_bnd_ehs, one_ob_bnd_ehs))
		//	graph.insert_edge (S_idx, i, 1);
		//if (JC::intersects (one_ob_bnd_ehs, t_bnd_ehs))
		//	graph.insert_edge (i, T_idx, 1);

		fInsertGraphEdges (s_bnd_ehs, one_ob_bnd_ehs, S_idx, i, 0, false);
		fInsertGraphEdges (one_ob_bnd_ehs, t_bnd_ehs, i, T_idx, 0, false);
		ob_bnd_ehs.push_back (one_ob_bnd_ehs);
	}

	//ob之间
	for (int i = 0; i != obstacles.size () - 1; ++i){
		if (obstacles.size() == 0)
		{
			break;
		}
		auto ob_bnd_ehs_i = ob_bnd_ehs[i];
		for (int j = i + 1; j != obstacles.size (); ++j){
			auto ob_bnd_ehs_j = ob_bnd_ehs[j];
			//if (JC::intersects (ob_bnd_ehs_i, ob_bnd_ehs_j)){
			//	graph.insert_edge (1 + i, 1 + j, 1);
			//	graph.insert_edge (1 + j, 1 + i, 1);
			//}
			fInsertGraphEdges (ob_bnd_ehs_i, ob_bnd_ehs_j, i, j, 0,true);
		}
	}
	//普通节点之间
	foreach (auto &fh, normal_fhs){
		auto fh_idx = f_n_mapping[fh];
		std::unordered_set<OvmFaH> adj_fhs;
		get_adj_boundary_faces_around_face (mesh, fh, adj_fhs);
		std::unordered_set<OvmEgH> adj_ehs;
		get_adj_edges_around_face (mesh, fh, adj_ehs);

		bool adj_to_S = false, adj_to_T = false;
		std::set<int> ob_indices;
		std::set<OvmFaH> nor_fhs;
		foreach (auto &adj_fh, adj_fhs){
			int ob_idx = -1;
			if (contains (s_fhs, adj_fh))
				adj_to_S = true;
			else if (contains (t_fhs, adj_fh))
				adj_to_T = true;
			else if (fWhichOb (adj_fh, ob_idx))
				ob_indices.insert (ob_idx);
			else{

				nor_fhs.insert (adj_fh);
			}
		}
		if (adj_to_S)
		{
			auto adj_hehs = mesh->face(fh).halfedges();
			double temp_area = 0;
			foreach (auto heh, adj_hehs)
			{
				OvmEgH current_eh = mesh->edge_handle(heh);
				if (s_bnd_ehs.find(current_eh) != s_bnd_ehs.end())
					temp_area += fGetEdgeLength(current_eh);
			}
			//graph.insert_edge (S_idx, fh_idx, 1);
			fInsertGraphEdges (s_bnd_ehs, adj_ehs, S_idx, fh_idx, temp_area, false);
		}
		if (adj_to_T)
		{
			auto adj_hehs = mesh->face(fh).halfedges();
			double temp_area = 0;
			foreach (auto heh, adj_hehs)
			{
				OvmEgH current_eh = mesh->edge_handle(heh);
				if (t_bnd_ehs.find(current_eh) != t_bnd_ehs.end())
					temp_area += fGetEdgeLength(current_eh);
			}
			//graph.insert_edge (fh_idx, T_idx, 1);
			fInsertGraphEdges (adj_ehs, t_bnd_ehs, fh_idx, T_idx, temp_area, false);
		}
		foreach (auto &ob_idx, ob_indices){
			//graph.insert_edge (ob_idx, fh_idx, 1);
			//graph.insert_edge (fh_idx, ob_idx, 1);
			fInsertGraphEdges (ob_bnd_ehs[ob_idx], adj_ehs, ob_idx, fh_idx, 0, true);
		}
		//foreach (auto &adj_fh_idx, nor_indices){
		foreach (auto &adj_fh, nor_fhs){
			//graph.insert_edge (fh_idx, adj_fh_idx, 1);
			auto adj_fh_idx = f_n_mapping[adj_fh];
			std::unordered_set<OvmEgH> adj_ehs2, _adj_ehs1;
			get_adj_edges_around_face (mesh, adj_fh, adj_ehs2);
			get_adj_edges_around_face (mesh, fh, _adj_ehs1);
			double temp_length = 0;
			foreach (auto eh, adj_ehs2)
			{
				if (_adj_ehs1.find(eh) != _adj_ehs1.end())
					temp_length += fGetEdgeLength(eh);
			}
			fInsertGraphEdges (adj_ehs, adj_ehs2, fh_idx, adj_fh_idx, temp_length, false);
		}
	}
	//下面求最大流最小割
	//QString max_flow_str = QString ("Max flow is %1").arg (graph.ford_fulkerson());
	auto min_cut = graph.min_cut(S_idx,T_idx,N_node);//graph.mincut ();

	std::unordered_set<OvmEgH> min_cut_ehs;

	//给定一个图中的节点序号，求得它的外围边界边，要分四种情况
	auto fGetTmpBndEhs = [&] (int tmp_idx) -> std::unordered_set<OvmEgH>{
		std::unordered_set<OvmEgH> bnd_ehs;
		if (tmp_idx == S_idx)
			bnd_ehs = s_bnd_ehs;
		else if (tmp_idx == T_idx)
			bnd_ehs = t_bnd_ehs;
		else if (1<= tmp_idx && tmp_idx <= obstacles.size ())
			bnd_ehs = ob_bnd_ehs[tmp_idx];
		else{
			auto nor_fh = n_f_mapping[tmp_idx];
			auto hehs = mesh->face (nor_fh).halfedges ();
			foreach (auto &heh, hehs){
				auto eh = mesh->edge_handle (heh);
				bnd_ehs.insert (eh);
			}
		}
		return bnd_ehs;
	};

	auto fGetSharedEhs = [] (std::unordered_set<OvmEgH> &ehs1, std::unordered_set<OvmEgH> &ehs2)->std::unordered_set<OvmEgH>{
		std::unordered_set<OvmEgH> shared_ehs;
		foreach (auto &eh, ehs1){
			if (contains (ehs2, eh))
				shared_ehs.insert (eh);
		}
		return shared_ehs;
	};

	for (int i = 0; i != min_cut.size (); ++i){
		auto bnd_ehs1 = fGetTmpBndEhs (min_cut[i].first), 
			bnd_ehs2 = fGetTmpBndEhs (min_cut[i].second);

		auto shared_ehs = fGetSharedEhs (bnd_ehs1, bnd_ehs2);
		foreach (auto &eh, shared_ehs)
			min_cut_ehs.insert (eh);
	}
	return min_cut_ehs;
}  



void get_source_and_target(VolumeMesh* mesh, std::vector<OvmHaEgH> one_loop,std::unordered_set<OvmEgH> all_loops, 
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs)
{
	std::unordered_set<OvmFaH> s_fhs, t_fhs;
	foreach(auto &heh, one_loop){
		for (auto hehf_it = mesh->hehf_iter (heh); hehf_it; ++hehf_it){
			if(mesh->is_boundary(*hehf_it)){
				//s_chs.insert(mesh->incident_cell(mesh->opposite_halfface_handle(*hehf_it)));
				s_fhs.insert(mesh->face_handle(*hehf_it));
			}
			else if(mesh->is_boundary(mesh->opposite_halfface_handle(*hehf_it))){
				//t_chs.insert(mesh->incident_cell(*hehf_it));
				t_fhs.insert(mesh->face_handle(*hehf_it));
			}
		}
	}

	std::unordered_set<OvmFaH> all_fhs;
	for(auto iter = mesh->faces_begin(); iter != mesh->faces_end(); iter++){
		if(!contains(s_fhs, *iter) && !contains(t_fhs, *iter) && mesh->is_boundary(*iter))
			all_fhs.insert(*iter);
	}

	std::vector<OvmFaH> s_fhs_vec;
	foreach(auto &fh, s_fhs)
		s_fhs_vec.push_back(fh);

	for(int i = 0; i < s_fhs_vec.size(); i++){
		std::unordered_set<OvmFaH> adj_fhs;
		get_adj_boundary_faces_around_face(mesh, s_fhs_vec[i], adj_fhs);
		foreach(auto fh, adj_fhs){
			if(contains(all_fhs, fh)){
				all_fhs.erase(fh);
				s_fhs_vec.push_back(fh);
			}
		}
	}

	foreach(auto &fh, all_fhs)
		t_fhs.insert(fh);

	foreach(auto &fh, s_fhs_vec)
		s_fhs.insert(fh);

	foreach(auto &fh, s_fhs){
		if(mesh->is_boundary(mesh->halfface_handle(fh, 0)))
			s_chs.insert(mesh->incident_cell(mesh->halfface_handle(fh, 1)));
		else
			s_chs.insert(mesh->incident_cell(mesh->halfface_handle(fh, 0)));
	}

	foreach(auto &fh, t_fhs){
		if(mesh->is_boundary(mesh->halfface_handle(fh, 0)))
			t_chs.insert(mesh->incident_cell(mesh->halfface_handle(fh, 1)));
		else
			t_chs.insert(mesh->incident_cell(mesh->halfface_handle(fh, 0)));
	}
}


void get_source_and_target_SC(VolumeMesh* mesh, std::vector<std::vector<OvmHaEgH>> loops,std::unordered_set<OvmFaH>& fhs,
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs)
{
	std::vector<std::unordered_set<OvmFaH>> s_t_fh;
	std::unordered_set<OvmFaH> s_t_fh_set;

	foreach(auto heh_loop, loops){
		std::unordered_set<OvmFaH> s_fh;
		std::unordered_set<OvmFaH> t_fh;

		foreach(auto heh, heh_loop){
			for (auto hehf_it = mesh->hehf_iter (heh); hehf_it; ++hehf_it){
				if(mesh->is_boundary(*hehf_it)){
					s_fh.insert(mesh->face_handle(*hehf_it));
				}
				else if(mesh->is_boundary(mesh->opposite_halfface_handle(*hehf_it))){
					t_fh.insert(mesh->face_handle(*hehf_it));
				}
			}
		}

		s_t_fh.push_back(s_fh);
		s_t_fh.push_back(t_fh);
		s_t_fh_set.insert(s_fh.begin(), s_fh.end());
		s_t_fh_set.insert(t_fh.begin(), t_fh.end());
	}

	std::unordered_set<OvmFaH> all_fh;
	for (auto fh_it = mesh->faces_begin(); fh_it != mesh->faces_end(); ++fh_it)
	{
		if (mesh->is_boundary(*fh_it) && !contains(s_t_fh_set, *fh_it))
			all_fh.insert(*fh_it);
	}

	//std::cout<<"all fh"<<all_fh.size()<<endl;




	std::unordered_set<OvmFaH> s_fhs;
	std::unordered_set<OvmFaH> t_fhs;
	OvmFaH fh0 = *all_fh.begin();
	std::vector<OvmFaH> s_fhs_vec;
	s_fhs_vec.push_back(fh0);
	all_fh.erase(fh0);

	for(int i = 0; i < s_fhs_vec.size(); i++){
		std::unordered_set<OvmFaH> adj_fhs;
		get_adj_boundary_faces_around_face(mesh, s_fhs_vec[i], adj_fhs);
		foreach(auto fh, adj_fhs){
			if(contains(all_fh, fh)){
				all_fh.erase(fh);
				s_fhs_vec.push_back(fh);
			}
		}
	}

	foreach(auto &fh, all_fh)
		t_fhs.insert(fh);

	foreach(auto &fh, s_fhs_vec)
		s_fhs.insert(fh);





	for (int i = 0; i < s_t_fh.size(); i = i+2)
	{
		std::unordered_set<OvmFaH>& sOrt_fh = s_t_fh[i];
		bool is_sf = false;
		foreach(auto fh, sOrt_fh){
			std::unordered_set<OvmFaH> adj_bou_fhs;
			get_adj_boundary_faces_around_face(mesh,fh,adj_bou_fhs);
			foreach(auto temp_fh, adj_bou_fhs){
				if(contains(s_fhs, temp_fh)){
					is_sf = true;
					break;
				}
			}
			if(is_sf)
				break;
		}

		if(is_sf){
			s_fhs.insert(s_t_fh[i].begin(), s_t_fh[i].end());
			t_fhs.insert(s_t_fh[i+1].begin(), s_t_fh[i+1].end());
		}
		else
		{
			t_fhs.insert(s_t_fh[i].begin(), s_t_fh[i].end());
			s_fhs.insert(s_t_fh[i+1].begin(), s_t_fh[i+1].end());
		}
	}

	foreach(auto &fh, s_fhs){
		if(mesh->is_boundary(mesh->halfface_handle(fh, 0)))
			s_chs.insert(mesh->incident_cell(mesh->halfface_handle(fh, 1)));
		else
			s_chs.insert(mesh->incident_cell(mesh->halfface_handle(fh, 0)));

	}

	foreach(auto &fh, t_fhs){
		if(mesh->is_boundary(mesh->halfface_handle(fh, 0)))
			t_chs.insert(mesh->incident_cell(mesh->halfface_handle(fh, 1)));
		else
			t_chs.insert(mesh->incident_cell(mesh->halfface_handle(fh, 0)));

	}
}


void get_source_and_target_on_boundary (VolumeMesh* mesh, std::vector<std::vector<OvmHaEgH>> loops,std::unordered_set<OvmFaH>& fhs,
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs)
{
	//Step 0 初始化  包括是否遍历过 以及 将边界的面分组 以及建立一个直接与dual loops相邻的面集
	std::vector<bool> if_visited, if_adj_dl1,if_adj_dl2;
	if_visited.resize(mesh->n_faces());if_adj_dl1.resize(mesh->n_faces());if_adj_dl2.resize(mesh->n_faces());
	int residual_num = 0;
	for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
	{
		if_visited[f_it->idx()] = (mesh->is_boundary(*f_it)) ? false : true;
		if (mesh->is_boundary(*f_it))
		{
			residual_num++;
		}
		if_adj_dl1[f_it->idx()] = false;
		if_adj_dl2[f_it->idx()] = false;
	}
	for (auto he_it = loops[0].begin();he_it != loops[0].end();he_it++)
	{
		for (auto hehf_it = mesh->hehf_iter(*he_it);hehf_it;hehf_it++)
		{
			OvmFaH cfh = mesh->face_handle(*hehf_it);
			if (mesh->is_boundary(cfh))
			{
				if_adj_dl1[cfh.idx()] = true;
			}
		}
	}
	for (auto he_it = loops[1].begin();he_it != loops[1].end();he_it++)
	{
		for (auto hehf_it = mesh->hehf_iter(*he_it);hehf_it;hehf_it++)
		{
			OvmFaH cfh = mesh->face_handle(*hehf_it);
			if (mesh->is_boundary(cfh))
			{
				if_adj_dl2[cfh.idx()] = true;
			}
		}
	}
	std::vector<std::unordered_set<OvmFaH>> d1_adj_fhs,d2_adj_fhs,both_adj_fhs;
	//Step 1 利用随机寻找没有遍历过的边界面做宽度优先算法，并记录其与对偶环相接触情况
	do 
	{
		if (residual_num == 0)
		{
			break;
		}
		OvmFaH seed_fh;
		for (auto bf_it = mesh->bf_iter();bf_it;bf_it++)
		{
			if (!if_visited[bf_it->idx()])
			{
				seed_fh = *bf_it;
				break;
			}
		}
		std::unordered_set<OvmFaH> fhs_group;
		bool ifadjd1 = false;bool ifadjd2 = false;
		std::queue<OvmFaH> q;
		q.push(seed_fh);
		while(!q.empty())
		{
			OvmFaH q_top = q.front();
			q.pop();
			fhs_group.insert(q_top);
			OvmHaFaH current_bhfh = (mesh->is_boundary(mesh->halfface_handle(q_top,0))) ? mesh->halfface_handle(q_top,0) : mesh->halfface_handle(q_top,1);
			for (auto hfhf_it = mesh->bhfhf_iter(current_bhfh);hfhf_it;hfhf_it++)
			{
				OvmFaH adj_fh = mesh->face_handle(*hfhf_it).idx();
				if (!if_visited[adj_fh.idx()])
				{
					if_visited[adj_fh.idx()] = true;
					if (if_adj_dl1[adj_fh.idx()])
					{
						fhs_group.insert(adj_fh);
						ifadjd1 = true;
					}
					else if (if_adj_dl2[adj_fh.idx()])
					{
						fhs_group.insert(adj_fh);
						ifadjd2 = true;
					}
					else
					{
						q.push(adj_fh);
					}
				}
			}
		}
		if (ifadjd1 && ifadjd2)
		{
			both_adj_fhs.push_back(fhs_group);
		}
		else if (ifadjd1)
		{
			d1_adj_fhs.push_back(fhs_group);
		}
		else
		{
			d2_adj_fhs.push_back(fhs_group);
		}
		residual_num -= fhs_group.size();
		std::unordered_set<OvmFaH>().swap(fhs_group);
	} while (true);
	std::unordered_set<OvmCeH>().swap(s_chs);std::unordered_set<OvmCeH>().swap(t_chs);
	for (auto b_it = both_adj_fhs.begin();b_it != both_adj_fhs.end();b_it++)
	{
		for (auto fh_it = b_it->begin();fh_it != b_it->end();fh_it++)
		{
			OvmCeH inc_cell = (mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) != mesh->InvalidCellHandle) ? mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) : mesh->incident_cell(mesh->halfface_handle(*fh_it,1));
			s_chs.insert(inc_cell);
		}
	}
	for (auto d1_it = d1_adj_fhs.begin();d1_it != d1_adj_fhs.end();d1_it++)
	{
		for (auto fh_it = d1_it->begin();fh_it != d1_it->end();fh_it++)
		{
			OvmCeH inc_cell = (mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) != mesh->InvalidCellHandle) ? mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) : mesh->incident_cell(mesh->halfface_handle(*fh_it,1));
			t_chs.insert(inc_cell);
		}
	}
	for (auto d1_it = d2_adj_fhs.begin();d1_it != d2_adj_fhs.end();d1_it++)
	{
		for (auto fh_it = d1_it->begin();fh_it != d1_it->end();fh_it++)
		{
			OvmCeH inc_cell = (mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) != mesh->InvalidCellHandle) ? mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) : mesh->incident_cell(mesh->halfface_handle(*fh_it,1));
			t_chs.insert(inc_cell);
		}
	}
}





void get_boundary_source_and_target (VolumeMesh* mesh, std::vector<std::vector<OvmHaEgH>> loops,std::unordered_set<OvmFaH>& fhs,
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs)
{
	std::unordered_set<OvmCeH>().swap(s_chs);
	std::unordered_set<OvmCeH>().swap(t_chs);
	if (loops.size() == 2)
	{
		std::vector<bool> sehs1(mesh->n_edges(),false);
		std::vector<bool> sehs2(mesh->n_edges(),false);
		std::vector<bool> if_visited(mesh->n_faces(),true);
		int residual_num = 0;
		for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
		{
			if (mesh->is_boundary(*f_it))
			{
				residual_num++;
				if_visited[f_it->idx()] = false;
			}
		}
		for (auto he_it = loops[0].begin();he_it != loops[0].end();he_it++)
		{
			sehs1[mesh->edge_handle(*he_it)] = true;
		}
		for (auto he_it = loops[1].begin();he_it != loops[1].end();he_it++)
		{
			sehs2[mesh->edge_handle(*he_it)] = true;
		}

		std::vector<std::unordered_set<OvmFaH>> adj_line1,adj_line2,adj_both;
		do 
		{
			bool if_adj1 = false;bool if_adj2 = false;
			if (residual_num == 0)
			{
				break;
			}
			OvmFaH seed_fh;
			for (auto bf_it = mesh->bf_iter();bf_it;bf_it++)
			{
				if (!if_visited[bf_it->idx()])
				{
					seed_fh = *bf_it;
					break;
				}
			}
			std::unordered_set<OvmFaH> fhs_group;
			std::queue<OvmFaH> q;
			q.push(seed_fh);
			while(!q.empty())
			{
				OvmFaH q_top = q.front();
				q.pop();
				fhs_group.insert(q_top);
				OvmHaFaH current_bhfh = (mesh->is_boundary(mesh->halfface_handle(q_top,0))) ? mesh->halfface_handle(q_top,0) : mesh->halfface_handle(q_top,1);
				for (auto hfhf_it = mesh->bhfhf_iter(current_bhfh);hfhf_it;hfhf_it++)
				{
					OvmFaH current_fh = mesh->face_handle(*hfhf_it);

					OvmEgH common_eh = get_common_edge_handle(mesh,q_top,current_fh);
					if (sehs1[common_eh.idx()] || sehs2[common_eh.idx()])
					{
						if (sehs1[common_eh.idx()])
						{
							if_adj1 = true;
						}
						if (sehs2[common_eh.idx()])
						{
							if_adj2 = true;
						}
						continue;
					}
					if (if_visited[current_fh.idx()])
					{
						continue;
					}
					if_visited[current_fh.idx()] = true;
					q.push(current_fh);
				}
			}
			//std::cout<<"adj "<<if_adj1<<" "<<if_adj2<<std::endl;
			if (if_adj1 && if_adj2)
			{
				adj_both.push_back(fhs_group);
			}
			else if (if_adj1)
			{
				adj_line1.push_back(fhs_group);
			}
			else
			{
				//fhs = fhs_group;
				//return;
				adj_line2.push_back(fhs_group);
			}
			residual_num -= fhs_group.size();
			std::unordered_set<OvmFaH>().swap(fhs_group);
		} while (true);

		if ((adj_both.size() + adj_line1.size() + adj_line2.size()) == 3)
		{
			for (auto fh_it = adj_both[0].begin();fh_it != adj_both[0].end();fh_it++)
			{
				OvmCeH inc_cell = (mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) != mesh->InvalidCellHandle) ? mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) : mesh->incident_cell(mesh->halfface_handle(*fh_it,1));
				s_chs.insert(inc_cell);
			}
			for (auto fh_it = adj_line1[0].begin();fh_it != adj_line1[0].end();fh_it++)
			{
				OvmCeH inc_cell = (mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) != mesh->InvalidCellHandle) ? mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) : mesh->incident_cell(mesh->halfface_handle(*fh_it,1));
				t_chs.insert(inc_cell);
			}
			for (auto fh_it = adj_line2[0].begin();fh_it != adj_line2[0].end();fh_it++)
			{
				OvmCeH inc_cell = (mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) != mesh->InvalidCellHandle) ? mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) : mesh->incident_cell(mesh->halfface_handle(*fh_it,1));
				t_chs.insert(inc_cell);
			}
		}
		else
		{
			for (auto fh_it = adj_both[0].begin();fh_it != adj_both[0].end();fh_it++)
			{
				OvmCeH inc_cell = (mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) != mesh->InvalidCellHandle) ? mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) : mesh->incident_cell(mesh->halfface_handle(*fh_it,1));
				s_chs.insert(inc_cell);
			}
			for (auto fh_it = adj_both[1].begin();fh_it != adj_both[1].end();fh_it++)
			{
				OvmCeH inc_cell = (mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) != mesh->InvalidCellHandle) ? mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) : mesh->incident_cell(mesh->halfface_handle(*fh_it,1));
				t_chs.insert(inc_cell);
			}
		}

	}
	else
	{
		std::vector<bool> sehs(mesh->n_edges(),false);
		std::vector<bool> if_visited(mesh->n_faces(),true);
		int residual_num = 0;
		for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
		{
			if (mesh->is_boundary(*f_it))
			{
				residual_num++;
				if_visited[f_it->idx()] = false;
			}
		}
		for (int lid = 0;lid < loops.size();lid++)
		{
			for (auto he_it = loops[lid].begin();he_it != loops[lid].end();he_it++)
			{
				sehs[mesh->edge_handle(*he_it)] = true;
			}
		}
		std::vector<std::unordered_set<OvmFaH>> separated_faces;
		do 
		{
			if (residual_num == 0)
			{
				break;
			}
			OvmFaH seed_fh;
			for (auto bf_it = mesh->bf_iter();bf_it;bf_it++)
			{
				if (!if_visited[bf_it->idx()])
				{
					seed_fh = *bf_it;
					break;
				}
			}
			std::unordered_set<OvmFaH> fhs_group;
			std::queue<OvmFaH> q;
			q.push(seed_fh);
			while(!q.empty())
			{
				OvmFaH q_top = q.front();
				q.pop();
				fhs_group.insert(q_top);
				OvmHaFaH current_bhfh = (mesh->is_boundary(mesh->halfface_handle(q_top,0))) ? mesh->halfface_handle(q_top,0) : mesh->halfface_handle(q_top,1);
				for (auto hfhf_it = mesh->bhfhf_iter(current_bhfh);hfhf_it;hfhf_it++)
				{
					OvmFaH current_fh = mesh->face_handle(*hfhf_it);
					if (if_visited[current_fh.idx()])
					{
						continue;
					}
					OvmEgH common_eh = get_common_edge_handle(mesh,q_top,current_fh);
					if (sehs[common_eh.idx()])
					{
						continue;
					}
					if_visited[current_fh.idx()] = true;
					q.push(current_fh);
				}
			}
			separated_faces.push_back(fhs_group);
			residual_num -= fhs_group.size();
			std::unordered_set<OvmFaH>().swap(fhs_group);
		} while (true);
		if (separated_faces.size() == 2)
		{
			for (auto fh_it = separated_faces[0].begin();fh_it != separated_faces[0].end();fh_it++)
			{
				OvmCeH inc_cell = (mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) != mesh->InvalidCellHandle) ? mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) : mesh->incident_cell(mesh->halfface_handle(*fh_it,1));
				s_chs.insert(inc_cell);
			}
			for (auto fh_it = separated_faces[1].begin();fh_it != separated_faces[1].end();fh_it++)
			{
				OvmCeH inc_cell = (mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) != mesh->InvalidCellHandle) ? mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) : mesh->incident_cell(mesh->halfface_handle(*fh_it,1));
				t_chs.insert(inc_cell);
			}
			std::vector<std::unordered_set<OvmFaH>>().swap(separated_faces);
		}
		else
		{
			std::cout<<"s t function wrong!"<<std::endl;
			system("pause");
		}
	}
}


void get_boundary_source_and_target (VolumeMesh* mesh, std::vector<OvmEgH> ehs, DualSheet* sheet, std::unordered_set<OvmCeH>& s_chs, std::unordered_set<OvmCeH>& t_chs, HoopsView* hoopsview)
{
	std::vector<bool> if_visited(mesh->n_faces(),true);
	int residual_num = 0;
	std::vector<bool> sehs(mesh->n_edges(), false);
	std::vector<bool> if_sheet_cells(mesh->n_cells(), false);

	std::vector<std::unordered_set<OvmFaH>> separated_faces;

	auto fInitialization = [&](){
		//std::cout<<__LINE__<<std::endl;
		std::unordered_set<OvmCeH>().swap(s_chs);
		std::unordered_set<OvmCeH>().swap(t_chs);
		for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
		{
			if (mesh->is_boundary(*f_it))
			{
				residual_num++;
				if_visited[f_it->idx()] =false;
			}
		}
		foreach (auto eh, ehs)
			sehs[eh.idx()] = true;
		//if (sheet->chs.size() == 0)
		//{
		//	std::cout<<"Sheet chs is empty!"<<std::endl;
		//	system("pause");
		//}
		if (sheet == NULL)
		{
			return;
		}
		foreach (auto ch, sheet->chs)
		{
			if_sheet_cells[ch.idx()] = true;
			auto adj_hfhs = mesh->cell(ch).halffaces();
			foreach (auto hfh, adj_hfhs)
			{
				OvmFaH cfh = mesh->face_handle(hfh);
				if (mesh->is_boundary(cfh))
				{
					if (if_visited[cfh.idx()] == false)
						residual_num--;
					if_visited[cfh.idx()] = true;
					auto adj_hehs = mesh->face(cfh).halfedges();
					foreach (auto heh, adj_hehs)
					{
						sehs[mesh->edge_handle(heh).idx()] = true;
					}
				}
			}
		}
		
	};

	auto fProcess = [&](){
		std::vector<std::unordered_set<OvmFaH>> separated_faces;
		do 
		{
			if (residual_num == 0)
			{
				break;
			}
			OvmFaH seed_fh;
			for (auto bf_it = mesh->bf_iter();bf_it;bf_it++)
			{
				if (!if_visited[bf_it->idx()])
				{
					seed_fh = *bf_it;
					break;
				}
			}
			std::unordered_set<OvmFaH> fhs_group;
			std::queue<OvmFaH> q;
			q.push(seed_fh);
			while(!q.empty())
			{
				OvmFaH q_top = q.front();
				q.pop();
				fhs_group.insert(q_top);
				OvmHaFaH current_bhfh = (mesh->is_boundary(mesh->halfface_handle(q_top,0))) ? mesh->halfface_handle(q_top,0) : mesh->halfface_handle(q_top,1);
				for (auto hfhf_it = mesh->bhfhf_iter(current_bhfh);hfhf_it;hfhf_it++)
				{
					OvmFaH current_fh = mesh->face_handle(*hfhf_it);
					if (if_visited[current_fh.idx()])
					{
						continue;
					}
					OvmEgH common_eh = get_common_edge_handle(mesh,q_top,current_fh);
					if (sehs[common_eh.idx()])
					{
						continue;
					}
					if_visited[current_fh.idx()] = true;
					q.push(current_fh);
				}
			}
			separated_faces.push_back(fhs_group);
			residual_num -= fhs_group.size();
			std::unordered_set<OvmFaH>().swap(fhs_group);
		} while (true);
		
		std::cout<<"separated faces size is "<<separated_faces.size()<<std::endl;
		for (int si = 0; si < separated_faces.size();si++)
		{
			auto newg = new VolumeMeshElementGroup(mesh);
			newg->fhs = separated_faces[si];
			if (hoopsview != NULL)
				hoopsview->render_mesh_group(newg, false,"red","red","red");
		}
		std::cout<<std::endl;
		if (separated_faces.size() == 2)
		{
			for (auto fh_it = separated_faces[0].begin();fh_it != separated_faces[0].end();fh_it++)
			{
				OvmCeH inc_cell = (mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) != mesh->InvalidCellHandle) ? mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) : mesh->incident_cell(mesh->halfface_handle(*fh_it,1));
				if (!if_sheet_cells[inc_cell.idx()])
					s_chs.insert(inc_cell);
			}
			for (auto fh_it = separated_faces[1].begin();fh_it != separated_faces[1].end();fh_it++)
			{
				OvmCeH inc_cell = (mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) != mesh->InvalidCellHandle) ? mesh->incident_cell(mesh->halfface_handle(*fh_it,0)) : mesh->incident_cell(mesh->halfface_handle(*fh_it,1));
				if (!if_sheet_cells[inc_cell.idx()])
					t_chs.insert(inc_cell);
			}
			std::vector<std::unordered_set<OvmFaH>>().swap(separated_faces);
		}
		else
		{
			std::cout<<"s t function wrong!"<<std::endl;
			system("pause");
		}
	};

	fInitialization();
	fProcess();
}

std::unordered_set<OvmFaH> get_volume_mesh_min_cut_considering_area_and_sheet (VolumeMesh *mesh, 
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs, std::unordered_set<OvmCeH> excluded_cells)

{
	clock_t time1 = clock();
	//double alpha = 100;
	std::vector<std::unordered_set<OvmVeH> > layered_vhs;
	rate_inner_vhs (mesh, s_chs, t_chs, layered_vhs);
	auto V_LEVEL = mesh->request_vertex_property<int> ("node level");
	auto V_MAX_LEVEL = mesh->request_mesh_property<int> ("max node level");
	const int INFINITY_LEVEL = std::numeric_limits<int>::max () / 1000;
	const int MAX_LEVEL = V_MAX_LEVEL[0];
	//建立网格面句柄和无向图中节点序号之间的映射关系
	std::hash_map<OvmCeH, int> c_n_mapping;
	std::hash_map<int, OvmCeH> n_c_mapping;
	std::unordered_set<OvmCeH> normal_chs;
	for (auto c_it = mesh->cells_begin (); c_it != mesh->cells_end (); ++c_it){
		auto ch = *c_it;
		if (s_chs.find(ch) == s_chs.end() && t_chs.find(ch) == t_chs.end()){
			normal_chs.insert (ch);
			int cur_idx = normal_chs.size ();
			c_n_mapping.insert (std::make_pair(ch, cur_idx));
			n_c_mapping.insert (std::make_pair(cur_idx, ch));
		}
	}
	//构建有向图
	int N_node = normal_chs.size () + 2;
	int S_idx = 0, T_idx = N_node - 1;
	Isap<double> graph;
	//graph.init(N_node,6*N_node);
	graph.init(N_node,6*N_node);
	std::hash_map<OvmFaH,int> fRateFh;
	for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
	{
		int all_vh_level = 0;int adj_num = 0;
		for (auto hfv_it = mesh->hfv_iter(mesh->halfface_handle(*f_it,0));hfv_it;hfv_it++)
		{
			all_vh_level += MAX_LEVEL - V_LEVEL[*hfv_it];
			adj_num++;
		}
		fRateFh[*f_it] = all_vh_level /adj_num * 4; 
	}
	auto fInsertGraphEdges = [&] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2,
		int idx1, int idx2,double _area, bool if_double){
			int all_level = 0;
			if (fhs1.size () < fhs2.size ()){
				foreach (auto &fh1, fhs1){
					if (fhs2.find(fh1) != fhs2.end()){
						auto rate_level = fRateFh[fh1];
						all_level += rate_level;
					}
				}
			}else{
				foreach (auto &fh2, fhs2){
					if (fhs1.find(fh2) != fhs1.end()){
						auto rate_level = fRateFh[fh2];
						all_level += rate_level;
					}
				}
			}
			if (all_level > 0){
				//std::cout<<__LINE__<<idx1<<" "<<idx2<<" "<<_area<<std::endl;
				graph.add_edge (idx1, idx2, /*all_level+*/alpha*_area);
				//graph.add_edge(idx1, idx2, _area);
				if (if_double)
				{
					//graph.add_edge(idx2, idx1, _area);
					graph.add_edge(idx2,idx1,/*all_level+*/alpha*_area);
				}
			}
	};
	auto fComputeQuadArea = [&](OvmFaH fh) ->double{
		auto adj_hehs = mesh->face(fh).halfedges();
		OvmVec3d dire[4];
		for (int hei = 0;hei < 4;hei++)
		{
			dire[hei] = (mesh->vertex(mesh->halfedge(adj_hehs[hei]).to_vertex()) - mesh->vertex(mesh->halfedge(adj_hehs[hei]).from_vertex()));
		}
		OvmVec3d cr1 = cross(dire[0], dire[1]);
		OvmVec3d cr2 = cross(dire[2], dire[3]);
		double ans = (dot(cr1,cr2) > 0) ? cr1.length() + cr2.length() : cr1.length() - cr2.length();
		ans = abs(ans)/2.0;
		return ans;
	};

	//连接s和t
	std::unordered_set<OvmFaH> s_bnd_fhs, t_bnd_fhs;
	collect_boundary_element (mesh, s_chs, NULL, NULL, &s_bnd_fhs);
	collect_boundary_element (mesh, t_chs, NULL, NULL, &t_bnd_fhs);
	double _temp_area;
	_temp_area = 0;
	for (auto sf_it = s_bnd_fhs.begin();sf_it != s_bnd_fhs.end();sf_it++)
	{
		if (t_bnd_fhs.find(*sf_it) != t_bnd_fhs.end())
		{
			_temp_area += fComputeQuadArea(*sf_it);
		}
	}
	fInsertGraphEdges (s_bnd_fhs, t_bnd_fhs, S_idx, T_idx,_temp_area,false);
	for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
	{
		//std::cout<<f_it->idx()<<std::endl;
		_temp_area = fComputeQuadArea(*f_it);
		OvmCeH ch1 = mesh->incident_cell(mesh->halfface_handle(*f_it,0));
		OvmCeH ch2 = mesh->incident_cell(mesh->halfface_handle(*f_it,1));
		if (excluded_cells.find(ch1) != excluded_cells.end() || excluded_cells.find(ch2) != excluded_cells.end())
			continue;
		//排除ch1 ch2都为 s_ch t_ch 集合中的元素
		if ((s_chs.find(ch1) != s_chs.end() || t_chs.find(ch1) != t_chs.end()) && (s_chs.find(ch2) != s_chs.end() || t_chs.find(ch2) != t_chs.end()))
		{
			continue;
		}
		else if (s_chs.find(ch1) != s_chs.end())
		{
			std::unordered_set<OvmFaH> adj_fhs;
			get_adj_faces_around_hexa(mesh, ch2,adj_fhs);
			fInsertGraphEdges (s_bnd_fhs, adj_fhs, S_idx, c_n_mapping[ch2],_temp_area,false);
		}
		else if (t_chs.find(ch1) != t_chs.end())
		{
			std::unordered_set<OvmFaH> adj_fhs;
			get_adj_faces_around_hexa(mesh, ch2, adj_fhs);
			fInsertGraphEdges (adj_fhs, t_bnd_fhs, c_n_mapping[ch2], T_idx,_temp_area,false);
		}
		else if (s_chs.find(ch2) != s_chs.end())
		{
			std::unordered_set<OvmFaH> adj_fhs;
			get_adj_faces_around_hexa(mesh, ch1, adj_fhs);
			fInsertGraphEdges (s_bnd_fhs, adj_fhs, S_idx, c_n_mapping[ch1],_temp_area,false);
		}
		else if (t_chs.find(ch2) != t_chs.end())
		{
			std::unordered_set<OvmFaH> adj_fhs;
			get_adj_faces_around_hexa(mesh, ch1, adj_fhs);
			fInsertGraphEdges (adj_fhs, t_bnd_fhs, c_n_mapping[ch1], T_idx,_temp_area,false);
		}
		else
		{
			std::unordered_set<OvmFaH> adj_fhs1,adj_fhs2;
			get_adj_faces_around_hexa(mesh, ch1, adj_fhs1);
			get_adj_faces_around_hexa(mesh, ch2, adj_fhs2);
			fInsertGraphEdges(adj_fhs1,adj_fhs2,c_n_mapping[ch1],c_n_mapping[ch2],_temp_area,true);
			//fInsertGraphEdges_double (adj_fhs1,adj_fhs2,c_n_mapping[ch1],c_n_mapping[ch2]);
		}
	}
	//QString max_flow_str = QString ("Max flow is %1").arg (graph.ford_fulkerson());
	auto min_cut = graph.min_cut(S_idx,T_idx,N_node);
	//给定一个图中的节点序号，求得它的外围面，要分四种情况
	auto fGetTmpBndFhs = [&] (int tmp_idx) -> std::unordered_set<OvmFaH>{
		std::unordered_set<OvmFaH> bnd_fhs;
		if (tmp_idx == S_idx)
			bnd_fhs = s_bnd_fhs;
		else if (tmp_idx == T_idx)
			bnd_fhs = t_bnd_fhs;
		else{
			auto nor_ch = n_c_mapping[tmp_idx];
			get_adj_faces_around_hexa (mesh, nor_ch, bnd_fhs);
		}
		return bnd_fhs;
	};

	auto fGetSharedFhs = [] (std::unordered_set<OvmFaH> &fhs1, std::unordered_set<OvmFaH> &fhs2)->std::unordered_set<OvmFaH>{
		std::unordered_set<OvmFaH> shared_fhs;
		foreach (auto &fh, fhs1){
			if (fhs2.find(fh) != fhs2.end()/*contains (fhs2, fh)*/)
				shared_fhs.insert (fh);
		}
		return shared_fhs;
	};
	std::unordered_set<OvmFaH> min_cut_fhs;
	for (int i = 0; i != min_cut.size (); ++i){
		auto bnd_fhs1 = fGetTmpBndFhs (min_cut[i].first), 
			bnd_fhs2 = fGetTmpBndFhs (min_cut[i].second);

		auto shared_fhs = fGetSharedFhs (bnd_fhs1, bnd_fhs2);
		foreach (auto &fh, shared_fhs)
			min_cut_fhs.insert (fh);
	}
	clock_t time2 = clock();
	double tcost22 = (double)(time2- time1)/CLOCKS_PER_SEC;
	std::cout<<"Mesh size is Cell "<<mesh->n_cells()<<"Time cost is "<<tcost22<<std::endl;

	//std::cout<<__LINE__<<std::endl;
	//get_common_face_handle()
	return min_cut_fhs;
}