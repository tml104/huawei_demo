#pragma once

// ACIS include
#include "ACISincluded.h"

// Project include
#ifndef IN_HUAWEI
#include "logger44/CoreOld.h"
#else
#include "CoreOld.h"
#endif
#include "MarkNum.h"
#include "MyConstant.h"
//#include "UtilityFunctions.h"

// STL
#include <time.h>
#include <fstream>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <functional>
#include <string>
#include <ctime>
#include <cmath>

namespace MyMeshManager {

	class MY_MESH_MANAGER : public POLYGON_POINT_MESH_MANAGER {
	private:
		std::vector<SPAposition> points;
		std::vector<SPAunit_vector> normals;
		std::vector<ENTITY*> faces;
		ENTITY* nowface;

		int face_num;

	public :

		MY_MESH_MANAGER() : POLYGON_POINT_MESH_MANAGER() {
			face_num = 0;
			nowface = nullptr;
		}

		virtual logical
			need_global_indexed_polygons(void) {

			return TRUE;
		}

		virtual logical
			need_precount_of_global_indexed_polygons(void) {

			return TRUE;
		}

		virtual logical
			need_coedge_pointers_on_polyedges() {

			return TRUE;
		}

		virtual logical
			need_indexed_polynode_with_data() {

			return TRUE;
		}

		virtual logical
			need_edge_indices() {

			return TRUE;
		}

		virtual void*
			null_node_id(void) {

			return (void *)-1;
		}

		virtual void*
			announce_global_node(int inode, VERTEX *ver, const SPAposition &Xi)
		{

			return((void*)inode);
		}

		virtual void*
			announce_global_node(int inode, EDGE* mod_edge, const SPAposition &Xi,
				double t)
		{

			return((void*)inode);
		}

		virtual void*
			announce_global_node(int inode, FACE* mod_face, const SPAposition &Xi,
				const SPApar_pos &uv)
		{

			return((void*)inode);
		}

		virtual void
			announce_polygon_model_face(ENTITY* ent)
		{
			nowface = ent;
			face_num++;
		}

		virtual void announce_indexed_polynode(ENTITY* ent, int ipoly, int i, void* idptr,
			const double &edge_tpar, const SPApar_pos &uv,
			const SPAposition &pos,
			const SPAunit_vector &uvec) {

			points.emplace_back(pos);
			normals.emplace_back(uvec);
			faces.emplace_back(nowface);
		}

		std::vector<SPAposition> GetPoints() {
			return this->points;
		}

		std::vector<SPAunit_vector> GetNormals() {
			return this->normals;
		}

		std::vector<ENTITY*> GetFaces() {
			return this->faces;
		}
	};

	void MyFacet(ENTITY* ent, std::vector<SPAposition> &out_mesh_points, std::vector<SPAunit_vector> &out_mesh_normals, std::vector<ENTITY*> & out_faces);

	void MyFacetEntityList(ENTITY* owner, ENTITY_LIST& entity_list, std::vector<SPAposition> &out_mesh_points, std::vector<SPAunit_vector> &out_mesh_normals, std::vector<ENTITY*> & out_faces);
} // namespace MyMeshManager 