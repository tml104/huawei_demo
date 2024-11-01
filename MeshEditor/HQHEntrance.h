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
#include "GeometryExporter.h"
#include "Timer.h"

#ifndef IN_HUAWEI
#include "ConstructModel.h"
#include "Exp1.h"
#include "Exp2.h"
#include "Experiment240621.h"
#include "SingleSideFaces.h"
#endif


#include "DegeneratedFaces.h"
#include "NonManifold2.h"
#include "StitchGap.h"
#include "DoubleModel.h"

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

#ifndef IN_HUAWEI
#include "hoopsview.h"
#endif

namespace HQHEntrance {

#ifdef IN_HUAWEI

	// 这上面还有函数

	static std::vector<std::string> path_vec2;

	ENTITY_LIST LoadEntityListFromID(int model_id, int flag = 0);

	ENTITY_LIST OpenBody(string file_path);

	void Run(int model_id, int option1);

#else

	void Run(const std::string& file_path, HoopsView* hoopsview);

#endif

}// namespace HQHEntrance