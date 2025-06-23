#pragma once

#include <vector>

namespace BPSystem {

	struct NodeParamsBase {
		//(void*)ptrs[99];
		std::vector<void*> ptrs;
	};

}