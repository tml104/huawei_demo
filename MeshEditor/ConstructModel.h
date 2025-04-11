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