#include "stdafx.h"
//
//#include "meshHeader.h"
//
//void getMeshPoint(std::vector<Interval*> &list, double& r)
//{
//    const double M = 0.00001;
//    for (int i = 0; i < list.size(); ++i) {  // 依次遍历每个区间
//        std::vector<double>& point= list[i]->meshPosition;  // 该区间的点坐标序列
//
//  //      std::vector<double> temp;
//		//for (int i = 0; i < point.size(); ++i){
//		//	if (i == 0){
//		//		temp.push_back(point[i]);
//		//	}
//		//	else { 
//		//		if (point[i] > temp[i-1]){
//		//			temp.push_back(point[i]);
//		//		}
//		//	}
//		//}
//		//point = temp;
//
//
//
//		//while(point[0] == point[1]){
//		//	point.erase(point.begin());
//		//}
//		//while(point[point.size()-1] == point[point.size()-2]){
//		//	point.erase(point.end()-1);
//		//}
//		
//		int meshPositionSize = point.size();  // 每次修改了vector之后要更新一下vector.size()
//		//std::cout << "mesh point size is : "<< meshPositionSize <<std::endl;
//
//		double offset_value = 0.05;
//		//log(r) + offset_value
//		auto mergeInterval = [&](double small_one, double big_one) -> bool {
//			if (small_one / big_one <= log(r) + offset_value) {
//				return true;
//			}
//			else {
//				return false;
//			}
//		};
//
//
//		if (meshPositionSize < 3){
//			std::cout << "error: too few points(less than 3)" << std::endl;
//			return;
//		}
//
//		//              |--|--------|  只有三个顶点
//		else if (meshPositionSize == 3){
//			double left = point[1] - point[0];
//			double right = point[2] - point[1];
//			if (left > right){
//				std::swap(left, right);
//			}
//
//			if (mergeInterval(left, right)){
//				point.erase(point.begin() + 1);
//				meshPositionSize = point.size();
//			}
//			else{
//				point[1] = (point[0] + point[2]) / 2;
//			}
//		}
//
//        //              |-|------|------|-----|-|   有大于三个顶点
//		else{
//
//			double left = point[1] - point[0];
//			double left_next = point[2] - point[1];
//			double left_next_next = point[3] - point[2];
//			double right = point[meshPositionSize - 2] - point[meshPositionSize - 1];
//			double right_pre = point[meshPositionSize - 3] - point[meshPositionSize - 2];
//			double right_pre_pre = point[meshPositionSize - 4] - point[meshPositionSize - 3];
//
//			double left_big = left_next > left_next_next ? left_next_next : left_next;
//			double right_big = right_pre > right_pre_pre ? right_pre_pre : right_pre;
//
//
//			if (mergeInterval(left, left_big)) {
//				point.erase(point.begin() + 1);
//				meshPositionSize = point.size();
//			}
//			else {
//                if (std::abs(left - left_next) <= M || std::abs(left_next / left - r) <= M || std::abs(left / left_next - r)<=M){
//                    point[1] = (point[0] + point[2]) / 2;
//                }
//                else {
//
//                    //point[1] = (point[0] + point[2]) / 2;
//                    point[1] = point[0] + (point[2] - point[0]) / (1 + r);
//                    //point[1] = point[0] + (left + left_next) / (1 + r);
//                }
//			}
//
//			if (mergeInterval(right, right_big)) {
//				point.erase(point.end() - 2);
//				meshPositionSize = point.size();
//			}
//			else {
//                if (std::abs(right - right_pre)<=M || std::abs(right / right_pre - r)<=M || std::abs(right_pre / right - r)<=M){
//                    point[meshPositionSize - 2] = (point[meshPositionSize - 3] + point[meshPositionSize - 1]) / 2;
//                }
//                else {
//                    //point[meshPositionSize - 2] = (point[meshPositionSize - 3] + point[meshPositionSize - 1]) / 2;
//                    point[meshPositionSize - 2] = point[meshPositionSize - 1] - (point[meshPositionSize - 1] - point[meshPositionSize - 3]) / (1 + r);
//                    //point[meshPositionSize - 2] = point[meshPositionSize - 1] - (right + right_pre) / (1 + r);
//                }
//			}
//		}
//	
//
//
//		//double left = point[1] - point[0];
//		//double left_next = point[2] - point[1];
//		//double right = point[meshPositionSize - 2] - point[meshPositionSize - 1];
//		//double right_pre = point[meshPositionSize - 3] - point[meshPositionSize - 2];
//
//
//		//if (mergeInterval(left, left_next)) {
//		//	point.erase(point.begin() + 1);
//		//}
//		//else {
//		//	//point[1] = (point[0] + point[2]) / 2;
//		//	point[1] = point[0] + (point[2] - point[0]) / (1 + r);
//		//	//point[1] = point[0] + (left + left_next) / (1 + r);
//
//		//}
//
//		//if (mergeInterval(right, right_pre)) {
//		//	point.erase(point.end() - 2);
//		//}
//		//else {
//		//	//point[meshPositionSize - 2] = (point[meshPositionSize - 3] + point[meshPositionSize - 1]) / 2;
//		//	point[meshPositionSize - 2] = point[meshPositionSize - 1] - (point[meshPositionSize - 1] - point[meshPositionSize - 3]) / (1 + r);
//		//	//point[meshPositionSize - 2] = point[meshPositionSize - 1] - (right + right_pre) / (1 + r);
//		//}
//	
//	}
//}