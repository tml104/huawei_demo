#include "stdafx.h"
#include "meshHeader.h"
//�����������Ķ��ֲ��� ����>=position����Сindex
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
//���������Ҳ�Ķ��ֲ��� ����<=position�����index
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