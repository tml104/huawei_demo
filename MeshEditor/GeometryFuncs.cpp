#include "stdafx.h"
#include "GeometryFuncs.h"
#include <edge.hxx>
#include <plane.hxx>
#include <face.hxx>
#include <unitvec.hxx>
#include <cmath>
#include <geom_utl.hxx>

HexJacobianTensor jacobian_metric_tensor(VolumeMesh* mesh,OvmCeH hexh)
{
	HexJacobianTensor hjt; 

	int vtxidx = 0;
	OpenVolumeMesh::CellVertexIter	hexahedvtx_iter(mesh->cv_iter(hexh));

	do 
	{
		// get JacobianMatrix
		double A[3][3];
		OvmVeH vh = *hexahedvtx_iter;
		OvmHaEgH heh[3];
		OvmHaFaH hfh[3];

		std::unordered_set<OvmHaEgH> v_hehs; 
		std::unordered_set<OvmHaFaH> v_hfhs;

		for (auto voh_it = mesh->voh_iter(vh); voh_it; ++voh_it)
			v_hehs.insert(*voh_it);

		int i = 0;
		foreach(auto & heh_temp,v_hehs){
			for (auto hehf_it = mesh->hehf_iter (heh_temp); hehf_it; ++hehf_it){
				if (*hehf_it == mesh->InvalidHalfFaceHandle)
					continue;
				if((mesh->incident_cell(*hehf_it) == mesh->InvalidCellHandle) || (mesh->incident_cell(*hehf_it) != hexh))
					continue;
				v_hfhs.insert(*hehf_it);
			}
			if(v_hfhs.size() != 0){
				heh[i] = heh_temp;
				hfh[i] = *v_hfhs.begin();
				v_hfhs.clear();
				++i;
			}
		}

		if(mesh->adjacent_halfface_in_cell(hfh[0],heh[0]) == hfh[1]){
			OvmHaEgH heh_temp = heh[1];
			OvmHaFaH hfh_temp = hfh[1];
			heh[1] = heh[2]; hfh[1] = hfh[2];
			heh[2] = heh_temp; hfh[2] = hfh_temp;
		}



		OvmVec3d this_pt = mesh->vertex(vh);
		OvmVec3d pts[3];
		for(int i = 0; i < 3; ++i)
		{
			pts[i] = mesh->vertex(mesh->halfedge(heh[i]).to_vertex());
		}

		for(int i = 0; i < 3; ++i)
		{
			for(int j = 0; j < 3; ++j)
			{
				A[i][j] = pts[j][i] - this_pt[i];
			}
		}

		// get determination
		hjt.det_sqrt[vtxidx] = A[0][0] * A[1][2] * A[2][1]
		+ A[0][1] * A[1][0] * A[2][2]
		+ A[0][2] * A[1][1] * A[2][0]
		- A[0][0] * A[1][1] * A[2][2]
		- A[0][1] * A[1][2] * A[2][0]
		- A[0][2] * A[1][0] * A[2][1];
		hjt.det[vtxidx] = hjt.det_sqrt[vtxidx] * hjt.det_sqrt[vtxidx];

		// get transposition matrix
		double AT[3][3];
		for(int i = 0; i < 3; ++i)
		{
			for(int j = 0; j < 3; ++j)
			{
				AT[i][j] = A[j][i];
			}
		}

		// get matrix tensor
		for(int i = 0; i < 3; ++i)
		{
			for(int j = 0; j < 3; ++j)
			{
				hjt.val[vtxidx][i][j] = 0.0;
				for(int k = 0; k < 3; ++k)
				{
					hjt.val[vtxidx][i][j] += AT[i][k] * A[k][j];
				}
				hjt.val_sqrt[vtxidx][i][j] = sqrt(hjt.val[vtxidx][i][j]);
			}
		}

		++vtxidx;
		++hexahedvtx_iter;
	} while (hexahedvtx_iter && vtxidx < 8);

	return hjt;

}

/** Get the minimal scaled determinant of all the Jacobian Matrix of a hexahedron.
*	\param  hjt the Jacobian Matrix Tensors of the 8 vertexes of the hexahedron.
*   \return the minimal scaled determinant of all the Jacobian Matrix. 
*/
double scaled_jacobian_metric(const HexJacobianTensor& hjt)
{
	double min_det = 1e20;
	for (int i = 0; i < 8; ++i)
	{
		double unit_det = hjt.det_sqrt[i] 
		/ (hjt.val_sqrt[i][0][0] * hjt.val_sqrt[i][1][1] * hjt.val_sqrt[i][2][2]);
		if(unit_det < min_det)
			min_det = unit_det;
	}

	if(min_det < 1e20)
		return min_det;
	else
		return -1.0;
}

double calc_dihedral_angle(FACE *face1, FACE *face2)
{
	double angle = 0;
	LOOP *loop = face1->loop();
	while (loop != NULL)
	{
		COEDGE *coedge = loop->start();
		do
		{
			FACE *face = coedge->partner()->loop()->face();
			if (face == face2)
			{
				SPAposition ver_pos = coedge->edge()->mid_pos();
				SPAvector ver_dire = coedge->edge()->mid_point_deriv();

				if (coedge->sense () == REVERSED)
					ver_dire = -ver_dire;
				surface *surface1 = face1->geometry()->trans_surface();
				SPAunit_vector unit_vector1 = surface1->point_normal(ver_pos);
				if (face1->sense () == REVERSED)
					unit_vector1 = -unit_vector1;

				surface *surface2 = face2->geometry()->trans_surface();
				SPAunit_vector unit_vector2 = surface2->point_normal(ver_pos);
				if (face2->sense() == REVERSED)
					unit_vector2 = -unit_vector2;
				angle = angle_between(unit_vector1, unit_vector2, normalise(ver_dire));
				if (angle < SPAresabs)
				{
					angle = M_PI;
				}
				return angle;
			}
			coedge = coedge->next();
		}while( loop->start() != coedge );
		loop = loop->next();
	}
	return -1;
}