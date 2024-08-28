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


namespace OldConstructModel{
	
	void Test1();

	void Test2();

	void Test3();

	void Test4();

} // namespace OldConstructModel

namespace ConstructModel {

	class MyModelConstructor {
	public:
		MyModelConstructor(const std::string& rt_save_path) : rt_save_path(rt_save_path) {
			api_initialize_constructors();
			api_initialize_booleans();
		};

		~MyModelConstructor() {
			api_terminate_booleans();
			api_terminate_constructors();
		}

		BODY* Construct240708(const std::string& file_name);
		BODY* Construct240710TotallyCoincident(const std::string& file_name);

	private:
		std::string rt_save_path;
		void save_constructed_body(const std::string& file_name, BODY* body);
	};

} // namespace ConstructModel