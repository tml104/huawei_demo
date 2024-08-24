#pragma once

// QT include
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QDockWidget>

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
#include "test.h"
#include "json/json.h"
#include "FileManagement.h"
#include "ohmConnection.h"
#include "pixel.h"
#include "GetPatchType.h"
#include "JsonHandle.h"
#include "GeometricFill.h"
#include "setAttr.h"

#include "logger44/CoreOld.h"
#include "MarkNum.h"
#include "MyConstant.h"
#include "UtilityFunctions.h"

// STL
#include <io.h>
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

	/*
		判断两个点是否几何一致
		几何一致：要么是同一个VERTEX，要么几何上差距小
	*/
	bool GeometryCoincidentVertex(VERTEX* v1, VERTEX* v2, const double epslion = POINT_EPSLION);

	bool GeometryCoincidentPoint(SPAposition p1, SPAposition p2, const double epslion = POINT_EPSLION);

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