#ifndef _GEOMETRY_FUNCS_H_
#define _GEOMETRY_FUNCS_H_

#include <string>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <unordered_set>
#include <vector>
#include <list>
#include <algorithm>
#include <hash_map>
#include <queue>

#include <QDomDocument>
#include <QDomElement>
#include <QSet>
#include <QHash>
#include <QMap>

#include "acis_headers.h"
#include "MeshDefs.h"
#include "DualDefs.h"

#define MAX_VALUE 10e5

struct HexJacobianTensor
{
	double			det[8];
	double			det_sqrt[8];
	double			val[8][3][3];
	double			val_sqrt[8][3][3];
};

HexJacobianTensor jacobian_metric_tensor(VolumeMesh* mesh,OvmCeH hexh);
double scaled_jacobian_metric(const HexJacobianTensor &hjt);

double calc_dihedral_angle(FACE *face1, FACE *face2);

#endif