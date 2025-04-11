#pragma once

// ACIS include
#include "ACISincluded.h"

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
	const double SAMPLE_POINTS_NUMBER = 11; // 最好是奇数

	void PrintEdgeGeometry(EDGE* e);
	void PrintFaceGeometry(FACE* f);

	// TODO
	SPAposition SamplePCurveIn3D(double par, pcurve& p, SURFACE* f);

	std::vector<SPAposition> SampleEdge(EDGE* edge, int sample_num = SAMPLE_POINTS_NUMBER);

	std::vector<SPAposition> SampleCoedge(COEDGE* coedge, int sample_num = SAMPLE_POINTS_NUMBER, bool reverse_flag = false);

	/*
		判断两个点是否几何一致
		几何一致：要么是同一个VERTEX，要么几何上差距小
	*/
	bool GeometryCoincidentVertex(VERTEX* v1, VERTEX* v2, const double epslion = POINT_EPSLION);

	bool GeometryCoincidentPoint(SPAposition p1, SPAposition p2, const double epslion = POINT_EPSLION);

	/*
		判断环上是不是有非流形边
	*/
	bool CheckLoopHasNonmanifoldEdge(LOOP* loop);
	bool CheckLoopHasSingleSideEdge(LOOP* loop);

	/*
		判断两条边是否符合“几何上相同”的条件
	*/
	bool GeometryCoincidentEdge(EDGE* e1, EDGE* e2);

	struct TopoChecker {
		ENTITY_LIST & bodies;

		void PrintTopo();

		TopoChecker(ENTITY_LIST &bodies): bodies(bodies) {}
	};

} // namespace GeometryUtils