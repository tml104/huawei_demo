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

	// �ж�һ�����Ƿ���quad set�ĹսǴ�
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
	// 0��������quad set�ĹսǴ�
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



	// Ϊ�˴���quad set�ĹսǴ������
	//auto helper_vh = mesh->InvalidVertexHandle;
	std::vector<OvmVeH> helper_vhs;

	// Ϊ�˴���ǽṹ����shrink set
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
	// original_vh ��quad set�ĹսǴ�
	else {
		assert(helper_vhs.size() == 2);

		//if (origin_vh.idx() == 4697) {
		//	qDebug() << "__LINE__: " << __LINE__;
		//	qDebug() << "--------------------";
		//}

		// ��һ���������ϣ��õ�һ����Խǵ��Ǹ���
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
		// ֮ǰʵ�ֵķ�����������
		//////////////////////////////////////////////////////
#ifdef OLD_WAY 
		// �ҶԽǵĵ�
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
// ���ݲ��ֵ�quad set, shrink set��original_vh�Ƶ������ѳ����µ�ļ��ι���
// ����˼������Ϊ�·��ѳ��ĵ�Ӧ�ù�����ԭ���shrink set�����������ĶԵ������ϡ�
// ԭ��origianl_vh
// shrink set�����������ĶԵ� adj_vh
static unsigned long infer_new_V_ENTITY_PTR_from_two_vhs(VolumeMesh *mesh, OvmVeH origi_vh, OvmVeH oppo_vh)
{
	assert (origi_vh != mesh->InvalidVertexHandle);
	assert (oppo_vh != mesh->InvalidVertexHandle);
	assert (mesh->vertex_property_exists<unsigned long>("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
	unsigned long origin_V_ENTITY_PTR = V_ENTITY_PTR[origi_vh];

	auto entity = (ENTITY*) (V_ENTITY_PTR[origi_vh]);
	auto oppo_entity = (ENTITY*) (V_ENTITY_PTR[oppo_vh]);

	// ԭ���ĵ������ڵ����
	if (origin_V_ENTITY_PTR == 0) {
		return 0;
	}
	else if (V_ENTITY_PTR[oppo_vh] == 0) { // �Ե������ڣ�������inflation
		return 0;
	}
	else if (is_FACE(entity)){ // ԭ���ĵ�������
		if (V_ENTITY_PTR[oppo_vh] != 0) { // ԭ���ĵ�����ı߽���
			return origin_V_ENTITY_PTR;
		}
		else { // ԭ���ĵ����������
			return 0; // �·��ѳ����ĵ�������
		}
	}
	else if (is_EDGE(entity)) {
		if (V_ENTITY_PTR[oppo_vh] != 0) { // ԭ���ĵ��ڱ߽���
			if (V_ENTITY_PTR[oppo_vh] == origin_V_ENTITY_PTR) { // (1) ���ѳ����µ㻹���������α���
				return origin_V_ENTITY_PTR;
			}
			else {
				if (is_FACE(oppo_entity)) {                     // (2) ���ѳ����µ�������
					return V_ENTITY_PTR[oppo_vh];
				}
				else {
					if (is_VERTEX(oppo_entity)) {              // (3) ���ѳ����µ�����ڵ��ϣ������������ԭ�������ڵıߣ�����

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
					// (4) ʣ�µ����,���ѳ����µ��ڵ��ϻ����
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
		if (V_ENTITY_PTR[oppo_vh] != 0) { // ԭ���ĵ��ڱ߽���
			if (is_EDGE(oppo_entity)) {                 // (1) ���ѳ������µ��ڱ��� misunderstanding.����Ӧ��Ҫ�����������
				// �µ��ڱ��ϣ��������ߺ;ɵ�����
				ENTITY_LIST adj_vertices_to_new_entity;
				api_get_vertices(oppo_entity, adj_vertices_to_new_entity);
				for (int i = 0; i < adj_vertices_to_new_entity.count(); ++i) {
					if (adj_vertices_to_new_entity[i] == entity) {
						return V_ENTITY_PTR[oppo_vh];
					}
				}

				// �µ��ڱ��ϣ��������߲��;ɵ�����
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
			else if (is_FACE(oppo_entity)) {            // (2) ���ѳ����µ�������
				return V_ENTITY_PTR[oppo_vh]; 
			}
			else if (is_VERTEX(oppo_entity)) {
				// �µ�;ɵ�֮���б�����
				ENTITY_LIST adj_edges_to_entity;
				ENTITY_LIST adj_edges_to_new_entity;
				api_get_edges(entity, adj_edges_to_entity);
				api_get_edges(oppo_entity, adj_edges_to_new_entity);
				for (int i = 0; i < adj_edges_to_entity.count(); i++) {
					if (adj_edges_to_new_entity.lookup(adj_edges_to_entity[i]) != -1) {
						return (unsigned long) adj_edges_to_entity[i];
					}
				}

				// �µ�;ɵ�֮��û�б�����
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


// �����ͨ���������һ������ѳ����������������ѳ��ĵ��ǵļ��ι���
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

// ���ʮ���ཻ��������õ�quad setsʮ�ֽ��洦�ĵ���ѳ��ĵ��ǵļ��ι���
// quad sets�ཻ��ʮ��״������origi_vh�����Ŀռ�ֳ�4���֣��ֱ��Ӧһ��one_chs_group
// ���Ǹ���one_chs_group�õ���Ӧ�ķ��ѳ����µ�ļ��ι���
// Eg.
// # ��ʾshrink set
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
// ������one_chs_group
// ��һ��one_chs_group: ��ͼ[0]
//   �ص㣺 û�к�shrink set�ཻ
//   �µĵ�ļ��ι����� ԭ��ļ��ι���
//
// �ڶ���one_chs_group: ��ͼ[1][2]
//   �ص㣺 ��shrink set�ཻ�Һ͵�һ��one_chs_group����
//   �µĵ�ļ��ι���: ����ԭ���ԭ�������one_chs_group����ͬshrink setһ�����򣩵�һ���Ե㣨����ͬһ���ߣ�
//                   (���õķ�����get_new_V_ENTITY_PTRs_for_ord)һ�������õ��µļ��ι���
// 
// ������one_chs_group: ��ͼ[3]
//   �ص㣺��shrink set�ཻ�����͵�һ��one_chs_group����
//   �µĵ�ļ��ι���������ԭ������Ӧ��[1][2]����one_chs_group����Ķ�Ӧ�ĵ����������ļ��ι���
static unsigned long get_new_V_ENTITY_PTRs_for_cross(VolumeMesh *mesh, OvmVeH origi_vh, 
	const std::unordered_set<OvmCeH> &one_chs_group, const std::vector<std::unordered_set<OvmCeH> > &cell_groups,
	const std::unordered_set<OvmFaH> &quad_set, const std::unordered_set<OvmCeH> &shrink_set)
{
	assert (mesh->vertex_property_exists<unsigned long>("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

	// �����һ��one_chs_group
	// ���one_chs_groupû�к�shrink_set�ཻ�������ǾͰѺ�one_chs_group��������µ�ļ��ι�������Ϊ
	// ����ǰ�ĵ�ļ��ι���
	if (!intersects(one_chs_group, shrink_set)) {
		return V_ENTITY_PTR[origi_vh];
	}

	// �õ�û����shrink set�е�chs_group
	std::unordered_set<OvmCeH> chs_groups_not_in_shrink_set;
	foreach (const auto &chs_group, cell_groups) {
		if (!intersects(chs_group, shrink_set)) {
			chs_groups_not_in_shrink_set = chs_group;
			break;
		}
	}
	assert (chs_groups_not_in_shrink_set.size() > 0);

	// �õ�����chs_group,��chs_groups_not_in_shrink_set���ڣ�
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

	// ����ڶ���one_chs_group
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

	// ���������one_chs_group
	// һ������������
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

	// ���one_chs_groupû�к�shrink_set�ཻ�������ǾͰѺ�one_chs_group��������µ�ļ��ι�������Ϊ
	// ����ǰ�ĵ�ļ��ι���
	if (!intersects(one_chs_group, shrink_set)) {
		return V_ENTITY_PTR[origi_vh];
	}

	// �ж������ڵ�һ��sheet(���ı�sheet����)
	// ���ǵڶ���sheet(�ı�sheet����)
	std::unordered_set<OvmVeH> vhs_on_int_ehs;
	foreach (const auto &eh, int_ehs) {
		auto vh1 = mesh->edge(eh).from_vertex();
		auto vh2 = mesh->edge(eh).to_vertex();
		vhs_on_int_ehs.insert(vh1);
		vhs_on_int_ehs.insert(vh2);
	}

	// ��һ��sheet(���ı�sheet����)
	if (!contains(vhs_on_int_ehs, origi_vh)) {
		return get_new_V_ENTITY_PTRs_for_ord(mesh, origi_vh, one_chs_group, quad_set, shrink_set);
	}
	// �ڶ���sheet(�ı�sheet����)
	else {
		// ���ж�one_chs_group��λ��
		assert(cell_groups.size() == 3);
		std::unordered_set<OvmCeH> middle_cell_group; // ���м�ģ����������ߵ�cell_group

		for (int i = 0; i < cell_groups.size(); ++i) {
			for (int j = i+1; j < cell_groups.size(); ++j) {
				std::unordered_set<OvmFaH> comm_fhs = get_common_faces(mesh, cell_groups[i], cell_groups[j]);
				if (intersects(comm_fhs, int_fhs)) {
					int mid_index = (1+2+3) - (i+1) - (j+1) - 1;
					middle_cell_group = cell_groups[mid_index];
				}
			}
		}

		// one_chs_group ���м�
		if (one_chs_group == middle_cell_group) {
			return get_new_V_ENTITY_PTRs_for_ord(mesh, origi_vh, one_chs_group, quad_set, shrink_set);
		}
		// one_chs_group ������
		else {
			// ��Ҫ�µ�one_cell_group��quad_set
			std::unordered_set<OvmCeH> new_cell_group = get_union(one_chs_group, middle_cell_group);
			std::unordered_set<OvmFaH> new_quad_set;

			
			// ����quad_set
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

// �õ����ѵ��µ�ļ��ι���
static unsigned long get_new_V_ENTITY_PTR_from_cell_group(VolumeMesh *mesh, OvmVeH origi_vh, 
	const std::unordered_set<OvmCeH> &one_chs_group, const std::vector<std::unordered_set<OvmCeH> > &cell_groups,
	const std::unordered_set<OvmFaH> &quad_set, const std::unordered_set<OvmCeH> &shrink_set,
	const std::unordered_set<OvmEgH> &int_ehs, const std::unordered_set<OvmFaH> &int_fhs)
{
	assert (mesh->vertex_property_exists<unsigned long>("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");

	// qDebug() << "get_new_V_ENTITY_PTR_from_cell_group";
	// ���ѵĵ�������
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

	// ���ѵĵ��ڱ߽���
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


//���ں������garbage_collection��ʱ���ɾ����һЩ�㣬Ȼ��OVM�ڲ������¼��㶥��ľ����ֵ
//������ҪԤ�ȱ���һ�¾ɵĶ����Ӧ�ļ���λ���Լ�ENTITY_PTR��ֵ
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
	// �õ�inflation_quad_set(����quad_set�Ĳ���)
	// �õ�int_fhs(quad sets�ཻ���漯)
	// �õ�shrink_set(����shrink_sets�Ĳ���)
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
	// �õ�int_ehs(quad sets�ཻ�����Ľ��߱߱߼�) 
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
	// ��Ե������ ֻ��Ҫ��һ��
	//
	// ��int_ehs(quad sets�ཻ�����Ľ��߱߱߼�)����
	// �õ���int_ehs_on_int_fhs(quad sets�ཻ��߽��ϵı߼�)�еı߼���ʹ���ܹ�ͳһ����������һ���������
	// �õ��ı߼����ܹ���int_ehs����ͨ��
	// �������أ��߼�
	auto fGetEhsConnectedIntEhs = [&]()->std::unordered_set<OvmEgH> {
		std::unordered_set<OvmEgH> res_ehs;

		// �ж��������Ƿ�����(�Ƿ��ܹ��������ŵ�������)
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
	// �õ�int_ehs(quad setsʮ���ཻ�������ߵı߼�)Ҳ��������һ���int_ehs_on_int_fhs(���֣�Ϊ��ͳһ����������һ���������)
	// �õ�int_ehs_on_int_fhs(quad sets�ཻ��߽��ϵı߼�)
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

		// �޸�int_ehsʹ��ֻ������ͨ�ཻ�ıߣ���������ཻ��ֻ���ཻ����
		foreach (const auto &eh, int_ehs_on_int_fhs) {
			int_ehs.erase(eh);
		}

		// ���ݵõ���ehs_connected_int_ehs���ı�
		// int_ehs��int_ehs_on_int_fhs��ֵ
		auto connected_ehs = fGetEhsConnectedIntEhs();
		foreach (const auto &eh, connected_ehs) {
			int_ehs.insert(eh);
			int_ehs_on_int_fhs.erase(eh);
		}

		// ͨ��shrink set���õ�Ӧ��������һ���������һ���߼�, �⴮�������ܹ��ı�����sheet������ // �����ڶ�������sheet
		// �������int_ehs
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
	std::unordered_set<OvmVeH> vhs_on_int_ehs_on_int_fhs_for_dual_sheet_1; //������һ������sheet�ĵ㼯
	// �õ��������ɵ�һ��sheet�ĵ㼯
	// Ҳ��������㼯������Ϊ���ܹ����������quad sets�ཻ��߽紦������
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
	//��žɵĵ���µĵ�֮���ӳ���ϵ
	//����һ��������ɵĵ�һ��Ϊ�������ڽ��洦�ɵĵ�һ��Ϊ�ģ����Ծɵ���µ��м������һ�Զ�Ķ�Ӧ��ϵ��
	//Ϊ������ɵ㵽�׶�Ӧ���ĸ��µ㣬��Ҫһ�������弯�������жϣ��������弯�Ͼ��Ǿɵ���ϵ�һ��shrink_set
	auto newly_created_vertices_cells_mapping = new std::map<OvmVeH, std::map<OvmVeH, std::unordered_set<OvmCeH> > >();
	//new_original_vhs_mapping�洢�����ɵĵ��ԭʼ��֮��Ķ�Ӧ��ϵ����һ�ֶ��һ�Ĺ�ϵ
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
	// ������ģ���е�ÿ�����һ��V_PREV_HANDLE�����������sheet inflationǰ��vertex handleֵ
	// ����ģ���е�ÿ���㶼��һ��������ϵ�����ǰ����������ϵ��V_ENTITY_PTR����ʾ
	assert(mesh->vertex_property_exists<unsigned long>("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
	auto V_PREV_HANDLE = mesh->request_vertex_property<OvmVeH>("prevhandle", mesh->InvalidVertexHandle);
	std::unordered_set<OvmVeH> all_vhs_on_fhs; // quad sets�ϵĵ�
	// Ϊ���ѵ���׼��
	// ���fhs�ϵ����ж���
	auto fPreSplitVhs = [&]() {
		// ��ʼ��V_PREV_HANDLE
		for (auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
			V_PREV_HANDLE[*v_it] = *v_it;

		foreach(auto &fh, inflation_quad_set){
			auto hehs = mesh->face(fh).halfedges();
			foreach(auto &heh, hehs)
				all_vhs_on_fhs.insert(mesh->halfedge(heh).to_vertex());
		}
	};

	// ��������������ǵõ�����cell_group���ཻ��ĸ���
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
	// ��quad set�ڱ߽����ʱ�������ĸ����п��ܳ���
	// �ú����б���ѵ�ĸ����Ƿ����
	// ����������
	// quad set�ڱ߽���ϣ����±���Ӧ�÷��ѳ������㣬����quad set������
	// ���ѳ��Ľ��Ϊ���ѳ���������
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
	// ����������������ж�����cell�Ƿ��ţ�����һ����
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
	// ��quad sets��ÿһ������з���
	// ��������£�һ����ֻ��һ��quad set�ϣ��������ѳ������µ�
	// ʮ���ཻ������£�һ����ͬʱ������quad set�ϣ� �������ѳ�4���µ�
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
				// ��quad set�ϵĵ��ڱ߽����ʱ��ԭ�����㷨���ҵ�����cell groups
				// ��Ҫ������cell groups �ϲ���һ��

				std::unordered_set<OvmCeH> right_cell_group;
				int right_pos = -1;
				for (int i = 0; i < cell_groups1.size(); i++) {
					int touch_num = 0; // ���cell group���������ٸ�cell group����
					for (int j = 0; j < cell_groups1.size(); j++) {
						if (i == j) continue;
						bool touch = false; // �ж�������cell group�Ƿ���������
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

			//���fhs�������������������ʱ��cell_groupsֻ����һ�����ϣ����ʱ��Ҫ�ٲ���һ���ռ���
			if (cell_groups.size() == 1){
				std::unordered_set<OvmCeH> tmp;
				tmp.insert(mesh->InvalidCellHandle);
				cell_groups.push_back(tmp);
			}
			////////////////////////////////////////////////////////////
			// DD:
			// ��inflation_quad_set�²�һ���ǹ����һ��sheet������Ҫ�б���һ��:
			// �����inflation_quad_set�������и�cell,������û��cell����Ҫ��һ��
			// InvalidCellHandle�����ֶԳ�
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
				//����Ҫ�ж���one_chs_group�Ƿ���shrink_set�С�����ڵĻ�����ô����µĵ����²����ģ���������entity_ptrΪ��
				//�������shrink_set�У���ô��������ԭ���ľɵ㣨��Ϊ�����޸ĵ���Ҫ�����½�������

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
				if (V_ENTITY_PTR[vh] == 0){ //�����ڲ���
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
	// ��sheet inflationǰ��ģ�͵ĵ�ļ�����Ϣ�͵�Ĺ�����ϵ��������
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
		// ����Ҳ��Ӧ�ð�ԭ���ĵ�(�ڽ����ϵĵ�)Ҳ����vh_info_for_readd
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
	std::hash_map<OvmCeH, std::vector<OvmVeH> > cells_rebuilding_recipes; //(cell, ��Ӧ���µ�8����)
	//�Ѽ���Щ��Ҫ�ؽ���������İ˸�����
	auto fCollectVhsOfRebuildHex = [&]() {
		foreach(auto &ch, all_adj_cells) {
			std::vector<OvmVeH> ch_vhs;
			std::hash_map<OvmVeH, OvmVeH> curr_old_new_vhs_mapping;//�洢��ǰ���������¾ɵ��ӳ���ϵ

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
	// �Ѽ���Щ��ͨ���򣨼������ڽ��������ϵ��µ�������İ˸�����
	auto fCollectVhsOfNewHexGeneratedBySingleQuadset = [&]() {
		std::unordered_set<OvmFaH> single_face_quad_set; // û�н����inflation quad set
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

				// ��������־��ע�������1��Ϊ�˽���������
				if (contains(vhs_on_int_fhs, *hfv_it) && ch1 == mesh->InvalidCellHandle) {
					newly_vh = *hfv_it;
				}
				ch_vhs.push_back(newly_vh);
			}

			//qSwap(ch_vhs[1], ch_vhs[3]);
			for (auto hfv_it = mesh->hfv_iter(hfh2); hfv_it; ++hfv_it){
				auto newly_vh = fGetNewVeHOnCeH(*hfv_it, ch2);
				assert(newly_vh != mesh->InvalidVertexHandle);

				// ��������־��ע���е����1��Ϊ�˽���������
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
	// �Ѽ����������µ�������İ˸�����
	auto fCollectVhsOfNewHexGeneratedByCrossQuadsets = [&]() {
		foreach(auto &eh, int_ehs) {
			auto heh = mesh->halfedge_handle(eh, 0);
			std::vector<OvmVeH> ch_vhs_up, ch_vhs_down;
			auto vh_up_origin = mesh->halfedge(heh).from_vertex(),
				vh_down_origin = mesh->halfedge(heh).to_vertex();

			// �ҳ�һ���ɵ���ѳ����ĸ��µ�
			auto fGetSplitedVhs = [&](OvmHaEgH current_heh, OvmVeH current_vh, std::vector<OvmVeH> &ch_vhs) {
				ch_vhs.clear();
				auto locate = newly_created_vertices_cells_mapping->find(current_vh);
				int num_vhs = (locate->second).size();

				// �������λ�ڴ�ͳ�����ֽ��潻�߱�Ե��quad sets�ཻ��߽��ԵʱҪ����һ��
				// �ҵ��Ǹ�����ͨ���ٽӰ�ߵİ������ҵ��ķ��ѳ����ĵ�
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
	// �Ѽ������������������ɵ��������8������
	auto fCollectVhsOfNewHexGeneratedByDualQuadsetPart = [&]() {
		foreach (const auto &fh, int_fhs) {
			auto hfh1 = mesh->halfface_handle(fh, 0);
			auto ch1 = mesh->incident_cell(hfh1);
			auto hfh2 = mesh->halfface_handle(fh, 1);
			auto ch2 = mesh->incident_cell(hfh2);

			// �Ѽ�һ�ߵ��������8����
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

			// �ռ���һ�ߵ��������8����
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
	//����ɾ���ɵ���Ҫɾ���Ķ��㣬����ɾ����������ڵ�������
	auto fDestoryRelatedCells = [&]() {
		foreach(auto &ch, all_adj_cells)
			status_attrib[ch].set_deleted(true);

		status_attrib.garbage_collection(true);
	};


	//--------------------------------------------------------------------------
	//vhs��֮ǰ����ĵ����У�fUpdateVhs���ڸ�����������У�
	//���vhs�еĵ���old_new_vhs_mapping���ܹ��ҵ���˵��garbage_collection�����в�û�н���ɾ����
	//���OpenVolumeMesh�Զ�������V_PREV_HANDLE�е����ԣ�
	//���vhs�еĵ㲻�ܹ���old_new_vhs_mapping���ҵ���˵����garbage_collection��ɾ���ˣ�
	//�����Ҫ����֮ǰ������vh_info_db�иõ�ļ�������λ�ü�������Ϣ�����ؽ���ͬʱ����V_PREV_HANDLE�õ�洢����Ϣ
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
	// ���������ɵ�������
	auto fBuildNewHex = [&]() {
		//old_new_vhs_mapping�洢�ɵĵ㣨����ǰ����Ѻ�ĵ㣬������original�ĵ㣩���µĵ�֮��Ķ�Ӧ��ϵ
		//�����Ƿ��Ѻ�ĵ㣬��������Ӧ��ϵ��һ��һӳ��
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
	// �õ�sheet inflation���ɵ�new sheets
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

			// ����all_eh_of_first_ch������sheet ����
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
// inflation_quad_setֻ��ʾһ����
// direction: [0, 1]
// ���������������
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
		// �����������������������shrink_set���м�ʱ�˳�
		// Couldn't generate the full shrink set in the direction, so quit immediately.
		if (cell == mesh->InvalidCellHandle) return false; 

		shrink_set.insert(cell);
		used_fhs.insert(mesh->face_handle(hfh));

		// ͨ��������ĸ�������������������ڣ��ҷ���һ�µİ���
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
 * ���Եõ��Խ�quad set��shrink set
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
		// �����������������������shrink_set���м�ʱ�˳�
		// Couldn't generate the full shrink set in the direction, so quit immediately.
		if (cell == mesh->InvalidCellHandle) return false; 

		shrink_set.insert(cell);
		used_fhs.insert(mesh->face_handle(hfh));

		int count_he = 0;
		// ͨ��������ĸ�������������������ڣ��ҷ���һ�µİ���
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
 * ���Եõ��Խ�quad set��shrink set
 * @ hfh ���ڵ�cell����Ŀ��shrink set�е�
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
		// �����������������������shrink_set���м�ʱ�˳�
		// Couldn't generate the full shrink set in the direction, so quit immediately.
		if (cell == mesh->InvalidCellHandle) return false; 

		shrink_set.insert(cell);
		used_fhs.insert(mesh->face_handle(hfh));

		int count_he = 0;
		// ͨ��������ĸ�������������������ڣ��ҷ���һ�µİ���
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

// inflation_quad_setֻ����һ����
// ���������Խ������
bool X_get_shrink_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmCeH> &shrink_set)
{
	bool ok = X_get_shrink_set_on_direction(mesh, inflation_quad_set, shrink_set, 0);
	if (ok) return true;
	ok = X_get_shrink_set_on_direction(mesh, inflation_quad_set, shrink_set, 1);
	return ok;
}


/*
 * ֻ�����˼򵥵������
 * ����������quad set���ཻ��
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
		// �ж�fh�ڱ߽���
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
					if (!flag) {                 // ��һ�����v1������������ڼ��α���
						flag = true;
					}
					else {                       // �ڶ������v1������������ڼ��α���
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
//						if (!flag) {                 // ��һ�����v1������������ڼ��α���
//							flag = true;
//						}
//						else {                       // �ڶ������v1������������ڼ��α���
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
 * ���ڵ���quad set��inflation�жϵ�
 * ��Ӧ2016-2017�궬ѧ�ڵ����ܱ� rule2
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
		std::unordered_set<EDGE*> edge_set1;  // {��v1��������quad set�ϵ�����ߣ����������ļ��α�}
		std::unordered_set<EDGE*> edge_set2;  // {��v1�������ڲ���quad set�ϣ���shrink_set�ϵ�����ߣ����������ļ��α�}
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

	// �õ�һ���ڼ��α����������inflation��������������
	//   ���е�ʵ�ַ�ʽ������shrink_set��Ҫ��complete��shrink set
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


	// �õ�������ÿ���ߵ����ԣ����ߵ����͡������Լ��������
	// ������Ҫ�õ����������
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
		shrink_set = shrink_set1; // Ĭ��ѡ��0�����shrink set
	}

	// ��ȫshrink set���õ���ͳ��shrink set
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
	
	// ��һ���㿪ʼ����������չ
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

	// ���ϵ�һ��һ��ش�����ȥ
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

	// ����һ���漯�������е�ÿһ�����Ǻ͵�ǰ��ı�����������quad set�ϵ���
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

	// ���ܱ�2016-2017�괺���ڶ����ܱ�����
	// ����bound_fhs��������еĵ㣬����û��cell��ָ���伸�ι�����
	//
	// �޸���shrink_set
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

	// �ı���visited_fhs
	// �ı���shrink_set
	// �ı���bound_fhs
	auto fExtendAroundFhs = [&](OvmVeH vh, std::unordered_set<OvmFaH> &around_fhs) {
		foreach (auto &fh, around_fhs) {
			visited_fhs.insert(fh);
		}
		std::unordered_set<OvmFaH> new_around_fhs(around_fhs);
		std::unordered_set<OvmFaH> total_around_fhs;   // ��¼������������չ�����漯

		while (!new_around_fhs.empty()) {
			std::unordered_set<OvmFaH> next_around_fhs;
			std::unordered_set<OvmVeH> tmp_vhs;
			foreach (auto &fh, new_around_fhs) {
				total_around_fhs.insert(fh);
				auto vhs_vec = get_adj_vertices_around_face(mesh, fh);
				tmp_vhs.insert(vhs_vec.begin(), vhs_vec.end());
			}

			foreach (auto &vh, tmp_vhs) {
				// ֻͨ���ڱ߽��ϵĵ�����չshrink set����Ϊ����ֻ��Ҫ�ڱ߽��ϵĵ���shrink set��ָ��
				if (!mesh->is_boundary(vh)) continue; 

				std::unordered_set<OvmFaH> tmp_fhs;
				get_adj_faces_around_vertex(mesh, vh, tmp_fhs);
				foreach (auto &fh, tmp_fhs) {
					if (!contains(quad_set, fh) || contains(visited_fhs, fh)) continue;
					// �ж������Ƿ�ͼ��α�����
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

		// ����һȦ�߽����
		// ͨ�����visited_fhs
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

	// ��������㣬������㸽����quad��Ӧ��cell���뵽shrink_set��ȥ
	foreach (const auto &vh, bound_vhs_on_quad_set) {
		if (contains(directed_vhs, vh)) continue;

		std::unordered_set<OvmFaH> around_fhs, next_around_fhs;
		fGetAroundFhsAndNextAroundFhs(vh, quad_set, around_fhs, next_around_fhs);
		std::vector<std::unordered_set<OvmCeH> > cell_groups;
		get_cell_groups_around_vertex(mesh, vh, quad_set, cell_groups);

		assert(cell_groups.size() == 2);
		if (!is_strictly_valid_inflation_rule2(mesh, around_fhs, cell_groups[0])) {
			if (is_strictly_valid_inflation_rule2(mesh, around_fhs, cell_groups[1])) {
				std::unordered_set<OvmCeH> spread_chs; // ������¼��ǰ�����չ���ʹ���chs
				foreach (auto &ch, cell_groups[1]) {
					shrink_set.insert(ch);
				}
				// �ж��Ƿ��г�ͻ����һ�������߶���cell��ʱ�򣬳�ͻ����
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
				// �ж��Ƿ��г�ͻ
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

	// ���Ѿ��������fhs������ʣ�»�û�д������fhs
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
							// ע�����ﴦ��ļ���
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


// �õ������Խ���quad set �Խ�����ָ��inflation��cells 
static bool get_direct_cells_for_self_cross_part(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set,
												 std::vector<std::unordered_set<OvmFaH> > &bound_fhs_groups,
												 std::vector<std::unordered_set<OvmFaH> > &around_fhs_groups,
												 std::unordered_set<OvmVeH> &directed_vhs,
												 std::unordered_set<OvmFaH> &visited_fhs)
{
	// Lambda function
	// �õ�int_ehs(quad sets�ཻ�����Ľ��߱߱߼�) 
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
	// ����һ��ʮ�ֽ����ߣ�������Ӧָ���伸�ι�����cells
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
		
		// ���ҵ�һ���㸽����ָ��cells
		bool flag = false; // ��ʾָ����һ�����cells��û���ҵ�
		std::unordered_set<OvmCeH> critical_cells; // ��ʾ����ȷ���������ĸ�cells_group�е�һ����Щcells
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
		// �ٴ����������չ�����ϵ��ҵ�ָ���������cells
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
			std::unordered_set<OvmCeH> that_cells; // ����������critical_cells
			get_cell_groups_around_vertex(mesh, vh, quad_set, cells_groups);
		
			// ��������critical_cells
			foreach (auto &cells, cells_groups) {
				// �ҵ�������critical_cells, ������صĴ���
				if (intersects(cells, critical_cells)) {
					that_cells = cells;
					critical_cells.insert(cells.begin(), cells.end());
					break;
				}
			}

			//-----------------------------------------------
			// �������Lambda������ؼ���һ��
			// ��ָ���������Ӧ��cells�����������shrink set
			// ��������around_fhs
			//------------------------------------------------
			
			// ��������around_fhs 
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
			// ��չһ��around_fhs
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

			// ���shrink set
			// ���that_cells��shrink set
			std::unordered_set<OvmCeH> chs_add_to_shrink_set;
			chs_add_to_shrink_set.insert(that_cells.begin(), that_cells.end());
			
			// ���that_cells������������չ��cells
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

			// ���ʮ�ֽ����������ߵ�cells
			foreach (auto cells, cells_groups) {
				if (cells == that_cells) continue;
				std::unordered_set<OvmFaH> tmp_fhs = fGetAdjFacesAroundCells(cells);
				if (intersects(tmp_fhs, fhs_for_extend)) {
					chs_add_to_shrink_set.insert(cells.begin(), cells.end());
				}
			}

			// ���ҵ���ָ����cells����shrink set
			// ������directed_vhs
			shrink_set.insert(chs_add_to_shrink_set.begin(), chs_add_to_shrink_set.end());
			foreach (auto ch, chs_add_to_shrink_set) {
				auto vhs_vec = get_adj_vertices_around_cell(mesh, ch);
				directed_vhs.insert(vhs_vec.begin(), vhs_vec.end());
			}

			// ������ؼ���һ��
			//-------------------------------------------------

			// ����һ����
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

		// ��bound_fhs
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


// �õ�����quad set��ֻ����һ������inflation��ָ��cells
static bool get_direct_cells_for_one_inflation_vhs(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set,
												   std::vector<std::unordered_set<OvmFaH> > &bound_fhs_groups,
												   std::vector<std::unordered_set<OvmFaH> > &around_fhs_groups,
												   std::unordered_set<OvmVeH> &directed_vhs,
												   std::unordered_set<OvmFaH> &visited_fhs)
{
	std::unordered_set<OvmVeH> bound_vhs_on_quad_set;

	// Lambda function:
	// ���ܱ�2016-2017�괺���ڶ����ܱ�����
	// bound_fhs��������еĵ㣬����û��cell��ָ���伸�ι�����
	// ����Ҫ��bound_fhs�ϵĵ����cells��ָ���伸�ι���
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
	// �жϱ߽��ϵĵ��Ƿ�ֻ����һ������inflation��������ҳ�ָ�������inflation��cells
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
	// ����ֻ����һ������inflation�ĵ㣬�������㼯���������Χ�ĵ�Ҳֻ����һ������inflation��ô���ϵ���չ����㼯
	// ������Щ������Ӧcells����shrink set��
	// �ı���:
	// directed_vhs
	// shrink_set
	// bound_fhs_groups
	// around_fhs_groups
	//
	auto fOneDirectionInflationVhsExtend = [&](OvmVeH vh, std::unordered_set<OvmCeH> direct_cells, std::unordered_set<OvmFaH> &quad_set){
		// ��ǵ㼰�����Ѿ���cellsָ������
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

			// �ҵ� next_around_fhs
			std::unordered_set<OvmVeH> tmp_vhs;
			foreach (auto &fh, around_fhs) {
				one_around_fhs.insert(fh);
				std::vector<OvmVeH> adj_vhs = get_adj_vertices_around_face(mesh, fh);
				std::for_each(adj_vhs.begin(), adj_vhs.end(), 
							  [&](OvmVeH vh) { tmp_vhs.insert(vh); });
			}
			// ͨ���м�vh���� next_around_fhs
			foreach (auto &vh, tmp_vhs) {
				// �����ʵ�ֻ��ǱȽ��м��ɣ���Ϊ����ֻ��Ҫ��ע��ģ�ͱ���߽��ϵĵ���cells ��ָ��
				if (!mesh->is_boundary(vh)) continue; 

				std::unordered_set<OvmFaH> tmp_fhs;
				get_adj_faces_around_vertex(mesh, vh, tmp_fhs);
				
				foreach (auto &fh, tmp_fhs) {
					if (contains(visited_fhs, fh) || !contains(quad_set, fh)) continue;
					// (1) �ж�������Ƿ��ڱ߽��ϣ�incident��cell�ǲ��Ǻ�shrink set����
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
					
					// (2) �ж������Ƿ�ͼ��α�����
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

		// ����һȦ�߽����
		// ͨ�����visited_fhs
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
	// ����ÿһ��bound_fhs
	foreach (auto bound_fhs, bound_fhs_groups) {

		// ���Ѿ��������fhs������ʣ�»�û�д������fhs
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
								// ע�����ﴦ��ļ���
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