#include "StdAfx.h"
#include "MyMeshManager.h"

void MyMeshManager::MyFacet(ENTITY * ent, std::vector<SPAposition>& out_mesh_points, std::vector<SPAunit_vector>& out_mesh_normals, std::vector<ENTITY*>& out_faces)
{
	api_initialize_faceter();

	REFINEMENT *ref = new REFINEMENT();
	ref->set_surf_mode(AF_SURF_ALL);
	ref->set_adjust_mode(AF_ADJUST_NONE);
	ref->set_triang_mode(AF_TRIANG_ALL);
	ref->set_surface_tol(0.001);
	api_set_default_refinement(ref);

	parameter_token ptoken[2];
	ptoken[0] = POSITION_TOKEN;
	ptoken[1] = NORMAL_TOKEN;
	VERTEX_TEMPLATE *v1 = new VERTEX_TEMPLATE(2, ptoken);
	api_set_default_vertex_template(v1);

	MY_MESH_MANAGER *mymm = new MY_MESH_MANAGER();
	api_set_mesh_manager(mymm);

	api_facet_entity(ent);

	out_mesh_points = mymm->GetPoints();
	out_mesh_normals = mymm->GetNormals();
	out_faces = mymm->GetFaces();

	api_terminate_faceter();
}

void MyMeshManager::MyFacetEntityList(ENTITY* owner, ENTITY_LIST& entity_list, std::vector<SPAposition> &out_mesh_points, std::vector<SPAunit_vector> &out_mesh_normals, std::vector<ENTITY*> & out_faces)
{
	api_initialize_faceter();

	REFINEMENT *ref = new REFINEMENT();
	ref->set_surf_mode(AF_SURF_ALL);
	ref->set_adjust_mode(AF_ADJUST_NONE);
	ref->set_triang_mode(AF_TRIANG_ALL);
	ref->set_surface_tol(0.001);
	api_set_default_refinement(ref);

	parameter_token ptoken[2];
	ptoken[0] = POSITION_TOKEN;
	ptoken[1] = NORMAL_TOKEN;
	VERTEX_TEMPLATE *v1 = new VERTEX_TEMPLATE(2, ptoken);
	api_set_default_vertex_template(v1);

	MY_MESH_MANAGER *mymm = new MY_MESH_MANAGER();
	api_set_mesh_manager(mymm);

	api_facet_entities(owner, &entity_list);

	out_mesh_points = mymm->GetPoints();
	out_mesh_normals = mymm->GetNormals();
	out_faces = mymm->GetFaces();

	api_terminate_faceter();
}
