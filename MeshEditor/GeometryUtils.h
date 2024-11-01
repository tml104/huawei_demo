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
#include <debug.hxx>
#include <eulerapi.hxx>
#include <geometry.hxx>

// Project include
#ifndef IN_HUAWEI
#include "logger44/CoreOld.h"
#else
#include "CoreOld.h"
#endif
#include "MarkNum.h"
#include "MyConstant.h"
#include "UtilityFunctions.h"

// STL
#include <time.h>
#include <fstream>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <functional>
#include <string>
#include <ctime>
#include <cmath>

namespace GeometryUtils {

	const double POINT_EPSLION = 1e-6;
	const double SAMPLE_POINTS_NUMBER = 11; // ���������

	void PrintEdgeGeometry(EDGE* e);
	void PrintFaceGeometry(FACE* f);

	std::vector<SPAposition> SampleEdge(EDGE* edge, int sample_num = SAMPLE_POINTS_NUMBER);

	/*
		�ж��������Ƿ񼸺�һ��
		����һ�£�Ҫô��ͬһ��VERTEX��Ҫô�����ϲ��С
	*/
	bool GeometryCoincidentVertex(VERTEX* v1, VERTEX* v2, const double epslion = POINT_EPSLION);

	bool GeometryCoincidentPoint(SPAposition p1, SPAposition p2, const double epslion = POINT_EPSLION);

	/*
		�жϻ����ǲ����з����α�
	*/
	bool CheckLoopHasNonmanifoldEdge(LOOP* loop);
	bool CheckLoopHasSingleSideEdge(LOOP* loop);

	/*
		�ж��������Ƿ���ϡ���������ͬ��������
	*/
	bool GeometryCoincidentEdge(EDGE* e1, EDGE* e2);

	struct TopoChecker {
		ENTITY_LIST & bodies;

		void PrintTopo();

		TopoChecker(ENTITY_LIST &bodies): bodies(bodies) {}
	};

} // namespace GeometryUtils