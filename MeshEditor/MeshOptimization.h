#ifndef _MESH_OPTIMIZATION_H_
#define _MESH_OPTIMIZATION_H_

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

void smooth_volume_mesh (VolumeMesh *mesh, BODY *body, int smooth_rounds);
void smooth_volume_mesh_local (VolumeMesh *mesh, BODY *body, int smooth_rounds);
void smooth_sphere_mesh (VolumeMesh *mesh, BODY *body, int smooth_rounds);

#endif