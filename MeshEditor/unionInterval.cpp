#include "stdafx.h"
#include "meshHeader.h"
bool mycmp(Interval *a, Interval *b) {
	return a->left < b->left;
}
void unionInterval(std::vector<Interval*> &list) {//对结果区间进行排序，并合并网格大小相同的相邻区间
	sort(list.begin(), list.end(), mycmp);
	int index = 0;
	//while (index < list.size() - 1) {
	//	if (list[index]->meshSize == list[index + 1]->meshSize && list[index]->right == list[index+1]->left) {
	//		list[index]->right = list[index + 1]->right;
	//		list.erase(list.begin() + index + 1);

	//	}
	//	else {
	//		index++;
	//	}
	//}
    for(int i=0;i<list.size();i++){
        Interval *interval = list[i];
        if(i==0){
            interval->rightInterval = list[i+1];
        }else if(i==list.size()-1){
            interval->leftInterval = list[i-1];
        }else{
            interval->rightInterval = list[i+1];
            interval->leftInterval = list[i-1];
        }

    }

	return;
}