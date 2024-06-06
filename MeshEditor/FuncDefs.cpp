#include "StdAfx.h"
#include "FuncDefs.h"
#include "PrioritySetManager.h"
#include <OpenVolumeMesh/Attribs/StatusAttrib.hh>
#include <OpenVolumeMesh/FileManager/FileManager.hh>
#include "topologyoptwidget.h"
#include <QMessageBox>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <qstring.h>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <unordered_map>
#include <time.h>
//定义于topologyopt_process中的omf_ovmch_mapping
std::hash_map<size_t,OvmCeH> omf_ovmch_mapping; 



struct pair_hash {
	static const size_t   bucket_size = 4;  
	static const size_t   min_buckets = 8;  

	std::size_t operator () (const std::pair<OvmFaH, OvmFaH> &p) const
	{
		auto h1 = (p.first);
		auto h2 = (p.second);
		return h1 ^ h2;
	}

	bool operator()(const std::pair<OvmFaH, OvmFaH> &pt1,const std::pair<OvmFaH, OvmFaH> &pt2)const  
	{  
		if(pt1.first < pt2.first)  
			return true;  
		else if(pt1.first > pt2.first)  
			return false;  
		else  
		{  
			return pt1.second < pt2.second;  
		}  
	}  
};


VolumeMesh* load_volume_mesh (QString mesh_path)
{
	VolumeMesh* hexa_mesh = new VolumeMesh ();
	OpenVolumeMesh::IO::FileManager file_manager;
	file_manager.readFile (mesh_path.toAscii ().data (), *hexa_mesh, false);
	return hexa_mesh;
}

VMesh* load_tet_mesh(QString mesh_path)
{
	VMesh* tet_mesh = new VMesh ();
	OpenVolumeMesh::IO::FileManager file_manager;
	file_manager.readFile (mesh_path.toAscii ().data (), *tet_mesh, false);
	//在这里还需要再进一步先将输入的四面体分裂成8个体
	//////////////////////////////////////////////////////////////////////////
	return tet_mesh;
}

VolumeMesh* tet2hex(VMesh* tetmesh)
{
	VolumeMesh *mesh_hex = new VolumeMesh ();
	QMap<OpenVolumeMesh::VertexHandle, OpenVolumeMesh::VertexHandle> indexmapping_vv;
	QMap<OpenVolumeMesh::EdgeHandle, OpenVolumeMesh::VertexHandle> indexmapping_ev;
	QMap<OpenVolumeMesh::FaceHandle, OpenVolumeMesh::VertexHandle> indexmapping_fv;
	QMap<OpenVolumeMesh::CellHandle, OpenVolumeMesh::VertexHandle> indexmapping_cv;
	int n_v = tetmesh->n_vertices();
	int n_e = tetmesh->n_edges();
	int n_f = tetmesh->n_faces();
	int n_c = tetmesh->n_cells();

	/*int num = n_v + n_e + n_f + n_c;
	fout<<num<<std::endl;*/

	for(auto v_iter = tetmesh->vertices_begin(); v_iter != tetmesh->vertices_end(); ++v_iter)
	{
		auto p = tetmesh->vertex(*v_iter).data();
		auto vh = mesh_hex->add_vertex(OvmVec3d(p[0],p[1],p[2]));
		indexmapping_vv.insert(*v_iter,vh);
	}
	for(auto e_iter = tetmesh->edges_begin(); e_iter != tetmesh->edges_end(); ++e_iter){
		auto p1 = tetmesh->vertex(tetmesh->edge(*e_iter).from_vertex()).data();
		auto p2 = tetmesh->vertex(tetmesh->edge(*e_iter).to_vertex()).data();
		auto p = OvmVec3d((p1[0]+p2[0])/2,(p1[1]+p2[1])/2,(p1[2]+p2[2])/2);
		auto vh = mesh_hex->add_vertex(p);
		indexmapping_ev.insert(*e_iter,vh);
	}
	for(auto f_iter = tetmesh->faces_begin(); f_iter != tetmesh->faces_end(); ++f_iter)
	{
		auto hfh = tetmesh->halfface_handle(*f_iter,0);
		double x(0.0),y(0.0),z(0.0);
		for(auto hfv_iter = tetmesh->hfv_iter(hfh); hfv_iter; ++hfv_iter){
			auto p = tetmesh->vertex(*hfv_iter).data();
			x += p[0];
			y += p[1];
			z += p[2];
		}
		auto p = OvmVec3d(x/3,y/3,z/3);
		auto vh = mesh_hex->add_vertex(p);
		indexmapping_fv.insert(*f_iter,vh);
	}
	std::vector<OpenVolumeMesh::VertexHandle> v_vec;
	for(auto c_iter = tetmesh->cells_begin(); c_iter != tetmesh->cells_end(); ++c_iter){
		v_vec.clear();
		for(auto cv_iter = tetmesh->cv_iter(*c_iter); cv_iter; ++cv_iter)
			v_vec.push_back(*cv_iter);

		auto p1 = tetmesh->vertex(v_vec[0]).data();
		auto p2 = tetmesh->vertex(v_vec[1]).data();
		auto p3 = tetmesh->vertex(v_vec[2]).data();
		auto p4 = tetmesh->vertex(v_vec[3]).data();
		auto p = OvmVec3d((p1[0]+p2[0]+p3[0]+p4[0])/4,(p1[1]+p2[1]+p3[1]+p4[1])/4,(p1[2]+p2[2]+p3[2]+p4[2])/4);
		auto vh = mesh_hex->add_vertex(p);
		indexmapping_cv.insert(*c_iter,vh);
	}

	std::vector<OpenVolumeMesh::VertexHandle> v_hex_vec;
	for(auto c_iter = tetmesh->cells_begin(); c_iter != tetmesh->cells_end(); ++c_iter){
		v_vec.clear();
		for(auto cv_iter = tetmesh->cv_iter(*c_iter); cv_iter; ++cv_iter)
			v_vec.push_back(*cv_iter);

		for(int i = 0; i != 4; i++){
			v_hex_vec.clear();
			auto hf_vec = tetmesh->cell(*c_iter).halffaces();
			OpenVolumeMesh::HalfFaceHandle hfh;
			bool is_ok = false;
			for(auto hfv_iter = tetmesh->hfv_iter(hf_vec[0]); hfv_iter; ++hfv_iter)
				if(v_vec[i] == *hfv_iter){
					is_ok = true;
					break;
				}
				if(is_ok == true)
					hfh = hf_vec[0];
				else
					hfh = hf_vec[1];

				auto he_vec = tetmesh->halfface(hfh).halfedges();
				OpenVolumeMesh::HalfEdgeHandle heh_temp;
				for(int j = 0; j != 3; j++)
					if(tetmesh->halfedge(he_vec[j]).to_vertex() == v_vec[i]){
						heh_temp = he_vec[j];
						break;
					}
					OpenVolumeMesh::VertexHandle vh_temp;
					auto eh = tetmesh->edge_handle(heh_temp);
					vh_temp = indexmapping_ev[eh];
					v_hex_vec.push_back(vh_temp);
					auto fh = tetmesh->face_handle(hfh);
					vh_temp = indexmapping_fv[fh];
					v_hex_vec.push_back(vh_temp);
					eh = tetmesh->edge_handle(tetmesh->next_halfedge_in_halfface(heh_temp, hfh));
					vh_temp = indexmapping_ev[eh];
					v_hex_vec.push_back(vh_temp);
					vh_temp = indexmapping_vv[v_vec[i]];
					v_hex_vec.push_back(vh_temp);

					hfh = tetmesh->adjacent_halfface_in_cell(hfh, heh_temp);
					fh = tetmesh->face_handle(hfh);
					vh_temp = indexmapping_fv[fh];
					v_hex_vec.push_back(vh_temp);
					heh_temp = tetmesh->prev_halfedge_in_halfface(tetmesh->opposite_halfedge_handle(heh_temp),hfh);
					eh = tetmesh->edge_handle(heh_temp);
					vh_temp = indexmapping_ev[eh];
					v_hex_vec.push_back(vh_temp);
					hfh = tetmesh->adjacent_halfface_in_cell(hfh, heh_temp);
					fh = tetmesh->face_handle(hfh);
					vh_temp = indexmapping_fv[fh];
					v_hex_vec.push_back(vh_temp);
					vh_temp = indexmapping_cv[*c_iter];
					v_hex_vec.push_back(vh_temp);
					auto ch = mesh_hex->add_cell (v_hex_vec);
		}
	}
	return mesh_hex;
}

VolumeMesh* tet2hex(VMesh* tetmesh, BODY* body)
{
	if (!tetmesh->vertex_property_exists<unsigned long>("entityptr"))
	{
		attach_tet_mesh_elements_to_ACIS_entities(tetmesh,body);
	}
	auto V_ENTITY_PTR = tetmesh->request_vertex_property<unsigned long>("entityptr");
	VolumeMesh *mesh_hex = new VolumeMesh ();
	QMap<OpenVolumeMesh::VertexHandle, OpenVolumeMesh::VertexHandle> indexmapping_vv;
	QMap<OpenVolumeMesh::EdgeHandle, OpenVolumeMesh::VertexHandle> indexmapping_ev;
	QMap<OpenVolumeMesh::FaceHandle, OpenVolumeMesh::VertexHandle> indexmapping_fv;
	QMap<OpenVolumeMesh::CellHandle, OpenVolumeMesh::VertexHandle> indexmapping_cv;
	int n_v = tetmesh->n_vertices();
	int n_e = tetmesh->n_edges();
	int n_f = tetmesh->n_faces();
	int n_c = tetmesh->n_cells();

	/*int num = n_v + n_e + n_f + n_c;
	fout<<num<<std::endl;*/

	for(auto v_iter = tetmesh->vertices_begin(); v_iter != tetmesh->vertices_end(); ++v_iter)
	{
		auto p = tetmesh->vertex(*v_iter).data();
		auto vh = mesh_hex->add_vertex(OvmVec3d(p[0],p[1],p[2]));
		indexmapping_vv.insert(*v_iter,vh);
	}
	for(auto e_iter = tetmesh->edges_begin(); e_iter != tetmesh->edges_end(); ++e_iter){
		OvmVec3d p;
		if (tetmesh->is_boundary(*e_iter))
		{
			p = find_ovmegh_mid_pos_in_body(tetmesh,body,*e_iter);
		}
		else
		{
			auto p1 = tetmesh->vertex(tetmesh->edge(*e_iter).from_vertex()).data();
			auto p2 = tetmesh->vertex(tetmesh->edge(*e_iter).to_vertex()).data();
			p = OvmVec3d((p1[0]+p2[0])/2,(p1[1]+p2[1])/2,(p1[2]+p2[2])/2);
		}
		auto vh = mesh_hex->add_vertex(p);
		indexmapping_ev.insert(*e_iter,vh);
	}
	for(auto f_iter = tetmesh->faces_begin(); f_iter != tetmesh->faces_end(); ++f_iter)
	{
		auto hfh = tetmesh->halfface_handle(*f_iter,0);
		double x(0.0),y(0.0),z(0.0);
		for(auto hfv_iter = tetmesh->hfv_iter(hfh); hfv_iter; ++hfv_iter){
			auto p = tetmesh->vertex(*hfv_iter).data();
			x += p[0];
			y += p[1];
			z += p[2];
		}
		auto p = OvmVec3d(x/3,y/3,z/3);
		if (tetmesh->is_boundary(*f_iter))
		{
			FACE* Face = get_associated_geometry_face_of_boundary_fh_gen_ver(tetmesh,*f_iter);
			SPAposition closet_p;double dis;
			api_entity_point_distance(Face, POS2SPA(p),closet_p, dis);
			//std::cout<<pre_pos[0]<<" "<<pre_pos[1]<<" "<<pre_pos[2]<<std::endl;
			p = SPA2POS(closet_p);
		}
		auto vh = mesh_hex->add_vertex(p);
		indexmapping_fv.insert(*f_iter,vh);
	}
	std::vector<OpenVolumeMesh::VertexHandle> v_vec;
	for(auto c_iter = tetmesh->cells_begin(); c_iter != tetmesh->cells_end(); ++c_iter){
		v_vec.clear();
		for(auto cv_iter = tetmesh->cv_iter(*c_iter); cv_iter; ++cv_iter)
			v_vec.push_back(*cv_iter);

		auto p1 = tetmesh->vertex(v_vec[0]).data();
		auto p2 = tetmesh->vertex(v_vec[1]).data();
		auto p3 = tetmesh->vertex(v_vec[2]).data();
		auto p4 = tetmesh->vertex(v_vec[3]).data();
		auto p = OvmVec3d((p1[0]+p2[0]+p3[0]+p4[0])/4,(p1[1]+p2[1]+p3[1]+p4[1])/4,(p1[2]+p2[2]+p3[2]+p4[2])/4);
		auto vh = mesh_hex->add_vertex(p);
		indexmapping_cv.insert(*c_iter,vh);
	}

	std::vector<OpenVolumeMesh::VertexHandle> v_hex_vec;
	for(auto c_iter = tetmesh->cells_begin(); c_iter != tetmesh->cells_end(); ++c_iter)
	{
		v_vec.clear();
		for(auto cv_iter = tetmesh->cv_iter(*c_iter); cv_iter; ++cv_iter)
			v_vec.push_back(*cv_iter);

		for(int i = 0; i != 4; i++){
			v_hex_vec.clear();
			auto hf_vec = tetmesh->cell(*c_iter).halffaces();
			OpenVolumeMesh::HalfFaceHandle hfh;
			bool is_ok = false;
			for(auto hfv_iter = tetmesh->hfv_iter(hf_vec[0]); hfv_iter; ++hfv_iter)
				if(v_vec[i] == *hfv_iter){
					is_ok = true;
					break;
				}
			if(is_ok == true)
				hfh = hf_vec[0];
			else
				hfh = hf_vec[1];

			auto he_vec = tetmesh->halfface(hfh).halfedges();
			OpenVolumeMesh::HalfEdgeHandle heh_temp;
			for(int j = 0; j != 3; j++)
				if(tetmesh->halfedge(he_vec[j]).to_vertex() == v_vec[i]){
					heh_temp = he_vec[j];
					break;
				}
			OpenVolumeMesh::VertexHandle vh_temp;
			auto eh = tetmesh->edge_handle(heh_temp);
			vh_temp = indexmapping_ev[eh];
			v_hex_vec.push_back(vh_temp);
			auto fh = tetmesh->face_handle(hfh);
			vh_temp = indexmapping_fv[fh];
			v_hex_vec.push_back(vh_temp);
			eh = tetmesh->edge_handle(tetmesh->next_halfedge_in_halfface(heh_temp, hfh));
			vh_temp = indexmapping_ev[eh];
			v_hex_vec.push_back(vh_temp);
			vh_temp = indexmapping_vv[v_vec[i]];
			v_hex_vec.push_back(vh_temp);

			hfh = tetmesh->adjacent_halfface_in_cell(hfh, heh_temp);
			fh = tetmesh->face_handle(hfh);
			vh_temp = indexmapping_fv[fh];
			v_hex_vec.push_back(vh_temp);
			heh_temp = tetmesh->prev_halfedge_in_halfface(tetmesh->opposite_halfedge_handle(heh_temp),hfh);
			eh = tetmesh->edge_handle(heh_temp);
			vh_temp = indexmapping_ev[eh];
			v_hex_vec.push_back(vh_temp);
			hfh = tetmesh->adjacent_halfface_in_cell(hfh, heh_temp);
			fh = tetmesh->face_handle(hfh);
			vh_temp = indexmapping_fv[fh];
			v_hex_vec.push_back(vh_temp);
			vh_temp = indexmapping_cv[*c_iter];
			v_hex_vec.push_back(vh_temp);
			auto ch = mesh_hex->add_cell (v_hex_vec);
		}
	}
	return mesh_hex;
}

std::vector<OvmVeH> get_ordered_vhs(VolumeMesh* mesh, OvmCeH ch){
	auto ovm_mesh = mesh;
	std::vector<OvmVeH> vhs;
	auto adj_hfhs = ovm_mesh->cell(ch).halffaces();
	for (auto hfv_it = ovm_mesh->hfv_iter(adj_hfhs[0]);hfv_it;hfv_it++)
		vhs.push_back(*hfv_it);
	std::unordered_set<int> vhs_set;
	foreach(auto vh, vhs) vhs_set.insert(vh.idx());
	OvmHaFaH opposite_hfh = ovm_mesh->InvalidHalfFaceHandle;
	for (int hid = 1;hid < adj_hfhs.size();hid++){
		bool if_target = true;
		for (auto hfv_it = ovm_mesh->hfv_iter(adj_hfhs[hid]);hfv_it;hfv_it++){
			if (vhs_set.find(hfv_it->idx()) != vhs_set.end()){
				if_target = false;
				break;
			}
		}
		if (if_target){
			opposite_hfh = adj_hfhs[hid];
			break;
		}
	}
	std::vector<OvmVeH> opposite_vhs;
	int target_vh_id = -1;
	for (auto hfv_it = ovm_mesh->hfv_iter(opposite_hfh);hfv_it;hfv_it++){
		opposite_vhs.push_back(*hfv_it);
		if (ovm_mesh->halfedge(vhs[0], *hfv_it) != ovm_mesh->InvalidHalfEdgeHandle){
			target_vh_id = opposite_vhs.size()-1;
		}
	}
	for (int vid = 0;vid < 4;vid++){
		vhs.push_back(opposite_vhs[(vid+target_vh_id)%4]);
	}
	std::swap(vhs[1], vhs[3]);
	std::swap(vhs[5], vhs[7]);
	//test
	return vhs;
}

void Get_Feature_Infos(VolumeMesh* mesh, std::vector<OvmVeH>& corners, std::vector<OvmEgH>& edges){
	edges.clear();corners.clear();
	double threshold = 0.707;
	auto fGetFeatureEdges = [&](){
		for (auto e_iter = mesh->edges_begin();e_iter != mesh->edges_end();e_iter++){
			if (!mesh->is_boundary(*e_iter)) continue;
			int adj_cell_counter = 0;
			for (auto hec_iter = mesh->hec_iter(mesh->halfedge_handle(*e_iter, 0));hec_iter;hec_iter++){
				if (*hec_iter != mesh->InvalidCellHandle)
					adj_cell_counter++;
			}
			if (adj_cell_counter != 2)
				edges.push_back(*e_iter);
			/*
			OvmHaFaH adj_hfhs[2];int hf_counter = 0;
			OvmHaEgH adj_hehs[2];
			for (auto hehf_iter = mesh->hehf_iter(mesh->halfedge_handle(*e_iter, 0));hehf_iter;hehf_iter++){
				if (mesh->is_boundary(*hehf_iter)){
					adj_hehs[hf_counter] = mesh->halfedge_handle(*e_iter, 0);
					adj_hfhs[hf_counter++] = *hehf_iter;
				}
				if (mesh->is_boundary(mesh->opposite_halfface_handle(*hehf_iter))){
					adj_hehs[hf_counter] = mesh->halfedge_handle(*e_iter, 1);
					adj_hfhs[hf_counter++] = mesh->opposite_halfface_handle(*hehf_iter);
				}
			}
			assert(hf_counter != 2);
			OvmVec3d normals[2];
			for (int fi = 0;fi < 2;fi++){
				auto center = mesh->barycenter(mesh->face_handle(adj_hfhs[fi]));
				auto p1 = mesh->vertex(mesh->halfedge(adj_hehs[fi]).from_vertex());
				auto p2 = mesh->vertex(mesh->halfedge(adj_hehs[fi]).to_vertex());
				normals[fi] = OpenVolumeMesh::Geometry::cross((center-p1),(p2-center));
			}
			auto cosine_value = OpenVolumeMesh::Geometry::dot(normals[0], normals[1]);
			if (std::abs(cosine_value) < threshold)
				edges.push_back(*e_iter);
				*/
		}
	};
	auto fGetCorners = [&](){
		std::unordered_set<OvmEgH> ehs_set;foreach (auto edge, edges) ehs_set.insert(edge);
		for (auto v_iter = mesh->vertices_begin();v_iter != mesh->vertices_end();v_iter++){
			int adj_counter = 0;
			for (auto voh_iter = mesh->voh_iter(*v_iter);voh_iter;voh_iter++){
				if (ehs_set.find(mesh->edge_handle(*voh_iter)) != ehs_set.end())
					adj_counter++;
			}
			if (adj_counter != 2 && adj_counter > 0)
				corners.push_back(*v_iter);
		}
	};
	/*************** main body *************************/
	fGetFeatureEdges();
	fGetCorners();
	//std::cout<<"Edges Vhs size "<<edges.size()<<" "<<corners.size()<<std::endl;
}

void Get_Block_Infos(VolumeMesh* mesh, std::unordered_set<OvmCeH> block, std::unordered_set<OvmVeH>& corner_vhs, 
	std::unordered_set<OvmEgH>& feature_edges, std::vector<std::unordered_set<OvmVeH>>& boundary_patch_vhs){
		corner_vhs.clear();feature_edges.clear();
		std::unordered_set<OvmVeH> feature_vhs;
		boundary_patch_vhs.clear();
		std::unordered_set<OvmFaH> boundary_fhs;
		auto fGetFeatureEdges = [&](){
			for (auto e_iter = mesh->edges_begin();e_iter != mesh->edges_end();e_iter++){
				int adj_c_counter = 0;
				for (auto hec_iter = mesh->hec_iter(mesh->halfedge_handle(*e_iter, 0));hec_iter;hec_iter++){
					if (block.find(*hec_iter) != block.end())
						adj_c_counter++;
				}
				if (adj_c_counter == 1)
					feature_edges.insert(*e_iter);
			}
		};
		auto fGetFeatureVhs = [&](){
			foreach (auto e, feature_edges){
				feature_vhs.insert(mesh->edge(e).from_vertex());
				feature_vhs.insert(mesh->edge(e).to_vertex());
			}
		};
		auto fGetCornerVhs = [&](){
			foreach (auto v, feature_vhs){
				int adj_e_counter = 0;
				for (auto voh_it = mesh->voh_iter(v);voh_it;voh_it++){
					if (feature_edges.find(mesh->edge_handle(*voh_it)) != feature_edges.end())
					//if (feature_vhs.find(mesh->halfedge(*voh_it).to_vertex()) != feature_vhs.end())
						adj_e_counter++;
				}
				if (adj_e_counter == 3)
					corner_vhs.insert(v);
			}
			if (corner_vhs.size() != 8){
				std::cout<<"Error !"<<corner_vhs.size()<<std::endl;
				//std::cout<<feature_vhs.size()<<"   "<<corner_vhs.size()<<std::endl;
			}
		};
		auto fGetBoundaryPatch = [&](){
			for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++){
				int incident_counter = 0;
				if (block.find(mesh->incident_cell(mesh->halfface_handle(*f_it, 0))) != block.end()) incident_counter++;
				if (block.find(mesh->incident_cell(mesh->halfface_handle(*f_it, 1))) != block.end()) incident_counter++;
				if (incident_counter == 1) boundary_fhs.insert(*f_it);
			}
			//std::cout<<"boundary_fhs.size is "<<boundary_fhs.size()<<std::endl;
			std::queue<OvmFaH> q;
			while(!boundary_fhs.empty()){
				q.push(*boundary_fhs.begin()); boundary_fhs.erase(boundary_fhs.begin());
				std::unordered_set<OvmFaH> patch;
				while(!q.empty()){
					auto q_top = q.front(); q.pop();
					patch.insert(q_top);
					auto adj_hehs = mesh->halfface(mesh->halfface_handle(q_top, 0)).halfedges();
					foreach (auto heh, adj_hehs){
						if (feature_edges.find(mesh->edge_handle(heh)) != feature_edges.end()) continue;
						for (auto hehf_it = mesh->hehf_iter(heh);hehf_it;hehf_it++){
							auto current_fh = mesh->face_handle(*hehf_it);
							if (boundary_fhs.find(current_fh) != boundary_fhs.end()){
								q.push(current_fh);boundary_fhs.erase(boundary_fhs.find(current_fh));
							}
						}
					}
				}
				std::unordered_set<OvmVeH> patch_vhs;
				foreach (auto fh, patch){
					for (auto fv_it = mesh->hfv_iter(mesh->halfface_handle(fh, 0)); fv_it;fv_it++){
						patch_vhs.insert(*fv_it);
					}
				}
				//std::cout<<"patch fhs size is "<<patch.size()<<std::endl;
				boundary_patch_vhs.push_back(patch_vhs);patch_vhs.clear();
				//q.push(*boundary_fhs.begin()); boundary_fhs.erase(boundary_fhs.begin());
			}
			if (boundary_patch_vhs.size() != 6){
				std::cout<<"Error size "<<boundary_patch_vhs.size()<<std::endl;
				//exit(0);
			}
		};
		/******************  end lamdas*****************************/
		fGetFeatureEdges();
		fGetFeatureVhs();
		fGetCornerVhs();
		fGetBoundaryPatch();
}




std::vector<OvmHaEgH> Get_Topology_Next_Hehs(VolumeMesh* mesh, OvmHaEgH heh){
	std::vector<OvmHaEgH> next_hehs;
	std::unordered_set<OvmCeH> adj_chs;
	for (auto hec_it = mesh->hec_iter(heh);hec_it;hec_it++){
		if (*hec_it != mesh->InvalidCellHandle)	adj_chs.insert(*hec_it);
	}
	for (auto voh_it = mesh->voh_iter(mesh->halfedge(heh).to_vertex());voh_it;voh_it++){
		if (mesh->opposite_halfedge_handle(*voh_it) == heh) continue;
		bool if_target = true;
		for (auto hec_it = mesh->hec_iter(*voh_it);hec_it;hec_it++){
			if (adj_chs.find(*hec_it) != adj_chs.end()){
				if_target = false;
				break;
			}

		}
		if (if_target)
			next_hehs.push_back(*voh_it);
	}
	return next_hehs;
}

void Get_Base_Complex(VolumeMesh* mesh, std::vector<std::unordered_set<OvmCeH>>& blocks, 
	std::vector<std::unordered_set<OvmVeH>>& block_boundary_vhs, std::vector<std::unordered_set<OvmVeH>>& block_inner_vhs)
{
	//Step 0 初始化
	//Step 1 获取网格奇异线
	//Step 2 从奇异线出发获取分割面
	//Step 3 利用分割线 和 BFS获取不同的块
	auto hex_mesh = mesh;
	//Step 0
	std::unordered_set<OvmFaH> separatrix_faces;
	std::unordered_set<OvmEgH> separatrix_edges;
	std::unordered_set<OvmEgH> separatrix_edges_3;
	std::unordered_set<OvmEgH> separatrix_edges_5;
	std::unordered_set<OvmEgH> inner_spehs;
	std::vector<std::unordered_set<OvmCeH>> block_cells;

	//计算获取同一个FaceHandle上与一条边对应的EdgeHandle
	auto fGetTopParallelEH = [&](OvmFaH fh, OvmEgH eh) -> OvmEgH {
		OvmVeH svh = hex_mesh->edge(eh).from_vertex();
		OvmVeH evh = hex_mesh->edge(eh).to_vertex();
		OvmVeH diffvh[2];int couter_v = 0;
		for (auto hfv_it = hex_mesh->hfv_iter(hex_mesh->halfface_handle(fh,0));hfv_it;hfv_it++)
		{
			if (*hfv_it != svh && *hfv_it != evh)
			{
				diffvh[couter_v++] = *hfv_it;
			}
		}
		OvmHaEgH pa_he = hex_mesh->halfedge(diffvh[0],diffvh[1]);
		OvmEgH _re_he = hex_mesh->edge_handle(pa_he);
		return _re_he;
	};
	//计算一条内部四度边面延展的下个面
	auto fGetNextFH = [&](OvmFaH fh, OvmEgH eh) ->OvmFaH {
		//OvmFaH next_fh = hex_mesh->InvalidFaceHandle;
		for (auto hehf_it = hex_mesh->hehf_iter(hex_mesh->halfedge_handle(eh,0));hehf_it;hehf_it++)
		{
			OvmFaH current_fh = hex_mesh->face_handle(*hehf_it);
			if (current_fh != hex_mesh->InvalidFaceHandle && current_fh != fh)
			{
				OvmCeH common_ch = get_common_cell_handle(hex_mesh,fh,current_fh);
				if (common_ch == hex_mesh->InvalidCellHandle)
				{
					return current_fh;
				}
			}
		}
	};

	//Step 1
	int inner_sg =0;int boundary_sg = 0;
	for (auto e_it = hex_mesh->edges_begin();e_it != hex_mesh->edges_end();e_it++)
	{
		int counter = 0;
		for (auto hehf_it = hex_mesh->hehf_iter(hex_mesh->halfedge_handle(*e_it,0));hehf_it;hehf_it++)
		{
			OvmCeH ch = hex_mesh->incident_cell(*hehf_it);
			if (ch != hex_mesh->InvalidCellHandle)
			{
				counter++;
			}
		}
		//std::cout<<"if boudnary : "<<hex_mesh->is_boundary(*e_it)<<" "<<counter<<std::endl;
		if (hex_mesh->is_boundary(*e_it))
		{
			
			//
			
			//geometry edge not considered
			if (counter != 2)
				separatrix_edges.insert(*e_it);
			if (counter == 1)
				separatrix_edges_3.insert(*e_it);
			if (counter == 3)
				separatrix_edges_5.insert(*e_it);

			if (counter != 2)
			{
				boundary_sg++;
			}
		}
		else
		{
			if (counter != 4)
			{
				separatrix_edges.insert(*e_it);
				if (counter == 3)
					separatrix_edges_3.insert(*e_it);
				else if (counter == 5)
					separatrix_edges_5.insert(*e_it);
				inner_spehs.insert(*e_it);
				inner_sg++;
			}
		}
	}
	//std::cout<<"Inner_sg: "<<inner_sg<<" boundary_sg: "<<boundary_sg<<std::endl;
	//Step 2 先将表面全部放进分割面内 也可以不放
	//同样利用类似BFS思想
	std::vector<bool> if_face_visited(hex_mesh->n_faces(),false);
	bool if_consider_boundary = false;
	if (if_consider_boundary)
	{
		for (auto bf_it = hex_mesh->bf_iter();bf_it;bf_it++)
		{
			separatrix_faces.insert(*bf_it);
		}
	}


	for (auto e_it = separatrix_edges.begin();e_it != separatrix_edges.end();e_it++)
	{
		for (auto hehf_it = hex_mesh->hehf_iter(hex_mesh->halfedge_handle(*e_it,0));hehf_it;hehf_it++)
		{
			OvmFaH current_fh = hex_mesh->face_handle(*hehf_it);
			//if_face_visited[current_fh.idx()] = true;
			if (current_fh == hex_mesh->InvalidFaceHandle || hex_mesh->is_boundary(current_fh)||if_face_visited[current_fh.idx()])
			{
				continue;
			}
			//OvmEgH next_eh = fGetTopParallelEH(current_fh,*e_it);
			std::queue<OvmFaH> q;
			if_face_visited[current_fh.idx()] = true;
			q.push(current_fh);
			while(!q.empty())
			{
				OvmFaH q_top = q.front();
				separatrix_faces.insert(q_top);
				q.pop();

				auto adj_hehs = hex_mesh->face(q_top).halfedges();
				for (auto he_it = adj_hehs.begin();he_it != adj_hehs.end();he_it++)
				{
					OvmEgH current_eh = hex_mesh->edge_handle(*he_it);
					if (hex_mesh->is_boundary(current_eh) || separatrix_edges.find(current_eh) != separatrix_edges.end())
					{
						continue;
					}
					current_fh = fGetNextFH(q_top,current_eh);
					if (!if_face_visited[current_fh.idx()] && current_fh != hex_mesh->InvalidFaceHandle)
					{
						if_face_visited[current_fh.idx()] = true;
						q.push(current_fh);
					}
				}
			}
		
		}
	}
	//Step 3 利用BFS将体分块
	std::vector<bool> if_cell_visited(hex_mesh->n_cells(),false);
	std::vector<bool> if_separatrix_faces(hex_mesh->n_faces(),false);

	for (auto sf_it = separatrix_faces.begin();sf_it != separatrix_faces.end();sf_it++)
	{
		if_separatrix_faces[sf_it->idx()] = true;
	}
	int residual_c = hex_mesh->n_cells();
	do 
	{
		if (residual_c == 0)
		{
			break;
		}
		OvmCeH seed_ch;
		for (auto c_it = hex_mesh->cells_begin();c_it != hex_mesh->cells_end();c_it++)
		{
			if (!if_cell_visited[c_it->idx()])
			{
				if_cell_visited[c_it->idx()] = true;
				seed_ch = *c_it;
				break;
			}
		}
		std::queue<OvmCeH> q;
		q.push(seed_ch);
		std::unordered_set<OvmCeH> cell_group;
		while(!q.empty())
		{
			OvmCeH q_top = q.front();
			q.pop();
			cell_group.insert(q_top);
			for (auto cc_it = hex_mesh->cc_iter(q_top);cc_it;cc_it++)
			{
				if (*cc_it != hex_mesh->InvalidCellHandle)
				{
					OvmCeH current_ch = *cc_it;
					OvmFaH common_fh = get_common_face_handle(hex_mesh,current_ch,q_top);
					if (if_separatrix_faces[common_fh.idx()]||if_cell_visited[current_ch.idx()])
					{
						continue;
					}
					if_cell_visited[current_ch.idx()] = true;
					q.push(current_ch);
				}
			}
		}
		block_cells.push_back(cell_group);
		residual_c -= cell_group.size();
		std::unordered_set<OvmCeH>().swap(cell_group);
	} while (true);
	blocks = block_cells;
	block_boundary_vhs.clear();
	block_inner_vhs.clear();
	for (int bi = 0;bi < blocks.size();bi++){
		std::unordered_set<OvmFaH> fhs;
		foreach (auto ch, blocks[bi]){
			auto adj_hfhs = mesh->cell(ch).halffaces();
			foreach (auto hfh, adj_hfhs){
				auto fh = mesh->face_handle(hfh);
				if (fhs.find(fh) != fhs.end())
					fhs.erase(fhs.find(fh));
				else
					fhs.insert(fh);
			}
		}
		std::unordered_set<OvmVeH> boundary_vhs;
		foreach (auto fh, fhs)
			for (auto hfv_it = mesh->hfv_iter(mesh->halfface_handle(fh, 0));hfv_it;hfv_it++){
				boundary_vhs.insert(*hfv_it);
			}
		std::unordered_set<OvmVeH> inner_vhs;
		foreach (auto ch, blocks[bi]){
			for (auto cv_it = mesh->cv_iter(ch);cv_it;cv_it++){
				if (boundary_vhs.find(*cv_it) == boundary_vhs.end())
					inner_vhs.insert(*cv_it);
			}
		}
		block_boundary_vhs.push_back(boundary_vhs);
		block_inner_vhs.push_back(inner_vhs);
	}
}


void Refine_Hex(VolumeMesh* mesh, BODY *body, int segment){
	VolumeMesh* new_mesh = new VolumeMesh();
	//TODO to be implemented!!
	//
}

OvmEgH tet_opposite_edge(VMesh* mesh,OvmCeH ch, OvmEgH eh)
{
	OvmVeH v_s = mesh->edge(eh).from_vertex();
	OvmVeH v_e = mesh->edge(eh).to_vertex();
	std::vector<OvmVeH> vhs;vhs.clear();
	for (auto cv_it = mesh->cv_iter(ch);cv_it;cv_it++)
	{
		if (*cv_it != v_s&&*cv_it != v_e)
		{
			vhs.push_back(*cv_it);
		}
	}
	OvmHaEgH heh = mesh->halfedge(vhs[0],vhs[1]);
	return mesh->edge_handle(heh);
}

void tet_edge_splitting(VMesh* mesh,OvmEgH eh,std::vector<OvmCeH>& _chs)
{
	std::clock_t start_t1,end_t1,start_t2,end_t2;
	start_t1 = clock();
	struct New_Point{
		OvmVeH vh;
		OvmVec3d v_position;
	};
	//Step 0 将所有原VertexHandle作为属性存储起来
	//Step 1 遍历所有与eh相邻的四面体单元，判断与之相邻的所有节点在删除掉所有该删的四面体时候是否也会被删除
	//       判断依据是这个节点相邻的四面体是否是只有这些要删除的四面体，若会被删除则构建一个New_Point存储Vh和几何点位置
	//Step 2 构建需要新建面的Vertex Handle值，以及构造新的两类面所需要的三个VH值的vector，同时需要增加因为悬空点被删而被删的面
	//Step 3 删除OvmCeH
	//Step 4 增加点，包括边中心点和悬空点,并构建old2new_vh_mapping
	//Step 5 增加面,增加体
	OvmVeH v_s = mesh->edge(eh).from_vertex();OvmVeH v_e = mesh->edge(eh).to_vertex();
	OpenVolumeMesh::StatusAttrib status_attrib(*mesh);
	//Step 0
	
	auto V_PREV_HANDLE = mesh->request_vertex_property<OvmVeH>("prevhandle");
	for (auto v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++)
		V_PREV_HANDLE[*v_it] = *v_it;
	//用于判定这个面是否在接下来中要用到
	auto F_IF_COVERED = mesh->request_face_property<int>("coveredfacehandle",0);
	//Step 1
	std::unordered_set<OvmCeH> deleted_chs;deleted_chs.clear();//所有待删除的cell handle
	std::unordered_set<OvmVeH> adj_deleted_vhs;adj_deleted_vhs.clear();//所有删除四面体单元之后悬空的点
	std::vector<std::vector<OvmVeH>> can_add_f_vhs;can_add_f_vhs.clear();//所有后续需要添加的面（vertices表示），包括多删的面，和后需要加的面
	for (auto hec_it = mesh->hec_iter(mesh->halfedge_handle(eh,0));hec_it;hec_it++)
	{
		if (*hec_it == mesh->InvalidCellHandle)
		{
			continue;
		}
		deleted_chs.insert(*hec_it);
	}
	//遍历所有与要删Cell相邻的VH,若这个VH只与要被删的Cells相邻则该VH也将被删除
	for (auto dc_it = deleted_chs.begin();dc_it != deleted_chs.end();dc_it++)
	{
		bool vh_deleted;
		for (auto cv_it = mesh->cv_iter(*dc_it);cv_it;cv_it++)
		{
			vh_deleted = true;
			for (auto vc_it = mesh->vc_iter(*cv_it);vc_it;vc_it++)
			{
				if (deleted_chs.find(*vc_it) == deleted_chs.end())
				{
					vh_deleted = false;
					break;
				}
			}
			if (vh_deleted == true)
			{
				adj_deleted_vhs.insert(*cv_it);
			}
		}
	}
	//构建在删除Cell后需要添加的Vertex的信息,所有这些信息集中在candidate_point中
	std::vector<New_Point> candidate_point;candidate_point.clear();
	OvmVec3d mid_position = (mesh->vertex(mesh->edge(eh).from_vertex())+mesh->vertex(mesh->edge(eh).to_vertex()))/2.0;
	OvmVeH new_vh = (OvmVeH) mesh->n_vertices();//这里新的节点可能有点问题
	New_Point mid_p;mid_p.v_position = mid_position;mid_p.vh = new_vh;
	candidate_point.push_back(mid_p);
	for (auto dv_it = adj_deleted_vhs.begin();dv_it != adj_deleted_vhs.end();dv_it++)
	{
		New_Point deleted_p;
		deleted_p.v_position = mesh->vertex(*dv_it);deleted_p.vh = *dv_it;
		candidate_point.push_back(deleted_p);
	}
	//遍历所有被删除的Cell，查看与被删的Cell的所有面是否有可能被删，这里要排除掉包含分裂边edgehandle的面，这是在之后处理用
	//若被删除则记录这个Face 的三个Vertex Handle，为避免多加Face,添加deleted_fhs;
	std::unordered_set<OvmFaH> deleted_fhs;
	for (auto dc_it = deleted_chs.begin();dc_it != deleted_chs.end();dc_it++)
	{
		//////////////////////////////////////////////////////////////////////////
		bool fh_deleted;
		auto adj_hfhs = mesh->cell(*dc_it).halffaces();
		for (auto can_hfh = adj_hfhs.begin();can_hfh != adj_hfhs.end();can_hfh++)
		{
			F_IF_COVERED[mesh->face_handle(*can_hfh)] = 1;
			fh_deleted = false;
			std::vector<OvmVeH> f_vhs;f_vhs.clear();
			auto hehs = mesh->face(mesh->face_handle(*can_hfh)).halfedges();
			std::vector<OvmEgH> _ehs;
			foreach(OvmHaEgH heh_ind,hehs)
			{
				_ehs.push_back(mesh->edge_handle(heh_ind));
			}
			std::vector<OvmHaEgH>().swap(hehs);
			//要删掉处于边界上或者这个面的两侧cell都被删除了，且这个半面不包含输入的Edge Handle
			if ((mesh->is_boundary(mesh->opposite_halfface_handle(*can_hfh))||contains(deleted_chs,mesh->incident_cell(mesh->opposite_halfface_handle(*can_hfh))))&&(!contains(_ehs,eh)))
			{
				fh_deleted = true;
			}
			//或者找到构成这个面的三个vertex中是否存在被删掉的vertex，若有，则同样需要被删除
			for (auto hfv_it = mesh->hfv_iter(*can_hfh);hfv_it;hfv_it++)
			{
				f_vhs.push_back(*hfv_it);
				if (adj_deleted_vhs.find(*hfv_it) != adj_deleted_vhs.end())
				{
					fh_deleted = true;
				}
			}

			if (fh_deleted&&(deleted_fhs.find(mesh->face_handle(*can_hfh)) == deleted_fhs.end()))
			{
				deleted_fhs.insert(mesh->face_handle(*can_hfh));
				can_add_f_vhs.push_back(f_vhs);
			}
			std::vector<OvmEgH>().swap(_ehs);
			std::vector<OvmVeH>().swap(f_vhs);
		}
	}
	std::unordered_set<OvmFaH>().swap(deleted_fhs);
	//Step 2
	//循环数组
	int ordered_int[3];ordered_int[0] = 1;ordered_int[1] = 2;ordered_int[2] = 0;
	//Cell所组成的halffaces的vhs
	std::vector<std::vector<std::vector<OvmVeH>>> c_hf_vhs;c_hf_vhs.clear();
	//增加中间面，且同时构造后续需要增加的cell的所需要半面的vhs
	for (auto dc_it = deleted_chs.begin();dc_it != deleted_chs.end();dc_it++)
	{
		auto b_hfhs = mesh->cell(*dc_it).halffaces();//每一个被删除cell的边界面
		//需要增加的面
		std::vector<OvmVeH> new_f_vhs;new_f_vhs.clear();
		auto op_eh = tet_opposite_edge(mesh,*dc_it,eh);
		auto o_vs = mesh->edge(op_eh).from_vertex();auto o_ve = mesh->edge(op_eh).to_vertex();
		new_f_vhs.push_back(o_vs);
		new_f_vhs.push_back(o_ve);
		new_f_vhs.push_back(new_vh);
		can_add_f_vhs.push_back(new_f_vhs);
		std::vector<OvmVeH>().swap(new_f_vhs);
		int counter;//用于计数对边的两个端点有几个在某个半面上
		for (auto chf = b_hfhs.begin();chf != b_hfhs.end();chf++)
		{
			counter = 0;
			std::vector<OvmVeH> f_vhs;f_vhs.clear();
			for (auto vhf_it = mesh->hfv_iter(*chf);vhf_it;vhf_it++)
			{
				f_vhs.push_back(*vhf_it);
				if (*vhf_it == o_vs||*vhf_it == o_ve)
				{
					counter++;
				}
			}
			if (counter == 2)//如果这个面含有对边，则需要在这个面上添加新的cell
			{
				std::vector<std::vector<OvmVeH>> hf_vhs;hf_vhs.clear();
				std::vector<OvmVeH>  _vhs;
				hf_vhs.push_back(f_vhs);
				for (int or_ind = 0;or_ind < 3;or_ind++)
				{
					_vhs.clear();
					_vhs.push_back(new_vh);_vhs.push_back(f_vhs[ordered_int[or_ind]]);_vhs.push_back(f_vhs[or_ind]);
					hf_vhs.push_back(_vhs);
					std::vector<OvmVeH>().swap(_vhs);
				}
				c_hf_vhs.push_back(hf_vhs);std::vector<std::vector<OvmVeH>>().swap(hf_vhs);
			}
		}
	}
	//增加因为分裂一个面到两个面的vhs
	for (auto hehf_it = mesh->hehf_iter(mesh->halfedge_handle(eh,0));hehf_it;hehf_it++)
	{
		int added_v_idx;int counter_idx = 0;
		std::vector<OvmVeH> hf_vhs;hf_vhs.clear();
		for (auto hfv_it = mesh->hfv_iter(*hehf_it);hfv_it;hfv_it++)
		{
			hf_vhs.push_back(*hfv_it);
			if (*hfv_it != v_s&&*hfv_it != v_e)
			{
				added_v_idx = counter_idx;
			}
			counter_idx++;
		}
		OvmVeH hf_op_vh = hf_vhs[added_v_idx];
		std::vector<OvmVeH>().swap(hf_vhs);
		hf_vhs.push_back(v_s);hf_vhs.push_back(new_vh);hf_vhs.push_back(hf_op_vh);
		can_add_f_vhs.push_back(hf_vhs);std::vector<OvmVeH>().swap(hf_vhs);
		hf_vhs.push_back(v_e);hf_vhs.push_back(new_vh);hf_vhs.push_back(hf_op_vh);
		can_add_f_vhs.push_back(hf_vhs);std::vector<OvmVeH>().swap(hf_vhs);
	}
	//Step 3删除OvmCeH
	foreach (auto &ch,deleted_chs)
		status_attrib[ch].set_deleted(true);
	status_attrib.garbage_collection(true);
	//Step 4 增加点
	for (auto dv_it = candidate_point.begin();dv_it != candidate_point.end();dv_it++)
	{
		OvmVeH added_vh = mesh->add_vertex(dv_it->v_position);V_PREV_HANDLE[added_vh] = dv_it->vh;
	}
	std::hash_map<OvmVeH,OvmVeH> old2newvh_mapping;old2newvh_mapping.clear();
	for (auto v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++)
		//old2newvh_mapping.insert(std::make_pair(V_PREV_HANDLE[*v_it],*v_it));
		old2newvh_mapping[V_PREV_HANDLE[*v_it]] = *v_it;
	//增加面
	for (auto f_it = can_add_f_vhs.begin();f_it != can_add_f_vhs.end();f_it++)
	{
		std::vector<OvmVeH> f_vhs;
		foreach (auto vh_ind,*f_it) f_vhs.push_back(old2newvh_mapping[vh_ind]);
		OvmFaH new_fh = mesh->add_face(f_vhs);
		F_IF_COVERED[new_fh] = 1;
		std::vector<OvmVeH>().swap(f_vhs);
	}
	//增加体
	start_t2 = clock();
	std::vector<OvmHaFaH> covered_hfhs;covered_hfhs.clear();
	for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
	{
		if (F_IF_COVERED[*f_it] == 1)
		{
			covered_hfhs.push_back(mesh->halfface_handle(*f_it,0));
			covered_hfhs.push_back(mesh->halfface_handle(*f_it,1));
		}
	}
	for (auto c_it = c_hf_vhs.begin();c_it != c_hf_vhs.end();c_it++)
	{
		std::vector<OvmHaFaH> c_hfhs;c_hfhs.clear();
		for (auto hf_it = c_it->begin();hf_it != c_it->end();hf_it++)
		{
			//新建一个f vhs将旧的vh映射到新的vhs上
			std::vector<OvmVeH> new_fvhs;new_fvhs.clear();
			new_fvhs.push_back(old2newvh_mapping[(*hf_it)[0]]);
			new_fvhs.push_back(old2newvh_mapping[(*hf_it)[1]]);
			new_fvhs.push_back(old2newvh_mapping[(*hf_it)[2]]);
			bool if_ok;
			for (auto m_hf_it = covered_hfhs.begin();m_hf_it != covered_hfhs.end();m_hf_it++)
			//for (auto m_hf_it = mesh->halffaces_begin();m_hf_it != mesh->halffaces_end();m_hf_it++)
			{
				if_ok = false;
				std::vector<OvmVeH> m_hfv;m_hfv.clear();
				for (auto m_hfv_it = mesh->hfv_iter(*m_hf_it);m_hfv_it;m_hfv_it++)
				{
					m_hfv.push_back(*m_hfv_it);
				}
				int first_idx,counter_idx = 0;
				for (auto mhfv_it = m_hfv.begin();mhfv_it != m_hfv.end();mhfv_it++)
				{
					if (*mhfv_it == new_fvhs[0])
					{
						if_ok = true;
						first_idx = counter_idx;
						break;
					}
					counter_idx++;
				}
				//测试下
				if (if_ok&&new_fvhs[0] == 9&&new_fvhs[1] == 3&&new_fvhs[2] == 25)
				//if (if_ok&&if_DEBUG)
				{
					std::cout<<"IF ok "<<m_hfv[first_idx]<<"  "<<m_hfv[ordered_int[first_idx]]<<"  "<<m_hfv[ordered_int[ordered_int[first_idx]]]<<std::endl;
				}
				if (if_ok&&new_fvhs[1] == m_hfv[ordered_int[first_idx]]&&new_fvhs[2] == m_hfv[ordered_int[ordered_int[first_idx]]])
				{
					c_hfhs.push_back(*m_hf_it);
					break;
				}
				
				std::vector<OvmVeH>().swap(m_hfv);
			}
			std::vector<OvmVeH>().swap(new_fvhs);
		}
		OvmCeH added_cell = mesh->add_cell(c_hfhs);std::vector<OvmHaFaH>().swap(c_hfhs);
		_chs.push_back(added_cell);
	}
	end_t1 = clock();
	end_t2 = end_t1;
	double tt1 = (double) (end_t1-start_t1);
	double tt2 = (double) (end_t2-start_t2);
	//std::cout<<"Percentage: "<<tt2/tt1<<std::endl;
}

void tet_edge_splitting_with_mapping(VMesh* mesh,OvmEgH eh,std::hash_map<OvmCeH, OvmCeH>& new2oldchmapping)
{
	std::clock_t start_t1,end_t1,start_t2,end_t2;
	start_t1 = clock();
	struct New_Point{
		OvmVeH vh;
		OvmVec3d v_position;
	};
	//Step 0 将所有原VertexHandle作为属性存储起来
	//Step 1 遍历所有与eh相邻的四面体单元，判断与之相邻的所有节点在删除掉所有该删的四面体时候是否也会被删除
	//       判断依据是这个节点相邻的四面体是否是只有这些要删除的四面体，若会被删除则构建一个New_Point存储Vh和几何点位置
	//Step 2 构建需要新建面的Vertex Handle值，以及构造新的两类面所需要的三个VH值的vector，同时需要增加因为悬空点被删而被删的面
	//Step 3 删除OvmCeH
	//Step 4 增加点，包括边中心点和悬空点,并构建old2new_vh_mapping
	//Step 5 增加面,增加体
	OvmVeH v_s = mesh->edge(eh).from_vertex();OvmVeH v_e = mesh->edge(eh).to_vertex();
	OpenVolumeMesh::StatusAttrib status_attrib(*mesh);
	//Step 0
	std::hash_map<OvmCeH, OvmCeH>().swap(new2oldchmapping);
	auto V_PREV_HANDLE = mesh->request_vertex_property<OvmVeH>("prevhandle");
	for (auto v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++)
		V_PREV_HANDLE[*v_it] = *v_it;
	//用于判定这个面是否在接下来中要用到
	auto F_IF_COVERED = mesh->request_face_property<int>("coveredfacehandle",0);
	//将所有cell赋值一个属性，到原来的OvmCeH的映射
	auto C_PRE_CHandle = mesh->request_cell_property<OvmCeH>("previous_cellhandle");
	for (auto c_it = mesh->cells_begin();c_it != mesh->cells_end();c_it++)
	{
		C_PRE_CHandle[*c_it] = *c_it;
	}
	//Step 1
	std::unordered_set<OvmCeH> deleted_chs;deleted_chs.clear();//所有待删除的cell handle
	std::unordered_set<OvmVeH> adj_deleted_vhs;adj_deleted_vhs.clear();//所有删除四面体单元之后悬空的点
	std::vector<std::vector<OvmVeH>> can_add_f_vhs;can_add_f_vhs.clear();//所有后续需要添加的面（vertices表示），包括多删的面，和后需要加的面
	for (auto hec_it = mesh->hec_iter(mesh->halfedge_handle(eh,0));hec_it;hec_it++)
	{
		if (*hec_it == mesh->InvalidCellHandle)
		{
			continue;
		}
		deleted_chs.insert(*hec_it);
	}
	//遍历所有与要删Cell相邻的VH,若这个VH只与要被删的Cells相邻则该VH也将被删除
	for (auto dc_it = deleted_chs.begin();dc_it != deleted_chs.end();dc_it++)
	{
		bool vh_deleted;
		for (auto cv_it = mesh->cv_iter(*dc_it);cv_it;cv_it++)
		{
			vh_deleted = true;
			for (auto vc_it = mesh->vc_iter(*cv_it);vc_it;vc_it++)
			{
				if (deleted_chs.find(*vc_it) == deleted_chs.end())
				{
					vh_deleted = false;
					break;
				}
			}
			if (vh_deleted == true)
			{
				adj_deleted_vhs.insert(*cv_it);
			}
		}
	}
	//构建在删除Cell后需要添加的Vertex的信息,所有这些信息集中在candidate_point中
	std::vector<New_Point> candidate_point;candidate_point.clear();
	OvmVec3d mid_position = (mesh->vertex(mesh->edge(eh).from_vertex())+mesh->vertex(mesh->edge(eh).to_vertex()))/2.0;
	OvmVeH new_vh = (OvmVeH) mesh->n_vertices();//这里新的节点可能有点问题
	New_Point mid_p;mid_p.v_position = mid_position;mid_p.vh = new_vh;
	candidate_point.push_back(mid_p);
	for (auto dv_it = adj_deleted_vhs.begin();dv_it != adj_deleted_vhs.end();dv_it++)
	{
		New_Point deleted_p;
		deleted_p.v_position = mesh->vertex(*dv_it);deleted_p.vh = *dv_it;
		candidate_point.push_back(deleted_p);
	}
	//遍历所有被删除的Cell，查看与被删的Cell的所有面是否有可能被删，这里要排除掉包含分裂边edgehandle的面，这是在之后处理用
	//若被删除则记录这个Face 的三个Vertex Handle，为避免多加Face,添加deleted_fhs;
	std::unordered_set<OvmFaH> deleted_fhs;
	for (auto dc_it = deleted_chs.begin();dc_it != deleted_chs.end();dc_it++)
	{
		//////////////////////////////////////////////////////////////////////////
		bool fh_deleted;
		auto adj_hfhs = mesh->cell(*dc_it).halffaces();
		for (auto can_hfh = adj_hfhs.begin();can_hfh != adj_hfhs.end();can_hfh++)
		{
			F_IF_COVERED[mesh->face_handle(*can_hfh)] = 1;
			fh_deleted = false;
			std::vector<OvmVeH> f_vhs;f_vhs.clear();
			auto hehs = mesh->face(mesh->face_handle(*can_hfh)).halfedges();
			std::vector<OvmEgH> _ehs;
			foreach(OvmHaEgH heh_ind,hehs)
			{
				_ehs.push_back(mesh->edge_handle(heh_ind));
			}
			std::vector<OvmHaEgH>().swap(hehs);
			//要删掉处于边界上或者这个面的两侧cell都被删除了，且这个半面不包含输入的Edge Handle
			if ((mesh->is_boundary(mesh->opposite_halfface_handle(*can_hfh))||contains(deleted_chs,mesh->incident_cell(mesh->opposite_halfface_handle(*can_hfh))))&&(!contains(_ehs,eh)))
			{
				fh_deleted = true;
			}
			//或者找到构成这个面的三个vertex中是否存在被删掉的vertex，若有，则同样需要被删除
			for (auto hfv_it = mesh->hfv_iter(*can_hfh);hfv_it;hfv_it++)
			{
				f_vhs.push_back(*hfv_it);
				if (adj_deleted_vhs.find(*hfv_it) != adj_deleted_vhs.end())
				{
					fh_deleted = true;
				}
			}

			if (fh_deleted&&(deleted_fhs.find(mesh->face_handle(*can_hfh)) == deleted_fhs.end()))
			{
				deleted_fhs.insert(mesh->face_handle(*can_hfh));
				can_add_f_vhs.push_back(f_vhs);
			}
			std::vector<OvmEgH>().swap(_ehs);
			std::vector<OvmVeH>().swap(f_vhs);
		}
	}
	std::unordered_set<OvmFaH>().swap(deleted_fhs);
	//Step 2
	//循环数组
	int ordered_int[3];ordered_int[0] = 1;ordered_int[1] = 2;ordered_int[2] = 0;
	//Cell所组成的halffaces的vhs
	std::vector<std::vector<std::vector<OvmVeH>>> c_hf_vhs;c_hf_vhs.clear();
	//上述cell所组成的vhs与输入mesh对应的原来OvmCeh之间对应
	std::vector<OvmCeH> old_chs;std::vector<OvmCeH>().swap(old_chs);
	//增加中间面，且同时构造后续需要增加的cell的所需要半面的vhs
	for (auto dc_it = deleted_chs.begin();dc_it != deleted_chs.end();dc_it++)
	{
		auto b_hfhs = mesh->cell(*dc_it).halffaces();//每一个被删除cell的边界面
		//需要增加的面
		std::vector<OvmVeH> new_f_vhs;new_f_vhs.clear();
		auto op_eh = tet_opposite_edge(mesh,*dc_it,eh);
		auto o_vs = mesh->edge(op_eh).from_vertex();auto o_ve = mesh->edge(op_eh).to_vertex();
		new_f_vhs.push_back(o_vs);
		new_f_vhs.push_back(o_ve);
		new_f_vhs.push_back(new_vh);
		can_add_f_vhs.push_back(new_f_vhs);
		std::vector<OvmVeH>().swap(new_f_vhs);
		int counter;//用于计数对边的两个端点有几个在某个半面上
		for (auto chf = b_hfhs.begin();chf != b_hfhs.end();chf++)
		{
			counter = 0;
			std::vector<OvmVeH> f_vhs;f_vhs.clear();
			for (auto vhf_it = mesh->hfv_iter(*chf);vhf_it;vhf_it++)
			{
				f_vhs.push_back(*vhf_it);
				if (*vhf_it == o_vs||*vhf_it == o_ve)
				{
					counter++;
				}
			}
			if (counter == 2)//如果这个面含有对边，则需要在这个面上添加新的cell
			{
				std::vector<std::vector<OvmVeH>> hf_vhs;hf_vhs.clear();
				std::vector<OvmVeH>  _vhs;
				hf_vhs.push_back(f_vhs);
				for (int or_ind = 0;or_ind < 3;or_ind++)
				{
					_vhs.clear();
					_vhs.push_back(new_vh);_vhs.push_back(f_vhs[ordered_int[or_ind]]);_vhs.push_back(f_vhs[or_ind]);
					hf_vhs.push_back(_vhs);
					std::vector<OvmVeH>().swap(_vhs);
				}
				c_hf_vhs.push_back(hf_vhs);std::vector<std::vector<OvmVeH>>().swap(hf_vhs);
				old_chs.push_back(*dc_it);
			}
		}
	}
	//增加因为分裂一个面到两个面的vhs
	for (auto hehf_it = mesh->hehf_iter(mesh->halfedge_handle(eh,0));hehf_it;hehf_it++)
	{
		int added_v_idx;int counter_idx = 0;
		std::vector<OvmVeH> hf_vhs;hf_vhs.clear();
		for (auto hfv_it = mesh->hfv_iter(*hehf_it);hfv_it;hfv_it++)
		{
			hf_vhs.push_back(*hfv_it);
			if (*hfv_it != v_s&&*hfv_it != v_e)
			{
				added_v_idx = counter_idx;
			}
			counter_idx++;
		}
		OvmVeH hf_op_vh = hf_vhs[added_v_idx];
		std::vector<OvmVeH>().swap(hf_vhs);
		hf_vhs.push_back(v_s);hf_vhs.push_back(new_vh);hf_vhs.push_back(hf_op_vh);
		can_add_f_vhs.push_back(hf_vhs);std::vector<OvmVeH>().swap(hf_vhs);
		hf_vhs.push_back(v_e);hf_vhs.push_back(new_vh);hf_vhs.push_back(hf_op_vh);
		can_add_f_vhs.push_back(hf_vhs);std::vector<OvmVeH>().swap(hf_vhs);
	}
	//Step 3删除OvmCeH
	foreach (auto &ch,deleted_chs)
		status_attrib[ch].set_deleted(true);
	status_attrib.garbage_collection(true);
	//Step 4 增加点
	for (auto dv_it = candidate_point.begin();dv_it != candidate_point.end();dv_it++)
	{
		OvmVeH added_vh = mesh->add_vertex(dv_it->v_position);V_PREV_HANDLE[added_vh] = dv_it->vh;
	}
	std::hash_map<OvmVeH,OvmVeH> old2newvh_mapping;old2newvh_mapping.clear();
	for (auto v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++)
		//old2newvh_mapping.insert(std::make_pair(V_PREV_HANDLE[*v_it],*v_it));
		old2newvh_mapping[V_PREV_HANDLE[*v_it]] = *v_it;
	//增加面
	for (auto f_it = can_add_f_vhs.begin();f_it != can_add_f_vhs.end();f_it++)
	{
		std::vector<OvmVeH> f_vhs;
		foreach (auto vh_ind,*f_it) f_vhs.push_back(old2newvh_mapping[vh_ind]);
		OvmFaH new_fh = mesh->add_face(f_vhs);
		F_IF_COVERED[new_fh] = 1;
		std::vector<OvmVeH>().swap(f_vhs);
	}
	//增加体
	start_t2 = clock();auto _old_ch_it = old_chs.begin();
	std::vector<OvmHaFaH> covered_hfhs;covered_hfhs.clear();
	for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
	{
		if (F_IF_COVERED[*f_it] == 1)
		{
			covered_hfhs.push_back(mesh->halfface_handle(*f_it,0));
			covered_hfhs.push_back(mesh->halfface_handle(*f_it,1));
		}
	}
	for (auto c_it = c_hf_vhs.begin();c_it != c_hf_vhs.end();c_it++,_old_ch_it++)
	{
		std::vector<OvmHaFaH> c_hfhs;c_hfhs.clear();
		for (auto hf_it = c_it->begin();hf_it != c_it->end();hf_it++)
		{
			//新建一个f vhs将旧的vh映射到新的vhs上
			std::vector<OvmVeH> new_fvhs;new_fvhs.clear();
			new_fvhs.push_back(old2newvh_mapping[(*hf_it)[0]]);
			new_fvhs.push_back(old2newvh_mapping[(*hf_it)[1]]);
			new_fvhs.push_back(old2newvh_mapping[(*hf_it)[2]]);
			bool if_ok;
			for (auto m_hf_it = covered_hfhs.begin();m_hf_it != covered_hfhs.end();m_hf_it++)
				//for (auto m_hf_it = mesh->halffaces_begin();m_hf_it != mesh->halffaces_end();m_hf_it++)
			{
				if_ok = false;
				std::vector<OvmVeH> m_hfv;m_hfv.clear();
				for (auto m_hfv_it = mesh->hfv_iter(*m_hf_it);m_hfv_it;m_hfv_it++)
				{
					m_hfv.push_back(*m_hfv_it);
				}
				int first_idx,counter_idx = 0;
				for (auto mhfv_it = m_hfv.begin();mhfv_it != m_hfv.end();mhfv_it++)
				{
					if (*mhfv_it == new_fvhs[0])
					{
						if_ok = true;
						first_idx = counter_idx;
						break;
					}
					counter_idx++;
				}
				//测试下
				if (if_ok&&new_fvhs[0] == 9&&new_fvhs[1] == 3&&new_fvhs[2] == 25)
					//if (if_ok&&if_DEBUG)
				{
					std::cout<<"IF ok "<<m_hfv[first_idx]<<"  "<<m_hfv[ordered_int[first_idx]]<<"  "<<m_hfv[ordered_int[ordered_int[first_idx]]]<<std::endl;
				}
				if (if_ok&&new_fvhs[1] == m_hfv[ordered_int[first_idx]]&&new_fvhs[2] == m_hfv[ordered_int[ordered_int[first_idx]]])
				{
					c_hfhs.push_back(*m_hf_it);
					break;
				}

				std::vector<OvmVeH>().swap(m_hfv);
			}
			std::vector<OvmVeH>().swap(new_fvhs);
		}
		OvmCeH added_cell = mesh->add_cell(c_hfhs);std::vector<OvmHaFaH>().swap(c_hfhs);
		C_PRE_CHandle[added_cell] = *_old_ch_it;
	}
	for (auto c_it = mesh->cells_begin();c_it != mesh->cells_end();c_it++)
	{
		new2oldchmapping[*c_it] = C_PRE_CHandle[*c_it];
	}
	end_t1 = clock();
	end_t2 = end_t1;
	double tt1 = (double) (end_t1-start_t1);
	double tt2 = (double) (end_t2-start_t2);
	//std::cout<<"Percentage: "<<tt2/tt1<<std::endl;
}

OvmVeH tet_edge_splitting_with_fix_point_and_mapping(VMesh* mesh, OvmEgH eh, OvmVec3d p_pos, std::hash_map<OvmCeH, OvmCeH>& new2oldchmapping)

{
	std::clock_t start_t1,end_t1,start_t2,end_t2;
	start_t1 = clock();
	struct New_Point{
		OvmVeH vh;
		OvmVec3d v_position;
	};
	//Step 0 将所有原VertexHandle作为属性存储起来
	//Step 1 遍历所有与eh相邻的四面体单元，判断与之相邻的所有节点在删除掉所有该删的四面体时候是否也会被删除
	//       判断依据是这个节点相邻的四面体是否是只有这些要删除的四面体，若会被删除则构建一个New_Point存储Vh和几何点位置
	//Step 2 构建需要新建面的Vertex Handle值，以及构造新的两类面所需要的三个VH值的vector，同时需要增加因为悬空点被删而被删的面
	//Step 3 删除OvmCeH
	//Step 4 增加点，包括边中心点和悬空点,并构建old2new_vh_mapping
	//Step 5 增加面,增加体
	OvmVeH v_s = mesh->edge(eh).from_vertex();OvmVeH v_e = mesh->edge(eh).to_vertex();
	OpenVolumeMesh::StatusAttrib status_attrib(*mesh);
	//Step 0
	std::hash_map<OvmCeH, OvmCeH>().swap(new2oldchmapping);
	auto V_PREV_HANDLE = mesh->request_vertex_property<OvmVeH>("prevhandle");
	for (auto v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++)
		V_PREV_HANDLE[*v_it] = *v_it;
	//用于判定这个面是否在接下来中要用到
	auto F_IF_COVERED = mesh->request_face_property<int>("coveredfacehandle",0);
	//将所有cell赋值一个属性，到原来的OvmCeH的映射
	auto C_PRE_CHandle = mesh->request_cell_property<OvmCeH>("previous_cellhandle");
	for (auto c_it = mesh->cells_begin();c_it != mesh->cells_end();c_it++)
	{
		C_PRE_CHandle[*c_it] = *c_it;
	}
	//Step 1
	std::unordered_set<OvmCeH> deleted_chs;deleted_chs.clear();//所有待删除的cell handle
	std::unordered_set<OvmVeH> adj_deleted_vhs;adj_deleted_vhs.clear();//所有删除四面体单元之后悬空的点
	std::vector<std::vector<OvmVeH>> can_add_f_vhs;can_add_f_vhs.clear();//所有后续需要添加的面（vertices表示），包括多删的面，和后需要加的面
	for (auto hec_it = mesh->hec_iter(mesh->halfedge_handle(eh,0));hec_it;hec_it++)
	{
		if (*hec_it == mesh->InvalidCellHandle)
		{
			continue;
		}
		deleted_chs.insert(*hec_it);
	}
	//遍历所有与要删Cell相邻的VH,若这个VH只与要被删的Cells相邻则该VH也将被删除
	for (auto dc_it = deleted_chs.begin();dc_it != deleted_chs.end();dc_it++)
	{
		bool vh_deleted;
		for (auto cv_it = mesh->cv_iter(*dc_it);cv_it;cv_it++)
		{
			vh_deleted = true;
			for (auto vc_it = mesh->vc_iter(*cv_it);vc_it;vc_it++)
			{
				if (deleted_chs.find(*vc_it) == deleted_chs.end())
				{
					vh_deleted = false;
					break;
				}
			}
			if (vh_deleted == true)
			{
				adj_deleted_vhs.insert(*cv_it);
			}
		}
	}
	//构建在删除Cell后需要添加的Vertex的信息,所有这些信息集中在candidate_point中
	std::vector<New_Point> candidate_point;candidate_point.clear();
	OvmVec3d mid_position = p_pos;
	OvmVeH new_vh = (OvmVeH) mesh->n_vertices();//这里新的节点可能有点问题
	New_Point mid_p;mid_p.v_position = mid_position;mid_p.vh = new_vh;
	candidate_point.push_back(mid_p);
	for (auto dv_it = adj_deleted_vhs.begin();dv_it != adj_deleted_vhs.end();dv_it++)
	{
		New_Point deleted_p;
		deleted_p.v_position = mesh->vertex(*dv_it);deleted_p.vh = *dv_it;
		candidate_point.push_back(deleted_p);
	}
	//遍历所有被删除的Cell，查看与被删的Cell的所有面是否有可能被删，这里要排除掉包含分裂边edgehandle的面，这是在之后处理用
	//若被删除则记录这个Face 的三个Vertex Handle，为避免多加Face,添加deleted_fhs;
	std::unordered_set<OvmFaH> deleted_fhs;
	for (auto dc_it = deleted_chs.begin();dc_it != deleted_chs.end();dc_it++)
	{
		//////////////////////////////////////////////////////////////////////////
		bool fh_deleted;
		auto adj_hfhs = mesh->cell(*dc_it).halffaces();
		for (auto can_hfh = adj_hfhs.begin();can_hfh != adj_hfhs.end();can_hfh++)
		{
			F_IF_COVERED[mesh->face_handle(*can_hfh)] = 1;
			fh_deleted = false;
			std::vector<OvmVeH> f_vhs;f_vhs.clear();
			auto hehs = mesh->face(mesh->face_handle(*can_hfh)).halfedges();
			std::vector<OvmEgH> _ehs;
			foreach(OvmHaEgH heh_ind,hehs)
			{
				_ehs.push_back(mesh->edge_handle(heh_ind));
			}
			std::vector<OvmHaEgH>().swap(hehs);
			//要删掉处于边界上或者这个面的两侧cell都被删除了，且这个半面不包含输入的Edge Handle
			if ((mesh->is_boundary(mesh->opposite_halfface_handle(*can_hfh))||contains(deleted_chs,mesh->incident_cell(mesh->opposite_halfface_handle(*can_hfh))))&&(!contains(_ehs,eh)))
			{
				fh_deleted = true;
			}
			//或者找到构成这个面的三个vertex中是否存在被删掉的vertex，若有，则同样需要被删除
			for (auto hfv_it = mesh->hfv_iter(*can_hfh);hfv_it;hfv_it++)
			{
				f_vhs.push_back(*hfv_it);
				if (adj_deleted_vhs.find(*hfv_it) != adj_deleted_vhs.end())
				{
					fh_deleted = true;
				}
			}

			if (fh_deleted&&(deleted_fhs.find(mesh->face_handle(*can_hfh)) == deleted_fhs.end()))
			{
				deleted_fhs.insert(mesh->face_handle(*can_hfh));
				can_add_f_vhs.push_back(f_vhs);
			}
			std::vector<OvmEgH>().swap(_ehs);
			std::vector<OvmVeH>().swap(f_vhs);
		}
	}
	std::unordered_set<OvmFaH>().swap(deleted_fhs);
	//Step 2
	//循环数组
	int ordered_int[3];ordered_int[0] = 1;ordered_int[1] = 2;ordered_int[2] = 0;
	//Cell所组成的halffaces的vhs
	std::vector<std::vector<std::vector<OvmVeH>>> c_hf_vhs;c_hf_vhs.clear();
	//上述cell所组成的vhs与输入mesh对应的原来OvmCeh之间对应
	std::vector<OvmCeH> old_chs;std::vector<OvmCeH>().swap(old_chs);
	//增加中间面，且同时构造后续需要增加的cell的所需要半面的vhs
	for (auto dc_it = deleted_chs.begin();dc_it != deleted_chs.end();dc_it++)
	{
		auto b_hfhs = mesh->cell(*dc_it).halffaces();//每一个被删除cell的边界面
		//需要增加的面
		std::vector<OvmVeH> new_f_vhs;new_f_vhs.clear();
		auto op_eh = tet_opposite_edge(mesh,*dc_it,eh);
		auto o_vs = mesh->edge(op_eh).from_vertex();auto o_ve = mesh->edge(op_eh).to_vertex();
		new_f_vhs.push_back(o_vs);
		new_f_vhs.push_back(o_ve);
		new_f_vhs.push_back(new_vh);
		can_add_f_vhs.push_back(new_f_vhs);
		std::vector<OvmVeH>().swap(new_f_vhs);
		int counter;//用于计数对边的两个端点有几个在某个半面上
		for (auto chf = b_hfhs.begin();chf != b_hfhs.end();chf++)
		{
			counter = 0;
			std::vector<OvmVeH> f_vhs;f_vhs.clear();
			for (auto vhf_it = mesh->hfv_iter(*chf);vhf_it;vhf_it++)
			{
				f_vhs.push_back(*vhf_it);
				if (*vhf_it == o_vs||*vhf_it == o_ve)
				{
					counter++;
				}
			}
			if (counter == 2)//如果这个面含有对边，则需要在这个面上添加新的cell
			{
				std::vector<std::vector<OvmVeH>> hf_vhs;hf_vhs.clear();
				std::vector<OvmVeH>  _vhs;
				hf_vhs.push_back(f_vhs);
				for (int or_ind = 0;or_ind < 3;or_ind++)
				{
					_vhs.clear();
					_vhs.push_back(new_vh);_vhs.push_back(f_vhs[ordered_int[or_ind]]);_vhs.push_back(f_vhs[or_ind]);
					hf_vhs.push_back(_vhs);
					std::vector<OvmVeH>().swap(_vhs);
				}
				c_hf_vhs.push_back(hf_vhs);std::vector<std::vector<OvmVeH>>().swap(hf_vhs);
				old_chs.push_back(*dc_it);
			}
		}
	}
	//增加因为分裂一个面到两个面的vhs
	for (auto hehf_it = mesh->hehf_iter(mesh->halfedge_handle(eh,0));hehf_it;hehf_it++)
	{
		int added_v_idx;int counter_idx = 0;
		std::vector<OvmVeH> hf_vhs;hf_vhs.clear();
		for (auto hfv_it = mesh->hfv_iter(*hehf_it);hfv_it;hfv_it++)
		{
			hf_vhs.push_back(*hfv_it);
			if (*hfv_it != v_s&&*hfv_it != v_e)
			{
				added_v_idx = counter_idx;
			}
			counter_idx++;
		}
		OvmVeH hf_op_vh = hf_vhs[added_v_idx];
		std::vector<OvmVeH>().swap(hf_vhs);
		hf_vhs.push_back(v_s);hf_vhs.push_back(new_vh);hf_vhs.push_back(hf_op_vh);
		can_add_f_vhs.push_back(hf_vhs);std::vector<OvmVeH>().swap(hf_vhs);
		hf_vhs.push_back(v_e);hf_vhs.push_back(new_vh);hf_vhs.push_back(hf_op_vh);
		can_add_f_vhs.push_back(hf_vhs);std::vector<OvmVeH>().swap(hf_vhs);
	}
	//Step 3删除OvmCeH
	foreach (auto &ch,deleted_chs)
		status_attrib[ch].set_deleted(true);
	status_attrib.garbage_collection(true);
	//Step 4 增加点
	for (auto dv_it = candidate_point.begin();dv_it != candidate_point.end();dv_it++)
	{
		OvmVeH added_vh = mesh->add_vertex(dv_it->v_position);V_PREV_HANDLE[added_vh] = dv_it->vh;
	}
	std::hash_map<OvmVeH,OvmVeH> old2newvh_mapping;old2newvh_mapping.clear();
	for (auto v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++)
		//old2newvh_mapping.insert(std::make_pair(V_PREV_HANDLE[*v_it],*v_it));
		old2newvh_mapping[V_PREV_HANDLE[*v_it]] = *v_it;
	//增加面
	for (auto f_it = can_add_f_vhs.begin();f_it != can_add_f_vhs.end();f_it++)
	{
		std::vector<OvmVeH> f_vhs;
		foreach (auto vh_ind,*f_it) f_vhs.push_back(old2newvh_mapping[vh_ind]);
		OvmFaH new_fh = mesh->add_face(f_vhs);
		F_IF_COVERED[new_fh] = 1;
		std::vector<OvmVeH>().swap(f_vhs);
	}
	//增加体
	start_t2 = clock();auto _old_ch_it = old_chs.begin();
	std::vector<OvmHaFaH> covered_hfhs;covered_hfhs.clear();
	for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
	{
		if (F_IF_COVERED[*f_it] == 1)
		{
			covered_hfhs.push_back(mesh->halfface_handle(*f_it,0));
			covered_hfhs.push_back(mesh->halfface_handle(*f_it,1));
		}
	}
	for (auto c_it = c_hf_vhs.begin();c_it != c_hf_vhs.end();c_it++,_old_ch_it++)
	{
		std::vector<OvmHaFaH> c_hfhs;c_hfhs.clear();
		for (auto hf_it = c_it->begin();hf_it != c_it->end();hf_it++)
		{
			//新建一个f vhs将旧的vh映射到新的vhs上
			std::vector<OvmVeH> new_fvhs;new_fvhs.clear();
			new_fvhs.push_back(old2newvh_mapping[(*hf_it)[0]]);
			new_fvhs.push_back(old2newvh_mapping[(*hf_it)[1]]);
			new_fvhs.push_back(old2newvh_mapping[(*hf_it)[2]]);
			bool if_ok;
			for (auto m_hf_it = covered_hfhs.begin();m_hf_it != covered_hfhs.end();m_hf_it++)
				//for (auto m_hf_it = mesh->halffaces_begin();m_hf_it != mesh->halffaces_end();m_hf_it++)
			{
				if_ok = false;
				std::vector<OvmVeH> m_hfv;m_hfv.clear();
				for (auto m_hfv_it = mesh->hfv_iter(*m_hf_it);m_hfv_it;m_hfv_it++)
				{
					m_hfv.push_back(*m_hfv_it);
				}
				int first_idx,counter_idx = 0;
				for (auto mhfv_it = m_hfv.begin();mhfv_it != m_hfv.end();mhfv_it++)
				{
					if (*mhfv_it == new_fvhs[0])
					{
						if_ok = true;
						first_idx = counter_idx;
						break;
					}
					counter_idx++;
				}
				//测试下
				if (if_ok&&new_fvhs[0] == 9&&new_fvhs[1] == 3&&new_fvhs[2] == 25)
					//if (if_ok&&if_DEBUG)
				{
					std::cout<<"IF ok "<<m_hfv[first_idx]<<"  "<<m_hfv[ordered_int[first_idx]]<<"  "<<m_hfv[ordered_int[ordered_int[first_idx]]]<<std::endl;
				}
				if (if_ok&&new_fvhs[1] == m_hfv[ordered_int[first_idx]]&&new_fvhs[2] == m_hfv[ordered_int[ordered_int[first_idx]]])
				{
					c_hfhs.push_back(*m_hf_it);
					break;
				}

				std::vector<OvmVeH>().swap(m_hfv);
			}
			std::vector<OvmVeH>().swap(new_fvhs);
		}
		OvmCeH added_cell = mesh->add_cell(c_hfhs);std::vector<OvmHaFaH>().swap(c_hfhs);
		C_PRE_CHandle[added_cell] = *_old_ch_it;
	}
	for (auto c_it = mesh->cells_begin();c_it != mesh->cells_end();c_it++)
	{
		new2oldchmapping[*c_it] = C_PRE_CHandle[*c_it];
	}
	end_t1 = clock();
	end_t2 = end_t1;
	double tt1 = (double) (end_t1-start_t1);
	double tt2 = (double) (end_t2-start_t2);
	//std::cout<<"Percentage: "<<tt2/tt1<<std::endl;
	return new_vh;
}

void tet_face_splitting(VMesh* mesh,OvmFaH fh,std::vector<OvmCeH>& _chs)
{
	struct New_Point{
		OvmVeH vh;
		OvmVec3d v_position;
	};
	//说明，这里有可能是边界面，处理方式可能会不同
	//Step 0 将所有原VertexHandle作为属性存储起来,并做一些初始化
	//Step 1 遍历所有与eh相邻的四面体单元，判断与之相邻的所有节点在删除掉所有该删的四面体时候是否也会被删除
	//       判断依据是这个节点相邻的四面体是否是只有这些要删除的四面体，若会被删除则构建一个New_Point存储Vh和几何点位置
	//Step 2 构建需要新建面的Vertex Handle值，以及构造新的两类面所需要的三个VH值的vector，同时需要增加因为悬空点被删而被删的面
	//Step 3 删除OvmCeH
	//Step 4 增加点，包括边中心点和悬空点,并构建old2new_vh_mapping
	//Step 5 增加面,增加体
	OpenVolumeMesh::StatusAttrib status_attrib(*mesh);
	//Step 0
	auto V_PREV_HANDLE = mesh->request_vertex_property<OvmVeH>("prevhandle");
	for (auto v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++)
		V_PREV_HANDLE[*v_it] = *v_it;
	std::vector<OvmVeH> t_f_vhs;
	for (auto hfv_it = mesh->hfv_iter(mesh->halfface_handle(fh,0));hfv_it;hfv_it++)
	{
		t_f_vhs.push_back(*hfv_it);
	}
	auto F_IF_COVERED = mesh->request_face_property<int>("coveredfacehandle",0);
	//Step 1
	std::unordered_set<OvmCeH> deleted_chs;deleted_chs.clear();//所有待删除的cell handle
	std::unordered_set<OvmVeH> adj_deleted_vhs;adj_deleted_vhs.clear();//所有删除四面体单元之后悬空的点
	std::vector<std::vector<OvmVeH>> can_add_f_vhs;can_add_f_vhs.clear();//所有将因为删除悬挂点，而被删除的面,并加上需要在后续添加面的面片节点
	
	if (mesh->is_boundary(fh))
	{
		auto dch = mesh->is_boundary(mesh->halfface_handle(fh,1)) ?  mesh->incident_cell(mesh->halfface_handle(fh,0)) : mesh->incident_cell(mesh->halfface_handle(fh,1));
		deleted_chs.insert(dch);
	} 
	else
	{
		auto dch0 = mesh->incident_cell(mesh->halfface_handle(fh,0));
		auto dch1 = mesh->incident_cell(mesh->halfface_handle(fh,1));
		deleted_chs.insert(dch0);deleted_chs.insert(dch1);
	}
	
	//遍历所有与要删Cell相邻的VH,若这个VH只与要被删的Cells相邻则该VH也将被删除
	for (auto dc_it = deleted_chs.begin();dc_it != deleted_chs.end();dc_it++)
	{
		bool vh_deleted;
		for (auto cv_it = mesh->cv_iter(*dc_it);cv_it;cv_it++)
		{
			vh_deleted = true;
			for (auto vc_it = mesh->vc_iter(*cv_it);vc_it;vc_it++)
			{
				if (deleted_chs.find(*vc_it) == deleted_chs.end())
				{
					vh_deleted = false;
					break;
				}
			}
			if (vh_deleted == true)
			{
				adj_deleted_vhs.insert(*cv_it);
			}
		}
	}
	//构建在删除Cell后需要添加的Vertex的信息,所有这些信息集中在candidate_point中
	std::vector<New_Point> candidate_point;candidate_point.clear();
	OvmVec3d mid_position = (mesh->vertex(t_f_vhs[0])+mesh->vertex(t_f_vhs[1])+mesh->vertex(t_f_vhs[2]))/3.0;
	OvmVeH new_vh = (OvmVeH) mesh->n_vertices();
	New_Point mid_p;mid_p.v_position = mid_position;mid_p.vh = new_vh;
	candidate_point.push_back(mid_p);
	for (auto dv_it = adj_deleted_vhs.begin();dv_it != adj_deleted_vhs.end();dv_it++)
	{
		New_Point deleted_p;
		deleted_p.v_position = mesh->vertex(*dv_it);deleted_p.vh = *dv_it;
		candidate_point.push_back(deleted_p);
	}
	//遍历所有被删除的Cell，查看与被删的Cell的所有面是否有可能被删，若是被删（这个面上存在被删的vertex handle）
	//若被删除则记录这个Face 的三个Vertex Handle,除了这个面中有一个点被删除，还有一种可能是这个面仅与这个被删的cell相邻
	for (auto dc_it = deleted_chs.begin();dc_it != deleted_chs.end();dc_it++)
	{
		bool fh_deleted;
		auto adj_hfhs = mesh->cell(*dc_it).halffaces();
		for (auto can_hfh = adj_hfhs.begin();can_hfh != adj_hfhs.end();can_hfh++)
		{
			F_IF_COVERED[mesh->face_handle(*can_hfh)] = 1;
			fh_deleted = false;
			std::vector<OvmVeH> f_vhs;f_vhs.clear();
			//对于这个面属于边界，或者两个属于同一个面上的halfface都是属于将要被删除的cell中，则这个面将被删除
			if ((mesh->is_boundary(mesh->opposite_halfface_handle(*can_hfh))&&deleted_chs.size() == 2)||(contains(deleted_chs,mesh->incident_cell(mesh->opposite_halfface_handle(*can_hfh)))&&mesh->face_handle(*can_hfh) != fh))
			{
				fh_deleted = true;
			}
			//若这个面中有一个点属于被删除点集的，则将被删除
			for (auto hfv_it = mesh->hfv_iter(*can_hfh);hfv_it;hfv_it++)
			{
				f_vhs.push_back(*hfv_it);
				if (adj_deleted_vhs.find(*hfv_it) != adj_deleted_vhs.end())
				{
					fh_deleted = true;
				}
			}
			if (fh_deleted)
			{
				can_add_f_vhs.push_back(f_vhs);
			}
			std::vector<OvmVeH>().swap(f_vhs);
		}
	}
	//Step 2
	//循环数组
	int ordered_int[3];ordered_int[0] = 1;ordered_int[1] = 2;ordered_int[2] = 0;
	//Cell所组成的halffaces的vhs
	std::vector<std::vector<std::vector<OvmVeH>>> c_hf_vhs;c_hf_vhs.clear();
	//增加将fh分裂为三个面的vhs
	for (int f_id = 0;f_id < 3;f_id++)
	{
		std::vector<OvmVeH> add_f_vhs;
		add_f_vhs.push_back(new_vh);
		add_f_vhs.push_back(t_f_vhs[f_id]);
		add_f_vhs.push_back(t_f_vhs[ordered_int[f_id]]);
		can_add_f_vhs.push_back(add_f_vhs);std::vector<OvmVeH>().swap(add_f_vhs);
	}
	//增加cell内部的can_add_f_vhs，同时增加cell halffaces vhs
	for (int side_idx = 0;side_idx < 2;side_idx++)
	{
		if (mesh->is_boundary(mesh->halfface_handle(fh,side_idx)))
		{
			continue;
		}
		auto b_hfhs = mesh->cell(mesh->incident_cell(mesh->halfface_handle(fh,side_idx))).halffaces();
		//构建体所需半面vertexhandle 按顺序的复合vector
		for (auto hfh_it = b_hfhs.begin();hfh_it != b_hfhs.end();hfh_it++)
		{
			if (*hfh_it == mesh->halfface_handle(fh,side_idx))
			{
				continue;
			}
			//原四面体非公共面上的半面
			std::vector<OvmVeH> b_fvhs;
			//需要新增cell的halffaces的vertexhandle 和某个半面按顺序vertex handle
			std::vector<std::vector<OvmVeH>> _hf_vhs;std::vector<OvmVeH> _vhs;
			for (auto hfv_it = mesh->hfv_iter(*hfh_it);hfv_it;hfv_it++)
			{
				b_fvhs.push_back(*hfv_it);
			}
			_hf_vhs.push_back(b_fvhs);
			for (int hf_idx = 0;hf_idx < 3;hf_idx++)
			{
				_vhs.push_back(new_vh);_vhs.push_back(b_fvhs[ordered_int[hf_idx]]);
				_vhs.push_back(b_fvhs[hf_idx]);_hf_vhs.push_back(_vhs);
				std::vector<OvmVeH>().swap(_vhs);
			}
			c_hf_vhs.push_back(_hf_vhs);std::vector<std::vector<OvmVeH>>().swap(_hf_vhs);
		}
		OvmVeH op_vh;
		for (auto cv_it = mesh->cv_iter(mesh->incident_cell(mesh->halfface_handle(fh,side_idx)));cv_it;cv_it++)
		{
			if ((*cv_it != t_f_vhs[0])&&(*cv_it != t_f_vhs[1])&&(*cv_it != t_f_vhs[2]))
			{
				op_vh = *cv_it;
				break;
			}
		}
		for (int t_f_idx = 0;t_f_idx < 3;t_f_idx++)
		{
			std::vector<OvmVeH> can_f_vhs;
			can_f_vhs.push_back(new_vh);can_f_vhs.push_back(op_vh);can_f_vhs.push_back(t_f_vhs[t_f_idx]);
			can_add_f_vhs.push_back(can_f_vhs);std::vector<OvmVeH>().swap(can_f_vhs);
		}
	}
	//Step 3删除OvmCeH
	foreach (auto &ch,deleted_chs)
		status_attrib[ch].set_deleted(true);

	status_attrib.garbage_collection(true);
	//Step 4 增加点
	for (auto dv_it = candidate_point.begin();dv_it != candidate_point.end();dv_it++)
	{
		OvmVeH added_vh = mesh->add_vertex(dv_it->v_position);V_PREV_HANDLE[added_vh] = dv_it->vh;
	}
	std::hash_map<OvmVeH,OvmVeH> old2newvh_mapping;old2newvh_mapping.clear();
	for (auto v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++)
		//old2newvh_mapping.insert(std::make_pair(V_PREV_HANDLE[*v_it],*v_it));
		old2newvh_mapping[V_PREV_HANDLE[*v_it]] = *v_it;
	//增加面
	for (auto f_it = can_add_f_vhs.begin();f_it != can_add_f_vhs.end();f_it++)
	{
		std::vector<OvmVeH> f_vhs;
		foreach (auto vh_ind,*f_it) f_vhs.push_back(old2newvh_mapping[vh_ind]);
		OvmFaH new_fh = mesh->add_face(f_vhs);
		F_IF_COVERED[new_fh] = 1;
		std::vector<OvmVeH>().swap(f_vhs);
	}
	//增加体
	std::vector<OvmHaFaH> covered_hfhs;covered_hfhs.clear();
	for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
	{
		if (F_IF_COVERED[*f_it] == 1)
		{
			covered_hfhs.push_back(mesh->halfface_handle(*f_it,0));
			covered_hfhs.push_back(mesh->halfface_handle(*f_it,1));
		}
	}
	for (auto c_it = c_hf_vhs.begin();c_it != c_hf_vhs.end();c_it++)
	{
		std::vector<OvmHaFaH> c_hfhs;c_hfhs.clear();
		for (auto hf_it = c_it->begin();hf_it != c_it->end();hf_it++)
		{
			//新建一个f vhs将旧的vh映射到新的vhs上
			std::vector<OvmVeH> new_fvhs;new_fvhs.clear();
			new_fvhs.push_back(old2newvh_mapping[(*hf_it)[0]]);
			new_fvhs.push_back(old2newvh_mapping[(*hf_it)[1]]);
			new_fvhs.push_back(old2newvh_mapping[(*hf_it)[2]]);
			bool if_ok;
			for (auto m_hf_it = covered_hfhs.begin();m_hf_it != covered_hfhs.end();m_hf_it++)
			//for (auto m_hf_it = mesh->halffaces_begin();m_hf_it != mesh->halffaces_end();m_hf_it++)
			{
				if_ok = false;
				std::vector<OvmVeH> m_hfv;m_hfv.clear();
				for (auto m_hfv_it = mesh->hfv_iter(*m_hf_it);m_hfv_it;m_hfv_it++)
				{
					m_hfv.push_back(*m_hfv_it);
				}
				int first_idx,counter_idx = 0;
				for (auto mhfv_it = m_hfv.begin();mhfv_it != m_hfv.end();mhfv_it++)
				{
					if (*mhfv_it == new_fvhs[0])
					{
						if_ok = true;
						first_idx = counter_idx;
						break;
					}
					counter_idx++;
				}
				if (if_ok&&new_fvhs[1] == m_hfv[ordered_int[first_idx]]&&new_fvhs[2] == m_hfv[ordered_int[ordered_int[first_idx]]])
				{
					c_hfhs.push_back(*m_hf_it);
					break;
				}

				std::vector<OvmVeH>().swap(m_hfv);
			}
			std::vector<OvmVeH>().swap(new_fvhs);
		}
		OvmCeH added_cell = mesh->add_cell(c_hfhs);std::vector<OvmHaFaH>().swap(c_hfhs);
		_chs.push_back(added_cell);
	}
}

OvmVeH tet_face_splitting_with_fix_point_and_mapping (VMesh* mesh, OvmFaH fh, OvmVec3d p_pos, std::hash_map<OvmCeH, OvmCeH>& new2oldchmapping)
{
	struct New_Point{
		OvmVeH vh;
		OvmVec3d v_position;
	};
	//说明，这里有可能是边界面，处理方式可能会不同
	//Step 0 将所有原VertexHandle作为属性存储起来,并做一些初始化
	//Step 1 遍历所有与eh相邻的四面体单元，判断与之相邻的所有节点在删除掉所有该删的四面体时候是否也会被删除
	//       判断依据是这个节点相邻的四面体是否是只有这些要删除的四面体，若会被删除则构建一个New_Point存储Vh和几何点位置
	//Step 2 构建需要新建面的Vertex Handle值，以及构造新的两类面所需要的三个VH值的vector，同时需要增加因为悬空点被删而被删的面
	//Step 3 删除OvmCeH
	//Step 4 增加点，包括边中心点和悬空点,并构建old2new_vh_mapping
	//Step 5 增加面,增加体
	OpenVolumeMesh::StatusAttrib status_attrib(*mesh);
	//Step 0
	auto V_PREV_HANDLE = mesh->request_vertex_property<OvmVeH>("prevhandle");
	for (auto v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++)
		V_PREV_HANDLE[*v_it] = *v_it;
	std::vector<OvmVeH> t_f_vhs;
	for (auto hfv_it = mesh->hfv_iter(mesh->halfface_handle(fh,0));hfv_it;hfv_it++)
	{
		t_f_vhs.push_back(*hfv_it);
	}
	auto F_IF_COVERED = mesh->request_face_property<int>("coveredfacehandle",0);
	//将所有cell赋值一个属性，到原来的OvmCeH的映射
	auto C_PRE_CHandle = mesh->request_cell_property<OvmCeH>("previous_cellhandle");
	for (auto c_it = mesh->cells_begin();c_it != mesh->cells_end();c_it++)
	{
		C_PRE_CHandle[*c_it] = *c_it;
	}
	//Step 1
	std::unordered_set<OvmCeH> deleted_chs;deleted_chs.clear();//所有待删除的cell handle
	std::unordered_set<OvmVeH> adj_deleted_vhs;adj_deleted_vhs.clear();//所有删除四面体单元之后悬空的点
	std::vector<std::vector<OvmVeH>> can_add_f_vhs;can_add_f_vhs.clear();//所有将因为删除悬挂点，而被删除的面,并加上需要在后续添加面的面片节点

	if (mesh->is_boundary(fh))
	{
		auto dch = mesh->is_boundary(mesh->halfface_handle(fh,1)) ?  mesh->incident_cell(mesh->halfface_handle(fh,0)) : mesh->incident_cell(mesh->halfface_handle(fh,1));
		deleted_chs.insert(dch);
	} 
	else
	{
		auto dch0 = mesh->incident_cell(mesh->halfface_handle(fh,0));
		auto dch1 = mesh->incident_cell(mesh->halfface_handle(fh,1));
		deleted_chs.insert(dch0);deleted_chs.insert(dch1);
	}

	//遍历所有与要删Cell相邻的VH,若这个VH只与要被删的Cells相邻则该VH也将被删除
	for (auto dc_it = deleted_chs.begin();dc_it != deleted_chs.end();dc_it++)
	{
		bool vh_deleted;
		for (auto cv_it = mesh->cv_iter(*dc_it);cv_it;cv_it++)
		{
			vh_deleted = true;
			for (auto vc_it = mesh->vc_iter(*cv_it);vc_it;vc_it++)
			{
				if (deleted_chs.find(*vc_it) == deleted_chs.end())
				{
					vh_deleted = false;
					break;
				}
			}
			if (vh_deleted == true)
			{
				adj_deleted_vhs.insert(*cv_it);
			}
		}
	}
	//构建在删除Cell后需要添加的Vertex的信息,所有这些信息集中在candidate_point中
	std::vector<New_Point> candidate_point;candidate_point.clear();
	OvmVec3d mid_position = p_pos;
	OvmVeH new_vh = (OvmVeH) mesh->n_vertices();
	New_Point mid_p;mid_p.v_position = mid_position;mid_p.vh = new_vh;
	candidate_point.push_back(mid_p);
	for (auto dv_it = adj_deleted_vhs.begin();dv_it != adj_deleted_vhs.end();dv_it++)
	{
		New_Point deleted_p;
		deleted_p.v_position = mesh->vertex(*dv_it);deleted_p.vh = *dv_it;
		candidate_point.push_back(deleted_p);
	}
	//遍历所有被删除的Cell，查看与被删的Cell的所有面是否有可能被删，若是被删（这个面上存在被删的vertex handle）
	//若被删除则记录这个Face 的三个Vertex Handle,除了这个面中有一个点被删除，还有一种可能是这个面仅与这个被删的cell相邻
	for (auto dc_it = deleted_chs.begin();dc_it != deleted_chs.end();dc_it++)
	{
		bool fh_deleted;
		auto adj_hfhs = mesh->cell(*dc_it).halffaces();
		for (auto can_hfh = adj_hfhs.begin();can_hfh != adj_hfhs.end();can_hfh++)
		{
			F_IF_COVERED[mesh->face_handle(*can_hfh)] = 1;
			fh_deleted = false;
			std::vector<OvmVeH> f_vhs;f_vhs.clear();
			//对于这个面属于边界，或者两个属于同一个面上的halfface都是属于将要被删除的cell中，则这个面将被删除
			if ((mesh->is_boundary(mesh->opposite_halfface_handle(*can_hfh))&&deleted_chs.size() == 2)||(contains(deleted_chs,mesh->incident_cell(mesh->opposite_halfface_handle(*can_hfh)))&&mesh->face_handle(*can_hfh) != fh))
			{
				fh_deleted = true;
			}
			//若这个面中有一个点属于被删除点集的，则将被删除
			for (auto hfv_it = mesh->hfv_iter(*can_hfh);hfv_it;hfv_it++)
			{
				f_vhs.push_back(*hfv_it);
				if (adj_deleted_vhs.find(*hfv_it) != adj_deleted_vhs.end())
				{
					fh_deleted = true;
				}
			}
			if (fh_deleted)
			{
				can_add_f_vhs.push_back(f_vhs);
			}
			std::vector<OvmVeH>().swap(f_vhs);
		}
	}
	//Step 2
	//循环数组
	int ordered_int[3];ordered_int[0] = 1;ordered_int[1] = 2;ordered_int[2] = 0;
	//Cell所组成的halffaces的vhs
	std::vector<std::vector<std::vector<OvmVeH>>> c_hf_vhs;c_hf_vhs.clear();
	//上述cell所组成的vhs与输入mesh对应的原来OvmCeh之间对应
	std::vector<OvmCeH> old_chs;std::vector<OvmCeH>().swap(old_chs);
	//增加将fh分裂为三个面的vhs
	for (int f_id = 0;f_id < 3;f_id++)
	{
		std::vector<OvmVeH> add_f_vhs;
		add_f_vhs.push_back(new_vh);
		add_f_vhs.push_back(t_f_vhs[f_id]);
		add_f_vhs.push_back(t_f_vhs[ordered_int[f_id]]);
		can_add_f_vhs.push_back(add_f_vhs);std::vector<OvmVeH>().swap(add_f_vhs);
	}
	//增加cell内部的can_add_f_vhs，同时增加cell halffaces vhs
	for (int side_idx = 0;side_idx < 2;side_idx++)
	{
		if (mesh->is_boundary(mesh->halfface_handle(fh,side_idx)))
		{
			continue;
		}
		OvmCeH current_ch = mesh->incident_cell(mesh->halfface_handle(fh,side_idx));
		auto b_hfhs = mesh->cell(current_ch).halffaces();
		//构建体所需半面vertexhandle 按顺序的复合vector
		for (auto hfh_it = b_hfhs.begin();hfh_it != b_hfhs.end();hfh_it++)
		{
			if (*hfh_it == mesh->halfface_handle(fh,side_idx))
			{
				continue;
			}
			//原四面体非公共面上的半面
			std::vector<OvmVeH> b_fvhs;
			//需要新增cell的halffaces的vertexhandle 和某个半面按顺序vertex handle
			std::vector<std::vector<OvmVeH>> _hf_vhs;std::vector<OvmVeH> _vhs;
			for (auto hfv_it = mesh->hfv_iter(*hfh_it);hfv_it;hfv_it++)
			{
				b_fvhs.push_back(*hfv_it);
			}
			_hf_vhs.push_back(b_fvhs);
			for (int hf_idx = 0;hf_idx < 3;hf_idx++)
			{
				_vhs.push_back(new_vh);_vhs.push_back(b_fvhs[ordered_int[hf_idx]]);
				_vhs.push_back(b_fvhs[hf_idx]);_hf_vhs.push_back(_vhs);
				std::vector<OvmVeH>().swap(_vhs);
			}
			c_hf_vhs.push_back(_hf_vhs);std::vector<std::vector<OvmVeH>>().swap(_hf_vhs);
			old_chs.push_back(current_ch);
		}
		OvmVeH op_vh;
		for (auto cv_it = mesh->cv_iter(mesh->incident_cell(mesh->halfface_handle(fh,side_idx)));cv_it;cv_it++)
		{
			if ((*cv_it != t_f_vhs[0])&&(*cv_it != t_f_vhs[1])&&(*cv_it != t_f_vhs[2]))
			{
				op_vh = *cv_it;
				break;
			}
		}
		for (int t_f_idx = 0;t_f_idx < 3;t_f_idx++)
		{
			std::vector<OvmVeH> can_f_vhs;
			can_f_vhs.push_back(new_vh);can_f_vhs.push_back(op_vh);can_f_vhs.push_back(t_f_vhs[t_f_idx]);
			can_add_f_vhs.push_back(can_f_vhs);std::vector<OvmVeH>().swap(can_f_vhs);
		}
	}
	//Step 3删除OvmCeH
	foreach (auto &ch,deleted_chs)
		status_attrib[ch].set_deleted(true);

	status_attrib.garbage_collection(true);
	//Step 4 增加点
	for (auto dv_it = candidate_point.begin();dv_it != candidate_point.end();dv_it++)
	{
		OvmVeH added_vh = mesh->add_vertex(dv_it->v_position);V_PREV_HANDLE[added_vh] = dv_it->vh;
	}
	std::hash_map<OvmVeH,OvmVeH> old2newvh_mapping;old2newvh_mapping.clear();
	for (auto v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++)
		//old2newvh_mapping.insert(std::make_pair(V_PREV_HANDLE[*v_it],*v_it));
		old2newvh_mapping[V_PREV_HANDLE[*v_it]] = *v_it;
	//增加面
	for (auto f_it = can_add_f_vhs.begin();f_it != can_add_f_vhs.end();f_it++)
	{
		std::vector<OvmVeH> f_vhs;
		foreach (auto vh_ind,*f_it) f_vhs.push_back(old2newvh_mapping[vh_ind]);
		OvmFaH new_fh = mesh->add_face(f_vhs);
		F_IF_COVERED[new_fh] = 1;
		std::vector<OvmVeH>().swap(f_vhs);
	}
	//增加体
	std::vector<OvmHaFaH> covered_hfhs;covered_hfhs.clear();
	for (auto f_it = mesh->faces_begin();f_it != mesh->faces_end();f_it++)
	{
		if (F_IF_COVERED[*f_it] == 1)
		{
			covered_hfhs.push_back(mesh->halfface_handle(*f_it,0));
			covered_hfhs.push_back(mesh->halfface_handle(*f_it,1));
		}
	}
	auto d_it = old_chs.begin();
	for (auto c_it = c_hf_vhs.begin();c_it != c_hf_vhs.end();c_it++,d_it++)
	{
		std::vector<OvmHaFaH> c_hfhs;c_hfhs.clear();
		for (auto hf_it = c_it->begin();hf_it != c_it->end();hf_it++)
		{
			//新建一个f vhs将旧的vh映射到新的vhs上
			std::vector<OvmVeH> new_fvhs;new_fvhs.clear();
			new_fvhs.push_back(old2newvh_mapping[(*hf_it)[0]]);
			new_fvhs.push_back(old2newvh_mapping[(*hf_it)[1]]);
			new_fvhs.push_back(old2newvh_mapping[(*hf_it)[2]]);
			bool if_ok;
			for (auto m_hf_it = covered_hfhs.begin();m_hf_it != covered_hfhs.end();m_hf_it++)
				//for (auto m_hf_it = mesh->halffaces_begin();m_hf_it != mesh->halffaces_end();m_hf_it++)
			{
				if_ok = false;
				std::vector<OvmVeH> m_hfv;m_hfv.clear();
				for (auto m_hfv_it = mesh->hfv_iter(*m_hf_it);m_hfv_it;m_hfv_it++)
				{
					m_hfv.push_back(*m_hfv_it);
				}
				int first_idx,counter_idx = 0;
				for (auto mhfv_it = m_hfv.begin();mhfv_it != m_hfv.end();mhfv_it++)
				{
					if (*mhfv_it == new_fvhs[0])
					{
						if_ok = true;
						first_idx = counter_idx;
						break;
					}
					counter_idx++;
				}
				if (if_ok&&new_fvhs[1] == m_hfv[ordered_int[first_idx]]&&new_fvhs[2] == m_hfv[ordered_int[ordered_int[first_idx]]])
				{
					c_hfhs.push_back(*m_hf_it);
					break;
				}

				std::vector<OvmVeH>().swap(m_hfv);
			}
			std::vector<OvmVeH>().swap(new_fvhs);
		}
		OvmCeH added_cell = mesh->add_cell(c_hfhs);std::vector<OvmHaFaH>().swap(c_hfhs);
		C_PRE_CHandle[added_cell] = *d_it;
	}
	std::hash_map<OvmCeH, OvmCeH>().swap(new2oldchmapping);
	for (auto c_it = mesh->cells_begin();c_it != mesh->cells_end();c_it++)
	{
		new2oldchmapping[*c_it] = C_PRE_CHandle[*c_it];
	}
	return new_vh;
}

OP_Mesh* create_boundary_mesh(VolumeMesh* mesh)
{
	OP_Mesh* tri_mesh = new OP_Mesh;
	//将OpenVolumeMesh的边界面上的网格节点加入到一个hash_map里，第一个表示在新的OpenMesh里面的handle值，第二个表示在OpenVolumeMesh里面的handle值
	//同时建立OmVeH的vector用于存放OP_mesh中handle值与对应的位置
	std::vector<OmVeH> ovhs;
	std::hash_map<OvmVeH,size_t> omv_ov_mapping;
	omv_ov_mapping.clear();
	omf_ovmch_mapping.clear();
	ovhs.clear();
	//加点到OpenMesh中并记下，openvolumemesh那些边界点与openmesh之间idx之间的对应关系
	for (OpenVolumeMesh::VertexIter v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++)
	{
		if (mesh->is_boundary(*v_it))
		{
			OP_Mesh::Point now_point(mesh->vertex(*v_it)[0],mesh->vertex(*v_it)[1],mesh->vertex(*v_it)[2]);
			OmVeH now_vh = tri_mesh->add_vertex(now_point);
			ovhs.push_back(now_vh);
			omv_ov_mapping[*v_it] = now_vh.idx();
		}
	}
	std::vector<OmVeH> face_vhandles;
	//将OpenVolumeMesh的边界面逐个分成两个三角形面，依次加入新的OpenMesh中的面中
	for (OpenVolumeMesh::BoundaryFaceIter bf_it = mesh->bf_iter();bf_it;bf_it++)
	{
		OvmHaFaH hfh = (mesh->is_boundary(mesh->halfface_handle(*bf_it,0))) ? mesh->halfface_handle(*bf_it,0) : mesh->halfface_handle(*bf_it,1);
		OvmHaFaH hfh_o = (mesh->is_boundary(mesh->halfface_handle(*bf_it,1))) ? mesh->halfface_handle(*bf_it,0) : mesh->halfface_handle(*bf_it,1);
	
		OvmCeH _cell = mesh->incident_cell(hfh_o);
		size_t ovh4[4];
		int counter_bf = 0;
		for (OpenVolumeMesh::HalfFaceVertexIter hfv_it = mesh->hfv_iter(hfh);hfv_it;hfv_it++)
		{
			ovh4[counter_bf++] = omv_ov_mapping[*hfv_it];
		}
		face_vhandles.clear();
		face_vhandles.push_back(ovhs[ovh4[2]]);
		face_vhandles.push_back(ovhs[ovh4[1]]);
		face_vhandles.push_back(ovhs[ovh4[0]]);
		OmFaH fh_1 = tri_mesh->add_face(face_vhandles);
		omf_ovmch_mapping[fh_1.idx()] = _cell;
		face_vhandles.clear();
		face_vhandles.push_back(ovhs[ovh4[0]]);
		face_vhandles.push_back(ovhs[ovh4[3]]);
		face_vhandles.push_back(ovhs[ovh4[2]]);
		OmFaH fh_2 = tri_mesh->add_face(face_vhandles);
		omf_ovmch_mapping[fh_2.idx()] = _cell;
	}
	return tri_mesh;
}

//用于quad_surfaces项目中的boundary OpenVolumeMesh的获取
OP_Mesh* get_boundary_mesh(VMesh* mesh)
{
	OP_Mesh* tri_mesh = new OP_Mesh;
	omf_ovmch_mapping.clear();
	//将OpenVolumeMesh的边界面上的网格节点加入到一个hash_map中，第一个表示在OpenVolumnMesh中的handle值，第二个表示OpenMesh中的size_t值
	std::hash_map<OvmVeH,size_t> omv_ov_mapping;omv_ov_mapping.clear();
	//所有OpenVertexHandle的集合
	std::vector<OmVeH> ovhs;ovhs.clear();
	//遍历所有tet mesh中的边界点，将其存入ovhs中
	for (OpenVolumeMesh::VertexIter v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++)
	{
		if (mesh->is_boundary(*v_it))
		{
			OP_Mesh::Point now_point(mesh->vertex(*v_it)[0],mesh->vertex(*v_it)[1],mesh->vertex(*v_it)[2]);
			OmVeH now_vh = tri_mesh->add_vertex(now_point);
			ovhs.push_back(now_vh);
			omv_ov_mapping[*v_it] = now_vh.idx();
		}
	}
	std::vector<OmVeH> face_vhs;
	//遍历所有边界三角形面，作为OpenMesh中的面
	for (OpenVolumeMesh::BoundaryFaceIter bf_it = mesh->bf_iter();bf_it;bf_it++)
	{
		OvmHaFaH ohf = (mesh->is_boundary(mesh->halfface_handle(*bf_it,1))) ? mesh->halfface_handle(*bf_it,0) : mesh->halfface_handle(*bf_it,1);
		OvmCeH _cell = mesh->incident_cell(ohf);
		//测试是否正确
		if (!_cell.is_valid())
		{
			
			std::cout<<bf_it->idx()<<"get_boundary_mesh_wrong!"<<std::endl;
			system("pause");
		}
		size_t ovhs3[3];
		int counter_b = 0;
		for (OpenVolumeMesh::HalfFaceVertexIter hfv_it = mesh->hfv_iter(ohf);hfv_it;hfv_it++)
		{
			ovhs3[counter_b++] = omv_ov_mapping[*hfv_it];
		}
		face_vhs.clear();
		face_vhs.push_back(ovhs[ovhs3[0]]);
		face_vhs.push_back(ovhs[ovhs3[1]]);
		face_vhs.push_back(ovhs[ovhs3[2]]);
		OmFaH now_face = tri_mesh->add_face(face_vhs);
		omf_ovmch_mapping[now_face.idx()] = _cell;
	}
	return tri_mesh;
}

OP_Mesh* get_boundary_mesh(VMesh* mesh, std::hash_map<size_t,OvmCeH>& f2cellmapping, std::hash_map<OvmCeH, size_t>& bcell2fmapping)
{
	OP_Mesh* tri_mesh = new OP_Mesh;
	omf_ovmch_mapping.clear();
	std::hash_map<size_t, OvmCeH>().swap(f2cellmapping);
	std::hash_map<OvmCeH, size_t>().swap(bcell2fmapping);
	//将OpenVolumeMesh的边界面上的网格节点加入到一个hash_map中，第一个表示在OpenVolumnMesh中的handle值，第二个表示OpenMesh中的size_t值
	std::hash_map<OvmVeH,size_t> omv_ov_mapping;omv_ov_mapping.clear();
	//所有OpenVertexHandle的集合
	std::vector<OmVeH> ovhs;ovhs.clear();
	//遍历所有tet mesh中的边界点，将其存入ovhs中
	for (OpenVolumeMesh::VertexIter v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++)
	{
		if (mesh->is_boundary(*v_it))
		{
			OP_Mesh::Point now_point(mesh->vertex(*v_it)[0],mesh->vertex(*v_it)[1],mesh->vertex(*v_it)[2]);
			OmVeH now_vh = tri_mesh->add_vertex(now_point);
			ovhs.push_back(now_vh);
			omv_ov_mapping[*v_it] = now_vh.idx();
		}
	}
	std::vector<OmVeH> face_vhs;
	//遍历所有边界三角形面，作为OpenMesh中的面
	for (OpenVolumeMesh::BoundaryFaceIter bf_it = mesh->bf_iter();bf_it;bf_it++)
	{
		OvmHaFaH ohf = (mesh->is_boundary(mesh->halfface_handle(*bf_it,1))) ? mesh->halfface_handle(*bf_it,0) : mesh->halfface_handle(*bf_it,1);
		OvmCeH _cell = mesh->incident_cell(ohf);
		//测试是否正确
		if (!_cell.is_valid())
		{
			std::cout<<"get_boundary_mesh_wrong!"<<std::endl;
			system("pause");
		}
		size_t ovhs3[3];
		int counter_b = 0;
		for (OpenVolumeMesh::HalfFaceVertexIter hfv_it = mesh->hfv_iter(ohf);hfv_it;hfv_it++)
		{
			ovhs3[counter_b++] = omv_ov_mapping[*hfv_it];
		}
		face_vhs.clear();
		face_vhs.push_back(ovhs[ovhs3[0]]);
		face_vhs.push_back(ovhs[ovhs3[1]]);
		face_vhs.push_back(ovhs[ovhs3[2]]);
		OmFaH now_face = tri_mesh->add_face(face_vhs);
		omf_ovmch_mapping[now_face.idx()] = _cell;
	}
	for (auto f_it = tri_mesh->faces_begin();f_it != tri_mesh->faces_end();f_it++)
	{
		f2cellmapping[f_it->idx()] = omf_ovmch_mapping[f_it->idx()];
		bcell2fmapping[omf_ovmch_mapping[f_it->idx()]] = f_it->idx();
	}
	return tri_mesh;
}

OP_Mesh* get_boundary_mesh(VMesh* mesh, std::hash_map<size_t,OvmCeH>& f2cellmapping, std::hash_map<OvmCeH, size_t>& bcell2fmapping, std::hash_map<size_t, OvmVeH>& omv2ovmvhmapping)
{
	OP_Mesh* tri_mesh = new OP_Mesh;
	omf_ovmch_mapping.clear();
	std::hash_map<size_t, OvmCeH>().swap(f2cellmapping);
	std::hash_map<OvmCeH, size_t>().swap(bcell2fmapping);
	//将OpenVolumeMesh的边界面上的网格节点加入到一个hash_map中，第一个表示在OpenVolumnMesh中的handle值，第二个表示OpenMesh中的size_t值
	std::hash_map<OvmVeH,size_t> omv_ov_mapping;omv_ov_mapping.clear();omv2ovmvhmapping.clear();
	//所有OpenVertexHandle的集合
	std::vector<OmVeH> ovhs;ovhs.clear();
	//遍历所有tet mesh中的边界点，将其存入ovhs中
	for (OpenVolumeMesh::VertexIter v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++)
	{
		if (mesh->is_boundary(*v_it))
		{
			OP_Mesh::Point now_point(mesh->vertex(*v_it)[0],mesh->vertex(*v_it)[1],mesh->vertex(*v_it)[2]);
			OmVeH now_vh = tri_mesh->add_vertex(now_point);
			ovhs.push_back(now_vh);
			omv_ov_mapping[*v_it] = now_vh.idx();
			omv2ovmvhmapping[now_vh.idx()] = *v_it;
		}
	}
	std::vector<OmVeH> face_vhs;
	//遍历所有边界三角形面，作为OpenMesh中的面
	for (OpenVolumeMesh::BoundaryFaceIter bf_it = mesh->bf_iter();bf_it;bf_it++)
	{
		OvmHaFaH ohf = (mesh->is_boundary(mesh->halfface_handle(*bf_it,1))) ? mesh->halfface_handle(*bf_it,0) : mesh->halfface_handle(*bf_it,1);
		OvmCeH _cell = mesh->incident_cell(ohf);
		//测试是否正确
		if (!_cell.is_valid())
		{
			std::cout<<"get_boundary_mesh_wrong!"<<std::endl;
			system("pause");
		}
		size_t ovhs3[3];
		int counter_b = 0;
		for (OpenVolumeMesh::HalfFaceVertexIter hfv_it = mesh->hfv_iter(ohf);hfv_it;hfv_it++)
		{
			ovhs3[counter_b++] = omv_ov_mapping[*hfv_it];
		}
		face_vhs.clear();
		face_vhs.push_back(ovhs[ovhs3[0]]);
		face_vhs.push_back(ovhs[ovhs3[1]]);
		face_vhs.push_back(ovhs[ovhs3[2]]);
		OmFaH now_face = tri_mesh->add_face(face_vhs);
		omf_ovmch_mapping[now_face.idx()] = _cell;
	}
	for (auto f_it = tri_mesh->faces_begin();f_it != tri_mesh->faces_end();f_it++)
	{
		f2cellmapping[f_it->idx()] = omf_ovmch_mapping[f_it->idx()];
		bcell2fmapping[omf_ovmch_mapping[f_it->idx()]] = f_it->idx();
	}
	return tri_mesh;
}

OP_Mesh* get_boundary_mesh(VolumeMesh* mesh, std::hash_map<size_t, OvmCeH>& f2cellmapping, std::hash_map<OvmCeH, size_t>& bcell2fmapping, std::hash_map<size_t, OvmVeH>& omv2ovmvhmapping, std::hash_map<OvmVeH, size_t>& ovmvh2omvhmapping)
{
	//初始化 并将传递参数清空
	OP_Mesh* boundary_mesh = new OP_Mesh;
	std::hash_map<size_t, OvmCeH>().swap(f2cellmapping); 
	std::hash_map<OvmCeH, size_t>().swap(bcell2fmapping);
	std::hash_map<size_t, OvmVeH>().swap(omv2ovmvhmapping);
	std::hash_map<OvmVeH, size_t>().swap(ovmvh2omvhmapping);
	//
	std::vector<OmVeH> ovhs;
	//遍历边界边 将顶点构建好先
	for (auto v_iter = mesh->vertices_begin();v_iter != mesh->vertices_end();v_iter++)
	{
		if (mesh->is_boundary(*v_iter))
		{
			OP_Mesh::Point now_point(mesh->vertex(*v_iter)[0],mesh->vertex(*v_iter)[1],mesh->vertex(*v_iter)[2]);
			OmVeH now_vh = boundary_mesh->add_vertex(now_point);
			ovhs.push_back(now_vh);
			ovmvh2omvhmapping[*v_iter] = now_vh.idx();
			omv2ovmvhmapping[now_vh.idx()] = *v_iter;
		}
	}
	std::vector<OmVeH> face_vhs;
	for (OpenVolumeMesh::BoundaryFaceIter bf_it = mesh->bf_iter();bf_it;bf_it++)
	{
		OvmHaFaH ohf = (mesh->is_boundary(mesh->halfface_handle(*bf_it,1))) ? mesh->halfface_handle(*bf_it,0) : mesh->halfface_handle(*bf_it,1);
		OvmCeH _cell = mesh->incident_cell(ohf);
		//测试是否正确
		if (!_cell.is_valid())
		{
			std::cout<<"get_boundary_mesh_wrong!"<<std::endl;
			system("pause");
		}
		int counter_b = 0;
		for (OpenVolumeMesh::HalfFaceVertexIter hfv_it = mesh->hfv_iter(ohf);hfv_it;hfv_it++)
		{
			face_vhs.push_back(ovhs[ovmvh2omvhmapping[*hfv_it]]);
		}
		OmFaH now_face = boundary_mesh->add_face(face_vhs);
		std::vector<OmVeH>().swap(face_vhs);
		f2cellmapping[now_face.idx()] = _cell;
		bcell2fmapping[_cell] = now_face.idx();
	}
	return boundary_mesh;
}

OP_Mesh* get_boundary_mesh(VolumeMesh* mesh, std::hash_map<size_t, OvmCeH>& f2cellmapping, std::hash_map<OvmCeH, std::vector<size_t>>& bcell2fmapping, std::hash_map<size_t, OvmVeH>& omv2ovmvhmapping, std::hash_map<OvmVeH, size_t>& ovmvh2omvhmapping)
{
	//初始化 并将传递参数清空
	OP_Mesh* boundary_mesh = new OP_Mesh;
	std::hash_map<size_t, OvmCeH>().swap(f2cellmapping); 
	std::hash_map<OvmCeH, std::vector<size_t>>().swap(bcell2fmapping);
	std::hash_map<size_t, OvmVeH>().swap(omv2ovmvhmapping);
	std::hash_map<OvmVeH, size_t>().swap(ovmvh2omvhmapping);
	//
	std::vector<OmVeH> ovhs;
	//遍历边界边 将顶点构建好先
	for (auto v_iter = mesh->vertices_begin();v_iter != mesh->vertices_end();v_iter++)
	{
		if (mesh->is_boundary(*v_iter))
		{
			OP_Mesh::Point now_point(mesh->vertex(*v_iter)[0],mesh->vertex(*v_iter)[1],mesh->vertex(*v_iter)[2]);
			OmVeH now_vh = boundary_mesh->add_vertex(now_point);
			ovhs.push_back(now_vh);
			ovmvh2omvhmapping[*v_iter] = now_vh.idx();
			omv2ovmvhmapping[now_vh.idx()] = *v_iter;
		}
	}
	std::vector<OmVeH> face_vhs;
	std::unordered_set<OvmCeH> bchs;
	for (OpenVolumeMesh::BoundaryFaceIter bf_it = mesh->bf_iter();bf_it;bf_it++)
	{
		OvmHaFaH ohf = (mesh->is_boundary(mesh->halfface_handle(*bf_it,1))) ? mesh->halfface_handle(*bf_it,0) : mesh->halfface_handle(*bf_it,1);
		OvmCeH _cell = mesh->incident_cell(ohf);
		//测试是否正确
		if (!_cell.is_valid())
		{
			std::cout<<"get_boundary_mesh_wrong!"<<std::endl;
			system("pause");
		}
		int counter_b = 0;
		for (OpenVolumeMesh::HalfFaceVertexIter hfv_it = mesh->hfv_iter(ohf);hfv_it;hfv_it++)
		{
			face_vhs.push_back(ovhs[ovmvh2omvhmapping[*hfv_it]]);
		}
		OmFaH now_face = boundary_mesh->add_face(face_vhs);
		std::vector<OmVeH>().swap(face_vhs);
		f2cellmapping[now_face.idx()] = _cell;
		omf_ovmch_mapping[now_face.idx()] = _cell;
		if (bchs.find(_cell) == bchs.end())
		{
			bchs.insert(_cell);
			bcell2fmapping[_cell] = std::vector<size_t>();
		}
	}
	for (auto fcit = f2cellmapping.begin();fcit != f2cellmapping.end();fcit++)
	{
		bcell2fmapping[fcit->second].push_back(fcit->first);
	}
	return boundary_mesh;
}

void init_volume_mesh (VolumeMesh *mesh, BODY *body, double myresabs)
{
	attach_mesh_elements_to_ACIS_entities (mesh, body, myresabs);

	assert(mesh->vertex_property_exists<unsigned long>("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

	auto V_PREV_HANDLE = mesh->request_vertex_property<OvmVeH> ("prevhandle", -1);
	mesh->set_persistent (V_PREV_HANDLE);
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
		V_PREV_HANDLE[*v_it] = *v_it;

	auto E_PREV_HANDLE = mesh->request_edge_property<OvmEgH> ("prevhandle", -1);
	mesh->set_persistent (E_PREV_HANDLE);
	for (auto e_it = mesh->edges_begin (); e_it != mesh->edges_end (); ++e_it)
		E_PREV_HANDLE[*e_it] = *e_it;

	auto F_PREV_HANDLE = mesh->request_face_property<OvmFaH> ("prevhandle", -1);
	mesh->set_persistent (F_PREV_HANDLE);
	for (auto c_it = mesh->faces_begin (); c_it != mesh->faces_end (); ++c_it)
		F_PREV_HANDLE[*c_it] = *c_it;

	auto C_PREV_HANDLE = mesh->request_cell_property<OvmCeH> ("prevhandle", -1);
	mesh->set_persistent (C_PREV_HANDLE);
	for (auto c_it = mesh->cells_begin (); c_it != mesh->cells_end (); ++c_it)
		C_PREV_HANDLE[*c_it] = *c_it;
}

void init_tet_mesh (VMesh *mesh, BODY *body, double myresabs)
{
	attach_tet_mesh_elements_to_ACIS_entities(mesh, body, myresabs);

	assert(mesh->vertex_property_exists<unsigned long>("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

	auto V_PREV_HANDLE = mesh->request_vertex_property<OvmVeH> ("prevhandle", -1);
	mesh->set_persistent (V_PREV_HANDLE);
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
		V_PREV_HANDLE[*v_it] = *v_it;

	auto E_PREV_HANDLE = mesh->request_edge_property<OvmEgH> ("prevhandle", -1);
	mesh->set_persistent (E_PREV_HANDLE);
	for (auto e_it = mesh->edges_begin (); e_it != mesh->edges_end (); ++e_it)
		E_PREV_HANDLE[*e_it] = *e_it;

	auto F_PREV_HANDLE = mesh->request_face_property<OvmFaH> ("prevhandle", -1);
	mesh->set_persistent (F_PREV_HANDLE);
	for (auto c_it = mesh->faces_begin (); c_it != mesh->faces_end (); ++c_it)
		F_PREV_HANDLE[*c_it] = *c_it;

	auto C_PREV_HANDLE = mesh->request_cell_property<OvmCeH> ("prevhandle", -1);
	mesh->set_persistent (C_PREV_HANDLE);
	for (auto c_it = mesh->cells_begin (); c_it != mesh->cells_end (); ++c_it)
		C_PREV_HANDLE[*c_it] = *c_it;
}

bool save_volume_mesh (VolumeMesh *mesh, QString mesh_path)
{
	OpenVolumeMesh::IO::FileManager fileManager;
	mesh->clear_mesh_props ();
	mesh->clear_vertex_props ();
	mesh->clear_edge_props ();
	mesh->clear_face_props ();
	mesh->clear_cell_props ();
	return fileManager.writeFile (mesh_path.toStdString (), *mesh);
}

BODY * load_acis_model (QString file_path)
{
	FILE *f = fopen (file_path.toAscii ().data (), "r");
	if (!f){
		QMessageBox::critical (NULL, QObject::tr ("打开错误！"), QObject::tr("打开模型文件失败！"));
		return NULL;
	}
	ENTITY_LIST entities;
	api_restore_entity_list (f, TRUE, entities);
	for (int i = 0; i != entities.count (); ++i)
	{
		ENTITY *ent = entities[i];
		if (is_BODY (ent))
		{
			//api_clean_entity(ent);
			return (BODY *)ent;
		}
	}
	return NULL;
}

void save_acis_entity(ENTITY *entity, const char * file_name)
{
	ENTITY_LIST elist;
	ENTITY *copyentity = NULL;
	api_deep_down_copy_entity (entity, copyentity);
	elist.add (copyentity);
	// Set the units and product_id.
	FileInfo fileinfo;
	fileinfo.set_units (1.0);
	fileinfo.set_product_id ("Example Application");
	outcome result = api_set_file_info((FileId | FileUnits), fileinfo);
	check_outcome(result); 

	// Also set the option to produce sequence numbers in the SAT file.
	result = api_set_int_option("sequence_save_files", 1);
	check_outcome(result);

	// Open a file for writing, save the list of entities, and close the file.
	FILE * save_file = fopen(file_name, "w");
	result = api_save_entity_list(save_file, TRUE, elist);
	fclose(save_file);
	check_outcome(result);
}

bool parse_xml_file (QString xml_path, QString &file_type, QString &data_name, std::vector<std::pair<QString, QString> > &path_pairs)
{
	QDomDocument doc("mydocument");
	QFile file(xml_path);
	if (!file.open(QIODevice::ReadOnly))
		return false;
	if (!doc.setContent(&file)) {
		file.close();
		return false;
	}
	file.close();
	QDomElement root_elem = doc.documentElement();
	file_type = root_elem.tagName ();
	data_name = root_elem.attribute ("name");
	QDomNode n = root_elem.firstChild();
	std::pair<QString, QString> path_pair;

	if (file_type != "meshedit" && file_type != "meshmatch" && file_type != "meshoptimization"){
		QMessageBox::warning (NULL, QObject::tr ("解析错误！"), QObject::tr ("这不是一个有效的文件！"));
		return false;
	}
	QFileInfo fileInfo (xml_path);
	QString xml_file_dir = fileInfo.absolutePath ();

	while (!n.isNull ()){
		QDomElement e = n.toElement();
		path_pair.first = xml_file_dir + "/" + e.attribute ("meshpath");
		path_pair.second = xml_file_dir + "/" + e.attribute ("modelpath");
		path_pairs.push_back (path_pair);
		n = n.nextSibling ();
	}

	return true;
}

std::vector<OvmFaH> get_adj_faces_around_edge (VolumeMesh *mesh, OvmHaEgH heh, bool on_boundary)
{
	std::vector<OvmFaH> fhs;
	for (auto hehf_it = mesh->hehf_iter (heh); hehf_it; ++hehf_it)
	{
		OvmFaH fh = mesh->face_handle (*hehf_it);
		if (fh == mesh->InvalidFaceHandle)
			continue;
		if (on_boundary && !mesh->is_boundary (fh))
			continue;
		fhs.push_back (fh);
	}
	return fhs;
}

std::vector<OvmFaH> get_adj_faces_around_edge (VolumeMesh *mesh, OvmEgH eh, bool on_boundary)
{
	return get_adj_faces_around_edge (mesh, mesh->halfedge_handle (eh, 0), on_boundary);
}

inline void get_adj_faces_around_edge (VolumeMesh *mesh, OvmEgH eh, std::unordered_set<OvmFaH> &faces, bool on_boundary)
{
	get_adj_faces_around_edge (mesh, mesh->halfedge_handle (eh, 0), faces, on_boundary);
}

void get_adj_faces_around_edge (VolumeMesh *mesh, OvmHaEgH heh, std::unordered_set<OvmFaH> &faces, bool on_boundary)
{
	for (auto hehf_it = mesh->hehf_iter (heh); hehf_it; ++hehf_it)
	{
		OvmFaH fh = mesh->face_handle (*hehf_it);
		if (fh == mesh->InvalidFaceHandle)
			continue;
		if (on_boundary && !mesh->is_boundary (fh))
			continue;
		faces.insert (fh);
	}
}

void get_adj_vertices_around_vertex (VolumeMesh *mesh, OvmVeH vh, std::unordered_set<OvmVeH> &vertices)
{
	for (auto voh_it = mesh->voh_iter (vh); voh_it; ++voh_it){
		vertices.insert (mesh->halfedge (*voh_it).to_vertex ());
	}
}

void get_adj_edges_around_face (VolumeMesh *mesh, OvmFaH fh, std::unordered_set<OvmEgH> &edges)
{
	auto hehs = mesh->face (fh).halfedges ();
	foreach (auto &heh, hehs){
		edges.insert (mesh->edge_handle (heh));
	}
}

void get_adj_edges_around_face_tetmesh(VMesh *mesh, OvmFaH fh, std::unordered_set<OvmEgH> &edges)
{
	auto hehs = mesh->face (fh).halfedges ();
	foreach (auto &heh, hehs){
		edges.insert (mesh->edge_handle (heh));
	}
}

void get_adj_edges_around_cell(VolumeMesh *mesh, OvmCeH ch, std::unordered_set<OvmEgH> & ehs_on_ch)
{
	foreach(auto & hfh, mesh->cell(ch).halffaces()){
		foreach(auto & heh, mesh->halfface(hfh).halfedges()){
			ehs_on_ch.insert(mesh->edge_handle(heh));
		}
	}
}

void get_adj_edges_around_tet(VMesh *mesh, OvmCeH ch, std::unordered_set<OvmEgH> &ehs_on_tet)
{
	foreach (auto & hfh, mesh->cell(ch).halffaces())
	{
		foreach (auto & heh, mesh->halfface(hfh).halfedges())
		{
			ehs_on_tet.insert(mesh->edge_handle(heh));
		}
	}
}

void get_adj_vertices_around_face (VolumeMesh *mesh, OvmFaH fh, std::vector<OvmVeH> &vertices)
{
	OvmHaFaH hfh = mesh->halfface_handle (fh, 0);
	for (auto hfv_it = mesh->hfv_iter (hfh); hfv_it; ++hfv_it)
		vertices.push_back (*hfv_it);
}

std::vector<OvmVeH> get_adj_vertices_around_face (VolumeMesh *mesh, OvmFaH fh)
{
	std::vector<OvmVeH> vertices;
	get_adj_vertices_around_face (mesh, fh, vertices);
	return vertices;
}

void get_adj_vertices_around_cell (VolumeMesh *mesh, OvmCeH ch, std::unordered_set<OvmVeH> &vertices)
{
	for (auto cv_it = mesh->cv_iter (ch); cv_it; ++cv_it)
		vertices.insert (*cv_it);
}

void get_adj_vertices_around_tet (VMesh *mesh, OvmCeH ch,std::unordered_set<OvmVeH> & vertices)
{
	for (auto cv_it = mesh->cv_iter(ch); cv_it;cv_it++)
		vertices.insert (*cv_it);
}

std::vector<OvmVeH> get_adj_vertices_around_cell (VolumeMesh *mesh, OvmCeH ch)
{
	std::vector<OvmVeH> vertices;
	for (auto cv_it = mesh->cv_iter (ch); cv_it; ++cv_it)
		vertices.push_back (*cv_it);
	return vertices;
}

std::vector<OvmVeH> get_adj_vertices_around_hexa (VolumeMesh *mesh, OvmCeH ch)
{
	std::vector<OvmVeH> vertices;
	for (auto cv_it = mesh->hv_iter (ch); cv_it; ++cv_it)
		vertices.push_back (*cv_it);
	return vertices;
}

void get_adj_boundary_vertices_around_vertex (VolumeMesh *mesh, OvmVeH vh, std::unordered_set<OvmVeH> &vertices)
{
	for (auto voh_it = mesh->voh_iter (vh); voh_it; ++voh_it){
		OvmVeH vh = mesh->halfedge (*voh_it).to_vertex ();
		if (mesh->is_boundary (vh))
			vertices.insert (vh);
	}
}

void get_adj_hexas_around_edge (VolumeMesh *mesh, OvmHaEgH heh, std::unordered_set<OvmCeH> &hexas)
{
	for (auto hehf_it = mesh->hehf_iter (heh); hehf_it; ++hehf_it){
		OvmCeH ch = mesh->incident_cell (*hehf_it);
		if (ch == mesh->InvalidCellHandle)
			continue;
		hexas.insert (ch);
	}
}

void get_adj_tets_around_edge(VMesh* mesh,OvmHaEgH heh,std::unordered_set<OvmCeH> &tets)
{
	for (auto hehf_it = mesh->hehf_iter(heh);hehf_it;hehf_it++)
	{
		OvmCeH ch = mesh->incident_cell(*hehf_it);
		if (ch == mesh->InvalidCellHandle) continue;
		tets.insert(ch);
	}
}

void get_adj_hexas_around_edge (VolumeMesh *mesh, OvmEgH eh, std::unordered_set<OvmCeH> &hexas)
{
	OvmHaEgH heh = mesh->halfedge_handle (eh, 0);
	return get_adj_hexas_around_edge (mesh, heh, hexas);
}

void get_adj_hexas_around_vertex (VolumeMesh *mesh, OvmVeH vh, std::unordered_set<OvmCeH> &hexas)
{
	for (auto vc_it = mesh->vc_iter (vh); vc_it; ++vc_it){
		OvmCeH ch = *vc_it;
		if (ch == mesh->InvalidCellHandle)
			continue;
		hexas.insert (ch);
	}
}

void get_adj_hexas_around_hexa (VolumeMesh *mesh, OvmCeH ch, std::unordered_set<OvmCeH> &hexas)
{
	for (auto cc_it = mesh->cc_iter (ch); cc_it; ++cc_it){
		if (*cc_it != mesh->InvalidCellHandle)
			hexas.insert (*cc_it);
	}
}

void get_adj_faces_around_hexa (VolumeMesh *mesh, OvmCeH ch, std::unordered_set<OvmFaH> &faces)
{
	auto hfhs = mesh->cell (ch).halffaces ();
	foreach (auto &hfh, hfhs){
		faces.insert (mesh->face_handle (hfh));
	}
}

void get_adj_edges_around_vertex (VolumeMesh *mesh, OvmVeH vh, std::unordered_set<OvmEgH> &edges)
{
	for (auto voh_it = mesh->voh_iter (vh); voh_it; ++voh_it){
		edges.insert (mesh->edge_handle (*voh_it));
	}
}

void get_adj_faces_around_vertex (VolumeMesh *mesh, OvmVeH vh, std::unordered_set<OvmFaH> &faces)
{
	for (auto voh_it = mesh->voh_iter (vh); voh_it; ++voh_it)
	{
		auto fhs = get_adj_faces_around_edge (mesh, *voh_it);
		foreach (OvmFaH fh, fhs)
			faces.insert (fh);
	}
}

void get_adj_boundary_faces_around_vertex (VolumeMesh *mesh, OvmVeH vh, std::unordered_set<OvmFaH> &faces)
{
	if (!mesh->is_boundary (vh))
		return;
	for (auto voh_it = mesh->voh_iter (vh); voh_it; ++voh_it)
	{
		if (!mesh->is_boundary (*voh_it))
			continue;
		auto fhs = get_adj_faces_around_edge (mesh, *voh_it);
		foreach (OvmFaH fh, fhs){
			if (mesh->is_boundary (fh))
				faces.insert (fh);
		}
	}
}

void get_adj_boundary_faces_around_edge (VolumeMesh *mesh, OvmEgH eh, std::unordered_set<OvmFaH> &faces)
{
	if (!mesh->is_boundary (eh)) return;
	auto heh = mesh->halfedge_handle (eh, 0);
	for (auto hehf_it = mesh->hehf_iter (heh); hehf_it; ++hehf_it){
		auto fh = mesh->face_handle (*hehf_it);
		if (mesh->is_boundary (fh))
			faces.insert (fh);
	}
}

void get_adj_boundary_faces_around_face (VolumeMesh *mesh, OvmFaH fh, std::unordered_set<OvmFaH> &faces)
{
	auto hfh = mesh->halfface_handle (fh, 0);
	if (!mesh->is_boundary (hfh))
		hfh = mesh->opposite_halfface_handle (hfh);
	for (auto bhfhf_it = mesh->bhfhf_iter (hfh); bhfhf_it; ++bhfhf_it)
		faces.insert (mesh->face_handle (*bhfhf_it));
}

void get_direct_adjacent_hexas (VolumeMesh *mesh, const std::unordered_set<OvmFaH> &patch, std::unordered_set<OvmCeH> &hexas)
{
	std::hash_map<OvmEgH, int> edges_counts;
	//get inner edges
	foreach (OvmFaH fh, patch){
		auto heh_vec = mesh->face (fh).halfedges ();
		foreach (OvmHaEgH heh, heh_vec){
			OvmEgH eh = mesh->edge_handle (heh);
			edges_counts[eh]++;
		}

		OvmHaFaH hfh = mesh->halfface_handle (fh, 0);
		OvmCeH ch = mesh->incident_cell (hfh);
		if (ch == mesh->InvalidCellHandle){
			hfh = mesh->opposite_halfface_handle (hfh);
			ch = mesh->incident_cell (hfh);
			assert (ch != mesh->InvalidCellHandle);
		}
		hexas.insert (ch);
	}

	for (auto it = edges_counts.begin (); it != edges_counts.end (); ++it)
	{
		//pass if the edge is on the boundary of patch
		if (it->second == 1)
			continue;
		auto heh = mesh->halfedge_handle (it->first, 0);
		for (auto hec_it = mesh->hec_iter (heh); hec_it; ++hec_it)
		{
			if (*hec_it != mesh->InvalidCellHandle)
				hexas.insert (*hec_it);
		}
	}
}

void get_cell_groups_around_vertex (VolumeMesh *mesh, OvmVeH vh, std::unordered_set<OvmFaH> &fhs,
	std::vector<std::unordered_set<OvmCeH> > &cell_groups)
{
	std::unordered_set<OvmCeH> all_cells;
	get_adj_hexas_around_vertex (mesh, vh, all_cells);

	while (!all_cells.empty ()) {
		auto ch = *(all_cells.begin ());
		std::unordered_set<OvmCeH> one_group;
		one_group.insert (ch);
		std::queue<OvmCeH> spread_set;
		spread_set.push (ch);
		while (!spread_set.empty ()){
			ch = spread_set.front ();
			spread_set.pop ();

			auto hfh_vec = mesh->cell (ch).halffaces ();
			foreach (auto &hfh, hfh_vec){
				auto fh = mesh->face_handle (hfh);
				if (contains (fhs, fh)) continue;
				auto oppo_ch = mesh->incident_cell (mesh->opposite_halfface_handle (hfh));
				if (oppo_ch == mesh->InvalidCellHandle) continue;
				if (!contains (all_cells, oppo_ch)) continue;
				if (contains (one_group, oppo_ch)) continue;

				spread_set.push (oppo_ch);
				one_group.insert (oppo_ch);
			}
		}//end while (!spread_set.empty ())...
		cell_groups.push_back (one_group);
		foreach (auto &ch, one_group)
			all_cells.erase (ch);
	}
}

void collect_boundary_element (VolumeMesh *mesh, std::set<OvmCeH> &chs, 
	std::set<OvmVeH> *bound_vhs, std::set<OvmEgH> *bound_ehs, std::set<OvmHaFaH> *bound_hfhs)
{
	std::hash_map<OvmFaH, int> fh_count;
	std::set<OvmHaFaH> all_hfhs;
	auto count_fh_on_cell = [&](OvmCeH ch){
		auto hfh_vec = mesh->cell (ch).halffaces ();
		foreach (auto &hfh, hfh_vec){
			fh_count[mesh->face_handle(hfh)]++;
			all_hfhs.insert (hfh);
		}
	};
	std::for_each (chs.begin (), chs.end (), count_fh_on_cell);

	bool newly_created_bound_hfhs = false;
	if (!bound_hfhs){
		bound_hfhs = new std::set<OvmHaFaH> ();
		newly_created_bound_hfhs = true;
	}

	auto collect_bound_hfh = [&](OvmHaFaH hfh){
		auto fh = mesh->face_handle (hfh);
		if (fh_count[fh] == 1)
			bound_hfhs->insert (hfh);
	};
	std::for_each (all_hfhs.begin (), all_hfhs.end (), collect_bound_hfh);

	if (bound_vhs || bound_ehs){
		auto collect_bound_vhs_ehs = [&](OvmHaFaH hfh){
			if (bound_vhs){
				for (auto hfv_iter = mesh->hfv_iter (hfh); hfv_iter; ++hfv_iter)
					bound_vhs->insert (*hfv_iter);
			}
			if (bound_ehs){
				auto heh_vec = mesh->halfface (hfh).halfedges ();
				foreach (auto &heh, heh_vec)
					bound_ehs->insert (mesh->edge_handle (heh));
			}
		};
		std::for_each (bound_hfhs->begin (), bound_hfhs->end (), collect_bound_vhs_ehs);
	}
	if (newly_created_bound_hfhs)
		delete bound_hfhs;
}

void collect_boundary_element (VolumeMesh *mesh, std::set<OvmCeH> &chs, 
	std::set<OvmVeH> *bound_vhs, std::set<OvmEgH> *bound_ehs, std::set<OvmFaH> *bound_fhs)
{
	std::set<OvmHaFaH> bound_hfhs;
	collect_boundary_element (mesh, chs, bound_vhs, bound_ehs, &bound_hfhs);
	if (bound_fhs){
		foreach (OvmHaFaH hfh, bound_hfhs)
			bound_fhs->insert (mesh->face_handle (hfh));
	}
}

void collect_boundary_element (VolumeMesh *mesh, std::unordered_set<OvmCeH> &chs, 
	std::unordered_set<OvmVeH> *bound_vhs, std::unordered_set<OvmEgH> *bound_ehs, std::unordered_set<OvmHaFaH> *bound_hfhs)
{
	std::hash_map<OvmFaH, int> fh_count;
	std::unordered_set<OvmHaFaH> all_hfhs;
	auto count_fh_on_cell = [&](OvmCeH ch){
		auto hfh_vec = mesh->cell (ch).halffaces ();
		foreach (auto &hfh, hfh_vec){
			fh_count[mesh->face_handle(hfh)]++;
			all_hfhs.insert (hfh);
		}
	};
	std::for_each (chs.begin (), chs.end (), count_fh_on_cell);

	bool newly_created_bound_hfhs = false;
	if (!bound_hfhs){
		bound_hfhs = new std::unordered_set<OvmHaFaH> ();
		newly_created_bound_hfhs = true;
	}

	auto collect_bound_hfh = [&](OvmHaFaH hfh){
		auto fh = mesh->face_handle (hfh);
		if (fh_count[fh] == 1)
			bound_hfhs->insert (hfh);
	};
	std::for_each (all_hfhs.begin (), all_hfhs.end (), collect_bound_hfh);

	if (bound_vhs || bound_ehs){
		auto collect_bound_vhs_ehs = [&](OvmHaFaH hfh){
			if (bound_vhs){
				for (auto hfv_iter = mesh->hfv_iter (hfh); hfv_iter; ++hfv_iter)
					bound_vhs->insert (*hfv_iter);
			}
			if (bound_ehs){
				auto heh_vec = mesh->halfface (hfh).halfedges ();
				foreach (auto &heh, heh_vec)
					bound_ehs->insert (mesh->edge_handle (heh));
			}
		};
		std::for_each (bound_hfhs->begin (), bound_hfhs->end (), collect_bound_vhs_ehs);
	}
	if (newly_created_bound_hfhs)
		delete bound_hfhs;
}

void collect_boundary_element (VolumeMesh *mesh, std::unordered_set<OvmCeH> &chs, 
	std::unordered_set<OvmVeH> *bound_vhs, std::unordered_set<OvmEgH> *bound_ehs, std::unordered_set<OvmFaH> *bound_fhs)
{
	//std::unordered_set<OvmHaFaH> bound_hfhs;
	//collect_boundary_element (mesh, chs, bound_vhs, bound_ehs, &bound_hfhs);
	//if (bound_fhs){
	//	foreach (OvmHaFaH hfh, bound_hfhs)
	//		bound_fhs->insert (mesh->face_handle (hfh));
	//}
	std::unordered_set<OvmFaH>().swap(*bound_fhs);
	std::vector<int> tag_fh(mesh->n_faces(),0);
	for (auto cit = chs.begin();cit != chs.end();cit++)
	{
		auto adj_hfhs = mesh->cell(*cit).halffaces();
		for (auto hfh_it = adj_hfhs.begin();hfh_it != adj_hfhs.end();hfh_it++)
		{
			OvmFaH current_fh = mesh->face_handle(*hfh_it);
			tag_fh[current_fh.idx()] = tag_fh[current_fh.idx()]++;
		}
	}
	for (int fi = 0;fi < mesh->n_faces();fi++)
	{
		if (tag_fh[fi] == 1)
		{
			bound_fhs->insert((OvmFaH)fi);
		}
	}
	//std::unordered_set<OvmFaH>().swap(*bound_fhs);
	//for (auto cit = chs.begin();cit != chs.end();cit++)
	//{
	//	auto adj_hfhs = mesh->cell(*cit).halffaces();
	//	for (auto hfh_it = adj_hfhs.begin();hfh_it != adj_hfhs.end();hfh_it++)
	//	{
	//		OvmFaH current_fh = mesh->face_handle(*hfh_it);
	//		if (bound_fhs->find(current_fh) == bound_fhs->end())
	//		{
	//			bound_fhs->insert(current_fh);
	//		}
	//		else
	//		{
	//			bound_fhs->erase(current_fh);
	//		}
	//	}
	//}
}

void collect_boundary_element (VolumeMesh *mesh, std::unordered_set<OvmHaFaH> &hfhs, 
	std::vector<OvmVeH> & bound_vhs, std::vector<std::vector<OvmHaEgH>> & bound_hehs)
{
	std::unordered_set<OvmHaEgH> heh_set;
	foreach(auto & hfh, hfhs){
		std::vector<OvmHaEgH> heh_vec(mesh->halfface(hfh).halfedges());
		foreach(auto & heh, heh_vec)
			heh_set.insert(heh);
	}
		
	std::unordered_set<OvmHaEgH> bound_heh_set;
	foreach(auto & heh, heh_set){
		if(contains(heh_set,mesh->opposite_halfedge_handle(heh)))
			continue;
		else
			bound_heh_set.insert(heh);
	}

	while(!bound_heh_set.empty()){
		OvmHaEgH heh = *(bound_heh_set.begin());
		std::vector<OvmHaEgH> hehs_temp;
		hehs_temp.push_back(heh);
		bound_heh_set.erase(heh);
		bool is_terminal = false;
		while(!bound_heh_set.empty()){
			is_terminal = false;
			OvmHaEgH heh = hehs_temp[hehs_temp.size()-1];
			OvmVeH vh = mesh->halfedge (heh).to_vertex();
			for(auto heh_iter = bound_heh_set.begin(); heh_iter != bound_heh_set.end(); ++heh_iter){
				if(mesh->halfedge(*heh_iter).from_vertex() == vh){
					is_terminal = true;
					hehs_temp.push_back(*heh_iter);
					bound_heh_set.erase(*heh_iter);
					break;
				}
			}
			if(!is_terminal)
				break;
		}
		bound_hehs.push_back(hehs_temp);
	}

	foreach(auto & hehs_vec, bound_hehs)
		foreach(auto &heh, hehs_vec)
			bound_vhs.push_back(mesh->halfedge(heh).from_vertex());
}

void collect_boundary_element (VolumeMesh *mesh, std::unordered_set<OvmHaFaH> &hfhs, 
	std::vector<OvmVeH> & bound_vhs, std::vector<OvmHaEgH> & bound_hehs)
{
	std::unordered_set<OvmHaEgH> heh_set;
	foreach(auto & hfh, hfhs){
		std::vector<OvmHaEgH> heh_vec(mesh->halfface(hfh).halfedges());
		foreach(auto & heh, heh_vec)
			heh_set.insert(heh);
	}

	std::unordered_set<OvmHaEgH> bound_heh_set;
	foreach(auto & heh, heh_set){
		if(contains(heh_set,mesh->opposite_halfedge_handle(heh)))
			continue;
		else
			bound_heh_set.insert(heh);
	}

	OvmHaEgH heh = *(bound_heh_set.begin());
	bound_hehs.push_back(heh);
	bound_heh_set.erase(heh);
	while(!bound_heh_set.empty()){
		OvmHaEgH heh = bound_hehs[bound_hehs.size()-1];
		OvmVeH vh = mesh->halfedge (heh).to_vertex();
		for(auto heh_iter = bound_heh_set.begin(); heh_iter != bound_heh_set.end(); ++heh_iter){
			if(mesh->halfedge(*heh_iter).from_vertex() == vh){
				bound_hehs.push_back(*heh_iter);
				bound_heh_set.erase(*heh_iter);
				break;
			}
		}
	}

	foreach(auto & heh, bound_hehs)
		bound_vhs.push_back(mesh->halfedge(heh).from_vertex());
}

void collect_boundary_element (VolumeMesh *mesh, std::unordered_set<OvmFaH> &fhs, 
	std::unordered_set<OvmVeH> &bound_vhs, std::unordered_set<OvmEgH> &bound_ehs)
{
	std::unordered_set<OvmEgH> eh_set;
	foreach(auto & fh, fhs){
		get_adj_edges_around_face(mesh, fh, eh_set);
	}

	foreach(auto & eh, eh_set){
		std::unordered_set<OvmFaH> adj_fhs;
		get_adj_faces_around_edge(mesh, eh, adj_fhs);
		int count(0);
		foreach(auto &fh, adj_fhs)
			if(contains(fhs, fh))
				count++;
		if(count == 1)
			bound_ehs.insert(eh);
	}

	foreach(auto & eh, bound_ehs){
		bound_vhs.insert(mesh->edge(eh).from_vertex());
		bound_vhs.insert(mesh->edge(eh).to_vertex());
	}
}

void get_ccw_boundary_edges_faces_around_vertex (VolumeMesh *mesh, OvmVeH vh,
	std::vector<OvmEgH> &ehs, std::vector<OvmFaH> &fhs)
{
	OvmHaEgH start_heh = mesh->InvalidHalfEdgeHandle;
	for (auto voh_it = mesh->voh_iter (vh); voh_it; ++voh_it)
	{
		if (mesh->is_boundary (*voh_it)){
			start_heh = *voh_it;
			break;
		}
	}
	assert (start_heh != mesh->InvalidHalfEdgeHandle);

	auto fGetPrevBoundHehAndFh = [&mesh](OvmHaEgH heh)->std::pair<OvmHaEgH, OvmFaH>{
		OvmHaFaH bound_hfh = mesh->InvalidHalfFaceHandle;
		for (auto hehf_it = mesh->hehf_iter (heh); hehf_it; ++hehf_it){
			if (mesh->is_boundary (*hehf_it)){
				bound_hfh = *hehf_it;
				break;
			}
		}
		assert (bound_hfh != mesh->InvalidHalfFaceHandle);
		std::pair<OvmHaEgH, OvmFaH> p;
		p.first = mesh->opposite_halfedge_handle (mesh->prev_halfedge_in_halfface (heh, bound_hfh));
		p.second = mesh->face_handle (bound_hfh);
		return p;
	};

	ehs.push_back (mesh->edge_handle (start_heh));
	auto p = fGetPrevBoundHehAndFh (start_heh);
	fhs.push_back (p.second);
	while (p.first != start_heh){
		ehs.push_back (mesh->edge_handle (p.first));
		p = fGetPrevBoundHehAndFh (p.first);
		fhs.push_back (p.second);
	}
}

void get_ccw_boundary_edges_faces_around_vertex (VolumeMesh *mesh, OvmVeH vh,
	std::unordered_set<OvmEgH> &key_ehs, std::vector<OvmEgH> &ehs, std::vector<std::unordered_set<OvmFaH> > &fhs_sets)
{
	std::vector<OvmFaH> fhs;
	fhs_sets.resize (key_ehs.size ());

	std::vector<OvmEgH> boundary_ehs;
	get_ccw_boundary_edges_faces_around_vertex (mesh, vh, boundary_ehs, fhs);
	while (!contains (key_ehs, boundary_ehs.front ())){
		boundary_ehs.push_back (boundary_ehs.front ());
		boundary_ehs.erase (boundary_ehs.begin ());

		fhs.push_back (fhs.front ());
		fhs.erase (fhs.begin ());
	}
	ehs.push_back (boundary_ehs.front ());
	int idx = 0;
	for (int i = 0; i != boundary_ehs.size (); ++i){
		fhs_sets[idx].insert (fhs[i]);
		if ((i + 1) == boundary_ehs.size ())
			break;
		if (std::find (key_ehs.begin (), key_ehs.end (), boundary_ehs[i + 1]) != key_ehs.end ()){
			ehs.push_back (boundary_ehs[i + 1]);
			idx++;
		}
	}
}

void get_ccw_boundary_edges_faces_around_vertex (VolumeMesh *mesh, OvmVeH vh,
	std::vector<OvmEgH> &key_ehs, std::vector<OvmEgH> &ehs, std::vector<std::unordered_set<OvmFaH> > &fhs_sets)
{
	std::unordered_set<OvmEgH> key_ehs_set;
	vector_to_unordered_set (key_ehs, key_ehs_set);
	get_ccw_boundary_edges_faces_around_vertex (mesh, vh, key_ehs_set, ehs, fhs_sets);
}

void get_cw_boundary_edges_faces_around_vertex (VolumeMesh *mesh, OvmVeH vh,
	std::vector<OvmEgH> &ehs, std::vector<OvmFaH> &fhs)
{
	get_ccw_boundary_edges_faces_around_vertex (mesh, vh, ehs, fhs);
	std::reverse (ehs.begin () + 1, ehs.end ());
	std::reverse (fhs.begin (), fhs.end ());
}

void get_cw_boundary_edges_faces_around_vertex (VolumeMesh *mesh, OvmVeH vh, 
	std::unordered_set<OvmEgH> &key_ehs, std::vector<OvmEgH> &ehs, std::vector<std::unordered_set<OvmFaH> > &fhs_sets)
{
	std::vector<OvmFaH> fhs;
	fhs_sets.resize (key_ehs.size ());

	std::vector<OvmEgH> boundary_ehs;
	get_cw_boundary_edges_faces_around_vertex (mesh, vh, boundary_ehs, fhs);
	while (!contains (key_ehs, boundary_ehs.front ())){
		boundary_ehs.push_back (boundary_ehs.front ());
		boundary_ehs.erase (boundary_ehs.begin ());

		fhs.push_back (fhs.front ());
		fhs.erase (fhs.begin ());
	}
	ehs.push_back (boundary_ehs.front ());
	int idx = 0;
	for (int i = 0; i != boundary_ehs.size (); ++i){
		fhs_sets[idx].insert (fhs[i]);
		if ((i + 1) == boundary_ehs.size ())
			break;
		if (std::find (key_ehs.begin (), key_ehs.end (), boundary_ehs[i + 1]) != key_ehs.end ()){
			ehs.push_back (boundary_ehs[i + 1]);
			idx++;
		}
	}
}

void get_cw_boundary_edges_faces_around_vertex (VolumeMesh *mesh, OvmVeH vh, 
	std::vector<OvmEgH> &key_ehs, std::vector<OvmEgH> &ehs, std::vector<std::unordered_set<OvmFaH> > &fhs_sets)
{
	std::unordered_set<OvmEgH> key_ehs_set;
	vector_to_unordered_set (key_ehs, key_ehs_set);
	get_cw_boundary_edges_faces_around_vertex (mesh, vh, key_ehs_set, ehs, fhs_sets);
}

OvmVeH get_other_vertex_on_edge (VolumeMesh *mesh, OvmEgH eh, OvmVeH vh)
{
	OvmVeH vh1 = mesh->edge (eh).from_vertex (),
		vh2 = mesh->edge (eh).to_vertex ();
	if (vh1 == vh) return vh2;
	else if (vh2 == vh) return vh1;
	else return mesh->InvalidVertexHandle;
}

OvmEgH get_opposite_edge_on_face (VolumeMesh *mesh, OvmFaH fh, OvmEgH eh)
{
	auto heh_vec = mesh->face (fh).halfedges ();
	std::vector<OvmEgH> ehs;
	foreach (OvmHaEgH heh, heh_vec)
		ehs.push_back (mesh->edge_handle (heh));

	if (!contains (ehs, eh)) return mesh->InvalidEdgeHandle;
	while (true){
		if (ehs.front () == eh) break;
		auto front_eh = ehs.front ();
		ehs.erase (ehs.begin ());
		ehs.push_back (front_eh);
	}

	return ehs[2];
}

OvmCeH get_common_cell_handle (VolumeMesh *mesh, OvmFaH & fh1, OvmFaH &fh2)
{
	OvmCeH com_ch = mesh->InvalidCellHandle;
	std::set<OvmCeH> chs_set1, chs_set2;
	OvmCeH ch_temp = mesh->incident_cell(mesh->halfface_handle(fh1, 0));
	if(ch_temp != mesh->InvalidCellHandle)
		chs_set1.insert(ch_temp);
	ch_temp = mesh->incident_cell(mesh->halfface_handle(fh1, 1));
	if(ch_temp != mesh->InvalidCellHandle)
		chs_set1.insert(ch_temp);
	ch_temp = mesh->incident_cell(mesh->halfface_handle(fh2, 0));
	if(ch_temp != mesh->InvalidCellHandle)
		chs_set2.insert(ch_temp);
	ch_temp = mesh->incident_cell(mesh->halfface_handle(fh2, 1));
	if(ch_temp != mesh->InvalidCellHandle)
		chs_set2.insert(ch_temp);

	std::vector<OvmCeH> com_chs;
	std::set_intersection (chs_set1.begin (), chs_set1.end (), chs_set2.begin (), chs_set2.end (),
		std::back_inserter (com_chs));
	if (com_chs.size () == 1)
		com_ch = com_chs.front ();
	return com_ch;
}

OvmFaH get_common_face_handle (VolumeMesh *mesh, OvmCeH &ch1, OvmCeH &ch2)
{
	OvmFaH com_fh = mesh->InvalidFaceHandle;
	auto hfhs_vec1 = mesh->cell (ch1).halffaces (),
		hfhs_vec2 = mesh->cell (ch2).halffaces ();

	std::set<OvmFaH> fhs_set1, fhs_set2;
	std::for_each (hfhs_vec1.begin (), hfhs_vec1.end (), [&](OvmHaFaH hfh){fhs_set1.insert (mesh->face_handle (hfh));});
	std::for_each (hfhs_vec2.begin (), hfhs_vec2.end (), [&](OvmHaFaH hfh){fhs_set2.insert (mesh->face_handle (hfh));});

	std::vector<OvmFaH> com_fhs;
	std::set_intersection (fhs_set1.begin (), fhs_set1.end (), fhs_set2.begin (), fhs_set2.end (),
		std::back_inserter (com_fhs));
	if (com_fhs.size () == 1)
		com_fh = com_fhs.front ();
	return com_fh;
}

OvmFaH get_common_face_handle_gen_ver (VMesh *mesh, OvmCeH &ch1, OvmCeH &ch2)
{
	OvmFaH com_fh = mesh->InvalidFaceHandle;
	auto hfhs_vec1 = mesh->cell (ch1).halffaces (),
		hfhs_vec2 = mesh->cell (ch2).halffaces ();

	std::set<OvmFaH> fhs_set1, fhs_set2;
	std::for_each (hfhs_vec1.begin (), hfhs_vec1.end (), [&](OvmHaFaH hfh){fhs_set1.insert (mesh->face_handle (hfh));});
	std::for_each (hfhs_vec2.begin (), hfhs_vec2.end (), [&](OvmHaFaH hfh){fhs_set2.insert (mesh->face_handle (hfh));});

	std::vector<OvmFaH> com_fhs;
	std::set_intersection (fhs_set1.begin (), fhs_set1.end (), fhs_set2.begin (), fhs_set2.end (),
		std::back_inserter (com_fhs));
	if (com_fhs.size () == 1)
		com_fh = com_fhs.front ();
	return com_fh;
}

OvmFaH get_common_face_handle (VolumeMesh *mesh, OvmEgH &eh1, OvmEgH &eh2)
{
	OvmFaH com_fh = mesh->InvalidFaceHandle;
	std::set<OvmFaH> fhs_set1, fhs_set2;
	for(auto hehf_iter = mesh->hehf_iter(mesh->halfedge_handle(eh1, 0)); hehf_iter; ++hehf_iter)
		fhs_set1.insert(mesh->face_handle(*hehf_iter));
	for(auto hehf_iter = mesh->hehf_iter(mesh->halfedge_handle(eh1, 1)); hehf_iter; ++hehf_iter)
		fhs_set1.insert(mesh->face_handle(*hehf_iter));
	for(auto hehf_iter = mesh->hehf_iter(mesh->halfedge_handle(eh2, 0)); hehf_iter; ++hehf_iter)
		fhs_set2.insert(mesh->face_handle(*hehf_iter));
	for(auto hehf_iter = mesh->hehf_iter(mesh->halfedge_handle(eh2, 1)); hehf_iter; ++hehf_iter)
		fhs_set2.insert(mesh->face_handle(*hehf_iter));
	std::vector<OvmFaH> com_fhs;
	std::set_intersection (fhs_set1.begin (), fhs_set1.end (), fhs_set2.begin (), fhs_set2.end (),
		std::back_inserter (com_fhs));
	if (com_fhs.size () == 1)
		com_fh = com_fhs.front ();
	return com_fh;
}


OvmVeH get_common_vertex_handle (VolumeMesh *mesh, OvmEgH eh1, OvmEgH eh2)
{
	OvmVeH v11 = mesh->edge (eh1).from_vertex (), v12 = mesh->edge (eh1).to_vertex ();
	OvmVeH v21 = mesh->edge (eh2).from_vertex (), v22 = mesh->edge (eh2).to_vertex ();
	if (v11 == v21 || v11 == v22)
		return v11;
	if (v12 == v21 || v12 == v22)
		return v12;
	return mesh->InvalidVertexHandle;
}

OvmVeH get_common_vertex_handle_gen_ver (VMesh *mesh, OvmEgH eh1, OvmEgH eh2)
{
	OvmVeH v11 = mesh->edge (eh1).from_vertex (), v12 = mesh->edge (eh1).to_vertex ();
	OvmVeH v21 = mesh->edge (eh2).from_vertex (), v22 = mesh->edge (eh2).to_vertex ();
	if (v11 == v21 || v11 == v22)
		return v11;
	if (v12 == v21 || v12 == v22)
		return v12;
	return mesh->InvalidVertexHandle;
}

OvmEgH get_common_edge_handle (VolumeMesh *mesh, OvmFaH fh1, OvmFaH fh2)
{
	auto heh1_vec = mesh->face (fh1).halfedges (),
		heh2_vec = mesh->face (fh2).halfedges ();
	foreach (auto &heh1, heh1_vec){
		auto eh1 = mesh->edge_handle (heh1);
		foreach (auto &heh2, heh2_vec){
			auto eh2 = mesh->edge_handle (heh2);
			if (eh1 == eh2) return eh1;
		}
	}
	return mesh->InvalidEdgeHandle;
}

OvmEgH get_common_tet_edge_handle(VMesh *mesh, OvmFaH fh1, OvmFaH fh2)
{
	auto hehs1 = mesh->face(fh1).halfedges();
	auto hehs2 = mesh->face(fh2).halfedges();
	foreach (OvmHaEgH heh1, hehs1)
	{
		OvmEgH eh1 = mesh->edge_handle(heh1);
		foreach (OvmHaEgH heh2,hehs2)
		{
			OvmEgH eh2 = mesh->edge_handle(heh2);
			if (eh1 == eh2) return eh1;
		}
	}
	return mesh->InvalidEdgeHandle;
}

OvmVec3d get_tet_center(VMesh* mesh,OvmCeH ch)
{
	OvmVec3d cen_pos(0,0,0);
	for (auto cv_it = mesh->cv_iter(ch);cv_it;cv_it++)
		cen_pos += mesh->vertex(*cv_it)/4.0;
	return cen_pos;
}

bool is_manifold (VolumeMesh *mesh, std::unordered_set<OvmFaH> &fhs)
{
	std::hash_map<OvmEgH, int> edge_count_mapping;
	std::unordered_set<OvmVeH> vhs;
	//std::set<OvmVeH> all_vhs;
	foreach (OvmFaH fh, fhs){
		auto heh_vec = mesh->face (fh).halfedges ();
		foreach (OvmHaEgH heh, heh_vec){
			edge_count_mapping[mesh->edge_handle (heh)]++;
			vhs.insert (mesh->halfedge (heh).to_vertex ());
		}
	}

	std::ofstream fout;
	fout.open("nonmanifold_edges.txt");

	//对于sheet生成的流形面集来说，不应出现自相交次数多于两次的情况
	bool is = true;
	foreach (auto &p, edge_count_mapping){
		if (p.second == 0 || p.second >= 3){
			fout<<p.first<<":"<<p.second;
			std::unordered_set<OvmFaH> fhs_temp;
			get_adj_faces_around_edge(mesh, mesh->halfedge_handle(p.first, 0), fhs_temp);
			foreach(auto &fh, fhs_temp)
				if(contains(fhs, fh)){
					fout<<"  "<<fh;
					if(mesh->is_boundary(fh))
						fout<<"("<<1<<")";
					else
						fout<<"("<<0<<")";
				}
			fout<<std::endl;
			is = false;
		}
	}
	fout.close();
	if(!is)
	    return false;
	//对面的连通性做检查
	std::queue<OvmFaH> fh_spread_set;
	fh_spread_set.push (*(fhs.begin ()));
	std::unordered_set<OvmFaH> tmp_fhs = fhs;
	tmp_fhs.erase (tmp_fhs.begin ());
	while (!fh_spread_set.empty ()){
		OvmFaH front_fh = fh_spread_set.front ();
		fh_spread_set.pop ();
		auto heh_vec = mesh->face (front_fh).halfedges ();
		foreach (OvmHaEgH test_heh, heh_vec){
			for (auto hehf_it = mesh->hehf_iter (test_heh); hehf_it; ++hehf_it){
				OvmFaH test_adj_fh = mesh->face_handle (*hehf_it);
				if (test_adj_fh == mesh->InvalidFaceHandle || test_adj_fh == front_fh)
					continue;
				auto locate = tmp_fhs.find (test_adj_fh);
				if (locate == tmp_fhs.end ())
					continue;
				tmp_fhs.erase (locate);
				fh_spread_set.push (test_adj_fh);
			}
		}//end foreach (OvmHaEgH test_heh, heh_vec){...
	}//end while (!fh_spread_set.empty ()){...

	//如果tmp_fhs里面还有剩余的面，则说明fhs里面的面并不都是通过边相邻的
	if (!tmp_fhs.empty ())
		return false;

	//对边做检查，如果一条边

	//下面对于点再做进一步的检查
	//对于流形体，每一个点所相邻的四边形都是通过边连接在一起的
	//如果不能够通过边相邻遍历完所有的面，则说明该点不是流形体上的点
	auto get_adj_faces_around_vertex_with_limitation = 
		[&mesh] (OvmVeH vh, std::unordered_set<OvmFaH> &limited_fhs)->std::unordered_set<OvmFaH>{
			std::unordered_set<OvmFaH> adj_fhs;
			for (auto voh_it = mesh->voh_iter (vh); voh_it; ++voh_it){
				for (auto hehf_it = mesh->hehf_iter (*voh_it); hehf_it; ++hehf_it){
					OvmFaH fh = mesh->face_handle (*hehf_it);
					if (fh != mesh->InvalidFaceHandle && contains (limited_fhs, fh))
						adj_fhs.insert (fh);
				}
			}
			return adj_fhs;
	};

	foreach (OvmVeH vh, vhs){
		auto adj_fhs = get_adj_faces_around_vertex_with_limitation (vh, fhs);
		std::queue<OvmFaH> spread_set;
		spread_set.push (*(adj_fhs.begin ()));
		while (!spread_set.empty ()){
			OvmFaH front_fh = spread_set.front ();
			spread_set.pop ();
			auto heh_vec = mesh->face (front_fh).halfedges ();
			foreach (OvmHaEgH heh, heh_vec){
				for (auto hehf_it = mesh->hehf_iter (heh); hehf_it; ++hehf_it){
					OvmFaH test_fh = mesh->face_handle (*hehf_it);
					if (test_fh == mesh->InvalidFaceHandle)	continue;
					if (adj_fhs.find (test_fh) == adj_fhs.end ()) continue;
					spread_set.push (test_fh); adj_fhs.erase (test_fh);
				}
			}//end foreach (OvmHaEgH heh, heh_vec){...
		}//end while (!spread_set.empty ()){...

		if (!adj_fhs.empty ()) return false;
	}//end foreach (OvmVeH vh, all_vhs){...
	return true;
}


bool is_manifold (VolumeMesh *mesh, std::unordered_set<OvmCeH> &chs)
{
	std::unordered_set<OvmVeH> cells_set_bound_vhs, tmp_bound_vhs;
	std::unordered_set<OvmEgH> cells_set_bound_ehs, tmp_bound_ehs;
	std::unordered_set<OvmFaH> cells_set_bound_fhs, tmp_bound_fhs;
	collect_boundary_element (mesh, chs, &tmp_bound_vhs, &tmp_bound_ehs, &tmp_bound_fhs);
	foreach (auto &vh, tmp_bound_vhs){
		if (!mesh->is_boundary (vh))
			cells_set_bound_vhs.insert (vh);
	}
	foreach (auto &fh, tmp_bound_fhs){
		if (!mesh->is_boundary (fh))
			cells_set_bound_fhs.insert (fh);
	}
	foreach (auto &eh, tmp_bound_ehs){
		if (!mesh->is_boundary (eh))
			cells_set_bound_ehs.insert (eh);
	}

	auto fCellsConnected = [&] (std::unordered_set<OvmCeH> chs)->bool{
		std::queue<OvmCeH> spread_set;
		auto start_ch = pop_begin_element (chs);
		spread_set.push (start_ch);
		while (!spread_set.empty ()){
			auto curr_ch = spread_set.front ();
			spread_set.pop ();
			for (auto cc_it = mesh->cc_iter (curr_ch); cc_it; ++cc_it){
				auto test_ch = *cc_it;
				if (test_ch == mesh->InvalidCellHandle) continue;
				if (!contains (chs, test_ch)) continue;
				chs.erase (test_ch);
				spread_set.push (test_ch);
			}
		}
		return chs.empty ();
	};

	foreach (OvmVeH vh, cells_set_bound_vhs){
		std::unordered_set<OvmCeH> adj_chs;
		get_adj_hexas_around_vertex (mesh, vh, adj_chs);

		std::unordered_set<OvmCeH> adj_chs_of_vh_in_hs;
		foreach (OvmCeH ch, adj_chs){
			if (contains (chs, ch)) adj_chs_of_vh_in_hs.insert (ch);
		}

		if (!fCellsConnected (adj_chs_of_vh_in_hs)) return false;
	}

	foreach (OvmEgH eh, cells_set_bound_ehs){
		std::unordered_set<OvmCeH> adj_chs;
		get_adj_hexas_around_edge (mesh, eh, adj_chs);

		std::unordered_set<OvmCeH> adj_chs_of_vh_in_hs;
		foreach (OvmCeH ch, adj_chs){
			if (contains (chs, ch)) adj_chs_of_vh_in_hs.insert (ch);
		}

		if (!fCellsConnected (adj_chs_of_vh_in_hs)) return false;
	}
	return true;
}

void get_fhs_on_acis_face (VolumeMesh *mesh, FACE *acis_face, std::unordered_set<OvmFaH> &fhs)
{
	assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

	fhs.clear ();

	for (auto f_it = mesh->faces_begin(); f_it != mesh->faces_end(); f_it++){
		auto f = get_associated_geometry_face_of_boundary_fh (mesh, *f_it);
		if (f == acis_face) fhs.insert (*f_it);
	}
}

FACE* get_associated_geometry_face_of_boundary_fh (VolumeMesh *mesh, OvmFaH fh)
{
	if (!mesh->vertex_property_exists<unsigned long> ("entityptr"))
		return NULL;
	if (!mesh->is_boundary (fh))
		return NULL;
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long> ("entityptr");
	auto adj_vhs = get_adj_vertices_around_face (mesh, fh);
	std::vector<std::set<FACE*> > all_adj_faces;
	foreach (auto vh, adj_vhs){
		ENTITY *entity = (ENTITY*)(V_ENTITY_PTR[vh]);
		assert (entity);
		if (is_FACE (entity))
			return (FACE*)entity;
		ENTITY_LIST face_list;
		api_get_faces (entity, face_list);
		std::set<FACE*> cur_adj_faces;
		for (int i = 0; i != face_list.count (); ++i){
			cur_adj_faces.insert ((FACE*)(face_list[i]));
		}
		all_adj_faces.push_back (cur_adj_faces);
	}
	assert (all_adj_faces.size () > 1);
	foreach (FACE *f, all_adj_faces.front ()){
		bool this_ok = true;
		for (int i = 1; i != all_adj_faces.size (); ++i){
			if (!contains (all_adj_faces[i], f)){
				this_ok = false;
				break;
			}
		}
		if (this_ok) return f;
	}
	return NULL;
}

FACE* get_associated_geometry_face_of_boundary_fh_gen_ver (VMesh *mesh, OvmFaH fh)
{
	if (!mesh->vertex_property_exists<unsigned long> ("entityptr"))
		return NULL;
	if (!mesh->is_boundary (fh))
		return NULL;
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long> ("entityptr");
	//auto adj_vhs = get_adj_vertices_around_face (mesh, fh);
	std::vector<OvmVeH> adj_vhs;
	for (auto fv_it = mesh->hfv_iter(mesh->halfface_handle(fh,0));fv_it;fv_it++)
		adj_vhs.push_back(*fv_it);
	std::vector<std::set<FACE*> > all_adj_faces;
	foreach (auto vh, adj_vhs){
		ENTITY *entity = (ENTITY*)(V_ENTITY_PTR[vh]);
		assert (entity);
		if (is_FACE (entity))
			return (FACE*)entity;
		ENTITY_LIST face_list;
		api_get_faces (entity, face_list);
		std::set<FACE*> cur_adj_faces;
		for (int i = 0; i != face_list.count (); ++i){
			cur_adj_faces.insert ((FACE*)(face_list[i]));
		}
		all_adj_faces.push_back (cur_adj_faces);
	}
	assert (all_adj_faces.size () > 1);
	//if (fh.idx() == 63118)
	//{
	//	std::cout<<"test geometry"<<std::endl;
	//}
	foreach (FACE *f, all_adj_faces.front ()){
		bool this_ok = true;
		for (int i = 1; i != all_adj_faces.size (); ++i){
			if (!contains (all_adj_faces[i], f)){
				this_ok = false;
				break;
			}
		}
		if (this_ok) return f;
	}
	//if (fh.idx() == 63118)
	//{
	//	for (int vi = 0;vi < all_adj_faces.size();vi++)
	//	{
	//		std::cout<<adj_vhs[vi]<<": ";
	//		if (adj_vhs[vi] == 878)
	//		{
	//			ENTITY *entity_ = (ENTITY*)(V_ENTITY_PTR[adj_vhs[vi]]);
	//			ENTITY_LIST adjfaces;
	//			api_get_faces(entity_,adjfaces);
	//			std::cout<<"Face size "<<adjfaces.count()<<" and  "<<is_VERTEX(entity_)<<" ";
	//		}
	//		for (auto fp = all_adj_faces[vi].begin();fp != all_adj_faces[vi].end();fp++)
	//		{
	//			std::cout<<*fp<<" ";
	//		}
	//		std::cout<<std::endl;
	//	}
	//}
	return NULL;
}


EDGE* get_associated_geometry_edge_of_boundary_eh (VolumeMesh *mesh, OvmEgH eh, OpenVolumeMesh::VertexPropertyT<unsigned long> &V_ENTITY_PTR)
{
	auto vh1 = mesh->edge (eh).from_vertex (),
		vh2 = mesh->edge (eh).to_vertex ();
	ENTITY *entity1 = (ENTITY*)(V_ENTITY_PTR[vh1]),
		*entity2 = (ENTITY*)(V_ENTITY_PTR[vh2]);

	if (entity1 == NULL || entity2 == NULL) return NULL;
	if (is_FACE (entity1) || is_FACE (entity2)) return NULL;

	if (is_EDGE (entity1)){
		if (is_EDGE (entity2)){
			if (entity1 == entity2) return (EDGE*)entity1;
			else return NULL;
		}else if (is_VERTEX (entity2)){
			ENTITY_LIST adj_edges_list;
			api_get_edges (entity2, adj_edges_list);
			if (adj_edges_list.lookup (entity1) != -1) return (EDGE*)entity1;
			else return NULL;
		}
	}else if (is_VERTEX (entity1)){
		ENTITY_LIST adj_edges_list1;
		api_get_edges (entity1, adj_edges_list1);
		if (is_EDGE (entity2)){
			if (adj_edges_list1.lookup (entity2) != -1) return (EDGE*)entity2;
			else return NULL;
		}else if (is_VERTEX (entity2)){
			ENTITY_LIST adj_edges_list2;
			api_get_edges (entity2, adj_edges_list2);
			for (int i = 0; i != adj_edges_list1.count (); ++i){
				for (int j = 0; j != adj_edges_list2.count (); ++j){
					if (adj_edges_list1[i] == adj_edges_list2[j])
						return (EDGE*)(adj_edges_list1[i]);
				}
			}
		}
	}
	return NULL;
}

EDGE* get_associated_geometry_edge_of_boundary_eh (VolumeMesh *mesh, OvmEgH eh)
{
	assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

	auto vh1 = mesh->edge (eh).from_vertex (),
		vh2 = mesh->edge (eh).to_vertex ();
	ENTITY *entity1 = (ENTITY*)(V_ENTITY_PTR[vh1]),
		*entity2 = (ENTITY*)(V_ENTITY_PTR[vh2]);

	if (entity1 == NULL || entity2 == NULL) return NULL;
	if (is_FACE (entity1) || is_FACE (entity2)) return NULL;

	if (is_EDGE (entity1)){
		if (is_EDGE (entity2)){
			if (entity1 == entity2) return (EDGE*)entity1;
			else return NULL;
		}else if (is_VERTEX (entity2)){
			ENTITY_LIST adj_edges_list;
			api_get_edges (entity2, adj_edges_list);
			if (adj_edges_list.lookup (entity1) != -1) return (EDGE*)entity1;
			else return NULL;
		}
	}else if (is_VERTEX (entity1)){
		ENTITY_LIST adj_edges_list1;
		api_get_edges (entity1, adj_edges_list1);
		if (is_EDGE (entity2)){
			if (adj_edges_list1.lookup (entity2) != -1) return (EDGE*)entity2;
			else return NULL;
		}else if (is_VERTEX (entity2)){
			ENTITY_LIST adj_edges_list2;
			api_get_edges (entity2, adj_edges_list2);
			for (int i = 0; i != adj_edges_list1.count (); ++i){
				for (int j = 0; j != adj_edges_list2.count (); ++j){
					if (adj_edges_list1[i] == adj_edges_list2[j])
						return (EDGE*)(adj_edges_list1[i]);
				}
			}
		}
	}
	return NULL;
}

EDGE* get_associated_geometry_edge_of_boundary_eh_gen_ver (VMesh *mesh, OvmEgH eh, OpenVolumeMesh::VertexPropertyT<unsigned long> &V_ENTITY_PTR)
{
	auto vh1 = mesh->edge (eh).from_vertex (),
		vh2 = mesh->edge (eh).to_vertex ();
	ENTITY *entity1 = (ENTITY*)(V_ENTITY_PTR[vh1]),
		*entity2 = (ENTITY*)(V_ENTITY_PTR[vh2]);

	if (entity1 == NULL || entity2 == NULL) return NULL;
	if (is_FACE (entity1) || is_FACE (entity2)) return NULL;

	if (is_EDGE (entity1)){
		if (is_EDGE (entity2)){
			if (entity1 == entity2) return (EDGE*)entity1;
			else return NULL;
		}else if (is_VERTEX (entity2)){
			ENTITY_LIST adj_edges_list;
			api_get_edges (entity2, adj_edges_list);
			if (adj_edges_list.lookup (entity1) != -1) return (EDGE*)entity1;
			else return NULL;
		}
	}else if (is_VERTEX (entity1)){
		ENTITY_LIST adj_edges_list1;
		api_get_edges (entity1, adj_edges_list1);
		if (is_EDGE (entity2)){
			if (adj_edges_list1.lookup (entity2) != -1) return (EDGE*)entity2;
			else return NULL;
		}else if (is_VERTEX (entity2)){
			ENTITY_LIST adj_edges_list2;
			api_get_edges (entity2, adj_edges_list2);
			for (int i = 0; i != adj_edges_list1.count (); ++i){
				for (int j = 0; j != adj_edges_list2.count (); ++j){
					if (adj_edges_list1[i] == adj_edges_list2[j])
						return (EDGE*)(adj_edges_list1[i]);
				}
			}
		}
	}
	return NULL;
}

EDGE* get_associated_geometry_edge_of_boundary_eh_gen_ver (VMesh *mesh, OvmEgH eh)
{
	assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

	auto vh1 = mesh->edge (eh).from_vertex (),
		vh2 = mesh->edge (eh).to_vertex ();
	ENTITY *entity1 = (ENTITY*)(V_ENTITY_PTR[vh1]),
		*entity2 = (ENTITY*)(V_ENTITY_PTR[vh2]);

	if (entity1 == NULL || entity2 == NULL) return NULL;
	if (is_FACE (entity1) || is_FACE (entity2)) return NULL;

	if (is_EDGE (entity1)){
		if (is_EDGE (entity2)){
			if (entity1 == entity2) return (EDGE*)entity1;
			else return NULL;
		}else if (is_VERTEX (entity2)){
			ENTITY_LIST adj_edges_list;
			api_get_edges (entity2, adj_edges_list);
			if (adj_edges_list.lookup (entity1) != -1) return (EDGE*)entity1;
			else return NULL;
		}
	}else if (is_VERTEX (entity1)){
		ENTITY_LIST adj_edges_list1;
		api_get_edges (entity1, adj_edges_list1);
		if (is_EDGE (entity2)){
			if (adj_edges_list1.lookup (entity2) != -1) return (EDGE*)entity2;
			else return NULL;
		}else if (is_VERTEX (entity2)){
			ENTITY_LIST adj_edges_list2;
			api_get_edges (entity2, adj_edges_list2);
			for (int i = 0; i != adj_edges_list1.count (); ++i){
				for (int j = 0; j != adj_edges_list2.count (); ++j){
					if (adj_edges_list1[i] == adj_edges_list2[j])
						return (EDGE*)(adj_edges_list1[i]);
				}
			}
		}
	}
	return NULL;
}


bool is_boundary_cell(VolumeMesh *mesh, OvmCeH ch)
{
	std::vector<OvmHaFaH> ch_hfhs = mesh->cell(ch).halffaces();
	foreach(auto hfh, ch_hfhs){
		if(mesh->is_boundary(mesh->face_handle(hfh)))
			return true;
	}
	return false;
}

void attach_mesh_elements_to_ACIS_entities (VolumeMesh *mesh, BODY *body, double myresabs)
{
	//system("pause");
	ENTITY_LIST vertices_temp_list,vertices_list, edges_list, faces_list;
	api_get_vertices (body, vertices_temp_list);
	//api_get_vertices (body, vertices_list);
	api_get_edges (body, edges_list);
	api_get_faces (body, faces_list);


	//不考虑没必要的点是否attach网格点
	vertices_temp_list.init();
	VERTEX* v;
	while(v = (VERTEX *)vertices_temp_list.next()){
		ENTITY_LIST e_list;
		api_get_edges(v, e_list);
		int adj_circu = 0;
		for (int e_c = 0;e_c < e_list.count();e_c++)
		{
			if (is_circular_edge(e_list[e_c]))
			{
				adj_circu++;
			}
		}
		//std::cout<<e_list.count()<<" "<<adj_circu<<std::endl;
		//if(e_list.count() != 1 && (is_circular_edge(e_list[0]) || is_linear_edge(e_list[0])))
		if(e_list.count() != 1 && adj_circu != 2/*&& (is_linear_edge(e_list[0]) || is_linear_edge(e_list[1]))*/)
			vertices_list.add((ENTITY*)v);
	}


	SPAposition closest_pos;
	double dist = 0.0f;
	auto fOnWhichEntity = [&myresabs, &closest_pos, &dist](ENTITY_LIST &entity_list, SPAposition &pos)->ENTITY*{
		for (int i = 0; i != entity_list.count (); ++i){
			api_entity_point_distance (entity_list[i], pos, closest_pos, dist);
			if (dist < myresabs)
				return entity_list[i];
		}
		return NULL;
	};

	auto V_ENT_PTR = mesh->request_vertex_property<unsigned long> ("entityptr", 0);
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
	{
		if (!mesh->is_boundary (*v_it))
			continue;

		SPAposition spa_pos = POS2SPA(mesh->vertex (*v_it));
		ENTITY* on_this_vertex = fOnWhichEntity (vertices_list, spa_pos);
		if (on_this_vertex)
			V_ENT_PTR[*v_it] = (unsigned long)on_this_vertex;
		else{
			ENTITY* on_this_edge = fOnWhichEntity (edges_list, spa_pos);
			if (on_this_edge)
				V_ENT_PTR[*v_it] = (unsigned long)on_this_edge;
			else{
				ENTITY* on_this_face = fOnWhichEntity (faces_list, spa_pos);
				if (on_this_face)
					V_ENT_PTR[*v_it] = (unsigned long)on_this_face;
				else{
					QMessageBox::critical (NULL, QObject::tr("错误!"), QObject::tr("这个点找不到对应的ACIS Entity!"));
					std::cout<<v_it->idx()<<"\n";
					assert (false);
				}
			}//end if (on_this_edge)
		}//end if (on_this_vertex)
	}
	mesh->set_persistent (V_ENT_PTR);
}
void attach_mesh_elements_to_ACIS_entities_with_repair (VolumeMesh *mesh, BODY *body, double myresabs)
{
	double ave_elen = 0;
	for (auto e_iter = mesh->edges_begin();e_iter != mesh->edges_end();e_iter++)
	{
		OvmVeH start_vh,end_vh;
		start_vh = mesh->edge(*e_iter).from_vertex();
		end_vh = mesh->edge(*e_iter).to_vertex();
		ave_elen += (mesh->vertex(start_vh)-mesh->vertex(end_vh)).length();
	}
	ave_elen /= mesh->n_edges();
	ENTITY_LIST vertices_temp_list,vertices_list, edges_list, faces_list;
	api_get_vertices (body, vertices_temp_list);
	//api_get_vertices (body, vertices_list);
	api_get_edges (body, edges_list);
	api_get_faces (body, faces_list);


	//不考虑没必要的点是否attach网格点
	vertices_temp_list.init();
	VERTEX* v;
	while(v = (VERTEX *)vertices_temp_list.next()){
		ENTITY_LIST e_list;
		api_get_edges(v, e_list);
		int adj_circu = 0;
		for (int e_c = 0;e_c < e_list.count();e_c++)
		{
			if (is_circular_edge(e_list[e_c]))
			{
				adj_circu++;
			}
		}
		//std::cout<<e_list.count()<<" "<<adj_circu<<std::endl;
		//if(e_list.count() != 1 && (is_circular_edge(e_list[0]) || is_linear_edge(e_list[0])))
		if(e_list.count() != 1/* && adj_circu != 2*//*&& (is_linear_edge(e_list[0]) || is_linear_edge(e_list[1]))*/)
			vertices_list.add((ENTITY*)v);
	}

	SPAposition closest_pos;
	double dist = 0.0f;
	auto fOnWhichEntity = [&myresabs, &closest_pos, &dist](ENTITY_LIST &entity_list, SPAposition &pos)->ENTITY*{
		for (int i = 0; i != entity_list.count (); ++i){
			api_entity_point_distance (entity_list[i], pos, closest_pos, dist);
			if (dist < myresabs)
				return entity_list[i];
		}
		return NULL;
	};

	std::unordered_set<OvmVeH> vhs_candidate;
	auto V_ENT_PTR = mesh->request_vertex_property<unsigned long> ("entityptr", 0);
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
	{
		if (!mesh->is_boundary (*v_it))
			continue;
		
		SPAposition spa_pos = POS2SPA(mesh->vertex (*v_it));
		ENTITY* on_this_vertex = fOnWhichEntity (vertices_list, spa_pos);
		if (on_this_vertex)
			V_ENT_PTR[*v_it] = (unsigned long)on_this_vertex;
		else{
			ENTITY* on_this_edge = fOnWhichEntity (edges_list, spa_pos);
			if (on_this_edge)
				V_ENT_PTR[*v_it] = (unsigned long)on_this_edge;
			else{
				ENTITY* on_this_face = fOnWhichEntity (faces_list, spa_pos);
				if (on_this_face)
					V_ENT_PTR[*v_it] = (unsigned long)on_this_face;
				else{
				vhs_candidate.insert(*v_it);
				}
			}//end if (on_this_edge)
		}//end if (on_this_vertex)
	}



	foreach(auto vh, vhs_candidate){
		ENTITY* close_E;
		double min_dis(1000);
		for(int i = 0; i < vertices_list.count(); i++){
			SPAposition c_pos;
			double dist = 0.0f;
			SPAposition s_pos = POS2SPA(mesh->vertex (vh));
			api_entity_point_distance(vertices_list[i],s_pos,c_pos,dist);
			if(dist < min_dis){
				min_dis = dist;
				close_E = vertices_list[i];
			}
		}
		for(int i = 0; i < edges_list.count(); i++){
			SPAposition c_pos;
			double dist = 0.0f;
			SPAposition s_pos = POS2SPA(mesh->vertex (vh));
			api_entity_point_distance(edges_list[i],s_pos,c_pos,dist);
			if(dist < min_dis){
				min_dis = dist;
				close_E = edges_list[i];
			}
		}
		for(int i = 0; i < faces_list.count(); i++){
			SPAposition c_pos;
			double dist = 0.0f;
			SPAposition s_pos = POS2SPA(mesh->vertex (vh));
			api_entity_point_distance(faces_list[i],s_pos,c_pos,dist);
			if(dist < min_dis){
				min_dis = dist;
				close_E = faces_list[i];
			}
		}
		SPAposition c_pos;
		double dist = 0.0f;
		SPAposition s_pos = POS2SPA(mesh->vertex (vh));
		api_entity_point_distance(close_E,s_pos,c_pos,dist);
		mesh->set_vertex(vh,SPA2POS(c_pos));
		V_ENT_PTR[vh] = (unsigned long)close_E;
	}
	//OvmVeH targetvh = (OvmVeH) 39;
	//ENTITY* close_E;
	//double min_dis(1000);
	//for(int i = 0; i < edges_list.count(); i++){
	//	SPAposition c_pos;
	//	double dist = 0.0f;
	//	SPAposition s_pos = POS2SPA(mesh->vertex (targetvh));
	//	api_entity_point_distance(edges_list[i],s_pos,c_pos,dist);
	//	if(dist < min_dis){
	//		min_dis = dist;
	//		close_E = edges_list[i];
	//	}
	//}
	//SPAposition c_pos;
	//dist = 0.0f;
	//SPAposition s_pos = POS2SPA(mesh->vertex (targetvh));
	//api_entity_point_distance(close_E,s_pos,c_pos,dist);
	//mesh->set_vertex(targetvh,SPA2POS(c_pos));
	//V_ENT_PTR[targetvh] = (unsigned long) close_E;

	//targetvh = (OvmVeH) 38;
	//min_dis = (1000);
	//for(int i = 0; i < edges_list.count(); i++){
	//	SPAposition c_pos;
	//	double dist = 0.0f;
	//	SPAposition s_pos = POS2SPA(mesh->vertex (targetvh));
	//	api_entity_point_distance(edges_list[i],s_pos,c_pos,dist);
	//	if(dist < min_dis){
	//		min_dis = dist;
	//		close_E = edges_list[i];
	//	}
	//}

	//s_pos = POS2SPA(mesh->vertex (targetvh));
	//api_entity_point_distance(close_E,s_pos,c_pos,dist);
	//mesh->set_vertex(targetvh,SPA2POS(c_pos));
	//V_ENT_PTR[targetvh] = (unsigned long)close_E;
	mesh->set_persistent (V_ENT_PTR);
}

void attach_mesh_elements_to_ACIS_entities_with_feature_repair (VolumeMesh *mesh, BODY *body, HoopsView* hoopsview, double myresabs)
{
	std::vector<std::unordered_set<OvmCeH>> blocks;
	std::vector<std::unordered_set<OvmVeH>> block_boundary_vhs;
	std::vector<std::unordered_set<OvmVeH>> block_inner_vhs;
	std::vector<std::unordered_set<OvmVeH>> block_feature_vhs;
	std::unordered_set<OvmVeH> blocks_corner_vhs;
	std::vector<std::vector<std::unordered_set<OvmVeH>>> block_boundary_patch;
	Get_Base_Complex(mesh, blocks, block_boundary_vhs, block_inner_vhs);
	std::vector<OvmEgH> feature_ehs;
	std::vector<OvmVeH> feature_vhs;
	Get_Feature_Infos(mesh, feature_vhs, feature_ehs);
	std::unordered_set<OvmVeH> corner_vhs_set, feature_vhs_set;
	foreach (auto v, feature_vhs) corner_vhs_set.insert(v);
	foreach (auto e, feature_ehs){
		feature_vhs_set.insert(mesh->edge(e).from_vertex());
		feature_vhs_set.insert(mesh->edge(e).to_vertex());
	}
	int block_id = 0;
	foreach (auto block, blocks){
		std::unordered_set<OvmEgH> f_ehs;std::unordered_set<OvmVeH> c_vhs;
		std::unordered_set<OvmVeH> feature_polyline_vhs;
		std::vector<std::unordered_set<OvmVeH>> boundary_patch_vhs;
		Get_Block_Infos(mesh, block, c_vhs, f_ehs, boundary_patch_vhs);
		blocks_corner_vhs.insert(c_vhs.begin(),c_vhs.end());
		foreach (auto eh, f_ehs){
			feature_polyline_vhs.insert(mesh->edge(eh).from_vertex());
			feature_polyline_vhs.insert(mesh->edge(eh).to_vertex());
		}
		block_boundary_patch.push_back(boundary_patch_vhs);
		//std::cout<<"feature_polyline_vhs.size is "<<feature_polyline_vhs.size()<<std::endl;
		block_feature_vhs.push_back(feature_polyline_vhs);feature_polyline_vhs.clear();
	}

	//std::cout<<"Blocks corner vhs size is "<<blocks_corner_vhs.size()<<std::endl;


	double ave_elen = 0;
	for (auto e_iter = mesh->edges_begin();e_iter != mesh->edges_end();e_iter++)
	{
		OvmVeH start_vh,end_vh;
		start_vh = mesh->edge(*e_iter).from_vertex();
		end_vh = mesh->edge(*e_iter).to_vertex();
		ave_elen += (mesh->vertex(start_vh)-mesh->vertex(end_vh)).length();
	}
	ave_elen /= mesh->n_edges();
	ENTITY_LIST vertices_temp_list,vertices_list, edges_list, faces_list;
	api_get_vertices (body, vertices_temp_list);
	//api_get_vertices (body, vertices_list);
	api_get_edges (body, edges_list);
	api_get_faces (body, faces_list);


	//不考虑没必要的点是否attach网格点
	vertices_temp_list.init();
	VERTEX* v;
	while(v = (VERTEX *)vertices_temp_list.next()){
		ENTITY_LIST e_list;
		api_get_edges(v, e_list);
		int adj_circu = 0;
		for (int e_c = 0;e_c < e_list.count();e_c++)
		{
			if (is_circular_edge(e_list[e_c]))
			{
				adj_circu++;
			}
		}
		//std::cout<<e_list.count()<<" "<<adj_circu<<std::endl;
		//if(e_list.count() != 1 && (is_circular_edge(e_list[0]) || is_linear_edge(e_list[0])))
		if(e_list.count() != 1/* && adj_circu != 2*//*&& (is_linear_edge(e_list[0]) || is_linear_edge(e_list[1]))*/)
			vertices_list.add((ENTITY*)v);
	}

	SPAposition closest_pos;
	double dist = 0.0f;
	
	auto fOnWhichEntity = [&myresabs, &closest_pos, &dist](ENTITY_LIST &entity_list, SPAposition &pos)->ENTITY*{
		for (int i = 0; i != entity_list.count (); ++i){
			api_entity_point_distance (entity_list[i], pos, closest_pos, dist);
			if (dist < myresabs)
				return entity_list[i];
		}
		return NULL;
	};
	//std::cout<<"Edge list and face list "<<edges_list.count()<<"$"<<faces_list.count()<<std::endl;
	std::unordered_set<OvmVeH> ring_vhs_set;
	std::unordered_set<int> affected_block_ids;
	std::unordered_set<OvmVeH> affected_vhs;
	/*************** lambdas******************************/


	auto fSmoothBoundary = [&](int iteration){
		while (iteration-- > 0){
			for (auto v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++){
				if (!mesh->is_boundary(*v_it) || feature_vhs_set.find(*v_it) != feature_vhs_set.end()) continue;
				std::vector<OvmVec3d> adj_poss;
				for (auto voh_it = mesh->voh_iter(*v_it);voh_it;voh_it++){
					
					auto end_vh = mesh->halfedge(*voh_it).to_vertex();
					if (mesh->is_boundary(end_vh) && mesh->is_boundary(*voh_it))
						adj_poss.push_back(mesh->vertex(end_vh));
				}
				OvmVec3d init_pos(0,0,0);
				foreach (auto t_pos, adj_poss) init_pos += t_pos/adj_poss.size();
				mesh->set_vertex(*v_it, 0.2*init_pos+0.8*mesh->vertex(*v_it));
			}
		}
	};

	auto fSmoothInner = [&](int iteration){
		while (iteration-- > 0){
			for (auto v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++){
				if (mesh->is_boundary(*v_it)) continue;
				std::vector<OvmVec3d> adj_poss;
				for (auto voh_it = mesh->voh_iter(*v_it);voh_it;voh_it++){
					auto end_vh = mesh->halfedge(*voh_it).to_vertex();
					//if (mesh->is_boundary(end_vh))
						adj_poss.push_back(mesh->vertex(end_vh));
				}
				OvmVec3d init_pos(0,0,0);
				foreach (auto t_pos, adj_poss) init_pos += t_pos/adj_poss.size();
				mesh->set_vertex(*v_it, 0.2*init_pos+0.8*mesh->vertex(*v_it));
			}
		}
	};
	auto fSmoothRingVhs = [&](int iteration){
		while(iteration-- > 0){
			foreach (auto vh, ring_vhs_set){
				std::vector<OvmVec3d> adj_poss;
				for (auto voh_it = mesh->voh_iter(vh);voh_it;voh_it++){
					auto end_vh = mesh->halfedge(*voh_it).to_vertex();
					if (mesh->is_boundary(end_vh) && mesh->is_boundary(*voh_it))
						adj_poss.push_back(mesh->vertex(end_vh));
				}
				OvmVec3d init_pos(0,0,0);
				foreach (auto t_pos, adj_poss) init_pos += t_pos/adj_poss.size();
				mesh->set_vertex(vh, init_pos);
			}
		}
	};

	auto fSmoothPolyline = [&](OvmHaEgH heh, int Max_Ring, OvmVec3d new_begin_position){
		std::vector<OvmHaEgH> polyline;
		if (feature_vhs_set.find(mesh->halfedge(heh).to_vertex()) != feature_vhs_set.end())
			return;
		polyline.push_back(heh);
		while (Max_Ring-- > 0){
			auto next_hehs = Get_Topology_Next_Hehs(mesh, polyline[polyline.size()-1]);
			if (next_hehs.size() != 1)
				break;
			polyline.push_back(next_hehs[0]);
		}
		if (polyline.size() == 1)
			return;
		std::vector<double> accumulate_lengths;
		for (int id = 0;id < polyline.size();id++){
			auto e = polyline[id];
			auto e_len = (mesh->vertex(mesh->halfedge(e).from_vertex())-mesh->vertex(mesh->halfedge(e).to_vertex())).length();
			if (id == 0)
				accumulate_lengths.push_back(e_len);
			else
				accumulate_lengths.push_back(accumulate_lengths[id-1] + e_len);
		}
		//auto s_pos = mesh->vertex(mesh->halfedge(polyline[0]).from_vertex());
		auto e_pos = mesh->vertex(mesh->halfedge(polyline[polyline.size()-1]).to_vertex());
		for (int id = 0;id < polyline.size()-1;id++){
			auto new_pos = (1-accumulate_lengths[id]/accumulate_lengths[polyline.size()-1])*new_begin_position
				+accumulate_lengths[id]/accumulate_lengths[polyline.size()-1]*e_pos;
			mesh->set_vertex(mesh->halfedge(polyline[id]).to_vertex(), new_pos);
		}
	};
	auto fSmoothBaseComplex =[&](std::unordered_set<int> block_ids, int iteration){
		std::cout<<"SmoothBaseComplex funs!"<<std::endl;
		/*
		for (int id = 0;id < blocks.size();id++){
			std::cout<<"********************** "<<id<<" ******************************"<<std::endl;
			std::cout<<"boundary_vhs:";
			foreach(auto vh , block_boundary_vhs[id]) std::cout<<vh.idx()<<" ";
			std::cout<<std::endl<<"block_boundary_patch: "<<std::endl;
			for (int fid = 0;fid < block_boundary_patch[id].size();fid++){
				std::cout<<fid<<": ";
				foreach (auto vh , block_boundary_patch[id][fid]) std::cout<<vh.idx()<<" ";
				std::cout<<std::endl;
			}
			std::cout<<"********************************************************************"<<std::endl;
		}
		*/
		while(iteration-- > 0){
			foreach (auto id, block_ids){
				//std::cout<<"Block "<<id<<": "<<"block_inner_vhs "<<block_inner_vhs[id].size()<<" block_boundary_vhs "<<block_boundary_vhs[id].size()
				//	<<" block_feature_vhs "<<block_feature_vhs.size()<<std::endl;
				foreach (auto vh, block_inner_vhs[id]){
					std::vector<OvmVec3d> poss;
					for (auto voh_it = mesh->voh_iter(vh);voh_it;voh_it++){
						poss.push_back(mesh->vertex(mesh->halfedge(*voh_it).to_vertex()));
					}
					OvmVec3d new_pos(0,0,0);
					foreach (auto pos, poss) new_pos += pos/poss.size();
					mesh->set_vertex(vh, new_pos);
				}
				foreach (auto vh, block_boundary_vhs[id]){
					if (block_feature_vhs[id].find(vh) != block_feature_vhs[id].end()) continue;
					int target_patch = -1;
					for (int pid = 0; pid < block_boundary_patch[id].size();pid++){
						if (block_boundary_patch[id][pid].find(vh) != block_boundary_patch[id][pid].end()){
							target_patch = pid;
							break;
						}
					}
					if (target_patch == -1){
						std::cout<<"Error target patch id!" <<vh.idx()<<std::endl; 
						//continue;
						exit(0);
					}
					std::vector<OvmVec3d> poss;
					for (auto voh_it = mesh->voh_iter(vh);voh_it;voh_it++){
						auto end_vh = mesh->halfedge(*voh_it).to_vertex();
						if (block_boundary_patch[id][target_patch].find(end_vh) != block_boundary_patch[id][target_patch].end()){
						//if (block_boundary_vhs[id].find(end_vh) != block_boundary_vhs[id].end()){
							poss.push_back(mesh->vertex(end_vh));
						}
					}
					if (poss.size() != 3){
						assert(poss.size() == 4);
						OvmVec3d new_pos(0,0,0);
						foreach (auto pos, poss) new_pos += pos/poss.size();
						mesh->set_vertex(vh, new_pos);
					}
				}
				
			}
		}
	};
	auto fModifying = [&](){
		//for Chaps_No_6
		//std::cout<<"Affected_vhs "<<affected_vhs.size()<<std::endl;
		std::unordered_set<OvmVeH> one_ring_vhs;
		std::vector<OvmVeH> one_ring_vhs_vec;
		std::vector<std::unordered_set<OvmVeH>> rings_vhs;
		foreach (auto v , affected_vhs){
			auto s_pos = mesh->vertex(v);
			//std::cout<<v.idx()<<", "<<s_pos[0]<<", "<<s_pos[1]<<", "<<s_pos[2]<<std::endl;
			if ((s_pos[2]-100) > 0.1){
				//std::cout<<"s_pos[2]"<<s_pos[2]<<std::endl;	
				continue; 
			}
			
			//std::cout<<"@@@@ "<<std::endl;
			for (auto voh_it= mesh->voh_iter(v);voh_it;voh_it++){
				auto end_vh = mesh->halfedge(*voh_it).to_vertex();
				double _z = mesh->vertex(end_vh)[2];
				if (_z == 100){
					one_ring_vhs.insert(end_vh);
				}
			}
		}
		foreach(auto vh , one_ring_vhs){
			one_ring_vhs_vec.push_back(vh);
			std::unordered_set<OvmVeH> ring_vhs;
			for (auto voh_it = mesh->voh_iter(vh);voh_it;voh_it++){
				auto tmp_vh = mesh->halfedge(*voh_it).to_vertex();
				if (mesh->vertex(tmp_vh)[2] == 100)
					ring_vhs.insert(tmp_vh);
			}
			rings_vhs.push_back(ring_vhs);
		}
		//std::cout<<"one_ring_vhs_vec.size = "<<one_ring_vhs_vec.size()<<std::endl;
		//std::cout<<"rings_vhs.size = "<<rings_vhs.size()<<std::endl;
		if (rings_vhs.size() != one_ring_vhs_vec.size()){
			std::cout<<"Error "<<__LINE__<<std::endl;
			exit(0);
		}

		for (int iteration = 0;iteration < 10;iteration++){
			for (int vi = 0;vi < rings_vhs.size();vi++){
				OvmVec3d center(0,0,0);
				foreach (auto vh , rings_vhs[vi]){
					center += mesh->vertex(vh);
				}
				center /= rings_vhs[vi].size();
				mesh->set_vertex(one_ring_vhs_vec[vi], center);
			}
		}

		//std::cout<<one_ring_vhs_vec[0]<<": ";
		//foreach (auto vh,  rings_vhs[0])
		//	std::cout<<vh<<", ";
		//std::cout<<std::endl;
		//
		std::unordered_set<OvmEgH> ehs_set;
		for (auto v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++){
			auto pos = mesh->vertex(*v_it);
			if (pos[2] == 92){
				OvmVeH target_vh1, target_vh2;target_vh1 = mesh->InvalidVertexHandle;target_vh2 = target_vh1;
				for (auto voh_it = mesh->voh_iter(*v_it);voh_it;voh_it++){
					auto tmp_vh = mesh->halfedge(*voh_it).to_vertex();
					if (mesh->vertex(tmp_vh)[2] == 100)
						target_vh1 = tmp_vh;
					if (mesh->vertex(tmp_vh)[2] == 90)
						target_vh2 = tmp_vh;
				}
				if (target_vh1 != mesh->InvalidVertexHandle){
					auto heh = mesh->halfedge(target_vh1, *v_it);
					if (heh != mesh->InvalidHalfEdgeHandle)
						ehs_set.insert(mesh->edge_handle(heh));
					auto tmp_pos = mesh->vertex(target_vh1);
					pos[0] = tmp_pos[0];pos[1] = tmp_pos[1];
					mesh->set_vertex(*v_it, pos);
				}
				auto tmp_bottom = mesh->vertex(target_vh2);
				tmp_bottom[0] = pos[0];tmp_bottom[1] = pos[1];
				mesh->set_vertex(target_vh2, tmp_bottom);
				auto heh1 = mesh->halfedge(*v_it, target_vh2);
				if (heh1 != mesh->InvalidHalfEdgeHandle)
					ehs_set.insert(mesh->edge_handle(heh1));
			}
		}
		//auto newg = new VolumeMeshElementGroup(mesh);
		//newg->ehs = ehs_set;
		//hoopsview->render_mesh_group(newg, false, "red", "red");
		return;
	};

	/**********************************************************************************/
	std::cout<<__LINE__<<std::endl;
	ring_vhs_set.clear();
	affected_block_ids.clear();
	int zero_dis = 0;
	for (auto v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++){
		if (!mesh->is_boundary(*v_it) /*|| blocks_corner_vhs.find(*v_it) != blocks_corner_vhs.end()*/) continue;
		auto spa_pos = POS2SPA(mesh->vertex(*v_it));
		OvmVec3d target_pos;
		double closet_dis = -100;
		if (feature_vhs_set.find(*v_it) != feature_vhs_set.end() /*&& corner_vhs_set.find(*v_it) == corner_vhs_set.end()*/){
			//std::cout<<v_it->idx()<<std::endl;
			//if (v_it->idx() == 53 || v_it->idx() == 58)
			//	continue;
				//std::cout<<"feature_vhs "<<std::endl;
			for (int i = 0;i != edges_list.count();i++){
				api_entity_point_distance(edges_list[i], spa_pos, closest_pos, dist);
				if (closet_dis == -100 || closet_dis > dist){
					//std::cout<<"!";
					closet_dis = dist;
					target_pos = SPA2POS(closest_pos);
				}
			}
			if (closet_dis < 0.01)
				zero_dis++;
			if (closet_dis >= 0.01){
				affected_vhs.insert(*v_it);
				//method1
				/*
				for (auto voh_it = mesh->voh_iter(*v_it);voh_it;voh_it++){
					if (mesh->is_boundary(*voh_it) && feature_vhs_set.find(mesh->halfedge(*voh_it).to_vertex()) == feature_vhs_set.end()){
						//fSmoothPolyline(*voh_it, 3, target_pos);
					}
				}
				*/
				//method2
				for (int bi = 0;bi < blocks.size();bi++){
					if (block_boundary_vhs[bi].find(*v_it) != block_boundary_vhs[bi].end())
						affected_block_ids.insert(bi);
				}
			}
			mesh->set_vertex(*v_it, target_pos);
		}
	}
	std::cout<<__LINE__<<std::endl;
	//fSmoothBoundary(10);
	//fSmoothInner(10);
	//fSmoothRingVhs(10);
	
	//fModifying();
	
	for (auto v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++){
		if (!mesh->is_boundary(*v_it)) continue;
		auto spa_pos = POS2SPA(mesh->vertex(*v_it));
		OvmVec3d target_pos;
		double closet_dis = -100;
		
		if (feature_vhs_set.find(*v_it) == feature_vhs_set.end()){
			
			for (int i = 0;i != faces_list.count();i++){
				api_entity_point_distance(faces_list[i], spa_pos, closest_pos, dist);
				if (closet_dis == -100 || closet_dis > dist){
					closet_dis = dist;
					target_pos = SPA2POS(closest_pos);
				}
			}
			
			if ((target_pos-mesh->vertex(*v_it)).length() > 0.001*ave_elen){
				//method1
				/*
				for (auto voh_it = mesh->voh_iter(*v_it);voh_it;voh_it++){
					if (mesh->is_boundary(*voh_it) && feature_vhs_set.find(mesh->halfedge(*voh_it).to_vertex()) == feature_vhs_set.end()){
						//fSmoothPolyline(*voh_it, 3, target_pos);
					}
				}
				*/
				//method2
				for (int bi = 0;bi < blocks.size();bi++){
					if (block_boundary_vhs[bi].find(*v_it) != block_boundary_vhs[bi].end())
						affected_block_ids.insert(bi);
				}
			}
			mesh->set_vertex(*v_it, target_pos);
		}
	}
	std::cout<<__LINE__<<std::endl;
	//fSmoothBaseComplex(affected_block_ids, 10);
	//return;
	std::unordered_set<OvmVeH> vhs_candidate;
	auto V_ENT_PTR = mesh->request_vertex_property<unsigned long> ("entityptr", 0);
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
	{
		if (!mesh->is_boundary (*v_it))
			continue;
		
		SPAposition spa_pos = POS2SPA(mesh->vertex (*v_it));
		ENTITY* on_this_vertex = fOnWhichEntity (vertices_list, spa_pos);
		if (on_this_vertex)
			V_ENT_PTR[*v_it] = (unsigned long)on_this_vertex;
		else{
			ENTITY* on_this_edge = fOnWhichEntity (edges_list, spa_pos);
			if (on_this_edge)
				V_ENT_PTR[*v_it] = (unsigned long)on_this_edge;
			else{
				ENTITY* on_this_face = fOnWhichEntity (faces_list, spa_pos);
				if (on_this_face)
					V_ENT_PTR[*v_it] = (unsigned long)on_this_face;
				else{
				vhs_candidate.insert(*v_it);
				}
			}//end if (on_this_edge)
		}//end if (on_this_vertex)
	}



	foreach(auto vh, vhs_candidate){
		ENTITY* close_E;
		double min_dis(1000);
		for(int i = 0; i < vertices_list.count(); i++){
			SPAposition c_pos;
			double dist = 0.0f;
			SPAposition s_pos = POS2SPA(mesh->vertex (vh));
			api_entity_point_distance(vertices_list[i],s_pos,c_pos,dist);
			if(dist < min_dis){
				min_dis = dist;
				close_E = vertices_list[i];
			}
		}
		for(int i = 0; i < edges_list.count(); i++){
			SPAposition c_pos;
			double dist = 0.0f;
			SPAposition s_pos = POS2SPA(mesh->vertex (vh));
			api_entity_point_distance(edges_list[i],s_pos,c_pos,dist);
			if(dist < min_dis){
				min_dis = dist;
				close_E = edges_list[i];
			}
		}
		for(int i = 0; i < faces_list.count(); i++){
			SPAposition c_pos;
			double dist = 0.0f;
			SPAposition s_pos = POS2SPA(mesh->vertex (vh));
			api_entity_point_distance(faces_list[i],s_pos,c_pos,dist);
			if(dist < min_dis){
				min_dis = dist;
				close_E = faces_list[i];
			}
		}
		SPAposition c_pos;
		double dist = 0.0f;
		SPAposition s_pos = POS2SPA(mesh->vertex (vh));
		api_entity_point_distance(close_E,s_pos,c_pos,dist);
		mesh->set_vertex(vh,SPA2POS(c_pos));
		V_ENT_PTR[vh] = (unsigned long)close_E;
	}
	mesh->set_persistent (V_ENT_PTR);
}

void attach_mesh_elements_to_ACIS_entities_for_SW_Level1(VolumeMesh *mesh, BODY *body, HoopsView* hoopsview, double myresabs)
{
	std::vector<std::unordered_set<OvmCeH>> blocks;
	std::vector<std::unordered_set<OvmVeH>> block_boundary_vhs;
	std::vector<std::unordered_set<OvmVeH>> block_inner_vhs;
	std::vector<std::unordered_set<OvmVeH>> block_feature_vhs;
	std::unordered_set<OvmVeH> blocks_corner_vhs;
	std::vector<std::vector<std::unordered_set<OvmVeH>>> block_boundary_patch;
	Get_Base_Complex(mesh, blocks, block_boundary_vhs, block_inner_vhs);
	std::vector<OvmEgH> feature_ehs;
	std::vector<OvmVeH> feature_vhs;
	std::unordered_set<OvmEgH> feature_ehs_set;
	Get_Feature_Infos(mesh, feature_vhs, feature_ehs);
	std::unordered_set<OvmVeH> corner_vhs_set, feature_vhs_set;
	foreach (auto v, feature_vhs) corner_vhs_set.insert(v);
	foreach (auto e, feature_ehs){
		feature_ehs_set.insert(e);
		feature_vhs_set.insert(mesh->edge(e).from_vertex());
		feature_vhs_set.insert(mesh->edge(e).to_vertex());
	}
	
	int block_id = 0;
	foreach (auto block, blocks){
		std::unordered_set<OvmEgH> f_ehs;std::unordered_set<OvmVeH> c_vhs;
		std::unordered_set<OvmVeH> feature_polyline_vhs;
		std::vector<std::unordered_set<OvmVeH>> boundary_patch_vhs;
		Get_Block_Infos(mesh, block, c_vhs, f_ehs, boundary_patch_vhs);
		blocks_corner_vhs.insert(c_vhs.begin(),c_vhs.end());
		foreach (auto eh, f_ehs){
			feature_polyline_vhs.insert(mesh->edge(eh).from_vertex());
			feature_polyline_vhs.insert(mesh->edge(eh).to_vertex());
		}
		block_boundary_patch.push_back(boundary_patch_vhs);
		//std::cout<<"feature_polyline_vhs.size is "<<feature_polyline_vhs.size()<<std::endl;
		block_feature_vhs.push_back(feature_polyline_vhs);feature_polyline_vhs.clear();
	}

	//std::cout<<"Blocks corner vhs size is "<<blocks_corner_vhs.size()<<std::endl;


	double ave_elen = 0;
	for (auto e_iter = mesh->edges_begin();e_iter != mesh->edges_end();e_iter++)
	{
		OvmVeH start_vh,end_vh;
		start_vh = mesh->edge(*e_iter).from_vertex();
		end_vh = mesh->edge(*e_iter).to_vertex();
		ave_elen += (mesh->vertex(start_vh)-mesh->vertex(end_vh)).length();
	}
	ave_elen /= mesh->n_edges();
	ENTITY_LIST vertices_temp_list,vertices_list, edges_list, faces_list;
	api_get_vertices (body, vertices_temp_list);
	//api_get_vertices (body, vertices_list);
	api_get_edges (body, edges_list);
	api_get_faces (body, faces_list);


	//不考虑没必要的点是否attach网格点
	vertices_temp_list.init();
	VERTEX* v;
	while(v = (VERTEX *)vertices_temp_list.next()){
		ENTITY_LIST e_list;
		api_get_edges(v, e_list);
		int adj_circu = 0;
		for (int e_c = 0;e_c < e_list.count();e_c++)
		{
			if (is_circular_edge(e_list[e_c]))
			{
				adj_circu++;
			}
		}
		//std::cout<<e_list.count()<<" "<<adj_circu<<std::endl;
		//if(e_list.count() != 1 && (is_circular_edge(e_list[0]) || is_linear_edge(e_list[0])))
		if(e_list.count() != 1/* && adj_circu != 2*//*&& (is_linear_edge(e_list[0]) || is_linear_edge(e_list[1]))*/)
			vertices_list.add((ENTITY*)v);
	}

	SPAposition closest_pos;
	double dist = 0.0f;
	
	auto fOnWhichEntity = [&myresabs, &closest_pos, &dist](ENTITY_LIST &entity_list, SPAposition &pos)->ENTITY*{
		for (int i = 0; i != entity_list.count (); ++i){
			api_entity_point_distance (entity_list[i], pos, closest_pos, dist);
			if (dist < myresabs)
				return entity_list[i];
		}
		return NULL;
	};
	std::vector<double> target_zs;target_zs.push_back(0);target_zs.push_back(4);
	target_zs.push_back(60);target_zs.push_back(65);
	auto fIfInRegion = [&](OvmVec3d _pos_)->bool{
		if (_pos_[0] <= 30 && _pos_[0] >= -30 && _pos_[1] >= 10 && _pos_[1] <= 55)
			return true;
		else
			return false;
	};
	auto fFindPolyline = [&](OvmVeH vh)->std::vector<OvmHaEgH>{
		auto target_z = mesh->vertex(vh)[2];
		auto start_heh = mesh->InvalidHalfEdgeHandle;
		for (auto voh_it = mesh->voh_iter(vh);voh_it;voh_it++){
			auto end_vh = mesh->halfedge(*voh_it).to_vertex();
			if (std::abs(mesh->vertex(end_vh)[2]-target_z) > 1 || feature_vhs_set.find(end_vh) != feature_vhs_set.end())
				continue;
			if (feature_ehs_set.find(mesh->edge_handle(*voh_it)) == feature_ehs_set.end()){
				start_heh = *voh_it;
				break;
			}
		}
		if (start_heh == mesh->InvalidHalfEdgeHandle)
			return std::vector<OvmHaEgH>();
		auto current_heh = start_heh;
		std::vector<OvmHaEgH> polyline;
		while(true){
			std::unordered_set<OvmHaEgH> adj_hehs;
			auto s_vh = mesh->halfedge(current_heh).to_vertex();
			if (!fIfInRegion(mesh->vertex(s_vh)))
				break;
			polyline.push_back(current_heh);
			for (auto voh_iter = mesh->voh_iter(s_vh);voh_iter;voh_iter++){
				auto new_e_vh = mesh->halfedge(*voh_iter).to_vertex();
				if (std::abs(mesh->vertex(new_e_vh)[2]-target_z) < 2 && (mesh->opposite_halfedge_handle(*voh_iter) != current_heh ))
					adj_hehs.insert(*voh_iter);
			}
			if (adj_hehs.size() != 3) break;
			std::unordered_set<OvmEgH> tmp_adj_ehs;
			for (auto hehf_it = mesh->hehf_iter(current_heh);hehf_it;hehf_it++){
				auto adj_hes = mesh->halfface(*hehf_it).halfedges();
				for (auto heit = adj_hes.begin();heit != adj_hes.end();heit++) tmp_adj_ehs.insert(mesh->edge_handle(*heit));
			}
			auto next_heh = mesh->InvalidHalfEdgeHandle;
			for (auto heh_it = adj_hehs.begin();heh_it != adj_hehs.end();heh_it++){
				if (tmp_adj_ehs.find(mesh->edge_handle(*heh_it)) == tmp_adj_ehs.end()){
					next_heh = *heh_it;
					break;
				}
			}
			if (next_heh == mesh->InvalidHalfEdgeHandle){
				std::cout<<"Error at FunDefs.cpp @ Line "<<__LINE__<<std::endl;
				exit(0);
			}
			current_heh = next_heh;
		}
		return polyline;
	};
	auto fConvertPolyline2Vhs = [&](std::vector<OvmHaEgH> polyline)->std::vector<OvmVeH>{
		std::vector<OvmVeH> vhs;
		vhs.push_back(mesh->halfedge(polyline[0]).from_vertex());
		for (int pi = 0;pi < polyline.size();pi++){
			vhs.push_back(mesh->halfedge(polyline[pi]).to_vertex());
		}
		return vhs;
	};
	std::unordered_set<OvmVeH> affected_vhs;
	//auto newg = new VolumeMeshElementGroup(mesh);
	//newg->ehs.insert(feature_ehs.begin(), feature_ehs.end());
	//newg->vhs.insert(feature_vhs.begin(), feature_vhs.end());
	//newg->vhs = feature_vhs_set;
	//hoopsview->render_mesh_group(newg, false, "red", "blue");
	//for (auto v_it = feature_vhs_set.begin();v_it != feature_vhs_set.end();v_it++){
	std::map<OvmVeH, OvmVec3d> vh2pos;
	for (auto v_it = mesh->vertices_begin();v_it != mesh->vertices_end();v_it++){
		if (!mesh->is_boundary(*v_it)) continue;
		if (corner_vhs_set.find(*v_it) != corner_vhs_set.end())	continue;
		auto pos = mesh->vertex(*v_it);
		if (!fIfInRegion(pos))
			continue;
		double target_z = pos[2];
		auto spa_pos = POS2SPA(pos);
		OvmVec3d target_pos;
		double closet_dis = -100;
		double dis;
		if (feature_vhs_set.find(*v_it) != feature_vhs_set.end()){
			for (int i = 0;i < edges_list.count();i++){
				api_entity_point_distance(edges_list[i], spa_pos, closest_pos, dis);
				auto tmp_target_pos = SPA2POS(closest_pos);
				if (std::abs(tmp_target_pos[2]-target_z) > 2)
					continue;
				if (closet_dis == -100 || closet_dis > dis){
					//if (target_z == 60){
					//	std::cout<<"# "<<v_it->idx()<<" vhid = "<<i<<"~ "<<closet_dis<<"vs dis "<<dis<<"  target pos is "<<tmp_target_pos[0]<<", "<<tmp_target_pos[1]<<", "<<tmp_target_pos[2]<<std::endl;
					//}
					closet_dis = dis; target_pos = tmp_target_pos;
				}
			}
			if (closet_dis > 0.5)
				affected_vhs.insert(*v_it);
		}
		else{
			for (int i = 0;i < faces_list.count();i++){
				api_entity_point_distance(faces_list[i], spa_pos, closest_pos, dis);
				auto tmp_target_pos = SPA2POS(closest_pos);
				if (std::abs(tmp_target_pos[2]-target_z) > 3)
					continue;
				if (closet_dis == -100 || closet_dis > dis){
					closet_dis = dis; target_pos = tmp_target_pos;
				}
			}
		}
		if (closet_dis == -100){
			std::cout<<"Error at FuncDefs.cpp @ Line "<<__LINE__<<std::endl;
			exit(0);
		}
		vh2pos[*v_it] = target_pos;
		//mesh->set_vertex(*v_it, target_pos);
	}
	//std::cout<<__LINE__<<std::endl;
	std::map<OvmVeH, int> vh2polyline;
	std::vector<std::vector<OvmVeH>> polylines_in_vhs;
	std::vector<std::vector<double>> lengths;
	for (auto vit = affected_vhs.begin();vit != affected_vhs.end();vit++){
		auto polyline = fFindPolyline(*vit);
		if (polyline.empty())
			continue;
		auto polyline_vhs = fConvertPolyline2Vhs(polyline);polylines_in_vhs.push_back(polyline_vhs);
		std::vector<double> length;
		for (int vi = 0;vi < polyline_vhs.size()-1;vi++){
			auto pos1 = mesh->vertex(polyline_vhs[vi]);
			auto pos2 = mesh->vertex(polyline_vhs[vi+1]);
			length.push_back((pos1-pos2).length());
		}
		lengths.push_back(length);
		vh2polyline[*vit] = lengths.size()-1;
	}
	//std::cout<<__LINE__<<std::endl;
	for (auto map_iter = vh2pos.begin();map_iter != vh2pos.end();map_iter++){
		mesh->set_vertex(map_iter->first, map_iter->second);
	}
	//std::cout<<__LINE__<<std::endl;
	for (auto v_it = feature_vhs_set.begin();v_it != feature_vhs_set.end();v_it++){
		auto pos = mesh->vertex(*v_it);
		if (std::abs(pos[2]-60) > 2) continue;
		bool if_reset = false;
		OvmVeH target_top_vh = mesh->InvalidVertexHandle;
		for (auto voh_it = mesh->voh_iter(*v_it);voh_it;voh_it++){
			auto end_vh = mesh->halfedge(*voh_it).to_vertex();
			if (std::abs(mesh->vertex(end_vh)[2] - 65) < 1){
				target_top_vh = end_vh;
				break;
			}
		}
		if (target_top_vh != mesh->InvalidVertexHandle){
			//newg->vhs.insert(*v_it);
			auto target_pos = mesh->vertex(target_top_vh);
			target_pos[2] = 60;
			mesh->set_vertex(*v_it, target_pos);
		}
	}
	for (auto v_it = feature_vhs_set.begin();v_it != feature_vhs_set.end();v_it++){
		auto pos = mesh->vertex(*v_it);
		if (std::abs(pos[2]-4) > 2) continue;
		bool if_reset = false;
		OvmVeH target_top_vh = mesh->InvalidVertexHandle;
		for (auto voh_it = mesh->voh_iter(*v_it);voh_it;voh_it++){
			auto end_vh = mesh->halfedge(*voh_it).to_vertex();
			if (std::abs(mesh->vertex(end_vh)[2] - 0) < 1){
				target_top_vh = end_vh;
				break;
			}
		}
		if (target_top_vh != mesh->InvalidVertexHandle){
			//newg->vhs.insert(*v_it);
			auto target_pos = mesh->vertex(target_top_vh);
			target_pos[2] = 4;
			mesh->set_vertex(*v_it, target_pos);
		}
	}
	//std::cout<<__LINE__<<std::endl;
	//std::cout<<"Affected vhs and vh2polyline size comparison: "<<affected_vhs.size()<<" vs "<<vh2polyline.size()<<std::endl;
	for (auto vhiter = vh2polyline.begin();vhiter != vh2polyline.end();vhiter++){
		std::cout<<vhiter->first<<": "<<vhiter->second<<std::endl;
		auto vhs = polylines_in_vhs[vhiter->second];
		auto start_pos = mesh->vertex(vhs[0]); auto end_pos = mesh->vertex(vhs[vhs.size()-1]);
		auto lens = lengths[vhiter->second];
		double len_sum = 0;
		for (auto l = lens.begin();l != lens.end();l++){
			len_sum += *l;
		}
		if (vhs.size() > 2){
			for (int i = 1;i < vhs.size()-1;i++){
				double accumulate_len = 0;
				for (int li = 0;li < i;li++){
					accumulate_len += lens[li];
				}
				double ratio = accumulate_len/len_sum;
				auto new_pos = (1-ratio)*start_pos + ratio*end_pos;
				mesh->set_vertex(vhs[i], new_pos);
			}
		}
	}
	
	//std::cout<<__LINE__<<std::endl;
	//newg->vhs = affected_vhs;
	//for (auto vit = affected_vhs.begin();vit != affected_vhs.end();vit++){
	//	//break;
	//	auto polyline = fFindPolyline(*vit);
	//	for (auto he_it = polyline.begin();he_it != polyline.end();he_it++){
	//		newg->ehs.insert(mesh->edge_handle(*he_it));
	//	}
	//}
	//hoopsview->render_mesh_group(newg, false, "red", "blue");
	
	std::unordered_set<OvmVeH> vhs_candidate;
	auto V_ENT_PTR = mesh->request_vertex_property<unsigned long> ("entityptr", 0);
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
	{
		if (!mesh->is_boundary (*v_it))
			continue;
		
		SPAposition spa_pos = POS2SPA(mesh->vertex (*v_it));
		ENTITY* on_this_vertex = fOnWhichEntity (vertices_list, spa_pos);
		if (on_this_vertex)
			V_ENT_PTR[*v_it] = (unsigned long)on_this_vertex;
		else{
			ENTITY* on_this_edge = fOnWhichEntity (edges_list, spa_pos);
			if (on_this_edge)
				V_ENT_PTR[*v_it] = (unsigned long)on_this_edge;
			else{
				ENTITY* on_this_face = fOnWhichEntity (faces_list, spa_pos);
				if (on_this_face)
					V_ENT_PTR[*v_it] = (unsigned long)on_this_face;
				else{
				vhs_candidate.insert(*v_it);
				}
			}//end if (on_this_edge)
		}//end if (on_this_vertex)
	}



	foreach(auto vh, vhs_candidate){
		ENTITY* close_E;
		double min_dis(1000);
		for(int i = 0; i < vertices_list.count(); i++){
			SPAposition c_pos;
			double dist = 0.0f;
			SPAposition s_pos = POS2SPA(mesh->vertex (vh));
			api_entity_point_distance(vertices_list[i],s_pos,c_pos,dist);
			if(dist < min_dis){
				min_dis = dist;
				close_E = vertices_list[i];
			}
		}
		for(int i = 0; i < edges_list.count(); i++){
			SPAposition c_pos;
			double dist = 0.0f;
			SPAposition s_pos = POS2SPA(mesh->vertex (vh));
			api_entity_point_distance(edges_list[i],s_pos,c_pos,dist);
			if(dist < min_dis){
				min_dis = dist;
				close_E = edges_list[i];
			}
		}

		for(int i = 0; i < faces_list.count(); i++){
			SPAposition c_pos;
			double dist = 0.0f;
			SPAposition s_pos = POS2SPA(mesh->vertex (vh));
			api_entity_point_distance(faces_list[i],s_pos,c_pos,dist);
			if(dist < min_dis){
				min_dis = dist;
				close_E = faces_list[i];
			}
		}
		SPAposition c_pos;
		double dist = 0.0f;
		SPAposition s_pos = POS2SPA(mesh->vertex (vh));
		api_entity_point_distance(close_E,s_pos,c_pos,dist);
		//mesh->set_vertex(vh,SPA2POS(c_pos));
		V_ENT_PTR[vh] = (unsigned long)close_E;
	}
	mesh->set_persistent (V_ENT_PTR);
}

void attach_tet_mesh_elements_to_ACIS_entities (VMesh *mesh, BODY *body, double myresabs)
{
	ENTITY_LIST vertices_temp_list,vertices_list, edges_list, faces_list;
	api_get_vertices (body, vertices_temp_list);
	//api_get_vertices (body, vertices_list);
	api_get_edges (body, edges_list);
	api_get_faces (body, faces_list);


	//不考虑没必要的点是否attach网格点
	vertices_temp_list.init();
	VERTEX* v;
	while(v = (VERTEX *)vertices_temp_list.next()){
		ENTITY_LIST e_list;
		api_get_edges(v, e_list);
		//if(e_list.count() != 1 && (is_circular_edge(e_list[0]) || is_linear_edge(e_list[0])))
		if(e_list.count() != 1 /*&& (is_linear_edge(e_list[0]) || is_linear_edge(e_list[1]))*/)
			vertices_list.add((ENTITY*)v);
	}


	SPAposition closest_pos;
	double dist = 0.0f;
	auto fOnWhichEntity = [&myresabs, &closest_pos, &dist](ENTITY_LIST &entity_list, SPAposition &pos)->ENTITY*{
		for (int i = 0; i != entity_list.count (); ++i){
			api_entity_point_distance (entity_list[i], pos, closest_pos, dist);
			if (dist < myresabs)
				return entity_list[i];
		}
		return NULL;
	};

	auto V_ENT_PTR = mesh->request_vertex_property<unsigned long> ("entityptr", 0);
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
	{
		if (!mesh->is_boundary (*v_it))
			continue;

		SPAposition spa_pos = POS2SPA(mesh->vertex (*v_it));
		ENTITY* on_this_vertex = fOnWhichEntity (vertices_list, spa_pos);
		if (on_this_vertex)
			V_ENT_PTR[*v_it] = (unsigned long)on_this_vertex;
		else{
			ENTITY* on_this_edge = fOnWhichEntity (edges_list, spa_pos);
			if (on_this_edge)
				V_ENT_PTR[*v_it] = (unsigned long)on_this_edge;
			else{
				ENTITY* on_this_face = fOnWhichEntity (faces_list, spa_pos);
				if (on_this_face)
					V_ENT_PTR[*v_it] = (unsigned long)on_this_face;
				else{
					QMessageBox::critical (NULL, QObject::tr("错误!"), QObject::tr("这个点找不到对应的ACIS Entity!"));
					std::cout<<v_it->idx()<<"\n";
					assert (false);
				}
			}//end if (on_this_edge)
		}//end if (on_this_vertex)
	}
	mesh->set_persistent (V_ENT_PTR);
}

void attach_tet_mesh_elements_to_ACIS_entities_with_repair (VMesh *mesh, BODY *body, double myresabs)
{
	double ave_elen = 0;
	for (auto e_iter = mesh->edges_begin();e_iter != mesh->edges_end();e_iter++)
	{
		OvmVeH start_vh,end_vh;
		start_vh = mesh->edge(*e_iter).from_vertex();
		end_vh = mesh->edge(*e_iter).to_vertex();
		ave_elen += (mesh->vertex(start_vh)-mesh->vertex(end_vh)).length();
	}
	ave_elen /= mesh->n_edges();
	ENTITY_LIST vertices_temp_list,vertices_list, edges_list, faces_list;
	api_get_vertices (body, vertices_temp_list);
	//api_get_vertices (body, vertices_list);
	api_get_edges (body, edges_list);
	api_get_faces (body, faces_list);


	//不考虑没必要的点是否attach网格点
	vertices_temp_list.init();
	VERTEX* v;
	while(v = (VERTEX *)vertices_temp_list.next()){
		ENTITY_LIST e_list;
		api_get_edges(v, e_list);
	
		//if(e_list.count() != 1 && (is_circular_edge(e_list[0]) || is_linear_edge(e_list[0])))
		if(e_list.count() != 1 /*&& (is_linear_edge(e_list[0]) || is_linear_edge(e_list[1]))*/)
		//if (e_list.count() != 1 && (adjacent_cir != 2))
			vertices_list.add((ENTITY*)v);
	}


	SPAposition closest_pos;
	double dist = 0.0f;
	auto fOnWhichEntity = [&myresabs, &closest_pos, &dist](ENTITY_LIST &entity_list, SPAposition &pos)->ENTITY*{
		for (int i = 0; i != entity_list.count (); ++i){
			api_entity_point_distance (entity_list[i], pos, closest_pos, dist);
			if (dist < myresabs)
				return entity_list[i];
		}
		return NULL;
	};
	std::unordered_set<OvmVeH> vhs_candidate;
	auto V_ENT_PTR = mesh->request_vertex_property<unsigned long> ("entityptr", 0);
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
	{
		if (!mesh->is_boundary (*v_it))
			continue;
		SPAposition spa_pos = POS2SPA(mesh->vertex (*v_it));
		ENTITY* on_this_vertex = fOnWhichEntity (vertices_list, spa_pos);
		if (on_this_vertex)
			V_ENT_PTR[*v_it] = (unsigned long)on_this_vertex;
		else{
			ENTITY* on_this_edge = fOnWhichEntity (edges_list, spa_pos);
			if (on_this_edge)
				V_ENT_PTR[*v_it] = (unsigned long)on_this_edge;
			else{
				ENTITY* on_this_face = fOnWhichEntity (faces_list, spa_pos);
				if (on_this_face)
					V_ENT_PTR[*v_it] = (unsigned long)on_this_face;
				else{
					vhs_candidate.insert(*v_it);
				}
			}//end if (on_this_edge)
		}//end if (on_this_vertex)
	}
	//////////////////////////////////////////////////////////////////////////
	//std::unordered_set<OvmVeH> model_targeted;
	//model_targeted.insert((OvmVeH)46);model_targeted.insert((OvmVeH)36);
	//for (auto vit = model_targeted.begin();vit != model_targeted.end();vit++)
	//{

	//}
	//////////////////////////////////////////////////////////////////////////
	foreach(auto vh, vhs_candidate){
		ENTITY* close_E;
		double min_dis(10e7);

		for(int i = 0; i < vertices_list.count(); i++){
			SPAposition c_pos;
			double dist = 0.0f;
			SPAposition s_pos = POS2SPA(mesh->vertex (vh));
			api_entity_point_distance(vertices_list[i],s_pos,c_pos,dist);
			if(dist < min_dis){
				min_dis = dist;
				close_E = vertices_list[i];
			}
		}
		for(int i = 0; i < edges_list.count(); i++){
			SPAposition c_pos;
			double dist = 0.0f;
			SPAposition s_pos = POS2SPA(mesh->vertex (vh));
			api_entity_point_distance(edges_list[i],s_pos,c_pos,dist);
			if(dist < min_dis){
				min_dis = dist;
				close_E = edges_list[i];
			}
		}
		for(int i = 0; i < faces_list.count(); i++){
			SPAposition c_pos;
			double dist = 0.0f;
			SPAposition s_pos = POS2SPA(mesh->vertex (vh));
			api_entity_point_distance(faces_list[i],s_pos,c_pos,dist);

			if(dist < min_dis){
				min_dis = dist;
				close_E = faces_list[i];
			}
		}
		SPAposition c_pos;
		double dist = 0.0f;
		SPAposition s_pos = POS2SPA(mesh->vertex (vh));
		api_entity_point_distance(close_E,s_pos,c_pos,dist);
		mesh->set_vertex(vh,SPA2POS(c_pos));
		V_ENT_PTR[vh] = (unsigned long)close_E;
	}
	//OvmVeH target_vh = (OvmVeH)824;OvmVeH up_vh = (OvmVeH)836;OvmVeH down_vh = (OvmVeH)812;
	//mesh->set_vertex(target_vh,(0.2*mesh->vertex(up_vh)+0.8*mesh->vertex(down_vh)));
	mesh->set_persistent (V_ENT_PTR);
}
void attach_tet_mesh_elements_to_ACIS_entities_with_repair (VMesh *mesh, BODY *body, HoopsView* hoopsview, double myresabs)
{
	//std::cout<<"Enter repair"<<std::endl;
	//if (mesh->is_boundary(((OvmVeH )878)))
	//{
	//	std::cout<<"878 is boundary"<<std::endl;
	//}
	std::cout<<"Enter with render"<<std::endl;
	double ave_elen = 0;
	for (auto e_iter = mesh->edges_begin();e_iter != mesh->edges_end();e_iter++)
	{
		OvmVeH start_vh,end_vh;
		start_vh = mesh->edge(*e_iter).from_vertex();
		end_vh = mesh->edge(*e_iter).to_vertex();
		ave_elen += (mesh->vertex(start_vh)-mesh->vertex(end_vh)).length();
	}
	ave_elen /= mesh->n_edges();
	std::cout<<"mesh average len "<<ave_elen<<std::endl;
	ENTITY_LIST vertices_temp_list,vertices_list, edges_list, faces_list;
	api_get_vertices (body, vertices_temp_list);
	//api_get_vertices (body, vertices_list);
	api_get_edges (body, edges_list);
	api_get_faces (body, faces_list);


	//不考虑没必要的点是否attach网格点
	vertices_temp_list.init();
	VERTEX* v;
	while(v = (VERTEX *)vertices_temp_list.next()){
		ENTITY_LIST e_list;
		api_get_edges(v, e_list);
		//if(e_list.count() != 1 && (is_circular_edge(e_list[0]) || is_linear_edge(e_list[0])))
		if(e_list.count() != 1 /*&& (is_linear_edge(e_list[0]) || is_linear_edge(e_list[1]))*/)
			vertices_list.add((ENTITY*)v);
	}


	SPAposition closest_pos;
	double dist = 0.0f;
	auto fOnWhichEntity = [&myresabs, &closest_pos, &dist](ENTITY_LIST &entity_list, SPAposition &pos)->ENTITY*{
		for (int i = 0; i != entity_list.count (); ++i){
			api_entity_point_distance (entity_list[i], pos, closest_pos, dist);
			if (dist < myresabs)
				return entity_list[i];
		}
		return NULL;
	};
	std::unordered_set<OvmVeH> vhs_candidate;
	auto V_ENT_PTR = mesh->request_vertex_property<unsigned long> ("entityptr", 0);
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
	{
		if (!mesh->is_boundary (*v_it))
			continue;
		SPAposition spa_pos = POS2SPA(mesh->vertex (*v_it));
		ENTITY* on_this_vertex = fOnWhichEntity (vertices_list, spa_pos);
		if (on_this_vertex)
			V_ENT_PTR[*v_it] = (unsigned long)on_this_vertex;
		else{
			ENTITY* on_this_edge = fOnWhichEntity (edges_list, spa_pos);
			if (on_this_edge)
				V_ENT_PTR[*v_it] = (unsigned long)on_this_edge;
			else{
				ENTITY* on_this_face = fOnWhichEntity (faces_list, spa_pos);
				if (on_this_face)
					V_ENT_PTR[*v_it] = (unsigned long)on_this_face;
				else{
					vhs_candidate.insert(*v_it);
				}
			}//end if (on_this_edge)
		}//end if (on_this_vertex)
	}
	//////////////////////////////////////////////////////////////////////////
	std::unordered_set<OvmVeH> model_targeted;
	std::unordered_set<OvmVeH> model_int;
	model_int.insert(24);    model_int.insert(26);    model_int.insert(31);    model_int.insert(25);
	model_int.insert(33);    model_int.insert(34);    model_int.insert(66);    model_int.insert(67);
	model_int.insert(35);    model_int.insert(68);    model_int.insert(32);    model_int.insert(65);
	model_int.insert(42);    model_int.insert(37);    model_int.insert(38);    model_int.insert(36);
	model_int.insert(50);    model_int.insert(44);    model_int.insert(48);    model_int.insert(46);
	model_int.insert(45);    model_int.insert(47);    model_int.insert(49);    model_int.insert(43);
	model_int.insert(58);    model_int.insert(52);    model_int.insert(53);    model_int.insert(51);
	model_int.insert(9);    model_int.insert(60);    model_int.insert(62);    model_int.insert(64);
	model_int.insert(61);    model_int.insert(63);    model_int.insert(59);    model_int.insert(6);
	model_int.insert(15);    model_int.insert(8);    model_int.insert(10);    model_int.insert(7);
	model_int.insert(23);    model_int.insert(17);    model_int.insert(21);    model_int.insert(19);
	model_int.insert(18);    model_int.insert(20);    model_int.insert(16);    model_int.insert(22);

	for (auto iit = model_int.begin();iit != model_int.end();iit++)
	{
		OvmVeH current_vh = (OvmVeH)*iit;
		model_targeted.insert(current_vh);
		ENTITY* close_V;
		double min_dis(10e7);
		OvmVec3d new_pos;
		for(int i = 0; i < vertices_list.count(); i++){
			SPAposition c_pos;
			double dist = 0.0f;
			SPAposition s_pos = POS2SPA(mesh->vertex (current_vh));
			api_entity_point_distance(vertices_list[i],s_pos,c_pos,dist);
			if(dist < min_dis){
				min_dis = dist;
				close_V = vertices_list[i];
				new_pos = SPA2POS(c_pos);
			}
		}
		mesh->set_vertex(current_vh, new_pos);
		V_ENT_PTR[current_vh] = (unsigned long)close_V;
		if (vhs_candidate.find(current_vh) != vhs_candidate.end())
		{
			vhs_candidate.erase(current_vh);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	foreach(auto vh, vhs_candidate){
		ENTITY* close_E;
		double min_dis(10e7);
		//if (vh == 824)
		//{
		//	std::cout<<mesh->vertex(vh)[0]<<" "<<mesh->vertex(vh)[1]<<" "<<mesh->vertex(vh)[2]<<std::endl;
		//	for (int i = 0;i < faces_list.count();i++)
		//	{
		//		if (is_cylindrical_face(faces_list[i]))
		//		{
		//			SPAposition c_pos;
		//			double dist = 0.0f;
		//			SPAposition s_pos = POS2SPA(mesh->vertex (vh));
		//			api_entity_point_distance(faces_list[i],s_pos,c_pos,dist);
		//			OvmVec3d current_vh = SPA2POS(c_pos);

		//			std::cout<<dist<<": "<<current_vh[0]<<" "<<current_vh[1]<<" "<<current_vh[2]<<std::endl;
		//		}

		//	}
		//}
		for(int i = 0; i < vertices_list.count(); i++){
			SPAposition c_pos;
			double dist = 0.0f;
			SPAposition s_pos = POS2SPA(mesh->vertex (vh));
			api_entity_point_distance(vertices_list[i],s_pos,c_pos,dist);
			//if (vh == 878)
			//{
			//	std::cout<<"878 dist "<<dist<<std::endl;
			//}
			if(dist < min_dis){
				min_dis = dist;
				close_E = vertices_list[i];
			}
		}
		for(int i = 0; i < edges_list.count(); i++){
			SPAposition c_pos;
			double dist = 0.0f;
			SPAposition s_pos = POS2SPA(mesh->vertex (vh));
			api_entity_point_distance(edges_list[i],s_pos,c_pos,dist);
			if(dist < min_dis){
				min_dis = dist;
				close_E = edges_list[i];
			}
		}
		for(int i = 0; i < faces_list.count(); i++){
			SPAposition c_pos;
			double dist = 0.0f;
			SPAposition s_pos = POS2SPA(mesh->vertex (vh));
			api_entity_point_distance(faces_list[i],s_pos,c_pos,dist);

			if(dist < min_dis){
				min_dis = dist;
				close_E = faces_list[i];
			}
		}
		SPAposition c_pos;
		double dist = 0.0f;
		SPAposition s_pos = POS2SPA(mesh->vertex (vh));
		api_entity_point_distance(close_E,s_pos,c_pos,dist);
		mesh->set_vertex(vh,SPA2POS(c_pos));
		V_ENT_PTR[vh] = (unsigned long)close_E;
	}
	//OvmVec3d mid_pos = (mesh->vertex(OvmVeH(812))+mesh->vertex(OvmVeH(836)))/2.0;
	//std::cout<<"mid pos"<<mid_pos[0]<<" "<<mid_pos[1]<<" "<<mid_pos[2]<<std::endl;
	mesh->set_persistent (V_ENT_PTR);
}
void attach_mesh_elements_to_ACIS_entities_of_tre2hex (VolumeMesh *mesh, BODY *body, double myresabs)
{
	std::ifstream file;
	file.open("boundary_tetrahedron_vertex.txt");
	std::unordered_set<int> boundary_tetrahedron_vertex_idx;
	int i;
	while(file>>i)
		boundary_tetrahedron_vertex_idx.insert(i);

	ENTITY_LIST vertices_list, edges_list, faces_list;
	api_get_vertices (body, vertices_list);
	api_get_edges (body, edges_list);
	api_get_faces (body, faces_list);
	SPAposition closest_pos;
	double dist = 0.0f;
	auto fOnWhichEntity = [&myresabs, &closest_pos, &dist](ENTITY_LIST &entity_list, SPAposition &pos)->ENTITY*{
		for (int i = 0; i != entity_list.count (); ++i){
			api_entity_point_distance (entity_list[i], pos, closest_pos, dist);
			if (dist < myresabs)
				return entity_list[i];
		}
		return NULL;
	};

	std::unordered_set<OpenVolumeMesh::VertexHandle> original_handle_vh;
	std::unordered_set<OpenVolumeMesh::VertexHandle> reoptimize_vh;
	auto V_ENT_PTR = mesh->request_vertex_property<unsigned long> ("entityptr", 0);
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
	{
		auto vh = *v_it;
		if(boundary_tetrahedron_vertex_idx.find(vh.idx()) != boundary_tetrahedron_vertex_idx.end())
		{
			original_handle_vh.insert(vh);
			SPAposition spa_pos = POS2SPA(mesh->vertex (*v_it));
			ENTITY* on_this_vertex = fOnWhichEntity (vertices_list, spa_pos);
			if (on_this_vertex)
				V_ENT_PTR[*v_it] = (unsigned long)on_this_vertex;
			else{
				ENTITY* on_this_edge = fOnWhichEntity (edges_list, spa_pos);
				if (on_this_edge)
					V_ENT_PTR[*v_it] = (unsigned long)on_this_edge;
				else{
					ENTITY* on_this_face = fOnWhichEntity (faces_list, spa_pos);
					if (on_this_face)
						V_ENT_PTR[*v_it] = (unsigned long)on_this_face;
					else{
						QMessageBox::critical (NULL, QObject::tr("错误!"), QObject::tr("这个点找不到对应的ACIS Entity!"));
						assert (false);
					}
				}//end if (on_this_edge)
			}//end if (on_this_vertex)
		}
	}

	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
	{
		if (!mesh->is_boundary (*v_it))
			continue;
		if(V_ENT_PTR[*v_it])
			continue;
		ENTITY_LIST adj_entity;
		std::unordered_set<OpenVolumeMesh::VertexHandle> adj_vh;
		get_adj_boundary_vertices_around_vertex(mesh, *v_it, adj_vh);
		foreach(auto & vh, adj_vh)
			if(original_handle_vh.find(vh) != original_handle_vh.end()){
				ENTITY * entity = (ENTITY *)V_ENT_PTR[vh];
				adj_entity.add(entity);
			}
		if(adj_entity.count() == 0)
			continue;
		if(adj_entity.count() > 2){
			QMessageBox::critical (NULL, QObject::tr("错误!"), QObject::tr("这个点对应的ACIS Entity出现错误!"));
			assert (false);
		}
		reoptimize_vh.insert(*v_it);
		if(adj_entity.count() == 1){
			V_ENT_PTR[*v_it] = (unsigned long) adj_entity[0];
			continue;
		}
		if(is_FACE(adj_entity[0]))
			V_ENT_PTR[*v_it] = (unsigned long) adj_entity[0];
		else if(is_FACE(adj_entity[1]))
			V_ENT_PTR[*v_it] = (unsigned long) adj_entity[1];
		else if(is_EDGE(adj_entity[0]) && is_EDGE(adj_entity[1]) && (adj_entity[0] == adj_entity[1]))
			V_ENT_PTR[*v_it] = (unsigned long) adj_entity[0];
		else if(is_EDGE(adj_entity[0]) && is_EDGE(adj_entity[1])  && (adj_entity[0] != adj_entity[1])){
			ENTITY_LIST  adj_face1;
			ENTITY_LIST  adj_face2;
			api_get_faces(adj_entity[0], adj_face1);
			api_get_faces(adj_entity[1], adj_face2);
			bool is_ok = false;
			for(int i = 0; i < adj_face1.count(); i++)
				if(adj_face2.lookup(adj_face1[i]) != -1){
					V_ENT_PTR[*v_it] = (unsigned long) adj_face1[i];
					is_ok = true;
					break;
				}
			if(is_ok == false){
				QMessageBox::critical (NULL, QObject::tr("错误!"), QObject::tr("两点之间的点找不到对应的ACIS Entity!"));
				assert (false);
			}
		}
		else if(is_EDGE(adj_entity[0]) && is_VERTEX(adj_entity[1])){
			ENTITY_LIST  adj_vertex;
			bool is_ok = false;
			api_get_vertices(adj_entity[0], adj_vertex);
			for(int i = 0; i < adj_vertex.count(); i++)
				if(adj_vertex.lookup(adj_entity[1]) != -1){
					V_ENT_PTR[*v_it] = (unsigned long) adj_entity[0];
					is_ok = true;
					break;
				}
			if(is_ok == false){
				ENTITY_LIST  adj_face1;
				ENTITY_LIST  adj_face2;
				api_get_faces(adj_entity[0], adj_face1);
				api_get_faces(adj_entity[1], adj_face2);
				for(int i = 0; i < adj_face1.count(); i++){
					if(adj_face2.lookup(adj_face1[i]) != -1){
						V_ENT_PTR[*v_it] = (unsigned long) adj_face1[i];
						is_ok = true;
						break;
					}
				}
			}
			if(is_ok == false){
				QMessageBox::critical (NULL, QObject::tr("错误!"), QObject::tr("两点之间的点找不到对应的ACIS Entity!!"));
				assert (false);
			}
		}
		else if(is_EDGE(adj_entity[1]) && is_VERTEX(adj_entity[0])){
			ENTITY_LIST  adj_vertex;
			bool is_ok = false;
			api_get_vertices(adj_entity[1], adj_vertex);
			for(int i = 0; i < adj_vertex.count(); i++)
				if(adj_vertex.lookup(adj_entity[0]) != -1){
					V_ENT_PTR[*v_it] = (unsigned long) adj_entity[1];
					is_ok = true;
					break;
				}
			if(is_ok == false){
				ENTITY_LIST  adj_face1;
				ENTITY_LIST  adj_face2;
				api_get_faces(adj_entity[0], adj_face1);
				api_get_faces(adj_entity[1], adj_face2);
				for(int i = 0; i < adj_face1.count(); i++)
					if(adj_face2.lookup(adj_face1[i]) != -1){
						V_ENT_PTR[*v_it] = (unsigned long) adj_face1[i];
						is_ok = true;
						break;
					}
			}
			if(is_ok == false){
				QMessageBox::critical (NULL, QObject::tr("错误!"), QObject::tr("两点之间的点找不到对应的ACIS Entity!!!"));
				assert (false);
			}
		}
		else{
			ENTITY_LIST  adj_edge1;
			ENTITY_LIST  adj_edge2;
			api_get_edges(adj_entity[0], adj_edge1);
			api_get_edges(adj_entity[1], adj_edge2);
			bool is_ok = false;
			for(int i = 0; i < adj_edge1.count(); i++)
				if(adj_edge2.lookup(adj_edge1[i]) != -1){
					V_ENT_PTR[*v_it] = (unsigned long) adj_edge1[i];
					is_ok = true;
					break;
				}
			if(is_ok == false){
				ENTITY_LIST  adj_face1;
				ENTITY_LIST  adj_face2;
				api_get_faces(adj_entity[0], adj_face1);
				api_get_faces(adj_entity[1], adj_face2);
				bool is_ok = false;
				for(int i = 0; i < adj_face1.count(); i++)
					if(adj_face2.lookup(adj_face1[i]) != -1){
						V_ENT_PTR[*v_it] = (unsigned long) adj_face1[i];
						is_ok = true;
						break;
					}
				if(is_ok == false){
					QMessageBox::critical (NULL, QObject::tr("错误!"), QObject::tr("两点之间的点找不到对应的ACIS Entity!!!!"));
					assert (false);
				}
			}
		}
	}

	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
	{
		if (!mesh->is_boundary (*v_it))
			continue;
		if(V_ENT_PTR[*v_it])
			continue;
		std::unordered_set<OpenVolumeMesh::VertexHandle> adj_vh;
		get_adj_boundary_vertices_around_vertex(mesh, *v_it, adj_vh);
		bool is_ok = false;
		foreach(auto & vh, adj_vh){
			ENTITY * entity = (ENTITY *)V_ENT_PTR[vh];
			if(is_FACE(entity)){
				V_ENT_PTR[*v_it] = V_ENT_PTR[vh];
				reoptimize_vh.insert(*v_it);
				is_ok = true;
				break;
			}
		}
		if(is_ok == false)
		{
			QMessageBox::critical (NULL, QObject::tr("错误!"), QObject::tr("这个点找不到对应的ACIS Entity!!"));
			assert (false);
		}
	}

	foreach(auto & vh, reoptimize_vh){
		ENTITY *entity = (ENTITY*)(V_ENT_PTR[vh]);
		SPAposition c_pos;
		double dist = 0.0f;
		SPAposition s_pos = POS2SPA(mesh->vertex (vh));
		api_entity_point_distance(entity,s_pos,c_pos,dist);
		mesh->set_vertex(vh,SPA2POS(c_pos));
	}
	mesh->set_persistent (V_ENT_PTR);
}

bool is_eh_belong_to_FACE(VolumeMesh * mesh,OvmEgH eh){
	if(!mesh->is_boundary(eh))
		return false;

	assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

	auto vh1 = mesh->edge (eh).from_vertex(),
		vh2 = mesh->edge (eh).to_vertex();
	auto entity1 = (ENTITY*)(V_ENTITY_PTR[vh1]),
		entity2 = (ENTITY*)(V_ENTITY_PTR[vh2]);

	if(is_FACE(entity1) || is_FACE(entity2))
		return true;
	if(is_EDGE(entity1) && is_EDGE(entity2) && (entity1 != entity2))
		return true;
	if(is_EDGE(entity1) && is_VERTEX(entity2)){
		ENTITY_LIST vertex_list;
		api_get_vertices(entity1,vertex_list);
		if((entity2 == vertex_list[0]) || (entity2 == vertex_list[1]))
			return false;
		else
			return true;
	}
	if(is_EDGE(entity2) && is_VERTEX(entity1)){
		ENTITY_LIST vertex_list;
		api_get_vertices(entity2,vertex_list);
		if((entity1 == vertex_list[0]) || (entity1 == vertex_list[1]))
			return false;
		else
			return true;
	}
	return false;
}


bool is_eh_belong_to_FACE_gen_ver(VMesh * mesh,OvmEgH eh){
	if(!mesh->is_boundary(eh))
		return false;

	assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

	auto vh1 = mesh->edge (eh).from_vertex(),
		vh2 = mesh->edge (eh).to_vertex();
	auto entity1 = (ENTITY*)(V_ENTITY_PTR[vh1]),
		entity2 = (ENTITY*)(V_ENTITY_PTR[vh2]);

	if(is_FACE(entity1) || is_FACE(entity2))
		return true;
	if(is_EDGE(entity1) && is_EDGE(entity2) && (entity1 != entity2))
		return true;
	if(is_EDGE(entity1) && is_VERTEX(entity2)){
		ENTITY_LIST vertex_list;
		api_get_vertices(entity1,vertex_list);
		if((entity2 == vertex_list[0]) || (entity2 == vertex_list[1]))
			return false;
		else
			return true;
	}
	if(is_EDGE(entity2) && is_VERTEX(entity1)){
		ENTITY_LIST vertex_list;
		api_get_vertices(entity2,vertex_list);
		if((entity1 == vertex_list[0]) || (entity1 == vertex_list[1]))
			return false;
		else
			return true;
	}
	return false;
}

void render_volume_mesh (VolumeMesh *mesh)
{
	std::unordered_set<int> bvs, bes, bfs, bcs;	//boundary elements
	std::unordered_set<int> ivs, ies, ifs, ics;	//inner elements
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
	{
		if (mesh->is_boundary (*v_it)) bvs.insert ((*v_it).idx ());
		else ivs.insert ((*v_it).idx ());
	}

	for (auto e_it = mesh->edges_begin (); e_it != mesh->edges_end (); ++e_it)
	{
		if (mesh->is_boundary (*e_it)) bes.insert ((*e_it).idx ());
		else ies.insert ((*e_it).idx ());
	}
	for (auto f_it = mesh->faces_begin (); f_it != mesh->faces_end (); ++f_it){
		if (mesh->is_boundary (*f_it)) bfs.insert ((*f_it).idx ());
		else ifs.insert ((*f_it).idx ());
	}
	for (auto c_it = mesh->cells_begin (); c_it != mesh->cells_end (); ++c_it){
		auto hfh_vec = mesh->cell (*c_it).halffaces ();
		bool is_bound = false;
		foreach (auto hfh, hfh_vec){
			if (mesh->is_boundary (mesh->face_handle (hfh))){
				bcs.insert ((*c_it).idx ());
				is_bound = true;
				break;
			}
		}
		if (!is_bound)
			ics.insert ((*c_it).idx ());
	}

	double cell_shrink_ratio = 0.6;

	char text_buf[20];
	auto fRenderWorker = [&mesh, &text_buf, &cell_shrink_ratio] (std::unordered_set<int> &vs, 
		std::unordered_set<int> &es, std::unordered_set<int> &fs, std::unordered_set<int> &cs){
//////////////////////////////////////////////////////////////////////////
				HC_Open_Segment ("meshvertices");{
			foreach (int idx, vs){
			OvmVeH vh = OvmVeH (idx);
			auto pt = mesh->vertex (vh);
			HC_KEY old_key = HC_Insert_Marker (pt[0], pt[1], pt[2]);
			HC_Open_Geometry (old_key);{
			HC_Set_User_Data (0, &idx, sizeof (int));
			}HC_Close_Geometry ();

			HC_Open_Segment ("indices");{
			HC_Set_Visibility ("everything=off");
			sprintf (text_buf, "v%d", idx);
			HC_Insert_Text (pt[0], pt[1], pt[2], text_buf);
			}HC_Close_Segment ();
			}
			}HC_Close_Segment ();
			//////////////////////////////////////////////////////////////////////////
			HC_Open_Segment ("meshedges");{
				//HC_Set_Visibility ("lines=off,edges=off,faces=off");
				foreach (int idx, es){

					//HC_Set_Line_Weight(2);

					OvmEgH eh = OvmEgH (idx);
					auto eg = mesh->edge (eh);
					auto pt1 = mesh->vertex (eg.from_vertex ()), pt2 = mesh->vertex (eg.to_vertex ());
					HC_KEY old_key = HC_Insert_Line (pt1[0], pt1[1], pt1[2], pt2[0], pt2[1], pt2[2]);

					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					HC_Open_Segment ("indices");{
						HC_Set_Visibility ("everything=off");
						sprintf (text_buf, "e%d", idx);
						auto mid = (pt1 + pt2) / 2;
						HC_Insert_Text (mid[0], mid[1], mid[2], text_buf);
					}HC_Close_Segment ();
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("meshfaces");{
				HC_Set_Color("cyan");
				//HC_Set_Visibility ("lines=off,edges=off,faces=on");
				//HC_Set_Selectability ("faces=on!");
				//HC_Set_Face_Pattern ("##");
				foreach (int idx, fs){
					OvmFaH fh = OvmFaH (idx);
					auto f = mesh->face (fh);
					auto hfh = mesh->halfface_handle (idx, 0);
					if (!mesh->is_boundary (hfh))
						hfh = mesh->opposite_halfface_handle (hfh);

					QVector<HPoint> pts;
					OvmVec3d centre = OvmVec3d (0, 0, 0);
					for (auto fv_it = mesh->hfv_iter (hfh); fv_it; ++fv_it)
					{
						auto pt = mesh->vertex (*fv_it);
						centre += pt;
						pts.push_back (HPoint (pt[0], pt[1], pt[2]));
					}
					HC_KEY old_key = HC_Insert_Polygon (pts.size (), pts.data ());
					centre /= pts.size ();
					//assert (*f_it > 0);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					HC_Open_Segment ("indices");{
						HC_Set_Visibility ("everything=off");
						sprintf (text_buf, "f%d", idx);
						HC_Insert_Text (centre[0], centre[1], centre[2], text_buf);
					}HC_Close_Segment ();
				}
			}HC_Close_Segment ();

			HC_Open_Segment ("meshcells");{
				HC_Set_Rendering_Options ("no lighting interpolation");
				HC_Set_Color ("edges=black");
				foreach (int c_idx, cs){
					OvmCeH ch = OvmCeH (c_idx);
					auto centre_pos = mesh->barycenter (ch);
					//HC_KEY old_key = HC_Insert_Marker (centre_pos[0], centre_pos[1], centre_pos[2]);
					QVector<HPoint> pts;
					int count = 0;
					QMap<OvmVeH, int> indices_map;
					for (auto hv_it = mesh->hv_iter (ch); hv_it; ++hv_it, ++count){
						auto pt = mesh->vertex (*hv_it);
						auto new_pt = centre_pos + (pt - centre_pos) * cell_shrink_ratio;
						pts.push_back (HPoint (new_pt[0], new_pt[1], new_pt[2]));
						indices_map.insert (*hv_it, count);
					}

					auto hfh_vec = mesh->cell (ch).halffaces ();
					QVector<int> face_list;
					foreach (OvmHaFaH hfh, hfh_vec){
						auto heh_vec = mesh->halfface (hfh).halfedges ();
						face_list.push_back (heh_vec.size ());
						foreach (OvmHaEgH heh, heh_vec){
							auto vh = mesh->halfedge (heh).to_vertex ();
							face_list.push_back (indices_map[vh]);
						}
					}
					HC_KEY old_key = HC_Insert_Shell (pts.size (), pts.data (), face_list.size (), face_list.data ());
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &c_idx, sizeof (int));
					}HC_Close_Geometry ();
					HC_Open_Segment ("indices");{
						HC_Set_Visibility ("everything=off");
						sprintf (text_buf, "c%d", c_idx);
						HC_Insert_Text (pts.front ().x, pts.front ().y, pts.front ().z, text_buf);
					}HC_Close_Segment ();
				}
			}HC_Close_Segment ();
	};
	//flush all the contents in this segment using wild cards
	HC_Flush_Segment ("...");

	HC_Open_Segment ("boundary");{
		fRenderWorker (bvs, bes, bfs, bcs);
	}HC_Close_Segment ();


	HC_Open_Segment ("inner");{
		fRenderWorker (ivs, ies, ifs, ics);
	}HC_Close_Segment ();
}

void render_volume_mesh_with_no_vertices (VolumeMesh *mesh)
{
	std::unordered_set<int> bvs, bes, bfs, bcs;	//boundary elements
	std::unordered_set<int> ivs, ies, ifs, ics;	//inner elements
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
	{
		if (mesh->is_boundary (*v_it)) bvs.insert ((*v_it).idx ());
		else ivs.insert ((*v_it).idx ());
	}

	for (auto e_it = mesh->edges_begin (); e_it != mesh->edges_end (); ++e_it)
	{
		if (mesh->is_boundary (*e_it)) bes.insert ((*e_it).idx ());
		else ies.insert ((*e_it).idx ());
	}
	for (auto f_it = mesh->faces_begin (); f_it != mesh->faces_end (); ++f_it){
		if (mesh->is_boundary (*f_it)) bfs.insert ((*f_it).idx ());
		else ifs.insert ((*f_it).idx ());
	}
	for (auto c_it = mesh->cells_begin (); c_it != mesh->cells_end (); ++c_it){
		auto hfh_vec = mesh->cell (*c_it).halffaces ();
		bool is_bound = false;
		foreach (auto hfh, hfh_vec){
			if (mesh->is_boundary (mesh->face_handle (hfh))){
				bcs.insert ((*c_it).idx ());
				is_bound = true;
				break;
			}
		}
		if (!is_bound)
			ics.insert ((*c_it).idx ());
	}

	double cell_shrink_ratio = 0.6;

	char text_buf[20];
	auto fRenderWorker = [&mesh, &text_buf, &cell_shrink_ratio] (std::unordered_set<int> &vs, 
		std::unordered_set<int> &es, std::unordered_set<int> &fs, std::unordered_set<int> &cs){
			//////////////////////////////////////////////////////////////////////////
			//HC_Open_Segment ("meshvertices");{
			//	foreach (int idx, vs){
			//		OvmVeH vh = OvmVeH (idx);
			//		auto pt = mesh->vertex (vh);
			//		HC_KEY old_key = HC_Insert_Marker (pt[0], pt[1], pt[2]);
			//		HC_Open_Geometry (old_key);{
			//			HC_Set_User_Data (0, &idx, sizeof (int));
			//		}HC_Close_Geometry ();

			//		HC_Open_Segment ("indices");{
			//			HC_Set_Visibility ("everything=off");
			//			sprintf (text_buf, "v%d", idx);
			//			HC_Insert_Text (pt[0], pt[1], pt[2], text_buf);
			//		}HC_Close_Segment ();
			//	}
			//}HC_Close_Segment ();
			//////////////////////////////////////////////////////////////////////////
			HC_Open_Segment ("meshedges");{
				//HC_Set_Visibility ("lines=off,edges=off,faces=off");
				foreach (int idx, es){

					//HC_Set_Line_Weight(2);

					OvmEgH eh = OvmEgH (idx);
					auto eg = mesh->edge (eh);
					auto pt1 = mesh->vertex (eg.from_vertex ()), pt2 = mesh->vertex (eg.to_vertex ());
					HC_KEY old_key = HC_Insert_Line (pt1[0], pt1[1], pt1[2], pt2[0], pt2[1], pt2[2]);

					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					HC_Open_Segment ("indices");{
						HC_Set_Visibility ("everything=off");
						sprintf (text_buf, "e%d", idx);
						auto mid = (pt1 + pt2) / 2;
						HC_Insert_Text (mid[0], mid[1], mid[2], text_buf);
					}HC_Close_Segment ();
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("meshfaces");{
				//HC_Set_Visibility ("lines=off,edges=off,faces=on");
				//HC_Set_Selectability ("faces=on!");
				//HC_Set_Face_Pattern ("##");
				foreach (int idx, fs){
					OvmFaH fh = OvmFaH (idx);
					auto f = mesh->face (fh);
					auto hfh = mesh->halfface_handle (idx, 0);
					if (!mesh->is_boundary (hfh))
						hfh = mesh->opposite_halfface_handle (hfh);

					QVector<HPoint> pts;
					OvmVec3d centre = OvmVec3d (0, 0, 0);
					for (auto fv_it = mesh->hfv_iter (hfh); fv_it; ++fv_it)
					{
						auto pt = mesh->vertex (*fv_it);
						centre += pt;
						pts.push_back (HPoint (pt[0], pt[1], pt[2]));
					}
					HC_KEY old_key = HC_Insert_Polygon (pts.size (), pts.data ());
					centre /= pts.size ();
					//assert (*f_it > 0);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					HC_Open_Segment ("indices");{
						HC_Set_Visibility ("everything=off");
						sprintf (text_buf, "f%d", idx);
						HC_Insert_Text (centre[0], centre[1], centre[2], text_buf);
					}HC_Close_Segment ();
				}
			}HC_Close_Segment ();

			HC_Open_Segment ("meshcells");{
				HC_Set_Rendering_Options ("no lighting interpolation");
				HC_Set_Color ("edges=black");
				foreach (int c_idx, cs){
					OvmCeH ch = OvmCeH (c_idx);
					auto centre_pos = mesh->barycenter (ch);
					//HC_KEY old_key = HC_Insert_Marker (centre_pos[0], centre_pos[1], centre_pos[2]);
					QVector<HPoint> pts;
					int count = 0;
					QMap<OvmVeH, int> indices_map;
					for (auto hv_it = mesh->hv_iter (ch); hv_it; ++hv_it, ++count){
						auto pt = mesh->vertex (*hv_it);
						auto new_pt = centre_pos + (pt - centre_pos) * cell_shrink_ratio;
						pts.push_back (HPoint (new_pt[0], new_pt[1], new_pt[2]));
						indices_map.insert (*hv_it, count);
					}

					auto hfh_vec = mesh->cell (ch).halffaces ();
					QVector<int> face_list;
					foreach (OvmHaFaH hfh, hfh_vec){
						auto heh_vec = mesh->halfface (hfh).halfedges ();
						face_list.push_back (heh_vec.size ());
						foreach (OvmHaEgH heh, heh_vec){
							auto vh = mesh->halfedge (heh).to_vertex ();
							face_list.push_back (indices_map[vh]);
						}
					}
					HC_KEY old_key = HC_Insert_Shell (pts.size (), pts.data (), face_list.size (), face_list.data ());
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &c_idx, sizeof (int));
					}HC_Close_Geometry ();
					HC_Open_Segment ("indices");{
						HC_Set_Visibility ("everything=off");
						sprintf (text_buf, "c%d", c_idx);
						HC_Insert_Text (pts.front ().x, pts.front ().y, pts.front ().z, text_buf);
					}HC_Close_Segment ();
				}
			}HC_Close_Segment ();
	};
	//flush all the contents in this segment using wild cards
	HC_Flush_Segment ("...");

	HC_Open_Segment ("boundary");{
		fRenderWorker (bvs, bes, bfs, bcs);
	}HC_Close_Segment ();


	HC_Open_Segment ("inner");{
		fRenderWorker (ivs, ies, ifs, ics);
	}HC_Close_Segment ();
}

void render_tet_mesh(VMesh *mesh)
{
	std::unordered_set<int> bvs, bes, bfs, bcs;	//boundary elements
	std::unordered_set<int> ivs, ies, ifs, ics;	//inner elements
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
	{
		if (mesh->is_boundary (*v_it)) bvs.insert ((*v_it).idx ());
		else ivs.insert ((*v_it).idx ());
	}

	for (auto e_it = mesh->edges_begin (); e_it != mesh->edges_end (); ++e_it)
	{
		if (mesh->is_boundary (*e_it)) bes.insert ((*e_it).idx ());
		else ies.insert ((*e_it).idx ());
	}
	for (auto f_it = mesh->faces_begin (); f_it != mesh->faces_end (); ++f_it){
		if (mesh->is_boundary (*f_it)) bfs.insert ((*f_it).idx ());
		else ifs.insert ((*f_it).idx ());
	}
	for (auto c_it = mesh->cells_begin (); c_it != mesh->cells_end (); ++c_it){
		auto hfh_vec = mesh->cell (*c_it).halffaces ();
		bool is_bound = false;
		foreach (auto hfh, hfh_vec){
			if (mesh->is_boundary (mesh->face_handle (hfh))){
				bcs.insert ((*c_it).idx ());
				is_bound = true;
				break;
			}
		}
		if (!is_bound)
			ics.insert ((*c_it).idx ());
	}

	double cell_shrink_ratio = 0.6;

	char text_buf[20];
	auto fRenderWorker = [&mesh, &text_buf, &cell_shrink_ratio] (std::unordered_set<int> &vs, 
		std::unordered_set<int> &es, std::unordered_set<int> &fs, std::unordered_set<int> &cs){
			//////////////////////////////////////////////////////////////////////////
			HC_Open_Segment ("meshvertices");{
				foreach (int idx, vs){
					OvmVeH vh = OvmVeH (idx);
					auto pt = mesh->vertex (vh);
					HC_KEY old_key = HC_Insert_Marker (pt[0], pt[1], pt[2]);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();

					HC_Open_Segment ("indices");{
						HC_Set_Visibility ("everything=off");
						sprintf (text_buf, "v%d", idx);
						HC_Insert_Text (pt[0], pt[1], pt[2], text_buf);
					}HC_Close_Segment ();
				}
			}HC_Close_Segment ();
			//////////////////////////////////////////////////////////////////////////
			HC_Open_Segment ("meshedges");{
				//HC_Set_Visibility ("lines=off,edges=off,faces=off");
				foreach (int idx, es){

					//HC_Set_Line_Weight(2);

					OvmEgH eh = OvmEgH (idx);
					auto eg = mesh->edge (eh);
					auto pt1 = mesh->vertex (eg.from_vertex ()), pt2 = mesh->vertex (eg.to_vertex ());
					HC_KEY old_key = HC_Insert_Line (pt1[0], pt1[1], pt1[2], pt2[0], pt2[1], pt2[2]);

					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					HC_Open_Segment ("indices");{
						HC_Set_Visibility ("everything=off");
						sprintf (text_buf, "e%d", idx);
						auto mid = (pt1 + pt2) / 2;
						HC_Insert_Text (mid[0], mid[1], mid[2], text_buf);
					}HC_Close_Segment ();
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("meshfaces");{
				HC_Set_Color("cyan");
				//HC_Set_Visibility ("lines=off,edges=off,faces=on");
				//HC_Set_Selectability ("faces=on!");
				//HC_Set_Face_Pattern ("##");
				foreach (int idx, fs){
					OvmFaH fh = OvmFaH (idx);
					auto f = mesh->face (fh);
					auto hfh = mesh->halfface_handle (idx, 0);
					if (!mesh->is_boundary (hfh))
						hfh = mesh->opposite_halfface_handle (hfh);

					QVector<HPoint> pts;
					OvmVec3d centre = OvmVec3d (0, 0, 0);
					for (auto fv_it = mesh->hfv_iter (hfh); fv_it; ++fv_it)
					{
						auto pt = mesh->vertex (*fv_it);
						centre += pt;
						pts.push_back (HPoint (pt[0], pt[1], pt[2]));
					}
					HC_KEY old_key = HC_Insert_Polygon (pts.size (), pts.data ());
					centre /= pts.size ();
					//assert (*f_it > 0);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					HC_Open_Segment ("indices");{
						HC_Set_Visibility ("everything=off");
						sprintf (text_buf, "f%d", idx);
						HC_Insert_Text (centre[0], centre[1], centre[2], text_buf);
					}HC_Close_Segment ();
				}
			}HC_Close_Segment ();

			HC_Open_Segment ("meshcells");{
				HC_Set_Rendering_Options ("no lighting interpolation");
				HC_Set_Color ("edges=black");
				foreach (int c_idx, cs){
					OvmCeH ch = OvmCeH (c_idx);
					auto centre_pos = mesh->barycenter (ch);
					//HC_KEY old_key = HC_Insert_Marker (centre_pos[0], centre_pos[1], centre_pos[2]);
					QVector<HPoint> pts;
					int count = 0;
					QMap<OvmVeH, int> indices_map;
					for (auto cv_it = mesh->cv_iter(ch); cv_it;++cv_it,++count)
					{
						auto pt = mesh->vertex(*cv_it);
						auto new_pt = centre_pos + (pt -centre_pos) * cell_shrink_ratio;
						pts.push_back(HPoint (new_pt[0], new_pt[1], new_pt[2]));
						indices_map.insert(*cv_it, count);
					}

					auto hfh_vec = mesh->cell (ch).halffaces ();
					QVector<int> face_list;
					foreach (OvmHaFaH hfh, hfh_vec){
						auto heh_vec = mesh->halfface (hfh).halfedges ();
						face_list.push_back (heh_vec.size ());
						foreach (OvmHaEgH heh, heh_vec){
							auto vh = mesh->halfedge (heh).to_vertex ();
							face_list.push_back (indices_map[vh]);
						}
					}
					HC_KEY old_key = HC_Insert_Shell (pts.size (), pts.data (), face_list.size (), face_list.data ());
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &c_idx, sizeof (int));
					}HC_Close_Geometry ();
					HC_Open_Segment ("indices");{
						HC_Set_Visibility ("everything=off");
						sprintf (text_buf, "c%d", c_idx);
						HC_Insert_Text (pts.front ().x, pts.front ().y, pts.front ().z, text_buf);
					}HC_Close_Segment ();
				}
			}HC_Close_Segment ();
	};
	//flush all the contents in this segment using wild cards
	HC_Flush_Segment ("...");

	HC_Open_Segment ("boundary");{
		fRenderWorker (bvs, bes, bfs, bcs);
	}HC_Close_Segment ();


	HC_Open_Segment ("inner");{
		fRenderWorker (ivs, ies, ifs, ics);
	}HC_Close_Segment ();
}

void render_triangle_mesh(OP_Mesh *mesh)
{
	std::unordered_set<int> vs,es,fs;	//boundary elements
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
	{
		//vs.insert(v_it->idx());
	}

	for (auto e_it = mesh->edges_begin (); e_it != mesh->edges_end (); ++e_it)
	{
		if (!mesh->is_boundary(*e_it))
		{
			continue;
		}
		es.insert(e_it->idx());
	}
	for (auto f_it = mesh->faces_begin (); f_it != mesh->faces_end (); ++f_it){
		fs.insert(f_it->idx());
	}
	

	char text_buf[20];
	auto fRenderWorker = [&mesh, &text_buf] (std::unordered_set<int> &vs, 
		std::unordered_set<int> &es, std::unordered_set<int> &fs){
			HC_Open_Segment ("meshvertices");{
				foreach (int idx, vs){
					OmVeH vh = OmVeH (idx);
					auto pt =mesh->point(vh);
					HC_KEY old_key = HC_Insert_Marker (pt[0], pt[1], pt[2]);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();

					HC_Open_Segment ("indices");{
						HC_Set_Visibility ("everything=off");
						sprintf (text_buf, "v%d", idx);
						HC_Insert_Text (pt[0], pt[1], pt[2], text_buf);
					}HC_Close_Segment ();
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("meshedges");{
				//HC_Set_Visibility ("lines=off,edges=off,faces=off");
				foreach (int idx, es){

					//HC_Set_Line_Weight(2);

					OmEgH eh = OmEgH (idx);
					auto pt1 = mesh->point (mesh->from_vertex_handle(mesh->halfedge_handle(eh,0))), pt2 = mesh->point (mesh->to_vertex_handle(mesh->halfedge_handle(eh,0)));
					HC_KEY old_key = HC_Insert_Line (pt1[0], pt1[1], pt1[2], pt2[0], pt2[1], pt2[2]);

					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					HC_Open_Segment ("indices");{
						HC_Set_Visibility ("everything=off");
						sprintf (text_buf, "e%d", idx);
						auto mid = (pt1 + pt2) / 2;
						HC_Insert_Text (mid[0], mid[1], mid[2], text_buf);
					}HC_Close_Segment ();
				}
			}HC_Close_Segment ();
			HC_Open_Segment ("meshfaces");{
				//HC_Set_Visibility ("lines=off,edges=off,faces=on");
				//HC_Set_Selectability ("faces=on!");
				//HC_Set_Face_Pattern ("##");
				foreach (int idx, fs){
					OmFaH fh = OmFaH (idx);

					QVector<HPoint> pts;
					OvmVec3d centre(0,0,0);
					for (auto fv_it = mesh->fv_iter (fh); fv_it; ++fv_it)
					{
						OP_Mesh::Point point_ = mesh->point(*fv_it);
						OvmVec3d pt(point_[0],point_[1],point_[2]);
						centre += pt;
						pts.push_back (HPoint (pt[0], pt[1], pt[2]));
					}
					HC_KEY old_key = HC_Insert_Polygon (pts.size (), pts.data ());
					centre /= pts.size ();
					//assert (*f_it > 0);
					HC_Open_Geometry (old_key);{
						HC_Set_User_Data (0, &idx, sizeof (int));
					}HC_Close_Geometry ();
					HC_Open_Segment ("indices");{
						HC_Set_Visibility ("everything=off");
						sprintf (text_buf, "f%d", idx);
						HC_Insert_Text (centre[0], centre[1], centre[2], text_buf);
					}HC_Close_Segment ();
				}
			}HC_Close_Segment ();
	};
	//flush all the contents in this segment using wild cards
	//HC_Flush_Segment ("...");

	HC_Open_Segment ("boundary");{
		fRenderWorker (vs, es, fs);
	}HC_Close_Segment ();
}

void render_mesh_group (VolumeMeshElementGroup *group)
{
}

//给定一个网格和body输入一条网格边OvmEgH返回这条边中点在这个模型上的落点
OvmVec3d find_ovmegh_mid_pos_in_body(VMesh* mesh, BODY* body, OvmEgH eh)
{
	//获取边界属性
	if (!mesh->vertex_property_exists<unsigned long>("entityptr"))
	{
		std::cout<<"Vertex_propert doesn't exist!!"<<std::endl;
		system("pause");
		return OvmVec3d(0,0,0);
	}
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
	
	OvmVeH svh = mesh->edge(eh).from_vertex();
	OvmVeH evh = mesh->edge(eh).to_vertex();
	OvmVec3d mid_pos = (mesh->vertex(svh)+mesh->vertex(evh))/2.0;
	//
	//std::cout<<"two vhs "<<svh<<" "<<evh<<std::endl;
	ENTITY* entity1 = (ENTITY*) V_ENTITY_PTR[svh];ENTITY* entity2 = (ENTITY*) V_ENTITY_PTR[evh];
	//std::cout<<is_VERTEX(entity1)<<" "<<is_EDGE(entity1)<<" "<<is_FACE(entity1)<<std::endl;
	//std::cout<<is_VERTEX(entity2)<<" "<<is_EDGE(entity2)<<" "<<is_FACE(entity2)<<std::endl;
	SPAposition closet_p;double dis;
	//点点
	if (is_VERTEX(entity1) && is_VERTEX(entity2))
	{
		ENTITY_LIST V_ADJ_E1,V_ADJ_E2;
		ENTITY* e_entity = NULL;
		api_get_edges(entity1,V_ADJ_E1);
		api_get_edges(entity2,V_ADJ_E2);
		for (int e_i = 0;e_i < V_ADJ_E1.count();e_i++)
		{
			if (V_ADJ_E2.lookup(V_ADJ_E1[e_i]) != -1)
			{
				e_entity = V_ADJ_E1[e_i];
				break;
			}
		}
		if (e_entity == NULL)
		{
			ENTITY_LIST V_ADJ_F1,V_ADJ_F2;
			api_get_faces(entity1,V_ADJ_F1);
			api_get_faces(entity2,V_ADJ_F2);
			for (int fi = 0;fi < V_ADJ_F1.count();fi++)
			{
				if (V_ADJ_F2.lookup(V_ADJ_F1[fi]) != -1)
				{
					e_entity = V_ADJ_F1[fi];
					break;
				}
			}
		}
		api_entity_point_distance(e_entity,POS2SPA(mid_pos),closet_p,dis);
	}//点边
	else if (is_VERTEX(entity1) && is_EDGE(entity2))
	{
		ENTITY_LIST V_ADJ_E;
		api_get_edges(entity1, V_ADJ_E);
		if (V_ADJ_E.lookup(entity2) != -1)//即这点属于边
		{
			api_entity_point_distance(entity2,POS2SPA(mid_pos),closet_p,dis);
		}
		else//若点不属于边 则该点和边应该是有一条公共面
		{
			ENTITY_LIST V_ADJ_F,E_ADJ_F;
			ENTITY* f_entity;
			api_get_faces(entity1,V_ADJ_F);
			api_get_faces(entity2,E_ADJ_F);
			for (int vf_i = 0;vf_i < V_ADJ_F.count();vf_i++)
			{
				if (E_ADJ_F.lookup(V_ADJ_F[vf_i]) != -1)
				{
					f_entity = V_ADJ_F[vf_i];
					break;
				}
			}
			api_entity_point_distance(f_entity,POS2SPA(mid_pos),closet_p,dis);
		}
	}
	else if (is_EDGE(entity1) && is_VERTEX(entity2))
	{
		ENTITY_LIST V_ADJ_E;
		api_get_edges(entity2, V_ADJ_E);
		if (V_ADJ_E.lookup(entity1) != -1)//即这点属于边
		{
			api_entity_point_distance(entity1,POS2SPA(mid_pos),closet_p,dis);
		}
		else//若点不属于边 则该点和边应该是有一条公共面
		{
			ENTITY_LIST V_ADJ_F,E_ADJ_F;
			ENTITY* f_entity;
			api_get_faces(entity2,V_ADJ_F);
			api_get_faces(entity1,E_ADJ_F);
			for (int vf_i = 0;vf_i < V_ADJ_F.count();vf_i++)
			{
				if (E_ADJ_F.lookup(V_ADJ_F[vf_i]) != -1)
				{
					f_entity = V_ADJ_F[vf_i];
					break;
				}
			}
			api_entity_point_distance(f_entity,POS2SPA(mid_pos),closet_p,dis);
		}
	}
	else if (is_VERTEX(entity1) && is_FACE(entity2))
	{
		api_entity_point_distance(entity2,POS2SPA(mid_pos),closet_p,dis);
	}
	else if (is_FACE(entity1) && is_VERTEX(entity2))
	{
		api_entity_point_distance(entity1,POS2SPA(mid_pos),closet_p,dis);
	}
	else if (is_EDGE(entity1) && is_EDGE(entity2))
	{
		if (entity1 == entity2)
		{
			api_entity_point_distance(entity1,POS2SPA(mid_pos),closet_p,dis);
		}
		else
		{
			ENTITY_LIST E_ADJ_F1,E_ADJ_F2;
			api_get_faces(entity1,E_ADJ_F1);
			api_get_faces(entity2,E_ADJ_F2);
			ENTITY* f_entity;
			for (int ef_i = 0;ef_i < E_ADJ_F1.count();ef_i++)
			{
				if (E_ADJ_F2.lookup(E_ADJ_F1[ef_i]) != -1)
				{
					f_entity = E_ADJ_F1[ef_i];
					break;
				}
			}
			api_entity_point_distance(f_entity,POS2SPA(mid_pos),closet_p,dis);
		}
	}
	else if (is_EDGE(entity1) && is_FACE(entity2))
	{
		api_entity_point_distance(entity2,POS2SPA(mid_pos),closet_p,dis);
	}
	else if (is_FACE(entity1) && is_EDGE(entity2))
	{
		api_entity_point_distance(entity1,POS2SPA(mid_pos),closet_p,dis);
	}
	else if (is_FACE(entity1) && is_FACE(entity2))
	{
		if (entity1 == entity2)
		{
			api_entity_point_distance(entity1,POS2SPA(mid_pos),closet_p,dis);;
		}
		else
		{
			std::cout<<"Faces not equal!"<<std::endl;
			system("pause");
		}
	}
	else
	{
		std::cout<<"Entity wrong!"<<std::endl;
		system("pause");
	}
	mid_pos = SPA2POS(closet_p);
	return mid_pos;
}

HC_KEY insert_boundary_shell (VolumeMesh *mesh, std::unordered_set<OvmVeH> &bound_vhs,
	std::unordered_set<OvmHaFaH> &bound_hfhs)
{
	HC_KEY shell_key = INVALID_KEY;
	std::hash_map<OvmVeH, int> vhs_idx_mapping;
	HPoint * pts = new HPoint[bound_vhs.size ()];
	int idx = 0;
	foreach (auto &vh, bound_vhs){
		auto pos = mesh->vertex (vh);
		pts[idx] = HPoint (pos[0], pos[1], pos[2]);
		vhs_idx_mapping.insert (std::make_pair (vh, idx));
		idx++;
	}

	int *face_list = new int[5 * bound_hfhs.size ()];

	idx = 0;
	foreach (auto &hfh, bound_hfhs){
		face_list[idx++] = mesh->valence (mesh->face_handle (hfh));
		for (auto hfv_it = mesh->hfv_iter (hfh); hfv_it; ++hfv_it){
			face_list[idx++] = vhs_idx_mapping[*hfv_it];
		}
	}

	shell_key = HC_Insert_Shell (bound_vhs.size (), pts, idx, face_list);
	delete[] pts;
	delete[] face_list;
	return shell_key;
}

HC_KEY insert_boundary_shell (VolumeMesh *mesh)
{
	std::unordered_set<OvmFaH> bound_fhs;
	for (auto bf_it = mesh->bf_iter (); bf_it; ++bf_it)
		bound_fhs.insert (*bf_it);
	std::unordered_set<OvmVeH> bound_vhs;
	std::unordered_set<OvmHaFaH> bound_hfhs;
	foreach (auto &fh, bound_fhs){
		auto hfh = mesh->halfface_handle (fh, 0);
		if (!mesh->is_boundary (hfh))
			hfh = mesh->opposite_halfface_handle (hfh);
		bound_hfhs.insert (hfh);
		for (auto hfv_it = mesh->hfv_iter (hfh); hfv_it; ++hfv_it)
			bound_vhs.insert (*hfv_it);
	}

	return insert_boundary_shell (mesh, bound_vhs, bound_hfhs);
}

HC_KEY insert_boundary_shell (VolumeMesh *mesh, std::unordered_set<OvmCeH> &chs)
{
	std::unordered_set<OvmVeH> bound_vhs;
	std::unordered_set<OvmHaFaH> inner_bound_hfhs, out_bound_hfhs;
	collect_boundary_element (mesh, chs, &bound_vhs, NULL, &inner_bound_hfhs);
	foreach (auto &hfh, inner_bound_hfhs)
		out_bound_hfhs.insert (mesh->opposite_halfface_handle (hfh));
	return insert_boundary_shell (mesh, bound_vhs, out_bound_hfhs);
}



void get_piecewise_halfedges_from_edges (VolumeMesh *mesh, std::vector<OvmEgH> &ehs, bool forward, std::vector<OvmHaEgH> &hehs)
{
	auto heh = mesh->halfedge_handle (ehs.front (), 0);
	auto first_comm_vh = get_common_vertex_handle (mesh, ehs[0], ehs[1]);
	if (mesh->halfedge (heh).to_vertex () != first_comm_vh)
		heh = mesh->opposite_halfedge_handle (heh);

	hehs.clear ();
	hehs.push_back (heh);
	auto he = mesh->halfedge (heh);
	auto vh = he.to_vertex ();
	for (auto it = ehs.begin () + 1; it != ehs.end (); ++it)
	{
		heh = mesh->halfedge_handle (*it, 0);
		he = mesh->halfedge (heh);
		if (he.from_vertex () != vh)
		{
			heh = mesh->opposite_halfedge_handle (heh);
			he = mesh->halfedge (heh);
		}
		vh = he.to_vertex ();
		hehs.push_back (heh);
	}
	if (!forward){
		std::reverse (hehs.begin (), hehs.end ());
		for (int i = 0; i != hehs.size (); ++i)
			hehs[i] = mesh->opposite_halfedge_handle (hehs[i]);
	}
}

std::vector<OvmVeH> get_ordered_vhs_from_unordered_ehs_with_start_direction(VMesh *mesh, std::vector<OvmEgH> &ehs, OvmEgH start_eh, OvmVeH start_vh)
{
	std::vector<OvmVeH> ordered_vhs;
	std::unordered_set<OvmEgH> temp_ehs;
	for (auto e_it = ehs.begin();e_it != ehs.end();e_it++)
	{
		temp_ehs.insert(*e_it);
	}
	OvmVeH pre_vh,next_vh;
	pre_vh = start_vh;
	OvmEgH current_eh = start_eh;
	temp_ehs.erase(current_eh);ordered_vhs.push_back(start_vh);
	next_vh = (mesh->edge(start_eh).from_vertex() == pre_vh) ? mesh->edge(start_eh).to_vertex() : mesh->edge(start_eh).from_vertex();
	//std::cout<<"Before while loop pre vh and next vh "<<pre_vh<<" "<<next_vh<<std::endl;
	while(next_vh != start_vh)
	{
		//std::cout<<pre_vh<<" ";
		ordered_vhs.push_back(next_vh);
		pre_vh = next_vh;
		for (auto voh_it = mesh->voh_iter(next_vh);voh_it;voh_it++)
		{
			if (temp_ehs.find(mesh->edge_handle(*voh_it)) != temp_ehs.end())
			{
				current_eh = mesh->edge_handle(*voh_it);
			}
		}
		temp_ehs.erase(current_eh);
		next_vh = (mesh->edge(current_eh).from_vertex() == pre_vh) ? mesh->edge(current_eh).to_vertex() : mesh->edge(current_eh).from_vertex();
	}
	//std::cout<<pre_vh<<std::endl;
	return ordered_vhs;
}


std::vector<OvmFaH> get_adj_faces_around_cell (VolumeMesh *mesh, OvmCeH ch)
{
	std::vector<OvmFaH> fhs;
	foreach (auto &hfh, mesh->cell(ch).halffaces()) {
		OvmFaH fh = mesh->face_handle(hfh);
		fhs.push_back(fh);
	}
	return fhs;
}

std::vector<OvmHaEgH> get_ordered_hehs_from_ehs(VolumeMesh* mesh, std::vector<OvmEgH> ehs)
{
	std::vector<OvmHaEgH> ordered_hehs;
	std::vector<int> if_ehs(mesh->n_edges(),0);
	foreach (auto eh, ehs)
		if_ehs[eh.idx()] = 1;
	OvmHaEgH current_heh = mesh->halfedge_handle(ehs[0],0);
	ordered_hehs.push_back(current_heh);
	if_ehs[ehs[0].idx()] = 0;
	while (true)
	{
		OvmVeH end_vertex = mesh->halfedge(current_heh).to_vertex();
		OvmHaEgH next_heh = mesh->InvalidHalfEdgeHandle;
		for (auto voheit = mesh->voh_iter(end_vertex);voheit;voheit++)
		{
			OvmEgH current_eh = mesh->edge_handle(*voheit);
			if (if_ehs[current_eh.idx()] == 1)
			{
				if_ehs[current_eh.idx()] = 0;
				next_heh = *voheit;
				current_heh = next_heh;
				ordered_hehs.push_back(next_heh);
				break;
			}
		}
		if (next_heh == mesh->InvalidHalfEdgeHandle)
			break;
	}
	return ordered_hehs;
}


void fOvm2VTK(VMesh* mesh, std::string outfile_name)
{

	std::ofstream f(outfile_name.c_str());
	f.clear();
	/*f << "MeshVersionFormatted 1\n";
	f << "Dimension \n3\n";
	f << "Vertices\n";
	f << mesh->n_vertices() << '\n';*/
	f << "# vtk DataFile Version 2.0\n";
	f << "tetra\n";
	f << "ASCII\n";
	f << "DATASET UNSTRUCTURED_GRID\n";
	f << "POINTS " << mesh->n_vertices() << " float\n";
		
		
	OvmVec3d point;

	for (OpenVolumeMesh::VertexIter iter = mesh->vertices_begin(); iter != mesh->vertices_end(); iter++)
	{
		OpenVolumeMesh::VertexHandle v = *iter;
		point = mesh->vertex(v);
		f << point[0] << " " << point[1] << " " << point[2] << '\n'; 
	}

	


	f << "CELLS " << mesh->n_cells() << " " << mesh->n_cells() * 5 << "\n";


		

	for (auto c_iter = mesh->cells_begin(); c_iter != mesh->cells_end(); ++c_iter)
	{
		std::vector<OvmVeH> vhs;
		for (auto cvit = mesh->cv_iter(*c_iter); cvit; cvit++)
			vhs.push_back(*cvit);

		std::vector<OvmVeH> tmp_vhs; 
		auto adj_hfhs = mesh->cell(*c_iter).halffaces();
		for (auto hfv_it = mesh->hfv_iter(*adj_hfhs.begin());hfv_it;hfv_it++)
			tmp_vhs.push_back(*hfv_it);
		std::unordered_set<OvmVeH> us_vhs;
		us_vhs.insert(tmp_vhs.begin(), tmp_vhs.end());
		for (auto vit = vhs.begin();vit != vhs.end();vit++)
		{
			if (us_vhs.find(*vit) == us_vhs.end())
				tmp_vhs.push_back(*vit);
		}
		vhs.swap(tmp_vhs);
		//if (c_iter->idx() == 1)
		//	std::cout<<"Vertex id "<<vhs[0]<<" "<<vhs[1]<<" "<<vhs[2]<<" "<<vhs[3]<<std::endl;
		//	system("pause");
		f << 4 <<" "<<vhs[0]<<" "<<vhs[2]<<" "<<vhs[1]<<" "<<vhs[3]<<std::endl;
	}

	f << "CELL_TYPES "<< mesh->n_cells() << '\n';
	for (auto c_iter = mesh->cells_begin(); c_iter != mesh->cells_end(); ++c_iter)
	{		
		f << 10 << '\n';	
	}
	
	//f << "End";
		
	f.close();
		
	return;

}


