#pragma once

class Endpoint
 {
public:
	bool isleft;
	double position;
	Endpoint
() {}
	Endpoint
(bool isleft, double position) {//��¼ÿһ�����λ���Լ�����ԭ������������˵㻹���Ҷ˵�
		this->isleft = isleft;
		this->position = position;
	}

};
