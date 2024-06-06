#pragma once

#ifndef DEBUGINFO
#define DEBUGINFO printf("[func:%s line:%d] ", __FUNCTION__,  __LINE__);
#endif

#ifndef DEBUGPRINTF
#define DEBUGPRINTF
#endif