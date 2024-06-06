#ifndef _FUNC_DEFS_H_
#define _FUNC_DEFS_H_

#include <string>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <unordered_set>
#include <vector>
#include <list>
#include <algorithm>
#include <hash_map>
#include <queue>



#include <QDomDocument>
#include <QDomElement>
#include <QSet>
#include <QHash>
#include <QMap>

#include "acis_headers.h"
#include "MeshDefs.h"
#include "DualDefs.h"

class HoopsView;
#include "hoopsview.h"
//////////////////////////////////////////////////////////////////////////

bool parse_xml_file (QString xml_path, QString &file_type, QString &data_name, 
	std::vector<std::pair<QString, QString> > &path_pairs);
VolumeMesh* load_volume_mesh (QString mesh_path);
VMesh* load_tet_mesh(QString mesh_path);
VolumeMesh* tet2hex(VMesh* tetmesh);
VolumeMesh* tet2hex(VMesh* tetmesh, BODY* body);

std::vector<OvmVeH> get_ordered_vhs(VolumeMesh* mesh, OvmCeH ch);

void Get_Feature_Infos(VolumeMesh* mesh, std::vector<OvmVeH>& corners, std::vector<OvmEgH>& edges);

void Get_Block_Infos(VolumeMesh* mesh, std::unordered_set<OvmCeH> block, std::unordered_set<OvmVeH>& corner_vhs, 
	std::unordered_set<OvmEgH>& feature_edges, std::vector<std::unordered_set<OvmVeH>>& boundary_patch_vhs);

std::vector<std::unordered_set<OvmVeH>> Get_Block_Boundary_Vhs (VolumeMesh* mesh, std::unordered_set<OvmCeH> block);

std::vector<OvmHaEgH> Get_Topology_Next_Hehs(VolumeMesh* mesh, OvmHaEgH heh);

void Get_Base_Complex(VolumeMesh* mesh, std::vector<std::unordered_set<OvmCeH>>& blocks, 
	std::vector<std::unordered_set<OvmVeH>>& block_boundary_vhs, std::vector<std::unordered_set<OvmVeH>>& block_inner_vhs);

void Refine_Hex(VolumeMesh* mesh, BODY *body, int segment = 4);

//VolumeMesh* tet2hex(VMesh* tetmesh, std::hash_map<OvmVeH, OvmVeH>& tet2hex_vh_mapping, std::hash_map<OvmCeH, OvmCeH>& tet2hex_ch_mapping, std::hash_map<OvmVeH, OvmVeH>& hex2tet_vh_mapping, std::hash_map<OvmCeH, OvmCeH>& hex2tet_ch_mapping);
//四面体分裂的函数暂时不需要，且没有写对
//VMesh* tetspliting(VMesh* tetmesh);
//获取一个四面体单元某一条边的对边
OvmEgH tet_opposite_edge(VMesh* mesh, OvmCeH ch, OvmEgH eh);
//四面体局部分裂，通过边来分裂
void tet_edge_splitting(VMesh* mesh,OvmEgH eh, std::vector<OvmCeH>& _chs);
//四面体局部分裂，同时伴有新到旧OvmCeH之间的映射
void tet_edge_splitting_with_mapping(VMesh* mesh, OvmEgH eh, std::hash_map<OvmCeH, OvmCeH>& new2oldchmapping);
//四面体局部边分裂，分裂点是给定的位置，同时给出新旧OvmCeH之间的映射
OvmVeH tet_edge_splitting_with_fix_point_and_mapping(VMesh* mesh, OvmEgH eh, OvmVec3d p_pos, std::hash_map<OvmCeH, OvmCeH>& new2oldchmapping);
//四面体局部分裂，通过面来分裂
void tet_face_splitting(VMesh* mesh,OvmFaH fh, std::vector<OvmCeH>& _chs);
//四面体局部分裂，通过面分裂，面分裂的中心点给定，返回增加的新的OvmVeH 且给出新旧OvmCeH之间的映射
OvmVeH tet_face_splitting_with_fix_point_and_mapping (VMesh* mesh, OvmFaH fh, OvmVec3d p_pos, std::hash_map<OvmCeH, OvmCeH>& new2oldchmapping);

OP_Mesh* create_boundary_mesh(VolumeMesh* mesh);
OP_Mesh* get_boundary_mesh(VMesh* mesh);
OP_Mesh* get_boundary_mesh(VMesh* mesh, std::hash_map<size_t,OvmCeH>& f2cellmapping, std::hash_map<OvmCeH, size_t>& bcell2fmapping);
OP_Mesh* get_boundary_mesh(VMesh* mesh, std::hash_map<size_t,OvmCeH>& f2cellmapping, std::hash_map<OvmCeH, size_t>& bcell2fmapping, std::hash_map<size_t, OvmVeH>& omv2ovmvhmapping);
//获取六面体网格单元的边界四边形网格 可能一个边界cell会映射到两个边界面上 所以第三个参数可能会比第二个参数小
OP_Mesh* get_boundary_mesh(VolumeMesh* mesh, std::hash_map<size_t, OvmCeH>& f2cellmapping, std::hash_map<OvmCeH, size_t>& bcell2fmapping, std::hash_map<size_t, OvmVeH>& omv2ovmvhmapping, std::hash_map<OvmVeH, size_t>& ovmvh2omvhmapping);
//获取六面体网格的边界四边形网格 一个边界体可能可以有多个面对应
OP_Mesh* get_boundary_mesh(VolumeMesh* mesh, std::hash_map<size_t, OvmCeH>& f2cellmapping, std::hash_map<OvmCeH, std::vector<size_t>>& bcell2fmapping, std::hash_map<size_t, OvmVeH>& omv2ovmvhmapping, std::hash_map<OvmVeH, size_t>& ovmvh2omvhmapping);

void init_volume_mesh (VolumeMesh *mesh, BODY *body, double myresabs);
void init_tet_mesh (VMesh *mesh, BODY *body, double myresabs);
bool save_volume_mesh (VolumeMesh *mesh, QString mesh_path);
BODY* load_acis_model (QString file_path);
void save_acis_entity(ENTITY *entity, const char * file_name);


void attach_mesh_elements_to_ACIS_entities (VolumeMesh *mesh, BODY *body, double myresabs = SPAresabs * 1000);
void attach_mesh_elements_to_ACIS_entities_with_repair (VolumeMesh *mesh, BODY *body, double myresabs = SPAresabs * 1000);
void attach_mesh_elements_to_ACIS_entities_with_feature_repair(VolumeMesh *mesh, BODY *body, HoopsView* hoopsview, double myresabs = SPAresabs * 1000);
void attach_mesh_elements_to_ACIS_entities_for_SW_Level1(VolumeMesh *mesh, BODY *body, HoopsView* hoopsview, double myresabs = SPAresabs * 1000);
void attach_tet_mesh_elements_to_ACIS_entities (VMesh *mesh, BODY *body, double myresabs = SPAresabs * 1000);
void attach_tet_mesh_elements_to_ACIS_entities_with_repair (VMesh *mesh, BODY *body, double myresabs = SPAresabs * 1000);
void attach_tet_mesh_elements_to_ACIS_entities_with_repair (VMesh *mesh, BODY *body, HoopsView *hoopsview, double myresabs = SPAresabs * 1000);
void attach_mesh_elements_to_ACIS_entities_of_tre2hex (VolumeMesh *mesh, BODY *body, double myresabs= SPAresabs * 1000);

FACE * get_associated_geometry_face_of_boundary_fh (VolumeMesh *mesh, OvmFaH fh);
FACE * get_associated_geometry_face_of_boundary_fh_gen_ver (VMesh *mesh, OvmFaH fh);
FACE * get_associated_geometry_face_of_boundary_fh_gen_ver (VMesh *mesh, OvmFaH fh);
EDGE * get_associated_geometry_edge_of_boundary_eh (VolumeMesh *mesh, OvmEgH eh, OpenVolumeMesh::VertexPropertyT<unsigned long> &V_ENTITY_PTR);
EDGE * get_associated_geometry_edge_of_boundary_eh (VolumeMesh *mesh, OvmEgH eh);
EDGE * get_associated_geometry_edge_of_boundary_eh_gen_ver (VMesh *mesh, OvmEgH eh, OpenVolumeMesh::VertexPropertyT<unsigned long> &V_ENTITY_PTR);
EDGE * get_associated_geometry_edge_of_boundary_eh_gen_ver (VMesh *mesh, OvmEgH eh);

void get_fhs_on_acis_face (VolumeMesh *mesh, FACE *acis_face, std::unordered_set<OvmFaH> &fhs);
//判断一个网格边是否在几何面上
bool is_eh_belong_to_FACE(VolumeMesh * mesh,OvmEgH eh);

bool is_eh_belong_to_FACE_gen_ver(VMesh * mesh,OvmEgH eh);

//判断一个cell是否在边界上,若cell有面位于几何面上，则该cell为边界cell
bool is_boundary_cell(VolumeMesh *mesh, OvmCeH ch);
//////////////////////////////////////////////////////////////////////////
//hexamesh toplogy functions
std::vector<OvmFaH> get_adj_faces_around_edge (VolumeMesh *mesh, OvmEgH eh, bool on_boundary = false);
std::vector<OvmFaH> get_adj_faces_around_edge (VolumeMesh *mesh, OvmHaEgH heh, bool on_boundary = false);
void get_adj_faces_around_edge (VolumeMesh *mesh, OvmEgH eh, std::unordered_set<OvmFaH> &faces, bool on_boundary = false);
void get_adj_faces_around_edge (VolumeMesh *mesh, OvmHaEgH heh, std::unordered_set<OvmFaH> &faces, bool on_boundary = false);

void get_adj_vertices_around_vertex (VolumeMesh *mesh, OvmVeH vh, std::unordered_set<OvmVeH> &vertices);
void get_adj_vertices_around_face (VolumeMesh *mesh, OvmFaH fh, std::vector<OvmVeH> &vertices);
std::vector<OvmVeH> get_adj_vertices_around_face (VolumeMesh *mesh, OvmFaH fh);
void get_adj_vertices_around_cell (VolumeMesh *mesh, OvmCeH ch, std::unordered_set<OvmVeH> &vertices);
void get_adj_vertices_around_tet (VMesh *mesh, OvmCeH ch,std::unordered_set<OvmVeH> & vertices);
std::vector<OvmVeH> get_adj_vertices_around_cell (VolumeMesh *mesh, OvmCeH ch);
std::vector<OvmVeH> get_adj_vertices_around_hexa (VolumeMesh *mesh, OvmCeH ch);
void get_adj_boundary_vertices_around_vertex (VolumeMesh *mesh, OvmVeH vh, std::unordered_set<OvmVeH> &vertices);
void get_adj_edges_around_face (VolumeMesh *mesh, OvmFaH fh, std::unordered_set<OvmEgH> &edges);
void get_adj_edges_around_face_tetmesh (VMesh *mesh, OvmFaH fh, std::unordered_set<OvmEgH> &edges);
void get_adj_hexas_around_edge (VolumeMesh *mesh, OvmHaEgH heh, std::unordered_set<OvmCeH> &hexas);
void get_adj_tets_around_edge(VMesh* mesh,OvmHaEgH heh,std::unordered_set<OvmCeH> &tets);
void get_adj_hexas_around_edge (VolumeMesh *mesh, OvmEgH eh, std::unordered_set<OvmCeH> &hexas);
void get_adj_hexas_around_hexa (VolumeMesh *mesh, OvmCeH ch, std::unordered_set<OvmCeH> &hexas);
void get_adj_faces_around_hexa (VolumeMesh *mesh, OvmCeH ch, std::unordered_set<OvmFaH> &faces);
void get_adj_hexas_around_vertex (VolumeMesh *mesh, OvmVeH vh, std::unordered_set<OvmCeH> &hexas);
void get_adj_faces_around_vertex (VolumeMesh *mesh, OvmVeH vh, std::unordered_set<OvmFaH> &faces);
void get_adj_edges_around_vertex (VolumeMesh *mesh, OvmVeH vh, std::unordered_set<OvmEgH> &edges);
void get_adj_boundary_faces_around_vertex (VolumeMesh *mesh, OvmVeH vh, std::unordered_set<OvmFaH> &faces);
void get_adj_boundary_faces_around_edge (VolumeMesh *mesh, OvmEgH eh, std::unordered_set<OvmFaH> &faces);
void get_adj_boundary_faces_around_face (VolumeMesh *mesh, OvmFaH fh, std::unordered_set<OvmFaH> &faces);

void get_adj_edges_around_cell(VolumeMesh *mesh, OvmCeH ch, std::unordered_set<OvmEgH> & ehs_on_ch);

void get_adj_edges_around_tet(VMesh *mesh, OvmCeH ch, std::unordered_set<OvmEgH> &ehs_on_tet);
void get_cell_groups_around_vertex (VolumeMesh *mesh, OvmVeH vh, std::unordered_set<OvmFaH> &fhs,
	std::vector<std::unordered_set<OvmCeH> > &cell_groups);

void get_direct_adjacent_hexas (VolumeMesh *mesh, const std::unordered_set<OvmFaH> &patch, std::unordered_set<OvmCeH> &hexas);
void collect_boundary_element (VolumeMesh *mesh, std::set<OvmCeH> &chs, 
	std::set<OvmVeH> *bound_vhs, std::set<OvmEgH> *bound_ehs, std::set<OvmHaFaH> *bound_hfhs);
void collect_boundary_element (VolumeMesh *mesh, std::set<OvmCeH> &chs, 
	std::set<OvmVeH> *bound_vhs, std::set<OvmEgH> *bound_ehs, std::set<OvmFaH> *bound_fhs);
void collect_boundary_element (VolumeMesh *mesh, std::unordered_set<OvmCeH> &chs, 
	std::unordered_set<OvmVeH> *bound_vhs, std::unordered_set<OvmEgH> *bound_ehs, std::unordered_set<OvmHaFaH> *bound_hfhs);
void collect_boundary_element (VolumeMesh *mesh, std::unordered_set<OvmCeH> &chs, 
	std::unordered_set<OvmVeH> *bound_vhs, std::unordered_set<OvmEgH> *bound_ehs, std::unordered_set<OvmFaH> *bound_fhs);
//可以处理出现n条环的情况
void collect_boundary_element (VolumeMesh *mesh, std::unordered_set<OvmHaFaH> &hfhs, 
	std::vector<OvmVeH> & bound_vhs, std::vector<std::vector<OvmHaEgH>> & bound_hehs);
//处理一条环的情况
void collect_boundary_element (VolumeMesh *mesh, std::unordered_set<OvmHaFaH> &hfhs, 
	std::vector<OvmVeH> & bound_vhs, std::vector<OvmHaEgH> & bound_hehs);
//找一组连通面的边界线
void collect_boundary_element (VolumeMesh *mesh, std::unordered_set<OvmFaH> &fhs, 
	std::unordered_set<OvmVeH>  &bound_vhs, std::unordered_set<OvmEgH> &bound_hehs);

void get_ccw_boundary_edges_faces_around_vertex (VolumeMesh *mesh, OvmVeH vh,
	std::vector<OvmEgH> &ehs, std::vector<OvmFaH> &fhs);
void get_ccw_boundary_edges_faces_around_vertex (VolumeMesh *mesh, OvmVeH vh,
	std::unordered_set<OvmEgH> &key_ehs, std::vector<OvmEgH> &ehs, std::vector<std::unordered_set<OvmFaH> > &fhs_sets);
void get_ccw_boundary_edges_faces_around_vertex (VolumeMesh *mesh, OvmVeH vh,
	std::vector<OvmEgH> &key_ehs, std::vector<OvmEgH> &ehs, std::vector<std::unordered_set<OvmFaH> > &fhs_sets);

void get_cw_boundary_edges_faces_around_vertex (VolumeMesh *mesh, OvmVeH vh,
	std::vector<OvmEgH> &ehs, std::vector<OvmFaH> &fhs);
void get_cw_boundary_edges_faces_around_vertex (VolumeMesh *mesh, OvmVeH vh, 
	std::unordered_set<OvmEgH> &key_ehs, std::vector<OvmEgH> &ehs, std::vector<std::unordered_set<OvmFaH> > &fhs_sets);
void get_cw_boundary_edges_faces_around_vertex (VolumeMesh *mesh, OvmVeH vh, 
	std::vector<OvmEgH> &key_ehs, std::vector<OvmEgH> &ehs, std::vector<std::unordered_set<OvmFaH> > &fhs_sets);

OvmVeH get_other_vertex_on_edge (VolumeMesh *mesh, OvmEgH eh, OvmVeH vh);
OvmEgH get_opposite_edge_on_face (VolumeMesh *mesh, OvmFaH fh, OvmEgH eh);

OvmFaH get_common_face_handle (VolumeMesh *mesh, OvmCeH &ch1, OvmCeH &ch2);
OvmFaH get_common_face_handle_gen_ver (VMesh *mesh, OvmCeH &ch1, OvmCeH &ch2);
OvmVeH get_common_vertex_handle (VolumeMesh *mesh, OvmEgH eh1, OvmEgH eh2);
OvmVeH get_common_vertex_handle_gen_ver (VMesh *mesh, OvmEgH eh1, OvmEgH eh2);
OvmEgH get_common_edge_handle (VolumeMesh *mesh, OvmFaH fh1, OvmFaH fh2);
OvmEgH get_common_tet_edge_handle(VMesh *mesh, OvmFaH fh1, OvmFaH fh2);
OvmFaH get_common_face_handle (VolumeMesh *mesh, OvmEgH &eh1, OvmEgH &eh2);
OvmCeH get_common_cell_handle (VolumeMesh *mesh, OvmFaH & fh1, OvmFaH &fh2);

OvmVec3d get_tet_center(VMesh* mesh,OvmCeH ch);


bool is_manifold (VolumeMesh *mesh, std::unordered_set<OvmFaH> &fhs);
bool is_manifold (VolumeMesh *mesh, std::unordered_set<OvmCeH> &chs);
//////////////////////////////////////////////////////////////////////////
//render
HC_KEY insert_boundary_shell (VolumeMesh *mesh);
HC_KEY insert_boundary_shell (VolumeMesh *mesh, std::unordered_set<OvmCeH> &chs);
void render_volume_mesh (VolumeMesh *mesh);
void render_volume_mesh_with_no_vertices (VolumeMesh *mesh);
void render_tet_mesh(VMesh *mesh);
void render_triangle_mesh (OP_Mesh *mesh);
void render_mesh_group (VolumeMeshElementGroup *group);

//给定一个网格和body 输入一条网格边OvmEgH返回这条边中点在这个模型上的落点
OvmVec3d find_ovmegh_mid_pos_in_body(VMesh* mesh, BODY* body, OvmEgH eh);


//由一组有顺序的边确定出一组首尾相连的半边
void get_piecewise_halfedges_from_edges (VolumeMesh *mesh, std::vector<OvmEgH> &ehs, bool forward, std::vector<OvmHaEgH> &hehs);

std::vector<OvmVeH> get_ordered_vhs_from_unordered_ehs_with_start_direction(VMesh *mesh, std::vector<OvmEgH> &ehs, OvmEgH start_eh, OvmVeH start_vh);

std::vector<OvmFaH> get_adj_faces_around_cell (VolumeMesh *mesh, OvmCeH ch);
//
std::vector<OvmHaEgH> get_ordered_hehs_from_ehs(VolumeMesh* mesh, std::vector<OvmEgH> ehs);


//mesh converter
void fOvm2VTK(VMesh* mesh, std::string outfile_name);





#endif