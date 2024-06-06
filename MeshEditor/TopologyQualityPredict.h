#ifndef _TOPOLOGY_QUALITY_PREDICT_H_
#define _TOPOLOGY_QUALITY_PREDICT_H_

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


enum EdgeGeomType
{
	INNEREDGE	= 4,
	FACEEDGE	= 3,
	EDGEEDGE	= 2,
	NULLEDGE	= -1
};
struct EdgeAttribute{
	EdgeGeomType edgetype;
	int valence;
	int idealvalence;
};

//Sheet_data存储的当该sheet删除时，合并点组的映射关系vh_unit_set以及合并边组的映射关系merge_edge_unit_set
struct Sheet_data{
	std::hash_map<int,int> vh_unit_set;
	std::hash_map<int,int> merge_edge_unit_set;
};

//网格中每条边的属性，即边的类型、度数以及理想度数
void get_edge_property(VolumeMesh *mesh, std::map<OvmEgH, EdgeAttribute> & edge_property);

//每条sheet删除时，合并点组以及合并边组的信息
void get_sheet_data(VolumeMesh* mesh, DualSheet * sheet, Sheet_data&);

//sheet set删除时，直接计算合并点组以及合并边组的信息
void get_sheet_set_data(VolumeMesh* mesh, std::vector<DualSheet*> sheets, Sheet_data&);

//两组sheet同时删除时，合并点组以及合并边组的信息
void merge_sheet_data(VolumeMesh *mesh,Sheet_data& data1,Sheet_data & data2,Sheet_data & result);

//根据sheet set中的合并点组的信息判断该sheet是否可被删除（几何有效性，网格密度）
//bool is_sheet_can_be_extracted (VolumeMesh *mesh, BODY *body, std::hash_map<int,int> &vh_union_set,double& density_quality);

//根据sheet set中的合并点组的信息判断该组sheet是否可被删除，并且计算删除后网格密度
bool is_sheets_can_be_extracted (VolumeMesh *mesh, BODY *body, std::hash_map<int,int> &vh_union_set,double& density_quality);

//根据sheet set中合并边组的信息判断该组sheet删除后对网格质量的影响
int topology_value_change_after_merge(VolumeMesh * mesh, std::vector<DualSheet*>&sheets,std::hash_map<int,int> &merge_edge_union_set, std::map<OvmEgH, EdgeAttribute> & edge_property);

//预判sheet删除后对网格密度的影响
bool is_sheet_can_be_extract_based_density(VolumeMesh *mesh, DualSheet* sheet);

//选择最优的待删除sheet set
void choose_extract_sheets(VolumeMesh *mesh, BODY * body, std::vector<DualSheet*> & sheets_vec, std::vector<DualSheet*> & choose_sheets);

//带约束的选择最优的待删除sheet set
void choose_extract_sheets_constrained(VolumeMesh *mesh, BODY * body, std::vector<DualSheet*> & sheets_vec, std::vector<int> &optional_sheet_num, std::vector<DualSheet*> & choose_sheets);

//组合算法实现
void combine(std::vector<int> & s, int n, int k,std::vector<std::vector<int>> & subsets);
//通过string来表示二进制数，若index1或者index2上有一位为‘1’则index那一位上也为‘1’
void add_binary(const std::string& index1,const std::string& index2,std::string &index);
//判断index2是否有index2的子集
bool is_subset(const std::string& index1,const std::string& index2);	


#endif