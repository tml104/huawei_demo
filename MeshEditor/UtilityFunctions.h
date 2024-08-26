#pragma once

// ACIS include
#include <boolapi.hxx>
#include <curextnd.hxx>
#include <cstrapi.hxx>
#include <face.hxx>
#include <lump.hxx>
#include <shell.hxx>
#include <split_api.hxx>
#include <kernapi.hxx>
#include <intrapi.hxx>
#include <intcucu.hxx>
#include <coedge.hxx>
#include <curdef.hxx>
#include <curve.hxx>
#include <ellipse.hxx>
#include <straight.hxx>
#include <intcurve.hxx>
#include <helix.hxx>
#include <undefc.hxx>
#include <elldef.hxx>
#include <interval.hxx>
#include <base.hxx>
#include <vector_utils.hxx>
#include <vertex.hxx>
#include <sps3crtn.hxx>
#include <sps2crtn.hxx>
#include <intdef.hxx>
#include <bool_api_options.hxx>
#include <loop.hxx>
#include <wire.hxx>
#include <pladef.hxx>
#include <plane.hxx>
#include <condef.hxx>
#include <cone.hxx>
#include <torus.hxx>
#include <tordef.hxx>
#include <attrib.hxx>
#include <af_api.hxx>
#include <attrib.hxx>
#include <geom_utl.hxx>
#include <body.hxx>
#include <point.hxx>
#include <ckoutcom.hxx>

#include <kernapi.hxx>
#include <time.h>
#include <fstream>
#include <tuple>
//#include "test.h" // ? ȥ��������Ҳ��� copyedge ����Ӧ�û��õ��� check_outcome ��
#include <fileinfo.hxx>
#include <stchapi.hxx>

#include "logger44/CoreOld.h"
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
} // namespace Utils