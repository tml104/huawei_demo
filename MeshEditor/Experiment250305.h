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

		void FacesCheck();

		void StartExperiment();

	private:

		ENTITY_LIST& bodies_to_be_checked;

	};

}