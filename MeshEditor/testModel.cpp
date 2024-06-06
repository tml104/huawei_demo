#include "stdafx.h"
#include "ohmConnection.h"
#include "swp_opts.hxx"
#include "sweepapi.hxx"
#include "cstrapi.hxx"
#include "vector.hxx"
using namespace std;
BODY* testModel1(){
	//创建平面
	SPAvector* vec = new SPAvector(0,0,1);
	SPAposition pos(0,0,0);
	FACE* face;
	api_face_plane(pos,5,5,vec,face);

	//创建轮廓
	/*SPAposition* pts = new SPAposition[11];
	for(int i=0;i<5;i++)
	{
		pts[i].set_x(0);
		pts[i].set_y(12.5*i);
	}
	pts[0].set_z(0),pts[1].set_z(2.5),pts[2].set_z(5),pts[3].set_z(2.5),pts[4].set_z(0);
	for(int i=5;i<10;i++)
	{
		pts[i].set_x(0);
		pts[i].set_y(50-12.5*(i-5));
	}
	pts[5].set_z(5),pts[6].set_z(7.5),pts[7].set_z(10),pts[8].set_z(7.5),pts[9].set_z(5);
	pts[10].set_x(0),pts[10].set_y(0),pts[10].set_z(0);
	
	EDGE* pProfileEdge = NULL;
	api_curve_spline(11,pts, NULL, NULL, pProfileEdge);
	*/
	

	//创建脊线
    SPAposition origin0(0,25,5);
	SPAposition origin1(12.5,25,7.5);
	SPAposition origin2(25,25,10);
	SPAposition origin3(37.5,25,7.5);
	SPAposition origin4(50,25,5);
	SPAposition array_pts[5];
	array_pts[0] = origin0;
	array_pts[1] = origin1;
	array_pts[2] = origin2;
	array_pts[3] = origin3;
	array_pts[4] = origin4;

	EDGE* pPathEdge= NULL;
    api_curve_spline(5,array_pts,NULL, NULL,pPathEdge);

	//创建扫掠体
	BODY* body=NULL;
    api_initialize_sweeping();
	sweep_options *options = ACIS_NEW sweep_options();
	options->set_solid(true);
	api_sweep_with_options((ENTITY*)face,(ENTITY*)pPathEdge,options,(BODY *&)body);
    api_terminate_sweeping();
	BODY* pSplineBody=(BODY*)body;

	/*BODY* pCubic=NULL;
	api_solid_block(SPAposition(12.5,12.5,10),SPAposition(37.5,37.5,-10),pCubic);
	api_boolean(pCubic,pSplineBody,SUBTRACTION);*/
	return body;
}

BODY* testModel2(){
	//创建平面
	SPAvector* vec = new SPAvector(0,1,0);
	SPAposition pos(0,0,0);
	FACE* face;
	api_face_plane(pos,1,1,vec,face);
	

	//创建脊线
    SPAposition origin0(0,0,0);
	SPAposition origin1(20,10,0);
	
	SPAposition array_pts[2];
	array_pts[0] = origin0;
	array_pts[1] = origin1;
	EDGE* pPathEdge= NULL;
    api_curve_spline(2,array_pts,NULL, NULL,pPathEdge);

	//创建扫掠体
	BODY* body=NULL;
    api_initialize_sweeping();
	sweep_options *options = ACIS_NEW sweep_options();
	options->set_solid(true);
	api_sweep_with_options((ENTITY*)face,(ENTITY*)pPathEdge,options,(BODY *&)body);
    api_terminate_sweeping();
	BODY* pSplineBody=(BODY*)body;

	/*BODY* pCubic=NULL;
	api_solid_block(SPAposition(12.5,12.5,10),SPAposition(37.5,37.5,-10),pCubic);
	api_boolean(pCubic,pSplineBody,SUBTRACTION);*/
	return body;
}
BODY* testModel3(){
	//创建平面
	SPAvector* vec = new SPAvector(0,0,1);
	SPAposition pos(0,0,0);
	FACE* face;
	api_face_plane(pos,3,3,vec,face);

	
	//创建脊线
    SPAposition origin0(0,0,0);
	SPAposition origin1(0,0.5,2);
	SPAposition origin2(0,1,4);
	SPAposition origin3(0,0.5,6);
	SPAposition origin4(0,0,8);
	SPAposition array_pts[5];
	array_pts[0] = origin0;
	array_pts[1] = origin1;
	array_pts[2] = origin2;
	array_pts[3] = origin3;
	array_pts[4] = origin4;



	EDGE* pPathEdge= NULL;
    api_curve_spline(5,array_pts,NULL, NULL,pPathEdge);

	//创建扫掠体
	BODY* body=NULL;
    api_initialize_sweeping();
	sweep_options *options = ACIS_NEW sweep_options();
	options->set_solid(true);
	api_sweep_with_options((ENTITY*)face,(ENTITY*)pPathEdge,options,(BODY *&)body);
    api_terminate_sweeping();
	BODY* pSplineBody=(BODY*)body;

	return body;
}

BODY* testModel4(){
	SPAposition p1(0,0,0),p2(0,0,2);
	BODY* body,*body2,*body3;
	api_solid_cylinder_cone(p1,p2,2,1,2,NULL,body);
	api_solid_cylinder_cone(p1,p2,0.4,0.2,0.4,NULL,body2);
	api_solid_block(SPAposition(0,0,0),SPAposition(2,2,2),body3);
	api_boolean(body2,body,SUBTRACTION);
	api_boolean(body3,body,INTERSECTION);
	return body;
}



Ohm_slice::Cell* testCell1(){
	Ohm_slice::Cell* cell = new Cell();
	cell->leftDown[0] = 8.6;
	cell->leftDown[1] = 4.1;
	cell->leftDown[2] = -1;

	cell->rightUp[0] = 9.6;
	cell->rightUp[1] = 4.6;
	cell->rightUp[2] = 0;
	return cell;
}


vector<Ohm_slice::Cell*> testCell2(){
	vector<Ohm_slice::Cell*> celllist;
	for(int i=0;i<4;i++){
		Ohm_slice::Cell* cell = new Cell();
		cell->leftDown[0] = 0;
		cell->leftDown[1] = 0;
		cell->leftDown[2] = 2*i;

		cell->rightUp[0] = 2;
		cell->rightUp[1] = 2;
		cell->rightUp[2] = 2*i+2;
		celllist.push_back(cell);
	}
	return celllist;
}

Ohm_slice::Cell* testCell3(){
	Ohm_slice::Cell* cell = new Cell();
	cell->leftDown[0] = 0;
	cell->leftDown[1] = 0;
	cell->leftDown[2] = 0;

	cell->rightUp[0] = 1.8;
	cell->rightUp[1] = 1.2;
	cell->rightUp[2] = 1.8;
	return cell;
}

Ohm_slice::Cell* testCell4(){
	Ohm_slice::Cell* cell = new Cell();
	cell->leftDown[0] = 9;
	cell->leftDown[1] = 4.1;
	cell->leftDown[2] = -1;

	cell->rightUp[0] = 10.3;
	cell->rightUp[1] = 4.6;
	cell->rightUp[2] = 0;
	return cell;
}

Ohm_slice::Cell* testCellPixel(){
	Ohm_slice::Cell* cell = new Cell();
	cell->leftDown[0] = 0;
	cell->leftDown[1] = 0;
	cell->leftDown[2] = 0;

	cell->rightUp[0] = 1;
	cell->rightUp[1] = 1;
	cell->rightUp[2] = 1;
	for (int i = 0; i < 6; ++i){
		Ohm_slice::Patch* temp = new Ohm_slice::Patch;
		for (int j = 0; j < 16; j++){
			for (int k = 0; k < 16; ++k){
				temp->pixel_list[j][k] = new Ohm_slice::Pixel(-9999, -1,-1);
			}
		}
		cell->patch_list[i] = temp;

	}

	for (int i = 9; i < 13; ++i){
		for (int j = 6; j < 12; ++j){
			//cell->patch_list[0]->pixel_list[i][j]->order = 9999;
			//cell->patch_list[1]->pixel_list[i][j]->order = 9999;
				
			//cell->patch_list[2]->pixel_list[i][j]->order = 9999;
			//cell->patch_list[3]->pixel_list[i][j]->order = 9999;
				
			cell->patch_list[4]->pixel_list[i][j]->order = 9999;
			cell->patch_list[5]->pixel_list[i][j]->order = 9999;
		}
	}
	return cell;
}