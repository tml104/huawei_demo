#pragma once

// ACIS include
#include "ACISincluded.h"

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