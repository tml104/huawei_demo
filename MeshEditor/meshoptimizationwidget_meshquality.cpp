#include "stdafx.h"
#include "meshoptimizationwidget.h"

/** Get the Jacobian Matrix Tensors which is A^T * A, and A is the Jacobian Matrix of a hexahedron vertex.
*	\param  mesh the pointer of HexadMesh.
*   \param hexh the handle of the hexahedron. 
*   \return the 8 Jacobian Matrix Tensors of the vertexes of the hexahedron. 
*/
HexJacobianTensor jacobian_metric_tensor(VolumeMesh* mesh,OvmCeH hexh)
{
	HexJacobianTensor hjt;

	int vtxidx = 0;
	OpenVolumeMesh::CellVertexIter hexahedvtx_iter = mesh->cv_iter(hexh);
	do 
	{
		// get JacobianMatrix
		double A[3][3];
		OvmVeH vh = *hexahedvtx_iter;
		OpenVolumeMesh::HalfEdgeHandle heh[3];
	    heh[0] = *mesh->voh_iter(vh);
		if(mesh->halfedge(heh[0]).from_vertex() != vh)
			heh[0] = mesh->mate_half_edge_handle(heh[0]);
			heh[1] = mesh->next_half_edge_handle(mesh->mate_half_edge_handle(heh[0]));
			heh[2] = mesh->next_half_edge_handle(mesh->mate_half_edge_handle(heh[1]));

			Point this_pt = mesh->point(vh);
			Point pts[3];
			for(int i = 0; i < 3; ++i)
			{
				pts[i] = mesh->point(mesh->to_vertex_handle(heh[i]));
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
		double min_det = VMMath::VM_MAX;
		for (int i = 0; i < 8; ++i)
		{
			double unit_det = hjt.det_sqrt[i] 
				/ (hjt.val_sqrt[i][0][0] * hjt.val_sqrt[i][1][1] * hjt.val_sqrt[i][2][2]);
			if(unit_det < min_det)
				min_det = unit_det;
		}

		if(min_det < VMMath::VM_MAX)
			return min_det;
		else
			return -1.0;
	}

void MeshOptimizationWidget::on_mesh_quality_calculation()
{
	int num_hex = mesh->n_cells();
	int num_vertex = mesh->n_vertices();

	double max_jac,min_jac,mean_jac,dev_jac;
	int max_eval,min_eval,mean_eval,dev_eval;
	int max_vval,min_vval,mean_vval,dev_vval;


}