#include "stdafx.h"
#include "DMsheet_inflation.h"
//#include "DualOperations.h"
//#include "blockgenwidget.h"
#include "PrioritySetManager.h"
#include <cmath>
#include <map>
#include <geom_utl.hxx>
#include <OpenVolumeMesh/Attribs/StatusAttrib.hh>
#include <OpenVolumeMesh/Core/TopologyKernel.hh>
#include <fstream>
#include <edge.hxx>
#include <plane.hxx>
#include <face.hxx>
#include <surface.hxx>
#include <unitvec.hxx>
#include <OpenVolumeMesh/FileManager/FileManager.hh>
#include "FuncDefs.h"
#include <stdexcept>

#define PI 3.1415926
#define P_A 1
#define P_B 0
#define P_C 0
#define P_D 0


std::ofstream fout("C:/Users/Administrator/Desktop/debug_infor.txt");


static std::unordered_set<OvmFaH> get_common_faces(VolumeMesh *mesh, std::unordered_set<OvmCeH> cells_group1, 
	std::unordered_set<OvmCeH> cells_group2);
static OvmVeH get_opposite_vh_on_cell_group(VolumeMesh *mesh, OvmVeH origin_vh, const std::unordered_set<OvmCeH> &cell_group,
	const std::unordered_set<OvmFaH> &quad_set_fragment);
static unsigned long infer_new_V_ENTITY_PTR_from_two_vhs(VolumeMesh *mesh, OvmVeH origi_vh, OvmVeH oppo_vh);
static unsigned long get_new_V_ENTITY_PTRs_for_ord(VolumeMesh *mesh, OvmVeH origi_vh, const std::unordered_set<OvmCeH> &one_chs_group,
	const std::unordered_set<OvmFaH> &quad_set, const std::unordered_set<OvmCeH> &shrink_set);
static unsigned long get_new_V_ENTITY_PTRs_for_cross(VolumeMesh *mesh, OvmVeH origi_vh, 
	const std::unordered_set<OvmCeH> &one_chs_group, const std::vector<std::unordered_set<OvmCeH> > &cell_groups,
	const std::unordered_set<OvmFaH> &quad_set, const std::unordered_set<OvmCeH> &shrink_set);
static unsigned long get_new_V_ENTITY_PTRs_for_dual_face(VolumeMesh *mesh, OvmVeH origi_vh,
	const std::unordered_set<OvmCeH> &one_chs_group, const std::vector<std::unordered_set<OvmCeH> > &cell_groups,
	const std::unordered_set<OvmFaH> &quad_set, const std::unordered_set<OvmCeH> &shrink_set,
	const std::unordered_set<OvmEgH> &int_ehs, const std::unordered_set<OvmFaH> &int_fhs);
static unsigned long get_new_V_ENTITY_PTR_from_cell_group(VolumeMesh *mesh, OvmVeH origi_vh, 
	const std::unordered_set<OvmCeH> &one_chs_group, const std::vector<std::unordered_set<OvmCeH> > &cell_groups,
	const std::unordered_set<OvmFaH> &quad_set, const std::unordered_set<OvmCeH> &shrink_set,
	const std::unordered_set<OvmEgH> &int_ehs, const std::unordered_set<OvmFaH> &int_fhs);
static OvmVeH get_top_vertex(VolumeMesh *mesh, OvmVeH vh, std::unordered_set<OvmFaH> &int_fhs, 
	std::map<OvmVeH, std::map<OvmVeH, std::unordered_set<OvmCeH> > > *newly_created_vertices_cells_mapping);
//----------------------------------------------------------------------------------------------------------------------------------




// Topology operations relate to cells
static std::unordered_set<OvmFaH> get_common_faces(VolumeMesh *mesh, std::unordered_set<OvmCeH> cells_group1, 
	std::unordered_set<OvmCeH> cells_group2)
{
	std::unordered_set<OvmFaH> fhs_group1, fhs_group2;
	foreach (const auto &ch, cells_group1) {
		auto hfhs = mesh->cell(ch).halffaces();
		foreach (const auto &hfh, hfhs) {
			fhs_group1.insert(mesh->face_handle(hfh));
		}
	}

	foreach (const auto &ch, cells_group2) {
		auto hfhs = mesh->cell(ch).halffaces();
		foreach (const auto &hfh, hfhs) {
			fhs_group2.insert(mesh->face_handle(hfh));
		}
	}

	return intersection(fhs_group1, fhs_group2);
}

// Improvement at 2016.11.08
static OvmVeH get_opposite_vh_on_cell_group(VolumeMesh *mesh, OvmVeH origin_vh, const std::unordered_set<OvmCeH> &cell_group,
	const std::unordered_set<OvmFaH> &quad_set_fragment)
{
	assert (mesh->vertex_property_exists<unsigned long>("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

	std::unordered_set<OvmCeH> adj_chs;
	std::unordered_set<OvmVeH> adj_vhs_on_quad_set;
	std::unordered_set<OvmFaH> adj_fhs_on_quad_set, adj_fhs;

	get_adj_faces_around_vertex(mesh, origin_vh, adj_fhs);
	// Get adj_fhs_on_quad_set
	foreach (const auto &fh, adj_fhs) {
		if (contains(quad_set_fragment, fh)) {
			adj_fhs_on_quad_set.insert(fh);
		}
	}
	// Get adj_vhs_on_quad_set
	foreach (const auto &fh, adj_fhs_on_quad_set) {
		auto vhs = get_adj_vertices_around_face(mesh, fh);
		adj_vhs_on_quad_set.insert(vhs.begin(), vhs.end());	
	}

	get_adj_hexas_around_vertex(mesh, origin_vh, adj_chs);

	// 判断一个点是否在quad set的拐角处
	// Implement in the way which is similar to the main part of "get_opposite_vh_on_cell_group"
	// function.
	// Eg.
	//     #|
	//     #|
	//     #|
	//     #|
	//     #|
	//     #|
	//     #|
	//      0-------------------
	//       ###################
	// 0这个点就在quad set的拐角处
	auto fIsVhOnQuadSetCorner = [&](OvmVeH vh)->bool {
		//std::vector<std::unordered_set<OvmCeH> > cell_groups;
		//get_cell_groups_around_vertex(mesh, vh, quad_set, cell_groups);
		//if (cell_groups.size() == 1) {
		//	return cell_groups[0].size() > 2;
		//}
		//else if (cell_groups.size() == 2) {
		//	return cell_groups[0].size() == cell_groups[1].size();
		//}
		//return false;

		foreach (const auto &ch, adj_chs) {
			if (contains(cell_group, ch)) {
				std::unordered_set<OvmEgH> ehs;
				get_adj_edges_around_cell(mesh, ch, ehs);
				foreach (const auto &eh, ehs) {
					auto vh1 = mesh->edge(eh).from_vertex();
					auto vh2 = mesh->edge(eh).to_vertex();
					if (vh1 == vh && !contains(adj_vhs_on_quad_set, vh2)) {
						return false;
					}
					if (vh2 == vh && !contains(adj_vhs_on_quad_set, vh1)) {
						return false;
					}
				}

				break;
			}
		}
		return true;
	};

	//qDebug() << "===================";
	//qDebug() << "origin_vh: " << origin_vh.idx();
	//qDebug() << "__LINE__: " << __LINE__;
	//qDebug() << "===================";
	//if (origin_vh.idx() == 4697) {
	//	qDebug() << "====================";
	//	qDebug() << "origin vh: " << origin_vh.idx();
	//	qDebug() << "The info of quad set fragment: ";
	//	foreach (auto &fh, quad_set_fragment) {
	//		qDebug() << "fh: " << fh.idx();
	//	}
	//	qDebug() << "The info of shrink set:";
	//	foreach (auto &ch, cell_group) {
	//		qDebug() << "ch: " << ch.idx();
	//	}
	//	
	//	qDebug() << "====================";
	//}



	// 为了处理quad set的拐角处的情况
	//auto helper_vh = mesh->InvalidVertexHandle;
	std::vector<OvmVeH> helper_vhs;

	// 为了处理非结构化的shrink set
	int oppo_vhs_num = 0;  
	std::unordered_set<OvmVeH> candidate_vhs;

	foreach (const auto &ch, adj_chs) {
		if (contains(cell_group, ch)) {
			std::unordered_set<OvmEgH> ehs;
			get_adj_edges_around_cell(mesh, ch, ehs);
			foreach (const auto &eh, ehs) {
				auto vh1 = mesh->edge(eh).from_vertex();
				auto vh2 = mesh->edge(eh).to_vertex();
				if (vh1 == origin_vh) {
					//if (!contains(adj_vhs_on_quad_set, vh2) && V_ENTITY_PTR[vh2] != 0) {
					if (!contains(adj_vhs_on_quad_set, vh2)) {
						oppo_vhs_num++;
						//return vh2;
						candidate_vhs.insert(vh2);
					}
					else if (fIsVhOnQuadSetCorner(origin_vh)) {
						//helper_vh = vh2;
						helper_vhs.push_back(vh2);
					}
				}
				else if (vh2 == origin_vh) {
					//if (!contains(adj_vhs_on_quad_set, vh1) && V_ENTITY_PTR[vh1] != 0) {
					if (!contains(adj_vhs_on_quad_set, vh1)) {
						oppo_vhs_num++;
						//return vh1;
						candidate_vhs.insert(vh1);
					}
					else if (fIsVhOnQuadSetCorner(origin_vh)) {
						//helper_vh = vh1;
						helper_vhs.push_back(vh1);
					}
				}
			}
		}
	}

	//if (origin_vh.idx() == 30) {
	//	qDebug() << "====================";
	//	qDebug() << "origi vh: " << origin_vh.idx();
	//	if (fIsVhOnQuadSetCorner(origin_vh)) {
	//		qDebug() << "origi vh is on quad set corner.";
	//	}
	//	else {
	//		qDebug() << "origi vh is not on quad set corner.";
	//	}
	//	qDebug() << "helper vh:" << helper_vh.idx();
	//	qDebug() << "====================";
	//}

	if (oppo_vhs_num > 0) {
		auto origin_entity = (ENTITY*) V_ENTITY_PTR[origin_vh];
		foreach (const auto &vh, candidate_vhs) {
			auto entity = (ENTITY*) V_ENTITY_PTR[vh];
			if (is_VERTEX(entity)) {
				if (is_VERTEX(origin_entity)) {
					ENTITY_LIST adj_edges_to_entity;
					ENTITY_LIST adj_edges_to_origin_entity;
					api_get_edges(entity, adj_edges_to_entity);
					api_get_edges(origin_entity, adj_edges_to_origin_entity);
					for (int i = 0; i < adj_edges_to_entity.count(); ++i) {
						if (adj_edges_to_origin_entity.lookup(adj_edges_to_entity[i]) != -1) {
							return vh;
						}
					}

				}
				else if (is_EDGE(origin_entity)) {
					ENTITY_LIST adj_edges;
					api_get_edges(entity, adj_edges);
					if (adj_edges.lookup(origin_entity) != -1) {
						return vh;
					}
				}
			}
			else if (is_EDGE(entity)) {
				if (is_VERTEX(origin_entity)) {
					ENTITY_LIST adj_edges;
					api_get_edges(origin_entity, adj_edges);
					if (adj_edges.lookup(entity) != -1) {
						return vh;
					}
				}
				else if (is_EDGE(origin_entity)) {
					if (V_ENTITY_PTR[vh] == V_ENTITY_PTR[origin_vh]) {
						return vh;
					}
				}
			}
		}

		//if (origin_vh.idx() == 814) {
		//	qDebug() << "+++++++++++++++++++++";
		//	qDebug() << "origin vh: " << origin_vh.idx();
		//	qDebug() << "The info of candidate_vhs: ";
		//	foreach (auto &vh, candidate_vhs) {
		//		qDebug() << "vh: " << vh.idx();
		//		qDebug() << "V_ENTITY_PTR: " << V_ENTITY_PTR[vh];
		//	}
		//	unsigned long x = -1;
		//	qDebug() << "X= " << x;
		//	qDebug() << "+++++++++++++++++++++";
		//}

		foreach (auto &vh, candidate_vhs) {
			unsigned long MAX_V = -1;
			if (V_ENTITY_PTR[vh] != MAX_V && V_ENTITY_PTR[vh] != 0) {
				return vh;
			}
		}

		return *candidate_vhs.begin();
	}
	// original_vh 在quad set的拐角处
	else {
		assert(helper_vhs.size() == 2);

		//if (origin_vh.idx() == 4697) {
		//	qDebug() << "__LINE__: " << __LINE__;
		//	qDebug() << "--------------------";
		//}

		// 在一个网格面上，得到一个点对角的那个点
		auto fGetDiagonalVhOfFh = [&](OvmVeH vh0, OvmFaH fh) -> OvmVeH {
			std::vector<OvmVeH> vhs = get_adj_vertices_around_face(mesh, fh);
			auto iter = std::find(vhs.begin(), vhs.end(), vh0);
			int index = -1;
			for (int i = 0; i < vhs.size(); ++i) {
				if (vhs[i] == vh0) {
					index = i;
					break;
				}
			}
			if (index == -1) {
				return mesh->InvalidVertexHandle;
			}
			else {
				return vhs[(index+2) % vhs.size()];
			}
		};

#define NEW_WAY
#ifdef NEW_WAY
		//std::unordered_set<OvmFaH> adj_fhs; Definition before...
		//std::unordered_set<OvmVeH> adj_vhs_on_quad_set; Definition before...
		foreach (const auto &fh, adj_fhs) {
			if (!contains(quad_set_fragment, fh)) {
				std::vector<OvmVeH> vhs = get_adj_vertices_around_face(mesh, fh);
				std::vector<int> vhs_idxes, adj_vhs_on_quad_set_idxes;
				std::for_each(vhs.begin(), vhs.end(), [&vhs_idxes](OvmVeH vh) { vhs_idxes.push_back(vh.idx()); } );
				
				std::vector<int> comm_vhs_idxes;
				std::for_each(adj_vhs_on_quad_set.begin(), adj_vhs_on_quad_set.end(), 
							  [&adj_vhs_on_quad_set_idxes](OvmVeH vh) { adj_vhs_on_quad_set_idxes.push_back(vh.idx()); } );

				/*
				std::set_intersection(vhs_idxes.begin(), vhs_idxes.end(), 
									  adj_vhs_on_quad_set_idxes.begin(), adj_vhs_on_quad_set_idxes.end(),
									  std::back_inserter(comm_vhs_idxes));
				*/
				foreach (const auto &idx0, vhs_idxes) {
					if (contains(adj_vhs_on_quad_set_idxes, idx0)) {
						comm_vhs_idxes.push_back(idx0);
					}
				}

				//if (fh.idx() == 1384 && origin_vh.idx() == 10) {
				//	fout << "The size of vhs: " << vhs.size() << std::endl;
				//	foreach (auto &i, vhs_idxes) {
				//		fout << i << std::endl;
				//	}
				//	fout << "The size of adj_vhs_on_quad_set: " << adj_vhs_on_quad_set.size() << std::endl;
				//	foreach (auto &i, adj_vhs_on_quad_set_idxes) {
				//		fout << i << std::endl;
				//	}
				//	fout << "The size of comm_vhs: " << comm_vhs_idxes.size() << std::endl;
				//	foreach (auto i, comm_vhs_idxes) {
				//		fout << i << std::endl;
				//	}
				//	fout << "----------------" << std::endl;
				//}

				if (comm_vhs_idxes.size() == 3) {
					//fout << "Origin vh: " << origin_vh.idx() << std::endl;
					//fout << "Diagonal vh: " << fGetDiagonalVhOfFh(origin_vh, fh) << std::endl;
					return fGetDiagonalVhOfFh(origin_vh, fh);
				}
			}
		}
#endif

		
		//////////////////////////////////////////////////////
		// 之前实现的方法，有问题
		//////////////////////////////////////////////////////
#ifdef OLD_WAY 
		// 找对角的点
		std::unordered_set<OvmVeH> adj_vhs0, adj_vhs1;
		get_adj_vertices_around_vertex(mesh, helper_vhs[0], adj_vhs0);
		get_adj_vertices_around_vertex(mesh, helper_vhs[1], adj_vhs1);
		foreach (const auto &vh, intersection(adj_vhs0, adj_vhs1)) {
			if (vh != origin_vh) {
				return vh;
			}
		}
#endif

		//return get_opposite_vh_on_cell_group(mesh, helper_vhs[0], cell_group, quad_set_fragment);
		return origin_vh;
	}
}

// Return the vh's suitable V_ENTITY_PTR
// 根据部分的quad set, shrink set从original_vh推导出分裂出的新点的几何归属
// 核心思想是认为新分裂出的点应该归属于原点和shrink set方向的六面体的对点连线上。
// 原点origianl_vh
// shrink set方向的六面体的对点 adj_vh
static unsigned long infer_new_V_ENTITY_PTR_from_two_vhs(VolumeMesh *mesh, OvmVeH origi_vh, OvmVeH oppo_vh)
{
	assert (origi_vh != mesh->InvalidVertexHandle);
	assert (oppo_vh != mesh->InvalidVertexHandle);
	assert (mesh->vertex_property_exists<unsigned long>("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
	unsigned long origin_V_ENTITY_PTR = V_ENTITY_PTR[origi_vh];

	auto entity = (ENTITY*) (V_ENTITY_PTR[origi_vh]);
	auto oppo_entity = (ENTITY*) (V_ENTITY_PTR[oppo_vh]);

	// 原来的点在体内的情况
	if (origin_V_ENTITY_PTR == 0) {
		return 0;
	}
	else if (V_ENTITY_PTR[oppo_vh] == 0) { // 对点在体内，向体内inflation
		return 0;
	}
	else if (is_FACE(entity)){ // 原来的点在面上
		if (V_ENTITY_PTR[oppo_vh] != 0) { // 原来的点在面的边界上
			return origin_V_ENTITY_PTR;
		}
		else { // 原来的点在面的里面
			return 0; // 新分裂出来的点在体内
		}
	}
	else if (is_EDGE(entity)) {
		if (V_ENTITY_PTR[oppo_vh] != 0) { // 原来的点在边界上
			if (V_ENTITY_PTR[oppo_vh] == origin_V_ENTITY_PTR) { // (1) 分裂出的新点还在这条几何边上
				return origin_V_ENTITY_PTR;
			}
			else {
				if (is_FACE(oppo_entity)) {                     // (2) 分裂出的新点在面上
					return V_ENTITY_PTR[oppo_vh];
				}
				else {
					if (is_VERTEX(oppo_entity)) {              // (3) 分裂出的新点可能在点上，并且这个点与原来点所在的边，相连

						//ENTITY_LIST adj_edges_to_entity;
						//api_get_edges(entity, adj_edges_to_entity);
						//for (int i = 0; i != adj_edges_to_entity.count(); ++i) {
						//	ENTITY_LIST adj_vertices_to_edge;
						//	api_get_vertices(adj_edges_to_entity[i], adj_vertices_to_edge);
						//	if (adj_vertices_to_edge.lookup(entity) != -1) {
						//		return (unsigned long) adj_edges_to_entity[i];
						//	}
						//}
						ENTITY_LIST adj_edges_to_oppo_entity;
						api_get_edges(oppo_entity, adj_edges_to_oppo_entity);
						if (adj_edges_to_oppo_entity.lookup(entity) != -1) {
							return (unsigned long) entity;
						}
					}
					// (4) 剩下的情况,分裂出的新点在点上或边上
					ENTITY_LIST  adj_faces_to_entity;
					ENTITY_LIST adj_faces_to_new_entity;
					api_get_faces(entity, adj_faces_to_entity);
					api_get_faces(oppo_entity, adj_faces_to_new_entity);
					for (int i = 0; i != adj_faces_to_entity.count(); i++) {
						if (adj_faces_to_new_entity.lookup(adj_faces_to_entity[i]) != -1) {
							return (unsigned long) adj_faces_to_entity[i];
						}						
					}
				}
			}
		}
		else {
			return 0;
		}
	}
	else if (is_VERTEX(entity)) {
		if (V_ENTITY_PTR[oppo_vh] != 0) { // 原来的点在边界上
			if (is_EDGE(oppo_entity)) {                 // (1) 分裂出来的新点在边上 misunderstanding.这里应该要分情况来讨论
				// 新点在边上，且这条边和旧点相连
				ENTITY_LIST adj_vertices_to_new_entity;
				api_get_vertices(oppo_entity, adj_vertices_to_new_entity);
				for (int i = 0; i < adj_vertices_to_new_entity.count(); ++i) {
					if (adj_vertices_to_new_entity[i] == entity) {
						return V_ENTITY_PTR[oppo_vh];
					}
				}

				// 新点在边上，但这条边不和旧点相连
				ENTITY_LIST adj_faces_to_entity;
				ENTITY_LIST adj_faces_to_new_entity;
				api_get_faces(entity, adj_faces_to_entity);
				api_get_faces(oppo_entity, adj_faces_to_new_entity);
				for (int i = 0; i < adj_faces_to_entity.count(); ++i) {
					if (adj_faces_to_new_entity.lookup(adj_faces_to_entity[i]) != -1) {
						return (unsigned long) adj_faces_to_entity[i];
					}
				}
			}
			else if (is_FACE(oppo_entity)) {            // (2) 分裂出的新点在面上
				return V_ENTITY_PTR[oppo_vh]; 
			}
			else if (is_VERTEX(oppo_entity)) {
				// 新点和旧点之间有边连接
				ENTITY_LIST adj_edges_to_entity;
				ENTITY_LIST adj_edges_to_new_entity;
				api_get_edges(entity, adj_edges_to_entity);
				api_get_edges(oppo_entity, adj_edges_to_new_entity);
				for (int i = 0; i < adj_edges_to_entity.count(); i++) {
					if (adj_edges_to_new_entity.lookup(adj_edges_to_entity[i]) != -1) {
						return (unsigned long) adj_edges_to_entity[i];
					}
				}

				// 新点和旧点之间没有边连接
				ENTITY_LIST adj_faces_to_entity;
				ENTITY_LIST adj_faces_to_new_entity;
				api_get_faces(entity, adj_faces_to_entity);
				api_get_faces(oppo_entity, adj_faces_to_new_entity);
				for (int i = 0; i < adj_faces_to_entity.count(); ++i) {
					if (adj_faces_to_new_entity.lookup(adj_faces_to_entity[i]) != -1) {
						return (unsigned long) adj_faces_to_entity[i];
					}
				}
			}
		}
		else {
			return 0;
		}
	}

	return origin_V_ENTITY_PTR;
}


// 针对普通情况（就是一个点分裂成两个点的情况）分裂出的点们的几何归属
static unsigned long get_new_V_ENTITY_PTRs_for_ord(VolumeMesh *mesh, OvmVeH origi_vh, const std::unordered_set<OvmCeH> &one_chs_group,
	const std::unordered_set<OvmFaH> &quad_set, const std::unordered_set<OvmCeH> &shrink_set)
{
	assert (mesh->vertex_property_exists<unsigned long>("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

	if (!intersects(one_chs_group, shrink_set)) {
		return V_ENTITY_PTR[origi_vh];
	}
	else {
		auto oppo_vh = get_opposite_vh_on_cell_group(mesh, origi_vh, one_chs_group, quad_set);

		auto new_V_ENTITY_PTR = infer_new_V_ENTITY_PTR_from_two_vhs(mesh, origi_vh, oppo_vh);
		if (new_V_ENTITY_PTR == 0) {
			return -1;
		}
		else {
			return new_V_ENTITY_PTR;
		}
	}
}

// 针对十字相交的情况，得到quad sets十字交叉处的点分裂出的点们的几何归属
// quad sets相交成十字状，将和origi_vh相连的空间分成4部分，分别对应一个one_chs_group
// 我们根据one_chs_group得到相应的分裂出的新点的几何归属
// Eg.
// # 表示shrink set
//                    |#
//                    |#
//                    |#
//                    |#
//                [0] |# [1]
//--------------------|#---------------------
//####################|######################
//                [2] |# [3]
//                    |#
//                    |#
//                    |#
//                    |#
// 有三类one_chs_group
// 第一类one_chs_group: 如图[0]
//   特点： 没有和shrink set相交
//   新的点的几何归属： 原点的几何归属
//
// 第二类one_chs_group: 如图[1][2]
//   特点： 和shrink set相交且和第一类one_chs_group相邻
//   新的点的几何归属: 根据原点和原点在这个one_chs_group方向（同shrink set一个方向）的一个对点（共享同一条边）
//                   (所用的方法和get_new_V_ENTITY_PTRs_for_ord)一样，来得到新的几何归属
// 
// 第三类one_chs_group: 如图[3]
//   特点：和shrink set相交但不和第一类one_chs_group相邻
//   新的点的几何归属：根据原点所对应的[1][2]两个one_chs_group方向的对应的点所共享的面的几何归属
static unsigned long get_new_V_ENTITY_PTRs_for_cross(VolumeMesh *mesh, OvmVeH origi_vh, 
	const std::unordered_set<OvmCeH> &one_chs_group, const std::vector<std::unordered_set<OvmCeH> > &cell_groups,
	const std::unordered_set<OvmFaH> &quad_set, const std::unordered_set<OvmCeH> &shrink_set)
{
	assert (mesh->vertex_property_exists<unsigned long>("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

	// 处理第一类one_chs_group
	// 如果one_chs_group没有和shrink_set相交，那我们就把和one_chs_group相关联的新点的几何归属设置为
	// 分裂前的点的几何归属
	if (!intersects(one_chs_group, shrink_set)) {
		return V_ENTITY_PTR[origi_vh];
	}

	// 得到没有在shrink set中的chs_group
	std::unordered_set<OvmCeH> chs_groups_not_in_shrink_set;
	foreach (const auto &chs_group, cell_groups) {
		if (!intersects(chs_group, shrink_set)) {
			chs_groups_not_in_shrink_set = chs_group;
			break;
		}
	}
	assert (chs_groups_not_in_shrink_set.size() > 0);

	// 得到两个chs_group,和chs_groups_not_in_shrink_set相邻，
	std::vector<std::unordered_set<OvmCeH> > adj_chs_group_vec;
	auto fGetCommonFhForChs = [&](std::unordered_set<OvmCeH> chs1, std::unordered_set<OvmCeH> chs2)->OvmFaH {
		foreach (auto ch1, chs1) {
			foreach (auto ch2, chs2) {
				auto fh = get_common_face_handle(mesh, ch1, ch2);
				if (fh != mesh->InvalidFaceHandle) {
					return fh;
				}
			}
		}
		return mesh->InvalidFaceHandle;
	};

	foreach (const auto &chs_group, cell_groups) {
		if (intersects(chs_group, shrink_set) && 
			(fGetCommonFhForChs(chs_groups_not_in_shrink_set, chs_group) != mesh->InvalidFaceHandle)) {
				adj_chs_group_vec.push_back(chs_group);
		}
	}
	assert (adj_chs_group_vec.size() == 2);

	// 处理第二类one_chs_group
	if (std::find(adj_chs_group_vec.begin(), adj_chs_group_vec.end(), one_chs_group) != adj_chs_group_vec.end()) {
		auto fh = fGetCommonFhForChs(one_chs_group, chs_groups_not_in_shrink_set);
		std::unordered_set<OvmFaH> quad_set_fragment;
		quad_set_fragment.insert(fh);
		unsigned long new_V_ENTITY_PTR = get_new_V_ENTITY_PTRs_for_ord(mesh, origi_vh, one_chs_group, quad_set_fragment, shrink_set);
		// unsigned long V_ENTITY_PTR = get_new_V_ENTITY_PTRs_for_ord(mesh, origi_vh, one_chs_group, quad_set_fragment, one_chs_group);
		
		//if (origi_vh.idx() == 4697) {
		//	qDebug() << "origin vh: " << origi_vh.idx();
		//	qDebug() << "Adj fh:" << fh.idx();
		//	qDebug() << "New_V_ENTITY_PTRs: " << new_V_ENTITY_PTR;
		//	qDebug() << "------------";
		//}
		
		return new_V_ENTITY_PTR;
	}

	// 处理第三类one_chs_group
	// 一定归属于面上
	std::unordered_set<OvmFaH> quad_set_fragment1;
	std::unordered_set<OvmFaH> quad_set_fragment2;
	auto comm_fh1 = fGetCommonFhForChs(adj_chs_group_vec[0], chs_groups_not_in_shrink_set);
	auto comm_fh2 = fGetCommonFhForChs(adj_chs_group_vec[1], chs_groups_not_in_shrink_set); 
	quad_set_fragment1.insert(comm_fh1);
	quad_set_fragment2.insert(comm_fh2);

	auto oppo_vh1 = get_opposite_vh_on_cell_group(mesh, origi_vh, adj_chs_group_vec[0], quad_set_fragment1);
	auto oppo_vh2 = get_opposite_vh_on_cell_group(mesh, origi_vh, adj_chs_group_vec[1], quad_set_fragment2);
	auto entity1 = (ENTITY*) V_ENTITY_PTR[oppo_vh1];
	auto entity2 = (ENTITY*) V_ENTITY_PTR[oppo_vh2];

	ENTITY_LIST adj_faces_to_entity1;
	ENTITY_LIST adj_faces_to_entity2;
	api_get_faces(entity1, adj_faces_to_entity1);
	api_get_faces(entity2, adj_faces_to_entity2);

	for (int i = 0; i < adj_faces_to_entity1.count(); ++i) {
		if (adj_faces_to_entity2.lookup(adj_faces_to_entity1[i]) != -1) {
			return (unsigned long) adj_faces_to_entity1[i];
		}
	}
}


static unsigned long get_new_V_ENTITY_PTRs_for_dual_face(VolumeMesh *mesh, OvmVeH origi_vh,
	const std::unordered_set<OvmCeH> &one_chs_group, const std::vector<std::unordered_set<OvmCeH> > &cell_groups,
	const std::unordered_set<OvmFaH> &quad_set, const std::unordered_set<OvmCeH> &shrink_set,
	const std::unordered_set<OvmEgH> &int_ehs, const std::unordered_set<OvmFaH> &int_fhs
	)
{
	assert (mesh->vertex_property_exists<unsigned long>("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

	// 如果one_chs_group没有和shrink_set相交，那我们就把和one_chs_group相关联的新点的几何归属设置为
	// 分裂前的点的几何归属
	if (!intersects(one_chs_group, shrink_set)) {
		return V_ENTITY_PTR[origi_vh];
	}

	// 判断是用于第一类sheet(不改变sheet走向)
	// 还是第二类sheet(改变sheet走向)
	std::unordered_set<OvmVeH> vhs_on_int_ehs;
	foreach (const auto &eh, int_ehs) {
		auto vh1 = mesh->edge(eh).from_vertex();
		auto vh2 = mesh->edge(eh).to_vertex();
		vhs_on_int_ehs.insert(vh1);
		vhs_on_int_ehs.insert(vh2);
	}

	// 第一类sheet(不改变sheet走向)
	if (!contains(vhs_on_int_ehs, origi_vh)) {
		return get_new_V_ENTITY_PTRs_for_ord(mesh, origi_vh, one_chs_group, quad_set, shrink_set);
	}
	// 第二类sheet(改变sheet走向)
	else {
		// 先判断one_chs_group的位置
		assert(cell_groups.size() == 3);
		std::unordered_set<OvmCeH> middle_cell_group; // 在中间的，不是在两边的cell_group

		for (int i = 0; i < cell_groups.size(); ++i) {
			for (int j = i+1; j < cell_groups.size(); ++j) {
				std::unordered_set<OvmFaH> comm_fhs = get_common_faces(mesh, cell_groups[i], cell_groups[j]);
				if (intersects(comm_fhs, int_fhs)) {
					int mid_index = (1+2+3) - (i+1) - (j+1) - 1;
					middle_cell_group = cell_groups[mid_index];
				}
			}
		}

		// one_chs_group 在中间
		if (one_chs_group == middle_cell_group) {
			return get_new_V_ENTITY_PTRs_for_ord(mesh, origi_vh, one_chs_group, quad_set, shrink_set);
		}
		// one_chs_group 在两边
		else {
			// 需要新的one_cell_group和quad_set
			std::unordered_set<OvmCeH> new_cell_group = get_union(one_chs_group, middle_cell_group);
			std::unordered_set<OvmFaH> new_quad_set;

			
			// 更新quad_set
			std::unordered_set<OvmFaH> adj_fhs, adj_fhs_on_quad_set;
			get_adj_faces_around_vertex(mesh, origi_vh, adj_fhs);
			foreach (const auto &fh, adj_fhs) {
				if (contains(quad_set, fh)) {
					adj_fhs_on_quad_set.insert(fh);
				}
			}

			std::unordered_set<OvmFaH> comm_fhs = get_common_faces(mesh, middle_cell_group, one_chs_group);
			new_quad_set = difference(adj_fhs_on_quad_set, comm_fhs);

			return get_new_V_ENTITY_PTRs_for_ord(mesh, origi_vh, new_cell_group, new_quad_set, shrink_set);
		}
	}

	return 0;
}

// 得到分裂的新点的几何归属
static unsigned long get_new_V_ENTITY_PTR_from_cell_group(VolumeMesh *mesh, OvmVeH origi_vh, 
	const std::unordered_set<OvmCeH> &one_chs_group, const std::vector<std::unordered_set<OvmCeH> > &cell_groups,
	const std::unordered_set<OvmFaH> &quad_set, const std::unordered_set<OvmCeH> &shrink_set,
	const std::unordered_set<OvmEgH> &int_ehs, const std::unordered_set<OvmFaH> &int_fhs)
{
	assert (mesh->vertex_property_exists<unsigned long>("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

	// qDebug() << "get_new_V_ENTITY_PTR_from_cell_group";
	// 分裂的点在体内
	//if (origi_vh.idx() == 3289) {
	//	qDebug() << "------------------";
	//	qDebug() << "one_chs_group about: " << origi_vh.idx();
	//	foreach (auto &ch, one_chs_group) {
	//		qDebug() << ch.idx();
	//	}
	//	qDebug() << "------------------";
	//}

	if (V_ENTITY_PTR[origi_vh] == 0) {
		return 0;
	}

	// 分裂的点在边界上
	if (cell_groups.size() == 2) {
		//qDebug() << "cell_groups.size() == 2";
		return get_new_V_ENTITY_PTRs_for_ord(mesh, origi_vh, one_chs_group, quad_set, shrink_set);
	}
	else if (cell_groups.size() == 4) {
		//qDebug() << "cell_groups.size() == 4";
		return get_new_V_ENTITY_PTRs_for_cross(mesh, origi_vh, one_chs_group, cell_groups, quad_set, shrink_set);
	}
	else if (cell_groups.size() == 3) {
		//qDebug() << "cell_groups.size() == 3";
		return get_new_V_ENTITY_PTRs_for_dual_face(mesh, origi_vh, one_chs_group, cell_groups, quad_set, shrink_set, 
												   int_ehs, int_fhs);
	}
	return V_ENTITY_PTR[origi_vh];
}

static OvmVeH get_top_vertex(VolumeMesh *mesh, OvmVeH vh, std::unordered_set<OvmFaH> &int_fhs, 
	std::map<OvmVeH, std::map<OvmVeH, std::unordered_set<OvmCeH> > > *newly_created_vertices_cells_mapping)
{
	auto locate = newly_created_vertices_cells_mapping->find(vh);
	assert(locate != newly_created_vertices_cells_mapping->end());
	auto mapping = locate->second;

	foreach (auto &iter1, mapping) {
		bool has_common_face = false;
		foreach (auto &iter2, mapping) {
			if ((iter1.first).idx() == (iter2.first).idx()) continue;
			auto common_faces = get_common_faces(mesh, iter1.second, iter2.second); 
			if (intersection(common_faces, int_fhs).size() > 0) has_common_face = true;
		}
		if (!has_common_face) return iter1.first;
	}

	return mesh->InvalidVertexHandle;
}


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

std::vector<DualSheet *> DM_sheet_inflation(VolumeMesh *mesh, std::vector<std::unordered_set<OvmFaH> > &quad_sets, std::vector<std::unordered_set<OvmCeH> >& shrink_sets)
{
	std::unordered_set<OvmEgH> int_ehs, int_ehs_on_int_fhs;
	std::unordered_set<OvmFaH> int_fhs;

	std::unordered_set<OvmFaH> inflation_quad_set;
	std::unordered_set<OvmCeH> shrink_set;
	std::hash_map<OvmVeH, OvmVeH> old_new_vhs_mapping;

	std::vector<std::unordered_set<OvmFaH> > dual_face_sets;


	//--------------------------------------------------------------------
	// 得到inflation_quad_set(所有quad_set的并集)
	// 得到int_fhs(quad sets相交面面集)
	// 得到shrink_set(所有shrink_sets的并集)
	auto fDealQuadsetAndShrinkset = [&]() {
		foreach (const auto &quad_set, quad_sets) {
			foreach (const auto &fh, quad_set) {
				if (contains(inflation_quad_set, fh)) {
					int_fhs.insert(fh);
				}
				else {
					inflation_quad_set.insert(fh);
				}
			}
		}

		for (int i = 0; i < shrink_sets.size(); i++) {
			//qDebug() << "i: " << i;
			shrink_set = get_union(shrink_set, shrink_sets[i]);
		}
	};


	//--------------------------------------------------------------------
	// 得到int_ehs(quad sets相交出来的交线边边集) 
	auto fGetGeneralIntEhs = [](VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmEgH> &int_ehs) {
		int_ehs.clear();
		foreach (const auto &fh, inflation_quad_set) {
			auto hehs = mesh->face(fh).halfedges();
			foreach (const auto &heh, hehs) {
				auto eh = mesh->edge_handle(heh);
				auto fhs = get_adj_faces_around_edge(mesh, eh, false);
				int adj_fs_num = 0;
				foreach (const auto &fh2, fhs) {
					if (contains(inflation_quad_set, fh2)) adj_fs_num++;
				}
				if (adj_fs_num >= 3) {
					int_ehs.insert(eh);
				}
			}
		}
	};


	//--------------------------------------------------------------------
	// 针对的情况： 只需要连一次
	//
	// 从int_ehs(quad sets相交出来的交线边边集)出发
	// 得到在int_ehs_on_int_fhs(quad sets相交面边界上的边集)中的边集，使得能够统一地生成连在一起的六面体
	// 得到的边集是能够与int_ehs想连通的
	// 函数返回：边集
	auto fGetEhsConnectedIntEhs = [&]()->std::unordered_set<OvmEgH> {
		std::unordered_set<OvmEgH> res_ehs;

		// 判断两个边是否相连(是否能够生成连着的六面体)
		auto fConnected = [&](OvmEgH eh0, OvmEgH eh1)->bool {
			OvmVeH vh0 = mesh->edge(eh0).from_vertex();
			OvmVeH vh1 = mesh->edge(eh0).to_vertex();
			std::vector<OvmVeH> vhs_0; vhs_0.push_back(vh0); vhs_0.push_back(vh1);
			std::vector<OvmFaH> fhs_0 = get_adj_faces_around_edge(mesh, eh0);

			OvmVeH vh2 = mesh->edge(eh1).from_vertex();
			OvmVeH vh3 = mesh->edge(eh1).to_vertex();
			std::vector<OvmVeH> vs_1; vs_1.push_back(vh2); vs_1.push_back(vh3);
			std::vector<OvmVeH> comm_vhs;
			std::set_intersection(vhs_0.begin(), vhs_0.end(), vs_1.begin(), vs_1.end(),
				std::back_inserter(comm_vhs));

			if (!comm_vhs.empty()) {
				std::vector<OvmFaH> fhs_1 = get_adj_faces_around_edge(mesh, eh1);
				std::vector<OvmFaH> comm_fhs;
				std::set_intersection(fhs_0.begin(), fhs_0.end(), fhs_1.begin(), fhs_1.end(),
					std::back_inserter(comm_fhs));
				if (comm_fhs.empty()) {
					return true;
				}
			}

			return false;
		};

		OvmEgH border_eh = mesh->InvalidEdgeHandle;
		foreach (const auto &eh0, int_ehs) {
			bool find = false;
			foreach (const auto &eh1, int_ehs_on_int_fhs) {
				if (fConnected(eh0, eh1)) {
					find = true;
				}
			}
			if (find) {
				border_eh = eh0;
			}
		}
		if (border_eh == mesh->InvalidEdgeHandle) {
			return res_ehs;
		}

		std::unordered_set<OvmEgH> visited_ehs;
		bool flag = true;
		while (flag) {
			flag = false;
			foreach (const auto &eh, int_ehs_on_int_fhs) {
				if (contains(visited_ehs, eh)) continue;
				if (fConnected(border_eh, eh)) {
					flag = true;
					visited_ehs.insert(eh);
					res_ehs.insert(eh);
					border_eh = eh;
					break;
				}
			}
		}

		return res_ehs;
	};


	//--------------------------------------------------------------------
	// 得到int_ehs(quad sets十字相交出来的线的边集)也包括连在一起的int_ehs_on_int_fhs(部分，为能统一地生成连在一起的六面体)
	// 得到int_ehs_on_int_fhs(quad sets相交面边界上的边集)
	auto fGetIntEhs = [&]() {
		fGetGeneralIntEhs(mesh, inflation_quad_set, int_ehs);	

		std::unordered_set<OvmEgH> ehs_on_int_fhs;
		foreach (const auto &fh, int_fhs) {
			foreach (const auto &heh, mesh->face(fh).halfedges()) {
				ehs_on_int_fhs.insert(mesh->edge_handle(heh));
			}
		}
		foreach (const auto &eh, int_ehs) {
			if (contains(ehs_on_int_fhs, eh)) {
				int_ehs_on_int_fhs.insert(eh);
			}
		}

		// 修改int_ehs使其只包含普通相交的边，这个样的相交边只在相交面上
		foreach (const auto &eh, int_ehs_on_int_fhs) {
			int_ehs.erase(eh);
		}

		// 根据得到的ehs_connected_int_ehs来改变
		// int_ehs和int_ehs_on_int_fhs的值
		auto connected_ehs = fGetEhsConnectedIntEhs();
		foreach (const auto &eh, connected_ehs) {
			int_ehs.insert(eh);
			int_ehs_on_int_fhs.erase(eh);
		}

		// 通过shrink set来得到应用于生成一串六面体的一串边集, 这串六面体能够改变生成sheet的走向 // 用作第二类生成sheet
		// 将其加入int_ehs
		std::unordered_set<OvmCeH> comm_shrink_set;
		std::unordered_set<OvmEgH> ehs_on_comm_shrink_set;
		for (auto it1 = shrink_sets.begin(); it1 != shrink_sets.end(); ++it1) {
			for (auto it2 = shrink_sets.begin(); it2 != shrink_sets.end(); ++it2) {
				if (it1 != it2) {
					auto comm_chs = intersection(*it1, *it2);
					comm_shrink_set.insert(comm_chs.begin(), comm_chs.end());
				}
			}
			std::unordered_set<OvmEgH> com_ehs, tmp_ehs;
			foreach (const auto &ch, comm_shrink_set) {
				tmp_ehs.clear();
				get_adj_edges_around_cell(mesh, ch, tmp_ehs);
				ehs_on_comm_shrink_set.insert(tmp_ehs.begin(), tmp_ehs.end());
			}
		}
		
		foreach (const auto &eh, ehs_on_comm_shrink_set) {
			if (contains(int_ehs_on_int_fhs, eh)) {
				int_ehs.insert(eh);
				int_ehs_on_int_fhs.erase(eh);
			}
		}
	};


	//--------------------------------------------------------------------
	std::unordered_set<OvmVeH> vhs_on_int_ehs_on_int_fhs_for_dual_sheet_1; //用作第一类生成sheet的点集
	// 得到用于生成第一类sheet的点集
	// 也就是这个点集是用于为了能够合理地生成quad sets相交面边界处的网格
	auto fGetVhsOnIntEhsOnIntFhsForDualSheet1 = [&]() {
		std::unordered_set<OvmVeH> vhs_on_int_ehs;
		foreach (const auto &eh, int_ehs) {
			auto vh0 = mesh->edge(eh).from_vertex();
			auto vh1 = mesh->edge(eh).to_vertex();
			vhs_on_int_ehs.insert(vh0);
			vhs_on_int_ehs.insert(vh1);
		}

		foreach (const auto &eh, int_ehs_on_int_fhs) {
			auto vh0 = mesh->edge(eh).from_vertex();
			auto vh1 = mesh->edge(eh).to_vertex();
			if (!contains(vhs_on_int_ehs, vh0)) {
				vhs_on_int_ehs_on_int_fhs_for_dual_sheet_1.insert(vh0);
			}
			if (!contains(vhs_on_int_ehs, vh1)) {
				vhs_on_int_ehs_on_int_fhs_for_dual_sheet_1.insert(vh1);
			}
		}
	};


	//--------------------------------------------------------------------
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

	//--------------------------------------------------------------------
	// 对网格模型中的每个点存一下V_PREV_HANDLE，即这个点在sheet inflation前的vertex handle值
	// 网格模型中的每个点都有一个归属关系，我们把这个归属关系用V_ENTITY_PTR来表示
	assert(mesh->vertex_property_exists<unsigned long>("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
	auto V_PREV_HANDLE = mesh->request_vertex_property<OvmVeH>("prevhandle", mesh->InvalidVertexHandle);
	std::unordered_set<OvmVeH> all_vhs_on_fhs; // quad sets上的点
	// 为分裂点做准备
	// 获得fhs上的所有顶点
	auto fPreSplitVhs = [&]() {
		// 初始化V_PREV_HANDLE
		for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
			V_PREV_HANDLE[*v_it] = *v_it;

		foreach(auto &fh, inflation_quad_set){
			auto hehs = mesh->face(fh).halfedges();
			foreach(auto &heh, hehs)
				all_vhs_on_fhs.insert(mesh->halfedge(heh).to_vertex());
		}
	};

	// 这个函数的作用是得到两个cell_group的相交面的个数
	auto fGetCommonFaces = [&](VolumeMesh *mesh, std::unordered_set<OvmCeH> cells_group1, 
		std::unordered_set<OvmCeH> cells_group2)->std::unordered_set<OvmFaH> {

			std::unordered_set<OvmFaH> fhs_group1, fhs_group2;
			foreach (const auto &ch, cells_group1) {
				auto hfhs = mesh->cell(ch).halffaces();
				foreach (const auto &hfh, hfhs) {
					fhs_group1.insert(mesh->face_handle(hfh));
				}
			}

			foreach (const auto &ch, cells_group2) {
				auto hfhs = mesh->cell(ch).halffaces();
				foreach (const auto &hfh, hfhs) {
					fhs_group2.insert(mesh->face_handle(hfh));
				}
			}

			return intersection(fhs_group1, fhs_group2);
	};

	//--------------------------------------------------------------------
	// 当quad set在边界边上时，分类点的个数有可能出错
	// 该函数判别分裂点的个数是否出错
	// 出错的情况：
	// quad set在边界边上，导致本来应该分裂成两个点，各在quad set的两边
	// 分裂出的结果为分裂出了三个点
	auto fSplitVertexCorrtectly = [&](VolumeMesh *mesh, std::vector<std::unordered_set<OvmCeH> > &cell_groups)->bool {
		if (cell_groups.size() != 3) return true;

		foreach (const auto &cell_group1, cell_groups) {
			int touch_cell_num = 0;
			foreach (const auto &cell_group2, cell_groups) {
				if (cell_group1 == cell_group2) continue;
				if (fGetCommonFaces(mesh, cell_group1, cell_group2).size() > 0) {
					touch_cell_num++;
				}
			}
			if (touch_cell_num < 2) return false;
		}

		return true;
	};

	//--------------------------------------------------------------------
	// 这个函数的作用是判断两个cell是否挨着，贴着一个面
	auto fIsTwoCellsAdjToFace = [&](VolumeMesh *mesh, OvmCeH ch1, OvmCeH ch2)->bool {
		auto hfhs1 = mesh->cell(ch1).halffaces();
		auto hfhs2 = mesh->cell(ch2).halffaces();
		std::unordered_set<OvmFaH> fhs1, fhs2;

		foreach (const auto &hfh, hfhs1) {
			fhs1.insert(mesh->face_handle(hfh));
		}
		foreach (const auto &hfh, hfhs2) {
			fhs2.insert(mesh->face_handle(hfh));
		}

		return (intersection(fhs1, fhs2)).size() > 0;
	};


	//--------------------------------------------------------------------
	// 对quad sets的每一个点进行分裂
	// 正常情况下，一个点只在一个quad set上，这个点分裂出两个新点
	// 十字相交的情况下，一个点同时在两个quad set上， 这个点分裂出4个新点
	//stores all the cells adjacent to faces in fhs
	std::unordered_set<OvmCeH> all_adj_cells;
	std::unordered_set<OvmVeH> new_vertices;
	auto fSplitVhs = [&]() {
		foreach(auto &vh, all_vhs_on_fhs) {
			std::vector<std::unordered_set<OvmCeH> > cell_groups1;

			std::vector<std::unordered_set<OvmCeH> > cell_groups;
			std::map<OvmVeH, std::unordered_set<OvmCeH> > newly_vertices_distribution;

			get_cell_groups_around_vertex(mesh, vh, inflation_quad_set, cell_groups1);

			if (!fSplitVertexCorrtectly(mesh, cell_groups1) && mesh->is_boundary(vh)) { 
				// 当quad set上的点在边界边上时，原来的算法会找到三个cell groups
				// 需要把两个cell groups 合并成一个

				std::unordered_set<OvmCeH> right_cell_group;
				int right_pos = -1;
				for (int i = 0; i < cell_groups1.size(); i++) {
					int touch_num = 0; // 这个cell group和其他多少个cell group相连
					for (int j = 0; j < cell_groups1.size(); j++) {
						if (i == j) continue;
						bool touch = false; // 判断这两个cell group是否有面贴合
						foreach (const auto &ch1, cell_groups1[i]) {
							foreach (const auto &ch2, cell_groups1[j]) {
								if (fIsTwoCellsAdjToFace(mesh, ch1, ch2)) {
									touch = true;
									break;
								}
							}
							if (touch) break;
						}
						if (touch) touch_num++;
					}
					if (touch_num == cell_groups1.size() - 1) {
						right_pos = i;
						break;
					}
				}
				right_cell_group = cell_groups1[right_pos];
				cell_groups.push_back(right_cell_group);
				std::unordered_set<OvmCeH> merged_cell_group;
				for (int i = 0; i < cell_groups1.size(); i++) {
					if (i == right_pos) continue;
					foreach (const auto &ch, cell_groups1[i]) {
						merged_cell_group.insert(ch);
					}
				}
				cell_groups.push_back(merged_cell_group);
			}
			else {
				cell_groups = cell_groups1;
			}

			//如果fhs部分区域贴着网格表面时，cell_groups只包含一个集合，则此时需要再补上一个空集合
			if (cell_groups.size() == 1){
				std::unordered_set<OvmCeH> tmp;
				tmp.insert(mesh->InvalidCellHandle);
				cell_groups.push_back(tmp);
			}
			////////////////////////////////////////////////////////////
			// DD:
			// 在inflation_quad_set下不一定是规则的一个sheet，所以要判别处理一下:
			// 如果在inflation_quad_set上面是有个cell,而下面没有cell，则要加一个
			// InvalidCellHandle来保持对称
			////////////////////////////////////////////////////////////
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

				/*OvmVeH new_vertex = OvmVeH(-1);
				OvmVec3d reasonable_pos(0, 0, 0);
				reasonable_pos = mesh->vertex(vh);
				new_vertex = mesh->add_vertex(reasonable_pos);*/
				
				
				// Test code
				auto oppo_vh = get_opposite_vh_on_cell_group(mesh, vh, one_chs_group, inflation_quad_set);

				OvmVeH new_vertex = OvmVeH(-1);
				OvmVec3d tmp_pos(0, 0, 0);
				OvmVec3d origin_pos = mesh->vertex(vh);
				tmp_pos = mesh->vertex(oppo_vh);
				OvmVec3d new_pos = (origin_pos + tmp_pos) / 2;

				new_vertex = mesh->add_vertex(new_pos);
				

				/*
				if (V_ENTITY_PTR[vh] == 0){ //代表内部点
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
				*/

				V_ENTITY_PTR[new_vertex] = get_new_V_ENTITY_PTR_from_cell_group(mesh, vh, one_chs_group, cell_groups, inflation_quad_set, shrink_set, 
																				int_ehs, int_fhs);
				
				//-------------------debug------------------//
				if (vh.idx() == 814) {
					//fout << "----------------------" << std::endl;
					//fout << "Origin vh id: " << vh.idx() << std::endl;
					//fout << "old V_ENTITY_PTR: " << V_ENTITY_PTR[vh] << std::endl;
					//fout << "new V_ENTITY_PTR: " << V_ENTITY_PTR[new_vertex] << std::endl;
			
					//if (is_FACE((ENTITY*)V_ENTITY_PTR[new_vertex])) {
					//	fout << "is_FACE" << std::endl;
					//}
					//else if (is_EDGE((ENTITY*)V_ENTITY_PTR[new_vertex])) {
					//	fout << "is_EDGE" << std::endl;
					//}
					//else if (is_VERTEX((ENTITY*)V_ENTITY_PTR[new_vertex])) {
					//	fout << "is_VERTEX" << std::endl;
					//}

					//fout << "Relate cell group:" << std::endl;
					//foreach (auto &ch, one_chs_group) {
					//	fout << ch.idx() << std::endl;
					//}

					//fout << "----------------------" << std::endl;

					//auto fun = (ENTITY*)V_ENTITY_PTR[new_vertex];
					/*
					if (is_FACE((ENTITY*)V_ENTITY_PTR[new_vertex])) {
						qDebug() << "is_FACE";
					}
					else if (is_EDGE((ENTITY*)V_ENTITY_PTR[new_vertex])) {
						qDebug() << "is_EDGE";
					}
					else if (is_VERTEX((ENTITY*)V_ENTITY_PTR[new_vertex])) {
						qDebug() << "is_VERTEX";
					}
					*/

					//fout << "Relate cell group:" << std::endl;
					//foreach (auto &ch, one_chs_group) {
					//	fout << ch.idx() << std::endl;
					//}
				}
				//-------------------debug------------------//

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
	};


	//--------------------------------------------------------------------------
	std::hash_map<OvmVeH, PointInfo> vh_info_for_readd;
	// 把sheet inflation前的模型的点的几何信息和点的归属关系保存下来
	auto fSavePointInfo = [&]() {
		foreach(auto &ch, all_adj_cells){
			for (auto v_it = mesh->cv_iter(ch); v_it; ++v_it){
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

		//////////////////////////////////////////////////
		// 这里也许应该把原来的点(在交面上的点)也加入vh_info_for_readd
		std::unordered_set<OvmVeH> vhs_on_int_fhs;
		foreach (const auto &fh, int_fhs) {
			auto vhs_on_fh = get_adj_vertices_around_face(mesh, fh);
			foreach (const auto &vh, vhs_on_fh) {
				vhs_on_int_fhs.insert(vh);
			}
		}

		foreach (const auto &vh, vhs_on_int_fhs) {
			PointInfo point_info;
			point_info.pos = mesh->vertex(vh);
			point_info.entity_ptr = V_ENTITY_PTR[vh];
			vh_info_for_readd.insert(std::make_pair(vh, point_info));
		}

	};


	//--------------------------------------------------------------------------
	std::hash_map<OvmCeH, std::vector<OvmVeH> > cells_rebuilding_recipes; //(cell, 对应的新的8个点)
	//搜集那些需要重建的六面体的八个顶点
	auto fCollectVhsOfRebuildHex = [&]() {
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
		}//end foreach (auto &ch, all_adj_cells) {...
	};


	//--------------------------------------------------------------------------	
	std::vector<std::vector<OvmVeH> > ord_newly_created_cells_recipes;
	// 搜集那些普通区域（即不是在交叉区域）上的新的六面体的八个顶点
	auto fCollectVhsOfNewHexGeneratedBySingleQuadset = [&]() {
		std::unordered_set<OvmFaH> single_face_quad_set; // 没有交面的inflation quad set
		foreach (const auto &fh, inflation_quad_set) {
			if (!contains(int_fhs, fh)) single_face_quad_set.insert(fh);
		}

		std::unordered_set<OvmVeH> vhs_on_int_fhs;
		foreach (const auto &fh, int_fhs) {
			auto vhs = get_adj_vertices_around_face(mesh, fh);
			foreach (const auto &vh, vhs) {
				vhs_on_int_fhs.insert(vh);
			}
		}

		foreach(auto &fh, single_face_quad_set) {
			auto hfh1 = mesh->halfface_handle(fh, 0), hfh2 = mesh->halfface_handle(fh, 1);
			auto ch1 = mesh->incident_cell(hfh1), ch2 = mesh->incident_cell(hfh2);
			std::vector<OvmVeH> ch_vhs;

			for (auto hfv_it = mesh->hfv_iter(hfh1); hfv_it; ++hfv_it){
				auto newly_vh = fGetNewVeHOnCeH(*hfv_it, ch1);
				assert(newly_vh != mesh->InvalidVertexHandle);

				// 见开发日志的注释中情况1，为了解决这种情况
				if (contains(vhs_on_int_fhs, *hfv_it) && ch1 == mesh->InvalidCellHandle) {
					newly_vh = *hfv_it;
				}
				ch_vhs.push_back(newly_vh);
			}

			//qSwap(ch_vhs[1], ch_vhs[3]);
			for (auto hfv_it = mesh->hfv_iter(hfh2); hfv_it; ++hfv_it){
				auto newly_vh = fGetNewVeHOnCeH(*hfv_it, ch2);
				assert(newly_vh != mesh->InvalidVertexHandle);

				// 见开发日志的注释中的情况1，为了解决这种情况
				if (contains(vhs_on_int_fhs, *hfv_it) && ch2 == mesh->InvalidCellHandle) {
					newly_vh = *hfv_it;
				}
				ch_vhs.push_back(newly_vh);
			}
			//qSwap(ch_vhs[5], ch_vhs[7]);
			assert(ch_vhs.size() == 8);
			ord_newly_created_cells_recipes.push_back(ch_vhs);
		}
	};


	//--------------------------------------------------------------------------
	std::vector<std::vector<OvmVeH> > int_newly_created_cells_recipes;
	std::map<OvmVeH, OvmVeH> vhs_on_int_fhs_map_to_splited_vhs;
	// 搜集交叉区域新的六面体的八个顶点
	auto fCollectVhsOfNewHexGeneratedByCrossQuadsets = [&]() {
		foreach(auto &eh, int_ehs) {
			auto heh = mesh->halfedge_handle(eh, 0);
			std::vector<OvmVeH> ch_vhs_up, ch_vhs_down;
			auto vh_up_origin = mesh->halfedge(heh).from_vertex(),
				vh_down_origin = mesh->halfedge(heh).to_vertex();

			// 找出一个旧点分裂出的四个新点
			auto fGetSplitedVhs = [&](OvmHaEgH current_heh, OvmVeH current_vh, std::vector<OvmVeH> &ch_vhs) {
				ch_vhs.clear();
				auto locate = newly_created_vertices_cells_mapping->find(current_vh);
				int num_vhs = (locate->second).size();

				// 当这个点位于传统的四字交叉交线边缘和quad sets相交面边界边缘时要处理一下
				// 找到那个不能通过临接半边的半面来找到的分裂出来的点
				std::unordered_set<OvmVeH> splited_vhs;
				for (auto hehf_it = mesh->hehf_iter(current_heh); hehf_it; ++hehf_it) {
					auto fh = mesh->face_handle(*hehf_it);
					if (contains(inflation_quad_set, fh)) {
						auto test_ch = mesh->incident_cell(*hehf_it);
						auto newly_vh = fGetNewVeHOnCeH(current_vh, test_ch);
						assert(newly_vh != mesh->InvalidVertexHandle);
						splited_vhs.insert(newly_vh);
					}
				}
				OvmVeH missed_vh = mesh->InvalidVertexHandle;
				if (splited_vhs.size() == 3 && num_vhs == 4) {
					foreach (auto &p, locate->second) {
						if (!contains(splited_vhs, p.first)) {
							missed_vh = p.first;
							vhs_on_int_fhs_map_to_splited_vhs[current_vh] = missed_vh;
							break;
						}
					}
				}

				//use the intersecting halfedges to find the adjacent cell groups,
				//one cell group indicates one newly created vertex
				for (auto hehf_it = mesh->hehf_iter(current_heh); hehf_it; ++hehf_it){
					auto fh = mesh->face_handle(*hehf_it);
					//if fh is in fhs, one cell group is found
					if (inflation_quad_set.find(fh) != inflation_quad_set.end()) {
						//---------------------------------------
						if (num_vhs == 3 && contains(int_fhs, fh)) {
							ch_vhs.push_back(current_vh);
						}
						if (num_vhs == 4 && splited_vhs.size() == 3 && contains(int_fhs, fh)) {
							ch_vhs.push_back(missed_vh);
						}
						//---------------------------------------
						auto test_ch = mesh->incident_cell(*hehf_it);
						auto newly_vh = fGetNewVeHOnCeH(current_vh, test_ch);
						assert(newly_vh != mesh->InvalidVertexHandle);
						ch_vhs.push_back(newly_vh);
					}
				}

				//if (num_vhs == 4 && ch_vhs.size() == 3) {
				//	qDebug() << "---------------------------------";
				//	qDebug() << "num_vhs: " << num_vhs;
				//	qDebug() << "Size of ch_vhs: " << ch_vhs.size();
				//	qDebug() << "The current vh id: " << current_vh.idx();
				//	qDebug() << "The current eh id: " << (mesh->edge_handle(current_heh)).idx();

				//	qDebug() << "**********************************";
				//	qDebug() << "Information about current vh related cells";
				//	auto locate = newly_created_vertices_cells_mapping->find(current_vh);
				//	auto &map = locate->second;
				//	int num_iter = 0;
				//	foreach (const auto &iter, map) {
				//		qDebug() << "num_iter: " << ++num_iter;
				//		qDebug() << "Relate cell handle id:";
				//		foreach (const auto &ch, iter.second) {
				//			qDebug() << "ch id: " << ch.idx();
				//		}
				//		qDebug() << "+";
				//	}

				//	qDebug() << "---------------------------------";
				//}
			};

			fGetSplitedVhs(heh, vh_up_origin, ch_vhs_up);
			fGetSplitedVhs(heh, vh_down_origin, ch_vhs_down);

			assert(ch_vhs_up.size() == 4 && ch_vhs_down.size() == 4);
			qSwap(ch_vhs_up[1], ch_vhs_up[3]);
			ch_vhs_up.insert(ch_vhs_up.end(), ch_vhs_down.begin(), ch_vhs_down.end());
			int_newly_created_cells_recipes.push_back(ch_vhs_up);
		}
	};

	//--------------------------------------------------------------------------
	std::vector<std::vector<OvmVeH> > int_face_newly_created_cells_recipes;
	// 搜集交面两个方向新生成的六面体的8个顶点
	auto fCollectVhsOfNewHexGeneratedByDualQuadsetPart = [&]() {
		foreach (const auto &fh, int_fhs) {
			auto hfh1 = mesh->halfface_handle(fh, 0);
			auto ch1 = mesh->incident_cell(hfh1);
			auto hfh2 = mesh->halfface_handle(fh, 1);
			auto ch2 = mesh->incident_cell(hfh2);

			// 搜集一边的六面体的8个点
			std::vector<OvmVeH> ch_vhs_up1, ch_vhs_down1;
			for (auto hfv_it = mesh->hfv_iter(hfh1); hfv_it; hfv_it++) {
				//if (contains(vhs_on_int_ehs_on_int_fhs_for_simple, *hfv_it)) {
				if (contains(vhs_on_int_ehs_on_int_fhs_for_dual_sheet_1, *hfv_it)) {
					auto top_vh = get_top_vertex(mesh, *hfv_it, int_fhs, newly_created_vertices_cells_mapping);
					assert(top_vh != mesh->InvalidVertexHandle);
					ch_vhs_down1.push_back(top_vh);
				}
				else if (vhs_on_int_fhs_map_to_splited_vhs.find(*hfv_it) !=vhs_on_int_fhs_map_to_splited_vhs.end()) {
					ch_vhs_down1.push_back(vhs_on_int_fhs_map_to_splited_vhs[*hfv_it]);
				}
				else {
					ch_vhs_down1.push_back(*hfv_it);
				}
				auto new_vh = fGetNewVeHOnCeH(*hfv_it, ch1);
				assert(new_vh != mesh->InvalidVertexHandle);
				ch_vhs_up1.push_back(new_vh);
			}
			assert(ch_vhs_up1.size() == 4 && ch_vhs_down1.size() == 4);
			qSwap(ch_vhs_down1[1], ch_vhs_down1[3]);
			ch_vhs_up1.insert(ch_vhs_up1.end(), ch_vhs_down1.begin(), ch_vhs_down1.end());
			int_face_newly_created_cells_recipes.push_back(ch_vhs_up1);

			// 收集另一边的六面体的8个点
			std::vector<OvmVeH> ch_vhs_up2, ch_vhs_down2;
			for (auto hfv_it = mesh->hfv_iter(hfh2); hfv_it; hfv_it++) {
				//if (contains(vhs_on_int_ehs_on_int_fhs_for_simple, *hfv_it)) {
				if (contains(vhs_on_int_ehs_on_int_fhs_for_dual_sheet_1, *hfv_it)) {
					auto top_vh = get_top_vertex(mesh, *hfv_it, int_fhs, newly_created_vertices_cells_mapping);
					assert(top_vh != mesh->InvalidVertexHandle);
					ch_vhs_down2.push_back(top_vh);
				}
				else if (vhs_on_int_fhs_map_to_splited_vhs.find(*hfv_it) != vhs_on_int_fhs_map_to_splited_vhs.end()) {
					ch_vhs_down2.push_back(vhs_on_int_fhs_map_to_splited_vhs[*hfv_it]);
				}
				else {
					ch_vhs_down2.push_back(*hfv_it);
				}
				auto new_vh = fGetNewVeHOnCeH(*hfv_it, ch2);
				assert(new_vh != mesh->InvalidVertexHandle);
				ch_vhs_up2.push_back(new_vh);
			}
			assert(ch_vhs_up2.size() == 4 && ch_vhs_down2.size() == 4);
			//qSwap(ch_vhs_up2[1], ch_vhs_up2[3]);
			qSwap(ch_vhs_down2[1], ch_vhs_down2[3]);
			ch_vhs_up2.insert(ch_vhs_up2.end(), ch_vhs_down2.begin(), ch_vhs_down2.end());
			int_face_newly_created_cells_recipes.push_back(ch_vhs_up2);
		}
	};


	//--------------------------------------------------------------------------
	OpenVolumeMesh::StatusAttrib status_attrib(*mesh);
	//首先删除旧的需要删除的顶点，连带删除了与此相邻的六面体
	auto fDestoryRelatedCells = [&]() {
		foreach(auto &ch, all_adj_cells)
			status_attrib[ch].set_deleted(true);

		status_attrib.garbage_collection(true);
	};


	//--------------------------------------------------------------------------
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


	//--------------------------------------------------------------------------
	std::unordered_set<OvmCeH> new_chs;
	// 构建新生成的六面体
	auto fBuildNewHex = [&]() {
		//old_new_vhs_mapping存储旧的点（经过前面分裂后的点，并不是original的点）和新的点之间的对应关系
		//由于是分裂后的点，因此这个对应关系是一对一映射
		//std::hash_map<OvmVeH, OvmVeH> old_new_vhs_mapping;
		for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it){
			auto new_vh = *v_it;
			auto ori_vh = V_PREV_HANDLE[new_vh];
			if (ori_vh != mesh->InvalidVertexHandle)
				old_new_vhs_mapping.insert(std::make_pair(ori_vh, new_vh));
		}

		// Build RebuildHex
		for (auto it = cells_rebuilding_recipes.begin(); it != cells_rebuilding_recipes.end(); ++it){
			fUpdateVhs(it->second);
			auto ch = mesh->add_cell(it->second);
			assert(ch != mesh->InvalidCellHandle);
		}
		//qDebug() << "Finish Build RebuildHex...";

		// Build new hex generated by single quad set
		for (int i = 0; i != ord_newly_created_cells_recipes.size(); ++i){
			auto &vhs = ord_newly_created_cells_recipes[i];
			fUpdateVhs(vhs);
			auto ch = mesh->add_cell(vhs);
			assert(ch != mesh->InvalidCellHandle);
			new_chs.insert(ch);
		}
		//qDebug() << "Finish Build new hex generated by single quad set...";

		// Build new hex generated by cross quad sets
		for (int i = 0; i != int_newly_created_cells_recipes.size(); ++i){
			auto &vhs = int_newly_created_cells_recipes[i];
			fUpdateVhs(vhs);
			auto ch = mesh->add_cell(vhs);
			assert(ch != mesh->InvalidCellHandle);
			new_chs.insert(ch);
		}
		//qDebug() << "Finish Build new hex generated by cross quad sets...";

		// Build new hex generated by dual quad sets part
		for (int i = 0; i != int_face_newly_created_cells_recipes.size(); i++) { 
			auto &vhs = int_face_newly_created_cells_recipes[i];
			fUpdateVhs(vhs);
			auto ch = mesh->add_cell(vhs);
			assert(ch != mesh->InvalidCellHandle); 
			new_chs.insert(ch);
		}
		//qDebug() << "Finish Build new hex generated by dual quad sets part...";
	};


	//--------------------------------------------------------------------------
	std::vector<DualSheet *> new_sheets;
	// 得到sheet inflation生成的new sheets
	auto fGetNewSheets = [&]() {
		auto fIsNewSheet = [&](std::unordered_set<OvmCeH> &chs)->bool{
			foreach(auto &ch, chs){
				if (!contains(new_chs, ch)) return false;
			}
			return true;
		};

		auto tmp_chs = new_chs;
		while (!tmp_chs.empty()){
			DualSheet *new_sheet = new DualSheet(mesh);

			std::unordered_set<OvmEgH> all_ehs_of_first_ch;
			OvmCeH first_ch = *(tmp_chs.begin());
			auto hfh_vec = mesh->cell(first_ch).halffaces();
			foreach(auto &hfh, hfh_vec){
				auto heh_vec = mesh->halfface(hfh).halfedges();
				foreach(auto &heh, heh_vec)
					all_ehs_of_first_ch.insert(mesh->edge_handle(heh));
			}

			// 考虑all_eh_of_first_ch的两个sheet 方向
			foreach (const auto &eh, all_ehs_of_first_ch) {
				std::unordered_set<OvmEgH> sheet_ehs;
				std::unordered_set<OvmCeH> sheet_chs;
				retrieve_one_sheet(mesh, eh, sheet_ehs, sheet_chs);

				if (fIsNewSheet(sheet_chs)) {
					new_sheet->ehs = sheet_ehs;
					new_sheet->chs = sheet_chs;
					break;
				}
			}

			foreach(auto &ch, new_sheet->chs)
				tmp_chs.erase(ch);
			new_sheets.push_back(new_sheet);
		}
	};


	//--------------------------------------------------------------------------
	qDebug() <<"Begin the function...";
	fDealQuadsetAndShrinkset();
	qDebug() << "Finish fDealQuadsetAndShrinkset...";

	fGetIntEhs();
	qDebug() << "Finish fGetIntEhs...";
	// For debug
	//std::vector<DualSheet *> debug_new_sheets;
	//return new_sheets;
	// For debug

	//qDebug() << "Size of int_ehs: " << int_ehs.size();
	//qDebug() << "Size of int_fhs: " << int_fhs.size();
	//qDebug() << "Size of int_ehs_on_int_fhs: " << int_ehs_on_int_fhs.size();
	fGetVhsOnIntEhsOnIntFhsForDualSheet1();
	qDebug() << "Finish fGetVhsOnIntEhsOnIntFhsForDualSheet1...";

	//qDebug() << "Size of vhs_on_int_ehs_on_int_fhs_for_dual_sheet_1: " << vhs_on_int_ehs_on_int_fhs_for_dual_sheet_1.size();

	fPreSplitVhs();
	qDebug() << "Finish fPreSplitVhs";
	//qDebug() << "Size of all_vhs_on_fhs: " << all_vhs_on_fhs.size();
	fSplitVhs();
	qDebug() << "Finish fSplitVhs";
	//qDebug() << "Size of all_adj_cells: " << all_adj_cells.size();
	//qDebug() << "Size of new_vertices: " << new_vertices.size();

	fSavePointInfo();
	qDebug() << "Finish fSavePointInfo";
	fCollectVhsOfRebuildHex();
	qDebug() << "Finish fCollectVhsOfRebuildHex";
	fCollectVhsOfNewHexGeneratedBySingleQuadset();
	qDebug() << "Finish fCollectVhsOfNewHexGeneratedBySingleQuadset";
	fCollectVhsOfNewHexGeneratedByCrossQuadsets();
	qDebug() << "Finish fCollectVhsOfNewHexGeneratedByCrossQuadsets";
	fCollectVhsOfNewHexGeneratedByDualQuadsetPart();
	qDebug() << "Finish fCollectVhsOfNewHexGeneratedByDualQuadsetPart";

	//qDebug() << "Num of vertices in mesh before garbage collection: " << mesh->n_vertices();
	fDestoryRelatedCells();
	qDebug() << "Finish fDestoryRelatedCells";
	//qDebug() << "Num of vertices in mesh After garbage collection: " << mesh->n_vertices();

	fBuildNewHex();
	qDebug() << "Finish fBuildNewHex";

	fGetNewSheets();
	qDebug("Finish getting new sheets...");

	//--------------------------------------------------------------------------

	delete newly_created_vertices_cells_mapping;

	return new_sheets;
}


std::vector<DualSheet *> X_sheet_inflation(VolumeMesh *mesh, std::vector<std::unordered_set<OvmFaH> > &quad_sets,
										   std::vector<std::unordered_set<OvmCeH> > &shrink_sets)
{
	shrink_sets.clear();
	bool ok = get_shrink_sets_from_quad_sets(mesh, quad_sets, shrink_sets);

	if (!ok) {
		throw std::logic_error("Can't get the proper shrink sets.");
	} 

	return DM_sheet_inflation(mesh, quad_sets, shrink_sets);
}

// Improve to self-cross quad set
// inflation_quad_set只表示一个面
// direction: [0, 1]
// 代表面的两个方向
/*
static bool X_get_shrink_set_on_direction(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmCeH> &shrink_set, int direction)
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
		if (cell == mesh->InvalidCellHandle) return false; 

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

	return true;
}
*/

/*
 * 可以得到自交quad set的shrink set
 */
bool X_get_shrink_set_on_direction(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmCeH> &shrink_set, int direction)
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

			while (mesh->face_handle(tmp_hfhs.front()) != mesh->face_handle(hfh)) {
				auto tmp = tmp_hfhs.front();
				tmp_hfhs.pop();
				tmp_hfhs.push(tmp);
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
		}
	}

	//qDebug() << "End X_get_shrink_set_on_direction";

	return true;
}

/*
 * 可以得到自交quad set的shrink set
 * @ hfh 相邻的cell是在目标shrink set中的
 */
bool X_get_shrink_set_on_direction(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmCeH> &shrink_set, OvmHaFaH hfh)
{
	shrink_set.clear();
	std::queue<OvmHaFaH> queue_hfhs;
	std::unordered_set<OvmFaH> used_fhs;

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

			while (mesh->face_handle(tmp_hfhs.front()) != mesh->face_handle(hfh)) {
				auto tmp = tmp_hfhs.front();
				tmp_hfhs.pop();
				tmp_hfhs.push(tmp);
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
		}
	}

	//qDebug() << "End X_get_shrink_set_on_direction";

	return true;
}

// inflation_quad_set只考虑一个面
// 不考虑面自交的情况
bool X_get_shrink_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmCeH> &shrink_set)
{
	bool ok = X_get_shrink_set_on_direction(mesh, inflation_quad_set, shrink_set, 0);
	if (ok) return true;
	ok = X_get_shrink_set_on_direction(mesh, inflation_quad_set, shrink_set, 1);
	return ok;
}


/*
 * 只考虑了简单的情况：
 * 至多有两个quad set有相交面
*/
bool get_shrink_sets_from_quad_sets(VolumeMesh *mesh, std::vector<std::unordered_set<OvmFaH> > &quad_sets, 
								   std::vector<std::unordered_set<OvmCeH> > &shrink_sets)
{
	std::map<int, int> quad_set_shrink_set_direction_map;
	std::unordered_set<int> single_quad_sets_index;
	//std::vector<std::pair<int, int> > dual_face_quad_set_index;
	std::vector<std::vector<int> > dual_face_quad_set_index;
	std::unordered_set<OvmCeH> shrink_set;

	shrink_sets.clear();
	for (int i = 0; i < quad_sets.size(); ++i) {
		shrink_sets.push_back(shrink_set);
	}

	std::unordered_set<int> used_indexes;
	for (int i = 0; i < quad_sets.size(); ++i) {
		if (contains(used_indexes, i)) continue;
		bool flag = false;
		for (int j = i+1; j < quad_sets.size(); ++j) {
			std::unordered_set<OvmFaH> inter_fhs = intersection(quad_sets[i], quad_sets[j]);
			if (intersects(quad_sets[i], quad_sets[j])) {
				used_indexes.insert(i);
				used_indexes.insert(j);

				std::vector<int> indexes_vec;
				indexes_vec.push_back(i);
				indexes_vec.push_back(j);
				indexes_vec.push_back(int(*(inter_fhs.begin())));
				dual_face_quad_set_index.push_back(indexes_vec);
				flag = true;
				break;
			}
		}
		if (!flag) {
			single_quad_sets_index.insert(i);
		}
	}

	qDebug() << "The size of dual_face_quad_set: " << dual_face_quad_set_index.size();

	// Deal with quad sets with dual face
	foreach (const auto &indexes, dual_face_quad_set_index) {
		qDebug() << "There are dual faces...";
		
		int direction = 0;
		bool ok_first = X_get_shrink_set_on_direction(mesh, quad_sets[indexes[0]], shrink_sets[indexes[0]], mesh->halfface_handle(indexes[2], direction));
		bool ok_second = X_get_shrink_set_on_direction(mesh, quad_sets[indexes[1]], shrink_sets[indexes[1]], mesh->halfface_handle(indexes[2], 1-direction));

		if (ok_first && ok_second) {
			continue;
		}

		direction = 1 - direction;
		ok_first = X_get_shrink_set_on_direction(mesh, quad_sets[indexes[0]], shrink_sets[indexes[0]], mesh->halfface_handle(indexes[2], direction));
		ok_second = X_get_shrink_set_on_direction(mesh, quad_sets[indexes[1]], shrink_sets[indexes[1]], mesh->halfface_handle(indexes[2], 1-direction));

		if (ok_first && ok_second) {
			continue;
		}
		else {
			return false;
		}
	}

	// Deal with separated quad set
	foreach (const auto &i, single_quad_sets_index) {
		qDebug() << "There are separated quad set...";
		int direction = 0;
		bool ok = X_get_shrink_set_on_direction(mesh, quad_sets[i], shrink_sets[i], direction);
		if (!ok) {
			direction = 1 - direction;
			ok = X_get_shrink_set_on_direction(mesh, quad_sets[i], shrink_sets[i], direction);
			if (!ok) return false;
		}
	}
	qDebug() << "Finish get the separated quad set.";

	//for (int i = 0; i < quad_sets.size(); ++i) {
	//	int direction = quad_set_shrink_set_direction_map[i];
	//	std::unordered_set<OvmCeH> tmp_shrink_set;
	//	X_get_shrink_set_on_direction(mesh, quad_sets[i], tmp_shrink_set, direction);
	//	shrink_sets.push_back(tmp_shrink_set);
	//}

	// Complete shrink set
	for (int i = 0; i < quad_sets.size(); ++i) {
		complete_shrink_set(mesh, quad_sets[i], shrink_sets[i]);
	}

	return true;

}


void complete_shrink_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set)
{
	foreach (const auto &fh, quad_set) {
		// 判断fh在边界上
		if (mesh->is_boundary(fh)) continue;
		auto hehs_vec = mesh->face(fh).halfedges();
		foreach (const auto &heh, hehs_vec) {
			//if (mesh->is_boundary(heh)) {
			//	auto vh = mesh->halfedge(heh).from_vertex();
			//	std::vector<std::unordered_set<OvmCeH> > cell_groups;
			//	get_cell_groups_around_vertex(mesh, vh, quad_set, cell_groups);
			//	foreach (const auto &cell_group, cell_groups) {
			//		if (intersects(cell_group, shrink_set)) {
			//			shrink_set.insert(cell_group.begin(), cell_group.end());
			//			break;
			//		}
			//	}
			//	//----------------------------------------
			//	auto vh1 = mesh->halfedge(heh).to_vertex();
			//	std::vector<std::unordered_set<OvmCeH> > cell_groups1;
			//	get_cell_groups_around_vertex(mesh, vh1, quad_set, cell_groups1);
			//	foreach (const auto &cell_group, cell_groups1) {
			//		if (intersects(cell_group, shrink_set)) {
			//			shrink_set.insert(cell_group.begin(), cell_group.end());
			//			break;
			//		}
			//	}
			//}
			auto vh = mesh->halfedge(heh).from_vertex();
			std::vector<std::unordered_set<OvmCeH> > cell_groups;
			get_cell_groups_around_vertex(mesh, vh, quad_set, cell_groups);
			foreach (const auto &cell_group, cell_groups) {
				if (intersects(cell_group, shrink_set)) {
					shrink_set.insert(cell_group.begin(), cell_group.end());
					break;
				}
			}
			//----------------------------------------
			auto vh1 = mesh->halfedge(heh).to_vertex();
			std::vector<std::unordered_set<OvmCeH> > cell_groups1;
			get_cell_groups_around_vertex(mesh, vh1, quad_set, cell_groups1);
			foreach (const auto &cell_group, cell_groups1) {
				if (intersects(cell_group, shrink_set)) {
					shrink_set.insert(cell_group.begin(), cell_group.end());
					break;
				}
			}
		}
	}
}

// Implement 1 at 2016.12.28
bool is_valid_inflation_rule1(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set)
{
	std::unordered_set<OvmVeH> vhs_on_border_quad_set;
	std::unordered_set<OvmVeH> vhs_on_shrink_set;

	// Get vhs_on_border_quad_set
	foreach (const auto &fh, quad_set) {
		std::vector<OvmVeH> adj_vhs = get_adj_vertices_around_face(mesh, fh);
		foreach (const auto &vh, adj_vhs) {
			if (mesh->is_boundary(vh)) {
				vhs_on_border_quad_set.insert(vh);
			}
		}
	}
	// Get vhs_on_shrink_set
	foreach (const auto &ch, shrink_set) {
		std::vector<OvmVeH> adj_vhs = get_adj_vertices_around_hexa(mesh, ch);
		vhs_on_shrink_set.insert(adj_vhs.begin(), adj_vhs.end());
	}

	auto fIsEdgeOnTwoVhs = [&](OvmVeH vh1, OvmVeH vh2)->bool {
		assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
		auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
		ENTITY *entity1 = (ENTITY*)(V_ENTITY_PTR[vh1]),
			*entity2 = (ENTITY*)(V_ENTITY_PTR[vh2]);

		if (is_EDGE(entity1)) {
			if (is_EDGE(entity2)) {
				if (entity1 == entity2) return true;
			}
			else if (is_VERTEX(entity2)) {
				ENTITY_LIST adj_edges_list;
				api_get_edges(entity2, adj_edges_list);
				if (adj_edges_list.lookup(entity1) != -1) return true;
			}
		}
		else if (is_VERTEX(entity1)) {
			ENTITY_LIST adj_edges_list1;
			api_get_edges(entity1, adj_edges_list1);
			if (is_EDGE(entity2)) {
				if (adj_edges_list1.lookup(entity2) != -1) return true;
			}
			else if (is_VERTEX(entity2)) {
				ENTITY_LIST adj_edges_list2;
				api_get_edges(entity2, adj_edges_list2);
				for (int i = 0; i != adj_edges_list2.count(); ++i) {
					if (adj_edges_list1.lookup(adj_edges_list2[i]) != -1) return true;
				}
			}
		}
		
		return false;
	};

	foreach (const auto &vh1, vhs_on_border_quad_set) {
		std::unordered_set<OvmVeH> adj_vhs;
		get_adj_vertices_around_vertex(mesh, vh1, adj_vhs);
		bool flag = false;
		foreach (const auto &vh2, adj_vhs) {
			if (mesh->is_boundary(vh2) && contains(vhs_on_shrink_set, vh2) && 
				!contains(vhs_on_border_quad_set, vh2)) {
				if (fIsEdgeOnTwoVhs(vh1, vh2)) {
					if (!flag) {                 // 第一个点和v1相连的网格边在几何边上
						flag = true;
					}
					else {                       // 第二个点和v1相连的网格边在几何边上
						return false;
					}
				}
			}
		}
	}

	return true;
}


//// Implement with get_associated_geometry_edge_of_boundary_eh() function
//// Last change: 2016.12.28
//bool is_valid_inflation_rule1(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set)
//{
//	std::unordered_set<OvmVeH> vhs_on_border_quad_set;
//	std::unordered_set<OvmVeH> vhs_on_shrink_set;
//
//	// Get vhs_on_border_quad_set
//	foreach (const auto &fh, quad_set) {
//		std::vector<OvmVeH> adj_vhs = get_adj_vertices_around_face(mesh, fh);
//		foreach (const auto &vh, adj_vhs) {
//			if (mesh->is_boundary(vh)) {
//				vhs_on_border_quad_set.insert(vh);
//			}
//		}
//	}
//	// Get vhs_on_shrink_set
//	foreach (const auto &ch, shrink_set) {
//		std::vector<OvmVeH> adj_vhs = get_adj_vertices_around_hexa(mesh, ch);
//		vhs_on_shrink_set.insert(adj_vhs.begin(), adj_vhs.end());
//	}
//
//	auto fIsEdgeOnTwoVhs = [&](OvmVeH vh1, OvmVeH vh2)->bool {
//		assert (mesh->vertex_property_exists<unsigned long> ("entityptr"));
//		auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
//		ENTITY *entity1 = (ENTITY*)(V_ENTITY_PTR[vh1]),
//			*entity2 = (ENTITY*)(V_ENTITY_PTR[vh2]);
//
//		if (is_EDGE(entity1)) {
//			if (is_EDGE(entity2)) {
//				if (entity1 == entity2) return true;
//			}
//			else if (is_VERTEX(entity2)) {
//				ENTITY_LIST adj_edges_list;
//				api_get_edges(entity2, adj_edges_list);
//				if (adj_edges_list.lookup(entity1) != -1) return true;
//			}
//		}
//		else if (is_VERTEX(entity1)) {
//			ENTITY_LIST adj_edges_list1;
//			api_get_edges(entity1, adj_edges_list1);
//			if (is_EDGE(entity2)) {
//				if (adj_edges_list1.lookup(entity2) != -1) return true;
//			}
//			else if (is_VERTEX(entity2)) {
//				ENTITY_LIST adj_edges_list2;
//				api_get_edges(entity2, adj_edges_list2);
//				for (int i = 0; i != adj_edges_list2.count(); ++i) {
//					if (adj_edges_list1.lookup(adj_edges_list2[i]) != -1) return true;
//				}
//			}
//		}
//
//		return false;
//	};
//
//	foreach (const auto &vh1, vhs_on_border_quad_set) {
//		std::unordered_set<OvmVeH> adj_vhs;
//		get_adj_vertices_around_vertex(mesh, vh1, adj_vhs);
//		bool flag = false;
//		foreach (const auto &vh2, adj_vhs) {
//			if (mesh->is_boundary(vh2) && contains(vhs_on_shrink_set, vh2) && 
//				!contains(vhs_on_border_quad_set, vh2)) {
//					if (fIsEdgeOnTwoVhs(vh1, vh2)) {
//						if (!flag) {                 // 第一个点和v1相连的网格边在几何边上
//							flag = true;
//						}
//						else {                       // 第二个点和v1相连的网格边在几何边上
//							return false;
//						}
//					}
//			}
//		}
//	}
//
//	//--------------------------------------------------------------------------------
//
//	foreach (const auto &vh1, vhs_on_border_quad_set) {
//		std::unordered_set<OvmEgH> adj_ehs;
//		get_adj_edges_around_vertex(mesh, vh1, adj_ehs);
//		foreach (const auto &eh, adj_ehs) {
//			mesh->edge(eh).from_vertex();
//		}
//	}
//
//	return true;
//}


/*
 * 对于单个quad set做inflation判断的
 * 对应2016-2017年冬学期第七周报 rule2
 *
*/
bool is_strictly_valid_inflation_rule2(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set)
{
	std::unordered_set<OvmVeH> vhs_on_border_quad_set;
	std::unordered_set<OvmVeH> vhs_on_shrink_set;

	// Get vhs_on_border_quad_set
	foreach (const auto &fh, quad_set) {
		std::vector<OvmVeH> adj_vhs = get_adj_vertices_around_face(mesh, fh);
		foreach (const auto &vh, adj_vhs) {
			if (mesh->is_boundary(vh)) {
				vhs_on_border_quad_set.insert(vh);
			}
		}
	}
	// Get vhs_on_shrink_set
	foreach (const auto &ch, shrink_set) {
		std::vector<OvmVeH> adj_vhs = get_adj_vertices_around_hexa(mesh, ch);
		vhs_on_shrink_set.insert(adj_vhs.begin(), adj_vhs.end());
	}

	foreach (const auto &vh1, vhs_on_border_quad_set) {
		std::unordered_set<EDGE*> edge_set1;  // {和v1相连的在quad set上的网格边，所归属到的几何边}
		std::unordered_set<EDGE*> edge_set2;  // {和v1相连的在不在quad set上，在shrink_set上的网格边，所归属到的几何边}
		std::unordered_set<OvmEgH> adj_ehs;
		get_adj_edges_around_vertex(mesh, vh1, adj_ehs);
		foreach (const auto &eh, adj_ehs) {
			auto from_vh = mesh->edge(eh).from_vertex();
			auto to_vh = mesh->edge(eh).to_vertex();
			auto vh2 = mesh->InvalidVertexHandle;

			if (from_vh == vh1 || to_vh == vh1) {
				if (from_vh == vh1) vh2 = to_vh;
				else if (to_vh == vh1) vh2 = from_vh;
			
				// Get edge_set1
				if (mesh->is_boundary(vh2) && contains(vhs_on_border_quad_set, vh2)) {
					EDGE* tmp_edge = get_associated_geometry_edge_of_boundary_eh(mesh, eh);
					if (tmp_edge != NULL) {
						edge_set1.insert(tmp_edge);
					}
				}

				// Get edge_set2
				if (mesh->is_boundary(vh2) && !contains(vhs_on_border_quad_set, vh2) &&
					contains(vhs_on_shrink_set, vh2)) {
					EDGE* tmp_edge = get_associated_geometry_edge_of_boundary_eh(mesh, eh);
					if (tmp_edge != NULL) {
						edge_set2.insert(tmp_edge);
					}
				}
			}
		}

		if (intersects(edge_set1, edge_set2)) return false;
	}

	return true;
}

bool is_valid_inflation_case(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set,
	std::unordered_set<OvmCeH> &shrink_set)
{
	return (is_valid_inflation_rule1(mesh, quad_set, shrink_set) &&
		is_strictly_valid_inflation_rule2(mesh, quad_set, shrink_set));
}


// Helper function for get_better_shrink_set_for_single_quad_set
static int abs_mesh_num_after_inflation_between_idea_num(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, 
														 std::unordered_set<OvmCeH> &shrink_set,
														 std::map<OvmEgH, EdgeAttribute> &edge_property_map)
{
	int total = 0;
	std::unordered_set<OvmEgH> ehs_on_quad_set_boundary;

	// Get ehs_on_quad_set_boundary
	foreach (const auto &fh, quad_set) {
		auto hehs_vec = mesh->face(fh).halfedges();
		foreach (const auto &heh, hehs_vec) {
			OvmEgH eh = mesh->edge_handle(heh);
			if (mesh->is_boundary(eh)) {
				ehs_on_quad_set_boundary.insert(eh);
			}
		}
	}

	// 得到一个在几何边上网格边在inflation后相连的网格数
	//   下列的实现方式依赖于shrink_set需要是complete的shrink set
	auto fMeshNumAfterInflation = [&](OvmEgH eh, std::unordered_set<OvmCeH> &shrink_set)->int {
		int count = 0;
		std::unordered_set<OvmCeH> adj_chs;
		get_adj_hexas_around_edge(mesh, eh, adj_chs);

		foreach (const auto &ch, adj_chs) {
			if (!contains(shrink_set, ch)) {
				count++;
			}
		}

		return count;
	};

	foreach (const auto &eh, ehs_on_quad_set_boundary) {
		auto tmp_edge = get_associated_geometry_edge_of_boundary_eh(mesh, eh);
		if (tmp_edge != NULL) {
			total += std::abs(edge_property_map[eh].idealvalence-1 - fMeshNumAfterInflation(eh, shrink_set));
		}
	}
	
	return total;
}

void get_better_shrink_set_for_single_quad_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set,
	std::unordered_set<OvmCeH> &shrink_set)
{
	shrink_set.clear();

	std::unordered_set<OvmCeH> shrink_set1, shrink_set2;
	bool res1 = X_get_shrink_set_on_direction(mesh, quad_set, shrink_set1, 0);
	bool res2 = X_get_shrink_set_on_direction(mesh, quad_set, shrink_set2, 1);

	if (res1 && res2 == false) {
		if (res1) {
			shrink_set = shrink_set1;
			complete_shrink_set(mesh, quad_set, shrink_set);
		}
		else {
			shrink_set = shrink_set2;
			complete_shrink_set(mesh, quad_set, shrink_set);
		}
		return;
	}

	complete_shrink_set(mesh, quad_set, shrink_set1);
	complete_shrink_set(mesh, quad_set, shrink_set2);


	// 得到网格中每条边的属性，即边的类型、度数以及理想度数
	// 我们主要用的是理想度数
	std::map<OvmEgH, EdgeAttribute> edge_property_map;
	get_edge_property(mesh, edge_property_map);

	int dif_num1 = abs_mesh_num_after_inflation_between_idea_num(mesh, quad_set, shrink_set1, edge_property_map);
	int dif_num2 = abs_mesh_num_after_inflation_between_idea_num(mesh, quad_set, shrink_set2, edge_property_map);

	if (dif_num1 < dif_num2) {
		shrink_set = shrink_set1;
	}
	else {
		shrink_set = shrink_set2;
	}
}


void get_traditional_shrink_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set)
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
		shrink_set = shrink_set1; // 默认选择0方向的shrink set
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

bool get_shrink_set_in_cylinder_case_wrong(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set)
{
	shrink_set.clear();
	std::unordered_set<OvmVeH> bound_vhs_on_quad_set;
	std::map<OvmFaH, int> quad_direct_map;
	std::map<OvmVeH, int> vh_direct_map;
	std::hash_map<OvmVeH, std::unordered_set<OvmFaH> > vh_next_around_quads_map;

	auto fCompleteSmallShrinkSet = [&](OvmVeH vh, std::unordered_set<OvmFaH> &adj_fhs, std::unordered_set<OvmCeH> &small_shrink_set) {
		std::vector<std::unordered_set<OvmCeH> > cell_groups;
		get_cell_groups_around_vertex(mesh, vh, adj_fhs, cell_groups);
		foreach (const auto &chs, cell_groups) {
			if (intersects(chs, small_shrink_set)) {
				small_shrink_set.insert(chs.begin(), chs.end());
			}
		}
	};

	auto fGetAroundFhsAndNextAroundFhs = [&](OvmVeH vh, std::unordered_set<OvmFaH> &quad_set, 
						  std::unordered_set<OvmFaH> &around_fhs, std::unordered_set<OvmFaH> &next_around_fhs) {
		std::unordered_set<OvmFaH> tmp_fhs;
		get_adj_faces_around_vertex(mesh, vh, tmp_fhs);
		
		around_fhs.clear();
		next_around_fhs.clear();

		std::for_each(tmp_fhs.begin(), tmp_fhs.end(), [&](OvmFaH fh) { if (contains(quad_set, fh)) around_fhs.insert(fh);} );

		foreach (const auto &fh, around_fhs) {
			auto vhs_vec = get_adj_vertices_around_face(mesh, fh);
			foreach (const auto &vh, vhs_vec) {
				tmp_fhs.clear();
				get_adj_faces_around_vertex(mesh, vh, tmp_fhs);
				std::for_each(tmp_fhs.begin(), tmp_fhs.end(),
						      [&](OvmFaH fh) {if (contains(quad_set, fh) && (!contains(around_fhs, fh))) next_around_fhs.insert(fh);});
			}
		}
	};

	auto fOKInDirection = [&](OvmVeH vh, int direction, std::unordered_set<OvmFaH> &quad_set) -> bool {
		std::unordered_set<OvmFaH> tmp_fhs, adj_fhs;
		get_adj_faces_around_vertex(mesh, vh, tmp_fhs);
		std::for_each(tmp_fhs.begin(), tmp_fhs.end(), 
					  [&](OvmFaH fh) { if (contains(quad_set, fh)) adj_fhs.insert(fh);});

		std::unordered_set<OvmCeH> small_shrink_set;

		if (X_get_shrink_set_on_direction(mesh, adj_fhs, small_shrink_set, direction)) {
			fCompleteSmallShrinkSet(vh, adj_fhs, small_shrink_set);
			if (is_strictly_valid_inflation_rule2(mesh, adj_fhs, small_shrink_set)) {
				return true;
			}
		}
		return false;

	};

	// Get bound vhs on quad set
	foreach (const auto &fh, quad_set) {
		std::vector<OvmVeH> vhs_vec = get_adj_vertices_around_face(mesh, fh);
		std::for_each(vhs_vec.begin(), vhs_vec.end(), 
			[&](OvmVeH vh){ if (mesh->is_boundary(vh)) bound_vhs_on_quad_set.insert(vh);});
	}

	// Classify vhs on quad set bound by its directions
	foreach (const auto &vh, bound_vhs_on_quad_set) {
		if (!fOKInDirection(vh, 0, quad_set)) {
			if (fOKInDirection(vh, 1, quad_set)) {
				vh_direct_map[vh] = 1;

				std::unordered_set<OvmFaH> around_fhs, next_around_fhs;
				fGetAroundFhsAndNextAroundFhs(vh, quad_set, around_fhs, next_around_fhs);
				
				foreach (const auto &fh, around_fhs) {
					if (quad_direct_map.find(fh) == quad_direct_map.end()) {
						quad_direct_map[fh] = vh_direct_map[vh];
					}
				}
				vh_next_around_quads_map[vh] = next_around_fhs;
			}
			else {
				return false;
			}
		}
		else if (!fOKInDirection(vh, 1, quad_set)) {
			if (fOKInDirection(vh, 0, quad_set)) {
				vh_direct_map[vh] = 0;
			
				std::unordered_set<OvmFaH> around_fhs, next_around_fhs;
				fGetAroundFhsAndNextAroundFhs(vh, quad_set, around_fhs, next_around_fhs);
			
				foreach (const auto &fh, around_fhs) {
					if (quad_direct_map.find(fh) == quad_direct_map.end()) {
						quad_direct_map[fh] = vh_direct_map[vh];
					}
				}
				vh_next_around_quads_map[vh] = next_around_fhs;
			}
			else {
				return false;
			}
		}
	}
	
	// 从一个点开始往其他点扩展
	OvmVeH start_node = vh_next_around_quads_map.begin()->first;
	std::unordered_set<OvmFaH> visited_fhs;
	std::unordered_set<OvmFaH> around_fhs, next_around_fhs;

	auto fGetVhFromNextAroundFh = [&](OvmFaH fh) -> OvmVeH {
		for (auto iter = vh_next_around_quads_map.begin(); iter != vh_next_around_quads_map.end(); ++iter) {
			if (contains(iter->second, fh)) {
				return iter->first;
			}
		}
		return mesh->InvalidVertexHandle;
	};

	fGetAroundFhsAndNextAroundFhs(start_node, quad_set, around_fhs, next_around_fhs);
	visited_fhs.insert(around_fhs.begin(), around_fhs.end());
	for (auto iter = quad_direct_map.begin(); iter != quad_direct_map.end(); ++iter) {
		visited_fhs.insert(iter->first);
	}

	// 不断地一层一层地传播下去
	while (!next_around_fhs.empty()) {
		visited_fhs.insert(next_around_fhs.begin(), next_around_fhs.end());
		foreach (const auto &fh, next_around_fhs) {
			auto new_vh = fGetVhFromNextAroundFh(fh);
			if (new_vh == mesh->InvalidVertexHandle && quad_direct_map.find(fh) == quad_direct_map.end()) {
				quad_direct_map[fh] = vh_direct_map[start_node];
			}
			else if (vh_direct_map[new_vh] == vh_direct_map[start_node]) {
				foreach (const auto &fh, vh_next_around_quads_map[new_vh]) {
					if (quad_direct_map.find(fh) == quad_direct_map.end()) {
						quad_direct_map[fh] = vh_direct_map[start_node];
					}
				}
			}
		}

		std::unordered_set<OvmFaH> new_next_around_fhs;
		foreach (const auto &fh, next_around_fhs) {
			auto vhs_vec = get_adj_vertices_around_face(mesh, fh);
			foreach (const auto &vh, vhs_vec) {
				std::unordered_set<OvmFaH> tmp_fhs;
				get_adj_faces_around_vertex(mesh, vh, tmp_fhs);
				std::for_each(tmp_fhs.begin(), tmp_fhs.end(),
							  [&](OvmFaH fh) {if (contains(quad_set, fh) && !contains(visited_fhs, fh)) new_next_around_fhs.insert(fh);});
			}
		}
		next_around_fhs = new_next_around_fhs;
	}
	
	qDebug() << "The size of quad_direct_map: " << quad_direct_map.size();
	
	
	for (auto iter = quad_direct_map.begin(); iter != quad_direct_map.end(); ++iter) {
		auto hfh = mesh->halfface_handle(iter->first, iter->second);
		auto ch = mesh->incident_cell(hfh);
		shrink_set.insert(ch);
	}

	return true;
}

bool get_shrink_set_in_cylinder_case(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set,
									std::vector<std::unordered_set<OvmFaH> > &debug_group_fhs,
									std::unordered_set<OvmFaH> &debug_fhs)
{
	assert(mesh->vertex_property_exists<unsigned long>("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
	shrink_set.clear();
	std::unordered_set<OvmVeH> bound_vhs_on_quad_set;
	std::unordered_set<OvmFaH> visited_fhs, bound_fhs;
	std::vector<std::unordered_set<OvmFaH> > bound_fhs_groups, around_fhs_groups;
	std::unordered_set<OvmVeH> directed_vhs;

	auto fGetAroundFhsAndNextAroundFhs = [&](OvmVeH vh, std::unordered_set<OvmFaH> &quad_set, 
		std::unordered_set<OvmFaH> &around_fhs, std::unordered_set<OvmFaH> &next_around_fhs) {
			std::unordered_set<OvmFaH> tmp_fhs;
			get_adj_faces_around_vertex(mesh, vh, tmp_fhs);

			around_fhs.clear();
			next_around_fhs.clear();

			std::for_each(tmp_fhs.begin(), tmp_fhs.end(), [&](OvmFaH fh) { if (contains(quad_set, fh)) around_fhs.insert(fh);} );

			foreach (const auto &fh, around_fhs) {
				auto vhs_vec = get_adj_vertices_around_face(mesh, fh);
				foreach (const auto &vh, vhs_vec) {
					tmp_fhs.clear();
					get_adj_faces_around_vertex(mesh, vh, tmp_fhs);
					std::for_each(tmp_fhs.begin(), tmp_fhs.end(),
						[&](OvmFaH fh) {if (contains(quad_set, fh) && (!contains(around_fhs, fh))) next_around_fhs.insert(fh);});
				}
			}
	};

	auto fGetAroundFhsFromFh = [&](OvmFaH fh, std::unordered_set<OvmFaH> &quad_set)->std::unordered_set<OvmFaH> {
		std::unordered_set<OvmFaH> around_fhs;
		auto vhs_vec = get_adj_vertices_around_face(mesh, fh);
		foreach (auto &vh, vhs_vec) {
			std::unordered_set<OvmFaH> fhs;
			get_adj_faces_around_vertex(mesh, vh, fhs);
			std::for_each(fhs.begin(), fhs.end(), 
						  [&](OvmFaH fh){if (contains(quad_set, fh)) around_fhs.insert(fh);});
		}
		return around_fhs;
	};

	// 返回一个面集，集合中的每一个面是和当前面的边相连，且在quad set上的面
	auto fGetNextFhsFromFh = [&](OvmFaH fh, std::unordered_set<OvmFaH> &quad_set)->std::unordered_set<OvmFaH> {
		std::unordered_set<OvmFaH> next_fhs;
		auto vhs_vec = get_adj_vertices_around_face(mesh, fh);
		foreach (auto &vh, vhs_vec) {
			std::unordered_set<OvmFaH> tmp_fhs;
			get_adj_faces_around_vertex(mesh, vh, tmp_fhs);
			foreach (auto &fh2, tmp_fhs) {
				if (fh2 != fh && contains(quad_set, fh2)) {
					next_fhs.insert(fh2);
				}
			}
		}
		return next_fhs;
	};

	// Get bound vhs on quad set
	foreach (const auto &fh, quad_set) {
		std::vector<OvmVeH> vhs_vec = get_adj_vertices_around_face(mesh, fh);
		std::for_each(vhs_vec.begin(), vhs_vec.end(), 
			[&](OvmVeH vh){ if (mesh->is_boundary(vh)) bound_vhs_on_quad_set.insert(vh);});
	}

	// 如周报2016-2017年春季第二周周报所述
	// 找完bound_fhs，上面的有的点，可能没有cell来指导其几何归属。
	//
	// 修个了shrink_set
	auto fCompleteBoundaryVhShrinkSet = [&](std::unordered_set<OvmFaH> &one_bound_fhs) {
		std::unordered_set<OvmVeH> vhs_on_bound_fhs;
		foreach (auto &fh, one_bound_fhs) {
			auto vhs_vec = get_adj_vertices_around_face(mesh, fh);
			vhs_on_bound_fhs.insert(vhs_vec.begin(), vhs_vec.end());
		}

		foreach (auto &vh, vhs_on_bound_fhs) {
			std::unordered_set<OvmFaH> tmp_fhs;
			get_adj_faces_around_vertex(mesh, vh, tmp_fhs);

			int num = 0;
			foreach (auto &fh, tmp_fhs) {
				if (contains(quad_set, fh)) {
					num++;
				}
			}

			if (num == 1) {
				std::vector<std::unordered_set<OvmCeH> > cell_groups;
				get_cell_groups_around_vertex(mesh, vh, quad_set, cell_groups);
				assert(cell_groups.size() == 2);

				std::unordered_set<OvmFaH> fhs_of_shrink_set;
				foreach (auto &ch , shrink_set) {
					auto hfhs = mesh->cell(ch).halffaces();
					foreach (auto &hfh, hfhs) {
						OvmFaH fh = mesh->face_handle(hfh);
						fhs_of_shrink_set.insert(fh);
					}
				}

				std::unordered_set<OvmFaH> fhs_of_cell_group0, fhs_of_cell_group1;
				foreach (auto &ch , cell_groups[0]) {
					auto hfhs = mesh->cell(ch).halffaces();
					foreach (auto &hfh, hfhs) {
						OvmFaH fh = mesh->face_handle(hfh);
						fhs_of_cell_group0.insert(fh);
					}
				}
				foreach (auto &ch , cell_groups[1]) {
					auto hfhs = mesh->cell(ch).halffaces();
					foreach (auto &hfh, hfhs) {
						OvmFaH fh = mesh->face_handle(hfh);
						fhs_of_cell_group1.insert(fh);
					}
				}				

				if (intersection(fhs_of_cell_group0, fhs_of_shrink_set).size() > 0) {
					shrink_set.insert(cell_groups[0].begin(), cell_groups[0].end());
				}
				else {
					shrink_set.insert(cell_groups[1].begin(), cell_groups[1].end());
				}
			}
		}
	};

	// 改变了visited_fhs
	// 改变了shrink_set
	// 改变了bound_fhs
	auto fExtendAroundFhs = [&](OvmVeH vh, std::unordered_set<OvmFaH> &around_fhs) {
		foreach (auto &fh, around_fhs) {
			visited_fhs.insert(fh);
		}
		std::unordered_set<OvmFaH> new_around_fhs(around_fhs);
		std::unordered_set<OvmFaH> total_around_fhs;   // 记录从这个点出发扩展过的面集

		while (!new_around_fhs.empty()) {
			std::unordered_set<OvmFaH> next_around_fhs;
			std::unordered_set<OvmVeH> tmp_vhs;
			foreach (auto &fh, new_around_fhs) {
				total_around_fhs.insert(fh);
				auto vhs_vec = get_adj_vertices_around_face(mesh, fh);
				tmp_vhs.insert(vhs_vec.begin(), vhs_vec.end());
			}

			foreach (auto &vh, tmp_vhs) {
				// 只通过在边界上的点来扩展shrink set，因为现在只需要在边界上的点有shrink set来指导
				if (!mesh->is_boundary(vh)) continue; 

				std::unordered_set<OvmFaH> tmp_fhs;
				get_adj_faces_around_vertex(mesh, vh, tmp_fhs);
				foreach (auto &fh, tmp_fhs) {
					if (!contains(quad_set, fh) || contains(visited_fhs, fh)) continue;
					// 判断面上是否和几何边相连
					auto hehs = mesh->face(fh).halfedges();
					bool flag = false;
					foreach (auto heh, hehs) {
						if (get_associated_geometry_edge_of_boundary_eh(mesh, mesh->edge_handle(heh)) != NULL) {
							flag = true;
							break;
						}
					}
					if (!flag) continue;
					
					std::vector<std::unordered_set<OvmCeH> > cell_groups;
					get_cell_groups_around_vertex(mesh, vh, quad_set, cell_groups);
					assert(cell_groups.size() == 2);
					
					if (intersection(cell_groups[0], shrink_set).size() > 0) {
						foreach (auto &ch, cell_groups[0]) {
							auto ch_vhs = get_adj_vertices_around_cell(mesh, ch);

							shrink_set.insert(ch);
							directed_vhs.insert(ch_vhs.begin(), ch_vhs.end());
						}
					}
					else if (intersection(cell_groups[1], shrink_set).size() > 0) {
						foreach (auto &ch, cell_groups[1]) {
							auto ch_vhs = get_adj_vertices_around_cell(mesh, ch);

							shrink_set.insert(ch);
							directed_vhs.insert(ch_vhs.begin(), ch_vhs.end());
						}
					}

					next_around_fhs.insert(fh);
					visited_fhs.insert(fh);
				}
			}
			new_around_fhs = next_around_fhs;
		}

		// 创造一圈边界出来
		// 通过添加visited_fhs
		//-------------------
		std::unordered_set<OvmFaH> one_bound_fhs;
		foreach (auto &fh, total_around_fhs) {
			auto adj_fhs = fGetNextFhsFromFh(fh, quad_set);
			foreach (auto &fh2, adj_fhs) {
				if (!contains(visited_fhs, fh2)) {
					bound_fhs.insert(fh2);
					one_bound_fhs.insert(fh2);
				}
			}
		}

		bound_fhs_groups.push_back(one_bound_fhs);
		around_fhs_groups.push_back(total_around_fhs);

		fCompleteBoundaryVhShrinkSet(one_bound_fhs);
	};

	// 对于特殊点，把特殊点附件的quad对应的cell加入到shrink_set中去
	foreach (const auto &vh, bound_vhs_on_quad_set) {
		if (contains(directed_vhs, vh)) continue;

		std::unordered_set<OvmFaH> around_fhs, next_around_fhs;
		fGetAroundFhsAndNextAroundFhs(vh, quad_set, around_fhs, next_around_fhs);
		std::vector<std::unordered_set<OvmCeH> > cell_groups;
		get_cell_groups_around_vertex(mesh, vh, quad_set, cell_groups);

		assert(cell_groups.size() == 2);
		if (!is_strictly_valid_inflation_rule2(mesh, around_fhs, cell_groups[0])) {
			if (is_strictly_valid_inflation_rule2(mesh, around_fhs, cell_groups[1])) {
				std::unordered_set<OvmCeH> spread_chs; // 用来记录当前这次扩展访问过的chs
				foreach (auto &ch, cell_groups[1]) {
					shrink_set.insert(ch);
				}
				// 判断是否有冲突，当一个面两边都有cell的时候，冲突产生
				foreach (auto &fh, around_fhs) {
					auto ch0 = mesh->incident_cell(mesh->halfface_handle(fh, 0));
					auto ch1 = mesh->incident_cell(mesh->halfface_handle(fh, 1));

					if (contains(shrink_set, ch0) && contains(shrink_set, ch1)) {
						return false;
					}
				}
				fExtendAroundFhs(vh, around_fhs);

				directed_vhs.insert(vh);
			}			
		}
		else {
			if (!is_strictly_valid_inflation_rule2(mesh, around_fhs, cell_groups[1])) {
				std::unordered_set<OvmCeH> spread_chs;
				foreach (auto &ch, cell_groups[0]) {
					shrink_set.insert(ch);
				}
				// 判断是否有冲突
				foreach (auto &fh, around_fhs) {
					auto ch0 = mesh->incident_cell(mesh->halfface_handle(fh, 0));
					auto ch1 = mesh->incident_cell(mesh->halfface_handle(fh, 1));

					if (contains(shrink_set, ch0) && contains(shrink_set, ch1)) {
						return false;
					}
				}
				fExtendAroundFhs(vh, around_fhs);
				directed_vhs.insert(vh);
			}
		}
	}

	// 从已经处理过的fhs出发找剩下还没有处理过的fhs
	std::queue<OvmFaH> fhs_queue;

	assert(around_fhs_groups.size() > 0);
	foreach (auto &fh, around_fhs_groups[0]) {
		fhs_queue.push(fh);
	}

	while (!fhs_queue.empty()) {
		OvmFaH current_fh = fhs_queue.front();
		fhs_queue.pop();

		std::vector<OvmVeH> vhs_vec = get_adj_vertices_around_face(mesh, current_fh);
		foreach (auto &vh, vhs_vec) {
			std::unordered_set<OvmFaH> adj_fhs;
			get_adj_faces_around_vertex(mesh, vh, adj_fhs);
			foreach (auto &fh, adj_fhs) {
				if (contains(bound_fhs, fh) && !contains(bound_fhs_groups[0], fh)) continue;
				if (contains(quad_set, fh) && !contains(visited_fhs, fh)) {
					std::vector<std::unordered_set<OvmCeH> > cell_groups;
					get_cell_groups_around_vertex(mesh, vh, quad_set, cell_groups);
					assert(cell_groups.size() == 2);
					if (intersection(shrink_set, cell_groups[0]).size() > 0) {
						foreach (auto &ch, cell_groups[0]) {
							// 注意这里处理的技巧
							bool ch_useable = true;
							auto hfhs_vec = mesh->cell(ch).halffaces();
							foreach (auto &hfh, hfhs_vec) {
								auto fh2 = mesh->face_handle(hfh);
								if (contains(bound_fhs, fh2) && !contains(bound_fhs_groups[0], fh2)) {
									ch_useable = false;
									break;
								}
							}
							if (!ch_useable) continue;

							shrink_set.insert(ch);
						}
					}
					else if (intersection(shrink_set, cell_groups[1]).size() > 0) {
						foreach (auto &ch, cell_groups[1]) {
							bool ch_useable = true;
							auto hfhs_vec = mesh->cell(ch).halffaces();
							foreach (auto &hfh, hfhs_vec) {
								auto fh2 = mesh->face_handle(hfh);
								if (contains(bound_fhs, fh2) && !contains(bound_fhs_groups[0], fh2)) {
									ch_useable = false;
									break;
								}
							}
							if (!ch_useable) continue;

							shrink_set.insert(ch);
						}
					}
					visited_fhs.insert(fh);
					fhs_queue.push(fh);
				}
			}
		}
	}
	

	//-----------debug----------//
	debug_fhs = bound_fhs;
	debug_group_fhs = bound_fhs_groups;
	//-----------debug----------//

	complete_shrink_set_in_cylinder_case(mesh, quad_set, shrink_set);

	return true;
}


/*
 * Changed shrink set
 */
void complete_shrink_set_in_cylinder_case(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set)
{
	std::unordered_set<OvmCeH> old_shrink_set(shrink_set);
	std::unordered_set<OvmVeH> vhs_on_quad_set;
	foreach (auto &fh, quad_set) {
		std::vector<OvmVeH> tmp_vhs = get_adj_vertices_around_face(mesh, fh);
		vhs_on_quad_set.insert(tmp_vhs.begin(), tmp_vhs.end());
	}

	foreach (auto &vh, vhs_on_quad_set) {
		std::vector<std::unordered_set<OvmCeH> > cell_groups;
		get_cell_groups_around_vertex(mesh, vh, quad_set, cell_groups);
	
		foreach (auto &chs, cell_groups) {
			if (intersection(chs, old_shrink_set).size() > 0) {
				foreach (auto &ch, chs) {
					bool flag = true;
					auto hfhs = mesh->cell(ch).halffaces();
					foreach (auto &hfh, hfhs) {
						auto fh = mesh->face_handle(hfh);
						if (contains(quad_set, fh)) {
							flag = false;
							break;
						}
					}
					if (flag) {
						shrink_set.insert(ch);
					}
				}
			}
		}

		//foreach (auto &chs, cell_groups) {
		//	if (intersection(chs, old_shrink_set).size() > 0) {
		//		foreach (auto &ch, chs) {
		//			bool adj_to_bound_fhs = false;
		//			foreach (auto &hfh, mesh->cell(ch).halffaces()) {
		//				auto fh = mesh->face_handle(hfh);
		//				if (contains(bound_fhs, fh)) {
		//					adj_to_bound_fhs = true;
		//					break;
		//				}
		//			}

		//			if (!adj_to_bound_fhs) {
		//				shrink_set.insert(ch);
		//			}
		//		}

		//		break;
		//	}
		//}
	}
}


bool sheet_inflation_by_halfface(VolumeMesh *mesh, std::vector<std::unordered_set<OvmHaFaH> > &halfface_quad_sets,
	std::vector<DualSheet *> &sheets = std::vector<DualSheet *> ())
{
	std::vector<std::unordered_set<OvmCeH> > shrink_sets;
	std::vector<std::unordered_set<OvmFaH> > quad_sets;

	// Get shrink sets
	foreach (auto &halfface_quad_set, halfface_quad_sets) {
		std::unordered_set<OvmCeH> shrink_set;
		std::unordered_set<OvmFaH> quad_set;

		foreach (auto &hfh, halfface_quad_set) {
			if (mesh->incident_cell(hfh) == mesh->InvalidCellHandle) {
				return false;
			}
			else {
				shrink_set.insert(mesh->incident_cell(hfh));
			}

			quad_set.insert(mesh->face_handle(hfh));
		}

		complete_shrink_set(mesh, quad_set, shrink_set);
		shrink_sets.push_back(shrink_set);
		quad_sets.push_back(quad_set);
	}

	sheets = DM_sheet_inflation(mesh, quad_sets, shrink_sets);

	return true;
}


// 得到关于自交的quad set 自交部分指导inflation的cells 
static bool get_direct_cells_for_self_cross_part(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set,
												 std::vector<std::unordered_set<OvmFaH> > &bound_fhs_groups,
												 std::vector<std::unordered_set<OvmFaH> > &around_fhs_groups,
												 std::unordered_set<OvmVeH> &directed_vhs,
												 std::unordered_set<OvmFaH> &visited_fhs)
{
	// Lambda function
	// 得到int_ehs(quad sets相交出来的交线边边集) 
	auto fGetIntEhs = [](VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::vector<std::unordered_set<OvmEgH> > &int_ehs_vec) {
		int_ehs_vec.clear();
		std::unordered_set<OvmEgH> int_ehs;

		// Firstly, find all int_ehs
		foreach (const auto &fh, inflation_quad_set) {
			auto hehs = mesh->face(fh).halfedges();
			foreach (const auto &heh, hehs) {
				auto eh = mesh->edge_handle(heh);
				auto fhs = get_adj_faces_around_edge(mesh, eh, false);
				int adj_fs_num = 0;
				foreach (const auto &fh2, fhs) {
					if (contains(inflation_quad_set, fh2)) adj_fs_num++;
				}
				if (adj_fs_num == 4) {
					int_ehs.insert(eh);
				}
			}
		}

		// Secondly, generate int_ehs_vec to store intersaction edge handles
		while (!int_ehs.empty()) {
			std::unordered_set<OvmEgH> one_part_ehs;
			OvmEgH first_eh = *(int_ehs.begin());
			std::queue<OvmEgH> ehs_que;
			std::unordered_set<OvmEgH> visited_ehs;
			ehs_que.push(first_eh);
			visited_ehs.insert(first_eh);

			// Use BFS to find one intersaction edges
			while (!ehs_que.empty()) {
				OvmEgH eh1 = ehs_que.front();
				ehs_que.pop();
				one_part_ehs.insert(eh1);

				std::unordered_set<OvmVeH> tmp_vhs1;
				tmp_vhs1.insert(mesh->edge(eh1).to_vertex());
				tmp_vhs1.insert(mesh->edge(eh1).from_vertex());

				foreach (auto &eh2, int_ehs) {
					if (contains(visited_ehs, eh2)) continue;
					std::unordered_set<OvmVeH> tmp_vhs2;
					tmp_vhs2.insert(mesh->edge(eh2).to_vertex());
					tmp_vhs2.insert(mesh->edge(eh2).from_vertex());

					if (intersection(tmp_vhs1, tmp_vhs2).size() == 1) {
						ehs_que.push(eh2);
						visited_ehs.insert(eh2);
					}
				}
			}

			int_ehs_vec.push_back(one_part_ehs);
			foreach (auto &eh, one_part_ehs) {
				int_ehs.erase(eh);
			}
		}
	};

	auto fGetAdjFacesAroundCells = [&](const std::unordered_set<OvmCeH> &chs)-> std::unordered_set<OvmFaH> {
		std::unordered_set<OvmFaH> total_fhs;
		foreach (auto &ch, chs) {
			std::vector<OvmFaH> fhs = get_adj_faces_around_cell(mesh, ch);
			total_fhs.insert(fhs.begin(), fhs.end());
		}
		return total_fhs;
	};
	
	// Lambda function
	// 对于一条十字交叉线，生成相应指导其几何归属的cells
	auto fGetDirectCellsForAIntEhs = [&](std::unordered_set<OvmEgH> &int_ehs, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set,
										 std::vector<std::unordered_set<OvmFaH> > &around_fhs_groups, std::vector<std::unordered_set<OvmFaH> > &bound_fhs_groups, 
										 std::unordered_set<OvmVeH> &directed_vhs,
										 std::unordered_set<OvmFaH> &visited_fhs) {

		std::unordered_set<OvmVeH> vhs_on_int_ehs, bound_vhs_on_int_ehs;
		std::unordered_set<OvmFaH> fhs_on_shrink_set = fGetAdjFacesAroundCells(shrink_set);
		
		foreach (auto &eh, int_ehs) {
			auto vh1 = mesh->edge(eh).from_vertex();
			auto vh2 = mesh->edge(eh).to_vertex();
			vhs_on_int_ehs.insert(vh1);
			vhs_on_int_ehs.insert(vh2);
			if (mesh->is_boundary(vh1)) {
				bound_vhs_on_int_ehs.insert(vh1);
			}
			if (mesh->is_boundary(vh2)) {
				bound_vhs_on_int_ehs.insert(vh2);
			}
		}
		
		// 先找到一个点附近的指导cells
		bool flag = false; // 表示指导第一个点的cells有没有找到
		std::unordered_set<OvmCeH> critical_cells; // 表示首先确定下来的四个cells_group中的一个那些cells
		OvmVeH begin_vh = mesh->InvalidVertexHandle;

		foreach (auto &vh, bound_vhs_on_int_ehs) {
			if (flag) break;
			std::vector<std::unordered_set<OvmCeH> > cells_groups;
			get_cell_groups_around_vertex(mesh, vh, quad_set, cells_groups);
			foreach (auto &cells, cells_groups) {
				std::unordered_set<OvmFaH> fhs_on_cells = fGetAdjFacesAroundCells(cells);
				if (intersects(cells, shrink_set) || intersects(fhs_on_cells, fhs_on_shrink_set)) {
					flag = true;
					critical_cells.insert(cells.begin(), cells.end());
					begin_vh = vh;
					break;
				}
				if (flag) break;
			}
		}
		if (begin_vh == mesh->InvalidVertexHandle) {
			begin_vh = *(bound_vhs_on_int_ehs.begin());
			std::vector<std::unordered_set<OvmCeH> > cells_groups;
			get_cell_groups_around_vertex(mesh, begin_vh, quad_set, cells_groups);
			critical_cells.insert(cells_groups[0].begin(), cells_groups[0].end());
		}

		std::unordered_set<OvmFaH> around_fhs, bound_fhs;
		// 再从这个点来扩展，不断地找到指导其他点的cells
		std::queue<OvmVeH> vhs_queue;
		vhs_queue.push(begin_vh);
		std::unordered_set<OvmEgH> to_check_ehs(int_ehs);
		while (!vhs_queue.empty()) {
			OvmVeH vh = vhs_queue.front();

			//---------------debug------------//
			//qDebug() << "next_vh: " << vh.idx();
			//---------------debug------------//

			vhs_queue.pop();

			std::vector<std::unordered_set<OvmCeH> > cells_groups;
			std::unordered_set<OvmCeH> that_cells; // 对于这个点的critical_cells
			get_cell_groups_around_vertex(mesh, vh, quad_set, cells_groups);
		
			// 找这个点的critical_cells
			foreach (auto &cells, cells_groups) {
				// 找到这个点的critical_cells, 进行相关的处理
				if (intersects(cells, critical_cells)) {
					that_cells = cells;
					critical_cells.insert(cells.begin(), cells.end());
					break;
				}
			}

			//-----------------------------------------------
			// 进行这个Lambda函数最关键的一步
			// 找指导这个点相应的cells，并将其加入shrink set
			// 找这个点的around_fhs
			//------------------------------------------------
			
			// 找这个点的around_fhs 
			std::unordered_set<OvmFaH> fhs_on_that_cells = fGetAdjFacesAroundCells(that_cells);
			std::unordered_set<OvmFaH> tmp_fhs, fhs_for_extend;
			get_adj_faces_around_vertex(mesh, vh, tmp_fhs);
			foreach (auto fh, tmp_fhs) {
				if (contains(quad_set, fh)) {
					around_fhs.insert(fh);
					if (contains(fhs_on_that_cells, fh)) {
						fhs_for_extend.insert(fh);
					}
				}
			}
			// 扩展一下around_fhs
			foreach (auto fh, fhs_for_extend) {
				auto vhs_vec = get_adj_vertices_around_face(mesh, fh);
				foreach (auto vh2, vhs_vec) {
					std::unordered_set<OvmFaH> tmp_fhs;
					get_adj_faces_around_vertex(mesh, vh2, tmp_fhs);
					foreach (auto fh2, tmp_fhs) {
						if (contains(quad_set, fh2)) {
							around_fhs.insert(fh2);
						}
					}
				}
			}

			// 添加shrink set
			// 添加that_cells到shrink set
			std::unordered_set<OvmCeH> chs_add_to_shrink_set;
			chs_add_to_shrink_set.insert(that_cells.begin(), that_cells.end());
			
			// 添加that_cells相连的用于延展的cells
			std::unordered_set<OvmVeH> vhs_on_fhs_for_extend;
			foreach (auto fh, fhs_for_extend) {
				auto vhs_vec = get_adj_vertices_around_face(mesh, fh);
				vhs_on_fhs_for_extend.insert(vhs_vec.begin(), vhs_vec.end());
			}
			std::unordered_set<OvmVeH> adj_vhs, vhs_for_extend;
			get_adj_vertices_around_vertex(mesh, vh, adj_vhs);
			foreach (auto vh2, adj_vhs) {
				if (contains(vhs_on_fhs_for_extend, vh2)) {
					vhs_for_extend.insert(vh2);
				}
			}
			foreach (auto vh2, vhs_for_extend) {
				std::vector<std::unordered_set<OvmCeH> > tmp_cells_groups;
				get_cell_groups_around_vertex(mesh, vh2, quad_set, tmp_cells_groups);
				foreach (auto cells, tmp_cells_groups) {
					if (intersects(cells, that_cells)) {
						chs_add_to_shrink_set.insert(cells.begin(), cells.end());
						break;
					}
				}
			}

			// 添加十字交叉另外两边的cells
			foreach (auto cells, cells_groups) {
				if (cells == that_cells) continue;
				std::unordered_set<OvmFaH> tmp_fhs = fGetAdjFacesAroundCells(cells);
				if (intersects(tmp_fhs, fhs_for_extend)) {
					chs_add_to_shrink_set.insert(cells.begin(), cells.end());
				}
			}

			// 将找到的指导的cells加入shrink set
			// 并更新directed_vhs
			shrink_set.insert(chs_add_to_shrink_set.begin(), chs_add_to_shrink_set.end());
			foreach (auto ch, chs_add_to_shrink_set) {
				auto vhs_vec = get_adj_vertices_around_cell(mesh, ch);
				directed_vhs.insert(vhs_vec.begin(), vhs_vec.end());
			}

			// 结束最关键的一步
			//-------------------------------------------------

			// 找下一个点
			OvmEgH adj_eh = mesh->InvalidEdgeHandle;
			foreach (auto eh, to_check_ehs) {
				auto vh1 = mesh->edge(eh).from_vertex();
				auto vh2 = mesh->edge(eh).to_vertex();
			
				if (vh1 == vh) {
					vhs_queue.push(vh2);
					adj_eh = eh;
					break;
				}
				else if (vh2 == vh) {
					vhs_queue.push(vh1);
					adj_eh = eh;
					break;
				}
			}
			if (adj_eh != mesh->InvalidEdgeHandle) {
				to_check_ehs.erase(adj_eh);
			}
		}

		// 找bound_fhs
		around_fhs_groups.push_back(around_fhs);
		foreach (auto fh, around_fhs) {
			visited_fhs.insert(fh);
		}
		foreach (auto fh, around_fhs) {
			auto vhs_vec = get_adj_vertices_around_face(mesh, fh);
			foreach (auto vh2, vhs_vec) {
				std::unordered_set<OvmFaH> tmp_fhs;
				get_adj_faces_around_vertex(mesh, vh2, tmp_fhs);
				foreach (auto fh2, tmp_fhs) {
					if (contains(quad_set, fh2) && !contains(around_fhs, fh2)) {
						bound_fhs.insert(fh2);
					}
				}
			}
		}
		bound_fhs_groups.push_back(bound_fhs);
	};

	std::vector<std::unordered_set<OvmEgH> > int_ehs_vec;
	fGetIntEhs(mesh, quad_set, int_ehs_vec);
	
	foreach (auto int_ehs, int_ehs_vec) {
		fGetDirectCellsForAIntEhs(int_ehs, quad_set, shrink_set, 
								  around_fhs_groups, bound_fhs_groups,
								  directed_vhs, visited_fhs);	
	}

	return true;
}


// 得到关于quad set上只能往一个方向inflation的指导cells
static bool get_direct_cells_for_one_inflation_vhs(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set,
												   std::vector<std::unordered_set<OvmFaH> > &bound_fhs_groups,
												   std::vector<std::unordered_set<OvmFaH> > &around_fhs_groups,
												   std::unordered_set<OvmVeH> &directed_vhs,
												   std::unordered_set<OvmFaH> &visited_fhs)
{
	std::unordered_set<OvmVeH> bound_vhs_on_quad_set;

	// Lambda function:
	// 如周报2016-2017年春季第二周周报所述
	// bound_fhs，上面的有的点，可能没有cell来指导其几何归属。
	// 所以要对bound_fhs上的点加入cells来指导其几何归属
	auto fCompleteBoundaryVhShrinkSet = [&](std::unordered_set<OvmFaH> &one_bound_fhs) {
		std::unordered_set<OvmVeH> vhs_on_bound_fhs;
		foreach (auto &fh, one_bound_fhs) {
			auto vhs_vec = get_adj_vertices_around_face(mesh, fh);
			vhs_on_bound_fhs.insert(vhs_vec.begin(), vhs_vec.end());
		}

		foreach (auto &vh, vhs_on_bound_fhs) {
			std::unordered_set<OvmFaH> tmp_fhs;
			get_adj_faces_around_vertex(mesh, vh, tmp_fhs);

			int num = 0;
			foreach (auto &fh, tmp_fhs) {
				if (contains(quad_set, fh)) {
					num++;
				}
			}

			if (num == 1) {
				std::vector<std::unordered_set<OvmCeH> > cell_groups;
				get_cell_groups_around_vertex(mesh, vh, quad_set, cell_groups);
				assert(cell_groups.size() == 2);

				std::unordered_set<OvmFaH> fhs_of_shrink_set;
				foreach (auto &ch , shrink_set) {
					auto hfhs = mesh->cell(ch).halffaces();
					foreach (auto &hfh, hfhs) {
						OvmFaH fh = mesh->face_handle(hfh);
						fhs_of_shrink_set.insert(fh);
					}
				}

				std::unordered_set<OvmFaH> fhs_of_cell_group0, fhs_of_cell_group1;
				foreach (auto &ch , cell_groups[0]) {
					auto hfhs = mesh->cell(ch).halffaces();
					foreach (auto &hfh, hfhs) {
						OvmFaH fh = mesh->face_handle(hfh);
						fhs_of_cell_group0.insert(fh);
					}
				}
				foreach (auto &ch , cell_groups[1]) {
					auto hfhs = mesh->cell(ch).halffaces();
					foreach (auto &hfh, hfhs) {
						OvmFaH fh = mesh->face_handle(hfh);
						fhs_of_cell_group1.insert(fh);
					}
				}               

				if (intersection(fhs_of_cell_group0, fhs_of_shrink_set).size() > 0) {
					shrink_set.insert(cell_groups[0].begin(), cell_groups[0].end());
				}
				else {
					shrink_set.insert(cell_groups[1].begin(), cell_groups[1].end());
				}
			}
		}
	};

	// Lambda function:
	// 判断边界上的点是否只能往一个方向inflation，如果是找出指导这个点inflation的cells
	auto fOnlyBeInflatedOneDirection = [&](OvmVeH vh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &direct_cells)->bool {
		direct_cells.clear();
		std::vector<std::unordered_set<OvmCeH> > cells_groups;
		std::unordered_set<OvmFaH> adj_small_quad_set;
		get_cell_groups_around_vertex(mesh, vh, quad_set, cells_groups);

		std::unordered_set<OvmFaH> tmp_fhs;
		get_adj_faces_around_vertex(mesh, vh, tmp_fhs);
		foreach (auto &fh, tmp_fhs) {
			if (contains(quad_set, fh)) {
				adj_small_quad_set.insert(fh);
			}
		}

		if (cells_groups.size() == 1) {
			direct_cells = cells_groups[0];	
			return true;
		}
		else if (cells_groups.size() == 2) {
			if (!is_strictly_valid_inflation_rule2(mesh, adj_small_quad_set, cells_groups[0])) {
				if (is_strictly_valid_inflation_rule2(mesh, adj_small_quad_set, cells_groups[1])) {
					direct_cells = cells_groups[1];
					return true;
				}           
			}
			else {
				if (!is_strictly_valid_inflation_rule2(mesh, adj_small_quad_set, cells_groups[1])) {
					direct_cells = cells_groups[0];
					return true;
				}
			}
		}

		return false;
	};

	// Lambda function:
	// 输入只能往一个方向inflation的点，将其加入点集。如果其周围的点也只能往一个方向inflation那么不断地扩展这个点集
	// 并将这些点所对应cells加入shrink set中
	// 改变了:
	// directed_vhs
	// shrink_set
	// bound_fhs_groups
	// around_fhs_groups
	//
	auto fOneDirectionInflationVhsExtend = [&](OvmVeH vh, std::unordered_set<OvmCeH> direct_cells, std::unordered_set<OvmFaH> &quad_set){
		// 标记点及附加已经被cells指导方向
		foreach (auto &ch, direct_cells) {
			shrink_set.insert(ch);
			std::vector<OvmVeH> tmp_vhs = get_adj_vertices_around_cell(mesh, ch);
			foreach (auto &vh, tmp_vhs) {
				directed_vhs.insert(vh);
			}
		}
		
		std::unordered_set<OvmFaH> around_fhs, tmp_fhs, one_around_fhs;
		get_adj_faces_around_vertex(mesh, vh, tmp_fhs);
		std::for_each(tmp_fhs.begin(), tmp_fhs.end(), 
					  [&](OvmFaH fh) {if (contains(quad_set, fh)) {around_fhs.insert(fh);} });
		foreach (auto &fh, around_fhs) {
			visited_fhs.insert(fh);
		}

		while (!around_fhs.empty()) {
			std::unordered_set<OvmFaH> next_around_fhs;

			// 找到 next_around_fhs
			std::unordered_set<OvmVeH> tmp_vhs;
			foreach (auto &fh, around_fhs) {
				one_around_fhs.insert(fh);
				std::vector<OvmVeH> adj_vhs = get_adj_vertices_around_face(mesh, fh);
				std::for_each(adj_vhs.begin(), adj_vhs.end(), 
							  [&](OvmVeH vh) { tmp_vhs.insert(vh); });
			}
			// 通过中间vh来找 next_around_fhs
			foreach (auto &vh, tmp_vhs) {
				// 这里的实现还是比较有技巧，因为我们只需要关注在模型表面边界上的点有cells 来指导
				if (!mesh->is_boundary(vh)) continue; 

				std::unordered_set<OvmFaH> tmp_fhs;
				get_adj_faces_around_vertex(mesh, vh, tmp_fhs);
				
				foreach (auto &fh, tmp_fhs) {
					if (contains(visited_fhs, fh) || !contains(quad_set, fh)) continue;
					// (1) 判断这个面是否在边界上，incident的cell是不是和shrink set贴合
					if (mesh->is_boundary(fh)) {
						std::vector<std::unordered_set<OvmCeH> > cell_groups;
						get_cell_groups_around_vertex(mesh, vh, quad_set, cell_groups);
						assert(cell_groups.size() == 1);

						if (!intersects(shrink_set, cell_groups[0])) continue;

						foreach (auto &ch, cell_groups[0]) {
							auto ch_vhs = get_adj_vertices_around_cell(mesh, ch);
							shrink_set.insert(ch);
							directed_vhs.insert(ch_vhs.begin(), ch_vhs.end());

						}
						next_around_fhs.insert(fh);
						visited_fhs.insert(fh);
						continue;
					}
					
					// (2) 判断面上是否和几何边相连
					auto hehs = mesh->face(fh).halfedges();
					bool flag = false;
					foreach (auto heh, hehs) {
						if (get_associated_geometry_edge_of_boundary_eh(mesh, mesh->edge_handle(heh)) != NULL) {
							flag = true;
							break;
						}
					}
					if (!flag) continue;

					std::vector<std::unordered_set<OvmCeH> > cell_groups;
					get_cell_groups_around_vertex(mesh, vh, quad_set, cell_groups);
					assert(cell_groups.size() == 2);

					if (intersection(cell_groups[0], shrink_set).size() > 0) {
						foreach (auto &ch, cell_groups[0]) {
							auto ch_vhs = get_adj_vertices_around_cell(mesh, ch);

							shrink_set.insert(ch);
							directed_vhs.insert(ch_vhs.begin(), ch_vhs.end());
						}
					}
					else if (intersection(cell_groups[1], shrink_set).size() > 0) {
						foreach (auto &ch, cell_groups[1]) {
							auto ch_vhs = get_adj_vertices_around_cell(mesh, ch);

							shrink_set.insert(ch);
							directed_vhs.insert(ch_vhs.begin(), ch_vhs.end());
						}
					}

					next_around_fhs.insert(fh);
					visited_fhs.insert(fh);
				}
			}

			around_fhs = next_around_fhs;
		}

		// 创造一圈边界出来
		// 通过添加visited_fhs
		//-------------------
		std::unordered_set<OvmFaH> one_bound_fhs;
		foreach (auto &fh, one_around_fhs) {
			std::vector<OvmVeH> tmp_vhs = get_adj_vertices_around_face(mesh, fh);
			foreach (auto &vh, tmp_vhs) {
				std::unordered_set<OvmFaH> tmp_fhs;
				get_adj_faces_around_vertex(mesh, vh, tmp_fhs);
				foreach (auto &fh2, tmp_fhs) {
					if (contains(quad_set, fh2) && !contains(visited_fhs, fh2)) {
						one_bound_fhs.insert(fh2);
					}
				}
			}
		}

		bound_fhs_groups.push_back(one_bound_fhs);
		around_fhs_groups.push_back(one_around_fhs);

		fCompleteBoundaryVhShrinkSet(one_bound_fhs);

	};


	// Get bound vhs on quad set
	foreach (const auto &fh, quad_set) {
		std::vector<OvmVeH> vhs_vec = get_adj_vertices_around_face(mesh, fh);
		std::for_each(vhs_vec.begin(), vhs_vec.end(), 
			[&](OvmVeH vh){ if (mesh->is_boundary(vh)) bound_vhs_on_quad_set.insert(vh);});
	}

	foreach (auto &vh, bound_vhs_on_quad_set) {
		if (contains(directed_vhs, vh)) continue;

		std::unordered_set<OvmCeH> direct_cells;
		if (fOnlyBeInflatedOneDirection(vh, quad_set, direct_cells)) {
			directed_vhs.insert(vh);

			fOneDirectionInflationVhsExtend(vh, direct_cells, quad_set);
		}
	}

	return true;
}

static bool get_direct_cells_for_bound_fhs_groups(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set,
	std::vector<std::unordered_set<OvmFaH> > &bound_fhs_groups,
	std::vector<std::unordered_set<OvmFaH> > &around_fhs_groups,
	std::unordered_set<OvmVeH> &directed_vhs,
	std::unordered_set<OvmFaH> &visited_fhs)
{
	// 处理每一环bound_fhs
	foreach (auto bound_fhs, bound_fhs_groups) {

		// 从已经处理过的fhs出发找剩下还没有处理过的fhs
		std::queue<OvmFaH> fhs_queue;

		assert(around_fhs_groups.size() > 0);
		foreach (auto &fh, around_fhs_groups[0]) {
			fhs_queue.push(fh);
		}

		while (!fhs_queue.empty()) {
			OvmFaH current_fh = fhs_queue.front();
			fhs_queue.pop();

			std::vector<OvmVeH> vhs_vec = get_adj_vertices_around_face(mesh, current_fh);
			foreach (auto &vh, vhs_vec) {
				std::unordered_set<OvmFaH> adj_fhs;
				get_adj_faces_around_vertex(mesh, vh, adj_fhs);
				foreach (auto &fh, adj_fhs) {
					if (contains(bound_fhs, fh) && !contains(bound_fhs_groups[0], fh)) continue;
					if (contains(quad_set, fh) && !contains(visited_fhs, fh)) {
						std::vector<std::unordered_set<OvmCeH> > cell_groups;
						get_cell_groups_around_vertex(mesh, vh, quad_set, cell_groups);
						assert(cell_groups.size() == 2);
						if (intersection(shrink_set, cell_groups[0]).size() > 0) {
							foreach (auto &ch, cell_groups[0]) {
								// 注意这里处理的技巧
								bool ch_useable = true;
								auto hfhs_vec = mesh->cell(ch).halffaces();
								foreach (auto &hfh, hfhs_vec) {
									auto fh2 = mesh->face_handle(hfh);
									if (contains(bound_fhs, fh2) && !contains(bound_fhs_groups[0], fh2)) {
										ch_useable = false;
										break;
									}
								}
								if (!ch_useable) continue;

								shrink_set.insert(ch);
							}
						}
						else if (intersection(shrink_set, cell_groups[1]).size() > 0) {
							foreach (auto &ch, cell_groups[1]) {
								bool ch_useable = true;
								auto hfhs_vec = mesh->cell(ch).halffaces();
								foreach (auto &hfh, hfhs_vec) {
									auto fh2 = mesh->face_handle(hfh);
									if (contains(bound_fhs, fh2) && !contains(bound_fhs_groups[0], fh2)) {
										ch_useable = false;
										break;
									}
								}
								if (!ch_useable) continue;

								shrink_set.insert(ch);
							}
						}
						visited_fhs.insert(fh);
						fhs_queue.push(fh);
					}
				}
			}
		}


	}


	return true;
}


bool get_shrink_set_for_self_cross_quad_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set,
	std::vector<std::unordered_set<OvmFaH> > &debug_fhs_groups1,
	std::vector<std::unordered_set<OvmFaH> > &debug_fhs_groups2)

{
	std::unordered_set<OvmVeH> directed_vhs;
	std::unordered_set<OvmVeH> bound_vhs_on_quad_set;
	std::vector<std::unordered_set<OvmFaH> > bound_fhs_groups, around_fhs_groups;
	std::unordered_set<OvmFaH> visited_fhs;
	shrink_set.clear();

	if (!get_direct_cells_for_one_inflation_vhs(mesh, quad_set, shrink_set,
											   bound_fhs_groups, around_fhs_groups, directed_vhs,
											   visited_fhs)) {
		return false;
	}

	if (!get_direct_cells_for_self_cross_part(mesh, quad_set, shrink_set,
											  bound_fhs_groups, around_fhs_groups, directed_vhs,
											  visited_fhs)) {
		return false;
	}

	if (!get_direct_cells_for_bound_fhs_groups(mesh, quad_set, shrink_set,
		bound_fhs_groups, around_fhs_groups, directed_vhs,
		visited_fhs)) {
			return false;
	}



	//-----------------------------------------------------------------//
	debug_fhs_groups1 = around_fhs_groups;
	debug_fhs_groups2 = bound_fhs_groups;

}