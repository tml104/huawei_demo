#include "stdafx.h"
#include "meshHeader.h"
//用于区间左侧的二分查找 返回>=position的最小index
int leftBinarySearch(double position, std::vector<Endpoint*> EndpointList) {
	if (EndpointList.empty()) return -1;
	int left = 0;
	int right = EndpointList.size() - 1;
	if (EndpointList[right]->position < position) return -1;
	while (left < right) {
		int mid = (left + right) / 2;
		if (EndpointList[mid]->position == position) return mid;
		else if (EndpointList[mid]->position > position) right = mid;
		else left = mid + 1;
	}
	return right;
}
//用于区间右侧的二分查找 返回<=position的最大index
int rightBinarySearch(double position, std::vector<Endpoint*> EndpointList) {
	if (EndpointList.empty()) return -1;
	int left = 0;
	int right = EndpointList.size() - 1;
	if (EndpointList[left]->position > position) return -1;
	while (left < right) {
		int mid = (left + right + 1) / 2;
		if (EndpointList[mid]->position == position) return mid;
		else if (EndpointList[mid]->position > position) right = mid - 1;
		else left = mid;
	}
	return left;
}