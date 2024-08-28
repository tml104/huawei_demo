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
	ʵ��3�����Cent������ʵ��
*/
namespace Exp3 {
	struct Exp3 {
		/*
			1: ��ȡ��3610��3642�ıߵ�����boundsȻ��ϲ�
		*/
		std::pair<SPAposition, SPAposition> GetEdgesLowHigh(BODY* blank);

		/*
			2������ʵ��
		*/
		BODY* MakeTool(std::pair<SPAposition, SPAposition> LH);

		/*
			3: ���������г�
		*/
		void DoCut(BODY* tool, BODY* blank);
		void Init(BODY* blank);
	};

} // namespace Exp3

/*
	2024��7��9�� 15:27:17
	ʵ��4�� ����һ����������������ģ���������漸��
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