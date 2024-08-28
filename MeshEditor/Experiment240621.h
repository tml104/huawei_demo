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

// Project include

#include "logger44/CoreOld.h"
#include "MarkNum.h"
#include "MyConstant.h"
#include "UtilityFunctions.h"
#include "GeometryUtils.h"

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

#ifdef USE_HOOPSVIEW
#include "hoopsview.h"
#endif

/*
	实验3：针对Cent的特殊实验
*/
namespace Exp3 {
	struct Exp3 {
		/*
			1: 获取从3610到3642的边的所有bounds然后合并
		*/
		std::pair<SPAposition, SPAposition> GetEdgesLowHigh(BODY* blank);

		/*
			2：构造实体
		*/
		BODY* MakeTool(std::pair<SPAposition, SPAposition> LH);

		/*
			3: 布尔运算切除
		*/
		void DoCut(BODY* tool, BODY* blank);
		void Init(BODY* blank);
	};

} // namespace Exp3

/*
	2024年7月9日 15:27:17
	实验4： 读入一个包含两个正方体的，输出各个面几何
*/
namespace Exp4 {

	class Exp4 {
	public:
		Exp4(ENTITY_LIST& bodies): bodies_to_be_checked(bodies){}
		~Exp4() {}

		/*
			1:
		*/
		void PrintFacesGeometry();

		void FaceFaceIntersectionExperiment();
		void FaceFaceIntersectionExperiment2();
		void FaceFaceIntersectionExperiment3();
		void FaceFaceIntersectionExperiment4();
		void StartExperiment();
	private:
		ENTITY_LIST& bodies_to_be_checked;
	};

}// namespace Exp4