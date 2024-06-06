#include "stdafx.h"
#include "meshHeader.h"

double getMeshMin(double left,double right,double preSize,double r){
    double len = 0;
    while( len + preSize * r < std::abs(left-right)){
        len +=preSize*r;
        preSize *= r;
    }
    return preSize;
}

void generationNonuniformMesh(std::vector<Interval*> &list,double r){

    double preLeftMin;
    for(int i=1;i<list.size();i++){
        Interval *preInterval = list[i-1];
        Interval *interval = list[i];
        if(i==1){
            interval->leftMeshSize = preInterval->meshSize;
            preLeftMin = preInterval->meshSize;
        }else{
            preLeftMin = std::min(getMeshMin(preInterval->left,preInterval->right,preLeftMin,r),preInterval->meshSize);
            interval->leftMeshSize = preLeftMin;
        }
    }
    double preRightMin;
    for(int i=list.size()-2;i>=0;i--){
        Interval *preInterval = list[i+1];
        Interval *interval = list[i];
        if(i==list.size()-1){
            interval->rightMeshSize = preInterval->meshSize;
            preRightMin = preInterval->meshSize;
        }else{
            preRightMin = std::min(getMeshMin(preInterval->left,preInterval->right,preRightMin,r),preInterval->meshSize);
            interval->rightMeshSize = preRightMin;
        }
    }
    for(int i=0;i<list.size();i++){
        std::cout<<"meshSize="<<list[i]->meshSize<<" leftMeshSize="<<list[i]->leftMeshSize<<" rightMeshSize="<<list[i]->rightMeshSize<<std::endl;
    }
    for(int i=0;i<list.size();i++){
        Interval *interval = list[i];
        if(interval->meshSize<=interval->leftMeshSize && interval->meshSize<=interval->rightMeshSize){
            //在区间内按照原网格大小生成均匀网格
            int index = 0;
            while(interval->left+index*interval->meshSize < interval->right){
                interval->meshPosition.push_back(interval->left+index*interval->meshSize);
                index++;
            }
            interval->meshPosition.push_back(interval->right);
        }
        else if(interval->leftMeshSize < interval->meshSize && interval->rightMeshSize < interval->meshSize){
            //两侧均小于原网格大小
            double leftSt = interval->left;
            double leftPreMeshSize = std::min(interval->leftMeshSize*r,interval->meshSize);

            interval->meshPosition.push_back(leftSt);
            double rightSt = interval->right;
            double rightPreMeshSize = std::min(interval->rightMeshSize*r,interval->meshSize);
            interval->meshPosition.push_back(rightSt);
            int insertPos = 1;
            while(leftSt + leftPreMeshSize < rightSt - rightPreMeshSize){
                leftSt += leftPreMeshSize;
                rightSt -= rightPreMeshSize;

                interval->meshPosition.insert(interval->meshPosition.begin()+insertPos,rightSt);
                interval->meshPosition.insert(interval->meshPosition.begin()+insertPos,leftSt);
                insertPos++;
                leftPreMeshSize = std::min(leftPreMeshSize*r,interval->meshSize);
                rightPreMeshSize = std::min(rightPreMeshSize*r,interval->meshSize);
            }
            double minMeshSize = std::min(leftPreMeshSize,rightPreMeshSize);
            if(rightSt - leftSt > 1.6*minMeshSize){
                interval->meshPosition.insert(interval->meshPosition.begin()+insertPos,(rightSt+leftSt)/2);
            }else if(rightSt - leftSt < 0.6*minMeshSize && rightSt - leftSt > 0.2*minMeshSize){
                interval->meshPosition.erase(interval->meshPosition.begin()+insertPos-1);
                interval->meshPosition.erase(interval->meshPosition.begin()+insertPos-1);
                interval->meshPosition.insert(interval->meshPosition.begin()+insertPos-1,(rightSt+leftSt)/2);
            }
            else if(rightSt - leftSt <= 0.2*minMeshSize){
                interval->meshPosition.erase(interval->meshPosition.begin()+insertPos);
            }
        }
        else if(interval->leftMeshSize < interval->meshSize){
            //左侧网格小于原网格大小
            double st = interval->left;
            double preMeshSize = interval->leftMeshSize*r;
            interval->meshPosition.push_back(st);
            while(st + preMeshSize < interval->right && preMeshSize < interval->meshSize){
                st += preMeshSize;
                preMeshSize *= r;
                interval->meshPosition.push_back(st);
            }
            while(st + interval->meshSize < interval->right){
                st += interval->meshSize;
                interval->meshPosition.push_back(st);
            }
            interval->meshPosition.push_back(interval->right);
        }
        else{
            //右侧网格小于原网格大小
            double st = interval->right;
            double preMeshSize = interval->rightMeshSize*r;
            interval->meshPosition.push_back(st);
            while(st - preMeshSize > interval->left && preMeshSize < interval->meshSize){
                st -= preMeshSize;
                preMeshSize *= r;
                interval->meshPosition.push_back(st);
            }
            while(st - interval->meshSize > interval->left){
                st -= interval->meshSize;
                interval->meshPosition.push_back(st);
            }
            interval->meshPosition.push_back(interval->left);
            std::reverse(interval->meshPosition.begin(), interval->meshPosition.end());
        }
    }
}