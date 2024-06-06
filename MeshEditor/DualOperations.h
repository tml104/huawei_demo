#ifndef _DUAL_OPERATIONS_H_
#define _DUAL_OPERATIONS_H_

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


void retrieve_one_sheet(VolumeMesh *mesh, OvmEgH start_eh, std::unordered_set<OvmEgH> &sheet_ehs, std::unordered_set<OvmCeH> &sheet_chs);

void retrieve_sheets (VolumeMesh *mesh, SheetSet &sheet_set);

//判断sheet抽取是否会导致几何退化
bool is_sheet_can_be_extracted (VolumeMesh *mesh, DualSheet *sheet);

//通用sheet的extraction操作
bool one_simple_sheet_extraction (VolumeMesh *mesh, DualSheet *sheet, std::unordered_set<OvmFaH> &result_fhs);

bool general_sheets_extraction (VolumeMesh *mesh, BODY *body, std::vector<DualSheet *> & sheets);

//bool general_sheets_extraction_test (VolumeMesh *mesh, BODY *body, std::vector<DualSheet *> & sheets, SheetSet & other_sheets);

void get_shrink_set_on_direction(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmCeH> &shrink_set, int direction);

std::vector<DualSheet *> one_simple_sheet_inflation(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, 
	std::unordered_set<OvmCeH> &shrink_set, std::unordered_set<OvmEgH> &int_ehs, std::hash_map<OvmVeH, OvmVeH> &old_new_vhs_mapping);

std::vector<DualSheet *> sheet_dicing (VolumeMesh *mesh, BODY *body, DualSheet* sheet);

bool bulid_funmesh(VolumeMesh *mesh, BODY *body);

//建立基于捕获平面的fun sheet
void build_funsheet_planar_face(VolumeMesh* mesh, OvmVec3d normal, OvmVec3d pos, std::vector<std::unordered_set<OvmFaH>> &fhs, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set);

//根据种子四边形面集original_fhs，找出完整的四边形面集,采用简单的平面分割方法
void get_new_fhs_global(VolumeMesh* mesh,OvmVec3d& normal,OvmVec3d& pos,std::vector<std::unordered_set<OvmFaH>>& result_fh_vec, std::unordered_set<OvmCeH> & shrink_set);

//根据模型所有几何边找到所有的法向
bool determine_locations_of_inflation(VolumeMesh *mesh, BODY *body, std::vector<OvmVec3d>& normal,std::vector<OvmVec3d>& positon);

//对简单的quad set确定位于其一侧的shrink set
void get_traditional_shrink_set_for_simple_quad_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set);

//优化得到的四边形面集，暂仅考虑内部quad-set的情况
void optimize_toplogy_shrink_set(VolumeMesh* mesh, std::unordered_set<OvmCeH> &shrink_set, std::unordered_set<OvmEgH> &fixed_ehs, std::unordered_set<OvmFaH> &quad_set);

//根据一个边界网格面，找到它所属的column，可推广到任意网格面
void get_corresponding_column(VolumeMesh* mesh, OvmFaH &boundary_fh, std::vector<OvmCeH> &column);
#endif