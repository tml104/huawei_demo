#pragma once

#include <vector>
class Interval {
public:
	double left;//区间左端点
	double right;//区间右端点
	double meshSize;//该区间要求的网格大小
    double leftMeshSize;
    double rightMeshSize;
    Interval *leftInterval;
    Interval *rightInterval;
    bool isCovered;
    std::vector<double> meshPosition;
	Interval() {
		left = 0;
		right = 0;
		meshSize = 0;
        leftMeshSize = 0;
        rightMeshSize = 0;
        leftInterval = NULL;
        rightInterval = NULL;
	}
	Interval(double left, double right, double meshSize) {
		this->left = left;
		this->right = right;
		this->meshSize = meshSize;
        this->leftMeshSize = meshSize;
        this->rightMeshSize = meshSize;
        leftInterval = NULL;
        rightInterval = NULL;
	}
};
class compareNodePointer {//用于优先队列排序 小网格在前
public:
	bool operator () (Interval* &a, Interval* &b) {
		return a->meshSize > b->meshSize;
	}
};
