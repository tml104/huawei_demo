#pragma once
//#include "topologyoptwidget.h"
#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include "body.hxx"
#include "entity.hxx"
#include "lists.hxx"
#include "cstrapi.hxx"
#include "lump.hxx"
#include "kernapi.hxx"
#include "boolapi.hxx"
#include "shell.hxx"
#include "face.hxx"
#include "faceqry.hxx"
#include <unitvec.hxx>
#include <intrapi.hxx>
#include <clash_bodies.hxx>
#include <fstream>
#include "ohm_struct.h"
#include "Cell.h"
#include <set>
#include "ohmConnection.h"
#define MIND 6
using namespace Ohm_slice;
using namespace std;
struct pixel_node {
	int bound_tag;
	int order;
	int index;
	int xPos;
	int yPos;
};
void get_patch_attribute(Ohm_slice::Patch* patch);
void get_part_boundary_lj(vector<vector<pixel_node> >& body_list, vector<pixel_node>& part, vector<pixel_node>& boundary);
int dfs(pixel_node & temp, map<int, bool>& visited, vector<vector<pixel_node> > & body_list);
void bfs_patch_attri(Ohm_slice::Patch* patch, vector<vector<pixel_node>>& body_list);
void handleCase2Conduction(Ohm_slice::Cell* cell,int splitFaceId,int halfCircleId);



void identify_if_type_2(Ohm_slice::Cell* cell, int patch_id);
BODY* getCellBox(Ohm_slice::Cell *cell);
std::vector<BODY*> intersectPart(BODY* cellBox,ENTITY_LIST &bodies);
bool isInterfering(BODY* body1,BODY* body2);
bool isInterferingFace(FACE* cellFace, LUMP* lump);
void handleCase1(Ohm_slice::Cell *cell,BODY* cellBox,std::vector<BODY*> partList,int faceId1,int faceId2);
void handleCase1Conduction(Ohm_slice::Cell* cell,int face_dir);
FACE* getCellFace(BODY* cellBox,int faceId);
std::vector<EDGE*> showEdge(Ohm_slice::Cell* cell);
