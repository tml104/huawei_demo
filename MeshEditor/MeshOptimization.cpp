#include "stdafx.h"
#include "MeshOptimization.h"
#include "FuncDefs.h"
#include "PrioritySetManager.h"
#include "hoopsview.h"
//void smooth_volume_mesh (VolumeMesh *mesh, BODY *body, int smooth_rounds)
//	{
//		//原来的smooth的程序
//		/*assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
//		auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
//		//对边的类型统计一下
//		std::hash_map<VERTEX*, OvmVeH> vertices_mapping;
//		std::hash_map<EDGE*, std::unordered_set<OvmVeH> > edges_vertices_mapping;
//		std::hash_map<FACE*, std::unordered_set<OvmVeH> > faces_vertices_mapping;
//		std::unordered_set<OvmVeH> volume_vhs, new_boundary_vhs;
//
//		ENTITY_LIST vertices_list, edges_list, faces_list;
//		api_get_vertices (body, vertices_list);
//		api_get_edges (body, edges_list);
//		api_get_faces (body, faces_list);
//
//		for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
//		{
//			auto entity_ptr_uint = V_ENTITY_PTR[*v_it];
//			if (entity_ptr_uint == 0) volume_vhs.insert (*v_it);
//			else if (entity_ptr_uint == -1){
//				//判断一下这个新点是否在体内，如果是的话，则将他的entity_ptr设置为0
//				if (!mesh->is_boundary (*v_it)){
//					V_ENTITY_PTR[*v_it] = 0;
//					volume_vhs.insert (*v_it);
//				}else
//					new_boundary_vhs.insert (*v_it);
//			}else{
//				ENTITY *entity = (ENTITY*)(entity_ptr_uint);
//				if (is_VERTEX (entity)) vertices_mapping[(VERTEX*)entity] = *v_it;
//				else if (is_EDGE (entity)) edges_vertices_mapping[(EDGE*)entity].insert (*v_it);
//				else faces_vertices_mapping[(FACE*)entity].insert (*v_it);
//			}
//		}
//
//		assert (vertices_list.count () == vertices_mapping.size ());
//
//		auto fDivideOneEdge = [&](EDGE *edge, int dis_num, std::vector<SPAposition> &pts){
//			SPAinterval inter = edge->param_range ();
//			double step = inter.length () / dis_num,
//				start_param = inter.start_pt ();
//			curve *crv = edge->geometry ()->trans_curve ();
//
//			for (int i = 0; i <= dis_num; ++i){
//				double param_val = start_param + step * i;
//				SPAposition pos;
//				crv->eval (param_val, pos);
//				pts.push_back (pos);
//			}
//		};
//
//		auto fSmoothAllEdges = [&](){
//			for (int i = 0; i != edges_list.count (); ++i){
//				EDGE *acis_edge = (EDGE*)edges_list[i];
//				VERTEX *acis_v1 = acis_edge->start (),
//					*acis_v2 = acis_edge->end ();
//				OvmVeH mesh_vh1 = vertices_mapping[acis_v1],
//					mesh_vh2 = vertices_mapping[acis_v2];
//				auto &mesh_vhs_set = edges_vertices_mapping[acis_edge];
//				mesh_vhs_set.insert (mesh_vh2);
//				std::vector<OvmVeH> sorted_vhs;
//				while (mesh_vh1 != mesh_vh2){
//					sorted_vhs.push_back (mesh_vh1);
//					OvmVeH next_vh = mesh->InvalidVertexHandle;
//					//首先在mesh_vhs_set中找下一个顶点
//					for (auto v_it = mesh_vhs_set.begin (); v_it != mesh_vhs_set.end (); ++v_it)
//					{
//						OvmHaEgH test_heh = mesh->halfedge (mesh_vh1, *v_it);
//						if (test_heh != mesh->InvalidHalfEdgeHandle){
//							next_vh = *v_it;
//							mesh_vhs_set.erase (next_vh);
//							break;
//						}
//					}
//					//如果在mesh_vhs_set中找不到，那么在new_vhs中继续找
//					if (next_vh == mesh->InvalidVertexHandle){
//						double min_dist = std::numeric_limits<double>::max ();
//						OvmVeH best_next_vh = OvmVeH (-1);
//						foreach (auto &vh, new_boundary_vhs){
//							OvmHaEgH test_heh = mesh->halfedge (mesh_vh1, vh);
//							if (test_heh != mesh->InvalidHalfEdgeHandle){
//								SPAposition closest_pt;
//								double dist = 0.0f;
//								OvmVec3d pos = mesh->vertex (vh);
//								api_entity_point_distance (acis_edge, POS2SPA(pos), closest_pt, dist);
//								if (dist < min_dist){
//									best_next_vh = vh;
//									min_dist = dist;
//								}
//							}
//						}
//						next_vh = best_next_vh;
//						assert (next_vh != mesh->InvalidVertexHandle);
//						new_boundary_vhs.erase (next_vh);
//						V_ENTITY_PTR[next_vh] = (unsigned long) acis_edge;
//					}
//					mesh_vh1 = next_vh;
//				}
//				sorted_vhs.push_back (mesh_vh2);
//
//				std::vector<SPAposition> pts;
//				fDivideOneEdge (acis_edge, sorted_vhs.size () - 1, pts);
//
//				//如果和边的离散顺序相反，则翻转一下
//				if (!same_point (pts.front (), POS2SPA(mesh->vertex (sorted_vhs.front ())), SPAresabs * 100))
//					std::reverse (sorted_vhs.begin (), sorted_vhs.end ());
//				for (int i = 0; i != sorted_vhs.size (); ++i)
//					mesh->set_vertex (sorted_vhs[i], SPA2POS(pts[i]));
//			}
//		};//end auto fSmoothAllEdges = [&](){...
//
//		auto fSmoothAllFaces = [&] (int times){
//
//			for (int i = 0; i != faces_list.count (); ++i){
//				FACE *f = (FACE*)(faces_list[i]);
//				//first we collect all the vertices handles on this face
//				auto vhs_on_this_face = faces_vertices_mapping[f];
//				
//				//then we try to smooth them using simplest laplacian smoothing
//				int round = 0;
//				while (round++ < times){
//					foreach (auto &cur_vh, vhs_on_this_face){
//						//get all the adjacent vertices of this vertex on the face
//						//std::unordered_set<OvmVeH> adj_vhs;
//						//get_adj_boundary_vertices_around_vertex (mesh, cur_vh, adj_vhs);
//						std::unordered_set<OvmFaH> adj_fhs;
//						get_adj_boundary_faces_around_vertex (mesh, cur_vh, adj_fhs);
//
//						OvmVec3d new_pos = OvmVec3d (0, 0, 0);
//						foreach (auto &adj_fh, adj_fhs)
//							new_pos += mesh->barycenter (adj_fh);
//						new_pos /= adj_fhs.size ();
//						if (!is_PLANE (f)){
//							SPAposition closest_pt;
//							double dist = 0.0f;
//							api_entity_point_distance (f, POS2SPA(new_pos), closest_pt, dist);
//							new_pos = SPA2POS(closest_pt);
//						}
//						mesh->set_vertex (cur_vh, new_pos);
//					}
//				}
//			}
//		};//end auto fSmoothAllFaces = [&] (int times){..
//
//		auto fSmoothVolumes = [&](int times){
//			int round = 0;
//			while (round++ < times){
//				foreach (auto &cur_vh, volume_vhs){
//					//std::unordered_set<OvmVeH> adj_vhs;
//					//get_adj_vertices_around_vertex (mesh, cur_vh, adj_vhs);
//					std::unordered_set<OvmCeH> adj_chs;
//					get_adj_hexas_around_vertex (mesh, cur_vh, adj_chs);
//
//					OvmVec3d new_pos = OvmVec3d (0, 0, 0);
//					foreach (auto &adj_ch, adj_chs)
//						new_pos += mesh->barycenter (adj_ch);
//					new_pos /= adj_chs.size ();
//					mesh->set_vertex (cur_vh, new_pos);
//				}
//				//for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it){
//				//	if (mesh->is_boundary (*v_it)) continue;
//				//	int idx = (*v_it).idx ();
//				//	std::unordered_set<OvmVeH> adj_vhs;
//				//	get_adj_vertices_around_vertex (mesh, *v_it, adj_vhs);
//
//				//	OvmVec3d new_pos = OvmVec3d (0, 0, 0);
//				//	foreach (auto &adj_vh, adj_vhs)
//				//		new_pos += mesh->vertex (adj_vh);
//				//	new_pos /= adj_vhs.size ();
//				//	mesh->set_vertex (*v_it, new_pos);
//				//}
//			}
//		};//end auto fSmoothVolumes = [&](int times){...
//
//		//先光顺边
//		fSmoothAllEdges ();
//		//在光顺边的时候，会将一些新的未确定位置的点的位置确定，所以先要将剩下的点的位置确定，即确定它们具体在那个面上
//		foreach (auto &vh, new_boundary_vhs){
//			int idx = vh.idx ();
//			if (V_ENTITY_PTR[vh] != -1) continue;
//
//			std::unordered_set<VERTEX*> adj_vertices_ptr;
//			std::unordered_set<EDGE*> adj_edges_ptr;
//			FACE* adj_face_ptr = NULL;
//			std::queue<OvmVeH> spread_set;
//			std::unordered_set<OvmVeH> visited_vhs;//遍历过的非点、边上的点，也是一同找到的未确定具体位置的新点
//			visited_vhs.insert (vh);
//			spread_set.push (vh);
//
//			while (!spread_set.empty ()){
//				OvmVeH curr_vh = spread_set.front ();
//				spread_set.pop ();
//				std::unordered_set<OvmVeH> adj_vhs;
//				get_adj_boundary_vertices_around_vertex (mesh, curr_vh, adj_vhs);
//				foreach (auto &adj_vh, adj_vhs){
//					if (contains (visited_vhs, adj_vh)) continue;
//
//					if (V_ENTITY_PTR[adj_vh] == -1){
//						spread_set.push (adj_vh);
//						visited_vhs.insert (adj_vh);
//						continue;
//					}
//					ENTITY *entity = (ENTITY*)(V_ENTITY_PTR[adj_vh]);
//					if (is_FACE (entity)){
//						adj_face_ptr = (FACE*)entity;
//						break;
//					}else if (is_EDGE (entity)){
//						adj_edges_ptr.insert ((EDGE*)entity);
//					}else if (is_VERTEX (entity)){
//						adj_vertices_ptr.insert ((VERTEX*)entity);
//					}
//				}
//				if (adj_face_ptr)
//					break;
//			}
//
//			if (adj_face_ptr){
//				foreach (auto &vh, visited_vhs){
//					V_ENTITY_PTR[vh] = (unsigned long)adj_face_ptr;
//					faces_vertices_mapping[adj_face_ptr].insert (vh);
//				}
//			}
//			else if (!adj_edges_ptr.empty ()){
//				assert (false);
//			}else if (!adj_vertices_ptr.empty ()){
//				assert (false);
//			}
//		}
//		fSmoothAllFaces (smooth_rounds);
//		fSmoothVolumes (smooth_rounds);*/
//
//        //更新后的smooth程序
//        //#define MYDEBUG
//		#ifdef MYDEBUG
//			QMessageBox::information(NULL, "pre", "enter");
//		#endif
//			assert(mesh->vertex_property_exists<unsigned long>("entityptr"));
//			auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
//
//			std::unordered_set<OvmVeH> fixed_vhs;
//			if (mesh->vertex_property_exists<bool>("vertexfixedforsmooth")){
//				auto V_FIXED = mesh->request_vertex_property<bool>("vertexfixedforsmooth");
//				auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
//				for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it){
//					if (V_FIXED[*v_it] == true) fixed_vhs.insert(*v_it);
//				}
//			}
//
//			//对边的类型统计一下
//			std::hash_map<VERTEX*, OvmVeH> vertices_mapping;
//			std::hash_map<EDGE*, std::unordered_set<OvmVeH> > edges_vertices_mapping;
//			std::hash_map<FACE*, std::unordered_set<OvmVeH> > faces_vertices_mapping;
//			std::unordered_set<OvmVeH> volume_vhs, new_boundary_vhs;
//
//			ENTITY_LIST vertices_list, edges_list, faces_list;
//			api_get_vertices(body, vertices_list);
//			api_get_edges(body, edges_list);
//			api_get_faces(body, faces_list);
//
//			auto visitedEdge = mesh->request_edge_property<bool>("edgevisited", false);
//
//			std::set<FACE*> all_faces;
//			entity_list_to_set(faces_list, all_faces);
//			std::vector<std::set<FACE*> > cylindrical_face_groups;
//			std::hash_map<ENTITY*, int> edge_face_group_mapping;
//			std::vector<std::unordered_set<OvmVeH> > determined_vhs_on_face_groups, real_vhs_on_face_groups;
//			std::unordered_set<OvmEgH> all_ehs_on_geom_edges;
//			std::unordered_set<OvmVeH> vhs_on_geom_edges;
//
//
//			std::set<ENTITY*> cylinder_edges;
//			for (int i = 0; i != edges_list.count(); ++i){
//				ENTITY_LIST adj_faces;
//				api_get_faces(edges_list[i], adj_faces);
//				bool is_cyl_f = true;
//				for (int j = 0; j != adj_faces.count(); ++j){
//					if (!is_cylindrical_face(adj_faces[j])){
//						is_cyl_f = false; break;
//					}
//				}
//				if (is_cyl_f && is_linear_edge(edges_list[i])){
//					cylinder_edges.insert(edges_list[i]);
//					std::set<FACE*> adj_faces_set;
//					entity_list_to_set(adj_faces, adj_faces_set);
//					if (contains(all_faces, adj_faces_set)){
//						edge_face_group_mapping[edges_list[i]] = cylindrical_face_groups.size();
//						cylindrical_face_groups.push_back(adj_faces_set);
//					}
//					else{
//						auto locate = std::find_if(cylindrical_face_groups.begin(), cylindrical_face_groups.end(),
//							[&](const std::set<FACE*> &one_group)->bool{
//							if (intersects(one_group, adj_faces_set)) return true;
//							else return false;
//						});
//						assert(locate != cylindrical_face_groups.end());
//						foreach(auto &f, adj_faces_set)
//							locate->insert(f);
//						edge_face_group_mapping[edges_list[i]] = locate - cylindrical_face_groups.begin();
//					}
//					foreach(auto &f, adj_faces_set)
//						all_faces.erase(f);
//				}
//			}
//		#ifdef MYDEBUG
//			QMessageBox::information(NULL, "pre", "step1");
//		#endif
//			foreach(auto &f, all_faces){
//				std::set<FACE*> one_group;
//				one_group.insert(f);
//				cylindrical_face_groups.push_back(one_group);
//			}
//
//			determined_vhs_on_face_groups.resize(cylindrical_face_groups.size());
//
//			//特注：需要做优化的点
//			std::unordered_set<OvmVeH> needed_smooth_vhs, vhs_temp;
//
//			for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
//			{
//				auto entity_ptr_uint = V_ENTITY_PTR[*v_it];
//				if (entity_ptr_uint == 0) volume_vhs.insert(*v_it);
//				else if (entity_ptr_uint == -1){
//					vhs_temp.insert(*v_it);
//					//判断一下这个新点是否在体内，如果是的话，则将他的entity_ptr设置为0
//					if (!mesh->is_boundary(*v_it)){
//						V_ENTITY_PTR[*v_it] = 0;
//						volume_vhs.insert(*v_it);
//					}
//					else{
//						new_boundary_vhs.insert(*v_it);
//					}
//				}
//				else{
//					ENTITY *entity = (ENTITY*)(entity_ptr_uint);
//					if (is_VERTEX(entity)) vertices_mapping[(VERTEX*)entity] = *v_it;
//					//else if (is_EDGE (entity)) edges_vertices_mapping[(EDGE*)entity].insert (*v_it);
//					//else faces_vertices_mapping[(FACE*)entity].insert (*v_it);
//					else if (is_EDGE(entity)){
//						if (contains(cylinder_edges, entity)){
//							int idx = edge_face_group_mapping[entity];
//							determined_vhs_on_face_groups[idx].insert(*v_it);
//						}
//						else{
//							edges_vertices_mapping[(EDGE*)entity].insert(*v_it);
//						}
//					}
//					else{
//						auto f = (FACE*)entity;
//						auto locate = std::find_if(cylindrical_face_groups.begin(), cylindrical_face_groups.end(),
//							[&](const std::set<FACE*> &one_group)->bool{
//							if (contains(one_group, f)) return true;
//							else return false;
//						});
//						determined_vhs_on_face_groups[locate - cylindrical_face_groups.begin()].insert(*v_it);
//					}
//				}
//			}
//			//if (!new_boundary_vhs.empty ())
//			//	QMessageBox::information (NULL, "info", "new_boundary_vhs不为空");
//
//			//特注：
//			/*foreach(auto vh, vhs_temp){
//				std::unordered_set<OvmVeH> adj_vhs;
//				needed_smooth_vhs.insert(vh);
//				get_adj_vertices_around_vertex(mesh, vh, adj_vhs);
//				foreach(auto vh_temp, adj_vhs)
//					needed_smooth_vhs.insert(vh_temp);
//			}
//			for(auto it = mesh->vertices_begin(); it != mesh->vertices_end(); it++){
//				if(!contains(needed_smooth_vhs, *it))
//					fixed_vhs.insert(*it);
//			}*/
//
//		#ifdef MYDEBUG
//			QMessageBox::information(NULL, "pre", "step2");
//		#endif
//			if (vertices_list.count() != vertices_mapping.size()){
//				//if (hoopsview){
//				//	//auto group = new VolumeMeshElementGroup (mesh, "si", "quad set for inflation");
//				//	//foreach (auto &p, vertices_mapping){
//				//	//	group->vhs.insert (p.second);
//				//	//}
//				//	//MeshRenderOptions render_options;
//				//	//render_options.vertex_color = "red";
//				//	//render_options.vertex_size = 2;
//				//	//hoopsview->render_mesh_group (group, render_options);
//				//	HC_Open_Segment_By_Key(hoopsview->GetHoopsView()->GetModelKey()); {
//				//		HC_Open_Segment("geom vertices"); {
//				//			HC_Flush_Geometry("...");
//				//			HC_Set_Marker_Size(2);
//				//			HC_Set_Color("markers=blue");
//				//			HC_Set_Visibility("markers=on");
//				//			for (int i = 0; i != vertices_list.count(); ++i){
//				//				auto V = (VERTEX*)(vertices_list[i]);
//				//				if (vertices_mapping.find(V) == vertices_mapping.end()){
//				//					SPAposition pos = V->geometry()->coords();
//				//					HC_Insert_Marker(pos.x(), pos.y(), pos.z());
//				//				}
//				//			}
//				//		}HC_Close_Segment();
//				//	}HC_Close_Segment();
//				//	hoopsview->GetHoopsView()->SetGeometryChanged();
//				//	hoopsview->GetHoopsView()->Update();
//				//	hoopsview->show_mesh_faces(false);
//				//	return;
//				//}
//			}
//
//			auto fDivideOneEdge = [&](EDGE *edge, int dis_num, std::vector<SPAposition> &pts){
//				SPAinterval inter = edge->param_range();
//				double step = inter.length() / dis_num,
//					start_param = inter.start_pt();
//				curve *crv = edge->geometry()->trans_curve();
//
//				for (int i = 0; i <= dis_num; ++i){
//					double param_val = start_param + step * i;
//					SPAposition pos;
//					crv->eval(param_val, pos);
//					pts.push_back(pos);
//				}
//			};
//
//			auto fSmoothAllEdges = [&](){
//				for (int i = 0; i != edges_list.count(); ++i){
//		#ifdef MYDEBUG
//					QMessageBox::information(NULL, "edge", QString("edge %1 begin").arg(i));
//		#endif
//					ENTITY *entity = edges_list[i];
//					if (contains(cylinder_edges, entity)) continue;
//
//					EDGE *acis_edge = (EDGE*)entity;
//					VERTEX *acis_v1 = acis_edge->start(),
//						*acis_v2 = acis_edge->end();
//					OvmVeH mesh_vh1 = vertices_mapping[acis_v1],
//						mesh_vh2 = vertices_mapping[acis_v2];
//					auto &mesh_vhs_set = edges_vertices_mapping[acis_edge];
//					mesh_vhs_set.insert(mesh_vh2);
//					std::vector<OvmVeH> sorted_vhs;
//					if(acis_v1 != acis_v2){
//						while (mesh_vh1 != mesh_vh2){
//							sorted_vhs.push_back(mesh_vh1);
//							OvmVeH next_vh = mesh->InvalidVertexHandle;
//							//首先在mesh_vhs_set中找下一个顶点
//							for (auto v_it = mesh_vhs_set.begin(); v_it != mesh_vhs_set.end(); ++v_it)
//							{
//								OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, *v_it);
//								if (test_heh != mesh->InvalidHalfEdgeHandle){
//									next_vh = *v_it;
//									mesh_vhs_set.erase(next_vh);
//									break;
//								}
//							}
//							//如果在mesh_vhs_set中找不到，那么在new_vhs中继续找
//							if (next_vh == mesh->InvalidVertexHandle){
//								double min_dist = std::numeric_limits<double>::max();
//								OvmVeH best_next_vh = OvmVeH(-1);
//								foreach(auto &vh, new_boundary_vhs){
//									OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, vh);
//									if (test_heh != mesh->InvalidHalfEdgeHandle){
//										SPAposition closest_pt;
//										double dist = 0.0f;
//										OvmPoint3d pos = mesh->vertex(vh);
//										api_entity_point_distance(acis_edge, POS2SPA(pos), closest_pt, dist);
//										if (dist < min_dist){
//											best_next_vh = vh;
//											min_dist = dist;
//										}
//									}
//								}
//								next_vh = best_next_vh;
//								assert(next_vh != mesh->InvalidVertexHandle);
//								new_boundary_vhs.erase(next_vh);
//								V_ENTITY_PTR[next_vh] = (unsigned long)acis_edge;
//							}
//							mesh_vh1 = next_vh;
//						}//end while (mesh_vh1 != mesh_vh2){...
//						sorted_vhs.push_back(mesh_vh2);
//
//						std::vector<SPAposition> pts;
//						fDivideOneEdge(acis_edge, sorted_vhs.size() - 1, pts);
//						if (!same_point(pts.front(), POS2SPA(mesh->vertex(sorted_vhs.front())), SPAresabs * 1000))
//							std::reverse(sorted_vhs.begin(), sorted_vhs.end());
//						for (int j = 0; j != sorted_vhs.size(); ++j)
//							mesh->set_vertex(sorted_vhs[j], SPA2POS(pts[j]));
//					}
//					else{
//						sorted_vhs.push_back(mesh_vh1);
//						OvmVeH next_vh = mesh->InvalidVertexHandle;
//						//首先在mesh_vhs_set中找下一个顶点
//						for (auto v_it = mesh_vhs_set.begin(); v_it != mesh_vhs_set.end(); ++v_it)
//						{
//							OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, *v_it);
//							if (test_heh != mesh->InvalidHalfEdgeHandle){
//								next_vh = *v_it;
//								mesh_vhs_set.erase(next_vh);
//								break;
//							}
//						}
//						//如果在mesh_vhs_set中找不到，那么在new_vhs中继续找
//						if (next_vh == mesh->InvalidVertexHandle){
//							double min_dist = std::numeric_limits<double>::max();
//							OvmVeH best_next_vh = OvmVeH(-1);
//							foreach(auto &vh, new_boundary_vhs){
//								OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, vh);
//								if (test_heh != mesh->InvalidHalfEdgeHandle){
//									SPAposition closest_pt;
//									double dist = 0.0f;
//									OvmPoint3d pos = mesh->vertex(vh);
//									api_entity_point_distance(acis_edge, POS2SPA(pos), closest_pt, dist);
//									if (dist < min_dist){
//										best_next_vh = vh;
//										min_dist = dist;
//									}
//								}
//							}
//							next_vh = best_next_vh;
//							assert(next_vh != mesh->InvalidVertexHandle);
//							new_boundary_vhs.erase(next_vh);
//							V_ENTITY_PTR[next_vh] = (unsigned long)acis_edge;
//						}
//						mesh_vh1 = next_vh;
//						sorted_vhs.push_back(mesh_vh1);
//						next_vh = mesh->InvalidVertexHandle;
//						//首先在mesh_vhs_set中找下一个顶点
//						for (auto v_it = mesh_vhs_set.begin(); v_it != mesh_vhs_set.end(); ++v_it)
//						{
//							if(*v_it == mesh_vh2)
//								continue;
//							OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, *v_it);
//							if (test_heh != mesh->InvalidHalfEdgeHandle){
//								next_vh = *v_it;
//								mesh_vhs_set.erase(next_vh);
//								break;
//							}
//						}
//						//如果在mesh_vhs_set中找不到，那么在new_vhs中继续找
//						if (next_vh == mesh->InvalidVertexHandle){
//							double min_dist = std::numeric_limits<double>::max();
//							OvmVeH best_next_vh = OvmVeH(-1);
//							foreach(auto &vh, new_boundary_vhs){
//								OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, vh);
//								if (test_heh != mesh->InvalidHalfEdgeHandle){
//									SPAposition closest_pt;
//									double dist = 0.0f;
//									OvmPoint3d pos = mesh->vertex(vh);
//									api_entity_point_distance(acis_edge, POS2SPA(pos), closest_pt, dist);
//									if (dist < min_dist){
//										best_next_vh = vh;
//										min_dist = dist;
//									}
//								}
//							}
//							next_vh = best_next_vh;
//							assert(next_vh != mesh->InvalidVertexHandle);
//							new_boundary_vhs.erase(next_vh);
//							V_ENTITY_PTR[next_vh] = (unsigned long)acis_edge;
//						}
//						mesh_vh1 = next_vh;
//						while (mesh_vh1 != mesh_vh2){
//							sorted_vhs.push_back(mesh_vh1);
//							OvmVeH next_vh = mesh->InvalidVertexHandle;
//							//首先在mesh_vhs_set中找下一个顶点
//							for (auto v_it = mesh_vhs_set.begin(); v_it != mesh_vhs_set.end(); ++v_it)
//							{
//								OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, *v_it);
//								if (test_heh != mesh->InvalidHalfEdgeHandle){
//									next_vh = *v_it;
//									mesh_vhs_set.erase(next_vh);
//									break;
//								}
//							}
//							//如果在mesh_vhs_set中找不到，那么在new_vhs中继续找
//							if (next_vh == mesh->InvalidVertexHandle){
//								double min_dist = std::numeric_limits<double>::max();
//								OvmVeH best_next_vh = OvmVeH(-1);
//								foreach(auto &vh, new_boundary_vhs){
//									OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, vh);
//									if (test_heh != mesh->InvalidHalfEdgeHandle){
//										SPAposition closest_pt;
//										double dist = 0.0f;
//										OvmPoint3d pos = mesh->vertex(vh);
//										api_entity_point_distance(acis_edge, POS2SPA(pos), closest_pt, dist);
//										if (dist < min_dist){
//											best_next_vh = vh;
//											min_dist = dist;
//										}
//									}
//								}
//								next_vh = best_next_vh;
//								assert(next_vh != mesh->InvalidVertexHandle);
//								new_boundary_vhs.erase(next_vh);
//								V_ENTITY_PTR[next_vh] = (unsigned long)acis_edge;
//							}
//							mesh_vh1 = next_vh;
//						}//end while (mesh_vh1 != mesh_vh2){...
//						sorted_vhs.push_back(mesh_vh2);
//
//						std::vector<SPAposition> pts;
//						fDivideOneEdge(acis_edge, sorted_vhs.size() - 1, pts);
//						OvmVec3d vec1(SPA2POS(pts[1]) - SPA2POS(pts[0])), vec2(OvmVec3d(mesh->vertex(sorted_vhs[1]).data()) - SPA2POS(pts[0]));
//						if(dot(vec1, vec2) < 0)
//							std::reverse(sorted_vhs.begin(), sorted_vhs.end());
//						for (int j = 0; j != sorted_vhs.size(); ++j)
//							mesh->set_vertex(sorted_vhs[j], SPA2POS(pts[j]));
//					}
//
//		#ifdef MYDEBUG
//					QMessageBox::information(NULL, "edge", QString("edge %1 out while").arg(i));
//		#endif
//					//std::vector<SPAposition> pts;
//					//fDivideOneEdge(acis_edge, sorted_vhs.size() - 1, pts);
//
//					////如果和边的离散顺序相反，则翻转一下
//					////OvmVec3d vec1(SPA2POS(pts[1]) - SPA2POS(pts[0])), vec2(sorted_vhs[1] - sorted_vhs[0]);
//					////if(dot(vec1, vec2) < 0)
//					//if (!same_point(pts.front(), POS2SPA(mesh->vertex(sorted_vhs.front())), SPAresabs * 1000))
//					//	std::reverse(sorted_vhs.begin(), sorted_vhs.end());
//					//for (int j = 0; j != sorted_vhs.size(); ++j)
//					//	mesh->set_vertex(sorted_vhs[j], SPA2POS(pts[j]));
//					if (sorted_vhs.size() > 2){
//						for (int j = 1; j != sorted_vhs.size() - 1; ++j){
//
//							V_ENTITY_PTR[sorted_vhs[j]] = (unsigned long)acis_edge;
//						}
//					}
//
//					for (int j = 0; j != sorted_vhs.size() - 1; ++j){
//						auto eh = mesh->edge_handle(mesh->halfedge(sorted_vhs[j], sorted_vhs[j + 1]));
//						if (eh == mesh->InvalidEdgeHandle)
//							QMessageBox::warning(NULL, "ERROR", "eh invalid!");
//						visitedEdge[eh] = true;
//						all_ehs_on_geom_edges.insert(eh);
//					}
//		#ifdef MYDEBUG
//					QMessageBox::information(NULL, "edge", QString("edge %1 finished").arg(i));
//		#endif
//				}
//			};//end auto fSmoothAllEdges = [&](){...
//
//			auto bisect_angle = [&](OvmPoint3d& v1, OvmPoint3d& v2)-> OvmPoint3d
//			{
//				OvmPoint3d uv1 = v1 / v1.norm(), uv2 = v2 / v2.norm();
//				return (uv1 + uv2).normalize();
//			};
//
//			auto intersection = [&](const OvmPoint3d& NA, const OvmPoint3d& NB,
//				const OvmPoint3d& tail, const OvmPoint3d& V,
//				OvmPoint3d& interPt /* = Point */)->bool
//			{
//				OvmPoint3d P0 = NA;
//				OvmPoint3d P1 = tail;
//				OvmPoint3d d0 = NB - NA, d1 = V;
//				OvmPoint3d u = P0 - P1;
//				double a = d0 | d0;
//				double b = d0 | d1;
//				double c = d1 | d1;
//				double d = d0 | u;
//				double e = d1 | u;
//				double f = u | u;
//				double det = a * c - b * b;
//
//				//check for parallelism
//				bool res = false;
//				if (fabs(det) < SPAresabs){
//					res = false;
//				}
//				else{
//					double invDet = 1 / det;
//					double s = (b * e - c * d) * invDet;
//					double t = (a * e - b * d) * invDet;
//					if ((s > -SPAresabs && s < 1 + SPAresabs) && (t > -SPAresabs))
//						res = true;
//					interPt = P0 + d0 * s;
//				}
//				return res;
//			};
//			auto calc_delta_C = [&](OvmVeH Ni, OvmVeH Nj, OvmVeH Ni_minus_1, OvmVeH Ni_plus_1, OvmPoint3d &delta_c)
//			{
//				OvmPoint3d Pi_minus_1 = mesh->vertex(Ni_minus_1) - mesh->vertex(Nj);
//				OvmPoint3d Pi = mesh->vertex(Ni) - mesh->vertex(Nj);
//				OvmPoint3d Pi_plus_1 = mesh->vertex(Ni_plus_1) - mesh->vertex(Nj);
//
//				OvmPoint3d PB1 = bisect_angle(Pi_minus_1, Pi_plus_1);
//				OvmPoint3d PB2 = bisect_angle(PB1, Pi);
//				OvmPoint3d Q(0, 0, 0);
//				if (!intersection(mesh->vertex(Ni_minus_1), mesh->vertex(Ni_plus_1), mesh->vertex(Nj), PB2, Q))
//					return;
//
//				double lD = (mesh->vertex(Ni) - mesh->vertex(Nj)).length();
//				double lQ = (Q - mesh->vertex(Nj)).length();
//				PB2 = PB2.normalize() * ((lD > lQ) ? (lD + lQ) / 2 : lD);
//
//				delta_c = PB2 - Pi;
//			};
//
//			auto fSmoothAllFaces = [&](int times){
//				for (int i = 0; i != cylindrical_face_groups.size(); ++i){
//					//first we collect all the vertices handles on this face
//					auto &vhs_test = determined_vhs_on_face_groups[i];
//					std::unordered_set<OvmVeH> vhs_on_this_group;
//					foreach(auto &vhs, real_vhs_on_face_groups){
//						if (intersects(vhs, vhs_test)){
//							vhs_on_this_group = vhs; break;
//						}
//					}
//					if (vhs_on_this_group.empty())
//					{
//						QMessageBox::information(NULL, "face", QString("face %1 empty!").arg(i));
//
//						//debug
//						/*ENTITY_LIST save_f_list;
//						set<FACE *> save_faces = cylindrical_face_groups[i];
//						set_to_entity_list(save_f_list, save_faces);
//						my_save_entity_list(save_f_list, "../empty_faces.sat");*/
//					}
//					//then we try to smooth them using simplest laplacian smoothing
//					int round = 0;
//					ENTITY_LIST face_group_list;
//					foreach(auto &f, cylindrical_face_groups[i])
//						face_group_list.add(f);
//
//		#ifdef MYDEBUG
//					QMessageBox::information(NULL, "face", QString("face %1 enter while").arg(i));
//		#endif
//
//					while (round++ < times){
//						std::vector<OvmVeH> vhs_vec;
//						unordered_set_to_vector(vhs_on_this_group, vhs_vec);
//						SPAposition closest_pos;
//						double dist;
//						for (int j = 0; j != vhs_vec.size(); ++j){
//							auto Ni = vhs_vec[j];
//							if (contains(fixed_vhs, Ni)) continue;
//
//							//get all the adjacent vertices of this vertex on the face
//							std::unordered_set<OvmVeH> adj_vhs;
//							get_adj_boundary_vertices_around_vertex(mesh, Ni, adj_vhs);
//
//							auto curr_pos = mesh->vertex(Ni);
//							std::vector<OvmPoint3d> Cj_vec;
//							foreach(auto &adj_vh, adj_vhs){
//								auto pos = mesh->vertex(adj_vh);
//								auto Cj = pos - curr_pos;
//								//if (contains (vhs_on_geom_edges, adj_vh)){
//								//	auto heh = mesh->halfedge (Ni, adj_vh);
//								//	auto eh = mesh->edge_handle (heh);
//								//	std::unordered_set<OvmFaH> bound_adj_fhs;
//								//	get_adj_boundary_faces_around_edge (mesh, eh, bound_adj_fhs);
//								//	if (bound_adj_fhs.size () != 2){
//								//		QMessageBox::critical (NULL, "错误", "bound_adj_fhs个数不为2！");
//								//		//return;
//								//	}
//
//								//	std::vector<OvmFaH> bound_adj_fhs_vec;
//								//	unordered_set_to_vector (bound_adj_fhs, bound_adj_fhs_vec);
//								//	std::vector<OvmVeH> Ni_adj; Ni_adj.resize (2);
//								//	for (int m = 0; m != 2; ++m){
//								//		auto tmp_fh = bound_adj_fhs_vec[m];
//								//		auto heh_vec = mesh->face (tmp_fh).halfedges ();
//								//		foreach (auto &tmp_heh, heh_vec){
//								//			auto vh1 = mesh->halfedge (tmp_heh).from_vertex (),
//								//				vh2 = mesh->halfedge (tmp_heh).to_vertex ();
//								//			if (vh1 == Ni && vh2 != adj_vh)
//								//				Ni_adj[m] = vh2;
//								//			else if (vh2 == Ni && vh1 != adj_vh)
//								//				Ni_adj[m] = vh1;
//								//		}
//								//	}
//								//	OvmPoint3d delta_c(0,0,0);
//								//	calc_delta_C (Ni, adj_vh, Ni_adj.front (), Ni_adj.back (), delta_c);
//								//	if (delta_c[0] == 0 && delta_c[1] == 0 && delta_c[2] == 0){
//								//		QMessageBox::information (NULL, "为空", QString ("为空 %1").arg (Ni.idx ()));
//								//	}
//								//	Cj_vec.push_back ((Cj + delta_c));
//								//}else{
//								Cj_vec.push_back(Cj);
//								//}
//							}
//
//							OvmPoint3d upp(0, 0, 0);
//							double den = 0;
//							//cumulate the Vjs according to the formula
//							foreach(auto &Cj, Cj_vec){
//								//upp += (Cj.length () * Cj);
//								//den += Cj.length ();
//								upp += (1.0 * Cj);
//								den += 1.0;
//							}
//							auto delta_i = upp / den;
//							auto new_pos = curr_pos + delta_i;
//
//							auto spa_pos = POS2SPA(new_pos);
//							double curr_closest_dist = std::numeric_limits<double>::max();
//							ENTITY *curr_closest_f = NULL;
//							SPAposition curr_closest_pos;
//							for (int m = 0; m != face_group_list.count(); ++m){
//								auto tmp_f = face_group_list[m];
//								api_entity_point_distance(tmp_f, spa_pos, closest_pos, dist);
//								if (dist < curr_closest_dist){
//									curr_closest_dist = dist;
//									curr_closest_pos = closest_pos;
//									curr_closest_f = tmp_f;
//								}
//							}
//
//							mesh->set_vertex(Ni, SPA2POS(curr_closest_pos));
//							V_ENTITY_PTR[Ni] = (unsigned long)(curr_closest_f);
//						}
//						//QMessageBox::information (NULL, "face", QString ("round %1 finished").arg (round));
//					}
//		#ifdef MYDEBUG
//					QMessageBox::information(NULL, "face", QString("face %1 end").arg(i));
//		#endif
//				}
//			};//end auto fSmoothAllFaces = [&] (int times){..
//
//			auto fSmoothVolumes = [&](int times){
//				for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it){
//					auto entity_int = V_ENTITY_PTR[*v_it];
//					if (entity_int == -1){
//						volume_vhs.insert(*v_it);
//						V_ENTITY_PTR[*v_it] = 0;
//					}
//				}
//				int round = 0;
//				while (round++ < times){
//					foreach(auto &cur_vh, volume_vhs){
//						//std::unordered_set<OvmVeH> adj_vhs;
//						//get_adj_vertices_around_vertex (mesh, cur_vh, adj_vhs);
//						std::unordered_set<OvmCeH> adj_chs;
//						get_adj_hexas_around_vertex(mesh, cur_vh, adj_chs);
//
//						OvmPoint3d new_pos = OvmPoint3d(0, 0, 0);
//						foreach(auto &adj_ch, adj_chs)
//							new_pos += mesh->barycenter(adj_ch);
//						new_pos /= adj_chs.size();
//						mesh->set_vertex(cur_vh, new_pos);
//					}
//				}
//			};//end auto fSmoothVolumes = [&](int times){...
//
//			//先光顺边
//			fSmoothAllEdges();
//			//auto group = new VolumeMeshElementGroup (mesh, "si", "quad set for inflation");
//			//group->ehs = all_ehs_on_geom_edges;
//			//MeshRenderOptions render_options;
//			//render_options.edge_color = "red";
//			//render_options.edge_width = 6;
//			//hoopsview->render_mesh_group (group, render_options);
//			//return;
//
//		#ifdef MYDEBUG
//			QMessageBox::information(NULL, "info", "edge finished");
//		#endif
//			//在光顺边的时候，会将一些新的未确定位置的点的位置确定，所以先要将剩下的点的位置确定，即确定它们具体在那个面上
//			foreach (auto &vh, new_boundary_vhs){
//				int idx = vh.idx ();
//				if (V_ENTITY_PTR[vh] != -1) continue;
//
//				std::unordered_set<VERTEX*> adj_vertices_ptr;
//				std::unordered_set<EDGE*> adj_edges_ptr;
//				FACE* adj_face_ptr = NULL;
//				std::queue<OvmVeH> spread_set;
//				std::unordered_set<OvmVeH> visited_vhs;//遍历过的非点、边上的点，也是一同找到的未确定具体位置的新点
//				visited_vhs.insert (vh);
//				spread_set.push (vh);
//
//				while (!spread_set.empty ()){
//					OvmVeH curr_vh = spread_set.front ();
//					spread_set.pop ();
//					std::unordered_set<OvmVeH> adj_vhs;
//					get_adj_boundary_vertices_around_vertex (mesh, curr_vh, adj_vhs);
//					foreach (auto &adj_vh, adj_vhs){
//						if (contains (visited_vhs, adj_vh)) continue;
//
//						if (V_ENTITY_PTR[adj_vh] == -1){
//							spread_set.push (adj_vh);
//							visited_vhs.insert (adj_vh);
//							continue;
//						}
//						ENTITY *entity = (ENTITY*)(V_ENTITY_PTR[adj_vh]);
//						if (is_FACE (entity)){
//							adj_face_ptr = (FACE*)entity;
//							break;
//						}else if (is_EDGE (entity)){
//							adj_edges_ptr.insert ((EDGE*)entity);
//						}else if (is_VERTEX (entity)){
//							adj_vertices_ptr.insert ((VERTEX*)entity);
//						}
//					}
//					if (adj_face_ptr)
//						break;
//				}
//
//				if (adj_face_ptr){
//					foreach (auto &vh, visited_vhs){
//						V_ENTITY_PTR[vh] = (unsigned long)adj_face_ptr;
//						faces_vertices_mapping[adj_face_ptr].insert (vh);
//					}
//				}
//				else if (!adj_edges_ptr.empty ()){
//					assert (false);
//				}else if (!adj_vertices_ptr.empty ()){
//					assert (false);
//				}
//			}
//			//首先根据boundEdge上的信息把体网格表面网格进行分块儿
//
//			auto all_boundary_fhs = new std::unordered_set<OvmFaH>();
//			for (auto bf_it = mesh->bf_iter(); bf_it; ++bf_it){
//				all_boundary_fhs->insert(*bf_it);
//			}
//			std::vector<std::vector<OvmFaH> > quad_patches;
//			while (!all_boundary_fhs->empty()){
//				OvmFaH first_fh = pop_begin_element(*all_boundary_fhs);
//				std::queue<OvmFaH> spread_set;
//				spread_set.push(first_fh);
//				std::vector<OvmFaH> one_patch;
//				one_patch.push_back(first_fh);
//				while (!spread_set.empty()){
//					auto cur_fh = spread_set.front();
//					spread_set.pop();
//					auto heh_vec = mesh->face(cur_fh).halfedges();
//					foreach(auto heh, heh_vec){
//						auto eh = mesh->edge_handle(heh);
//						if (visitedEdge[eh]) continue;
//
//						OvmFaH adj_fh = mesh->InvalidFaceHandle;
//						for (auto hehf_it = mesh->hehf_iter(heh); hehf_it; ++hehf_it)
//						{
//							auto test_fh = mesh->face_handle(*hehf_it);
//							if (test_fh == mesh->InvalidFaceHandle || test_fh == cur_fh)
//								continue;
//							if (all_boundary_fhs->find(test_fh) == all_boundary_fhs->end())
//								continue;
//							adj_fh = test_fh;
//							break;
//						}
//						one_patch.push_back(adj_fh);
//						visitedEdge[eh] = true;
//						spread_set.push(adj_fh);
//					}
//				}//end while (!spread_set.empty ())...
//				quad_patches.push_back(one_patch);
//				foreach(auto tmp_fh, one_patch)
//					all_boundary_fhs->erase(tmp_fh);
//		#ifdef MYDEBUG
//				QMessageBox::information(NULL, "faces", QString("one patch %1 finished").arg(one_patch.size()));
//		#endif
//			}
//			delete all_boundary_fhs;
//		#ifdef MYDEBUG
//			QMessageBox::information(NULL, "faces", QString("sep patches count %1").arg(quad_patches.size()));
//		#endif
//
//			foreach(auto &eh, all_ehs_on_geom_edges){
//				auto eg = mesh->edge(eh);
//				vhs_on_geom_edges.insert(eg.from_vertex());
//				vhs_on_geom_edges.insert(eg.to_vertex());
//			}
//
//			foreach(auto &one_patch, quad_patches){
//				std::unordered_set<OvmVeH> vhs_on_this_patch;
//				foreach(auto &fh, one_patch){
//					auto adj_vhs = get_adj_vertices_around_face(mesh, fh);
//					foreach(auto &vh, adj_vhs){
//						if (!contains(vhs_on_geom_edges, vh))
//							vhs_on_this_patch.insert(vh);
//					}
//				}
//
//				real_vhs_on_face_groups.push_back(vhs_on_this_patch);
//			}
//		#ifdef MYDEBUG
//			QMessageBox::information(NULL, "info", "sep patches finished");
//		#endif
//			//char *colors[] = {"yellow", "pink", "blue", "green", "red", "purple"};
//			//for (int i = 0; i != real_vhs_on_face_groups.size (); ++i){
//			//	auto &vhs_patch = real_vhs_on_face_groups[i];
//			//	auto group = new VolumeMeshElementGroup (mesh, "si", "ehs on geometric edges");
//			//	group->vhs = vhs_patch;
//			//	MeshRenderOptions render_options;
//			//	render_options.vertex_color = colors[i%6];
//			//	render_options.vertex_size = 1;
//			//	hoopsview->render_mesh_group (group, render_options);
//			//}
//			//return;
//			fSmoothAllFaces(smooth_rounds);
//		#ifdef MYDEBUG
//			QMessageBox::information(NULL, "info", "smooth faces finished");
//		#endif
//			fSmoothVolumes(smooth_rounds);
//		#ifdef MYDEBUG
//			QMessageBox::information(NULL, "info", "smooth volume finished");
//		#endif
//	}

void smooth_volume_mesh (VolumeMesh *mesh, BODY *body, int smooth_rounds)
{
	#ifdef MYDEBUG
		QMessageBox::information(NULL, "pre", "enter");
	#endif
		assert(mesh->vertex_property_exists<unsigned long>("entityptr"));
		auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

		std::unordered_set<OvmVeH> fixed_vhs;
		if (mesh->vertex_property_exists<bool>("vertexfixedforsmooth")){
			auto V_FIXED = mesh->request_vertex_property<bool>("vertexfixedforsmooth");
			auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
			for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it){
				if (V_FIXED[*v_it] == true) fixed_vhs.insert(*v_it);
			}
		}
		//对边的类型统计一下
		std::hash_map<VERTEX*, OvmVeH> vertices_mapping;
		std::hash_map<EDGE*, std::unordered_set<OvmVeH> > edges_vertices_mapping;
		std::hash_map<FACE*, std::unordered_set<OvmVeH> > faces_vertices_mapping;
		std::unordered_set<OvmVeH> volume_vhs, new_boundary_vhs;

		ENTITY_LIST vertices_list, edges_list, faces_list;
		api_get_vertices(body, vertices_list);
		api_get_edges(body, edges_list);
		api_get_faces(body, faces_list);

		auto visitedEdge = mesh->request_edge_property<bool>("edgevisited", false);

		std::hash_map<ENTITY*, int> edge_face_group_mapping;
		std::unordered_set<OvmEgH> all_ehs_on_geom_edges;
		std::unordered_set<OvmVeH> vhs_on_geom_edges;

	#ifdef MYDEBUG
		QMessageBox::information(NULL, "pre", "step1");
	#endif
		std::cout<<"After initialization "<<mesh->n_vertices()<<std::endl;
		//特注：需要做优化的点
		std::unordered_set<OvmVeH> needed_smooth_vhs, vhs_temp;
		int v_counter = 0;
		for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
		{
			auto entity_ptr_uint = V_ENTITY_PTR[*v_it];
			if (entity_ptr_uint == 0) volume_vhs.insert(*v_it);
			else if (entity_ptr_uint == -1){
				vhs_temp.insert(*v_it);
				//判断一下这个新点是否在体内，如果是的话，则将他的entity_ptr设置为0
				if (!mesh->is_boundary(*v_it)){
					V_ENTITY_PTR[*v_it] = 0;
					volume_vhs.insert(*v_it);
				}
				else{
					new_boundary_vhs.insert(*v_it);
				}
			}
			else{
				ENTITY *entity = (ENTITY*)(entity_ptr_uint);
				if (is_VERTEX(entity)) vertices_mapping[(VERTEX*)entity] = *v_it;
				else if (is_EDGE(entity))
					edges_vertices_mapping[(EDGE*)entity].insert(*v_it);
				else
					faces_vertices_mapping[(FACE*)entity].insert(*v_it);
			}
		}
		for (auto v_it = vertices_mapping.begin();v_it != vertices_mapping.end();v_it++)
		{
			std::cout<<v_it->second<<" ";
		}
		//特注：
		/*foreach(auto vh, vhs_temp){
			std::unordered_set<OvmVeH> adj_vhs;
			needed_smooth_vhs.insert(vh);
			get_adj_vertices_around_vertex(mesh, vh, adj_vhs);
			foreach(auto vh_temp, adj_vhs)
				needed_smooth_vhs.insert(vh_temp);
		}
		for(auto it = mesh->vertices_begin(); it != mesh->vertices_end(); it++){
			if(!contains(needed_smooth_vhs, *it))
				fixed_vhs.insert(*it);
		}*/


	#ifdef MYDEBUG
		QMessageBox::information(NULL, "pre", "step2");
	#endif
		if (vertices_list.count() != vertices_mapping.size()){
			;
		}

		auto fDivideOneEdge = [&](EDGE *edge, int dis_num, std::vector<SPAposition> &pts){
			SPAinterval inter = edge->param_range();
			double step = inter.length() / dis_num,
				start_param = inter.start_pt();
			curve *crv = edge->geometry()->trans_curve();

			for (int i = 0; i <= dis_num; ++i){
				double param_val = start_param + step * i;
				SPAposition pos;
				crv->eval(param_val, pos);
				pts.push_back(pos);
			}
		};

		auto fSmoothAllEdges = [&](){
			for (int i = 0; i != edges_list.count(); ++i){
	#ifdef MYDEBUG
				QMessageBox::information(NULL, "edge", QString("edge %1 begin").arg(i));
	#endif
				ENTITY *entity = edges_list[i];

				EDGE *acis_edge = (EDGE*)entity;
				VERTEX *acis_v1 = acis_edge->start(),
					*acis_v2 = acis_edge->end();
				//if (acis_v1 == acis_v2) continue;
				OvmVeH mesh_vh1 = vertices_mapping[acis_v1],
					mesh_vh2 = vertices_mapping[acis_v2];
				auto &mesh_vhs_set = edges_vertices_mapping[acis_edge];
				mesh_vhs_set.insert(mesh_vh2);
				std::vector<OvmVeH> sorted_vhs;
				if(acis_v1 != acis_v2){
					while (mesh_vh1 != mesh_vh2){
						sorted_vhs.push_back(mesh_vh1);
						OvmVeH next_vh = mesh->InvalidVertexHandle;
						//首先在mesh_vhs_set中找下一个顶点
						for (auto v_it = mesh_vhs_set.begin(); v_it != mesh_vhs_set.end(); ++v_it)
						{
							OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, *v_it);
							if (test_heh != mesh->InvalidHalfEdgeHandle){
								next_vh = *v_it;
								mesh_vhs_set.erase(next_vh);
								break;
							}
						}
						//如果在mesh_vhs_set中找不到，那么在new_vhs中继续找
						if (next_vh == mesh->InvalidVertexHandle){
							double min_dist = std::numeric_limits<double>::max();
							OvmVeH best_next_vh = OvmVeH(-1);
							foreach(auto &vh, new_boundary_vhs){
								OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, vh);
								if (test_heh != mesh->InvalidHalfEdgeHandle){
									SPAposition closest_pt;
									double dist = 0.0f;
									OvmPoint3d pos = mesh->vertex(vh);
									api_entity_point_distance(acis_edge, POS2SPA(pos), closest_pt, dist);
									if (dist < min_dist){
										best_next_vh = vh;
										min_dist = dist;
									}
								}
							}
							next_vh = best_next_vh;
							assert(next_vh != mesh->InvalidVertexHandle);
							new_boundary_vhs.erase(next_vh);
							V_ENTITY_PTR[next_vh] = (unsigned long)acis_edge;
						}
						mesh_vh1 = next_vh;
					}//end while (mesh_vh1 != mesh_vh2){...
					sorted_vhs.push_back(mesh_vh2);

					std::vector<SPAposition> pts;
					fDivideOneEdge(acis_edge, sorted_vhs.size() - 1, pts);
					if (!same_point(pts.front(), POS2SPA(mesh->vertex(sorted_vhs.front())), SPAresabs * 1000))
						std::reverse(sorted_vhs.begin(), sorted_vhs.end());
					for (int j = 0; j != sorted_vhs.size(); ++j)
						mesh->set_vertex(sorted_vhs[j], SPA2POS(pts[j]));
				}
				else{
					sorted_vhs.push_back(mesh_vh1);
					OvmVeH next_vh = mesh->InvalidVertexHandle;
					//首先在mesh_vhs_set中找下一个顶点
					for (auto v_it = mesh_vhs_set.begin(); v_it != mesh_vhs_set.end(); ++v_it)
					{
						OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, *v_it);
						if (test_heh != mesh->InvalidHalfEdgeHandle){
							next_vh = *v_it;
							mesh_vhs_set.erase(next_vh);
							break;
						}
					}
					//如果在mesh_vhs_set中找不到，那么在new_vhs中继续找
					if (next_vh == mesh->InvalidVertexHandle){
						double min_dist = std::numeric_limits<double>::max();
						OvmVeH best_next_vh = OvmVeH(-1);
						foreach(auto &vh, new_boundary_vhs){
							OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, vh);
							if (test_heh != mesh->InvalidHalfEdgeHandle){
								SPAposition closest_pt;
								double dist = 0.0f;
								OvmPoint3d pos = mesh->vertex(vh);
								api_entity_point_distance(acis_edge, POS2SPA(pos), closest_pt, dist);
								if (dist < min_dist){
									best_next_vh = vh;
									min_dist = dist;
								}
							}
						}
						next_vh = best_next_vh;
						assert(next_vh != mesh->InvalidVertexHandle);
						new_boundary_vhs.erase(next_vh);
						V_ENTITY_PTR[next_vh] = (unsigned long)acis_edge;
					}
					mesh_vh1 = next_vh;
					sorted_vhs.push_back(mesh_vh1);
					next_vh = mesh->InvalidVertexHandle;
					//首先在mesh_vhs_set中找下一个顶点
					for (auto v_it = mesh_vhs_set.begin(); v_it != mesh_vhs_set.end(); ++v_it)
					{
						if(*v_it == mesh_vh2)
							continue;
						OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, *v_it);
						if (test_heh != mesh->InvalidHalfEdgeHandle){
							next_vh = *v_it;
							mesh_vhs_set.erase(next_vh);
							break;
						}
					}
					//如果在mesh_vhs_set中找不到，那么在new_vhs中继续找
					if (next_vh == mesh->InvalidVertexHandle){
						double min_dist = std::numeric_limits<double>::max();
						OvmVeH best_next_vh = OvmVeH(-1);
						foreach(auto &vh, new_boundary_vhs){
							OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, vh);
							if (test_heh != mesh->InvalidHalfEdgeHandle){
								SPAposition closest_pt;
								double dist = 0.0f;
								OvmPoint3d pos = mesh->vertex(vh);
								api_entity_point_distance(acis_edge, POS2SPA(pos), closest_pt, dist);
								if (dist < min_dist){
									best_next_vh = vh;
									min_dist = dist;
								}
							}
						}
						next_vh = best_next_vh;
						assert(next_vh != mesh->InvalidVertexHandle);
						new_boundary_vhs.erase(next_vh);
						V_ENTITY_PTR[next_vh] = (unsigned long)acis_edge;
					}
					mesh_vh1 = next_vh;
					while (mesh_vh1 != mesh_vh2){
						sorted_vhs.push_back(mesh_vh1);
						OvmVeH next_vh = mesh->InvalidVertexHandle;
						//首先在mesh_vhs_set中找下一个顶点
						for (auto v_it = mesh_vhs_set.begin(); v_it != mesh_vhs_set.end(); ++v_it)
						{
							OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, *v_it);
							if (test_heh != mesh->InvalidHalfEdgeHandle){
								next_vh = *v_it;
								mesh_vhs_set.erase(next_vh);
								break;
							}
						}
						//如果在mesh_vhs_set中找不到，那么在new_vhs中继续找
						if (next_vh == mesh->InvalidVertexHandle){
							double min_dist = std::numeric_limits<double>::max();
							OvmVeH best_next_vh = OvmVeH(-1);
							foreach(auto &vh, new_boundary_vhs){
								OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, vh);
								if (test_heh != mesh->InvalidHalfEdgeHandle){
									SPAposition closest_pt;
									double dist = 0.0f;
									OvmPoint3d pos = mesh->vertex(vh);
									api_entity_point_distance(acis_edge, POS2SPA(pos), closest_pt, dist);
									if (dist < min_dist){
										best_next_vh = vh;
										min_dist = dist;
									}
								}
							}
							next_vh = best_next_vh;
							assert(next_vh != mesh->InvalidVertexHandle);
							new_boundary_vhs.erase(next_vh);
							V_ENTITY_PTR[next_vh] = (unsigned long)acis_edge;
						}
						mesh_vh1 = next_vh;
					}//end while (mesh_vh1 != mesh_vh2){...
					sorted_vhs.push_back(mesh_vh2);

					std::vector<SPAposition> pts;
					fDivideOneEdge(acis_edge, sorted_vhs.size() - 1, pts);
					OvmVec3d vec1(SPA2POS(pts[1]) - SPA2POS(pts[0])), vec2(OvmVec3d(mesh->vertex(sorted_vhs[1]).data()) - SPA2POS(pts[0]));
					if(dot(vec1, vec2) < 0)
						std::reverse(sorted_vhs.begin(), sorted_vhs.end());
					for (int j = 0; j != sorted_vhs.size(); ++j)
						mesh->set_vertex(sorted_vhs[j], SPA2POS(pts[j]));
				}

	#ifdef MYDEBUG
				QMessageBox::information(NULL, "edge", QString("edge %1 out while").arg(i));
	#endif
				if (sorted_vhs.size() > 2){
					for (int j = 1; j != sorted_vhs.size() - 1; ++j){

						V_ENTITY_PTR[sorted_vhs[j]] = (unsigned long)acis_edge;
					}
				}


				for (int j = 0; j != sorted_vhs.size() - 1; ++j){
					auto eh = mesh->edge_handle(mesh->halfedge(sorted_vhs[j], sorted_vhs[j + 1]));
					if (eh == mesh->InvalidEdgeHandle)
					{
						QMessageBox::warning(NULL, "ERROR", "eh invalid!");
						system("pause");
					}
					visitedEdge[eh] = true;
					all_ehs_on_geom_edges.insert(eh);
				}
	#ifdef MYDEBUG
				QMessageBox::information(NULL, "edge", QString("edge %1 finished").arg(i));
	#endif
			}
		};//end auto fSmoothAllEdges = [&](){...

		auto bisect_angle = [&](OvmPoint3d& v1, OvmPoint3d& v2)-> OvmPoint3d
		{
			OvmPoint3d uv1 = v1 / v1.norm(), uv2 = v2 / v2.norm();
			return (uv1 + uv2).normalize();
		};

		auto intersection = [&](const OvmPoint3d& NA, const OvmPoint3d& NB,
			const OvmPoint3d& tail, const OvmPoint3d& V,
			OvmPoint3d& interPt /* = Point */)->bool
		{
			OvmPoint3d P0 = NA;
			OvmPoint3d P1 = tail;
			OvmPoint3d d0 = NB - NA, d1 = V;
			OvmPoint3d u = P0 - P1;
			double a = d0 | d0;
			double b = d0 | d1;
			double c = d1 | d1;
			double d = d0 | u;
			double e = d1 | u;
			double f = u | u;
			double det = a * c - b * b;

			//check for parallelism
			bool res = false;
			if (fabs(det) < SPAresabs){
				res = false;
			}
			else{
				double invDet = 1 / det;
				double s = (b * e - c * d) * invDet;
				double t = (a * e - b * d) * invDet;
				if ((s > -SPAresabs && s < 1 + SPAresabs) && (t > -SPAresabs))
					res = true;
				interPt = P0 + d0 * s;
			}
			return res;
		};
		auto calc_delta_C = [&](OvmVeH Ni, OvmVeH Nj, OvmVeH Ni_minus_1, OvmVeH Ni_plus_1, OvmPoint3d &delta_c)
		{
			OvmPoint3d Pi_minus_1 = mesh->vertex(Ni_minus_1) - mesh->vertex(Nj);
			OvmPoint3d Pi = mesh->vertex(Ni) - mesh->vertex(Nj);
			OvmPoint3d Pi_plus_1 = mesh->vertex(Ni_plus_1) - mesh->vertex(Nj);

			OvmPoint3d PB1 = bisect_angle(Pi_minus_1, Pi_plus_1);
			OvmPoint3d PB2 = bisect_angle(PB1, Pi);
			OvmPoint3d Q(0, 0, 0);
			if (!intersection(mesh->vertex(Ni_minus_1), mesh->vertex(Ni_plus_1), mesh->vertex(Nj), PB2, Q))
				return;

			double lD = (mesh->vertex(Ni) - mesh->vertex(Nj)).length();
			double lQ = (Q - mesh->vertex(Nj)).length();
			PB2 = PB2.normalize() * ((lD > lQ) ? (lD + lQ) / 2 : lD);

			delta_c = PB2 - Pi;
		};

		auto fSmoothAllFaces = [&](int times){
			for (int i = 0; i != faces_list.count(); ++i){
				//first we collect all the vertices handles on this face
				ENTITY * entity = faces_list[i];
				FACE* face = (FACE*) entity;
				auto &vhs_on_this_face = faces_vertices_mapping[face];
				if (vhs_on_this_face.empty())
				{
					QMessageBox::information(NULL, "face", QString("face %1 empty!").arg(i));

				}
				//then we try to smooth them using simplest laplacian smoothing
				int round = 0;

	#ifdef MYDEBUG
				QMessageBox::information(NULL, "face", QString("face %1 enter while").arg(i));
	#endif

				while (round++ < times){
					std::vector<OvmVeH> vhs_vec;
					unordered_set_to_vector(vhs_on_this_face, vhs_vec);
					SPAposition closest_pos;
					double dist;
					for (int j = 0; j != vhs_vec.size(); ++j){
						auto Ni = vhs_vec[j];
						if (contains(fixed_vhs, Ni)) continue;

						//get all the adjacent vertices of this vertex on the face
						std::unordered_set<OvmVeH> adj_vhs;
						get_adj_boundary_vertices_around_vertex(mesh, Ni, adj_vhs);

						auto curr_pos = mesh->vertex(Ni);
						std::vector<OvmPoint3d> Cj_vec;
						foreach(auto &adj_vh, adj_vhs){
							auto pos = mesh->vertex(adj_vh);
							auto Cj = pos - curr_pos;
							Cj_vec.push_back(Cj);
						}

						OvmPoint3d upp(0, 0, 0);
						double den = 0;
						//cumulate the Vjs according to the formula
						foreach(auto &Cj, Cj_vec){
							//upp += (Cj.length () * Cj);
							//den += Cj.length ();
							upp += (1.0 * Cj);
							den += 1.0;
						}
						auto delta_i = upp / den;
						auto new_pos = curr_pos + delta_i;

						auto spa_pos = POS2SPA(new_pos);
						api_entity_point_distance(faces_list[i], spa_pos, closest_pos, dist);

						mesh->set_vertex(Ni, SPA2POS(closest_pos));
						V_ENTITY_PTR[Ni] = (unsigned long)(entity);
					}
					//QMessageBox::information (NULL, "face", QString ("round %1 finished").arg (round));
				}
	#ifdef MYDEBUG
				QMessageBox::information(NULL, "face", QString("face %1 end").arg(i));
	#endif
			}
		};//end auto fSmoothAllFaces = [&] (int times){..

		auto fSmoothVolumes = [&](int times){
			for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it){
				auto entity_int = V_ENTITY_PTR[*v_it];
				if (entity_int == -1){
					volume_vhs.insert(*v_it);
					V_ENTITY_PTR[*v_it] = 0;
				}
			}
			int round = 0;
			while (round++ < times){
				foreach(auto &cur_vh, volume_vhs){
					//std::unordered_set<OvmVeH> adj_vhs;
					//get_adj_vertices_around_vertex (mesh, cur_vh, adj_vhs);
					std::unordered_set<OvmCeH> adj_chs;
					get_adj_hexas_around_vertex(mesh, cur_vh, adj_chs);

					OvmPoint3d new_pos = OvmPoint3d(0, 0, 0);
					foreach(auto &adj_ch, adj_chs)
						new_pos += mesh->barycenter(adj_ch);
					new_pos /= adj_chs.size();
					mesh->set_vertex(cur_vh, new_pos);
				}
			}
		};//end auto fSmoothVolumes = [&](int times){...

		//先光顺边
		fSmoothAllEdges();

	#ifdef MYDEBUG
		QMessageBox::information(NULL, "info", "edge finished");
	#endif

		//在光顺边的时候，会将一些新的未确定位置的点的位置确定，所以先要将剩下的点的位置确定，即确定它们具体在那个面上
		foreach (auto &vh, new_boundary_vhs){
			int idx = vh.idx ();
			if (V_ENTITY_PTR[vh] != -1) continue;
		
			std::unordered_set<VERTEX*> adj_vertices_ptr;
			std::unordered_set<EDGE*> adj_edges_ptr;
			FACE* adj_face_ptr = NULL;
			std::queue<OvmVeH> spread_set;
			std::unordered_set<OvmVeH> visited_vhs;//遍历过的非点、边上的点，也是一同找到的未确定具体位置的新点
			visited_vhs.insert (vh);
			spread_set.push (vh);
		
			while (!spread_set.empty ()){
				OvmVeH curr_vh = spread_set.front ();
				spread_set.pop ();
				std::unordered_set<OvmVeH> adj_vhs;
				get_adj_boundary_vertices_around_vertex (mesh, curr_vh, adj_vhs);
				foreach (auto &adj_vh, adj_vhs){
					if (contains (visited_vhs, adj_vh)) continue;
		
					if (V_ENTITY_PTR[adj_vh] == -1){
						spread_set.push (adj_vh);
						visited_vhs.insert (adj_vh);
						continue;
					}
					ENTITY *entity = (ENTITY*)(V_ENTITY_PTR[adj_vh]);
					if (is_FACE (entity)){
						adj_face_ptr = (FACE*)entity;
						break;
					}else if (is_EDGE (entity)){
						adj_edges_ptr.insert ((EDGE*)entity);
					}else if (is_VERTEX (entity)){
						adj_vertices_ptr.insert ((VERTEX*)entity);
					}
				}
				if (adj_face_ptr)
					break;
			}
		
			if (adj_face_ptr){
				foreach (auto &vh, visited_vhs){
					V_ENTITY_PTR[vh] = (unsigned long)adj_face_ptr;
					faces_vertices_mapping[adj_face_ptr].insert (vh);
				}
			}
			else if (!adj_edges_ptr.empty ()){
				assert (false);
			}else if (!adj_vertices_ptr.empty ()){
				assert (false);
			}
		}

		//首先根据boundEdge上的信息把体网格表面网格进行分块儿

		auto all_boundary_fhs = new std::unordered_set<OvmFaH>();
		for (auto bf_it = mesh->bf_iter(); bf_it; ++bf_it){
			all_boundary_fhs->insert(*bf_it);
		}
		std::vector<std::vector<OvmFaH> > quad_patches;
		while (!all_boundary_fhs->empty()){
			OvmFaH first_fh = pop_begin_element(*all_boundary_fhs);
			std::queue<OvmFaH> spread_set;
			spread_set.push(first_fh);
			std::vector<OvmFaH> one_patch;
			one_patch.push_back(first_fh);
			while (!spread_set.empty()){
				auto cur_fh = spread_set.front();
				spread_set.pop();
				auto heh_vec = mesh->face(cur_fh).halfedges();
				foreach(auto heh, heh_vec){
					auto eh = mesh->edge_handle(heh);
					if (visitedEdge[eh]) continue;

					OvmFaH adj_fh = mesh->InvalidFaceHandle;
					for (auto hehf_it = mesh->hehf_iter(heh); hehf_it; ++hehf_it)
					{
						auto test_fh = mesh->face_handle(*hehf_it);
						if (test_fh == mesh->InvalidFaceHandle || test_fh == cur_fh)
							continue;
						if (all_boundary_fhs->find(test_fh) == all_boundary_fhs->end())
							continue;
						adj_fh = test_fh;
						break;
					}
					one_patch.push_back(adj_fh);
					visitedEdge[eh] = true;
					spread_set.push(adj_fh);
				}
			}//end while (!spread_set.empty ())...
			quad_patches.push_back(one_patch);
			foreach(auto tmp_fh, one_patch)
				all_boundary_fhs->erase(tmp_fh);
	#ifdef MYDEBUG
			QMessageBox::information(NULL, "faces", QString("one patch %1 finished").arg(one_patch.size()));
	#endif
		}
		delete all_boundary_fhs;
	#ifdef MYDEBUG
		QMessageBox::information(NULL, "faces", QString("sep patches count %1").arg(quad_patches.size()));
	#endif

		foreach(auto &eh, all_ehs_on_geom_edges){
			auto eg = mesh->edge(eh);
			vhs_on_geom_edges.insert(eg.from_vertex());
			vhs_on_geom_edges.insert(eg.to_vertex());
		}

		foreach(auto &one_patch, quad_patches){
			std::unordered_set<OvmVeH> vhs_on_this_patch;
			foreach(auto &fh, one_patch){
				auto adj_vhs = get_adj_vertices_around_face(mesh, fh);
				foreach(auto &vh, adj_vhs){
					if (!contains(vhs_on_geom_edges, vh))
						vhs_on_this_patch.insert(vh);
				}
			}

			//real_vhs_on_face_groups.push_back(vhs_on_this_patch);
		}
	#ifdef MYDEBUG
		QMessageBox::information(NULL, "info", "sep patches finished");
	#endif
		fSmoothAllFaces(smooth_rounds);
	#ifdef MYDEBUG
		QMessageBox::information(NULL, "info", "smooth faces finished");
	#endif
		fSmoothVolumes(smooth_rounds);
	#ifdef MYDEBUG
		QMessageBox::information(NULL, "info", "smooth volume finished");
	#endif
}

void smooth_volume_mesh_local (VolumeMesh *mesh, BODY *body, int smooth_rounds)
{
	#ifdef MYDEBUG
		QMessageBox::information(NULL, "pre", "enter");
	#endif
		assert(mesh->vertex_property_exists<unsigned long>("entityptr"));
		auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

		std::unordered_set<OvmVeH> fixed_vhs;
		if (mesh->vertex_property_exists<bool>("vertexfixedforsmooth")){
			auto V_FIXED = mesh->request_vertex_property<bool>("vertexfixedforsmooth");
			auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
			for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it){
				if (V_FIXED[*v_it] == true) fixed_vhs.insert(*v_it);
			}
		}
		//对边的类型统计一下
		std::hash_map<VERTEX*, OvmVeH> vertices_mapping;
		std::hash_map<EDGE*, std::unordered_set<OvmVeH> > edges_vertices_mapping;
		std::hash_map<FACE*, std::unordered_set<OvmVeH> > faces_vertices_mapping;
		std::unordered_set<OvmVeH> volume_vhs, new_boundary_vhs;

		ENTITY_LIST vertices_list, edges_list, faces_list;
		api_get_vertices(body, vertices_list);
		api_get_edges(body, edges_list);
		api_get_faces(body, faces_list);

		auto visitedEdge = mesh->request_edge_property<bool>("edgevisited", false);

		std::hash_map<ENTITY*, int> edge_face_group_mapping;
		std::unordered_set<OvmEgH> all_ehs_on_geom_edges;
		std::unordered_set<OvmVeH> vhs_on_geom_edges;

	#ifdef MYDEBUG
		QMessageBox::information(NULL, "pre", "step1");
	#endif

		//特注：需要做优化的点
		std::unordered_set<OvmVeH> needed_smooth_vhs, vhs_temp;

		for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
		{
			auto entity_ptr_uint = V_ENTITY_PTR[*v_it];
			if (entity_ptr_uint == 0) volume_vhs.insert(*v_it);
			else if (entity_ptr_uint == -1){
				vhs_temp.insert(*v_it);
				//判断一下这个新点是否在体内，如果是的话，则将他的entity_ptr设置为0
				if (!mesh->is_boundary(*v_it)){
					V_ENTITY_PTR[*v_it] = 0;
					volume_vhs.insert(*v_it);
				}
				else{
					new_boundary_vhs.insert(*v_it);
				}
			}
			else{
				ENTITY *entity = (ENTITY*)(entity_ptr_uint);
				if (is_VERTEX(entity)) vertices_mapping[(VERTEX*)entity] = *v_it;
				else if (is_EDGE(entity))
					edges_vertices_mapping[(EDGE*)entity].insert(*v_it);
				else
					faces_vertices_mapping[(FACE*)entity].insert(*v_it);
			}
		}

		//特注：
		foreach(auto vh, vhs_temp){
			std::unordered_set<OvmVeH> adj_vhs;
			needed_smooth_vhs.insert(vh);
			get_adj_vertices_around_vertex(mesh, vh, adj_vhs);
			foreach(auto vh_temp, adj_vhs)
				needed_smooth_vhs.insert(vh_temp);
		}
		for(auto it = mesh->vertices_begin(); it != mesh->vertices_end(); it++){
			if(!contains(needed_smooth_vhs, *it))
				fixed_vhs.insert(*it);
		}

	#ifdef MYDEBUG
		QMessageBox::information(NULL, "pre", "step2");
	#endif
		if (vertices_list.count() != vertices_mapping.size()){
			;
		}

		auto fDivideOneEdge = [&](EDGE *edge, int dis_num, std::vector<SPAposition> &pts){
			SPAinterval inter = edge->param_range();
			double step = inter.length() / dis_num,
				start_param = inter.start_pt();
			curve *crv = edge->geometry()->trans_curve();

			for (int i = 0; i <= dis_num; ++i){
				double param_val = start_param + step * i;
				SPAposition pos;
				crv->eval(param_val, pos);
				pts.push_back(pos);
			}
		};

		auto fSmoothAllEdges = [&](){
			for (int i = 0; i != edges_list.count(); ++i){
	#ifdef MYDEBUG
				QMessageBox::information(NULL, "edge", QString("edge %1 begin").arg(i));
	#endif
				ENTITY *entity = edges_list[i];

				EDGE *acis_edge = (EDGE*)entity;
				VERTEX *acis_v1 = acis_edge->start(),
					*acis_v2 = acis_edge->end();
				OvmVeH mesh_vh1 = vertices_mapping[acis_v1],
					mesh_vh2 = vertices_mapping[acis_v2];
				auto &mesh_vhs_set = edges_vertices_mapping[acis_edge];
				mesh_vhs_set.insert(mesh_vh2);
				std::vector<OvmVeH> sorted_vhs;
				if(acis_v1 != acis_v2){
					while (mesh_vh1 != mesh_vh2){
						sorted_vhs.push_back(mesh_vh1);
						OvmVeH next_vh = mesh->InvalidVertexHandle;
						//首先在mesh_vhs_set中找下一个顶点
						for (auto v_it = mesh_vhs_set.begin(); v_it != mesh_vhs_set.end(); ++v_it)
						{
							OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, *v_it);
							if (test_heh != mesh->InvalidHalfEdgeHandle){
								next_vh = *v_it;
								mesh_vhs_set.erase(next_vh);
								break;
							}
						}
						//如果在mesh_vhs_set中找不到，那么在new_vhs中继续找
						if (next_vh == mesh->InvalidVertexHandle){
							double min_dist = std::numeric_limits<double>::max();
							OvmVeH best_next_vh = OvmVeH(-1);
							foreach(auto &vh, new_boundary_vhs){
								OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, vh);
								if (test_heh != mesh->InvalidHalfEdgeHandle){
									SPAposition closest_pt;
									double dist = 0.0f;
									OvmPoint3d pos = mesh->vertex(vh);
									api_entity_point_distance(acis_edge, POS2SPA(pos), closest_pt, dist);
									if (dist < min_dist){
										best_next_vh = vh;
										min_dist = dist;
									}
								}
							}
							next_vh = best_next_vh;
							assert(next_vh != mesh->InvalidVertexHandle);
							new_boundary_vhs.erase(next_vh);
							V_ENTITY_PTR[next_vh] = (unsigned long)acis_edge;
						}
						mesh_vh1 = next_vh;
					}//end while (mesh_vh1 != mesh_vh2){...
					sorted_vhs.push_back(mesh_vh2);

					std::vector<SPAposition> pts;
					fDivideOneEdge(acis_edge, sorted_vhs.size() - 1, pts);
					if (!same_point(pts.front(), POS2SPA(mesh->vertex(sorted_vhs.front())), SPAresabs * 1000))
						std::reverse(sorted_vhs.begin(), sorted_vhs.end());
					for (int j = 0; j != sorted_vhs.size(); ++j)
						mesh->set_vertex(sorted_vhs[j], SPA2POS(pts[j]));
				}
				else{
					sorted_vhs.push_back(mesh_vh1);
					OvmVeH next_vh = mesh->InvalidVertexHandle;
					//首先在mesh_vhs_set中找下一个顶点
					for (auto v_it = mesh_vhs_set.begin(); v_it != mesh_vhs_set.end(); ++v_it)
					{
						OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, *v_it);
						if (test_heh != mesh->InvalidHalfEdgeHandle){
							next_vh = *v_it;
							mesh_vhs_set.erase(next_vh);
							break;
						}
					}
					//如果在mesh_vhs_set中找不到，那么在new_vhs中继续找
					if (next_vh == mesh->InvalidVertexHandle){
						double min_dist = std::numeric_limits<double>::max();
						OvmVeH best_next_vh = OvmVeH(-1);
						foreach(auto &vh, new_boundary_vhs){
							OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, vh);
							if (test_heh != mesh->InvalidHalfEdgeHandle){
								SPAposition closest_pt;
								double dist = 0.0f;
								OvmPoint3d pos = mesh->vertex(vh);
								api_entity_point_distance(acis_edge, POS2SPA(pos), closest_pt, dist);
								if (dist < min_dist){
									best_next_vh = vh;
									min_dist = dist;
								}
							}
						}
						next_vh = best_next_vh;
						assert(next_vh != mesh->InvalidVertexHandle);
						new_boundary_vhs.erase(next_vh);
						V_ENTITY_PTR[next_vh] = (unsigned long)acis_edge;
					}
					mesh_vh1 = next_vh;
					sorted_vhs.push_back(mesh_vh1);
					next_vh = mesh->InvalidVertexHandle;
					//首先在mesh_vhs_set中找下一个顶点
					for (auto v_it = mesh_vhs_set.begin(); v_it != mesh_vhs_set.end(); ++v_it)
					{
						if(*v_it == mesh_vh2)
							continue;
						OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, *v_it);
						if (test_heh != mesh->InvalidHalfEdgeHandle){
							next_vh = *v_it;
							mesh_vhs_set.erase(next_vh);
							break;
						}
					}
					//如果在mesh_vhs_set中找不到，那么在new_vhs中继续找
					if (next_vh == mesh->InvalidVertexHandle){
						double min_dist = std::numeric_limits<double>::max();
						OvmVeH best_next_vh = OvmVeH(-1);
						foreach(auto &vh, new_boundary_vhs){
							OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, vh);
							if (test_heh != mesh->InvalidHalfEdgeHandle){
								SPAposition closest_pt;
								double dist = 0.0f;
								OvmPoint3d pos = mesh->vertex(vh);
								api_entity_point_distance(acis_edge, POS2SPA(pos), closest_pt, dist);
								if (dist < min_dist){
									best_next_vh = vh;
									min_dist = dist;
								}
							}
						}
						next_vh = best_next_vh;
						assert(next_vh != mesh->InvalidVertexHandle);
						new_boundary_vhs.erase(next_vh);
						V_ENTITY_PTR[next_vh] = (unsigned long)acis_edge;
					}
					mesh_vh1 = next_vh;
					while (mesh_vh1 != mesh_vh2){
						sorted_vhs.push_back(mesh_vh1);
						OvmVeH next_vh = mesh->InvalidVertexHandle;
						//首先在mesh_vhs_set中找下一个顶点
						for (auto v_it = mesh_vhs_set.begin(); v_it != mesh_vhs_set.end(); ++v_it)
						{
							OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, *v_it);
							if (test_heh != mesh->InvalidHalfEdgeHandle){
								next_vh = *v_it;
								mesh_vhs_set.erase(next_vh);
								break;
							}
						}
						//如果在mesh_vhs_set中找不到，那么在new_vhs中继续找
						if (next_vh == mesh->InvalidVertexHandle){
							double min_dist = std::numeric_limits<double>::max();
							OvmVeH best_next_vh = OvmVeH(-1);
							foreach(auto &vh, new_boundary_vhs){
								OvmHaEgH test_heh = mesh->halfedge(mesh_vh1, vh);
								if (test_heh != mesh->InvalidHalfEdgeHandle){
									SPAposition closest_pt;
									double dist = 0.0f;
									OvmPoint3d pos = mesh->vertex(vh);
									api_entity_point_distance(acis_edge, POS2SPA(pos), closest_pt, dist);
									if (dist < min_dist){
										best_next_vh = vh;
										min_dist = dist;
									}
								}
							}
							next_vh = best_next_vh;
							assert(next_vh != mesh->InvalidVertexHandle);
							new_boundary_vhs.erase(next_vh);
							V_ENTITY_PTR[next_vh] = (unsigned long)acis_edge;
						}
						mesh_vh1 = next_vh;
					}//end while (mesh_vh1 != mesh_vh2){...
					sorted_vhs.push_back(mesh_vh2);

					std::vector<SPAposition> pts;
					fDivideOneEdge(acis_edge, sorted_vhs.size() - 1, pts);
					OvmVec3d vec1(SPA2POS(pts[1]) - SPA2POS(pts[0])), vec2(OvmVec3d(mesh->vertex(sorted_vhs[1]).data()) - SPA2POS(pts[0]));
					if(dot(vec1, vec2) < 0)
						std::reverse(sorted_vhs.begin(), sorted_vhs.end());
					for (int j = 0; j != sorted_vhs.size(); ++j)
						mesh->set_vertex(sorted_vhs[j], SPA2POS(pts[j]));
				}

	#ifdef MYDEBUG
				QMessageBox::information(NULL, "edge", QString("edge %1 out while").arg(i));
	#endif
				if (sorted_vhs.size() > 2){
					for (int j = 1; j != sorted_vhs.size() - 1; ++j){

						V_ENTITY_PTR[sorted_vhs[j]] = (unsigned long)acis_edge;
					}
				}

				for (int j = 0; j != sorted_vhs.size() - 1; ++j){
					auto eh = mesh->edge_handle(mesh->halfedge(sorted_vhs[j], sorted_vhs[j + 1]));
					//std::ostringstream os; os<<eh.idx();std::string outpu = os.str()+" invalid";
					if (eh == mesh->InvalidEdgeHandle)
						QMessageBox::warning(NULL, "ERROR", "eh invalid!");
					visitedEdge[eh] = true;
					all_ehs_on_geom_edges.insert(eh);
				}
	#ifdef MYDEBUG
				QMessageBox::information(NULL, "edge", QString("edge %1 finished").arg(i));
	#endif
			}
		};//end auto fSmoothAllEdges = [&](){...

		auto bisect_angle = [&](OvmPoint3d& v1, OvmPoint3d& v2)-> OvmPoint3d
		{
			OvmPoint3d uv1 = v1 / v1.norm(), uv2 = v2 / v2.norm();
			return (uv1 + uv2).normalize();
		};

		auto intersection = [&](const OvmPoint3d& NA, const OvmPoint3d& NB,
			const OvmPoint3d& tail, const OvmPoint3d& V,
			OvmPoint3d& interPt /* = Point */)->bool
		{
			OvmPoint3d P0 = NA;
			OvmPoint3d P1 = tail;
			OvmPoint3d d0 = NB - NA, d1 = V;
			OvmPoint3d u = P0 - P1;
			double a = d0 | d0;
			double b = d0 | d1;
			double c = d1 | d1;
			double d = d0 | u;
			double e = d1 | u;
			double f = u | u;
			double det = a * c - b * b;

			//check for parallelism
			bool res = false;
			if (fabs(det) < SPAresabs){
				res = false;
			}
			else{
				double invDet = 1 / det;
				double s = (b * e - c * d) * invDet;
				double t = (a * e - b * d) * invDet;
				if ((s > -SPAresabs && s < 1 + SPAresabs) && (t > -SPAresabs))
					res = true;
				interPt = P0 + d0 * s;
			}
			return res;
		};
		auto calc_delta_C = [&](OvmVeH Ni, OvmVeH Nj, OvmVeH Ni_minus_1, OvmVeH Ni_plus_1, OvmPoint3d &delta_c)
		{
			OvmPoint3d Pi_minus_1 = mesh->vertex(Ni_minus_1) - mesh->vertex(Nj);
			OvmPoint3d Pi = mesh->vertex(Ni) - mesh->vertex(Nj);
			OvmPoint3d Pi_plus_1 = mesh->vertex(Ni_plus_1) - mesh->vertex(Nj);

			OvmPoint3d PB1 = bisect_angle(Pi_minus_1, Pi_plus_1);
			OvmPoint3d PB2 = bisect_angle(PB1, Pi);
			OvmPoint3d Q(0, 0, 0);
			if (!intersection(mesh->vertex(Ni_minus_1), mesh->vertex(Ni_plus_1), mesh->vertex(Nj), PB2, Q))
				return;

			double lD = (mesh->vertex(Ni) - mesh->vertex(Nj)).length();
			double lQ = (Q - mesh->vertex(Nj)).length();
			PB2 = PB2.normalize() * ((lD > lQ) ? (lD + lQ) / 2 : lD);

			delta_c = PB2 - Pi;
		};

		auto fSmoothAllFaces = [&](int times){
			for (int i = 0; i != faces_list.count(); ++i){
				//first we collect all the vertices handles on this face
				ENTITY * entity = faces_list[i];
				FACE* face = (FACE*) entity;
				auto &vhs_on_this_face = faces_vertices_mapping[face];
				if (vhs_on_this_face.empty())
				{
					QMessageBox::information(NULL, "face", QString("face %1 empty!").arg(i));

				}
				//then we try to smooth them using simplest laplacian smoothing
				int round = 0;

	#ifdef MYDEBUG
				QMessageBox::information(NULL, "face", QString("face %1 enter while").arg(i));
	#endif

				while (round++ < times){
					std::vector<OvmVeH> vhs_vec;
					unordered_set_to_vector(vhs_on_this_face, vhs_vec);
					SPAposition closest_pos;
					double dist;
					for (int j = 0; j != vhs_vec.size(); ++j){
						auto Ni = vhs_vec[j];
						if (contains(fixed_vhs, Ni)) continue;

						//get all the adjacent vertices of this vertex on the face
						std::unordered_set<OvmVeH> adj_vhs;
						get_adj_boundary_vertices_around_vertex(mesh, Ni, adj_vhs);

						auto curr_pos = mesh->vertex(Ni);
						std::vector<OvmPoint3d> Cj_vec;
						foreach(auto &adj_vh, adj_vhs){
							auto pos = mesh->vertex(adj_vh);
							auto Cj = pos - curr_pos;
							Cj_vec.push_back(Cj);
						}

						OvmPoint3d upp(0, 0, 0);
						double den = 0;
						//cumulate the Vjs according to the formula
						foreach(auto &Cj, Cj_vec){
							//upp += (Cj.length () * Cj);
							//den += Cj.length ();
							upp += (1.0 * Cj);
							den += 1.0;
						}
						auto delta_i = upp / den;
						auto new_pos = curr_pos + delta_i;

						auto spa_pos = POS2SPA(new_pos);
						api_entity_point_distance(faces_list[i], spa_pos, closest_pos, dist);

						mesh->set_vertex(Ni, SPA2POS(closest_pos));
						V_ENTITY_PTR[Ni] = (unsigned long)(entity);
					}
					//QMessageBox::information (NULL, "face", QString ("round %1 finished").arg (round));
				}
	#ifdef MYDEBUG
				QMessageBox::information(NULL, "face", QString("face %1 end").arg(i));
	#endif
			}
		};//end auto fSmoothAllFaces = [&] (int times){..

		auto fSmoothVolumes = [&](int times){
			for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it){
				auto entity_int = V_ENTITY_PTR[*v_it];
				if (entity_int == -1){
					volume_vhs.insert(*v_it);
					V_ENTITY_PTR[*v_it] = 0;
				}
			}
			int round = 0;
			while (round++ < times){
				foreach(auto &cur_vh, volume_vhs){
					//std::unordered_set<OvmVeH> adj_vhs;
					//get_adj_vertices_around_vertex (mesh, cur_vh, adj_vhs);
					std::unordered_set<OvmCeH> adj_chs;
					get_adj_hexas_around_vertex(mesh, cur_vh, adj_chs);

					OvmPoint3d new_pos = OvmPoint3d(0, 0, 0);
					foreach(auto &adj_ch, adj_chs)
						new_pos += mesh->barycenter(adj_ch);
					new_pos /= adj_chs.size();
					mesh->set_vertex(cur_vh, new_pos);
				}
			}
		};//end auto fSmoothVolumes = [&](int times){...

		//先光顺边
		fSmoothAllEdges();

	#ifdef MYDEBUG
		QMessageBox::information(NULL, "info", "edge finished");
	#endif

		//在光顺边的时候，会将一些新的未确定位置的点的位置确定，所以先要将剩下的点的位置确定，即确定它们具体在那个面上
		foreach (auto &vh, new_boundary_vhs){
			int idx = vh.idx ();
			if (V_ENTITY_PTR[vh] != -1) continue;
		
			std::unordered_set<VERTEX*> adj_vertices_ptr;
			std::unordered_set<EDGE*> adj_edges_ptr;
			FACE* adj_face_ptr = NULL;
			std::queue<OvmVeH> spread_set;
			std::unordered_set<OvmVeH> visited_vhs;//遍历过的非点、边上的点，也是一同找到的未确定具体位置的新点
			visited_vhs.insert (vh);
			spread_set.push (vh);
		
			while (!spread_set.empty ()){
				OvmVeH curr_vh = spread_set.front ();
				spread_set.pop ();
				std::unordered_set<OvmVeH> adj_vhs;
				get_adj_boundary_vertices_around_vertex (mesh, curr_vh, adj_vhs);
				foreach (auto &adj_vh, adj_vhs){
					if (contains (visited_vhs, adj_vh)) continue;
		
					if (V_ENTITY_PTR[adj_vh] == -1){
						spread_set.push (adj_vh);
						visited_vhs.insert (adj_vh);
						continue;
					}
					ENTITY *entity = (ENTITY*)(V_ENTITY_PTR[adj_vh]);
					if (is_FACE (entity)){
						adj_face_ptr = (FACE*)entity;
						break;
					}else if (is_EDGE (entity)){
						adj_edges_ptr.insert ((EDGE*)entity);
					}else if (is_VERTEX (entity)){
						adj_vertices_ptr.insert ((VERTEX*)entity);
					}
				}
				if (adj_face_ptr)
					break;
			}
		
			if (adj_face_ptr){
				foreach (auto &vh, visited_vhs){
					V_ENTITY_PTR[vh] = (unsigned long)adj_face_ptr;
					faces_vertices_mapping[adj_face_ptr].insert (vh);
				}
			}
			else if (!adj_edges_ptr.empty ()){
				assert (false);
			}else if (!adj_vertices_ptr.empty ()){
				assert (false);
			}
		}

		//首先根据boundEdge上的信息把体网格表面网格进行分块儿

		auto all_boundary_fhs = new std::unordered_set<OvmFaH>();
		for (auto bf_it = mesh->bf_iter(); bf_it; ++bf_it){
			all_boundary_fhs->insert(*bf_it);
		}
		std::vector<std::vector<OvmFaH> > quad_patches;
		while (!all_boundary_fhs->empty()){
			OvmFaH first_fh = pop_begin_element(*all_boundary_fhs);
			std::queue<OvmFaH> spread_set;
			spread_set.push(first_fh);
			std::vector<OvmFaH> one_patch;
			one_patch.push_back(first_fh);
			while (!spread_set.empty()){
				auto cur_fh = spread_set.front();
				spread_set.pop();
				auto heh_vec = mesh->face(cur_fh).halfedges();
				foreach(auto heh, heh_vec){
					auto eh = mesh->edge_handle(heh);
					if (visitedEdge[eh]) continue;

					OvmFaH adj_fh = mesh->InvalidFaceHandle;
					for (auto hehf_it = mesh->hehf_iter(heh); hehf_it; ++hehf_it)
					{
						auto test_fh = mesh->face_handle(*hehf_it);
						if (test_fh == mesh->InvalidFaceHandle || test_fh == cur_fh)
							continue;
						if (all_boundary_fhs->find(test_fh) == all_boundary_fhs->end())
							continue;
						adj_fh = test_fh;
						break;
					}
					one_patch.push_back(adj_fh);
					visitedEdge[eh] = true;
					spread_set.push(adj_fh);
				}
			}//end while (!spread_set.empty ())...
			quad_patches.push_back(one_patch);
			foreach(auto tmp_fh, one_patch)
				all_boundary_fhs->erase(tmp_fh);
	#ifdef MYDEBUG
			QMessageBox::information(NULL, "faces", QString("one patch %1 finished").arg(one_patch.size()));
	#endif
		}
		delete all_boundary_fhs;
	#ifdef MYDEBUG
		QMessageBox::information(NULL, "faces", QString("sep patches count %1").arg(quad_patches.size()));
	#endif

		foreach(auto &eh, all_ehs_on_geom_edges){
			auto eg = mesh->edge(eh);
			vhs_on_geom_edges.insert(eg.from_vertex());
			vhs_on_geom_edges.insert(eg.to_vertex());
		}

		foreach(auto &one_patch, quad_patches){
			std::unordered_set<OvmVeH> vhs_on_this_patch;
			foreach(auto &fh, one_patch){
				auto adj_vhs = get_adj_vertices_around_face(mesh, fh);
				foreach(auto &vh, adj_vhs){
					if (!contains(vhs_on_geom_edges, vh))
						vhs_on_this_patch.insert(vh);
				}
			}

			//real_vhs_on_face_groups.push_back(vhs_on_this_patch);
		}
	#ifdef MYDEBUG
		QMessageBox::information(NULL, "info", "sep patches finished");
	#endif
		fSmoothAllFaces(smooth_rounds);
	#ifdef MYDEBUG
		QMessageBox::information(NULL, "info", "smooth faces finished");
	#endif
		fSmoothVolumes(smooth_rounds);
	#ifdef MYDEBUG
		QMessageBox::information(NULL, "info", "smooth volume finished");
	#endif
}

void smooth_sphere_mesh (VolumeMesh *mesh, BODY *body, int smooth_rounds)
{
	assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
	//对边的类型统计一下
	std::hash_map<VERTEX*, OvmVeH> vertices_mapping;
	std::hash_map<EDGE*, std::unordered_set<OvmVeH> > edges_vertices_mapping;
	std::hash_map<FACE*, std::unordered_set<OvmVeH> > faces_vertices_mapping;
	std::unordered_set<OvmVeH> volume_vhs, new_boundary_vhs;

	ENTITY_LIST vertices_list, edges_list, faces_list;
	api_get_vertices (body, vertices_list);
	api_get_edges (body, edges_list);
	api_get_faces (body, faces_list);

	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
	{
		auto entity_ptr_uint = V_ENTITY_PTR[*v_it];
		if (entity_ptr_uint == 0) volume_vhs.insert (*v_it);
		else if (entity_ptr_uint == -1){
			//判断一下这个新点是否在体内，如果是的话，则将他的entity_ptr设置为0
			if (!mesh->is_boundary (*v_it)){
				V_ENTITY_PTR[*v_it] = 0;
				volume_vhs.insert (*v_it);
			}else
				new_boundary_vhs.insert (*v_it);
		}else{
			ENTITY *entity = (ENTITY*)(entity_ptr_uint);
			if (is_VERTEX (entity)) vertices_mapping[(VERTEX*)entity] = *v_it;
			else if (is_EDGE (entity)) edges_vertices_mapping[(EDGE*)entity].insert (*v_it);
			else faces_vertices_mapping[(FACE*)entity].insert (*v_it);
		}
	}

	assert (vertices_list.count () == vertices_mapping.size ());

	auto fDivideOneEdge = [&](EDGE *edge, int dis_num, std::vector<SPAposition> &pts){
		SPAinterval inter = edge->param_range ();
		double step = inter.length () / dis_num,
			start_param = inter.start_pt ();
		curve *crv = edge->geometry ()->trans_curve ();

		for (int i = 0; i <= dis_num; ++i){
			double param_val = start_param + step * i;
			SPAposition pos;
			crv->eval (param_val, pos);
			pts.push_back (pos);
		}
	};

	auto fSmoothAllEdges = [&](){
		for (int i = 0; i != edges_list.count (); ++i){
			EDGE *acis_edge = (EDGE*)edges_list[i];
			VERTEX *acis_v1 = acis_edge->start (),
				*acis_v2 = acis_edge->end ();
			OvmVeH mesh_vh1 = vertices_mapping[acis_v1],
				mesh_vh2 = vertices_mapping[acis_v2];
			auto &mesh_vhs_set = edges_vertices_mapping[acis_edge];
			mesh_vhs_set.insert (mesh_vh2);
			std::vector<OvmVeH> sorted_vhs;
			sorted_vhs.push_back(mesh_vh1);
			sorted_vhs.push_back(*mesh_vhs_set.begin());
			sorted_vhs.push_back(mesh_vh2);
			std::vector<SPAposition> pts;
			fDivideOneEdge (acis_edge, sorted_vhs.size () - 1, pts);

			//如果和边的离散顺序相反，则翻转一下
			if (!same_point (pts.front (), POS2SPA(mesh->vertex (sorted_vhs.front ())), SPAresabs * 100))
				std::reverse (sorted_vhs.begin (), sorted_vhs.end ());
			for (int i = 0; i != sorted_vhs.size (); ++i)
				mesh->set_vertex (sorted_vhs[i], SPA2POS(pts[i]));
		}
	};//end auto fSmoothAllEdges = [&](){...

	auto fSmoothAllFaces = [&] (int times){

		for (int i = 0; i != faces_list.count (); ++i){
			FACE *f = (FACE*)(faces_list[i]);
			//first we collect all the vertices handles on this face
			auto vhs_on_this_face = faces_vertices_mapping[f];

			//then we try to smooth them using simplest laplacian smoothing
			int round = 0;
			while (round++ < times){
				foreach (auto &cur_vh, vhs_on_this_face){
					//get all the adjacent vertices of this vertex on the face
					//std::unordered_set<OvmVeH> adj_vhs;
					//get_adj_boundary_vertices_around_vertex (mesh, cur_vh, adj_vhs);
					std::unordered_set<OvmFaH> adj_fhs;
					get_adj_boundary_faces_around_vertex (mesh, cur_vh, adj_fhs);

					OvmVec3d new_pos = OvmVec3d (0, 0, 0);
					foreach (auto &adj_fh, adj_fhs)
						new_pos += mesh->barycenter (adj_fh);
					new_pos /= adj_fhs.size ();
					if (!is_PLANE (f)){
						SPAposition closest_pt;
						double dist = 0.0f;
						api_entity_point_distance (f, POS2SPA(new_pos), closest_pt, dist);
						new_pos = SPA2POS(closest_pt);
					}
					mesh->set_vertex (cur_vh, new_pos);
				}
			}
		}
	};//end auto fSmoothAllFaces = [&] (int times){..

	auto fSmoothVolumes = [&](int times){
		int round = 0;
		while (round++ < times){
			foreach (auto &cur_vh, volume_vhs){
				//std::unordered_set<OvmVeH> adj_vhs;
				//get_adj_vertices_around_vertex (mesh, cur_vh, adj_vhs);
				std::unordered_set<OvmCeH> adj_chs;
				get_adj_hexas_around_vertex (mesh, cur_vh, adj_chs);

				OvmVec3d new_pos = OvmVec3d (0, 0, 0);
				foreach (auto &adj_ch, adj_chs)
					new_pos += mesh->barycenter (adj_ch);
				new_pos /= adj_chs.size ();
				mesh->set_vertex (cur_vh, new_pos);
			}
			//for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it){
			//	if (mesh->is_boundary (*v_it)) continue;
			//	int idx = (*v_it).idx ();
			//	std::unordered_set<OvmVeH> adj_vhs;
			//	get_adj_vertices_around_vertex (mesh, *v_it, adj_vhs);

			//	OvmVec3d new_pos = OvmVec3d (0, 0, 0);
			//	foreach (auto &adj_vh, adj_vhs)
			//		new_pos += mesh->vertex (adj_vh);
			//	new_pos /= adj_vhs.size ();
			//	mesh->set_vertex (*v_it, new_pos);
			//}
		}
	};//end auto fSmoothVolumes = [&](int times){...

	//先光顺边
	fSmoothAllEdges ();
	//在光顺边的时候，会将一些新的未确定位置的点的位置确定，所以先要将剩下的点的位置确定，即确定它们具体在那个面上
	foreach (auto &vh, new_boundary_vhs){
		int idx = vh.idx ();
		if (V_ENTITY_PTR[vh] != -1) continue;

		std::unordered_set<VERTEX*> adj_vertices_ptr;
		std::unordered_set<EDGE*> adj_edges_ptr;
		FACE* adj_face_ptr = NULL;
		std::queue<OvmVeH> spread_set;
		std::unordered_set<OvmVeH> visited_vhs;//遍历过的非点、边上的点，也是一同找到的未确定具体位置的新点
		visited_vhs.insert (vh);
		spread_set.push (vh);

		while (!spread_set.empty ()){
			OvmVeH curr_vh = spread_set.front ();
			spread_set.pop ();
			std::unordered_set<OvmVeH> adj_vhs;
			get_adj_boundary_vertices_around_vertex (mesh, curr_vh, adj_vhs);
			foreach (auto &adj_vh, adj_vhs){
				if (contains (visited_vhs, adj_vh)) continue;

				if (V_ENTITY_PTR[adj_vh] == -1){
					spread_set.push (adj_vh);
					visited_vhs.insert (adj_vh);
					continue;
				}
				ENTITY *entity = (ENTITY*)(V_ENTITY_PTR[adj_vh]);
				if (is_FACE (entity)){
					adj_face_ptr = (FACE*)entity;
					break;
				}else if (is_EDGE (entity)){
					adj_edges_ptr.insert ((EDGE*)entity);
				}else if (is_VERTEX (entity)){
					adj_vertices_ptr.insert ((VERTEX*)entity);
				}
			}
			if (adj_face_ptr)
				break;
		}

		if (adj_face_ptr){
			foreach (auto &vh, visited_vhs){
				V_ENTITY_PTR[vh] = (unsigned long)adj_face_ptr;
				faces_vertices_mapping[adj_face_ptr].insert (vh);
			}
		}
		else if (!adj_edges_ptr.empty ()){
			assert (false);
		}else if (!adj_vertices_ptr.empty ()){
			assert (false);
		}
	}
	fSmoothAllFaces (smooth_rounds);
	fSmoothVolumes (smooth_rounds);
}