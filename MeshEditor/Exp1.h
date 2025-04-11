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

/*
	ʱ�䣺2024��3��8�� 16:42:44
	����ļ�����ҪĿ�ľ�����һЩ��鼸���ϵ�ʵ�飬�������Ҫ����֮����㷨ʵ��Ļ���˵��
*/

namespace Exp1 {
	struct Exp1 {
		std::set<EDGE*> bad_edge_set;

		void UpdateBadCoedgeSet(ENTITY_LIST& bodies);

		void TraverseLoops(LOOP* iloop, bool traverse_incident_loops, COEDGE* begin_coedge = nullptr);

		void StartExperiment(ENTITY_LIST & bodies);

		void Init(ENTITY_LIST &bodies);
	};
}