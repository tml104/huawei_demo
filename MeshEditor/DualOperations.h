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

//�ж�sheet��ȡ�Ƿ�ᵼ�¼����˻�
bool is_sheet_can_be_extracted (VolumeMesh *mesh, DualSheet *sheet);

//ͨ��sheet��extraction����
bool one_simple_sheet_extraction (VolumeMesh *mesh, DualSheet *sheet, std::unordered_set<OvmFaH> &result_fhs);

bool general_sheets_extraction (VolumeMesh *mesh, BODY *body, std::vector<DualSheet *> & sheets);

//bool general_sheets_extraction_test (VolumeMesh *mesh, BODY *body, std::vector<DualSheet *> & sheets, SheetSet & other_sheets);

void get_shrink_set_on_direction(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, std::unordered_set<OvmCeH> &shrink_set, int direction);

std::vector<DualSheet *> one_simple_sheet_inflation(VolumeMesh *mesh, std::unordered_set<OvmFaH> &inflation_quad_set, 
	std::unordered_set<OvmCeH> &shrink_set, std::unordered_set<OvmEgH> &int_ehs, std::hash_map<OvmVeH, OvmVeH> &old_new_vhs_mapping);

std::vector<DualSheet *> sheet_dicing (VolumeMesh *mesh, BODY *body, DualSheet* sheet);

bool bulid_funmesh(VolumeMesh *mesh, BODY *body);

//�������ڲ���ƽ���fun sheet
void build_funsheet_planar_face(VolumeMesh* mesh, OvmVec3d normal, OvmVec3d pos, std::vector<std::unordered_set<OvmFaH>> &fhs, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set);

//���������ı����漯original_fhs���ҳ��������ı����漯,���ü򵥵�ƽ��ָ��
void get_new_fhs_global(VolumeMesh* mesh,OvmVec3d& normal,OvmVec3d& pos,std::vector<std::unordered_set<OvmFaH>>& result_fh_vec, std::unordered_set<OvmCeH> & shrink_set);

//����ģ�����м��α��ҵ����еķ���
bool determine_locations_of_inflation(VolumeMesh *mesh, BODY *body, std::vector<OvmVec3d>& normal,std::vector<OvmVec3d>& positon);

//�Լ򵥵�quad setȷ��λ����һ���shrink set
void get_traditional_shrink_set_for_simple_quad_set(VolumeMesh *mesh, std::unordered_set<OvmFaH> &quad_set, std::unordered_set<OvmCeH> &shrink_set);

//�Ż��õ����ı����漯���ݽ������ڲ�quad-set�����
void optimize_toplogy_shrink_set(VolumeMesh* mesh, std::unordered_set<OvmCeH> &shrink_set, std::unordered_set<OvmEgH> &fixed_ehs, std::unordered_set<OvmFaH> &quad_set);

//����һ���߽������棬�ҵ���������column�����ƹ㵽����������
void get_corresponding_column(VolumeMesh* mesh, OvmFaH &boundary_fh, std::vector<OvmCeH> &column);
#endif