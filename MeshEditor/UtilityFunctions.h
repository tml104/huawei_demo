#pragma once

// ACIS include
#include "ACISincluded.h"


#include <time.h>
#include <fstream>
#include <tuple>
//#include "test.h" // ? ȥ��������Ҳ��� copyedge ����Ӧ�û��õ��� check_outcome ��
#include <set>

#ifndef IN_HUAWEI
#include "logger44/CoreOld.h"
#else
#include "CoreOld.h"
#endif
#include "MarkNum.h"

#include "MyMeshManager.h"

namespace Utils {
	int CoedgeCount(EDGE* iedge);
	int PartnerCount(COEDGE* icoedge);
	int LoopLength(LOOP* lp);
	std::vector<COEDGE*> CoedgeOfEdge(EDGE* iedge);
	EDGE* CopyEdge(EDGE* in_edge);

	// ����entity_list����Ӧ·��
#ifdef USE_QSTRING
	void SaveToSAT(QString file_path, ENTITY_LIST &bodies);
#endif
	void SaveToSAT(const std::string& file_path, ENTITY_LIST &bodies);
	// ���浥��body����Ӧ·��
#ifdef USE_QSTRING
	void SaveToSATBody(QString file_path, BODY* body);
#endif
	void SaveToSATBody(const std::string& file_path, BODY* body);

	// ����·�����������Ϊ��Ԫ�飺������·�����ļ������ļ���չ����
#ifdef USE_QSTRING
	std::tuple<std::string, std::string, std::string> SplitPath(QString file_path);
#endif

	std::tuple<std::string, std::string, std::string> SplitPath(std::string file_path);

	// ��������bodies list�����浽�����ļ��У�
	void SaveModifiedBodies(const std::tuple<std::string, std::string, std::string>& split_path_tuple, ENTITY_LIST& bodies);

	// ����bodies list�еĸ���body���ֿ����浽�����ļ��У�
	void SaveModifiedBodiesRespectly(const std::tuple<std::string, std::string, std::string>& split_path_tuple, ENTITY_LIST& bodies);



	SPAvector GetNormalFromPoints(SPAposition x, SPAposition y, SPAposition z);

	void SaveSTL(const std::string& stl_file_path, std::vector<SPAposition> &out_mesh_points, std::vector<SPAunit_vector> &out_mesh_normals, std::vector<ENTITY*> & out_faces);

	void SAT2STL(const std::tuple<std::string, std::string, std::string>& split_path_tuple, ENTITY_LIST& bodies);

	void SAT2STL(const std::tuple<std::string, std::string, std::string>& split_path_tuple, ENTITY_LIST& bodies, const std::set<int>& selected_bodies);

	/*
		DEPRECATE��û��
	*/
	void EntityList2STL(const std::tuple<std::string, std::string, std::string>& split_path_tuple, ENTITY_LIST& bodies);
} // namespace Utils