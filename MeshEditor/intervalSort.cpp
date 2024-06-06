#include "stdafx.h"
#include "meshHeader.h"
std::vector<Interval*> intervalSort(std::priority_queue<Interval*, std::vector<Interval*>, compareNodePointer> &q) {
	std::vector<Endpoint*> EndpointList;//�������䷶Χ�����
	std::vector<Interval*> ans;//�������
	while (!q.empty()) {//�������ȶ��н�����Ҫ��С���������ڶ�ͷ

		Interval* interval = q.top();
		q.pop();
		int left = leftBinarySearch(interval->left, EndpointList);//�������������λ��
		int right = rightBinarySearch(interval->right, EndpointList);//�������������λ��
		if (left != -1 && right != -1) { //�����������ڲ��ҵ�����λ��
			for (auto it = EndpointList.begin() + left; it != EndpointList.begin() + right + 1; it++) {//������Ҫ��������������Ҷ˵�������ԭ����˵�
				Endpoint* p = *it;
				if (it == EndpointList.begin() + left && p->isleft && p->position != interval->left) {//�����������˵����ڵĵ�һ��ԭ����˵���һ����˵��Ҹõ�����������˵㲻�غ�
					Interval *temp = new Interval(interval->left, p->position, interval->meshSize);//�������������˵㵽ԭ�����һ���˵����������
					ans.push_back(temp);
				}
				else if (it == EndpointList.begin() + right && !p->isleft && p->position != interval->right) {//����������Ҷ˵����ڵĵ�һ��ԭ����˵���һ���Ҷ˵��Ҹõ����������Ҷ˵㲻�غ�
					Interval *temp = new Interval(p->position, interval->right, interval->meshSize);//�����ԭ�����һ�����ڶ˵㵽�������Ҷ˵����������
					ans.push_back(temp);
				}
				else if (!p->isleft && it != EndpointList.begin() + right) {//�����������ڻ�δ������������򣬼��������ڲ����Ҷ˵�����˵㹹����������
					Endpoint* pp = *(it + 1);
					Interval *temp = new Interval(p->position, pp->position, interval->meshSize);
					ans.push_back(temp);
				}

			}
			//��������Χ�������䷶Χ�������ɾ����������Χ��������ԭ���䷶Χ
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
		else if (left == -1 && right == -1) {//EndpointListΪ��
			EndpointList.push_back(new Endpoint(true, interval->left));
			EndpointList.push_back(new Endpoint(false, interval->right));
			ans.push_back(new Interval(interval->left, interval->right, interval->meshSize));
		}
		else if (left == -1) {//left�����������ֵ �������������Ҳ����������
			EndpointList.push_back(new Endpoint(true, interval->left));
			EndpointList.push_back(new Endpoint(false, interval->right));
			ans.push_back(new Interval(interval->left, interval->right, interval->meshSize));
		}
		else {//rightС������������Сֵ ������������������������

			EndpointList.insert(EndpointList.begin(),new Endpoint(false, interval->right));
			EndpointList.insert(EndpointList.begin(), new Endpoint(true, interval->left));
			ans.push_back(new Interval(interval->left, interval->right, interval->meshSize));
		}
		//printEndpoint(EndpointList);
	}
	return ans;
}


