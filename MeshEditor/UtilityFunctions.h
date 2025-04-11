#pragma once

// ACIS include
#include "ACISincluded.h"


#include <time.h>
#include <fstream>
#include <tuple>
//#include "test.h" // ? 去掉这个就找不到 copyedge 里面应该会用到的 check_outcome 了
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

	// 保存entity_list到对应路径
#ifdef USE_QSTRING
	void SaveToSAT(QString file_path, ENTITY_LIST &bodies);
#endif
	void SaveToSAT(const std::string& file_path, ENTITY_LIST &bodies);
	// 保存单个body到对应路径
#ifdef USE_QSTRING
	void SaveToSATBody(QString file_path, BODY* body);
#endif
	void SaveToSATBody(const std::string& file_path, BODY* body);

	// 给定路径，将其分离为三元组：（基本路径，文件名，文件扩展名）
#ifdef USE_QSTRING
	std::tuple<std::string, std::string, std::string> SplitPath(QString file_path);
#endif

	std::tuple<std::string, std::string, std::string> SplitPath(std::string file_path);

	// 保存整个bodies list（保存到单个文件中）
	void SaveModifiedBodies(const std::tuple<std::string, std::string, std::string>& split_path_tuple, ENTITY_LIST& bodies);

	// 保存bodies list中的各个body（分开保存到各个文件中）
	void SaveModifiedBodiesRespectly(const std::tuple<std::string, std::string, std::string>& split_path_tuple, ENTITY_LIST& bodies);



	SPAvector GetNormalFromPoints(SPAposition x, SPAposition y, SPAposition z);

	void SaveSTL(const std::string& stl_file_path, std::vector<SPAposition> &out_mesh_points, std::vector<SPAunit_vector> &out_mesh_normals, std::vector<ENTITY*> & out_faces);

	void SAT2STL(const std::tuple<std::string, std::string, std::string>& split_path_tuple, ENTITY_LIST& bodies);

	void SAT2STL(const std::tuple<std::string, std::string, std::string>& split_path_tuple, ENTITY_LIST& bodies, const std::set<int>& selected_bodies);

	/*
		DEPRECATE：没用
	*/
	void EntityList2STL(const std::tuple<std::string, std::string, std::string>& split_path_tuple, ENTITY_LIST& bodies);
} // namespace Utils