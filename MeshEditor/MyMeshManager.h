#pragma once

// ACIS include
#include <boolapi.hxx>
#include <curextnd.hxx>
#include <cstrapi.hxx>
#include <face.hxx>
#include <lump.hxx>
#include <shell.hxx>
#include <split_api.hxx>
#include <kernapi.hxx>
#include <intrapi.hxx>
#include <intcucu.hxx>
#include <coedge.hxx>
#include <curdef.hxx>
#include <curve.hxx>
#include <ellipse.hxx>
#include <straight.hxx>
#include <intcurve.hxx>
#include <helix.hxx>
#include <undefc.hxx>
#include <elldef.hxx>
#include <interval.hxx>
#include <base.hxx>
#include <vector_utils.hxx>
#include <vertex.hxx>
#include <sps3crtn.hxx>
#include <sps2crtn.hxx>
#include <intdef.hxx>
#include <bool_api_options.hxx>
#include <loop.hxx>
#include <wire.hxx>
#include <pladef.hxx>
#include <plane.hxx>
#include <condef.hxx>
#include <cone.hxx>
#include <torus.hxx>
#include <tordef.hxx>
#include <attrib.hxx>
#include <af_api.hxx>
#include <attrib.hxx>
#include <geom_utl.hxx>
#include <body.hxx>
#include <point.hxx>
#include <debug.hxx>
#include <eulerapi.hxx>
#include <geometry.hxx>

// Project include
#include "logger44/CoreOld.h"
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
} // namespace MyMeshManager 