#ifndef _DMSHEET_INFLATION_H_
#define _DMSHEET_INFLATION_H_

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
#include <stack>

#include <QDomDocument>
#include <QDomElement>
#include <QSet>
#include <QHash>
#include <QMap>

#include "acis_headers.h"
#include "hoopsview.h"
#include "MeshDefs.h"
#include "DualDefs.h"
#include "FuncDefs.h"
#include "DualOperations.h"
#include "TopologyQualityPredict.h"
#endif

/*
 * @quad_sets: 多个quad set用来做inflation的四边形集合的vector
 * @shrink_sets: 对于上面每个quad set相应的shrink set的vector
*/
std::vector<DualSheet *> DM_sheet_inflation(VolumeMesh *mesh, std::vector<std::unordered_set<OvmFaH> > &quad_sets,
	std::vector<std::unordered_set<OvmCeH> >& shrink_sets);

/*
 * @quad_sets: 多个quad set用来做inflation的四边形集合的vector
*/
std::vector<DualSheet *> X_sheet_inflation(VolumeMesh *mesh, std::vector<std::unordered_set<OvmFaH> > &quad_sets,
										  std::vector<std::unordered_set<OvmCeH> > &shrink_sets);

//----------------------------------------------------------------------------------------------------------
/*
 * @quad_sets: 多个quad set用来做inflation的四边形集合的vector,
 *             但面的类型时halfface 通过halfface来指定shrink set的方向
 * return sheet inflation 成功没有 
*/
bool sheet_inflation_by_halfface(VolumeMesh *mesh, std::vector<std::unordered_set<OvmHaFaH> > &halfface_quad_sets,
							std::vector<DualSheet *> &sheets);

//----------------------------------------------------------------------------------------------------------



bool X_get_shrink_set_on_direction(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmCeH> &shrink_set, int direction);
bool X_get_shrink_set_on_direction(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmCeH> &shrink_set, OvmHaFaH hfh);
bool X_get_shrink_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmCeH> &shrink_set);


/*
 * @quad_sets: 多个quad set用来做inflation的四边形集合的vector
*/
bool get_shrink_sets_from_quad_sets(VolumeMesh *mesh, std::vector<std::unordered_set<OvmFaH> > &quad_sets,
								   std::vector<std::unordered_set<OvmCeH> > &shrink_sets);

/*
 * 对于单个quad set做inflation判断的
 * 对应2016-2017年冬学期第七周报 rule1
*/
bool is_valid_inflation_rule1(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set,
							  std::unordered_set<OvmCeH> &shrink_set);

/*
 * 对于单个quad set做inflation判断的
 * 对应2016-2017年冬学期第七周报 rule2
 *
*/
bool is_strictly_valid_inflation_rule2(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set,
								       std::unordered_set<OvmCeH> &shrink_set);

/*
 * 根据需求该改变函数以确保需要达到怎样的效果
 *
*/
bool is_valid_inflation_case(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set,
							 std::unordered_set<OvmCeH> &shrink_set);

/*
 * 对于单个quad set判断哪边的shrink set更适合做inflation.
 * 在2016-2017年冬学期第七周报 第3点中有描述。
 *
*/
void get_better_shrink_set_for_single_quad_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set,
											   std::unordered_set<OvmCeH> &shrink_set);

/*
 * 补全 shrink set 让 shrink set没有间隙
 */
void complete_shrink_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set);

/*
 * 对于单个没有自交的quad set得到传统的shrink set，即被quad set分成两部分中一部分的网格集合
 */
void get_traditional_shrink_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set);

/*
 * 对于quad set和两个以上的圆柱相交的情况，自动来找shrink set
 * 这个实现是有一定问题的:
 *   1) 靠direction(取值0和1)来确定一个face handle的halfface handle再来确定cell的方法不可行
 *   2) 对于每个特殊点，在其周围确定shrink set后，应该立马开始传播。
 */
bool get_shrink_set_in_cylinder_case_wrong(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set);


bool get_shrink_set_in_cylinder_case(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set,
									std::vector<std::unordered_set<OvmFaH> > &debug_group_fhs,
									std::unordered_set<OvmFaH> &debug_fhs);

bool get_shrink_set_for_self_cross_quad_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set,
											std::vector<std::unordered_set<OvmFaH> > &debug_fhs_groups1,
											std::vector<std::unordered_set<OvmFaH> > &debug_fhs_groups2);

void complete_shrink_set_in_cylinder_case(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set);
