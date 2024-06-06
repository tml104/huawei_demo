#pragma once

class Endpoint
 {
public:
	bool isleft;
	double position;
	Endpoint
() {}
	Endpoint
(bool isleft, double position) {//记录每一个点的位置以及其在原有区间上是左端点还是右端点
		this->isleft = isleft;
		this->position = position;
	}

};
