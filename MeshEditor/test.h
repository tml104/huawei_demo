#pragma once
#include "Cell.h"
#include "ohmConnection.h"
#include "GeometricFill.h"
#include "muilthreadHandle.h"
void DebugPatch(Ohm_slice::Patch *patch, int patch_id);
void DebugCell(Ohm_slice::Cell *cell);
void DebugConnectInfo(Ohm_slice::ConnectInfo &info);
void DebugPixelInfo(Ohm_slice::Pixel &pixel);

Ohm_slice::Patch *getNewPatch();
Ohm_slice::Cell *getNewCell();
void setTestPatchOrder(Ohm_slice::Patch *patch, vector<vector<int>> &order);
void setTestPatchGeom(Ohm_slice::Patch *patch, vector<vector<int>> &geom);
void testOhmCellAttr();
void testOhm2InnerLoop();
Ohm_slice::Cell* testOhm1InnerLoop();
Ohm_slice::Cell* testUnflodConnect();

void testFill();
void testMuilThread();