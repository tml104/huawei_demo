#pragma once
#include <cmath>

//#define IN_HUAWEI

#ifndef USE_HOOPSVIEW
#	ifndef IN_HUAWEI
#		define USE_HOOPSVIEW
#	endif
#endif

#ifndef USE_QSTRING
#	ifndef IN_HUAWEI
#		define USE_QSTRING
#	endif
#endif

const double MYPI = acos(-1.0);