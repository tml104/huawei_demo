#pragma once

#include "MeshDefs.h"
#include "DualDefs.h"
#include "FuncDefs.h"

#include <set>
#include <map>
#include <deque>
#include <tuple>
//#include <boost/graph/adjacency_list.hpp>
//#include <boost/graph/graph_traits.hpp>
//#include <boost/graph/one_bit_color_map.hpp>
//#include <boost/graph/stoer_wagner_min_cut.hpp>
//#include <boost/property_map/property_map.hpp>
//#include <boost/typeof/typeof.hpp>


//////////////////////////////////////////////////////////////////////////
//       min_cut��������get_volume_mesh_min_cut_new_version����
//get_volume_mesh_min_cut_considering_area�����߶࿼���������������Ȩ��Ϊalpha
//         ������dual loops�Ա߽���з������Ҫ������
//		get_boundary_source_and_target ���ܿ��ǵ�������� 
//			������ǰ�İ汾���Ƕ����� Ч��Ҳ��Ը�Щ
//////////////////////////////////////////////////////////////////////////



//
////����һЩundirected_graph ĿǰȨ�����õ�int�� �����Ժ��ı�
//typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
//	boost::no_property, boost::property<boost::edge_weight_t, int> > undirected_graph;
//typedef boost::property_map<undirected_graph, boost::edge_weight_t>::type weight_map_type;
//typedef boost::property_traits<weight_map_type>::value_type weight_type;
//typedef boost::graph_traits < undirected_graph >::vertex_descriptor vertexdescriptor;  
//typedef boost::graph_traits < undirected_graph >::edge_descriptor edgedescriptor;  

//����ԭ���汾 �µİ汾����Isap���������С���㷨ʵ�� �������������°汾��׺����
//new_version������

std::unordered_set<OvmFaH> get_volume_mesh_min_cut (VolumeMesh *mesh, 
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs);


//////////////////////////////////////////////////////////////////////////
//�°��min cut�㷨 �ٶ�Ŀǰ������� ����ֻ�����˴ӱ�������ĵڼ���
//����ı�Ȩ����int�͵�
std::unordered_set<OvmFaH> get_volume_mesh_min_cut_new_version (VolumeMesh *mesh, 
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs);

//�����ǰһ���汾���������  ��ʵ�����Լ�һЩ����������ȥ Ŀǰֻ�п������ 
//��ǰ��汾��ʱԭ�����ڻ���Ҫ����������ҿ������ظ��ģ�
std::unordered_set<OvmFaH> get_volume_mesh_min_cut_considering_area (VolumeMesh *mesh, 
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs);


//ʹ��Boost����С���㷨  Ч��̫�� ������ʵ��ԭ�� ���Բ�����
//std::unordered_set<OvmFaH> get_volume_mesh_min_cut_boost_version (VolumeMesh *mesh, 
//	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs);

//////////////////////////////////////////////////////////////////////////

std::unordered_set<OvmEgH> get_quad_mesh_min_cut (VolumeMesh *mesh, 
	std::unordered_set<OvmFaH> &s_fhs, std::unordered_set<OvmFaH> &t_fhs,
	std::vector<std::unordered_set<OvmFaH> > &obstacles);

std::unordered_set<OvmEgH> get_quad_mesh_min_cut_new_version (VolumeMesh *mesh, 
	std::unordered_set<OvmFaH> &s_fhs, std::unordered_set<OvmFaH> &t_fhs,
	std::vector<std::unordered_set<OvmFaH> > &obstacles);

std::unordered_set<OvmEgH> get_quad_mesh_min_cut_considering_area(VolumeMesh *mesh,
	std::unordered_set<OvmFaH> &s_fhs, std::unordered_set<OvmFaH> &t_fhs,
	std::vector<std::unordered_set<OvmFaH> > &obstacles);


std::unordered_set<OvmFaH> get_local_mesh_min_cut (VolumeMesh *mesh, 
	std::unordered_set<OvmCeH> &all_chs, std::unordered_set<OvmCeH> &s_chs, 
	std::unordered_set<OvmCeH> &t_chs);

std::unordered_set<OvmFaH> get_local_mesh_min_cut_new_version (VolumeMesh *mesh, 
	std::unordered_set<OvmCeH> &all_chs, std::unordered_set<OvmCeH> &s_chs, 
	std::unordered_set<OvmCeH> &t_chs);

void get_source_and_target(VolumeMesh* mesh, std::vector<OvmHaEgH> one_loop,std::unordered_set<OvmEgH> all_loops, 
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs);

void get_source_and_target_SC(VolumeMesh* mesh, std::vector<std::vector<OvmHaEgH>> loops,std::unordered_set<OvmFaH>& fhs,
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs);


void get_source_and_target_on_boundary (VolumeMesh* mesh, std::vector<std::vector<OvmHaEgH>> loops,std::unordered_set<OvmFaH>& fhs,
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs);


//Ŀǰ����һ��������� �������������loops���ܻὫ����ֳ����� ���������Ϊһ�� 
//ʣ��������ǽ�����ֳ�s t�������
//ʵ�ʿ��ܻ���ָ��ิ�ӵ����Ŀǰ ֻ��������loop������ֳ����� ��������һ������
void get_boundary_source_and_target (VolumeMesh* mesh, std::vector<std::vector<OvmHaEgH>> loops,std::unordered_set<OvmFaH>& fhs,
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs);

void get_boundary_source_and_target (VolumeMesh* mesh, std::vector<OvmEgH> ehs, DualSheet* sheet, std::unordered_set<OvmCeH>& s_chs, std::unordered_set<OvmCeH>& t_chs, HoopsView* hoopsview = NULL);


std::unordered_set<OvmFaH> get_volume_mesh_min_cut_considering_area_and_sheet (VolumeMesh *mesh, 
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs, std::unordered_set<OvmCeH> excluded_cells);