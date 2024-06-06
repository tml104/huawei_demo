#ifndef OPTFRAME_H
#define OPTFRAME_H

#include "StdAfx.h"

#include "StdAfx.h"
#include "topologyoptwidget.h"
#include <dlib/optimization.h>
using namespace dlib;
typedef matrix<double,0,1> column_vec;

#include "topologyoptwidget.h"
#include <ellipse.hxx>
#include <edge.hxx>
#include <unitvec.hxx>
#include <face.hxx>
#include <loop.hxx>
#include <QMessageBox>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cone.hxx>
#include <vector_utils.hxx>
#include <edentrel.hxx>
#include <cone.hxx>
#include <lump.hxx>
#include <shell.hxx>
#include <string>
#include <dlib/optimization.h>
#include <bs2curve.hxx>
#include <sp3crtn.hxx>
#include "Frame.h"
#include "TopologyQualityPredict.h"
using namespace dlib;
#define PI 3.1415926
#define INF 100000

#include <glpk.h>
#include <stdio.h>
#include <stdlib.h>

typedef double ini_mat[3][3];


int ini_coe(VolumeMesh *mesh,std::hash_map<OvmCeH,Frame> cell_frame_mapping,Frame tar_Frame);
double obj_fun_1(const column_vec& angles);
const column_vec obj_1_derivate(const column_vec& angles);
void initializ_mat(Frame tar_Frame);
double obj_fun_2(const column_vec& angles);
const column_vec obj_2_derivate(const column_vec& angles);
void local_frame_smoothing(VolumeMesh *mesh,std::hash_map<OvmCeH, Frame> & cell_frame_mapping,std::hash_map<OvmCeH, Frame> & cell_frame_mapping_updating,int iteration_num );


#endif