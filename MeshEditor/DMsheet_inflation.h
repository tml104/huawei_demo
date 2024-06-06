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
 * @quad_sets: ���quad set������inflation���ı��μ��ϵ�vector
 * @shrink_sets: ��������ÿ��quad set��Ӧ��shrink set��vector
*/
std::vector<DualSheet *> DM_sheet_inflation(VolumeMesh *mesh, std::vector<std::unordered_set<OvmFaH> > &quad_sets,
	std::vector<std::unordered_set<OvmCeH> >& shrink_sets);

/*
 * @quad_sets: ���quad set������inflation���ı��μ��ϵ�vector
*/
std::vector<DualSheet *> X_sheet_inflation(VolumeMesh *mesh, std::vector<std::unordered_set<OvmFaH> > &quad_sets,
										  std::vector<std::unordered_set<OvmCeH> > &shrink_sets);

//----------------------------------------------------------------------------------------------------------
/*
 * @quad_sets: ���quad set������inflation���ı��μ��ϵ�vector,
 *             ���������ʱhalfface ͨ��halfface��ָ��shrink set�ķ���
 * return sheet inflation �ɹ�û�� 
*/
bool sheet_inflation_by_halfface(VolumeMesh *mesh, std::vector<std::unordered_set<OvmHaFaH> > &halfface_quad_sets,
							std::vector<DualSheet *> &sheets);

//----------------------------------------------------------------------------------------------------------



bool X_get_shrink_set_on_direction(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmCeH> &shrink_set, int direction);
bool X_get_shrink_set_on_direction(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmCeH> &shrink_set, OvmHaFaH hfh);
bool X_get_shrink_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmCeH> &shrink_set);


/*
 * @quad_sets: ���quad set������inflation���ı��μ��ϵ�vector
*/
bool get_shrink_sets_from_quad_sets(VolumeMesh *mesh, std::vector<std::unordered_set<OvmFaH> > &quad_sets,
								   std::vector<std::unordered_set<OvmCeH> > &shrink_sets);

/*
 * ���ڵ���quad set��inflation�жϵ�
 * ��Ӧ2016-2017�궬ѧ�ڵ����ܱ� rule1
*/
bool is_valid_inflation_rule1(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set,
							  std::unordered_set<OvmCeH> &shrink_set);

/*
 * ���ڵ���quad set��inflation�жϵ�
 * ��Ӧ2016-2017�궬ѧ�ڵ����ܱ� rule2
 *
*/
bool is_strictly_valid_inflation_rule2(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set,
								       std::unordered_set<OvmCeH> &shrink_set);

/*
 * ��������øı亯����ȷ����Ҫ�ﵽ������Ч��
 *
*/
bool is_valid_inflation_case(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set,
							 std::unordered_set<OvmCeH> &shrink_set);

/*
 * ���ڵ���quad set�ж��ıߵ�shrink set���ʺ���inflation.
 * ��2016-2017�궬ѧ�ڵ����ܱ� ��3������������
 *
*/
void get_better_shrink_set_for_single_quad_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set,
											   std::unordered_set<OvmCeH> &shrink_set);

/*
 * ��ȫ shrink set �� shrink setû�м�϶
 */
void complete_shrink_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set);

/*
 * ���ڵ���û���Խ���quad set�õ���ͳ��shrink set������quad set�ֳ���������һ���ֵ����񼯺�
 */
void get_traditional_shrink_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set);

/*
 * ����quad set���������ϵ�Բ���ཻ��������Զ�����shrink set
 * ���ʵ������һ�������:
 *   1) ��direction(ȡֵ0��1)��ȷ��һ��face handle��halfface handle����ȷ��cell�ķ���������
 *   2) ����ÿ������㣬������Χȷ��shrink set��Ӧ������ʼ������
 */
bool get_shrink_set_in_cylinder_case_wrong(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set);


bool get_shrink_set_in_cylinder_case(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set,
									std::vector<std::unordered_set<OvmFaH> > &debug_group_fhs,
									std::unordered_set<OvmFaH> &debug_fhs);

bool get_shrink_set_for_self_cross_quad_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set,
											std::vector<std::unordered_set<OvmFaH> > &debug_fhs_groups1,
											std::vector<std::unordered_set<OvmFaH> > &debug_fhs_groups2);

void complete_shrink_set_in_cylinder_case(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set);
