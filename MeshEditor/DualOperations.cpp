#include "stdafx.h"
#include "DualOperations.h"
#include "topologyoptwidget.h"
#include "PrioritySetManager.h"
#include <cmath>
#include <geom_utl.hxx>
#include <OpenVolumeMesh/Attribs/StatusAttrib.hh>
#include <OpenVolumeMesh/Core/TopologyKernel.hh>
#include <fstream>
#include <edge.hxx>
#include <plane.hxx>
#include <face.hxx>
#include <surface.hxx>
#include <unitvec.hxx>
#include <stack>
#include <OpenVolumeMesh/FileManager/FileManager.hh>
#define PI 3.1415926
#define P_A 1
#define P_B 0
#define P_C 0
#define P_D 0

void retrieve_one_sheet(VolumeMesh *mesh, OvmEgH start_eh, std::unordered_set<OvmEgH> &sheet_ehs, std::unordered_set<OvmCeH> &sheet_chs)
{
	auto E_VISITED = mesh->request_edge_property<bool>("visied", false);
	std::queue<OvmHaEgH> spread_set;
	OvmHaEgH start_heh = mesh->halfedge_handle(start_eh, 0);
	sheet_ehs.insert(start_eh);
	E_VISITED[start_eh] = true;
	spread_set.push(start_heh);
	while (!spread_set.empty()){
		start_heh = spread_set.front();
		spread_set.pop();
		for (auto hehf_iter = mesh->hehf_iter(start_heh); hehf_iter; ++hehf_iter){
			OvmHaFaH hfh = *hehf_iter;
			OvmHaEgH oppo_heh = mesh->prev_halfedge_in_halfface(
				mesh->prev_halfedge_in_halfface(start_heh, hfh), hfh);
			OvmEgH eh = mesh->edge_handle(oppo_heh);
			if (eh != mesh->InvalidEdgeHandle)
				sheet_ehs.insert(eh);
			OvmCeH ch = mesh->incident_cell(hfh);
			if (ch != mesh->InvalidCellHandle)
				sheet_chs.insert(mesh->incident_cell(hfh));
			if (!E_VISITED[eh]){
				E_VISITED[eh] = true;
				spread_set.push(oppo_heh);
			}
		}//end for (auto hehf_iter = mesh->hehf_iter (start_heh); hehf_iter; ++hehf_iter){...
	}//end while (!spread_set.empty ()){...
}

void retrieve_sheets (VolumeMesh *mesh, SheetSet &sheet_set)
{
	sheet_set.clear ();
	if (!mesh->edge_property_exists<unsigned long> ("sheetptr")){
		auto prop = mesh->request_edge_property<unsigned long> ("sheetptr", 0);
		mesh->set_persistent (prop);
	}
	auto E_SHEET_PTR = mesh->request_edge_property<unsigned long> ("sheetptr");
	auto E_VISITED = mesh->request_edge_property<bool> ("visied", false);

	int sheet_count = 0;
	for (auto e_it = mesh->edges_begin (); e_it != mesh->edges_end (); ++e_it){
		if (E_VISITED[*e_it]) continue;

		auto seed_eh = *e_it;
		std::unordered_set<OvmEgH> sheet_ehs;
		std::unordered_set<OvmCeH> sheet_chs;
		retrieve_one_sheet (mesh, seed_eh, sheet_ehs, sheet_chs);
		DualSheet *sheet = NULL;
		if (E_SHEET_PTR[seed_eh]){
			sheet = (DualSheet *)(E_SHEET_PTR[seed_eh]);
		}else{
			sheet = new DualSheet (mesh);
		}

		sheet->ehs = sheet_ehs; sheet->chs = sheet_chs; 
		foreach (auto &eh, sheet_ehs)
			E_SHEET_PTR[eh] = (unsigned long)sheet;

		sheet_set.insert (sheet);
	}
}

//判定sheet是否可以进行extraction操作,是否会导致几何退化
bool is_sheet_can_be_extracted (VolumeMesh *mesh, DualSheet *sheet)
{
	assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

	//判断一下哪些情况是无法处理的
	foreach (auto &eh, sheet->ehs){
		auto vh1 = mesh->edge (eh).from_vertex (),
			vh2 = mesh->edge (eh).to_vertex ();
		auto entity1 = (ENTITY*)(V_ENTITY_PTR[vh1]),
			entity2 = (ENTITY*)(V_ENTITY_PTR[vh2]);
		//如果两个顶点都在几何点上是不行的
		if (is_VERTEX (entity1) && is_VERTEX (entity2))
			return false;
		//如果两个顶点都在几何边上，但是所在几何边又不同，也是不行的
		else if (is_EDGE (entity1) && is_EDGE (entity2) && entity1 != entity2)
			return false;
		//如果两个顶点都在几何面上，但是所在几何面又不同，也是不行的
		else if (is_FACE (entity1) && is_FACE (entity2) && entity1 != entity2)
			return false;

		//如果一个顶点在几何点上，一个顶点在几何边上
		//1 这个几何点和几何边不相连，也是不行的;
		//2 这个几何点和几何边相连，但是几何边为曲边，且几何边上除了这个顶点外没有其余的网格点，为了保证网格质量，也是不行的
		else if (is_VERTEX(entity1) && is_EDGE(entity2)){
			ENTITY_LIST vertex_list;
			api_get_vertices(entity2,vertex_list);
			if(vertex_list.lookup(entity1) == -1)
				return false;
			if(is_straight(&((EDGE *)entity2)->geometry()->equation()) == false){
				std::unordered_set<OvmVeH> adj_vertices;
				get_adj_vertices_around_vertex(mesh,vh2,adj_vertices);//放入与vh2有连接关系的网格点
				bool multi_vertices_in_edge = false;
				foreach(auto &vh3,adj_vertices){
					auto entity3 = (ENTITY*)(V_ENTITY_PTR[vh3]);
					if(is_EDGE(entity3) && entity3 == entity2){
						multi_vertices_in_edge = true;
						break;
					}
				}
				if(multi_vertices_in_edge == false)
					return false;
			}
		}
		else if (is_VERTEX(entity2) && is_EDGE(entity1)){
			ENTITY_LIST vertex_list;
			api_get_vertices(entity1,vertex_list);
			if(vertex_list.lookup(entity2) == -1)
				return false;
			if(is_straight(&((EDGE *)entity1)->geometry()->equation()) == false){
				std::unordered_set<OvmVeH> adj_vertices;
				get_adj_vertices_around_vertex(mesh,vh1,adj_vertices);//放入与vh1有连接关系的网格点
				bool multi_vertices_in_edge = false;
				foreach(auto &vh3,adj_vertices){
					auto entity3 = (ENTITY*)(V_ENTITY_PTR[vh3]);
					if(is_EDGE(entity3) && entity3 == entity1){
						multi_vertices_in_edge = true;
						break;
					}
				}
				if(multi_vertices_in_edge == false)
					return false;
			}
		}

		//如果一个顶点在几何点上，一个顶点在几何面上，但是这个几何点和几何面不相连，也是不行的
		else if (is_VERTEX(entity1) && is_FACE(entity2)){
			ENTITY_LIST vertex_list;
			api_get_vertices(entity2,vertex_list);
			if(vertex_list.lookup(entity1) ==-1)
				return false;
		}
		else if (is_VERTEX(entity2) && is_FACE(entity1)){
			ENTITY_LIST vertex_list;
			api_get_vertices(entity1,vertex_list);
			if(vertex_list.lookup(entity2) == -1)
				return false;
		}
		//如果一个顶点在几何边上，一个顶点在几何面上，但是这个几何点和几何面不相连，也是不行的
		else if (is_EDGE(entity1) && is_FACE(entity2)){
			ENTITY_LIST edge_list;
			api_get_edges(entity2,edge_list);
			if(edge_list.lookup(entity1) == -1)
				return false;
		}
		else if (is_EDGE(entity2) && is_FACE(entity1)){
			ENTITY_LIST edge_list;
			api_get_edges(entity1,edge_list);
			if(edge_list.lookup(entity2) == -1)
				return false;
		}
	}
	return true;

}

bool one_simple_sheet_extraction (VolumeMesh *mesh, DualSheet *sheet, std::unordered_set<OvmFaH> &result_fhs)
{
	assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
	auto V_PREV_HANDLE = mesh->request_vertex_property<OvmVeH> ("prevhandle");
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
		V_PREV_HANDLE[*v_it] = *v_it;

	if(is_sheet_can_be_extracted(mesh,sheet) == false)
		return false;

	//获取sheet上所有的顶点
	std::unordered_set<OvmVeH> vhs_on_sheet;
	foreach (auto &eh, sheet->ehs){
		auto vh1 = mesh->edge (eh).from_vertex (),
			vh2 = mesh->edge (eh).to_vertex ();
		vhs_on_sheet.insert (vh1); vhs_on_sheet.insert (vh2);
	}
	//获得和sheet上的六面体相邻的六面体，这些六面体都需要进行重建（即先删除，然后更新它们的八个顶点的句柄，然后再重建）
	std::unordered_set<OvmCeH> adj_chs_need_rebuilding;
	foreach (auto &vh, vhs_on_sheet){
		std::unordered_set<OvmCeH> adj_chs;
		get_adj_hexas_around_vertex (mesh, vh, adj_chs);
		foreach (auto &ch, adj_chs){
			if (!contains (sheet->chs, ch))
				adj_chs_need_rebuilding.insert (ch);
		}
		std::unordered_set<OvmCeH>().swap(adj_chs);
	}
	std::unordered_set<OvmVeH> ().swap(vhs_on_sheet);
	//获得adj_chs_need_rebuilding上的六面体的八个顶点
	std::hash_map<OvmCeH, std::vector<OvmVeH>> adj_chs_rebuild_recipe;
	foreach (auto &ch, adj_chs_need_rebuilding){
		std::vector<OvmVeH> one_chs_recipe;
		for (auto hv_it = mesh->hv_iter (ch); hv_it; ++hv_it)
			one_chs_recipe.push_back (*hv_it);
		adj_chs_rebuild_recipe.insert (std::make_pair (ch, one_chs_recipe));
		std::vector<OvmVeH> ().swap(one_chs_recipe);
	}
	std::unordered_set<OvmCeH> ().swap(adj_chs_need_rebuilding);
	//将sheet的所有边进行分组，每组内的边均有连接关系
	std::unordered_set<OvmEgH> all_egh(sheet->ehs);
	std::set<std::unordered_set<OvmEgH>> edge_pairs;
	while (!all_egh.empty()){
		auto eh = *(all_egh.begin());
		std::unordered_set<OvmEgH> edge_pair;
		edge_pair.insert(eh);
		int original_number,final_number;
		do {
			original_number = edge_pair.size();
			std::unordered_set<OvmVeH> edge_pair_veh;
			foreach(auto &eh,edge_pair){
				auto vh1 = mesh->edge(eh).from_vertex(),
					vh2 = mesh->edge(eh).to_vertex();
				edge_pair_veh.insert(vh1),edge_pair_veh.insert(vh2);
			}
			foreach(auto &vh,edge_pair_veh){
				std::unordered_set<OvmEgH> adj_edges;
				get_adj_edges_around_vertex (mesh,vh,adj_edges);//放入与点vh相连接的所有边的handle值
				foreach(auto &eh,adj_edges){
					if(all_egh.find(eh) != all_egh.end()){
						edge_pair.insert(eh);
						all_egh.erase(eh);
					}
				}
				std::unordered_set<OvmEgH> ().swap(adj_edges);
			}
			std::unordered_set<OvmVeH> ().swap(edge_pair_veh);
			final_number = edge_pair.size();
		} while (original_number != final_number);
		edge_pairs.insert(edge_pair);
		std::unordered_set<OvmEgH>().swap(edge_pair);
	}
	std::unordered_set<OvmEgH> ().swap(all_egh);

	struct PointInfo{
		OvmVec3d pos;
		unsigned long entity_ptr;
	};

	//由于后面的删除操作可能会将一些游离的点一并删除，所以需要先将所有可能被删除的点的信息都存起来
	std::map<OvmVeH, PointInfo> vhs_info_for_readd;
	std::unordered_set<OvmVeH> new_vhs;
	std::hash_map<OvmVeH, OvmVeH> old_new_vhs_mapping;
	//edge_pairs中的每组边均合为一点
	foreach(auto &edge_pair,edge_pairs){
		if(edge_pair.size() == 1){
			auto eh = *edge_pair.begin();
			auto vh1 = mesh->edge (eh).from_vertex (),
				vh2 = mesh->edge (eh).to_vertex ();
			auto entity1 = (ENTITY*)(V_ENTITY_PTR[vh1]),
				entity2 = (ENTITY*)(V_ENTITY_PTR[vh2]);

			auto new_pt_info = PointInfo ();

			if (is_VERTEX (entity1)){
				new_pt_info.pos = mesh->vertex (vh1);
				new_pt_info.entity_ptr = (unsigned long)entity1;
			}
			else if (is_VERTEX (entity2)){
				new_pt_info.pos = mesh->vertex (vh2);
				new_pt_info.entity_ptr = (unsigned long)entity2;
			}
			else if (is_EDGE (entity1)){
				new_pt_info.pos = mesh->vertex (vh1);
				new_pt_info.entity_ptr = (unsigned long)entity1;
			}
			else if (is_EDGE (entity2)){
				new_pt_info.pos = mesh->vertex (vh2);
				new_pt_info.entity_ptr = (unsigned long)entity2;
			}
			else if (is_FACE (entity1)){
				new_pt_info.pos = mesh->vertex (vh1);
				new_pt_info.entity_ptr = (unsigned long)entity1;
			}
			else if (is_FACE (entity2)){
				new_pt_info.pos = mesh->vertex (vh2);
				new_pt_info.entity_ptr = (unsigned long)entity2;
			}
			else{
				new_pt_info.pos = mesh->vertex (vh1);
				new_pt_info.entity_ptr = (unsigned long)entity1;
			}
			auto new_vh = mesh->add_vertex (new_pt_info.pos);
			vhs_info_for_readd.insert (std::make_pair (new_vh, new_pt_info));
			new_vhs.insert (new_vh);
			old_new_vhs_mapping.insert (std::make_pair (vh1, new_vh));
			old_new_vhs_mapping.insert (std::make_pair (vh2, new_vh));
		}
		else{
			std::unordered_set<OvmVeH> vhs;
			foreach (auto &eh, edge_pair){
				auto vh1 = mesh->edge (eh).from_vertex (),
					vh2 = mesh->edge (eh).to_vertex ();
				vhs.insert (vh1); vhs.insert (vh2);
			}
			OvmVec3d mid_pos (0, 0, 0);
			foreach (auto &vh, vhs)
				mid_pos += mesh->vertex (vh);
			mid_pos /= vhs.size ();

			PointInfo new_pt_info;
			new_pt_info.pos = mid_pos;

			bool vertex_entity = false;
			foreach (auto &vh, vhs){
				auto entity1 = (ENTITY*)(V_ENTITY_PTR[vh]);
				if(is_VERTEX(entity1)){
					vertex_entity = true;
					new_pt_info.entity_ptr = (unsigned long)entity1;
					break;
				}
			}
			if(vertex_entity == false){
				foreach(auto &vh, vhs){
					auto entity1 = (ENTITY*)(V_ENTITY_PTR[vh]);
					if(is_EDGE(entity1)){
						vertex_entity = true;
						new_pt_info.entity_ptr = (unsigned long)entity1;
						break;
					}
				}
			}
			if(vertex_entity == false){
				foreach(auto &vh, vhs){
					auto entity1 = (ENTITY*)(V_ENTITY_PTR[vh]);
					if(is_FACE(entity1)){
						vertex_entity = true;
						new_pt_info.entity_ptr = (unsigned long)entity1;
						break;
					}
				}
			}
			if(vertex_entity == false){
				new_pt_info.entity_ptr = V_ENTITY_PTR[*(vhs.begin ())];
			}

			auto new_vh = mesh->add_vertex (new_pt_info.pos);
			vhs_info_for_readd.insert (std::make_pair (new_vh, new_pt_info));
			new_vhs.insert (new_vh);

			foreach (auto &vh, vhs)
				old_new_vhs_mapping.insert (std::make_pair (vh, new_vh));
			std::unordered_set<OvmVeH> ().swap(vhs);
		}
	}
	std::set<std::unordered_set<OvmEgH>> ().swap(edge_pairs);

	//对adj_chs_rebuild_recipe中那些非收缩点也将其信息放入vhs_info_for_readd中，如上面所说，以用于被删除后的重新添加
	foreach (auto &p, adj_chs_rebuild_recipe){
		auto &one_recipe = p.second;
		foreach (auto &vh, one_recipe){
			if (old_new_vhs_mapping.find (vh) == old_new_vhs_mapping.end ()){
				PointInfo new_pt_info;
				new_pt_info.entity_ptr = V_ENTITY_PTR[vh];
				new_pt_info.pos = mesh->vertex (vh);
				vhs_info_for_readd.insert (std::make_pair (vh, new_pt_info));
			}
		}
	}

	OpenVolumeMesh::StatusAttrib status_attrib (*mesh);
	//首先删除sheet中的六面体
	foreach (auto &ch, sheet->chs)
		status_attrib[ch].set_deleted (true);
	//然后删除和sheet中的六面体相邻的六面体
	foreach (auto &p, adj_chs_rebuild_recipe)
		status_attrib[p.first].set_deleted (true);

	status_attrib.garbage_collection (true);

	std::hash_map<OvmVeH, OvmVeH> old_new_vhs_mapping2;
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
		old_new_vhs_mapping2.insert (std::make_pair (V_PREV_HANDLE[*v_it], *v_it));


	//更新adj_chs_rebuild_recipe中的旧的顶点句柄为新的顶点句柄
	foreach (auto &p, adj_chs_rebuild_recipe){
		std::vector<OvmVeH> one_recipe = p.second;
		for (int i = 0; i != one_recipe.size (); ++i){
			if (old_new_vhs_mapping.find (one_recipe[i]) != old_new_vhs_mapping.end ())
				one_recipe[i] = old_new_vhs_mapping[one_recipe[i]];
			if (old_new_vhs_mapping2.find (one_recipe[i]) != old_new_vhs_mapping2.end ())
				one_recipe[i] = old_new_vhs_mapping2[one_recipe[i]];
			else{
				assert (vhs_info_for_readd.find (one_recipe[i]) != vhs_info_for_readd.end ());
				PointInfo pt_info = vhs_info_for_readd[one_recipe[i]];
				auto new_vh = mesh->add_vertex (pt_info.pos);
				V_ENTITY_PTR[new_vh] = pt_info.entity_ptr;
				old_new_vhs_mapping2.insert (std::make_pair (one_recipe[i], new_vh));
				one_recipe[i] = new_vh;
			}
		}	
		auto rebuilt_ch = mesh->add_cell(one_recipe);
		assert (rebuilt_ch != mesh->InvalidCellHandle);
		std::vector<OvmVeH>().swap(one_recipe);
	}
	std::hash_map<OvmCeH, std::vector<OvmVeH>> ().swap(adj_chs_rebuild_recipe);
	std::map<OvmVeH, PointInfo> ().swap(vhs_info_for_readd);
	std::hash_map<OvmVeH, OvmVeH> ().swap(old_new_vhs_mapping);

	//下面搜集由sheet抽取而生成的面集，这些面的四个端点都是new_vhs中的点
	std::unordered_set<OvmVeH> updated_new_vhs;
	foreach (auto &vh, new_vhs)
		updated_new_vhs.insert (old_new_vhs_mapping2[vh]);
	std::unordered_set<OvmVeH> ().swap(new_vhs);
	std::hash_map<OvmVeH, OvmVeH> ().swap(old_new_vhs_mapping2);

	foreach (auto &vh, updated_new_vhs){
		std::unordered_set<OvmFaH> adj_fhs;
		get_adj_faces_around_vertex (mesh, vh, adj_fhs);
		foreach (auto &fh, adj_fhs){
			auto adj_vhs = get_adj_vertices_around_face (mesh, fh);
			bool is_ok = true;
			foreach (auto &adj_vh, adj_vhs){
				if (!contains (updated_new_vhs, adj_vh)){
					is_ok = false; break;
				}
			}
			if (is_ok)
				result_fhs.insert (fh);
		}
	}
	std::unordered_set<OvmVeH>().swap(updated_new_vhs);

	return true;
}

bool general_sheets_extraction (VolumeMesh *mesh, BODY *body, std::vector<DualSheet *> & sheets)
{
	assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
	auto V_PREV_HANDLE = mesh->request_vertex_property<OvmVeH> ("prevhandle");
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
		V_PREV_HANDLE[*v_it] = *v_it;

	//获取sheet上所有的顶点
	std::unordered_set<OvmEgH>* all_ehs = new std::unordered_set<OvmEgH>();
	int sheets_num = sheets.size();
	for(int i = 0; i < sheets_num; i++)
		foreach(auto & eh, sheets[i]->ehs)
			all_ehs->insert(eh);
	std::unordered_set<OvmVeH>* vhs_on_sheet = new std::unordered_set<OvmVeH>();
	foreach (auto &eh, *all_ehs){
		auto vh1 = mesh->edge (eh).from_vertex (),
			vh2 = mesh->edge (eh).to_vertex ();
		vhs_on_sheet->insert (vh1); vhs_on_sheet->insert (vh2);
	}

	std::cout<<"1/n";

	//获得和sheet上的六面体相邻的六面体，这些六面体都需要进行重建（即先删除，然后更新它们的八个顶点的句柄，然后再重建）
	std::unordered_set<OvmCeH>* all_chs = new std::unordered_set<OvmCeH>();
	for(int i = 0; i < sheets_num; i++)
		foreach(auto & ch, sheets[i]->chs)
			all_chs->insert(ch);

	std::cout<<"1.5/n";

	std::unordered_set<OvmCeH>* other_chs = new std::unordered_set<OvmCeH>();
	SheetSet all_sheets;
	retrieve_sheets(mesh, all_sheets);
	foreach(auto sheet, all_sheets)
		if(!contains(sheets, sheet))
			foreach(auto ch, sheet->chs)
			    other_chs->insert(ch);

	std::unordered_set<OvmCeH>* adj_chs_need_rebuilding = new std::unordered_set<OvmCeH>();
	
	foreach(auto ch, *other_chs)
		if(!contains(*all_chs, ch))
			adj_chs_need_rebuilding->insert (ch);

	/*foreach (auto &vh, *vhs_on_sheet){
		std::unordered_set<OvmCeH> adj_chs;
		get_adj_hexas_around_vertex (mesh, vh, adj_chs);
		foreach (auto &ch, adj_chs){
			if (!contains (*all_chs, ch))
				adj_chs_need_rebuilding->insert (ch);
		}
		adj_chs.clear();
	}*/
	std::cout<<"2/n";

	delete vhs_on_sheet;
	//获得adj_chs_need_rebuilding上的六面体的八个顶点
	std::hash_map<OvmCeH, std::vector<OvmVeH> >* adj_chs_rebuild_recipe = new std::hash_map<OvmCeH, std::vector<OvmVeH> >();
	foreach (auto &ch, *adj_chs_need_rebuilding){
		std::vector<OvmVeH> one_chs_recipe;
		for (auto hv_it = mesh->hv_iter (ch); hv_it; ++hv_it)
			one_chs_recipe.push_back (*hv_it);
		adj_chs_rebuild_recipe->insert (std::make_pair (ch, one_chs_recipe));
		one_chs_recipe.clear();
	}
	delete adj_chs_need_rebuilding;

	//将sheet的所有边进行分组，每组内的边均有连接关系
	std::set<std::unordered_set<OvmEgH>>* edge_pairs = new std::set<std::unordered_set<OvmEgH>>();
	while (!all_ehs->empty()){
		auto eh = *(all_ehs->begin());
		std::unordered_set<OvmEgH> edge_pair;
		edge_pair.insert(eh);
		int original_number,final_number;
		do {
			original_number = edge_pair.size();
			std::unordered_set<OvmVeH> edge_pair_veh;
			foreach(auto &eh,edge_pair){
				auto vh1 = mesh->edge(eh).from_vertex(),
					vh2 = mesh->edge(eh).to_vertex();
				edge_pair_veh.insert(vh1),edge_pair_veh.insert(vh2);
			}
			foreach(auto &vh,edge_pair_veh){
				std::unordered_set<OvmEgH> adj_edges;
				get_adj_edges_around_vertex (mesh,vh,adj_edges);//放入与点vh相连接的所有边的handle值
				foreach(auto &eh,adj_edges){
					if(all_ehs->find(eh) != all_ehs->end()){
						edge_pair.insert(eh);
						all_ehs->erase(eh);
					}
				}
				adj_edges.clear();
			}
			edge_pair_veh.clear();
			final_number = edge_pair.size();
		} while (original_number != final_number);
		edge_pairs->insert(edge_pair);
		edge_pair.clear();
	}
	delete all_ehs;

	struct PointInfo{
		OvmVec3d pos;
		unsigned long entity_ptr;
	};

	std::cout<<"3/n";

	//由于后面的删除操作可能会将一些游离的点一并删除，所以需要先将所有可能被删除的点的信息都存起来
	std::hash_map<OvmVeH, PointInfo>* vhs_info_for_readd = new std::hash_map<OvmVeH, PointInfo>();
	std::unordered_set<OvmVeH>* new_vhs = new std::unordered_set<OvmVeH>();
	std::hash_map<OvmVeH, OvmVeH>* old_new_vhs_mapping = new std::hash_map<OvmVeH, OvmVeH>();;
	//edge_pairs中的每组边均合为一点
	foreach(auto &edge_pair,*edge_pairs){
		if(edge_pair.size() == 1){
			auto eh = *edge_pair.begin();
			auto vh1 = mesh->edge (eh).from_vertex (),
				vh2 = mesh->edge (eh).to_vertex ();
			auto entity1 = (ENTITY*)(V_ENTITY_PTR[vh1]),
				entity2 = (ENTITY*)(V_ENTITY_PTR[vh2]);

			auto new_pt_info = PointInfo ();

			if (is_VERTEX (entity1)){
				new_pt_info.pos = mesh->vertex (vh1);
				new_pt_info.entity_ptr = (unsigned long)entity1;
			}
			else if (is_VERTEX (entity2)){
				new_pt_info.pos = mesh->vertex (vh2);
				new_pt_info.entity_ptr = (unsigned long)entity2;
			}
			else if (is_EDGE (entity1)){
				new_pt_info.pos = mesh->vertex (vh1);
				new_pt_info.entity_ptr = (unsigned long)entity1;
			}
			else if (is_EDGE (entity2)){
				new_pt_info.pos = mesh->vertex (vh2);
				new_pt_info.entity_ptr = (unsigned long)entity2;
			}
			else if (is_FACE (entity1)){
				new_pt_info.pos = mesh->vertex (vh1);
				new_pt_info.entity_ptr = (unsigned long)entity1;
			}
			else if (is_FACE (entity2)){
				new_pt_info.pos = mesh->vertex (vh2);
				new_pt_info.entity_ptr = (unsigned long)entity2;
			}
			else{
				OvmVec3d mid_pos (0, 0, 0);
				mid_pos += mesh->vertex (vh1);
				mid_pos += mesh->vertex (vh2);
				mid_pos /= 2;
				new_pt_info.pos = mid_pos;
				new_pt_info.entity_ptr = (unsigned long)entity1;
				/*new_pt_info.pos = mesh->vertex(vh1);
				new_pt_info.entity_ptr = (unsigned long)entity1;*/
			}
			auto new_vh = mesh->add_vertex (new_pt_info.pos);
			vhs_info_for_readd->insert (std::make_pair (new_vh, new_pt_info));
			new_vhs->insert (new_vh);
			old_new_vhs_mapping->insert (std::make_pair (vh1, new_vh));
			old_new_vhs_mapping->insert (std::make_pair (vh2, new_vh));
		}
		else{
			std::unordered_set<OvmVeH> vhs;
			foreach (auto &eh, edge_pair){
				auto vh1 = mesh->edge (eh).from_vertex (),
					vh2 = mesh->edge (eh).to_vertex ();
				vhs.insert (vh1); vhs.insert (vh2);
			}
			OvmVec3d mid_pos (0, 0, 0);
			foreach (auto &vh, vhs)
				mid_pos += mesh->vertex (vh);
			mid_pos /= vhs.size ();

			PointInfo new_pt_info;

			bool entity_state = false;
			foreach (auto &vh, vhs){
				auto entity1 = (ENTITY*)(V_ENTITY_PTR[vh]);
				if(is_VERTEX(entity1)){
					entity_state = true;
					new_pt_info.pos = mesh->vertex(vh);
					new_pt_info.entity_ptr = (unsigned long)entity1;
					break;
				}
			}
				
			if(entity_state == false){
				foreach(auto &vh, vhs){
					auto entity1 = (ENTITY*)(V_ENTITY_PTR[vh]);
					if(is_EDGE(entity1)){
						entity_state = true;
						new_pt_info.pos = mesh->vertex(vh);
						new_pt_info.entity_ptr = (unsigned long)entity1;
						break;
					}
				}
			}
			if(entity_state == false){
				foreach(auto &vh, vhs){
					auto entity1 = (ENTITY*)(V_ENTITY_PTR[vh]);
					if(is_FACE(entity1)){
						entity_state = true;
						new_pt_info.pos = mesh->vertex(vh);
						new_pt_info.entity_ptr = (unsigned long)entity1;
						break;
					}
				}
			}
			if(entity_state == false){
				new_pt_info.pos = mid_pos;
				new_pt_info.entity_ptr = V_ENTITY_PTR[*(vhs.begin ())];
				/*new_pt_info.pos = mesh->vertex(*(vhs.begin()));
				new_pt_info.entity_ptr = V_ENTITY_PTR[*(vhs.begin ())];*/
			}

			auto new_vh = mesh->add_vertex (new_pt_info.pos);
			vhs_info_for_readd->insert (std::make_pair (new_vh, new_pt_info));
			new_vhs->insert (new_vh);

			foreach (auto &vh, vhs)
				old_new_vhs_mapping->insert (std::make_pair (vh, new_vh));
			vhs.clear();
		}
	}
	delete edge_pairs;

	//对adj_chs_rebuild_recipe中那些非收缩点也将其信息放入vhs_info_for_readd中，如上面所说，以用于被删除后的重新添加
	foreach (auto &p, *adj_chs_rebuild_recipe){
		auto &one_recipe = p.second;
		foreach (auto &vh, one_recipe){
			if (old_new_vhs_mapping->find (vh) == old_new_vhs_mapping->end ()){
				PointInfo new_pt_info;
				new_pt_info.entity_ptr = V_ENTITY_PTR[vh];
				new_pt_info.pos = mesh->vertex (vh);
				vhs_info_for_readd->insert (std::make_pair (vh, new_pt_info));
			}
		}
	}

	std::cout<<"4/n";

	OpenVolumeMesh::StatusAttrib status_attrib (*mesh);
	//首先删除sheet中的六面体
	foreach (auto &ch, *all_chs)
		status_attrib[ch].set_deleted (true);
	//然后删除和sheet中的六面体相邻的六面体
	foreach (auto &p, *adj_chs_rebuild_recipe)
		status_attrib[p.first].set_deleted (true);

	std::cout<<"5/n";

	status_attrib.garbage_collection (true);

	std::cout<<"6/n";

	delete all_chs;

	std::hash_map<OvmVeH, OvmVeH>* old_new_vhs_mapping2 = new std::hash_map<OvmVeH, OvmVeH>();
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
		old_new_vhs_mapping2->insert (std::make_pair (V_PREV_HANDLE[*v_it], *v_it));

	std::cout<<"7/n";

	//更新adj_chs_rebuild_recipe中的旧的顶点句柄为新的顶点句柄
	int count(0);
	int size = adj_chs_rebuild_recipe->size();
	for (auto map_iter = adj_chs_rebuild_recipe->begin(); map_iter != adj_chs_rebuild_recipe->end(); map_iter++){
		count++;
		std::vector<OvmVeH> one_recipe = map_iter->second;
		for (int i = 0; i != one_recipe.size (); ++i){
			if (old_new_vhs_mapping->find (one_recipe[i]) != old_new_vhs_mapping->end ())
				one_recipe[i] = (*old_new_vhs_mapping)[one_recipe[i]];
			if (old_new_vhs_mapping2->find (one_recipe[i]) != old_new_vhs_mapping2->end ()){
				one_recipe[i] = (*old_new_vhs_mapping2)[one_recipe[i]];
			}
			else{
				assert (vhs_info_for_readd->find (one_recipe[i]) != vhs_info_for_readd->end ());
				PointInfo pt_info = (*vhs_info_for_readd)[one_recipe[i]];
				auto new_vh = mesh->add_vertex (pt_info.pos);
				V_ENTITY_PTR[new_vh] = pt_info.entity_ptr;
				old_new_vhs_mapping2->insert (std::make_pair (one_recipe[i], new_vh));
				one_recipe[i] = new_vh;
			}
		}
		auto rebuilt_ch = mesh->add_cell(one_recipe);
		assert (rebuilt_ch != mesh->InvalidCellHandle);
		//std::cout<<"num:"<<count<<" "<<size<<"\n";
		std::vector<OvmVeH>().swap(one_recipe);
	}

	delete adj_chs_rebuild_recipe;
	delete vhs_info_for_readd;
	delete old_new_vhs_mapping;
	delete old_new_vhs_mapping2;
	delete new_vhs;

	return true;
}

// 不考虑面自交的情况
// direction: [0, 1]
// 代表面的两个方向
void get_shrink_set_on_direction(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmCeH> &shrink_set, int direction)
{
	shrink_set.clear();
	std::stack<OvmHaFaH> stack_hfhs;
	std::unordered_set<OvmFaH> used_fhs;

	auto fh = *(inflation_quad_set.begin());
	auto hfh = mesh->halfface_handle(fh, direction);
	stack_hfhs.push(hfh);

	while (!stack_hfhs.empty()) {
		auto hfh = stack_hfhs.top();
		stack_hfhs.pop();
		auto cell = mesh->incident_cell(hfh);
		// 不能在这个方向生成完整的shrink_set所有及时退出
		// Couldn't generate the full shrink set in the direction, so quit immediately.
		if (cell == mesh->InvalidCellHandle) return; 

		shrink_set.insert(cell);
		used_fhs.insert(mesh->face_handle(hfh));

		// 通过半面的四个半边来求和这个半面相邻，且方向一致的半面
		foreach (const auto &heh, mesh->halfface(hfh).halfedges()) {
			auto oppo_heh = mesh->opposite_halfedge_handle(heh);
			for (auto hehf_it = mesh->hehf_iter(oppo_heh); hehf_it; hehf_it++) {
				auto fh = mesh->face_handle(*hehf_it);
				if (!contains(used_fhs, fh) && contains(inflation_quad_set, fh)) {
					stack_hfhs.push(*hehf_it);
				}
			}
		}
	}
}

std::vector<DualSheet *> one_simple_sheet_inflation(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmCeH> &shrink_set, std::unordered_set<OvmEgH> &int_ehs, std::hash_map<OvmVeH, OvmVeH> &old_new_vhs_mapping)
{
	assert(mesh->vertex_property_exists<unsigned long>("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
	auto V_PREV_HANDLE = mesh->request_vertex_property<OvmVeH>("prevhandle", mesh->InvalidVertexHandle);
	for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
		V_PREV_HANDLE[*v_it] = *v_it;

	//获得fhs上的所有顶点
	std::unordered_set<OvmVeH> all_vhs_on_fhs;
	foreach(auto &fh, inflation_quad_set){
		auto hehs = mesh->face(fh).halfedges();
		foreach(auto &heh, hehs)
			all_vhs_on_fhs.insert(mesh->halfedge(heh).to_vertex());
	}

	//存放旧的点和新的点之间的映射关系
	//由于一般情况，旧的点一分为二，而在交叉处旧的点一分为四，所以旧点喝新点中间存在着一对多的对应关系，
	//为了区别旧点到底对应于哪个新点，需要一个六面体集合来做判断，该六面体集合就是旧点边上的一个shrink_set
	auto newly_created_vertices_cells_mapping = new std::map<OvmVeH, std::map<OvmVeH, std::unordered_set<OvmCeH> > >();
	//new_original_vhs_mapping存储新生成的点和原始点之间的对应关系，是一种多对一的关系
	std::hash_map<OvmVeH, OvmVeH> new_original_vhs_mapping;
	//lambda function to get the corresponding new vertex according to the original vertex and the cell
	auto fGetNewVeHOnCeH = [&](OvmVeH vh, OvmCeH ch)->OvmVeH{
		auto locate = newly_created_vertices_cells_mapping->find(vh);
		//if not found, return invalid handle
		if (locate == newly_created_vertices_cells_mapping->end())
			return mesh->InvalidVertexHandle;
		auto &mapping = locate->second;
		foreach(auto &p, mapping) {
			if (contains(p.second, ch))
				return p.first;
			//if (p.second.empty () && ch == mesh->InvalidCellHandle)
			//	return p.first;
		}
		return mesh->InvalidVertexHandle;
	};

	//stores all the cells adjacent to faces in fhs
	std::unordered_set<OvmCeH> all_adj_cells;
	std::unordered_set<OvmVeH> new_vertices;
	foreach(auto &vh, all_vhs_on_fhs) {
		std::vector<std::unordered_set<OvmCeH> > cell_groups;
		std::map<OvmVeH, std::unordered_set<OvmCeH> > newly_vertices_distribution;
		get_cell_groups_around_vertex(mesh, vh, inflation_quad_set, cell_groups);
		//如果fhs部分区域贴着网格表面时，cell_groups只包含一个集合，则此时需要再补上一个空集合
		if (cell_groups.size() == 1){
			std::unordered_set<OvmCeH> tmp;
			tmp.insert(mesh->InvalidCellHandle);
			cell_groups.push_back(tmp);
		}
		else if (cell_groups.size() == 2){
			std::unordered_set<OvmFaH> adj_fhs, adj_fhs_on_inflation_fhs;
			get_adj_faces_around_vertex(mesh, vh, adj_fhs);
			foreach(auto &fh, adj_fhs){
				if (contains(inflation_quad_set, fh))
					adj_fhs_on_inflation_fhs.insert(fh);
			}
			foreach(auto &fh, adj_fhs_on_inflation_fhs){
				auto hfh = mesh->halfface_handle(fh, 0);
				auto ch1 = mesh->incident_cell(hfh);
				hfh = mesh->opposite_halfface_handle(hfh);
				auto ch2 = mesh->incident_cell(hfh);
				if (ch1 != mesh->InvalidCellHandle && ch2 != mesh->InvalidCellHandle)
					continue;
				if (ch1 == mesh->InvalidCellHandle){
					if (contains(cell_groups.front(), ch2)){
						cell_groups[1].insert(ch1);
					}
					else{
						cell_groups[0].insert(ch1);
					}
				}
				else{
					if (contains(cell_groups.front(), ch1)){
						cell_groups[1].insert(ch2);
					}
					else{
						cell_groups[0].insert(ch2);
					}
				}
			}
		}
		foreach(auto &one_chs_group, cell_groups){
			//下面要判断下one_chs_group是否在shrink_set中。如果在的话，那么这个新的点是新产生的，所以他的entity_ptr为空
			//如果不在shrink_set中，那么这个点就是原来的旧点（因为拓扑修改的需要才重新建了它）
			OvmVeH new_vertex = OvmVeH(-1);
			OvmVec3d reasonable_pos(0, 0, 0);
			reasonable_pos = mesh->vertex(vh);
			new_vertex = mesh->add_vertex(reasonable_pos);
			if (V_ENTITY_PTR[vh] == 0){
				V_ENTITY_PTR[new_vertex] = 0;
			}
			else{
				if (intersects(one_chs_group, shrink_set)){
					V_ENTITY_PTR[new_vertex] = -1;
				}
				else{
					V_ENTITY_PTR[new_vertex] = V_ENTITY_PTR[vh];
				}
			}
			new_original_vhs_mapping.insert(std::make_pair(new_vertex, vh));
			new_vertices.insert(new_vertex);
			newly_vertices_distribution.insert(std::make_pair(new_vertex, one_chs_group));
			foreach(auto &ch, one_chs_group){
				if (ch != mesh->InvalidCellHandle)
					all_adj_cells.insert(ch);
			}
		}
		newly_created_vertices_cells_mapping->insert(std::make_pair(vh, newly_vertices_distribution));
	}//end foreach (auto &vh, all_vhs) {...


	//由于后面进行garbage_collection的时候会删除掉一些点，然后OVM内部会重新计算顶点的句柄数值
	//所以需要预先保存一下旧的顶点对应的几何位置以及ENTITY_PTR的值
	struct PointInfo{
		PointInfo(){
			pos = OvmVec3d(0, 0, 0);
			entity_ptr = -1;
		}
		OvmVec3d pos;
		unsigned long entity_ptr;
	};

	std::hash_map<OvmVeH, PointInfo> vh_info_for_readd;
	foreach(auto &ch, all_adj_cells){
		for (auto v_it = mesh->hv_iter(ch); v_it; ++v_it){
			auto vh = *v_it;
			if (!contains(all_vhs_on_fhs, vh)){
				PointInfo point_info;
				point_info.pos = mesh->vertex(vh);
				point_info.entity_ptr = V_ENTITY_PTR[vh];
				vh_info_for_readd.insert(std::make_pair(vh, point_info));
			}
		}
	}
	foreach(auto &vh, new_vertices){
		PointInfo point_info;
		point_info.pos = mesh->vertex(vh);
		point_info.entity_ptr = V_ENTITY_PTR[vh];
		vh_info_for_readd.insert(std::make_pair(vh, point_info));
	}

	//auto old_mesh_sheet_ptr = new std::map<std::set<OvmVeH>, unsigned long>();
	//auto old_mesh_chord_ptr = new std::map<std::set<OvmVeH>, unsigned long>();

	//搜集那些需要重建的六面体的八个顶点
	std::hash_map<OvmCeH, std::vector<OvmVeH> > cells_rebuilding_recipes;
	foreach(auto &ch, all_adj_cells) {
		std::vector<OvmVeH> ch_vhs;
		std::hash_map<OvmVeH, OvmVeH> curr_old_new_vhs_mapping;//存储当前六面体中新旧点的映射关系
		for (auto hv_it = mesh->hv_iter(ch); hv_it; ++hv_it) {
			auto newly_vh = fGetNewVeHOnCeH(*hv_it, ch);
			if (newly_vh == mesh->InvalidVertexHandle){
				ch_vhs.push_back(*hv_it);
				curr_old_new_vhs_mapping[*hv_it] = *hv_it;
			}
			else{
				ch_vhs.push_back(newly_vh);
				curr_old_new_vhs_mapping[*hv_it] = newly_vh;
			}
		}
		cells_rebuilding_recipes.insert(std::make_pair(ch, ch_vhs));
		std::unordered_set<OvmEgH> ehs_on_ch;
		get_adj_edges_around_cell(mesh, ch, ehs_on_ch);

		/*foreach(auto &eh, ehs_on_ch){
			auto vh1 = curr_old_new_vhs_mapping[mesh->edge(eh).to_vertex()],
			vh2 = curr_old_new_vhs_mapping[mesh->edge(eh).from_vertex()];
			std::set<OvmVeH> vhs_set;
			vhs_set.insert(vh1); vhs_set.insert(vh2);
			(*old_mesh_sheet_ptr)[vhs_set] = E_SHEET_PTR[eh];
			(*old_mesh_chord_ptr)[vhs_set] = E_CHORD_PTR[eh];
			}*/
	}//end foreach (auto &ch, all_adj_cells) {...

	//搜集那些普通区域（即不是在交叉区域）上的新的六面体的八个顶点
	std::vector<std::vector<OvmVeH> > ord_newly_created_cells_recipes;
	foreach(auto &fh, inflation_quad_set) {
		auto hfh1 = mesh->halfface_handle(fh, 0), hfh2 = mesh->halfface_handle(fh, 1);
		auto ch1 = mesh->incident_cell(hfh1), ch2 = mesh->incident_cell(hfh2);
		std::vector<OvmVeH> ch_vhs;
		for (auto hfv_it = mesh->hfv_iter(hfh1); hfv_it; ++hfv_it){
			auto newly_vh = fGetNewVeHOnCeH(*hfv_it, ch1);
			assert(newly_vh != mesh->InvalidCellHandle);
			ch_vhs.push_back(newly_vh);
		}

		for (auto hfv_it = mesh->hfv_iter(hfh2); hfv_it; ++hfv_it){
			auto newly_vh = fGetNewVeHOnCeH(*hfv_it, ch2);
			assert(newly_vh != mesh->InvalidCellHandle);
			ch_vhs.push_back(newly_vh);
		}
		assert(ch_vhs.size() == 8);
		ord_newly_created_cells_recipes.push_back(ch_vhs);
	}

	//搜集交叉区域新的六面体的八个顶点
	std::vector<std::vector<OvmVeH> > int_newly_created_cells_recipes;
	foreach(auto &eh, int_ehs) {
		auto heh = mesh->halfedge_handle(eh, 0);
		std::vector<OvmVeH> ch_vhs_up, ch_vhs_down;
		auto vh_up_origin = mesh->halfedge(heh).from_vertex(),
			vh_down_origin = mesh->halfedge(heh).to_vertex();
		//use the intersecting halfedges to find the adjacent cell groups,
		//one cell group indicates one newly created vertex
		for (auto hehf_it = mesh->hehf_iter(heh); hehf_it; ++hehf_it){
			auto fh = mesh->face_handle(*hehf_it);
			//if fh is in fhs, one cell group is found
			if (inflation_quad_set.find(fh) != inflation_quad_set.end()) {
				//get the 1st newly created vertex
				auto test_ch = mesh->incident_cell(*hehf_it);
				auto newly_vh = fGetNewVeHOnCeH(vh_up_origin, test_ch);
				assert(newly_vh != mesh->InvalidVertexHandle);
				ch_vhs_up.push_back(newly_vh);

				//get the 2nd newly created vertex
				newly_vh = fGetNewVeHOnCeH(vh_down_origin, test_ch);
				assert(newly_vh != mesh->InvalidVertexHandle);
				ch_vhs_down.push_back(newly_vh);
			}
		}//end for (auto hehf_it = mesh->hehf_iter (heh); hehf_it; ++hehf_it)...
		assert(ch_vhs_up.size() == 4 && ch_vhs_down.size() == 4);
		qSwap(ch_vhs_up[1], ch_vhs_up[3]);
		ch_vhs_up.insert(ch_vhs_up.end(), ch_vhs_down.begin(), ch_vhs_down.end());
		int_newly_created_cells_recipes.push_back(ch_vhs_up);
	}


	OpenVolumeMesh::StatusAttrib status_attrib(*mesh);
	//首先删除旧的需要删除的顶点，连带删除了与此相邻的六面体
	foreach(auto &ch, all_adj_cells)
		status_attrib[ch].set_deleted(true);

	status_attrib.garbage_collection(true);


	//old_new_vhs_mapping存储旧的点（经过前面分裂后的点，并不是original的点）和新的点之间的对应关系
	//由于是分裂后的点，因此这个对应关系是一对一映射
	//std::hash_map<OvmVeH, OvmVeH> old_new_vhs_mapping;
	for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it){
		auto new_vh = *v_it;
		auto ori_vh = V_PREV_HANDLE[new_vh];
		if (ori_vh != mesh->InvalidVertexHandle)
			old_new_vhs_mapping.insert(std::make_pair(ori_vh, new_vh));
	}

	//vhs是之前保存的点序列，fUpdateVhs用于更新这个点序列，
	//如果vhs中的点在old_new_vhs_mapping中能够找到，说明garbage_collection过程中并没有将其删除，
	//因此OpenVolumeMesh自动更新了V_PREV_HANDLE中的属性；
	//如果vhs中的点不能够在old_new_vhs_mapping中找到，说明在garbage_collection中删除了，
	//因此需要根据之前保存着vh_info_db中该点的几何坐标位置及其他信息将其重建，同时更新V_PREV_HANDLE该点存储的信息
	auto fUpdateVhs = [&](std::vector<OvmVeH> &vhs){
		for (int i = 0; i != vhs.size(); ++i) {
			auto old_vh = vhs[i];
			auto locate = old_new_vhs_mapping.find(old_vh);
			if (locate != old_new_vhs_mapping.end())
				vhs[i] = locate->second;
			else{
				auto locate = vh_info_for_readd.find(old_vh);
				assert(locate != vh_info_for_readd.end());
				auto new_vh = mesh->add_vertex(locate->second.pos);
				V_PREV_HANDLE[new_vh] = old_vh;
				V_ENTITY_PTR[new_vh] = locate->second.entity_ptr;
				vhs[i] = new_vh;
				old_new_vhs_mapping.insert(std::make_pair(old_vh, new_vh));
			}
		}
	};

	for (auto it = cells_rebuilding_recipes.begin(); it != cells_rebuilding_recipes.end(); ++it){
		fUpdateVhs(it->second);
		auto ch = mesh->add_cell(it->second);
		assert(ch != mesh->InvalidCellHandle);
	}

	std::unordered_set<OvmCeH> new_chs;

	for (int i = 0; i != ord_newly_created_cells_recipes.size(); ++i){
		auto &vhs = ord_newly_created_cells_recipes[i];
		fUpdateVhs(vhs);
		auto ch = mesh->add_cell(vhs);
		assert(ch != mesh->InvalidCellHandle);
		new_chs.insert(ch);
	}

	for (int i = 0; i != int_newly_created_cells_recipes.size(); ++i){
		auto &vhs = int_newly_created_cells_recipes[i];
		fUpdateVhs(vhs);
		auto ch = mesh->add_cell(vhs);
		assert(ch != mesh->InvalidCellHandle);
		new_chs.insert(ch);
	}

	std::vector<DualSheet *> new_sheets;
	auto fIsNewSheet = [&](std::unordered_set<OvmCeH> &chs)->bool{
		foreach(auto &ch, chs){
			if (!contains(new_chs, ch)) return false;
		}
		return true;
	};

	while (!new_chs.empty()){
		DualSheet *new_sheet = new DualSheet(mesh);

		std::unordered_set<OvmEgH> all_ehs_of_first_ch;
		OvmCeH first_ch = *(new_chs.begin());
		auto hfh_vec = mesh->cell(first_ch).halffaces();
		foreach(auto &hfh, hfh_vec){
			auto heh_vec = mesh->halfface(hfh).halfedges();
			foreach(auto &heh, heh_vec)
				all_ehs_of_first_ch.insert(mesh->edge_handle(heh));
		}

		forever{
			assert(!all_ehs_of_first_ch.empty());
			auto test_eh = *(all_ehs_of_first_ch.begin());
			std::unordered_set<OvmEgH> sheet_ehs;
			std::unordered_set<OvmCeH> sheet_chs;
			retrieve_one_sheet(mesh, test_eh, sheet_ehs, sheet_chs);
			if (fIsNewSheet(sheet_chs)){
				new_sheet->ehs = sheet_ehs;
				new_sheet->chs = sheet_chs;
				break;
			}
			else{
				foreach(auto &eh, sheet_ehs)
					all_ehs_of_first_ch.erase(eh);
			}
		}
		/*foreach(auto &eh, new_sheet->ehs)
			E_SHEET_PTR[eh] = (unsigned long)new_sheet;*/

		foreach(auto &ch, new_sheet->chs)
			new_chs.erase(ch);
		new_sheets.push_back(new_sheet);
	}

		
	delete newly_created_vertices_cells_mapping;
	//delete old_mesh_chord_ptr;
	//delete old_mesh_sheet_ptr;
	return new_sheets;
}

std::vector<DualSheet *> sheet_dicing(VolumeMesh *mesh, BODY* body, DualSheet* sheet/*, std::hash_map<OvmVeH, OvmVeH> &old_new_vhs_mapping*/)
{
	assert(mesh->vertex_property_exists<unsigned long>("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
	auto V_PREV_HANDLE = mesh->request_vertex_property<OvmVeH>("prevhandle", mesh->InvalidVertexHandle);
	for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
		V_PREV_HANDLE[*v_it] = *v_it;

	//auto E_SHEET_PTR = mesh->request_edge_property<unsigned long>("sheetptr");
	//auto E_CHORD_PTR = mesh->request_edge_property<unsigned long>("chordptr");

	std::unordered_set<OvmFaH> inflation_quad_set; 
	std::unordered_set<OvmCeH> shrink_set(sheet->chs);
	std::unordered_set<OvmEgH> int_ehs;
	std::hash_map<OvmVeH, OvmVeH> old_new_vhs_mapping;

	//在sheet的边的每对顶点之间建立映射关系
	std::map<OvmVeH, OvmVeH> *corresponding_vh_pair = new std::map<OvmVeH, OvmVeH> ();
	std::unordered_set<OvmEgH> ehs_sheet(sheet->ehs);
	std::unordered_set<OvmHaFaH> hfhs_sheet;

	//首先找出这条sheet所在六面体的所有的半面
	foreach(auto & ch,sheet->chs){
			foreach(auto & hfh,mesh->cell(ch).halffaces())
				hfhs_sheet.insert(hfh);
		}
		//找出sheet的两组底面
		std::unordered_set<OvmHaFaH> hfhs_sheet_temp;
		foreach(auto & hfh,hfhs_sheet){
			bool is_ok = true;
			foreach(auto & heh,mesh->halfface(hfh).halfedges()){
				if(contains(ehs_sheet,mesh->edge_handle(heh))){
					is_ok = false;
					break;
				}
			}
			if(is_ok == true)
				hfhs_sheet_temp.insert(hfh);
		}
		hfhs_sheet.clear();

		std::vector<OvmHaFaH> hfhs_sheet_up;
		OvmHaFaH hfh_temp = *hfhs_sheet_temp.begin();
		hfhs_sheet_up.push_back(hfh_temp);
		hfhs_sheet_temp.erase(hfh_temp);
		for(int i = 0; i < hfhs_sheet_up.size(); ++i){
			for(auto hfh_set_iter = hfhs_sheet_temp.begin(); hfh_set_iter != hfhs_sheet_temp.end();){
				if(!(get_common_edge_handle(mesh,mesh->face_handle(hfhs_sheet_up[i]),mesh->face_handle(*hfh_set_iter)) == mesh->InvalidEdgeHandle)){
					OvmHaFaH hfh_ = *hfh_set_iter;
					hfhs_sheet_up.push_back(hfh_);
					++hfh_set_iter;
					hfhs_sheet_temp.erase(hfh_);
					continue;
				}
				++hfh_set_iter;
			}
		}
		foreach(auto & hfh, hfhs_sheet_up)
			inflation_quad_set.insert(mesh->face_handle(hfh));

		//获得fhs上的所有顶点
		std::unordered_set<OvmVeH> all_vhs_on_fhs;
		foreach (auto &fh, inflation_quad_set){
			auto hehs = mesh->face (fh).halfedges ();
			foreach (auto &heh, hehs)
				all_vhs_on_fhs.insert (mesh->halfedge (heh).to_vertex ());
		}
			
		foreach(auto &vh, all_vhs_on_fhs){
			foreach(auto eh, ehs_sheet){
				if(mesh->edge(eh).from_vertex() == vh){
					corresponding_vh_pair->insert(std::make_pair(vh, mesh->edge(eh).to_vertex()));
					ehs_sheet.erase(eh);
					break;
				}
				else if(mesh->edge(eh).to_vertex() == vh){
					corresponding_vh_pair->insert(std::make_pair(vh, mesh->edge(eh).from_vertex()));
					ehs_sheet.erase(eh);
					break;
				}
			}
		}

	//存放旧的点和新的点之间的映射关系
	//由于一般情况，旧的点一分为二，而在交叉处旧的点一分为四，所以旧点喝新点中间存在着一对多的对应关系，
	//为了区别旧点到底对应于哪个新点，需要一个六面体集合来做判断，该六面体集合就是旧点边上的一个shrink_set
	auto newly_created_vertices_cells_mapping = new std::map<OvmVeH, std::map<OvmVeH, std::unordered_set<OvmCeH> > >();
	//new_original_vhs_mapping存储新生成的点和原始点之间的对应关系，是一种多对一的关系
	std::hash_map<OvmVeH, OvmVeH> new_original_vhs_mapping;
	//lambda function to get the corresponding new vertex according to the original vertex and the cell
	auto fGetNewVeHOnCeH = [&](OvmVeH vh, OvmCeH ch)->OvmVeH{
		auto locate = newly_created_vertices_cells_mapping->find(vh);
		//if not found, return invalid handle
		if (locate == newly_created_vertices_cells_mapping->end())
			return mesh->InvalidVertexHandle;
		auto &mapping = locate->second;
		foreach(auto &p, mapping) {
			if (contains(p.second, ch))
				return p.first;
			//if (p.second.empty () && ch == mesh->InvalidCellHandle)
			//	return p.first;
		}
		return mesh->InvalidVertexHandle;
	};

	//stores all the cells adjacent to faces in fhs
	std::unordered_set<OvmCeH> all_adj_cells;
	std::unordered_set<OvmVeH> new_vertices;
	foreach(auto &vh, all_vhs_on_fhs) {
		std::vector<std::unordered_set<OvmCeH> > cell_groups;
		std::map<OvmVeH, std::unordered_set<OvmCeH> > newly_vertices_distribution;
		get_cell_groups_around_vertex(mesh, vh, inflation_quad_set, cell_groups);
		//如果fhs部分区域贴着网格表面时，cell_groups只包含一个集合，则此时需要再补上一个空集合
		if (cell_groups.size() == 1){
			std::unordered_set<OvmCeH> tmp;
			tmp.insert(mesh->InvalidCellHandle);
			cell_groups.push_back(tmp);
		}
		else if (cell_groups.size() == 2){
			std::unordered_set<OvmFaH> adj_fhs, adj_fhs_on_inflation_fhs;
			get_adj_faces_around_vertex(mesh, vh, adj_fhs);
			foreach(auto &fh, adj_fhs){
				if (contains(inflation_quad_set, fh))
					adj_fhs_on_inflation_fhs.insert(fh);
			}
			foreach(auto &fh, adj_fhs_on_inflation_fhs){
				auto hfh = mesh->halfface_handle(fh, 0);
				auto ch1 = mesh->incident_cell(hfh);
				hfh = mesh->opposite_halfface_handle(hfh);
				auto ch2 = mesh->incident_cell(hfh);
				if (ch1 != mesh->InvalidCellHandle && ch2 != mesh->InvalidCellHandle)
					continue;
				if (ch1 == mesh->InvalidCellHandle){
					if (contains(cell_groups.front(), ch2)){
						cell_groups[1].insert(ch1);
					}
					else{
						cell_groups[0].insert(ch1);
					}
				}
				else{
					if (contains(cell_groups.front(), ch1)){
						cell_groups[1].insert(ch2);
					}
					else{
						cell_groups[0].insert(ch2);
					}
				}
			}
		}
		foreach(auto &one_chs_group, cell_groups){
			//下面要判断下one_chs_group是否在shrink_set中。如果在的话，那么这个新的点是新产生的，所以他的entity_ptr为空
			//如果不在shrink_set中，那么这个点就是原来的旧点（因为拓扑修改的需要才重新建了它）
			OvmVeH new_vertex = OvmVeH(-1);
			OvmVec3d reasonable_pos(0, 0, 0);
			if (intersects (one_chs_group, shrink_set)){
				OvmVeH vh_opposite = (*corresponding_vh_pair)[vh];
				reasonable_pos = (mesh->vertex (vh) + mesh->vertex(vh_opposite)) / 2;
				new_vertex = mesh->add_vertex (reasonable_pos);
				if((V_ENTITY_PTR[vh] == 0) || (V_ENTITY_PTR[vh_opposite] == 0))
					V_ENTITY_PTR[new_vertex] = 0;
				else{
					ENTITY * entity1, *entity2;
					entity1 = (ENTITY *)V_ENTITY_PTR[vh];
					entity2 = (ENTITY *)V_ENTITY_PTR[vh_opposite];
					if(is_VERTEX(entity1) && is_VERTEX(entity2)){
						ENTITY *common_edge;
						bool is_ok = false;
						ENTITY_LIST acis_edges1, acis_edges2;
						api_get_edges(entity1, acis_edges1);
						api_get_edges(entity2, acis_edges2);
						for(int i = 0; i < acis_edges1.count(); i++){
							if(acis_edges2.lookup(acis_edges1[i]) != -1){
								common_edge = acis_edges1[i];
								is_ok = true;
								break;
							}
						}
						if(is_ok){
							V_ENTITY_PTR[new_vertex] = (unsigned long)common_edge;
							SPAposition c_pos;
							double dist = 0.0f;
							SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
							api_entity_point_distance(common_edge,s_pos,c_pos,dist);
							mesh->set_vertex(new_vertex,SPA2POS(c_pos));
						}
						else{
							ENTITY *common_face;
							ENTITY_LIST acis_faces1, acis_faces2;
							api_get_faces(entity1, acis_faces1);
							api_get_faces(entity2, acis_faces2);
							for(int i = 0; i < acis_faces1.count(); i++){
								if(acis_faces2.lookup(acis_faces1[i]) != -1){
									common_face = acis_faces1[i];
									is_ok = true;
									break;
								}
							}
							if(is_ok){
								V_ENTITY_PTR[new_vertex] = (unsigned long)common_face;
								SPAposition c_pos;
								double dist = 0.0f;
								SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
								api_entity_point_distance(common_face,s_pos,c_pos,dist);
								mesh->set_vertex(new_vertex,SPA2POS(c_pos));
							}
							else
								V_ENTITY_PTR[new_vertex] = 0;
						}
					}
					else if(is_VERTEX(entity1) && is_EDGE(entity2)){
						ENTITY_LIST acis_vertices;
						api_get_vertices(entity2, acis_vertices);
						if(acis_vertices.lookup(entity1) != -1){
							V_ENTITY_PTR[new_vertex] = (unsigned long)entity2;
							SPAposition c_pos;
							double dist = 0.0f;
							SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
							api_entity_point_distance(entity2,s_pos,c_pos,dist);
							mesh->set_vertex(new_vertex,SPA2POS(c_pos));
						}
						else{
							ENTITY *common_face;
							bool is_ok = false;
							ENTITY_LIST acis_faces1, acis_faces2;
							api_get_faces(entity1, acis_faces1);
							api_get_faces(entity2, acis_faces2);
							for(int i = 0; i < acis_faces1.count(); i++){
								if(acis_faces2.lookup(acis_faces1[i]) != -1){
									common_face = acis_faces1[i];
									is_ok = true;
									break;
								}
							}
							if(is_ok){
								V_ENTITY_PTR[new_vertex] = (unsigned long)common_face;
								SPAposition c_pos;
								double dist = 0.0f;
								SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
								api_entity_point_distance(common_face,s_pos,c_pos,dist);
								mesh->set_vertex(new_vertex,SPA2POS(c_pos));
							}
							else 
								V_ENTITY_PTR[new_vertex] = 0;
						}
					}
					else if(is_VERTEX(entity2) && is_EDGE(entity1)){
						ENTITY_LIST acis_vertices;
						api_get_vertices(entity1, acis_vertices);
						if(acis_vertices.lookup(entity2) != -1){
							V_ENTITY_PTR[new_vertex] = (unsigned long)entity1;
							SPAposition c_pos;
							double dist = 0.0f;
							SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
							api_entity_point_distance(entity1,s_pos,c_pos,dist);
							mesh->set_vertex(new_vertex,SPA2POS(c_pos));
						}
						else{
							ENTITY *common_face;
							bool is_ok = false;
							ENTITY_LIST acis_faces1, acis_faces2;
							api_get_faces(entity1, acis_faces1);
							api_get_faces(entity2, acis_faces2);
							for(int i = 0; i < acis_faces1.count(); i++){
								if(acis_faces2.lookup(acis_faces1[i]) != -1){
									common_face = acis_faces1[i];
									is_ok = true;
									break;
								}
							}
							if(is_ok){
								V_ENTITY_PTR[new_vertex] = (unsigned long)common_face;
								SPAposition c_pos;
								double dist = 0.0f;
								SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
								api_entity_point_distance(common_face,s_pos,c_pos,dist);
								mesh->set_vertex(new_vertex,SPA2POS(c_pos));
							}
							else
								V_ENTITY_PTR[new_vertex] = 0;
						}
					}
					else if(is_VERTEX(entity1) && is_FACE(entity2)){
						ENTITY_LIST acis_vertices;
						api_get_vertices(entity2, acis_vertices);
						if(acis_vertices.lookup(entity1) != -1){
							V_ENTITY_PTR[new_vertex] = (unsigned long)entity2;
							SPAposition c_pos;
							double dist = 0.0f;
							SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
							api_entity_point_distance(entity2,s_pos,c_pos,dist);
							mesh->set_vertex(new_vertex,SPA2POS(c_pos));
						}
						else{
							V_ENTITY_PTR[new_vertex] = 0;
						}
					}
					else if(is_VERTEX(entity2) && is_FACE(entity1)){
						ENTITY_LIST acis_vertices;
						api_get_vertices(entity1, acis_vertices);
						if(acis_vertices.lookup(entity2) != -1){
							V_ENTITY_PTR[new_vertex] = (unsigned long)entity1;
							SPAposition c_pos;
							double dist = 0.0f;
							SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
							api_entity_point_distance(entity1,s_pos,c_pos,dist);
							mesh->set_vertex(new_vertex,SPA2POS(c_pos));
						}
						else{
							V_ENTITY_PTR[new_vertex] = 0;
						}
					}
					else if(is_EDGE(entity1) && is_EDGE(entity2)){
						if(entity1 == entity2){
							V_ENTITY_PTR[new_vertex] = (unsigned long)entity1;
							SPAposition c_pos;
							double dist = 0.0f;
							SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
							double para1 = ((EDGE*)entity1)->geometry()->trans_curve()->param(POS2SPA(mesh->vertex(vh)));
							double para2 = ((EDGE*)entity1)->geometry()->trans_curve()->param(POS2SPA(mesh->vertex(vh_opposite)));
							double para = (para1 + para2)/2;
							if(((EDGE*)entity1)->start() == ((EDGE*)entity1)->end()){
								api_entity_point_distance(entity1,s_pos,c_pos,dist);
								mesh->set_vertex(new_vertex,SPA2POS(c_pos));
							}
							else{
								c_pos = ((EDGE*)entity1)->geometry()->trans_curve()->eval_position(para);
								//api_entity_point_distance(entity1,s_pos,c_pos,dist);
								mesh->set_vertex(new_vertex,SPA2POS(c_pos));
							}
						}
						else{
							ENTITY *common_face;
							bool is_ok = false;
							ENTITY_LIST acis_faces1, acis_faces2;
							api_get_faces(entity1, acis_faces1);
							api_get_faces(entity2, acis_faces2);
							for(int i = 0; i < acis_faces1.count(); i++){
								if(acis_faces2.lookup(acis_faces1[i]) != -1){
									common_face = acis_faces1[i];
									is_ok = true;
									break;
								}
							}
							if(is_ok){
								V_ENTITY_PTR[new_vertex] = (unsigned long)common_face;
								SPAposition c_pos;
								double dist = 0.0f;
								SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
								api_entity_point_distance(common_face,s_pos,c_pos,dist);
								mesh->set_vertex(new_vertex,SPA2POS(c_pos));
							}
							else{
								/*bool is_ok = false;
								ENTITY* common_vertex;
								ENTITY_LIST acis_vertex1, acis_vertex2;
								api_get_vertices(entity1, acis_vertex1);
								api_get_vertices(entity2, acis_vertex2);
								if((acis_vertex1[0] == acis_vertex2[0]) && (acis_vertex1[1] == acis_vertex2[1])){
									SPAposition c_pos1, c_pos2;
									double dist1(0), dist2(0);
									SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
									api_entity_point_distance(acis_vertex1[0],s_pos,c_pos1,dist1);
									api_entity_point_distance(acis_vertex1[1],s_pos,c_pos2,dist2);
									if(dist1 < dist2){
										V_ENTITY_PTR[new_vertex] = (unsigned long)acis_vertex1[0];
										mesh->set_vertex(new_vertex,SPA2POS(c_pos1));
									}
									else{
										V_ENTITY_PTR[new_vertex] = (unsigned long)acis_vertex1[1];
										mesh->set_vertex(new_vertex,SPA2POS(c_pos2));
									}
								}
								else if((acis_vertex1[0] == acis_vertex2[1]) && (acis_vertex1[1] == acis_vertex2[0])){
									SPAposition c_pos1, c_pos2;
									double dist1(0), dist2(0);
									SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
									api_entity_point_distance(acis_vertex1[0],s_pos,c_pos1,dist1);
									api_entity_point_distance(acis_vertex1[1],s_pos,c_pos2,dist2);
									if(dist1 < dist2){
										V_ENTITY_PTR[new_vertex] = (unsigned long)acis_vertex1[0];
										mesh->set_vertex(new_vertex,SPA2POS(c_pos1));
									}
									else{
										V_ENTITY_PTR[new_vertex] = (unsigned long)acis_vertex1[1];
										mesh->set_vertex(new_vertex,SPA2POS(c_pos2));
									}
								}
								else {
									for(int i = 0; i < acis_vertex1.count(); i++){
										if(acis_faces2.lookup(acis_vertex1[i]) != -1){
											common_vertex = acis_vertex1[i];
											is_ok = true;
											break;
										}
									}
									if(is_ok){
										V_ENTITY_PTR[new_vertex] = (unsigned long)common_vertex;
										SPAposition c_pos;
										double dist = 0.0f;
										SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
										api_entity_point_distance(common_vertex,s_pos,c_pos,dist);
										mesh->set_vertex(new_vertex,SPA2POS(c_pos));
									}*/
									//else
									V_ENTITY_PTR[new_vertex] = (unsigned long)entity1;
									SPAposition c_pos;
									double dist = 0.0f;
									SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
									api_entity_point_distance(entity1,s_pos,c_pos,dist);
									mesh->set_vertex(new_vertex,SPA2POS(c_pos));
							}
						}
					}
					else if(is_EDGE(entity1) && is_FACE(entity2)){
						ENTITY_LIST acis_edges;
						api_get_edges(entity2, acis_edges);
						if(acis_edges.lookup(entity1) != -1){
							V_ENTITY_PTR[new_vertex] = (unsigned long)entity2;
							SPAposition c_pos;
							double dist = 0.0f;
							SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
							api_entity_point_distance(entity2,s_pos,c_pos,dist);
							mesh->set_vertex(new_vertex,SPA2POS(c_pos));
						}
						else{
							V_ENTITY_PTR[new_vertex] = 0;
							//球
							//对球特殊处理
							/*ENTITY_LIST all_faces;
							api_get_faces(body, all_faces);
							SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
							SPAposition c_pos0;
							double dist0 = 0.0f;
							api_entity_point_distance(all_faces[0],s_pos,c_pos0,dist0);
							SPAposition c_pos1;
							double dist1 = 0.0f;
							api_entity_point_distance(all_faces[1],s_pos,c_pos1,dist1);
							SPAposition c_pos2;
							double dist2 = 0.0f;
							api_entity_point_distance(all_faces[2],s_pos,c_pos2,dist2);
							SPAposition c_pos3;
							double dist3 = 0.0f;
							api_entity_point_distance(all_faces[3],s_pos,c_pos3,dist3);
							if(dist0 <= dist1 && dist0 <= dist2 && dist0 <= dist3){
								V_ENTITY_PTR[new_vertex] = (unsigned long)all_faces[0];
								mesh->set_vertex(new_vertex,SPA2POS(c_pos0));
							}
							else if(dist1 <= dist0 && dist1 <= dist2 && dist1 <= dist3){
								V_ENTITY_PTR[new_vertex] = (unsigned long)all_faces[1];
								mesh->set_vertex(new_vertex,SPA2POS(c_pos1));
							}
							else if(dist2 <= dist0 && dist2 <= dist1 && dist2 <= dist3){
								V_ENTITY_PTR[new_vertex] = (unsigned long)all_faces[2];
								mesh->set_vertex(new_vertex,SPA2POS(c_pos2));
							}
							else if(dist3 <= dist0 && dist3 <= dist1 && dist3 <= dist2){
								V_ENTITY_PTR[new_vertex] = (unsigned long)all_faces[3];
								mesh->set_vertex(new_vertex,SPA2POS(c_pos3));
							}*/
						}
					}
					else if(is_EDGE(entity2) && is_FACE(entity1)){
						ENTITY_LIST acis_edges;
						api_get_edges(entity1, acis_edges);
						if(acis_edges.lookup(entity2) != -1){
							V_ENTITY_PTR[new_vertex] = (unsigned long)entity1;
							SPAposition c_pos;
							double dist = 0.0f;
							SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
							api_entity_point_distance(entity1,s_pos,c_pos,dist);
							mesh->set_vertex(new_vertex,SPA2POS(c_pos));
						}
						else{
							V_ENTITY_PTR[new_vertex] = 0;
							//对球特殊处理
							/*ENTITY_LIST all_faces;
							api_get_faces(body, all_faces);
							SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
							SPAposition c_pos0;
							double dist0 = 0.0f;
							api_entity_point_distance(all_faces[0],s_pos,c_pos0,dist0);
							SPAposition c_pos1;
							double dist1 = 0.0f;
							api_entity_point_distance(all_faces[1],s_pos,c_pos1,dist1);
							SPAposition c_pos2;
							double dist2 = 0.0f;
							api_entity_point_distance(all_faces[2],s_pos,c_pos2,dist2);
							SPAposition c_pos3;
							double dist3 = 0.0f;
							api_entity_point_distance(all_faces[3],s_pos,c_pos3,dist3);
							if(dist0 <= dist1 && dist0 <= dist2 && dist0 <= dist3){
								V_ENTITY_PTR[new_vertex] = (unsigned long)all_faces[0];
								mesh->set_vertex(new_vertex,SPA2POS(c_pos0));
							}
							else if(dist1 <= dist0 && dist1 <= dist2 && dist1 <= dist3){
								V_ENTITY_PTR[new_vertex] = (unsigned long)all_faces[1];
								mesh->set_vertex(new_vertex,SPA2POS(c_pos1));
							}
							else if(dist2 <= dist0 && dist2 <= dist1 && dist2 <= dist3){
								V_ENTITY_PTR[new_vertex] = (unsigned long)all_faces[2];
								mesh->set_vertex(new_vertex,SPA2POS(c_pos2));
							}
							else if(dist3 <= dist0 && dist3 <= dist1 && dist3 <= dist2){
								V_ENTITY_PTR[new_vertex] = (unsigned long)all_faces[3];
								mesh->set_vertex(new_vertex,SPA2POS(c_pos3));
							}*/
						}
					}
					else if(is_FACE(entity1) && is_FACE(entity2) && (entity1 == entity2)){
						V_ENTITY_PTR[new_vertex] = (unsigned long)entity1;
						SPAposition c_pos;
						double dist = 0.0f;
						SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
						api_entity_point_distance(entity1,s_pos,c_pos,dist);
						mesh->set_vertex(new_vertex,SPA2POS(c_pos));
					}
					else{
						V_ENTITY_PTR[new_vertex] = 0;
						//对球特殊处理
						//对球特殊处理
						/*ENTITY_LIST all_faces;
						api_get_faces(body, all_faces);
						SPAposition s_pos = POS2SPA(mesh->vertex (new_vertex));
						SPAposition c_pos0;
						double dist0 = 0.0f;
						api_entity_point_distance(all_faces[0],s_pos,c_pos0,dist0);
						SPAposition c_pos1;
						double dist1 = 0.0f;
						api_entity_point_distance(all_faces[1],s_pos,c_pos1,dist1);
						SPAposition c_pos2;
						double dist2 = 0.0f;
						api_entity_point_distance(all_faces[2],s_pos,c_pos2,dist2);
						SPAposition c_pos3;
						double dist3 = 0.0f;
						api_entity_point_distance(all_faces[3],s_pos,c_pos3,dist3);
						if(dist0 <= dist1 && dist0 <= dist2 && dist0 <= dist3){
							V_ENTITY_PTR[new_vertex] = (unsigned long)all_faces[0];
							mesh->set_vertex(new_vertex,SPA2POS(c_pos0));
						}
						else if(dist1 <= dist0 && dist1 <= dist2 && dist1 <= dist3){
							V_ENTITY_PTR[new_vertex] = (unsigned long)all_faces[1];
							mesh->set_vertex(new_vertex,SPA2POS(c_pos1));
						}
						else if(dist2 <= dist0 && dist2 <= dist1 && dist2 <= dist3){
							V_ENTITY_PTR[new_vertex] = (unsigned long)all_faces[2];
							mesh->set_vertex(new_vertex,SPA2POS(c_pos2));
						}
						else if(dist3 <= dist0 && dist3 <= dist1 && dist3 <= dist2){
							V_ENTITY_PTR[new_vertex] = (unsigned long)all_faces[3];
							mesh->set_vertex(new_vertex,SPA2POS(c_pos3));
						}*/
					}
				}
			}
			else{
				reasonable_pos = mesh->vertex (vh);
				new_vertex = mesh->add_vertex (reasonable_pos);
				V_ENTITY_PTR[new_vertex] = V_ENTITY_PTR[vh];
			}
			new_original_vhs_mapping.insert(std::make_pair(new_vertex, vh));
			new_vertices.insert(new_vertex);
			newly_vertices_distribution.insert(std::make_pair(new_vertex, one_chs_group));
			foreach(auto &ch, one_chs_group){
				if (ch != mesh->InvalidCellHandle)
					all_adj_cells.insert(ch);
			}
		}
		newly_created_vertices_cells_mapping->insert(std::make_pair(vh, newly_vertices_distribution));
	}//end foreach (auto &vh, all_vhs) {...


	//由于后面进行garbage_collection的时候会删除掉一些点，然后OVM内部会重新计算顶点的句柄数值
	//所以需要预先保存一下旧的顶点对应的几何位置以及ENTITY_PTR的值
	struct PointInfo{
		PointInfo(){
			pos = OvmVec3d(0, 0, 0);
			entity_ptr = -1;
		}
		OvmVec3d pos;
		unsigned long entity_ptr;
	};

	std::hash_map<OvmVeH, PointInfo> vh_info_for_readd;
	foreach(auto &ch, all_adj_cells){
		for (auto v_it = mesh->hv_iter(ch); v_it; ++v_it){
			auto vh = *v_it;
			if (!contains(all_vhs_on_fhs, vh)){
				PointInfo point_info;
				point_info.pos = mesh->vertex(vh);
				point_info.entity_ptr = V_ENTITY_PTR[vh];
				vh_info_for_readd.insert(std::make_pair(vh, point_info));
			}
		}
	}
	foreach(auto &vh, new_vertices){
		PointInfo point_info;
		point_info.pos = mesh->vertex(vh);
		point_info.entity_ptr = V_ENTITY_PTR[vh];
		vh_info_for_readd.insert(std::make_pair(vh, point_info));
	}

	//auto old_mesh_sheet_ptr = new std::map<std::set<OvmVeH>, unsigned long>();
	//auto old_mesh_chord_ptr = new std::map<std::set<OvmVeH>, unsigned long>();

	//搜集那些需要重建的六面体的八个顶点
	std::hash_map<OvmCeH, std::vector<OvmVeH> > cells_rebuilding_recipes;
	foreach(auto &ch, all_adj_cells) {
		std::vector<OvmVeH> ch_vhs;
		std::hash_map<OvmVeH, OvmVeH> curr_old_new_vhs_mapping;//存储当前六面体中新旧点的映射关系
		for (auto hv_it = mesh->hv_iter(ch); hv_it; ++hv_it) {
			auto newly_vh = fGetNewVeHOnCeH(*hv_it, ch);
			if (newly_vh == mesh->InvalidVertexHandle){
				ch_vhs.push_back(*hv_it);
				curr_old_new_vhs_mapping[*hv_it] = *hv_it;
			}
			else{
				ch_vhs.push_back(newly_vh);
				curr_old_new_vhs_mapping[*hv_it] = newly_vh;
			}
		}
		cells_rebuilding_recipes.insert(std::make_pair(ch, ch_vhs));
		std::unordered_set<OvmEgH> ehs_on_ch;
		get_adj_edges_around_cell(mesh, ch, ehs_on_ch);

		/*foreach(auto &eh, ehs_on_ch){
			auto vh1 = curr_old_new_vhs_mapping[mesh->edge(eh).to_vertex()],
			vh2 = curr_old_new_vhs_mapping[mesh->edge(eh).from_vertex()];
			std::set<OvmVeH> vhs_set;
			vhs_set.insert(vh1); vhs_set.insert(vh2);
			(*old_mesh_sheet_ptr)[vhs_set] = E_SHEET_PTR[eh];
			(*old_mesh_chord_ptr)[vhs_set] = E_CHORD_PTR[eh];
			}*/
	}//end foreach (auto &ch, all_adj_cells) {...

	//搜集那些普通区域（即不是在交叉区域）上的新的六面体的八个顶点
	std::vector<std::vector<OvmVeH> > ord_newly_created_cells_recipes;
	foreach(auto &fh, inflation_quad_set) {
		auto hfh1 = mesh->halfface_handle(fh, 0), hfh2 = mesh->halfface_handle(fh, 1);
		auto ch1 = mesh->incident_cell(hfh1), ch2 = mesh->incident_cell(hfh2);
		std::vector<OvmVeH> ch_vhs;
		for (auto hfv_it = mesh->hfv_iter(hfh1); hfv_it; ++hfv_it){
			auto newly_vh = fGetNewVeHOnCeH(*hfv_it, ch1);
			assert(newly_vh != mesh->InvalidCellHandle);
			ch_vhs.push_back(newly_vh);
		}

		for (auto hfv_it = mesh->hfv_iter(hfh2); hfv_it; ++hfv_it){
			auto newly_vh = fGetNewVeHOnCeH(*hfv_it, ch2);
			assert(newly_vh != mesh->InvalidCellHandle);
			ch_vhs.push_back(newly_vh);
		}
		assert(ch_vhs.size() == 8);
		ord_newly_created_cells_recipes.push_back(ch_vhs);
	}

	//搜集交叉区域新的六面体的八个顶点
	std::vector<std::vector<OvmVeH> > int_newly_created_cells_recipes;
	foreach(auto &eh, int_ehs) {
		auto heh = mesh->halfedge_handle(eh, 0);
		std::vector<OvmVeH> ch_vhs_up, ch_vhs_down;
		auto vh_up_origin = mesh->halfedge(heh).from_vertex(),
			vh_down_origin = mesh->halfedge(heh).to_vertex();
		//use the intersecting halfedges to find the adjacent cell groups,
		//one cell group indicates one newly created vertex
		for (auto hehf_it = mesh->hehf_iter(heh); hehf_it; ++hehf_it){
			auto fh = mesh->face_handle(*hehf_it);
			//if fh is in fhs, one cell group is found
			if (inflation_quad_set.find(fh) != inflation_quad_set.end()) {
				//get the 1st newly created vertex
				auto test_ch = mesh->incident_cell(*hehf_it);
				auto newly_vh = fGetNewVeHOnCeH(vh_up_origin, test_ch);
				assert(newly_vh != mesh->InvalidVertexHandle);
				ch_vhs_up.push_back(newly_vh);

				//get the 2nd newly created vertex
				newly_vh = fGetNewVeHOnCeH(vh_down_origin, test_ch);
				assert(newly_vh != mesh->InvalidVertexHandle);
				ch_vhs_down.push_back(newly_vh);
			}
		}//end for (auto hehf_it = mesh->hehf_iter (heh); hehf_it; ++hehf_it)...
		assert(ch_vhs_up.size() == 4 && ch_vhs_down.size() == 4);
		qSwap(ch_vhs_up[1], ch_vhs_up[3]);
		ch_vhs_up.insert(ch_vhs_up.end(), ch_vhs_down.begin(), ch_vhs_down.end());
		int_newly_created_cells_recipes.push_back(ch_vhs_up);
	}


	OpenVolumeMesh::StatusAttrib status_attrib(*mesh);
	//首先删除旧的需要删除的顶点，连带删除了与此相邻的六面体
	foreach(auto &ch, all_adj_cells)
		status_attrib[ch].set_deleted(true);

	status_attrib.garbage_collection(true);


	//old_new_vhs_mapping存储旧的点（经过前面分裂后的点，并不是original的点）和新的点之间的对应关系
	//由于是分裂后的点，因此这个对应关系是一对一映射
	//std::hash_map<OvmVeH, OvmVeH> old_new_vhs_mapping;
	for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it){
		auto new_vh = *v_it;
		auto ori_vh = V_PREV_HANDLE[new_vh];
		if (ori_vh != mesh->InvalidVertexHandle)
			old_new_vhs_mapping.insert(std::make_pair(ori_vh, new_vh));
	}

	//vhs是之前保存的点序列，fUpdateVhs用于更新这个点序列，
	//如果vhs中的点在old_new_vhs_mapping中能够找到，说明garbage_collection过程中并没有将其删除，
	//因此OpenVolumeMesh自动更新了V_PREV_HANDLE中的属性；
	//如果vhs中的点不能够在old_new_vhs_mapping中找到，说明在garbage_collection中删除了，
	//因此需要根据之前保存着vh_info_db中该点的几何坐标位置及其他信息将其重建，同时更新V_PREV_HANDLE该点存储的信息
	auto fUpdateVhs = [&](std::vector<OvmVeH> &vhs){
		for (int i = 0; i != vhs.size(); ++i) {
			auto old_vh = vhs[i];
			auto locate = old_new_vhs_mapping.find(old_vh);
			if (locate != old_new_vhs_mapping.end())
				vhs[i] = locate->second;
			else{
				auto locate = vh_info_for_readd.find(old_vh);
				assert(locate != vh_info_for_readd.end());
				auto new_vh = mesh->add_vertex(locate->second.pos);
				V_PREV_HANDLE[new_vh] = old_vh;
				V_ENTITY_PTR[new_vh] = locate->second.entity_ptr;
				vhs[i] = new_vh;
				old_new_vhs_mapping.insert(std::make_pair(old_vh, new_vh));
			}
		}
	};

	for (auto it = cells_rebuilding_recipes.begin(); it != cells_rebuilding_recipes.end(); ++it){
		fUpdateVhs(it->second);
		auto ch = mesh->add_cell(it->second);
		assert(ch != mesh->InvalidCellHandle);
	}

	std::unordered_set<OvmCeH> new_chs;

	for (int i = 0; i != ord_newly_created_cells_recipes.size(); ++i){
		auto &vhs = ord_newly_created_cells_recipes[i];
		fUpdateVhs(vhs);
		auto ch = mesh->add_cell(vhs);
		assert(ch != mesh->InvalidCellHandle);
		new_chs.insert(ch);
	}

	for (int i = 0; i != int_newly_created_cells_recipes.size(); ++i){
		auto &vhs = int_newly_created_cells_recipes[i];
		fUpdateVhs(vhs);
		auto ch = mesh->add_cell(vhs);
		assert(ch != mesh->InvalidCellHandle);
		new_chs.insert(ch);
	}

	std::vector<DualSheet *> new_sheets;
	auto fIsNewSheet = [&](std::unordered_set<OvmCeH> &chs)->bool{
		foreach(auto &ch, chs){
			if (!contains(new_chs, ch)) return false;
		}
		return true;
	};

	while (!new_chs.empty()){
		DualSheet *new_sheet = new DualSheet(mesh);

		std::unordered_set<OvmEgH> all_ehs_of_first_ch;
		OvmCeH first_ch = *(new_chs.begin());
		auto hfh_vec = mesh->cell(first_ch).halffaces();
		foreach(auto &hfh, hfh_vec){
			auto heh_vec = mesh->halfface(hfh).halfedges();
			foreach(auto &heh, heh_vec)
				all_ehs_of_first_ch.insert(mesh->edge_handle(heh));
		}

		forever{
			assert(!all_ehs_of_first_ch.empty());
			auto test_eh = *(all_ehs_of_first_ch.begin());
			std::unordered_set<OvmEgH> sheet_ehs;
			std::unordered_set<OvmCeH> sheet_chs;
			retrieve_one_sheet(mesh, test_eh, sheet_ehs, sheet_chs);
			if (fIsNewSheet(sheet_chs)){
				new_sheet->ehs = sheet_ehs;
				new_sheet->chs = sheet_chs;
				break;
			}
			else{
				foreach(auto &eh, sheet_ehs)
					all_ehs_of_first_ch.erase(eh);
			}
		}
		/*foreach(auto &eh, new_sheet->ehs)
			E_SHEET_PTR[eh] = (unsigned long)new_sheet;*/

		foreach(auto &ch, new_sheet->chs)
			new_chs.erase(ch);
		new_sheets.push_back(new_sheet);
	}

	/*for (auto e_it = mesh->edges_begin(); e_it != mesh->edges_end(); ++e_it){
		if (E_SHEET_PTR[*e_it]) continue;
		auto vh1 = mesh->edge(*e_it).to_vertex(),
		vh2 = mesh->edge(*e_it).from_vertex();
		auto old_vh1 = V_PREV_HANDLE[vh1], old_vh2 = V_PREV_HANDLE[vh2];
		std::set<OvmVeH> vhs_set;
		vhs_set.insert(old_vh1); vhs_set.insert(old_vh2);
		auto locate = old_mesh_sheet_ptr->find(vhs_set);
		assert(locate != old_mesh_sheet_ptr->end());
		E_SHEET_PTR[*e_it] = locate->second;
		}*/

	/*for (auto e_it = mesh->edges_begin(); e_it != mesh->edges_end(); ++e_it){
		if (E_CHORD_PTR[*e_it]) continue;
		auto vh1 = mesh->edge(*e_it).to_vertex(),
		vh2 = mesh->edge(*e_it).from_vertex();
		auto old_vh1 = V_PREV_HANDLE[vh1], old_vh2 = V_PREV_HANDLE[vh2];
		std::set<OvmVeH> vhs_set;
		vhs_set.insert(old_vh1); vhs_set.insert(old_vh2);
		auto locate = old_mesh_chord_ptr->find(vhs_set);
		if (locate != old_mesh_chord_ptr->end())
		E_CHORD_PTR[*e_it] = locate->second;
		}*/

	//clear
	delete newly_created_vertices_cells_mapping;
	//delete old_mesh_chord_ptr;
	//delete old_mesh_sheet_ptr;
	return new_sheets;
}

bool bulid_funmesh(VolumeMesh *mesh, BODY *body)
{
	std::vector<std::vector<FACE*>> planar_faces;
	std::unordered_set<OvmCeH> shrink_set;
	std::unordered_set<OvmFaH> quad_set;
	std::vector<OvmVec3d> normals;
	std::vector<OvmVec3d> poses;

	ENTITY_LIST faces_list;
	api_get_faces(body, faces_list);
	ENTITY * face = nullptr;
	faces_list.init();
	//先将所有的平面进行分组（若两个面的方程完全相同，则分为一组）
	while(face = faces_list.next()){
		if(is_planar_face(face)){
			FACE* f_temp = (FACE*) face;
			surface *surf = f_temp->geometry()->trans_surface();
			auto pos = ((plane *)surf)->root_point;
			auto unit_vec = ((plane *)surf)->normal;

			int same_index = 1000;
			for(int i = 0; i < normals.size(); i++){
				if(abs(dot(normals[i], SPA2POS(unit_vec))) > 0.9999){
					auto vec_dis = SPA2POS(pos) - poses[i];
					if(abs(dot(vec_dis, SPA2POS(unit_vec))) < 0.0001){
						same_index = i;
						break;
					}
				}
			}

			if(same_index == 1000){
				std::vector<FACE*> planar_face;
				planar_face.push_back(f_temp);
				planar_faces.push_back(planar_face);
				normals.push_back(SPA2POS(unit_vec));
				poses.push_back(SPA2POS(pos));
			}
			else{
				planar_faces[same_index].push_back(f_temp);
			}
		}
	}

	//处理每一组平面
	for(int round = 0; round < normals.size(); round++){
		std::map<FACE*, std::unordered_set<OvmFaH>> adj_fhs_of_FACEs;
		for(int i = 0; i < planar_faces[round].size(); i++){
			std::unordered_set<OvmFaH> fhs_temp;
			adj_fhs_of_FACEs.insert(std::make_pair(planar_faces[round][i], fhs_temp));
		}
		for(auto fh_iter = mesh->faces_begin(); fh_iter != mesh->faces_end(); fh_iter++){
			if(!mesh->is_boundary(*fh_iter))
				continue;
			FACE* f = get_associated_geometry_face_of_boundary_fh(mesh, *fh_iter);
			if(adj_fhs_of_FACEs.find(f) != adj_fhs_of_FACEs.end())
				adj_fhs_of_FACEs[f].insert(*fh_iter);
		}
		std::vector<std::unordered_set<OvmFaH>> fhses;
		for(auto iter = adj_fhs_of_FACEs.begin(); iter != adj_fhs_of_FACEs.end(); iter++)
			fhses.push_back(iter->second);
		quad_set.clear();
		shrink_set.clear();
		build_funsheet_planar_face(mesh, normals[round], poses[round], fhses, quad_set, shrink_set);
		/*hoopsview->rerender_hexamesh(mesh);
		hoopsview->derender_all_mesh_groups();
		auto group = new VolumeMeshElementGroup (mesh, "lsi", "quad_set");
		group->fhs = quad_set;
		group->chs = shrink_set;
		hoopsview->render_mesh_group (group, false, NULL, NULL, "(diffuse=red,transmission = (r=0.2 g=0.2 b=0.2))");*/
		std::vector<DualSheet*> sheets = one_simple_sheet_inflation (mesh, quad_set, shrink_set, std::unordered_set<OvmEgH>() , std::hash_map<OvmVeH, OvmVeH>());
		smooth_volume_mesh(mesh, body, 1);
		//hoopsview->rerender_hexamesh(mesh);
		//hoopsview->render_mesh_group (sheets[0], false, 0, 0, 0, "red");
	}
	return true;
}

void build_funsheet_planar_face(VolumeMesh* mesh, OvmVec3d normal, OvmVec3d pos, std::vector<std::unordered_set<OvmFaH>> &fhs, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set)
{
	//将六面体集合分成两部分
	std::unordered_set<OvmCeH> hexs_1,hexs_2;
	for(auto c_iter = mesh->cells_begin(); c_iter != mesh->cells_end(); ++c_iter){
		OvmVec3d center_hex(0,0,0);
		int one_side_count = 0;
		for(auto cv_iter = mesh->cv_iter(*c_iter); cv_iter; ++cv_iter){
			OvmVec3d p_temp(mesh->vertex(*cv_iter).data());
			center_hex += p_temp;
		}
		center_hex /= 8;
		OvmVec3d vec = center_hex - pos;
		if(dot(vec,normal) >= 0)
			hexs_1.insert(*c_iter);
		else
			hexs_2.insert(*c_iter);
	}
	//对这两部分面集进行简单的优化


	//找出两部分集合中间相交的四边形面集
	std::unordered_set<OvmFaH> result_fhs;
	std::unordered_set<OvmFaH> bound_fhs;
	collect_boundary_element (mesh, hexs_1, NULL, NULL, &bound_fhs);
	foreach (auto &fh, bound_fhs){
		if (!mesh->is_boundary (fh)) 
			result_fhs.insert (fh);
	}
	for(int i = 0; i < fhs.size(); i++){
		foreach(auto &fh, fhs[i])
			result_fhs.insert(fh);
	}

	//将面集分成若干连通的四边形面集
	std::vector<std::unordered_set<OvmFaH>> connected_patchs;
	std::unordered_set<int> fhs_temp;
	for (auto fh_iter = result_fhs.begin(); fh_iter != result_fhs.end(); ++fh_iter)
		fhs_temp.insert(*fh_iter);

	auto find_all_adj_face = [&](OvmFaH f,std::vector<OvmFaH>& adj_face_on_resultfhs){
		auto hehs = mesh->face(f).halfedges();
		foreach(auto he,hehs){
			OvmEgH eh = mesh->edge_handle(he);
			std::vector<OvmFaH> adj_fhs;
			adj_fhs = get_adj_faces_around_edge(mesh,eh);//一个边周围的面集只可能有一个符合要求
			foreach(auto adj_f, adj_fhs){
				auto find_fh = fhs_temp.find(adj_f);
				if (find_fh != fhs_temp.end())
				{
					adj_face_on_resultfhs.push_back(adj_f);
					fhs_temp.erase(find_fh);
				}
			}
		}
	};

	//将面集分为若干个连通子图
	do 
	{
		std::unordered_set<OvmFaH> one_patch;
		auto first_fh = fhs_temp.begin();
		std::unordered_set<OvmFaH> temp_adj_face_on_resultfhs;
		std::vector<OvmFaH> first_adj_face;
		find_all_adj_face(*first_fh,first_adj_face);
		one_patch.insert(first_adj_face.begin(),first_adj_face.end());
		temp_adj_face_on_resultfhs.insert(first_adj_face.begin(),first_adj_face.end());
		std::unordered_set<OvmFaH> next_adj;
		do
		{
			next_adj.clear();
			if (temp_adj_face_on_resultfhs.size() > 0)
			{
				foreach(auto fh,temp_adj_face_on_resultfhs){
					std::vector<OvmFaH> adj_face;
					find_all_adj_face(fh,adj_face);
					next_adj.insert(adj_face.begin(),adj_face.end());
					one_patch.insert(adj_face.begin(),adj_face.end());
				}
			}
			temp_adj_face_on_resultfhs.clear();	
			temp_adj_face_on_resultfhs.insert(next_adj.begin(),next_adj.end());
		}while (temp_adj_face_on_resultfhs.size() > 0);

		connected_patchs.push_back(one_patch);
	} while (fhs_temp.size() > 0);

	//找出包含输入面集的所有子图
	for(int i = 0; i < fhs.size(); i++){
		OvmFaH seed_fh = *fhs[i].begin();
		foreach(auto patch, connected_patchs){
			if(contains(patch, seed_fh)){
				foreach(auto fh, patch)
					quad_set.insert(fh);
				break;
			}
		}
	}


	//将这层点投影到这个平面上，提高它们的几何质量
	std::unordered_set<OvmVeH> vhs;
	foreach(auto &fh, result_fhs){
		auto hfh = mesh->halfface_handle(fh, 0);
		for(auto hfv_iter = mesh->hfv_iter(hfh); hfv_iter; hfv_iter++)
			vhs.insert(*hfv_iter);
	}
	foreach(OvmVeH vh, vhs){
		OvmVec3d p_vh = mesh->vertex(vh);
		OvmVec3d vec = p_vh - pos;
		double dis = dot(vec, normal);
		OvmVec3d p_final = p_vh - dis*normal;
		mesh->set_vertex(vh, p_final);
	}
	OvmCeH seed_ch;
	OvmFaH seed_fh = *fhs[0].begin();
	OvmHaFaH hfh1 = mesh->halfface_handle(seed_fh, 0);
	OvmHaFaH hfh2 = mesh->halfface_handle(seed_fh, 1);
	if(mesh->incident_cell(hfh1) == mesh->InvalidCellHandle)
		seed_ch = mesh->incident_cell(hfh2);
	else
		seed_ch = mesh->incident_cell(hfh1);

	std::unordered_set<OvmCeH> hexs;
	if(contains(hexs_1, seed_ch))
		hexs = hexs_1;
	else
		hexs = hexs_2;

	foreach(auto vh, vhs){
		for(auto iter = mesh->vc_iter(vh); iter; iter++)
			if(contains(hexs, *iter))
				shrink_set.insert(*iter);
	}
}

void get_new_fhs_global(VolumeMesh* mesh,OvmVec3d& normal,OvmVec3d& pos,std::vector<std::unordered_set<OvmFaH>>& result_fh_vec, std::unordered_set<OvmCeH> & shrink_set)
{
	//直接全局找,首先确定出六面体集合后，直接提取表面
	std::unordered_set<OvmCeH> hexs_1,hexs_2;

	for(auto heh_iter = mesh->cells_begin(); heh_iter != mesh->cells_end(); ++heh_iter){
		OvmVec3d center_hex(0,0,0);
		int one_side_count = 0;
		for(auto hv_iter = mesh->cv_iter(*heh_iter); hv_iter; ++hv_iter){
			OvmVec3d p_temp(mesh->vertex(*hv_iter).data());
			OvmVec3d vec = p_temp - pos;
			if(dot(vec,normal) >= 0)
				one_side_count++;
			//center_hex += p_temp;
		}
		////center_hex = center_hex/8;
		OvmVec3d vec = center_hex - pos;
		//if(dot(vec,normal) >= 0)
		//hexs_1.insert(*heh_iter);
		//else
		//hexs_2.insert(*heh_iter);
		if (one_side_count>=4)
			hexs_1.insert(*heh_iter);
		else
			hexs_2.insert(*heh_iter);
	}

	std::unordered_set<OvmFaH> result_fhs;
	result_fhs.clear ();
	std::unordered_set<OvmFaH> bound_fhs;
	collect_boundary_element (mesh, hexs_1, NULL, NULL, &bound_fhs);

	foreach (auto &fh, bound_fhs){
		if (!mesh->is_boundary (fh)) 
			result_fhs.insert (fh);
	}
	foreach(auto ch, hexs_1)
		shrink_set.insert(ch);


	std::unordered_set<int> face_h;
	for (auto it = result_fhs.begin(); it != result_fhs.end(); ++it)
	{
		face_h.insert(*it);
	}
	auto find_all_adj_face = [&](OvmFaH f,std::vector<OvmFaH>& adj_face_on_resultfhs){
		auto hehs = mesh->face(f).halfedges();
		foreach(auto he,hehs){
			OvmEgH eh = mesh->edge_handle(he);
			std::vector<OvmFaH> adj_fhs;
			adj_fhs = get_adj_faces_around_edge(mesh,eh);//一个边周围的面集只可能有一个符合要求
			foreach(auto adj_f, adj_fhs){
				auto find_fh = face_h.find(adj_f);
				if (find_fh != face_h.end())
				{
					adj_face_on_resultfhs.push_back(adj_f);
					face_h.erase(find_fh);
				}
			}
		}
	};

	//将面集分为若干个连通子图
	do 
	{
		std::unordered_set<OvmFaH> one_patch;
		auto first_fh = face_h.begin();
		std::unordered_set<OvmFaH> temp_adj_face_on_resultfhs;
		std::vector<OvmFaH> first_adj_face;
		find_all_adj_face(*first_fh,first_adj_face);
		one_patch.insert(first_adj_face.begin(),first_adj_face.end());
		temp_adj_face_on_resultfhs.insert(first_adj_face.begin(),first_adj_face.end());
		std::unordered_set<OvmFaH> next_adj;

		do
		{
			next_adj.clear();
			if (temp_adj_face_on_resultfhs.size() > 0)
			{
				foreach(auto fh,temp_adj_face_on_resultfhs){
					std::vector<OvmFaH> adj_face;
					find_all_adj_face(fh,adj_face);
					next_adj.insert(adj_face.begin(),adj_face.end());
					one_patch.insert(adj_face.begin(),adj_face.end());
				}
			}
			temp_adj_face_on_resultfhs.clear();	
			temp_adj_face_on_resultfhs.insert(next_adj.begin(),next_adj.end());
		}while (temp_adj_face_on_resultfhs.size() > 0);

		result_fh_vec.push_back(one_patch);
	} while (face_h.size() > 0);
}

bool determine_locations_of_inflation(VolumeMesh *mesh, BODY *body, std::vector<OvmVec3d>& normal,std::vector<OvmVec3d>& positon)
{
	//首先找出整个模型中的所有的几何边
	ENTITY_LIST edges_list;
	api_get_edges(body, edges_list);
	std::vector<ENTITY *> edges;
	entity_list_to_vector(edges_list, edges);

	//根据每条几何边找其相关的几何面，如果两个几何面均为平面，则首先计算两个法向的叉积；然后计算每个几何面的法向

	std::vector<SPAposition> all_mid_pos;
	std::vector<OvmVec3d> all_normal;

	foreach(auto & entity, edges){
		ENTITY_LIST adj_faces;
		api_get_faces(entity, adj_faces);
		std::vector<ENTITY *> adj_faces_vec;
		entity_list_to_vector(adj_faces, adj_faces_vec);

		if(is_planar_face((FACE *) adj_faces_vec[0]) && is_planar_face((FACE *) adj_faces_vec[1])){
			auto mid_pos = ((EDGE*)entity)->mid_pos();
			surface *surface1 = ((FACE*)(adj_faces_vec[0]))->geometry()->trans_surface();
			SPAunit_vector unit_vector1 = surface1->point_normal(mid_pos);
			surface *surface2 = ((FACE*)(adj_faces_vec[1]))->geometry()->trans_surface();
			SPAunit_vector unit_vector2 = surface2->point_normal(mid_pos);

			    OvmVec3d normal = cross(OvmVec3d(unit_vector1.x(), unit_vector1.y(), unit_vector1.z()), OvmVec3d(unit_vector2.x(), unit_vector2.y(), unit_vector2.z()));
				normal = normal.normalize_cond();
				all_mid_pos.push_back(mid_pos);
			    all_normal.push_back(normal);
				//priority_faces_normal.insert(normal);
		}
		else 
		{
				
			if (is_linear_edge((EDGE*)entity))
			{
				auto mid_pos = ((EDGE*)entity)->mid_pos();

				surface *surface1 = ((FACE*)(adj_faces_vec[0]))->geometry()->trans_surface();
				SPAunit_vector unit_vector1 = surface1->point_normal(mid_pos);
				surface *surface2 = ((FACE*)(adj_faces_vec[1]))->geometry()->trans_surface();
				SPAunit_vector unit_vector2 = surface2->point_normal(mid_pos);
				if(unit_vector1 == unit_vector2 || unit_vector1 == -unit_vector2){
					SPAinterval inter = ((EDGE*)entity)->param_range ();
					double step = inter.length () / 2,
						mid_param = inter.start_pt () + step;
					SPAvector tangent = ((EDGE*)entity)->geometry()->equation().eval_deriv(mid_param);
					SPAunit_vector uni_tangent = normalise(tangent);						
					OvmVec3d normal(uni_tangent.x(),uni_tangent.y(),uni_tangent.z());
					all_mid_pos.push_back(mid_pos);
					all_normal.push_back(normal);
				}
				else{
					OvmVec3d normal = cross(OvmVec3d(unit_vector1.x(), unit_vector1.y(), unit_vector1.z()), OvmVec3d(unit_vector2.x(), unit_vector2.y(), unit_vector2.z()));
					normal = normal.normalize_cond();
					all_mid_pos.push_back(mid_pos);
					all_normal.push_back(normal);
				}
			}
			else
			{
					
				SPAinterval inter = ((EDGE*)entity)->param_range ();
				double step = inter.length () / 6,
					start_param = inter.start_pt ();
				curve *crv = ((EDGE*)entity)->geometry ()->trans_curve ();
				std::vector<SPAposition> pts;
				for (int i = 2; i < 5; ++i){
					double param_val = start_param + step * i;
					SPAposition pos;
					crv->eval (param_val, pos);
					surface *surface1 = ((FACE*)(adj_faces_vec[0]))->geometry()->trans_surface();
					SPAunit_vector unit_vector1 = surface1->point_normal(pos);
					surface *surface2 = ((FACE*)(adj_faces_vec[1]))->geometry()->trans_surface();
					SPAunit_vector unit_vector2 = surface2->point_normal(pos);

					if(unit_vector1 == unit_vector2 || unit_vector1 == -unit_vector2){
							
						SPAvector tangent = ((EDGE*)entity)->geometry()->equation().eval_deriv(param_val);
						SPAunit_vector uni_tangent = normalise(tangent);						
						OvmVec3d normal(uni_tangent.x(),uni_tangent.y(),uni_tangent.z());
						all_mid_pos.push_back(pos);
						all_normal.push_back(normal);
					}
					else{
						OvmVec3d normal = cross(OvmVec3d(unit_vector1.x(), unit_vector1.y(), unit_vector1.z()), OvmVec3d(unit_vector2.x(), unit_vector2.y(), unit_vector2.z()));
						normal = normal.normalize_cond();
						all_mid_pos.push_back(pos);
						all_normal.push_back(normal);
					}
				}					
			}				
		}
	}
		
	std::set<int> need_erase_place;
	for (int i = 0; i < (all_mid_pos.size()-1); ++i)
	{
		if (need_erase_place.find(i) != need_erase_place.end()) continue;
		for (int j = i+1; j < all_mid_pos.size(); ++j)
		{
			if (need_erase_place.find(j) != need_erase_place.end()) continue;
			double cos_angle = abs(dot(all_normal[i],all_normal[j]));
			//if (all_normal[i] == all_normal[j] || all_normal[i] == -all_normal[j])
			if(cos_angle > cos(PI/36))
			{
				//plane  p = plane(all_mid_pos[i],SPAunit_vector(all_normal[i][0],all_normal[i][1],all_normal[i][2]));
				double dis = distance_to_plane(all_mid_pos[j],all_mid_pos[i],SPAunit_vector(all_normal[i][0],all_normal[i][1],all_normal[i][2]));
				if (dis < 0.001)
					need_erase_place.insert(j);
			}
		}
	}

		
	for (int i = 0; i < all_normal.size(); ++i)
	{
		if (need_erase_place.find(i) == need_erase_place.end())
		{
			normal.push_back(all_normal[i]);
			positon.push_back(OvmVec3d(all_mid_pos[i].x(),all_mid_pos[i].y(),all_mid_pos[i].z()));
		}
	}

	need_erase_place.clear();
	for (int i = 0; i < (positon.size()-1); ++i)
	{
		if (need_erase_place.find(i) != need_erase_place.end()) continue;
		for (int j = i+1; j < positon.size(); ++j)
		{
			if (need_erase_place.find(j) != need_erase_place.end()) continue;
			double cos_angle = abs(dot(normal[i],normal[j]));
			//if (all_normal[i] == all_normal[j] || all_normal[i] == -all_normal[j])
			if(cos_angle > cos(PI/36))
			{
				//plane  p = plane(all_mid_pos[i],SPAunit_vector(all_normal[i][0],all_normal[i][1],all_normal[i][2]));
				SPAposition sj(positon[j][0],positon[j][1],positon[j][2]);
				SPAposition si(positon[i][0],positon[i][1],positon[i][2]);
				double dis = distance_to_plane(sj,si,SPAunit_vector(normal[i][0],normal[i][1],normal[i][2]));
				if (dis < 0.001)
					need_erase_place.insert(j);
			}
		}
	}
	return false;
}

static bool X_get_shrink_set_on_direction(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmCeH> &shrink_set, int direction)
{
	shrink_set.clear();
	std::queue<OvmHaFaH> queue_hfhs;
	std::unordered_set<OvmFaH> used_fhs;

	auto fh = *(inflation_quad_set.begin());
	auto hfh = mesh->halfface_handle(fh, direction);
	queue_hfhs.push(hfh);

	while (!queue_hfhs.empty()) {
		auto hfh = queue_hfhs.front();
		queue_hfhs.pop();

		if (contains(used_fhs, mesh->face_handle(hfh))) {
			continue;
		}

		auto cell = mesh->incident_cell(hfh);
		// 不能在这个方向生成完整的shrink_set所有及时退出
		// Couldn't generate the full shrink set in the direction, so quit immediately.
		if (cell == mesh->InvalidCellHandle) return false; 

		shrink_set.insert(cell);
		used_fhs.insert(mesh->face_handle(hfh));

		int count_he = 0;
		// 通过半面的四个半边来求和这个半面相邻，且方向一致的半面
		foreach (const auto &heh, mesh->halfface(hfh).halfedges()) {

			// ============ debug ================
			//fout << "+++++++++++++++++++++++++" << std::endl;
			//fout << "count_he: " << ++count_he << std::endl;
			// ============ debug ================

			auto oppo_heh = mesh->opposite_halfedge_handle(heh);
			std::queue<OvmHaFaH> tmp_hfhs; 
			for (auto hehf_it = mesh->hehf_iter(oppo_heh); hehf_it; hehf_it++) {
				auto fh = mesh->face_handle(*hehf_it);
				auto ch = mesh->incident_cell(*hehf_it);
				//if (!contains(used_fhs, fh) && contains(inflation_quad_set, fh)){
				if (!contains(used_fhs, fh) && contains(inflation_quad_set, fh)){
					tmp_hfhs.push(*hehf_it);
				}
				if (fh == mesh->face_handle(hfh)) {
					tmp_hfhs.push(*hehf_it);
				}
			}

			// ============ debug ================
			//fout << "---------------------" << std::endl;
			//fout << "tmp_hfhs imformation:" << std::endl;
			//fout << "size of tmp_hfhs: " << tmp_hfhs.size() << std::endl;
			//fout << "origin fh idx: " << mesh->face_handle(hfh).idx() << std::endl;
			//
			//std::queue<OvmHaFaH> debug_queue = tmp_hfhs;
			//while (!debug_queue.empty()) {
			//  fout << "fh info: " << mesh->face_handle(debug_queue.front()) << std::endl;
			//  debug_queue.pop();
			//}

			// ============ debug ================

			//qDebug() << "__LINE__: " << __LINE__;
			//int count_fun = 0;
			while (mesh->face_handle(tmp_hfhs.front()) != mesh->face_handle(hfh)) {
				auto tmp = tmp_hfhs.front();
				tmp_hfhs.pop();
				tmp_hfhs.push(tmp);
				//qDebug() << "count_fun: " << ++count_fun;
			}

			if (tmp_hfhs.size() == 2) {
				tmp_hfhs.pop();
				queue_hfhs.push(tmp_hfhs.front());
			}
			else if (tmp_hfhs.size() == 4) {
				tmp_hfhs.pop();
				tmp_hfhs.pop();
				queue_hfhs.push(tmp_hfhs.front());
			}
			//qDebug() << "__LINE__: " << __LINE__;
		}

		//used_fhs.insert(mesh->face_handle(hfh));
	}

	return true;
}

void get_traditional_shrink_set_for_simple_quad_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set)
{
	shrink_set.clear();

	std::unordered_set<OvmCeH> shrink_set1, shrink_set2;
	bool res1 = X_get_shrink_set_on_direction(mesh, quad_set, shrink_set1, 0);
	bool res2 = X_get_shrink_set_on_direction(mesh, quad_set, shrink_set2, 1);

	if (res1 && res2 == false) {
		if (res1) {
			shrink_set = shrink_set1;
		}
		else {
			shrink_set = shrink_set2;
		}
	}
	else {
		shrink_set = shrink_set2; // 默认选择0方向的shrink set
	}

	// 补全shrink set，得到传统的shrink set
	std::unordered_set<OvmCeH> visited;
	std::queue<OvmCeH> que;

	que.push(*(shrink_set.begin()));
	visited.insert(que.front());
	while (!que.empty()) {
		auto tmp_ch = que.front();
		que.pop();

		foreach (auto &hfh, mesh->cell(tmp_ch).halffaces()) {
			auto fh = mesh->face_handle(hfh);
			if (contains(quad_set, fh)) continue;
			auto oppo_hfh = mesh->opposite_halfface_handle(hfh);
			auto oppo_ch = mesh->incident_cell(oppo_hfh);
			if (oppo_ch == mesh->InvalidCellHandle) continue;
			if (!contains(visited, oppo_ch)) {
				visited.insert(oppo_ch);
				que.push(oppo_ch);
			}
		}
	}

	shrink_set = visited;
}

void optimize_toplogy_shrink_set(VolumeMesh* mesh, std::unordered_set<OvmCeH> &shrink_set, std::unordered_set<OvmEgH> &fixed_ehs, std::unordered_set<OvmFaH> &inner_bound_fhs)
{
	//对于包含边界网格面的quad-set,要把它和内部网格面的交线作为fixed_ehs
	std::unordered_set<OvmFaH> outer_bound_fhs;
	foreach(auto &fh, inner_bound_fhs)
		if(mesh->is_boundary(fh))
			outer_bound_fhs.insert(fh);
	foreach(auto fh, outer_bound_fhs)
		inner_bound_fhs.erase(fh);
	if(!outer_bound_fhs.empty()){
		//outer_bound_fhs的边界线
		std::unordered_set<OvmEgH> bound_ehs1, bound_ehs2;
		collect_boundary_element (mesh, outer_bound_fhs, std::unordered_set<OvmVeH> (), bound_ehs1);
		//inner_bound_fhs的边界线
		collect_boundary_element (mesh, inner_bound_fhs, std::unordered_set<OvmVeH> (), bound_ehs2);
		foreach(auto &eh, bound_ehs1){
			if(contains(bound_ehs2, eh))
				fixed_ehs.insert(eh);
		}
	}
	

	std::unordered_set<OvmCeH> barrier_set;//收缩集一定不能包含的单元
	std::unordered_set<OvmCeH> shrink_set_need_contain;//收缩集一定需要包含的单元
	std::unordered_set<OvmCeH> chs_temp;
	foreach(auto &eh, fixed_ehs)
		get_adj_hexas_around_edge(mesh, eh, chs_temp);
	foreach (auto &ch,chs_temp)
	{
		if (!contains(shrink_set,ch))
			barrier_set.insert(ch);
		else
			shrink_set_need_contain.insert(ch);
	}
	chs_temp.clear();

	//在收缩集里插入一个cell，就更新内部边界
	auto fUpdateBoundaryFhsLocally = [&] (OvmCeH ch){
		auto hfh_vec = mesh->cell (ch).halffaces ();
		foreach (auto &hfh, hfh_vec){
			auto fh = mesh->face_handle (hfh);
			if (mesh->is_boundary (fh)) continue;

			if (contains (inner_bound_fhs, fh))
				inner_bound_fhs.erase (fh);
			else
				inner_bound_fhs.insert (fh);
		}
	};

	//所有四边形面集内部的边，暂仅考虑四边形内部边对拓扑质量的影响
	auto finner_ehs =[&](std::unordered_set<OvmEgH>& inner_ehs){
		std::hash_map<OvmEgH, int> edges_counts;
		foreach(auto &fh,inner_bound_fhs){
			auto hehs_vec = mesh->face(fh).halfedges();
			foreach(auto &heh,hehs_vec){
				OvmEgH eh = mesh->edge_handle(heh);
				edges_counts[eh]++;//一条边在四边形面集上出现的次数
			}
		}
		//std::unordered_set<OvmEgH> inner_ehs;所有四边形面集内部的边
		for (auto it = edges_counts.begin(); it != edges_counts.end(); ++it)
		{
			if (it->second != 1)
				inner_ehs.insert(it->first);
		}
	};

	auto fboundary_ehs =[&](std::unordered_set<OvmEgH>& boundary_ehs){
		std::hash_map<OvmEgH, int> edges_counts;
		foreach(auto &fh,inner_bound_fhs){
			auto hehs_vec = mesh->face(fh).halfedges();
			foreach(auto &heh,hehs_vec){
				OvmEgH eh = mesh->edge_handle(heh);
				edges_counts[eh]++;//一条边在四边形面集上出现的次数
			}
		}
		//std::unordered_set<OvmEgH> inner_ehs;所有四边形面集内部的边
		for (auto it = edges_counts.begin(); it != edges_counts.end(); ++it)
		{
			if (it->second == 1)
				boundary_ehs.insert(it->first);
		}
	};
	//找到一条凹边
	auto fconcave_ehs = [&](std::unordered_set<OvmEgH>& inner_ehs,OvmEgH &concave_ehs)->bool{
		foreach(auto& eh, inner_ehs){
			int valence = mesh->valence(eh); 
			std::unordered_set<OvmCeH> hexas;
			get_adj_hexas_around_edge(mesh, eh, hexas);
			int num_in_hexs1(0);
			foreach(auto & hex, hexas)
				if(contains(shrink_set, hex))
					num_in_hexs1++;
			if(valence % 2 == 0){
				if(num_in_hexs1 > (valence/2)){					
					concave_ehs = eh;
					return true;
				}
			}
			else
			{
				if(num_in_hexs1 > (valence/2 + 1)){
					concave_ehs = eh;
					return true;
				}
			}
		}
		return false;
	};
	//找到一条凸边
	auto fconvex_ehs = [&](std::unordered_set<OvmEgH>& inner_ehs,OvmEgH& convex_ehs)->bool{
		foreach(auto& eh, inner_ehs){
			int valence = mesh->valence(eh); 
			std::unordered_set<OvmCeH> hexas;
			get_adj_hexas_around_edge(mesh, eh, hexas);
			int num_in_hexs1(0);
			foreach(auto & hex, hexas)
				if(contains(shrink_set, hex))
					num_in_hexs1++;
			if(valence % 2 == 0){
				if(num_in_hexs1 < (valence/2)){
					convex_ehs = eh;
					return true;
				}
			}
			else
			{
				if(num_in_hexs1 < (valence/2)){
					convex_ehs = eh;
					return true;
				}
			}
		}
		return false;
	};
	/*assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

	auto find_edge_include_geometry_vertex = [&](std::unordered_set<OvmEgH>& inner_ehs,OvmEgH& ehs)->bool{		
		foreach(auto& eh, inner_ehs){
			OvmVeH from = mesh->edge(eh).from_vertex();
			OvmVeH to = mesh->edge(eh).to_vertex();
			auto entity_from = (ENTITY*)(V_ENTITY_PTR[from]);
			auto entity_to = (ENTITY*)(V_ENTITY_PTR[to]);
			if (is_VERTEX(entity_from) || is_VERTEX(entity_to))
			{
				ehs = eh;
				return true;
			}
		}
		return false;
	};

	auto find_edge_include_geometry_edge = [&](std::unordered_set<OvmEgH>& boundary_ehs,OvmEgH& ehs)->bool{		
		foreach(auto& eh, boundary_ehs){
			OvmVeH from = mesh->edge(eh).from_vertex();
			OvmVeH to = mesh->edge(eh).to_vertex();
			auto entity_from = (ENTITY*)(V_ENTITY_PTR[from]);
			auto entity_to = (ENTITY*)(V_ENTITY_PTR[to]);
			if (is_EDGE(entity_from) && is_EDGE(entity_to) && entity_from == entity_to)
			{
				ehs = eh;
				return true;
			}
		}
		return false;
	};*/


	//填凹边
	std::vector<OvmCeH> exp_chs;
	do 
	{
		exp_chs.clear();
		std::unordered_set<OvmEgH> inner_ehs;//四边形面集内部的边
		finner_ehs(inner_ehs);
		OvmEgH concave_ehs;//所有凹边
		if (fconcave_ehs(inner_ehs,concave_ehs))
		{
			std::unordered_set<OvmCeH> adj_chs;
			get_adj_hexas_around_edge(mesh, concave_ehs, adj_chs);
			foreach (auto &ch, adj_chs){
				if (contains(barrier_set,ch))//如果是在不能允许的cell集合中就跳过
					continue;
				if (!contains (shrink_set, ch)){
					shrink_set.insert(ch);
					fUpdateBoundaryFhsLocally (ch);//局部更新四边形面集
					exp_chs.push_back (ch);//此次加入的六面体单元，如果所有凹边都没有加入那么说明结束
				}
			}
		}
		else
			break;
	} while (exp_chs.size() > 0);
	
	//去凸边
	std::vector<OvmCeH> erase_chs;
	do 
	{
		erase_chs.clear();
		std::unordered_set<OvmEgH> inner_ehs;//四边形面集内部的边
		finner_ehs(inner_ehs);
		OvmEgH convex_ehs;//所有凹边
		if (fconvex_ehs(inner_ehs,convex_ehs))
		{
			std::unordered_set<OvmCeH> adj_chs;
			get_adj_hexas_around_edge(mesh, convex_ehs, adj_chs);
			foreach (auto &ch, adj_chs){
				if (contains(shrink_set_need_contain,ch))//如果是在不能允许的cell集合中就跳过
					continue;
				if (contains (shrink_set, ch)){
					shrink_set.erase(ch);//去凸边
					fUpdateBoundaryFhsLocally (ch);//局部跟新四边形面集
					erase_chs.push_back (ch);//此次加入的六面体单元，如果所有凹边都没有加入那么说明结束

				}
			}
		}
		else
			break;	
	} while (erase_chs.size() > 0);

	if(!outer_bound_fhs.empty()){
		foreach(auto fh, outer_bound_fhs)
			inner_bound_fhs.insert(fh);
	}
}

//根据一个边界网格面，找到它所属的column，可推广到任意网格面
void get_corresponding_column(VolumeMesh* mesh, OvmFaH &boundary_fh, std::vector<OvmCeH> &column)
{
	if(!mesh->is_boundary(boundary_fh))
		return;
	OvmHaFaH hfh = mesh->halfface_handle(boundary_fh, 0);
	if(mesh->is_boundary(hfh))
		hfh = mesh->opposite_halfface_handle(hfh);
	while(!mesh->is_boundary(hfh)){
		OvmCeH ch = mesh->incident_cell(hfh);
		column.push_back(ch);
		hfh = mesh->opposite_halfface_handle_in_cell(hfh, ch);
		hfh = mesh->opposite_halfface_handle(hfh);
	}
}
	