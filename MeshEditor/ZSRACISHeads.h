#ifndef ZSRACISHEADS_H 
#define ZSRACISHEADS_H 

//存在相冲突的接口
/**************System************/
#include <assert.h>

#include "ha_bridge.h"
#include "ha_rend_options.h"

//#include "base.hxx"  
//#include "logical.h"  
//#include "box.hxx"
//#include "position.hxx"
//#include "transf.hxx"
//#include "unitvec.hxx"
//#include <boolapi.hxx>
//#include <curextnd.hxx>
//#include <cstrapi.hxx>
//#include "ptentrel.hxx"
//#include "raytest.hxx"
//#include "intrapi.hxx" 
//
//#include "kernapi.hxx"
//#include "kernopts.hxx"
//#include "acis.hxx"  
//#include "acistype.hxx"  
//#include "attrib.hxx"
//#include "entity.hxx"
//
//#include "curve.hxx"
//#include "curdef.hxx"
//#include "surface.hxx"
//#include "point.hxx"
//#include "transfrm.hxx"
//#include "body.hxx"
//#include "shell.hxx"
//#include "face.hxx"
//#include "loop.hxx"
//#include "edge.hxx"
//#include "coedge.hxx"
//#include "vertex.hxx"
//#include "surdef.hxx"
//#include "bulletin.hxx"
//#include "ckoutcom.hxx"
//#include "straight.hxx"
//
//#include "esplit.hxx"
//
//#include "sp3srtn.hxx"
//#include "bs3surf.hxx"
//#include "spldef.hxx"
//
//#include "coverapi.hxx"
//#include "faceutil.hxx"

/**************Topology**********/
#include <face.hxx>
#include <loop.hxx>
#include <lump.hxx>
#include <shell.hxx>
//11、Point Attribution
#include <ptentrel.hxx> 
//12、Edge Attribution
#include<curveq.hxx>
#include<curdef.hxx>
#include <bl_enum.hxx> //边的凹凸性
#include <intrapi.hxx> 
#include<position.hxx>
#include <param.hxx>
#include <kernapi.hxx>
#include <vector_utils.hxx>
#include <edentrel.hxx>//边与其他实体的关系
#include <boolapi.hxx> 
//13、Face/surface Attribution
#include<faceqry.hxx>
#include <box.hxx> 
#include <surdef.hxx>

/**************Geometory**********/
#include <ellipse.hxx>
#include <cone.hxx>
#include <bs2curve.hxx>
#include <sp3crtn.hxx>
 
/**************Feature**********/
#include <blendapi.hxx> //倒圆
#include <cstrapi.hxx>  //立方体、圆柱、圆台等
#include <swp_opts.hxx>
#include <sweepapi.hxx>
#include <ha_bridge.h>//生成细分网格
#include <transf.hxx>
#include <kernapi.hxx>


//4、渲染效果
//41、修改实体颜色
#include <rnd_api.hxx>
#include <rgbcolor.hxx>

/**************Data structure**********/
#include <unitvec.hxx> 


/*******C++数据结构*******/
#include <iostream>
#include <fstream>
#include <sstream>
#include <qstring.h>
#include <string>
#include <iomanip>
#include <map>
#include <set>
#include <unordered_set>
#include <vector>
#include <list>
#include <algorithm>
#include <hash_map>
#include <queue>

#endif