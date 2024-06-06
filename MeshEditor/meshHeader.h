#pragma once
#include <vector>
#include <queue>
#include <algorithm>
#include <iostream>
#include <math.h>
#include <set>
#include "Endpoint.h"
#include "Interval.h"
#include <cmath>

int leftBinarySearch(double position, std::vector<Endpoint*> EndpointList);
int rightBinarySearch(double position, std::vector<Endpoint*> EndpointList);
std::vector<Interval*> intervalSort(std::priority_queue<Interval*, std::vector<Interval*>, compareNodePointer> &q);
void generationNonuniformMesh(std::vector<Interval*> &list,double r);
void unionInterval(std::vector<Interval*> &list);

//printInfo.cpp
void printAns(std::vector<Interval*> ans);
void printEndpoint(std::vector<Endpoint*> list);

//getPoint.cpp
void getMeshPoint(std::vector<Interval*> &list, double& r);

//nonuniformMesh.cpp
void generationNonuniformMesh(std::vector<Interval*> &list,double r);

//unionInterval.cpp
void unionInterval(std::vector<Interval*> &list);