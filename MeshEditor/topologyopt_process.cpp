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
#include <bl_enum.hxx> //�ߵİ�͹��


#define PI 3.1415926

//����������ķ���
SPAunit_vector find_normal_of_face(FACE* faceToExamine, SPAposition* pOnFace)
{
	SPAunit_vector v0;
	SPApar_pos parOfPos=(faceToExamine->geometry()->equation()).param(*pOnFace);
	
	//��ȡ�������Ӧ������ķ���
	v0= (faceToExamine->geometry()->equation()).eval_normal(parOfPos);
	
	//�ж��������뼸���淨ʸ�Ƿ�Ϊ�෴��ϵ
	if(faceToExamine->sense() == REVERSED)
		v0= -v0;
	return v0;
}


