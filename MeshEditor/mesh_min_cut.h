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
//       min_cut基本会用get_volume_mesh_min_cut_new_version或者
//get_volume_mesh_min_cut_considering_area，后者多考虑了面的面积，面积权重为alpha
//         输入是dual loops对边界进行分类的主要函数是
//		get_boundary_source_and_target 可能考虑的情况不足 
//			但比以前的版本考虑都多了 效率也相对高些
//////////////////////////////////////////////////////////////////////////



//
////定义一些undirected_graph 目前权重是用的int型 可能以后会改变
//typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
//	boost::no_property, boost::property<boost::edge_weight_t, int> > undirected_graph;
//typedef boost::property_map<undirected_graph, boost::edge_weight_t>::type weight_map_type;
//typedef boost::property_traits<weight_map_type>::value_type weight_type;
//typedef boost::graph_traits < undirected_graph >::vertex_descriptor vertexdescriptor;  
//typedef boost::graph_traits < undirected_graph >::edge_descriptor edgedescriptor;  

//保留原来版本 新的版本采用Isap的最大流最小割算法实现 三个函数加上新版本后缀加上
//new_version以区分

std::unordered_set<OvmFaH> get_volume_mesh_min_cut (VolumeMesh *mesh, 
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs);


//////////////////////////////////////////////////////////////////////////
//新版的min cut算法 速度目前已有最快 但是只考虑了从表面出发的第几层
//网络的边权重是int型的
std::unordered_set<OvmFaH> get_volume_mesh_min_cut_new_version (VolumeMesh *mesh, 
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs);

//相对于前一个版本考虑了面积  其实还可以加一些其他能量进去 目前只有考虑面积 
//比前面版本耗时原因在于还需要计算面积（且可能是重复的）
std::unordered_set<OvmFaH> get_volume_mesh_min_cut_considering_area (VolumeMesh *mesh, 
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs);


//使用Boost的最小割算法  效率太低 可能是实现原因 所以不采用
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


//目前碰到一种特殊情况 如果输入是两条loops可能会将表面分成三块 其中两块合为一块 
//剩余情况都是将表面分成s t两种情况
//实际可能会出现更多复杂的情况目前 只处理两条loop将表面分成三块 其中两块一起的情况
void get_boundary_source_and_target (VolumeMesh* mesh, std::vector<std::vector<OvmHaEgH>> loops,std::unordered_set<OvmFaH>& fhs,
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs);

void get_boundary_source_and_target (VolumeMesh* mesh, std::vector<OvmEgH> ehs, DualSheet* sheet, std::unordered_set<OvmCeH>& s_chs, std::unordered_set<OvmCeH>& t_chs, HoopsView* hoopsview = NULL);


std::unordered_set<OvmFaH> get_volume_mesh_min_cut_considering_area_and_sheet (VolumeMesh *mesh, 
	std::unordered_set<OvmCeH> &s_chs, std::unordered_set<OvmCeH> &t_chs, std::unordered_set<OvmCeH> excluded_cells);