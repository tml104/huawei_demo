#pragma once

// ���֮��Ҫ����ʵ�����Ļ��Ͷ����������Ū

// ACIS include
#include "ACISincluded.h"

// Project include
#ifndef IN_HUAWEI
#include "logger44/CoreOld.h"
#include "json/json.h"
#else
#include "CoreOld.h"
#include "json.h"
#endif
#include "MarkNum.h"
#include "DebugShow.h"
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

namespace Exp250305
{

	struct Exp250305
	{
		ENTITY_LIST& bodies;

		void ModifyPcurve();

		void Start();

		Exp250305(ENTITY_LIST& bodies) : bodies(bodies) {}

	};


};


/*
	2025��4��9�� 20:55:47
	ʵ��5�����glossy undersize surface
*/
namespace Exp5 {


	class Exp5 {
	public:
		Exp5(ENTITY_LIST& bodies) : bodies_to_be_checked(bodies) {}
		~Exp5() {}

		void EdgeAndCoedgeGeometryCheck(EDGE* edge, COEDGE* coedge);

		void ModifyFaces(const std::string& json_path);

		void FacesCheck();

		void StartExperiment();

	private:

		ENTITY_LIST& bodies_to_be_checked;

	};

}

/*
	2025��5��6�� 20:16:52
	ʵ��6�� api_merge_faces
*/

namespace Exp6 {

	class Exp6 {
	public:
		Exp6(ENTITY_LIST& bodies) : bodies(bodies) {}
		~Exp6() {}

		void MergeFace();

		void StartExperiment();

	private:
		ENTITY_LIST& bodies;

	};
}


/*
	2025��5��13�� 22:59:54
	ʵ��7�� Stitch����
*/
namespace Exp7 {

	class Exp7 {
	public:

		// bodies2: ���û���ϵ��Ǹ�
		Exp7(ENTITY_LIST& bodies, ENTITY_LIST& bodies2): bodies(bodies), bodies2(bodies2) {}
		~Exp7() {}

		void StitchModels();

		void MergeModels();

		void StartExperiment();

	private:
		ENTITY_LIST& bodies, bodies2;

	};

}