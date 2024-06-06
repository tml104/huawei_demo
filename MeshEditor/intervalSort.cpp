#include "stdafx.h"
#include "meshHeader.h"
std::vector<Interval*> intervalSort(std::priority_queue<Interval*, std::vector<Interval*>, compareNodePointer> &q) {
	std::vector<Endpoint*> EndpointList;//现有区间范围排序表
	std::vector<Interval*> ans;//结果区间
	while (!q.empty()) {//采用优先队列将网格要求小的区间排在队头

		Interval* interval = q.top();
		q.pop();
		int left = leftBinarySearch(interval->left, EndpointList);//搜索左区间插入位置
		int right = rightBinarySearch(interval->right, EndpointList);//搜索右区间插入位置
		if (left != -1 && right != -1) { //在现有区间内部找到插入位置
			for (auto it = EndpointList.begin() + left; it != EndpointList.begin() + right + 1; it++) {//遍历所要插入的新区间左右端点间的所有原区间端点
				Endpoint* p = *it;
				if (it == EndpointList.begin() + left && p->isleft && p->position != interval->left) {//若新区间的左端点相邻的第一个原区间端点是一个左端点且该点与新区间左端点不重合
					Interval *temp = new Interval(interval->left, p->position, interval->meshSize);//构造从新区间左端点到原区间第一个端点的网格区域
					ans.push_back(temp);
				}
				else if (it == EndpointList.begin() + right && !p->isleft && p->position != interval->right) {//若新区间的右端点相邻的第一个原区间端点是一个右端点且该点与新区间右端点不重合
					Interval *temp = new Interval(p->position, interval->right, interval->meshSize);//构造从原区间第一个相邻端点到新区间右端点的网格区域
					ans.push_back(temp);
				}
				else if (!p->isleft && it != EndpointList.begin() + right) {//构建新区间内还未生成网格的区域，即从区间内部的右端点向左端点构建网格区间
					Endpoint* pp = *(it + 1);
					Interval *temp = new Interval(p->position, pp->position, interval->meshSize);
					ans.push_back(temp);
				}

			}
			//将新区域范围插入区间范围排序表，并删除被新区域范围所包括的原区间范围
			//--------------------------------------------------------------------
			auto leftErase = EndpointList.begin() + left;
			auto rightErase = EndpointList.begin() + right;
			Endpoint *leftEndpoint = *leftErase;
			Endpoint *rightEndpoint= *rightErase;
			EndpointList.erase(leftErase, rightErase + 1);
			if (!rightEndpoint->isleft) {
				EndpointList.insert(EndpointList.begin() + left, new Endpoint(false, interval->right));
			}
			if (leftEndpoint->isleft) {
				EndpointList.insert(EndpointList.begin() + left, new Endpoint(true, interval->left));
			}
			//--------------------------------------------------------------------
			
		}
		else if (left == -1 && right == -1) {//EndpointList为空
			EndpointList.push_back(new Endpoint(true, interval->left));
			EndpointList.push_back(new Endpoint(false, interval->right));
			ans.push_back(new Interval(interval->left, interval->right, interval->meshSize));
		}
		else if (left == -1) {//left超过现有最大值 在现有区间最右侧插入新区间
			EndpointList.push_back(new Endpoint(true, interval->left));
			EndpointList.push_back(new Endpoint(false, interval->right));
			ans.push_back(new Interval(interval->left, interval->right, interval->meshSize));
		}
		else {//right小于现有区间最小值 在现有区间最左侧插入新区间

			EndpointList.insert(EndpointList.begin(),new Endpoint(false, interval->right));
			EndpointList.insert(EndpointList.begin(), new Endpoint(true, interval->left));
			ans.push_back(new Interval(interval->left, interval->right, interval->meshSize));
		}
		//printEndpoint(EndpointList);
	}
	return ans;
}


