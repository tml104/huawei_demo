#include "StdAfx.h"
#include "topologyoptwidget.h"  
//#include "mesh_optimization.h"
//#include "TopologyQualityPredict.h"
//#include "PrioritySetManager.h"
//#include "ZSRACISHeads.h"
#include "ZSRACISNotify.h"

#include <boolapi.hxx>
#include <curextnd.hxx>
#include <cstrapi.hxx>
#include <face.hxx>
#include <lump.hxx>
#include <shell.hxx>
#include <swp_opts.hxx>
#include <sweepapi.hxx>
#include<curveq.hxx>
#include<curdef.hxx>
#include <bl_enum.hxx> //边的凹凸性


#define PI 3.1415926

//计算拓扑面的法向
SPAunit_vector find_normal_of_face(FACE* faceToExamine, SPAposition* pOnFace)
{
	SPAunit_vector v0;
	SPApar_pos parOfPos=(faceToExamine->geometry()->equation()).param(*pOnFace);
	
	//获取拓扑面对应几何面的法向
	v0= (faceToExamine->geometry()->equation()).eval_normal(parOfPos);
	
	//判断拓扑面与几何面法矢是否为相反关系
	if(faceToExamine->sense() == REVERSED)
		v0= -v0;
	return v0;
}


