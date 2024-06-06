#pragma once
#include "GetPatchType.h"
#include "pixel.h"
#include "Cell.h"
#include "json.h"
#include "setAttr.h"
#include "FileManagement.h"
#include <rnd_api.hxx>
//#include <rgbcolor.hxx>
#define MINFULLCONNECT 0.5

enum AxialDir{xDir,yDir,zDir};
enum BodyPos{ST,MID,ED};
BODY* getBodyInsideCell(BODY* body,Ohm_slice::Cell* cell);
double getAxialLength(enum AxialDir,BODY* body,Ohm_slice::Cell* cell);
BodyPos locateBodyPos(AxialDir dir,BODY* body,Ohm_slice::Cell* cell);
void handleFullConnect(AxialDir dir,BodyPos bodyPos1,BodyPos bodyPos2,double ratio,Ohm_slice::Cell* cell);
void handleAllDirFullConnect(BODY* body,Ohm_slice::Cell* cell);
void handleCellConnect(Ohm_slice::Cell& cell);

void handleCellConnect(Ohm_slice::Cell& cell, ENTITY_LIST &bodies,std::vector<std::vector<std::set<std::pair<double, double>>>> &x_conduct,
	std::vector<std::vector<std::set<std::pair<double, double>>>> &y_conduct, std::vector<std::vector<std::set<std::pair<double, double>>>> &z_conduct);
ENTITY_LIST getMetal(ENTITY_LIST &body_list,string json_path);
std::unordered_set<Ohm_slice::CellIdx,Ohm_slice::CellIdxHash,Ohm_slice::CellEqual> getMetalCellIdx(ENTITY_LIST& metal_list,std::vector<double>& x_pos,std::vector<double>& y_pos,std::vector<double>& z_pos);
void connectMark(Ohm_slice::Cell* cell, int faceId, std::vector<std::vector<std::set<std::pair<double, double>>>> &conduct);
void innerLoopConnectMark(Ohm_slice::Cell* cell, int face_dir, ENTITY_LIST &bodies, std::vector<std::vector<std::set<std::pair<double, double>>>> &conduct);
bool judgeSideConnect(std::vector<std::pair<int, int>> &metal_edge1, std::vector<std::pair<int, int>> &metal_edge2, Ohm_slice::Cell *cell, int face_id, std::vector<std::vector<std::set<std::pair<double, double>>>> &conduct);
bool sideConnect(Ohm_slice::Cell &cell, int face_dir, std::vector<std::vector<std::set<std::pair<double, double>>>> &conduct);


void handlePartialConnect(Ohm_slice::Cell *cell);
void oneInnerLoopPartialConnect(Ohm_slice::Cell *cell, int face_id);
std::vector<EDGE*> transforToEdge(std::vector< Ohm_slice::Cell* > cell_list);
void saveToSat(std::vector<EDGE*> edge_list, string file_name);

//≤‚ ‘ƒ£–Õ
BODY* testModel1();
BODY* testModel2();

BODY* testModel3();
BODY* testModel4();

Ohm_slice::Cell* testCell1();
std::vector<Ohm_slice::Cell*> testCell2();
Ohm_slice::Cell* testCell3();
Ohm_slice::Cell* testCell4();
Ohm_slice::Cell* testCellPixel();