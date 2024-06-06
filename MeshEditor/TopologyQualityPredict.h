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

//Sheet_data�洢�ĵ���sheetɾ��ʱ���ϲ������ӳ���ϵvh_unit_set�Լ��ϲ������ӳ���ϵmerge_edge_unit_set
struct Sheet_data{
	std::hash_map<int,int> vh_unit_set;
	std::hash_map<int,int> merge_edge_unit_set;
};

//������ÿ���ߵ����ԣ����ߵ����͡������Լ��������
void get_edge_property(VolumeMesh *mesh, std::map<OvmEgH, EdgeAttribute> & edge_property);

//ÿ��sheetɾ��ʱ���ϲ������Լ��ϲ��������Ϣ
void get_sheet_data(VolumeMesh* mesh, DualSheet * sheet, Sheet_data&);

//sheet setɾ��ʱ��ֱ�Ӽ���ϲ������Լ��ϲ��������Ϣ
void get_sheet_set_data(VolumeMesh* mesh, std::vector<DualSheet*> sheets, Sheet_data&);

//����sheetͬʱɾ��ʱ���ϲ������Լ��ϲ��������Ϣ
void merge_sheet_data(VolumeMesh *mesh,Sheet_data& data1,Sheet_data & data2,Sheet_data & result);

//����sheet set�еĺϲ��������Ϣ�жϸ�sheet�Ƿ�ɱ�ɾ����������Ч�ԣ������ܶȣ�
//bool is_sheet_can_be_extracted (VolumeMesh *mesh, BODY *body, std::hash_map<int,int> &vh_union_set,double& density_quality);

//����sheet set�еĺϲ��������Ϣ�жϸ���sheet�Ƿ�ɱ�ɾ�������Ҽ���ɾ���������ܶ�
bool is_sheets_can_be_extracted (VolumeMesh *mesh, BODY *body, std::hash_map<int,int> &vh_union_set,double& density_quality);

//����sheet set�кϲ��������Ϣ�жϸ���sheetɾ���������������Ӱ��
int topology_value_change_after_merge(VolumeMesh * mesh, std::vector<DualSheet*>&sheets,std::hash_map<int,int> &merge_edge_union_set, std::map<OvmEgH, EdgeAttribute> & edge_property);

//Ԥ��sheetɾ����������ܶȵ�Ӱ��
bool is_sheet_can_be_extract_based_density(VolumeMesh *mesh, DualSheet* sheet);

//ѡ�����ŵĴ�ɾ��sheet set
void choose_extract_sheets(VolumeMesh *mesh, BODY * body, std::vector<DualSheet*> & sheets_vec, std::vector<DualSheet*> & choose_sheets);

//��Լ����ѡ�����ŵĴ�ɾ��sheet set
void choose_extract_sheets_constrained(VolumeMesh *mesh, BODY * body, std::vector<DualSheet*> & sheets_vec, std::vector<int> &optional_sheet_num, std::vector<DualSheet*> & choose_sheets);

//����㷨ʵ��
void combine(std::vector<int> & s, int n, int k,std::vector<std::vector<int>> & subsets);
//ͨ��string����ʾ������������index1����index2����һλΪ��1����index��һλ��ҲΪ��1��
void add_binary(const std::string& index1,const std::string& index2,std::string &index);
//�ж�index2�Ƿ���index2���Ӽ�
bool is_subset(const std::string& index1,const std::string& index2);	


#endif