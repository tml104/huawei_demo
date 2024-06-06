#include "stdafx.h"
#include "DualDefs.h"
#include <queue>

int DualSheet::sheet_idx = 0;
int DualColumn::column_idx = 0;
int DualChord::chord_idx = 0;

DualSheet::DualSheet (VolumeMesh *m)
	: VolumeMeshElementGroup (m)
{
	type = "sheet";
	name = QString ("sheet No.%1").arg (sheet_idx++);
}

DualColumn::DualColumn (VolumeMesh *m)
	: VolumeMeshElementGroup (m)
{
	type = "column";
	name = QString ("column No.%1").arg (column_idx++);
}

bool DualColumn::if_intersect (DualColumn *col)
{
	return true;
}

DualChord::DualChord (VolumeMesh *m)
	: VolumeMeshElementGroup (m)
{
	type = "chord";
	name = QString ("chord No.%1").arg (chord_idx++);
}

ChordPolyline get_chord_polyline (DualChord *chord)
{
	ChordPolyline chord_polyline;
	foreach (OvmEgH eh, chord->ordered_ehs)
		chord_polyline.push_back (chord->mesh->barycenter (eh));
	return chord_polyline;
}