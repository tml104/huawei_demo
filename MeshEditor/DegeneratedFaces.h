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

// my include

#ifndef IN_HUAWEI
#include "logger44/CoreOld.h"
#else
#include "CoreOld.h"
#endif

#include "MyConstant.h"
#include "MarkNum.h"
#include "UtilityFunctions.h"
#include "GeometryUtils.h"

// std include
#include <set>
#include <map>
#include <vector>
#include <array>
#include <bitset>
#include <algorithm>
#include <functional>
#include <string>
#include <ctime>
#include <cmath>
#include <exception>

namespace DegeneratedFaces {

	const double REQ_REL_ACCY = 1e-6;
	const double THRESHOLD_AREA = 3e-6;


	// TODO: 这里改成基于单个面的
	struct DegeneratedFacesFixer {

		int degenerated_face_count;

		std::set<FACE*> degenerated_faces;
		ENTITY_LIST& bodies;

		/* 1 */
		void FindDegeneratedFaces();

		void RemoveDegeneratedFaces();

		void Status();

		DegeneratedFacesFixer(ENTITY_LIST & bodies) : bodies(bodies), degenerated_face_count(0) {};

		bool Start();

		void Clear();
	};

} // namespace DegeneratedFaces