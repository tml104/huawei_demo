#pragma once
#include <iostream>
#include "Cell.h"
#include <vector>
#include <set>
#include <queue>
#include <map>
#include <algorithm>

void setCellAttr(Ohm_slice::Cell *cell);

void posMap(int face_id, int target_face_id, int x, int y, int &target_x, int &target_y);
void autoPosMap(int face_id, int x, int y, int &target_id, int &target_x, int &target_y);
void autoPosMap2(int face_id, int x, int y, int &target_id1, int &target_x1, int &target_y1, int &target_id2, int &target_x2, int &target_y2);
void mapRealPos(Ohm_slice::Cell *cell, int patch_id, int x, int y, double &real_x, double &real_y, double &real_z);

void copyPixelInfo(Ohm_slice::Pixel *to, Ohm_slice::Pixel *from);
void countUnflodMetalArea(Ohm_slice::Cell *cell);
void setPatchNonMetalInfo(Ohm_slice::Patch *patch);
void countPatchInnerLoopAndMetalArea(Ohm_slice::Patch *patch);

void markFullConnect(Ohm_slice::Cell *cell);
void markOneEdge(Ohm_slice::Cell *cell, Ohm_slice::NodePosition &a, Ohm_slice::NodePosition &b);
