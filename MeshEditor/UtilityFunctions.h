#pragma once

// ACIS include
#include <boolapi.hxx>
#include <curextnd.hxx>
#include <cstrapi.hxx>
#include <face.hxx>
#include <lump.hxx>
#include <shell.hxx>
#include <split_api.hxx>
#include <kernapi.hxx>
#include <intrapi.hxx>
#include <intcucu.hxx>
#include <coedge.hxx>
#include <curdef.hxx>
#include <curve.hxx>
#include <ellipse.hxx>
#include <straight.hxx>
#include <intcurve.hxx>
#include <helix.hxx>
#include <undefc.hxx>
#include <elldef.hxx>
#include <interval.hxx>
#include <base.hxx>
#include <vector_utils.hxx>
#include <vertex.hxx>
#include <sps3crtn.hxx>
#include <sps2crtn.hxx>
#include <intdef.hxx>
#include <bool_api_options.hxx>
#include <loop.hxx>
#include <wire.hxx>
#include <pladef.hxx>
#include <plane.hxx>
#include <condef.hxx>
#include <cone.hxx>
#include <torus.hxx>
#include <tordef.hxx>
#include <attrib.hxx>
#include <af_api.hxx>
#include <attrib.hxx>
#include <geom_utl.hxx>
#include <body.hxx>
#include <point.hxx>
#include <ckoutcom.hxx>

#include <kernapi.hxx>
#include <io.h>
#include <time.h>
#include <fstream>
#include <tuple>
//#include "test.h" // ? 去掉这个就找不到 copyedge 里面应该会用到的 check_outcome 了
#include <fileinfo.hxx>
#include <stchapi.hxx>

#include "logger44/CoreOld.h"
#include "MarkNum.h"

namespace Utils {
	int CoedgeCount(EDGE* iedge);
	int PartnerCount(COEDGE* icoedge);
	std::vector<COEDGE*> CoedgeOfEdge(EDGE* iedge);
	EDGE* CopyEdge(EDGE* in_edge);

	// 保存entity_list到对应路径
	void SaveToSAT(QString file_path, ENTITY_LIST &bodies);
	// 保存单个body到对应路径
	void SaveToSATBody(QString file_path, BODY* body);

	// 给定路径，将其分离为三元组：（基本路径，文件名，文件扩展名）
	std::tuple<std::string, std::string, std::string> SplitPath(QString file_path);


} // namespace Utils