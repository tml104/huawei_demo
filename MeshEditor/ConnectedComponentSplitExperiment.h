#pragma once

// QT include
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QDockWidget>

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

// Project include
#include "test.h"
#include "json/json.h"
#include "FileManagement.h"
#include "ohmConnection.h"
#include "pixel.h"
#include "GetPatchType.h"
#include "JsonHandle.h"
#include "GeometricFill.h"
#include "setAttr.h"

#include "MyDebugInfoMarco.h"
#include "MarkNum.h"
#include "MyConstant.h"
#include "UtilityFunctions.h"

// STL
#include <io.h>
#include <time.h>
#include <fstream>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <functional>
#include <string>
#include <ctime>
#include <cmath>

/*
	CCSplit = ConnectedComponentS
*/
namespace CCSplit {



	/*
		每个联通分量保存为一个单独的子文件（确保其中只有一个body、lump、shell）
	*/
	void SaveCC(QString file_path);

	/*
		输入：所有bodies的列表（ENTITY_LIST &bodies）， 文件路径
		输出：每个联通分量保存为一个单独的子文件（确保其中只有一个body、lump、shell）
	*/
	void Init(ENTITY_LIST &bodies, QString file_path);
} // namespace CCSplit