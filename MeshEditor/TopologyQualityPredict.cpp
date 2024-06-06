#include "stdafx.h"
#include "TopologyQualityPredict.h"
#include "topologyoptwidget.h"
#include "PrioritySetManager.h"
#include "GeometryFuncs.h"
#include "FuncDefs.h"
#include <cmath>
#include <geom_utl.hxx>
#include <OpenVolumeMesh/Attribs/StatusAttrib.hh>
#include <OpenVolumeMesh/Core/TopologyKernel.hh>
#include <fstream>
#include <edge.hxx>
#include <plane.hxx>
#include <face.hxx>
#include <unitvec.hxx>
#include <OpenVolumeMesh/FileManager/FileManager.hh>
#define PI 3.1415926
#define P_A 1
#define P_B 0
#define P_C 0
#define P_D 0


void get_edge_property(VolumeMesh *mesh, std::map<OvmEgH, EdgeAttribute> & edge_property)
{
	assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
	for(auto eh_iter = mesh->edges_begin(); eh_iter != mesh->edges_end(); eh_iter++){
		auto eh = *eh_iter;
		if(!mesh->is_boundary(eh)){
			int valence = mesh->valence(eh);
			EdgeAttribute edge_attri;
			edge_attri.edgetype = INNEREDGE;
			edge_attri.valence = valence;
			edge_attri.idealvalence = 4;
			edge_property.insert(std::make_pair(eh, edge_attri));
			continue;
		}
		EDGE* acis_edge = get_associated_geometry_edge_of_boundary_eh(mesh, eh, V_ENTITY_PTR);
		if(acis_edge == NULL){
			int valence = mesh->valence(eh);
			EdgeAttribute edge_attri;
			edge_attri.edgetype = FACEEDGE;
			edge_attri.valence = valence;
			edge_attri.idealvalence = 3;
			edge_property.insert(std::make_pair(eh, edge_attri));
			continue;
		}
		ENTITY_LIST faces_list;
		api_get_faces ((ENTITY *)acis_edge, faces_list);
		FACE *f1 = (FACE*)(faces_list[0]), *f2 = (FACE*)(faces_list[1]);
		double angle = calc_dihedral_angle(f1,f2);
		if(angle>=0 && angle<(0.75*M_PI)){
			int valence = mesh->valence(eh);
			EdgeAttribute edge_attri;
			edge_attri.edgetype = EDGEEDGE;
			edge_attri.valence = valence;
			edge_attri.idealvalence = 2;
			edge_property.insert(std::make_pair(eh, edge_attri));
			continue;
		}
		else if(angle>=0.75*M_PI && angle<(1.25*M_PI)){
			int valence = mesh->valence(eh);
			EdgeAttribute edge_attri;
			edge_attri.edgetype = EDGEEDGE;
			edge_attri.valence = valence;
			edge_attri.idealvalence = 3;
			edge_property.insert(std::make_pair(eh, edge_attri));
			continue;
		}
		else if(angle>=1.25*M_PI && angle<(1.75*M_PI)){
			int valence = mesh->valence(eh);
			EdgeAttribute edge_attri;
			edge_attri.edgetype = EDGEEDGE;
			edge_attri.valence = valence;
			edge_attri.idealvalence = 4;
			edge_property.insert(std::make_pair(eh, edge_attri));
			continue;
		}
		else {
			int valence = mesh->valence(eh);
			EdgeAttribute edge_attri;
			edge_attri.edgetype = EDGEEDGE;
			edge_attri.valence = valence;
			edge_attri.idealvalence = 5;
			edge_property.insert(std::make_pair(eh, edge_attri));
			continue;
		}
	}
}

void get_sheet_data(VolumeMesh* mesh, DualSheet * sheet, Sheet_data &sheet_data)
{
	std::unordered_set<OvmEgH> all_ehs;
	std::vector<std::unordered_set<OvmEgH>> eh_pairs;
	std::unordered_set<OvmEgH> edge_pair;
	std::vector<std::unordered_set<OvmVeH>> vh_pairs;
	std::unordered_set<OvmVeH> vertex_pair;

	//�����ҳ�sheet extraction������Ҫ�����ıߣ������ӱ�
	foreach(auto & eh, sheet->ehs){
		all_ehs.insert(eh);
	}

	//�����е����ӱ߸������ӹ�ϵ���з���
	while (!all_ehs.empty()){
		auto eh = *(all_ehs.begin());
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
				get_adj_edges_around_vertex (mesh,vh,adj_edges);//�������vh�����ӵ����бߵ�handleֵ
				foreach(auto &eh,adj_edges){
					if(all_ehs.find(eh) != all_ehs.end()){
						edge_pair.insert(eh);
						all_ehs.erase(eh);
					}
				}
				adj_edges.clear();
			}
			edge_pair_veh.clear();
			final_number = edge_pair.size();
		} while (original_number != final_number);
		eh_pairs.push_back(edge_pair);
		edge_pair.clear();
	}
	all_ehs.clear();

	//��ȡsheet�����еĶ���
	std::unordered_set<OvmVeH> vhs_on_sheet;
	//�������ֵı��飬�ҳ����Ӧ�ĵ��飬����extraction������Ҫ�ϲ���һ����ĵ���
	foreach(auto & eh_pair, eh_pairs){
		foreach(auto & eh, eh_pair){
			vertex_pair.insert(mesh->edge(eh).from_vertex());
			vertex_pair.insert(mesh->edge(eh).to_vertex());
			vhs_on_sheet.insert(mesh->edge(eh).from_vertex());
			vhs_on_sheet.insert(mesh->edge(eh).to_vertex());
		}
		vh_pairs.push_back(vertex_pair);
		int father = *vertex_pair.begin();
		sheet_data.vh_unit_set.insert(std::make_pair(father,father));
		foreach(auto &vh, vertex_pair){
			sheet_data.vh_unit_set.insert(std::make_pair(vh,father));
		}
		vertex_pair.clear();
	}
	eh_pairs.clear();

	//���ݵ��齫���еĵ����mapping��ͬһ��ĵ�ӳ�䵽ͬһ����
	std::map<OvmVeH, int> vh_mapping;
	int i = 0;
	foreach(auto & vh_pair, vh_pairs){
		foreach(auto & vh, vh_pair){
			vh_mapping.insert(std::make_pair(vh, i));
		}
		i++;
	}
	foreach(auto vh_pair, vh_pairs)
		vh_pair.clear();
	vh_pairs.clear();

	//�ҳ����е���sheet extraction������Ҫ�ϲ��ı�
	std::unordered_set<OvmEgH> all_merge_ehs;
		
	//��ú�sheet�ϵ����������ڵ�������
	std::unordered_set<OvmCeH> relavent_chs;
	foreach (auto &vh, vhs_on_sheet){
		std::unordered_set<OvmCeH> adj_chs;
		get_adj_hexas_around_vertex (mesh, vh, adj_chs);
		foreach (auto &ch, adj_chs){
			relavent_chs.insert (ch);
		}
		adj_chs.clear();
	}
	vhs_on_sheet.clear();
	//�ҳ�������Ӱ��ı�
	std::unordered_set<OvmEgH> all_relavent_ehs;
	foreach(auto & ch, relavent_chs){
		foreach(auto & hfh, mesh->cell(ch).halffaces()){
			foreach(auto & heh, mesh->halfface(hfh).halfedges())
				all_relavent_ehs.insert(mesh->edge_handle(heh));
		}
	}
	relavent_chs.clear();
	//�õ�Ҫ�ϲ��ı߼�
	foreach(auto & eh, all_relavent_ehs){
		if(!contains(sheet->ehs, eh)){
			auto vh1 = mesh->edge(eh).from_vertex();
			auto vh2 = mesh->edge(eh).to_vertex();
			if(vh_mapping.find(vh1) == vh_mapping.end())
				continue;
			if(vh_mapping.find(vh2) == vh_mapping.end())
				continue;
			all_merge_ehs.insert(eh);
		}
	}
	all_relavent_ehs.clear();
	//������ı߼����ҳ�����Ҫ�ϲ���һ���ߵı���
	std::unordered_set<OvmEgH> merge_ehs;
	while(all_merge_ehs.empty() == false){
		auto eh  = *all_merge_ehs.begin();
		int eh_v1 = vh_mapping[mesh->edge(eh).from_vertex()];
		int eh_v2 = vh_mapping[mesh->edge(eh).to_vertex()];
		merge_ehs.insert(eh);
		all_merge_ehs.erase(eh);
		for(auto eh_iter = all_merge_ehs.begin(); eh_iter != all_merge_ehs.end();){
			int v1 = vh_mapping[mesh->edge(*eh_iter).from_vertex()];
			int v2 = vh_mapping[mesh->edge(*eh_iter).to_vertex()];
			if((eh_v1 == v1) && (eh_v2 == v2)){
				auto eh_temp = *eh_iter;
				eh_iter++;
				merge_ehs.insert(eh_temp);
				all_merge_ehs.erase(eh_temp);
				continue;
			}
			if((eh_v1 == v2) && (eh_v2 == v1)){
				auto eh_temp = *eh_iter;
				eh_iter++;
				merge_ehs.insert(eh_temp);
				all_merge_ehs.erase(eh_temp);
				continue;
			}
			eh_iter++;
		}
		if(merge_ehs.size() > 1){
			int father = *merge_ehs.begin();
			sheet_data.merge_edge_unit_set.insert(std::make_pair(father,father));
			foreach(auto& eh, merge_ehs){
				sheet_data.merge_edge_unit_set.insert(std::make_pair(eh,father));
			}
		}
		merge_ehs.clear();
	}
	vh_mapping.clear();
}

void get_sheet_set_data(VolumeMesh* mesh, std::vector<DualSheet*> sheets, Sheet_data &sheet_data)
{
	std::unordered_set<OvmEgH> all_ehs;
	std::vector<std::unordered_set<OvmEgH>> eh_pairs;
	std::unordered_set<OvmEgH> edge_pair;
	std::vector<std::unordered_set<OvmVeH>> vh_pairs;
	std::unordered_set<OvmVeH> vertex_pair;

	//�����ҳ�sheet set extraction������Ҫ�����ıߣ������ӱ�
	foreach(auto &sheet, sheets){
		foreach(auto & eh, sheet->ehs){
			all_ehs.insert(eh);
		}	
	}

	//�����е����ӱ߸������ӹ�ϵ���з���
	while (!all_ehs.empty()){
		auto eh = *(all_ehs.begin());
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
				get_adj_edges_around_vertex (mesh,vh,adj_edges);//�������vh�����ӵ����бߵ�handleֵ
				foreach(auto &eh,adj_edges){
					if(all_ehs.find(eh) != all_ehs.end()){
						edge_pair.insert(eh);
						all_ehs.erase(eh);
					}
				}
				adj_edges.clear();
			}
			edge_pair_veh.clear();
			final_number = edge_pair.size();
		} while (original_number != final_number);
		eh_pairs.push_back(edge_pair);
		edge_pair.clear();
	}
	all_ehs.clear();

	//��ȡsheet�����еĶ���
	std::unordered_set<OvmVeH> vhs_on_sheet_set;
	//�������ֵı��飬�ҳ����Ӧ�ĵ��飬����extraction������Ҫ�ϲ���һ����ĵ���
	foreach(auto & eh_pair, eh_pairs){
		foreach(auto & eh, eh_pair){
			vertex_pair.insert(mesh->edge(eh).from_vertex());
			vertex_pair.insert(mesh->edge(eh).to_vertex());
			vhs_on_sheet_set.insert(mesh->edge(eh).from_vertex());
			vhs_on_sheet_set.insert(mesh->edge(eh).to_vertex());
		}
		vh_pairs.push_back(vertex_pair);
		int father = *vertex_pair.begin();
		sheet_data.vh_unit_set.insert(std::make_pair(father,father));
		foreach(auto &vh, vertex_pair){
			sheet_data.vh_unit_set.insert(std::make_pair(vh,father));
		}
		vertex_pair.clear();
	}


	eh_pairs.clear();

	//���ݵ��齫���еĵ����mapping��ͬһ��ĵ�ӳ�䵽ͬһ����
	std::map<OvmVeH, int> vh_mapping;
	int i = 0;
	foreach(auto & vh_pair, vh_pairs){
		foreach(auto & vh, vh_pair){
			vh_mapping.insert(std::make_pair(vh, i));
		}
		i++;
	}
	foreach(auto vh_pair, vh_pairs)
		vh_pair.clear();
	vh_pairs.clear();

	//�ҳ����е���sheet extraction������Ҫ�ϲ��ı�
	std::unordered_set<OvmEgH> all_merge_ehs;

	//��ú�sheet�ϵ����������ڵ�������
	std::unordered_set<OvmCeH> relavent_chs;
	foreach (auto &vh, vhs_on_sheet_set){
		std::unordered_set<OvmCeH> adj_chs;
		get_adj_hexas_around_vertex (mesh, vh, adj_chs);
		foreach (auto &ch, adj_chs){
			relavent_chs.insert (ch);
		}
		adj_chs.clear();
	}
	vhs_on_sheet_set.clear();
	//�ҳ�������Ӱ��ı�
	std::unordered_set<OvmEgH> all_relavent_ehs;
	foreach(auto & ch, relavent_chs){
		foreach(auto & hfh, mesh->cell(ch).halffaces()){
			foreach(auto & heh, mesh->halfface(hfh).halfedges())
				all_relavent_ehs.insert(mesh->edge_handle(heh));
		}
	}
	relavent_chs.clear();
	//�õ�Ҫ�ϲ��ı߼�
	foreach(auto &sheet, sheets){
		foreach(auto & eh, sheet->ehs){
			all_ehs.insert(eh);
		}	
	}
	foreach(auto & eh, all_relavent_ehs){
		if(!contains(all_ehs, eh)){
			auto vh1 = mesh->edge(eh).from_vertex();
			auto vh2 = mesh->edge(eh).to_vertex();
			if(vh_mapping.find(vh1) == vh_mapping.end())
				continue;
			if(vh_mapping.find(vh2) == vh_mapping.end())
				continue;
			all_merge_ehs.insert(eh);
		}
	}
	all_relavent_ehs.clear();
	//������ı߼����ҳ�����Ҫ�ϲ���һ���ߵı���
	std::unordered_set<OvmEgH> merge_ehs;
	while(all_merge_ehs.empty() == false){
		auto eh  = *all_merge_ehs.begin();
		int eh_v1 = vh_mapping[mesh->edge(eh).from_vertex()];
		int eh_v2 = vh_mapping[mesh->edge(eh).to_vertex()];
		merge_ehs.insert(eh);
		all_merge_ehs.erase(eh);
		for(auto eh_iter = all_merge_ehs.begin(); eh_iter != all_merge_ehs.end();){
			int v1 = vh_mapping[mesh->edge(*eh_iter).from_vertex()];
			int v2 = vh_mapping[mesh->edge(*eh_iter).to_vertex()];
			if((eh_v1 == v1) && (eh_v2 == v2)){
				auto eh_temp = *eh_iter;
				eh_iter++;
				merge_ehs.insert(eh_temp);
				all_merge_ehs.erase(eh_temp);
				continue;
			}
			if((eh_v1 == v2) && (eh_v2 == v1)){
				auto eh_temp = *eh_iter;
				eh_iter++;
				merge_ehs.insert(eh_temp);
				all_merge_ehs.erase(eh_temp);
				continue;
			}
			eh_iter++;
		}
		if(merge_ehs.size() > 1){
			int father = *merge_ehs.begin();
			sheet_data.merge_edge_unit_set.insert(std::make_pair(father,father));
			foreach(auto& eh, merge_ehs){
				sheet_data.merge_edge_unit_set.insert(std::make_pair(eh,father));
			}
		}
		merge_ehs.clear();
	}
	vh_mapping.clear();
}

void merge_sheet_data(VolumeMesh *mesh,Sheet_data& data1,Sheet_data & data2,Sheet_data & result)
{
	auto find_father = [&](std::hash_map<int,int>& vh_unit_set,int x)->int{
		int r=x;
		//std::unordered_set<int> already_retrieve;
		//already_retrieve.insert(r);
		while(/*vh_unit_set.find(r) != vh_unit_set.end() && */vh_unit_set[r] != r){
			r = vh_unit_set[r];				
			/*if(already_retrieve.find(r) != already_retrieve.end()){
				vh_unit_set[pre_r] = pre_r;
				return pre_r;
			}
			already_retrieve.insert(r);
			pre_r = r;*/
		}
		/*int i = x,j;
		while (i != r)
		{
			j = vh_unit_set[i];
			vh_unit_set[i] = r;
			i = j;
		}*/
		return  r ; 
	};
	if (data1.vh_unit_set.size() == 0)
	{
		result = data2;
		return;
	}
	if (data2.vh_unit_set.size() == 0)
	{
		result = data1;
		return;
	}

	result = data1;
	for (auto vh_iter = data2.vh_unit_set.begin(); vh_iter != data2.vh_unit_set.end(); ++vh_iter)
	{
		int vh = vh_iter->first;
		int s = vh_iter->second;
		//int curr_father = find_father(result.vh_unit_set,vh_iter->first);
		auto find_vh_iter = result.vh_unit_set.find(vh_iter->first);
		auto find_vh_father_iter = result.vh_unit_set.find(vh_iter->second);
		if (find_vh_iter == result.vh_unit_set.end() && find_vh_father_iter == result.vh_unit_set.end())//��data1���Ҳ�����Ӧ��vh
		{
			result.vh_unit_set.insert(std::make_pair(vh_iter->first,vh_iter->second));
			if (vh_iter->first != vh_iter->second)
			{
				result.vh_unit_set.insert(std::make_pair(vh_iter->second,vh_iter->second));
			}			
		}
		else if (find_vh_iter == result.vh_unit_set.end() && find_vh_father_iter != result.vh_unit_set.end())
		{
			if(result.vh_unit_set[vh_iter->second] == vh_iter->second){
				result.vh_unit_set.insert(std::make_pair(vh_iter->first,vh_iter->second));
			}
			else{				
				result.vh_unit_set.insert(std::make_pair(vh_iter->first,result.vh_unit_set[vh_iter->second]));
			}
		}
		else if(find_vh_iter != result.vh_unit_set.end() && find_vh_father_iter == result.vh_unit_set.end()){
			if (result.vh_unit_set[vh_iter->first] == vh_iter->first)
			{
				for (auto iter = result.vh_unit_set.begin(); iter != result.vh_unit_set.end(); ++iter)
				{
					if(iter->second == vh_iter->first)
						iter->second = vh_iter->second;
				}
				result.vh_unit_set.insert(std::make_pair(vh_iter->second,vh_iter->second));
			}
			else{
				result.vh_unit_set.insert(std::make_pair(vh_iter->second,result.vh_unit_set[vh_iter->first]));
			}
		}
		else {
			int a_father = result.vh_unit_set[vh_iter->first];
			int b_father = result.vh_unit_set[vh_iter->second];
			if (a_father != b_father)
			{
				if (vh_iter->first != a_father && vh_iter->second != b_father)
				{
					for (auto iter = result.vh_unit_set.begin(); iter != result.vh_unit_set.end(); ++iter)
					{
						if(iter->second == a_father)
							iter->second = b_father;
					}
				}
				else if(vh_iter->first != a_father && vh_iter->second == b_father){
					for (auto iter = result.vh_unit_set.begin(); iter != result.vh_unit_set.end(); ++iter)
					{
						if(iter->second == a_father)
							iter->second = b_father;
					}
				}
				else if(vh_iter->first == a_father && vh_iter->second != b_father){
					for (auto iter = result.vh_unit_set.begin(); iter != result.vh_unit_set.end(); ++iter)
					{
						if(iter->second == a_father)
							iter->second = b_father;
					}
				}
				else{
					for (auto iter = result.vh_unit_set.begin(); iter != result.vh_unit_set.end(); ++iter)
					{
						if(iter->second == a_father)
							iter->second = b_father;
					}
				}
			}
		}
	}


	for (auto vh_iter = data2.merge_edge_unit_set.begin(); vh_iter != data2.merge_edge_unit_set.end(); ++vh_iter)
	{
		int vh = vh_iter->first;
		int s = vh_iter->second;
		//int curr_father = find_father(result.merge_edge_unit_set,vh_iter->first);
		auto find_vh_iter = result.merge_edge_unit_set.find(vh_iter->first);
		auto find_vh_father_iter = result.merge_edge_unit_set.find(vh_iter->second);
		if (find_vh_iter == result.merge_edge_unit_set.end() && find_vh_father_iter == result.merge_edge_unit_set.end())//��result���Ҳ�����Ӧ��vh
		{
			result.merge_edge_unit_set.insert(std::make_pair(vh_iter->first,vh_iter->second));
			if (vh_iter->first != vh_iter->second)
			{
				result.merge_edge_unit_set.insert(std::make_pair(vh_iter->second,vh_iter->second));
			}			
		}
		else if (find_vh_iter == result.merge_edge_unit_set.end() && find_vh_father_iter != result.merge_edge_unit_set.end())
		{
			if(result.merge_edge_unit_set[vh_iter->second] == vh_iter->second){
				result.merge_edge_unit_set.insert(std::make_pair(vh_iter->first,vh_iter->second));
			}
			else{				
				result.merge_edge_unit_set.insert(std::make_pair(vh_iter->first,result.merge_edge_unit_set[vh_iter->second]));
			}
		}
		else if(find_vh_iter != result.merge_edge_unit_set.end() && find_vh_father_iter == result.merge_edge_unit_set.end()){
			if (result.merge_edge_unit_set[vh_iter->first] == vh_iter->first)
			{
				for (auto iter = result.merge_edge_unit_set.begin(); iter != result.merge_edge_unit_set.end(); ++iter)
				{
					if(iter->second == vh_iter->first)
						iter->second = vh_iter->second;
				}
				result.merge_edge_unit_set.insert(std::make_pair(vh_iter->second,vh_iter->second));
			}
			else{
				result.merge_edge_unit_set.insert(std::make_pair(vh_iter->second,result.merge_edge_unit_set[vh_iter->first]));
			}
		}
		else {
			int a_father = result.merge_edge_unit_set[vh_iter->first];
			int b_father = result.merge_edge_unit_set[vh_iter->second];
			if (a_father != b_father)
			{
				if (vh_iter->first != a_father && vh_iter->second != b_father)
				{
					for (auto iter = result.merge_edge_unit_set.begin(); iter != result.merge_edge_unit_set.end(); ++iter)
					{
						if(iter->second == a_father)
							iter->second = b_father;
					}
				}
				else if(vh_iter->first != a_father && vh_iter->second == b_father){
					for (auto iter = result.merge_edge_unit_set.begin(); iter != result.merge_edge_unit_set.end(); ++iter)
					{
						if(iter->second == a_father)
							iter->second = b_father;
					}
				}
				else if(vh_iter->first == a_father && vh_iter->second != b_father){
					for (auto iter = result.merge_edge_unit_set.begin(); iter != result.merge_edge_unit_set.end(); ++iter)
					{
						if(iter->second == a_father)
							iter->second = b_father;
					}
				}
				else{
					for (auto iter = result.merge_edge_unit_set.begin(); iter != result.merge_edge_unit_set.end(); ++iter)
					{
						if(iter->second == a_father)
							iter->second = b_father;
					}
				}
			}
		}
	}
		
		
}

bool is_sheets_can_be_extracted (VolumeMesh *mesh, BODY *body, std::hash_map<int,int> &vh_union_set, double& density_quality)
{
	assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
	//��������ߵķ������������ص������Ҳ���з��飬ÿ����ĵ����վ���Ϊһ���㣬��ͳ��ÿ���е��entityֵ
	auto find_father = [&](std::hash_map<int,int>& vh_unit_set,int x)->int{
		int r=x;  
		while(vh_unit_set.find(r) != vh_unit_set.end() && vh_unit_set[r] != r)
			r = vh_unit_set[r];                                                                
		return  r ; 
	};
	std::map<int,std::unordered_set<OvmVeH>> vertex_pairs_map;
	std::vector<std::vector<OvmVeH>> vertex_pairs;
	foreach(auto& vh, vh_union_set){
		int curr_father = find_father(vh_union_set,vh.first);
		auto find_iter = vertex_pairs_map.find(curr_father);
		if (find_iter == vertex_pairs_map.end())
		{
			std::unordered_set<OvmVeH> tmp;
			tmp.insert(vh.first);
			vertex_pairs_map.insert(std::make_pair(curr_father,tmp));
		}
		else
			find_iter->second.insert(vh.first);
	}
	std::vector<std::vector<ENTITY*>> vertex_entities;
	foreach(auto iter ,vertex_pairs_map){
		iter.second.insert(iter.first);
		std::vector<ENTITY*> vertex_entity;
		std::vector<OvmVeH> vertex_pair;
		foreach(auto& vh, iter.second){
			vertex_pair.push_back(vh);
			vertex_entity.push_back((ENTITY*)(V_ENTITY_PTR[vh]));
		}
		vertex_pairs.push_back(vertex_pair);
		vertex_entities.push_back(vertex_entity);
	}


	for(int i = 0; i < vertex_pairs.size(); i++){
		//�ж�ÿ����������պϲ��ɵ�����������
		bool is_vertex(false), is_edge(false), is_face(false);
		int num_vertex(0), num_edge(0), num_face(0);
		std::vector<int> vertex_vec, edge_vec, face_vec;
		for(int j = 0; j < vertex_entities[i].size(); j++){
			auto entity = vertex_entities[i][j];
			if(is_VERTEX(entity)){
				is_vertex = true;
				num_vertex++;
				vertex_vec.push_back(j);
			}
			else if(is_EDGE(entity)){
				is_edge = true;
				num_edge++;
				edge_vec.push_back(j);
			}
			else if(is_FACE(entity)){
				is_face = true;
				num_face++;
				face_vec.push_back(j);
			}
		}
		if(is_vertex == true){
			//���պϲ��ĵ������ΪVERTEX������������е�vertex_entity������ͬ(��vertex_vec�Ĵ�Сֻ��Ϊ1)��
			//���е�edge_entity����ʹ˵����ӣ����е�face_entity����Ҳ�ʹ˵�����
			//���򣬻ᵼ�¼����˻�
			if(vertex_vec.size() != 1)
				return false;
			ENTITY* entity = vertex_entities[i][vertex_vec[0]];
			//�ж����е�edge_entity�Ƿ��entity����
			ENTITY_LIST vertex_list;
			for(int k = 0; k < edge_vec.size(); k++){
				api_get_vertices(vertex_entities[i][edge_vec[k]], vertex_list);
				if(vertex_list.lookup(entity) == -1)
					return false;
				vertex_list.clear();
			}
			//�ж����е�face_entity�Ƿ��entity����
			for(int k = 0; k < face_vec.size(); k++){
				api_get_vertices(vertex_entities[i][face_vec[k]], vertex_list);
				if(vertex_list.lookup(entity) == -1)
					return false;
				vertex_list.clear();
			}
		}
		else if(is_edge == true){
			//���պϲ��ĵ������ΪEDGE������������е�edge_entity������ͬ��
			//���е�face_entity����Ҳ�ʹ˱�����
			//���򣬻ᵼ�¼����˻�
			ENTITY * entity = vertex_entities[i][edge_vec[0]];
			for(int k = 0; k < edge_vec.size(); k++){
				if(vertex_entities[i][edge_vec[k]] != entity)
					return false;
			}
			//�ж����е�face_entity�Ƿ��entity����
			ENTITY_LIST edge_list;
			for(int k = 0; k < face_vec.size(); k++){
				api_get_edges(vertex_entities[i][face_vec[k]], edge_list);
				if(edge_list.lookup(entity) == -1)
					return false;
				edge_list.clear();
			}
		}
		else if(is_face == true){
			//���պϲ��ĵ������ΪFACE������������е�face_entity������ͬ
			ENTITY * entity = vertex_entities[i][face_vec[0]];
			for(int k = 0; k < face_vec.size(); k++){
				if(vertex_entities[i][face_vec[k]] != entity)
					return false;
			}
		}
		else{
			;//���պϲ��ĵ�Ϊ�����ڲ��㣬�򲻻ᵼ�¼����˻�
		}
	}

	//�������������ٱ���һ���㣬�������߽����Ϊֱ�ߣ����¼����˻���
	//�����ڴ�ͬʱ�������м��α��ϵ������ܶȱ仯���
	//���Ȼ�ȡ���еı�entity
	ENTITY_LIST edges_list;
	api_get_edges (body, edges_list);
	std::vector<ENTITY *> edge_entities;
	ENTITY * cure = nullptr;
	edges_list.init();
	while (cure = edges_list.next())
		edge_entities.push_back(cure);
	//std::vector<ENTITY *> curve_edge_entities;
	//ENTITY * cure = nullptr;
	//edges_list.init();
	//while (cure = edges_list.next()){
	//	if(is_straight(&((EDGE *)cure)->geometry()->equation()) == false)
	//		curve_edge_entities.push_back(cure);
	//}

	//ͳ��ÿ�������ܹ��������ĸ����������˵㣩
	std::vector<int> edge_adj_vh_num;
	for(int i = 0; i < edge_entities.size(); i++)
		edge_adj_vh_num.push_back(2);
	for(auto vh_iter = mesh->vertices_begin(); vh_iter != mesh->vertices_end(); vh_iter++){
		ENTITY* entity = (ENTITY*)(V_ENTITY_PTR[*vh_iter]);
		if(!is_EDGE(entity))
			continue;
		for(int j = 0; j < edge_entities.size(); j++){
			if(entity == edge_entities[j]){
				edge_adj_vh_num[j]++;
				break;
			}
		}
	}

	//ͳ��Ҫ�ϲ��ĵ�������Щ���α����ܹ��������ĸ���
	std::vector<std::vector<int>> edge_merge_vh_num;
	for(int i = 0; i < edge_entities.size(); i++){
		std::vector<int> num_vec;
		for(int j = 0; j < vertex_pairs.size(); j++){
			num_vec.push_back(0);
		}
		edge_merge_vh_num.push_back(num_vec);
		num_vec.clear();
	}

	for(int i = 0; i < vertex_pairs.size(); i++){
		for(int j = 0; j < vertex_pairs[i].size(); j++){
			ENTITY* entity = vertex_entities[i][j];
			if(is_FACE(entity))
				continue;
			if(is_EDGE(entity)){
				for(int k = 0; k < edge_entities.size(); k++){
					if(entity == edge_entities[k]){
						edge_merge_vh_num[k][i]++;
						break;
					}
				}
			}
			else if(is_VERTEX(entity)){
				for(int k = 0; k < edge_entities.size(); k++){
					ENTITY_LIST vertex_list;
					api_get_vertices(edge_entities[k], vertex_list);
					if(vertex_list.lookup(entity) != -1)
						edge_merge_vh_num[k][i]++;
				}
			}
		}
	}

	//���������м�������
	std::vector<int> edge_final_vh_num;
	for(int i = 0; i < edge_entities.size(); i++){
		int final_vh_num(edge_adj_vh_num[i]);
		for(int j = 0; j < vertex_pairs.size(); j++){
			if(edge_merge_vh_num[i][j] != 0)
				final_vh_num = final_vh_num - edge_merge_vh_num[i][j] +1;
		}
		edge_final_vh_num.push_back(final_vh_num);
	}
	//�ж����߻᲻�ᵼ�¼����˻�
	for(int i = 0; i < edge_entities.size(); i++){
		if(is_straight(&((EDGE *)edge_entities[i])->geometry()->equation()))
			continue;
		if(edge_final_vh_num[i] < 3)
			return false;
	}
	//����ÿ��λ�ڱ��ϵ�����ߵ�ƽ������
	/*std::vector<double> length;
	double average_length(0), variance(0);
	float min_length(1000), max_length(0);
	for(int i = 0; i < edge_entities.size(); i++){
		length.push_back(((EDGE *)edge_entities[i])->length()/(edge_final_vh_num[i] - 1));
		average_length += length[i];
		if(length[i] > max_length)
			max_length = length[i];
		if(length[i] < min_length)
			min_length = length[i];
	}
		
	density_quality = max_length/min_length;*/
	//average_length /= edge_entities.size();
	//�󷽲�
	/*for(int i = 0; i < length.size(); i++){
		variance += (length[i]/average_length - 1) * (length[i]/average_length - 1);
	}
			
	density_quality = (variance/length.size());*/

	std::vector<ENTITY *>().swap(edge_entities);
	std::vector<int> ().swap(edge_adj_vh_num);
	std::vector<std::vector<int>> ().swap(edge_merge_vh_num);

	//�жϻ᲻�����һ�����������Ϊһ�����ʱ�򣬽�����Ǳ����ڵĵ�������
	//�б���ϵĵ��������������Ƿ������������
	//�б����ϵĵ��������������Ƿ������������

	return true;
}

bool is_sheet_can_be_extract_based_density(VolumeMesh *mesh, DualSheet* sheet)
{
	assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
	//�����жϴ�sheetɾ��ʱӰ�쵽������sheets
	std::unordered_set<OvmEgH> ehs = sheet->ehs;
	std::unordered_set<OvmVeH> vhs;
	foreach(auto eh, ehs){
		vhs.insert(mesh->edge(eh).from_vertex());
		vhs.insert(mesh->edge(eh).to_vertex());
	}
	std::vector<DualSheet*> relavent_sheets;
	SheetSet all_sheets;
	retrieve_sheets(mesh, all_sheets);
	foreach(auto sheet_temp, all_sheets){
		if(sheet_temp == sheet)
			continue;
		std::unordered_set<OvmEgH> ehs_temp(sheet_temp->ehs);
		foreach(auto eh, ehs_temp){
			if((contains(vhs, mesh->edge(eh).from_vertex()) && !contains(vhs, mesh->edge(eh).to_vertex())) || (contains(vhs, mesh->edge(eh).to_vertex()) && !contains(vhs, mesh->edge(eh).from_vertex()))){
				relavent_sheets.push_back(sheet_temp);
				break;
			}
		}
	}
	//Ԥ������sheetɾ��ʱ����Щ����Ҫ�ϲ�������������ϲ��ĳ�ʼλ��,��ֻ���Ǽ�sheet���������Լ��Խ������
	std::hash_map<OvmVeH, OvmVec3d> old_new_vhs_mapping;
	foreach(auto eh, ehs){
		OvmVeH vh1(mesh->edge(eh).from_vertex()), vh2(mesh->edge(eh).to_vertex());
		ENTITY* entity1((ENTITY*)(V_ENTITY_PTR[vh1]));
		ENTITY* entity2((ENTITY*)(V_ENTITY_PTR[vh2]));
	    OvmVec3d new_pt = OvmVec3d ();

		if (is_VERTEX (entity1))
			new_pt = mesh->vertex (vh1);
		else if (is_VERTEX (entity2))
			new_pt = mesh->vertex (vh2);
		else if (is_EDGE (entity1) && !is_EDGE(entity2))
			new_pt = mesh->vertex (vh1);
		else if (is_EDGE (entity2) && !is_EDGE(entity1))
			new_pt = mesh->vertex (vh2);
		else if (is_EDGE(entity1) && is_EDGE(entity2)){
			new_pt = (mesh->vertex(vh1)+mesh->vertex(vh2))/2;
			SPAposition closest_pos;
			double dis(0);
			api_entity_point_distance(entity1, POS2SPA(new_pt), closest_pos, dis);
			new_pt = SPA2POS(closest_pos);
		}
		else if (is_FACE (entity1) && !is_FACE(entity2))
			new_pt = mesh->vertex (vh1);
		else if (is_FACE (entity2) && !is_FACE(entity1))
			new_pt = mesh->vertex (vh2);
		else if (is_FACE(entity1) && is_FACE(entity2)){
			new_pt = (mesh->vertex(vh1)+mesh->vertex(vh2))/2;
			SPAposition closest_pos;
			double dis(0);
			api_entity_point_distance(entity1, POS2SPA(new_pt), closest_pos, dis);
			new_pt = SPA2POS(closest_pos);
		}
		else
			new_pt = (mesh->vertex(vh1)+mesh->vertex(vh2))/2;
		
		old_new_vhs_mapping.insert (std::make_pair (vh1, new_pt));
		old_new_vhs_mapping.insert (std::make_pair (vh2, new_pt));
	}

	foreach(auto sheet_temp, relavent_sheets){
		std::unordered_set<OvmEgH> ehs_temp = sheet_temp->ehs;
		double max_dis(-1000), min_dis(1000);
		//�����ʼ�����̱߱�
		foreach(auto eh, ehs_temp){
			if(!mesh->is_boundary(eh))
				continue;
			OvmVeH v1(mesh->edge(eh).from_vertex()), v2(mesh->edge(eh).to_vertex());
			OvmVec3d vec = mesh->vertex(v1) - mesh->vertex(v2);
			double dis = vec.length();
			if(dis < min_dis)
				min_dis = dis;
			if(dis > max_dis)
				max_dis = dis;
		}
		double origin_ratio = max_dis/min_dis;
		max_dis = -1000;
		min_dis = 1000;
		//�����ȡ������̱߱�
		foreach(auto eh, ehs_temp){
			if(!mesh->is_boundary(eh))
				continue;
			OvmVeH v1(mesh->edge(eh).from_vertex()), v2(mesh->edge(eh).to_vertex());
			OvmVec3d v_1(mesh->vertex(v1)), v_2(mesh->vertex(v2));
			if(contains(vhs, v1))
				v_1 = old_new_vhs_mapping[v1];
			if(contains(vhs, v2))
				v_2 = old_new_vhs_mapping[v2];
			OvmVec3d vec = v_1 - v_2;
			double dis = vec.length();
			if(dis < min_dis)
				min_dis = dis;
			if(dis > max_dis)
				max_dis = dis;
		}
		double final_ratio = max_dis/min_dis;

		if(final_ratio > origin_ratio * 1.5)
			return false;
	}

	return true;
}

int topology_value_change_after_merge(VolumeMesh * mesh, std::vector<DualSheet*> &sheets,std::hash_map<int,int> &merge_edge_union_set, std::map<OvmEgH, EdgeAttribute> & edge_property)
{
	assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
	int topology_value(0);
	//int num_sigular_edge(0);
	std::unordered_set<OvmEgH> ehs;
	//���ȼ��������ı߶��������������Ķ���
	foreach(auto & sheet, sheets){
		foreach(auto & eh, sheet->ehs){
			ehs.insert(eh);
			topology_value += pow(float(edge_property[eh].valence - edge_property[eh].idealvalence),2);
			//topology_value /= edge_pairs.eliminate_ehs.size();
			/*if(mesh->is_boundary(eh) == false){
				num_sigular_edge += abs(edge_property[eh].valence - 4);
			}*/
		}
	}
	int dis_origin,dis_final;

	auto find_father = [&](std::hash_map<int,int>& vh_unit_set,int x)->int{
		int r=x;  
		while(vh_unit_set.find(r) != vh_unit_set.end() && vh_unit_set[r] != r)
			r = vh_unit_set[r];                                                                
		return  r ; 
	};
	std::map<int,std::vector<OvmEgH>> merge_ehs;
	foreach(auto& eh, merge_edge_union_set){
		int curr_father = find_father(merge_edge_union_set,eh.first);
		auto find_iter = merge_ehs.find(curr_father);
		if (find_iter == merge_ehs.end())
		{
			std::vector<OvmEgH> tmp;
			tmp.push_back(eh.first);
			merge_ehs.insert(std::make_pair(curr_father,tmp));
		}
		else
			find_iter->second.push_back(eh.first);
	}



	//����ÿ��ϲ��ߺϲ��������������Ӱ��
	foreach(auto & merge_pair, merge_ehs){
		if(contains(ehs, *(merge_pair.second.begin())))
			continue;
		dis_origin = 0;
		dis_final = 0;
		foreach(auto & eh, merge_pair.second){
			ENTITY* entity1((ENTITY*)(V_ENTITY_PTR[mesh->edge(eh).from_vertex()]));
			ENTITY* entity2((ENTITY*)(V_ENTITY_PTR[mesh->edge(eh).to_vertex()]));
			if(mesh->is_boundary(eh) || is_VERTEX(entity1) || is_VERTEX(entity2) || is_EDGE(entity1) || is_EDGE(entity2)){
				dis_origin += pow(float(edge_property[eh].valence - edge_property[eh].idealvalence), 2)*3;
			}
			else
				dis_origin += pow(float(edge_property[eh].valence - edge_property[eh].idealvalence), 2);
		}
		//�����ºϲ��ıߵ����ͣ����������
		bool is_edge_5(false),is_edge_4(false),is_edge_3(false),is_edge_2(false),is_face(false),is_half_bound(false);
		foreach(auto & eh, merge_pair.second){
			ENTITY* entity1((ENTITY*)(V_ENTITY_PTR[mesh->edge(eh).from_vertex()]));
			ENTITY* entity2((ENTITY*)(V_ENTITY_PTR[mesh->edge(eh).to_vertex()]));
			if(is_VERTEX(entity1) || is_VERTEX(entity2) || is_EDGE(entity1) || is_EDGE(entity2))
				is_half_bound = true;
			if(edge_property[eh].edgetype == FACEEDGE)
				is_face = true;
			if((edge_property[eh].edgetype == EDGEEDGE) && (edge_property[eh].idealvalence == 5))
				is_edge_5 = true;
			if((edge_property[eh].edgetype == EDGEEDGE) && (edge_property[eh].idealvalence == 4))
				is_edge_4 = true;
			if((edge_property[eh].edgetype == EDGEEDGE) && (edge_property[eh].idealvalence == 3))
				is_edge_3 = true;
			if((edge_property[eh].edgetype == EDGEEDGE) && (edge_property[eh].idealvalence == 2))
				is_edge_2 = true;
		}
		int ideal_valence(0);
		if(is_edge_5 == true)
			ideal_valence = 5;
		else if(is_edge_4 == true)
			ideal_valence = 4;
		else if(is_edge_3 == true)
			ideal_valence = 3;
		else if(is_edge_2 == true)
			ideal_valence = 2;
		else if(is_face == true)
			ideal_valence = 3;
		else
			ideal_valence = 4;
		//��������ߺϲ�֮��ıߵĶ���
		int v(4);
		bool first_b(false);
		bool second_b;
		for(auto eh_iter = merge_pair.second.begin(); eh_iter != merge_pair.second.end(); ++eh_iter){
			if(edge_property[*eh_iter].edgetype == INNEREDGE)
				second_b = false;
			else
				second_b = true;
			if(first_b && second_b)
				v = v + edge_property[*eh_iter].valence - 3;
			else
				v = v + edge_property[*eh_iter].valence - 4;
			first_b = (first_b || second_b);
		}
		if(!(is_edge_5 || is_edge_4 || is_edge_3 || is_edge_2 || is_face)){
			//if(v < 3)
			//return -1000;
			//if(v > 5)
			//return -1000;
		}
		//dis_origin /= edge_pairs.merge_ehs.size();
		int dis_final = pow(float(v-ideal_valence),2);
		if(is_edge_5 || is_edge_4 || is_edge_3 || is_edge_2 || is_face || is_half_bound){
			topology_value += dis_origin - dis_final*3;
		}
		else{
			topology_value += (dis_origin - dis_final);
			/*foreach(auto & eh, merge_pair.second){
				if(mesh->is_boundary(eh) == false){
					num_sigular_edge += abs(edge_property[eh].valence - 4);
				}
			}
			num_sigular_edge -= abs(v-4);*/
		}
	}
	return topology_value;
	//return num_sigular_edge;

}

void choose_extract_sheets(VolumeMesh *mesh, BODY * body, std::vector<DualSheet*> & sheets_vec, std::vector<DualSheet*> & choose_sheets)
{
	std::map<OvmEgH, EdgeAttribute> edge_property;
	get_edge_property(mesh, edge_property);

	//һ����ɾ����sheet���������κͰ����ε�
	std::vector<int> costant_sheet_num;
	int sheets_num = sheets_vec.size();
	std::vector<int> optional_sheet_num;
	//std::ofstream fout;
	//fout.open("all_optional_result.txt");
	//std::ofstream fout2;
	//fout2.open("cant_extracted.txt");
	//std::ofstream fout3;
	//fout3.open("density.txt");
	//std::ofstream fout4;
	//fout4.open("fenzu.txt");
	//��ſ�ѡ��sheet����ϵ�edge_pairs��Ϣ
	std::map<std::string,Sheet_data>* optional_combination_data = new std::map<std::string,Sheet_data>();
	std::unordered_set<std::string>* cant_extracted = new std::unordered_set<std::string>();
	std::map<std::string,int> index_mark_map;
	for (int i = 0; i < sheets_vec.size(); ++i)
	{
		std::vector<DualSheet*> sheets;
		sheets.push_back(sheets_vec[i]);
		optional_sheet_num.push_back(i);
		Sheet_data sheet_data_temp;
		get_sheet_data(mesh,sheets_vec[i],sheet_data_temp);
		std::string index(sheets_num,'0');
		index[i] = '1';
		(*optional_combination_data)[index] = sheet_data_temp;
		int topology_mark = topology_value_change_after_merge(mesh,sheets,sheet_data_temp.merge_edge_unit_set,edge_property);
		index_mark_map[index] = topology_mark;


		//fout.open("all_optional_result.txt",std::ios_base::app);											
		//fout<<index<<" "<<topology_mark<<"\n";
		//for(int i = 0; i < index.size(); ++i)
		//	if (index[i] == '1')
		//		fout<<sheets_vec[i]->name.toStdString()<<" ";			
		//fout<<"\n\n";
		//fout.close();
	}
		
	for(int round = 2; round <= sheets_vec.size() ; round++){
		std::vector<std::vector<int>> subsets;
		if (round >= 4)//ɾ������Ҫ��sheetdata��Ϣ
		{
			combine(optional_sheet_num, optional_sheet_num.size(), round - 2, subsets);
			foreach(auto& subset,subsets){
				std::string index1(sheets_num,'0'), index2(sheets_num,'0'),index_total(sheets_num,'0');
				for(int i = 0; i < (subset.size() - 1); i++)
					index1[subset[i]] = '1';
				index2[subset[subset.size() - 1]] = '1';
				add_binary(index1,index2,index_total);
				(*optional_combination_data).erase(index_total);
			}
		}
		subsets.clear();
		combine(optional_sheet_num, optional_sheet_num.size(), round, subsets);

		foreach(auto & subset, subsets){
			std::vector<DualSheet*> sheets;
			std::string index1(sheets_num,'0'), index2(sheets_num,'0'),index_total(sheets_num,'0');
			for(int i = 0; i < (subset.size() - 1); i++){
				index1[subset[i]] = '1';
				sheets.push_back(sheets_vec[subset[i]]);
			}
			index2[subset[subset.size() - 1]] = '1';
			sheets.push_back(sheets_vec[subset[subset.size() - 1]]);
			add_binary(index1,index2,index_total);
			bool cant_extra = false;
			foreach (auto& substring,*cant_extracted)
			{
				if (is_subset(index_total,substring)){
					//fout2.open("cant_extracted.txt",std::ios_base::app);											
					//fout2<<index_total<<"\n";
					//fout2.close();
					cant_extra = true;
					break;
				}
			}
			if(cant_extra)
				continue;
			Sheet_data *sheet_data_temp = new Sheet_data();
			merge_sheet_data(mesh, (*optional_combination_data)[index1], (*optional_combination_data)[index2], *sheet_data_temp);

			double density;	
			if (is_sheets_can_be_extracted(mesh,body,sheet_data_temp->vh_unit_set,density)){
				//fout3<<index_total<<" "<<density<<"\n";
				//for(int i = 0; i < index_total.size(); ++i)
				//	if (index_total[i] == '1')
				//		fout3<<sheets_vec[i]->name.toStdString()<<" ";	
				//fout3<<"\n\n";
				(*optional_combination_data)[index_total] = *sheet_data_temp;
				if (density > 14)
					continue;
					
				int topology_mark = topology_value_change_after_merge(mesh,sheets,sheet_data_temp->merge_edge_unit_set,edge_property);
				index_mark_map[index_total] = topology_mark;
				//test
					
				//fout.open("all_optional_result.txt",std::ios_base::app);											
				//fout<<index_total<<" "<<topology_mark<<"\n";
				//for(int i = 0; i < index_total.size(); ++i)
				//	if (index_total[i] == '1')
				//		fout<<sheets_vec[i]->name.toStdString()<<" ";			
				//fout<<"\n\n";
				//fout.close();
			}
			else{
				cant_extracted->insert(index_total);
					
				//fout2.open("cant_extracted.txt",std::ios_base::app);											
				//fout2<<index_total<<"\n";
				//fout2.close();
			}				
			delete sheet_data_temp;
		}
	}
		
	//fout3.close();
	delete cant_extracted;
    delete optional_combination_data;
		

	//ѡ�����ŵ�һ����
	int max_mark(-1);
	std::string choose_index(sheets_num,'0');
	foreach(auto &map, index_mark_map){
		if(map.second > max_mark){
			max_mark = map.second;
			choose_index = map.first;
		}
		else if(map.second == max_mark){
			int choose_sheet_count(0);
			int curr_sheet_count(0);
			for(int i = 0; i < choose_index.size(); ++i)
			{
				if (choose_index[i]=='1')
					choose_sheet_count++;
				if(map.first[i]=='1')
					curr_sheet_count++;					
			}
			if(curr_sheet_count > choose_sheet_count)
				choose_index = map.first;
		}
	}
	//std::ofstream fout1;
	//fout1.open("best_index.txt");
	//fout1<<choose_index;
	//fout1.close();
	for (int i = 0; i < choose_index.size(); ++i)
	{
		if(choose_index[i] == '1')
			choose_sheets.push_back(sheets_vec[i]);
	}
}

void choose_extract_sheets_constrained(VolumeMesh *mesh, BODY * body, std::vector<DualSheet*> & sheets_vec, std::vector<int> &optional_sheet_num, std::vector<DualSheet*> & choose_sheets)
{
	std::map<OvmEgH, EdgeAttribute> edge_property;
	get_edge_property(mesh, edge_property);

	//һ����ɾ����sheet
	std::vector<int> costant_sheet_num;
	int sheets_num = sheets_vec.size();
	for(int i = 0; i < sheets_num; i++){
		if(!contains(optional_sheet_num, i))
			costant_sheet_num.push_back(i);
	}

	Sheet_data *constant_sheet_data = new Sheet_data();
	Sheet_data *constant_sheet_data_temp =  new Sheet_data();
	std::string index_constant(sheets_num,'0');
	int size = sizeof(index_constant);
	foreach(auto & i, costant_sheet_num){
		index_constant[i] = '1';
		constant_sheet_data->vh_unit_set.clear();
		constant_sheet_data->merge_edge_unit_set.clear();
		Sheet_data inf;
		get_sheet_data(mesh,sheets_vec[i],inf);
		merge_sheet_data(mesh,*constant_sheet_data_temp,inf,*constant_sheet_data);	
	/*	std::map<int,std::unordered_set<OvmVeH>> vertex_pairs_map;
		Sheet_data *constant_sheet_data2 = new Sheet_data();
		foreach(auto& vh, constant_sheet_data->vh_unit_set){
			int curr_father = constant_sheet_data->vh_unit_set[vh.first];
			auto find_iter = vertex_pairs_map.find(curr_father);
			if (find_iter == vertex_pairs_map.end())
			{
				std::unordered_set<OvmVeH> tmp;
				tmp.insert(vh.first);
				vertex_pairs_map.insert(std::make_pair(curr_father,tmp));
			}
			else
				find_iter->second.insert(vh.first);
		}*/
		constant_sheet_data_temp->vh_unit_set.clear();
		constant_sheet_data_temp->merge_edge_unit_set.clear();
		constant_sheet_data_temp->vh_unit_set = constant_sheet_data->vh_unit_set;
		constant_sheet_data_temp->merge_edge_unit_set = constant_sheet_data->merge_edge_unit_set;
	}
	delete constant_sheet_data_temp;
	/*auto find_father = [&](std::hash_map<int,int>& vh_unit_set,int x)->int{
		int r=x;  
		while(vh_unit_set.find(r) != vh_unit_set.end() && vh_unit_set[r] != r)
			r = vh_unit_set[r];                                                                
		return  r ; 
	};
	std::map<int,std::unordered_set<OvmVeH>> vertex_pairs_map;
	Sheet_data *constant_sheet_data2 = new Sheet_data();
	foreach(auto& vh, constant_sheet_data->vh_unit_set){
		int curr_father = find_father(constant_sheet_data->vh_unit_set,vh.first);
		auto find_iter = vertex_pairs_map.find(curr_father);
		if (find_iter == vertex_pairs_map.end())
		{
			std::unordered_set<OvmVeH> tmp;
			tmp.insert(vh.first);
			vertex_pairs_map.insert(std::make_pair(curr_father,tmp));
		}
		else
			find_iter->second.insert(vh.first);
	}
	for(auto map = vertex_pairs_map.begin(); map != vertex_pairs_map.end(); ++map){
		constant_sheet_data2->vh_unit_set.insert(std::make_pair(map->first,map->first));
		for (auto vh_iter = map->second.begin(); vh_iter != map->second.end(); ++vh_iter)
		{
			constant_sheet_data2->vh_unit_set.insert(std::make_pair(*vh_iter,map->first));
		}
	}
	vertex_pairs_map.clear();
	foreach(auto& vh, constant_sheet_data->merge_edge_unit_set){
		int curr_father = find_father(constant_sheet_data->merge_edge_unit_set,vh.first);
		auto find_iter = vertex_pairs_map.find(curr_father);
		if (find_iter == vertex_pairs_map.end())
		{
			std::unordered_set<OvmVeH> tmp;
			tmp.insert(vh.first);
			vertex_pairs_map.insert(std::make_pair(curr_father,tmp));
		}
		else
			find_iter->second.insert(vh.first);
	}
	for(auto map = vertex_pairs_map.begin(); map != vertex_pairs_map.end(); ++map){
		constant_sheet_data2->merge_edge_unit_set.insert(std::make_pair(map->first,map->first));
		for (auto vh_iter = map->second.begin(); vh_iter != map->second.end(); ++vh_iter)
		{
			constant_sheet_data2->merge_edge_unit_set.insert(std::make_pair(*vh_iter,map->first));
		}
	}
	delete constant_sheet_data;*/


		

	//��ſ�ѡ��sheet����ϵ�edge_pairs��Ϣ
	std::map<std::string,Sheet_data>* optional_combination_data = new std::map<std::string,Sheet_data>();
	std::unordered_set<std::string>* cant_extracted = new std::unordered_set<std::string>();
	std::map<std::string,int> index_mark_map;
	foreach(auto & i, optional_sheet_num){
		Sheet_data sheet_data_temp;
		get_sheet_data(mesh,sheets_vec[i],sheet_data_temp);
		std::string index(sheets_num,'0');
		index[i] = '1';
		(*optional_combination_data)[index] = sheet_data_temp;
	}
	//std::ofstream fout;
	//fout.open("all_optional_result.txt");
	//std::ofstream fout2;
	//fout2.open("cant_extracted.txt");
	for(int round = 2; round < optional_sheet_num.size() ; round++){
		std::vector<std::vector<int>> subsets;
		if (round >= 4)//ɾ������Ҫ��edge_pairs��Ϣ
		{
			combine(optional_sheet_num, optional_sheet_num.size(), round - 2, subsets);
			foreach(auto& subset,subsets){
				std::string index1(sheets_num,'0');
				for(int i = 0; i < (subset.size()); i++)
					index1[subset[i]] = '1';
				//if (optional_combination_data->find(index1)!= optional_combination_data->end())
				(*optional_combination_data).erase(index1);					
			}

		}
		subsets.clear();
		combine(optional_sheet_num, optional_sheet_num.size(), round, subsets);

		foreach(auto & subset, subsets){
			std::vector<DualSheet*> sheets;
			std::string index1(sheets_num,'0'), index2(sheets_num,'0'),index_total(sheets_num,'0');
			for(int i = 0; i < (subset.size() - 1); i++){
				index1[subset[i]] = '1';
			}
			index2[subset[subset.size() - 1]] = '1';
			add_binary(index1,index2,index_total);
			for(int i = 0; i < index_total.size(); ++i)
				if (index_total[i] == '1')
					sheets.push_back(sheets_vec[i]);
			bool cant_extra = false;
			foreach (auto& substring,*cant_extracted)
			{
				if (is_subset(index_total,substring)){
					cant_extra = true;
					break;
				}
			}
			if(cant_extra)
				continue;
			Sheet_data *sheet_data_temp = new Sheet_data();
			merge_sheet_data(mesh, (*optional_combination_data)[index1], (*optional_combination_data)[index2], *sheet_data_temp);

			double density;
			if (is_sheets_can_be_extracted(mesh,body,sheet_data_temp->vh_unit_set,density)){
				std::vector<DualSheet *> sheets_selected;
				std::string index_selected(sheets_num,'0');
				add_binary(index_total,index_constant,index_selected);
				for(int i = 0; i < index_selected.size(); ++i)
					if (index_selected[i] == '1')
						sheets_selected.push_back(sheets_vec[i]);
				Sheet_data *c_sheet_data_temp = new Sheet_data();
				merge_sheet_data(mesh, *sheet_data_temp, *constant_sheet_data,*c_sheet_data_temp);
				if(is_sheets_can_be_extracted(mesh,body,c_sheet_data_temp->vh_unit_set,density)){
					int topology_mark = topology_value_change_after_merge(mesh,sheets_selected,c_sheet_data_temp->merge_edge_unit_set,edge_property);
					index_mark_map[index_selected] = topology_mark;
					//test
					//fout.open("all_optional_result.txt",std::ios_base::app);											
					//fout<<index_selected<<" "<<topology_mark<<"\n";
					//for(int i = 0; i < index_selected.size(); ++i)
					//	if (index_selected[i] == '1')
					//		fout<<sheets_vec[i]->name.toStdString()<<" ";			
					//fout<<"\n\n";
					//fout.close();
				}
				else
				{
					//fout.open("all_optional_result.txt",std::ios_base::app);											
					//fout<<index_selected<<" cant_ex"<<"\n";
					//for(int i = 0; i < index_selected.size(); ++i)
					//	if (index_selected[i] == '1')
					//		fout<<sheets_vec[i]->name.toStdString()<<" ";			
					//fout<<"\n\n";
					//fout.close();
				}
				//int topology_mark = topology_value_change_after_merge(mesh,sheets,sheet_data_temp->merge_edge_unit_set,edge_property);
				//index_mark_map[index_total] = topology_mark;
				(*optional_combination_data)[index_total] = *sheet_data_temp;
				delete c_sheet_data_temp;
				/*fout.open("all_optional_result.txt",std::ios_base::app);											
				fout<<index_total<<" "<<topology_mark<<"\n";
				for(int i = 0; i < index_total.size(); ++i)
					if (index_total[i] == '1')
						fout<<sheets_vec[i]->name.toStdString()<<" ";			
				fout<<"\n\n";
				fout.close();*/					
			}
			else{
				cant_extracted->insert(index_total);

				//fout2.open("cant_extracted.txt",std::ios_base::app);											
				//fout2<<index_total<<"\n";
				//fout2.close();
			}				
			delete sheet_data_temp;
				
		}
	}
		
	delete cant_extracted;
	//��constant_sheet��ÿ��optional_sheet������ϣ�������markֵ
	//���ÿ����ϵ�index�Ͷ�Ӧ��markֵ
		
	//for(auto optional_com = optional_combination_data->begin(); optional_com != optional_combination_data->end(); optional_com++)
	//{
	//	std::vector<DualSheet *> sheets_selected;
	//	std::string index_selected(sheets_num,'0');
	//	add_binary(optional_com->first,index_constant,index_selected);

	//	for(int i = 0; i < index_selected.size(); ++i)
	//		if (index_selected[i] == '1'){
	//			sheets_selected.push_back(sheets_vec[i]);
	//			bool m = sheets_vec[i]->name == "sheet No.1";
	//		}
	//	Sheet_data *sheet_data_temp = new Sheet_data();
	//	merge_sheet_data(mesh, optional_com->second, *constant_sheet_data2,*sheet_data_temp);
	//	if(is_sheets_can_be_extracted(mesh,body,sheet_data_temp->vh_unit_set)){
	//		int topology_mark = topology_value_change_after_merge(mesh,sheets_selected,sheet_data_temp->merge_edge_unit_set,edge_property);
	//		index_mark_map[index_selected] = topology_mark;
	//		
	//		fout.open("all_optional_result.txt",std::ios_base::app);											
	//		fout<<index_selected<<" "<<topology_mark<<"\n";
	//		for(int i = 0; i < index_selected.size(); ++i)
	//			if (index_selected[i] == '1')
	//				fout<<sheets_vec[i]->name.toStdString()<<" ";			
	//		fout<<"\n\n";
	//		fout.close();
	//	}
	//	//optional_combination_data->erase(optional_com++);
	//	delete sheet_data_temp;
	//}

    delete optional_combination_data;

		

	//ѡ�����ŵ�һ����
	int max_mark(-1);
	std::string choose_index(sheets_num,'0');
	foreach(auto &map, index_mark_map){
		if(map.second > max_mark){
			max_mark = map.second;
			choose_index = map.first;
		}
		else if(map.second == max_mark){
			int choose_sheet_count(0);
			int curr_sheet_count(0);
			for(int i = 0; i < choose_index.size(); ++i)
			{
				if (choose_index[i]=='1')
					choose_sheet_count++;
				if(map.first[i]=='1')
					curr_sheet_count++;					
			}
			if(curr_sheet_count > choose_sheet_count)
				choose_index = map.first;
		}
	}

	for (int i = 0; i < choose_index.size(); ++i)
	{
		if(choose_index[i] == '1')
			choose_sheets.push_back(sheets_vec[i]);
	}

	std::ofstream fbest;
	fbest.open("best_option.txt");											
	fbest<<choose_index<<"\n";
	for(int i = 0; i < choose_index.size(); ++i)
		if (choose_index[i] == '1')
			fbest<<sheets_vec[i]->name.toStdString()<<" ";			
	fbest.close();
}

void combine(std::vector<int> & set, int n, int k, std::vector<std::vector<int>> & subsets)
{
	unsigned char * vec = new unsigned char[n];
	std::vector<int> subset;

	// build the 0-1 vector.
	for(int i = 0; i < n; i++)
	{
		if (i < k)
			vec[i] = 1;
		else
			vec[i] = 0;
	}

	// begin scan.
	bool has_next = true;
	while (has_next)
	{
		// get choosen.
		int j = 0;
		for (int i = 0; i < n; i++)
		{
			if (vec[i] == 1)
			{
				subset.push_back(set[i]);
			}
		}
		subsets.push_back(subset);
		subset.clear();

		has_next = false;
		for (int i = 0; i < n - 1; i++)
		{
			if (vec[i] == 1 && vec[i + 1] == 0)
			{
				vec[i] = 0;
				vec[i + 1] = 1;

				// move all 1 to left-most side.
				int count = 0;
				for (int j = 0; j < i; j++)
				{
					if (vec[j] == 1)
						count ++;
				}
				if (count < i)
				{
					for (int j = 0; j < count; j++)
					{
						vec[j] = 1;
					}
					for (int j = count; j < i; j++)
					{
						vec[j] = 0;
					}
				}

				has_next = true;
				break;
			}
		}
	}
	subset.clear();
	delete [] vec;
}

void add_binary(const std::string& index1,const std::string& index2,std::string &index)
{
	for (int i =0 ; i < index.size(); ++i)
	{
		if (index1[i] == '1' || index2[i] == '1')
			index[i] = '1';
	}
}

bool is_subset(const std::string& index,const std::string& subset)
{
	for (int i = 0; i < index.size(); ++i)
	{
		if(index[i] != subset[i] && subset[i] != '0')
			return false;
	}
	return true;
		
}
