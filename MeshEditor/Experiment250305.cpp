#include "StdAfx.h"
#include "GeometryExporter.h"
#include "Experiment250305.h"

void Exp250305::Exp250305::ModifyPcurve()
{
	int target_body_marknum = 1;
	int target_edge_marknum = 1;
	int target_coedge_marknum = 2;

	ENTITY* ibody_ptr = nullptr;
	ENTITY* target_coedge_ptr = nullptr;
	ENTITY_LIST coedge_list;

	LOG_DEBUG("0");

	for (int i = 0; i < bodies.count(); i++) {

		ENTITY* tmp_ibody_ptr = (bodies[i]);
		
		if(MarkNum::GetId(tmp_ibody_ptr) == target_body_marknum)
		{
			LOG_DEBUG("1");
			ibody_ptr = tmp_ibody_ptr;
			break;
		}
	}



	if (ibody_ptr)
	{
		api_get_coedges(ibody_ptr, coedge_list);

	
		// coedge
		for (int j = 0; j < coedge_list.count(); j++) {
			ENTITY* ptr = coedge_list[j];
			if (MarkNum::GetId(ptr) == target_coedge_marknum)
			{
				LOG_DEBUG("2");
				target_coedge_ptr = ptr;
				break;
			}
		}
	}

	LOG_DEBUG("3");


	// 改几何
	if (target_coedge_ptr)
	{
		LOG_INFO("ModifyPcurve for target_coedge_ptr begin: ", MarkNum::GetId(target_coedge_ptr));

		COEDGE* coedge = dynamic_cast<COEDGE*>(target_coedge_ptr);

		PCURVE* pc = coedge->geometry();

		if (pc)
		{ 
			pcurve xpc = pc->equation();

			bs2_curve bs2 = xpc.cur();

			// 取出control points
			int num_pts;
			int max_points = 999;
			SPApar_pos* ctrlpts = new SPApar_pos[max_points];

			bs2_curve_control_points(bs2, num_pts, ctrlpts);

			if (num_pts > max_points)
			{
				throw std::runtime_error("num_pts > max_points");
			}
			

			for (int i=0;i<num_pts;i++)
			{
				if (i == 4)
				{
					//ctrlpts[i].u = 206.817e-5;
					//ctrlpts[i].v = 169.732e-5;

					ctrlpts[i].u = 1;
					ctrlpts[i].v = 1;

				}
				else if (i==5)
				{
					//ctrlpts[i].u = -88.233e-5;
					//ctrlpts[i].v = 10.026e-5;
					ctrlpts[i].u = 0.5;
					ctrlpts[i].v = 0.5;
				}
				else if (i==7)
				{
					//ctrlpts[i].u = 98.543e-5;
					//ctrlpts[i].v = -92.835e-5;
					ctrlpts[i].u = 0.25;
					ctrlpts[i].v = 0.25;
				}
				else if (i == 8)
				{
					//ctrlpts[i].u = -107.182e-5;
					//ctrlpts[i].v = -590.902e-5;
					ctrlpts[i].u = 0.1;
					ctrlpts[i].v = 0.1;
				}
			}

			// Get weight
			int num_weights;
			int max_weights = 999;
			double* weights = new double[max_weights];
			bs2_curve_weights(bs2, num_weights, weights);

			// [debug] print weights
			LOG_DEBUG("num_weights: %d", num_weights);

			for (int i=0;i<num_weights;i++)
			{
				LOG_DEBUG("weight[%d]: %.5lf", i, weights[i]);
			}

			// 设置回去
			bs2_curve_set_ctrlpts(bs2, ctrlpts, weights);

			//pc->set_def(xpc);
		}


		LOG_INFO("ModifyPcurve for target_coedge_ptr end");
	}

}

void Exp250305::Exp250305::Start()
{
	ModifyPcurve();

}



void Exp5::Exp5::EdgeAndCoedgeGeometryCheck(EDGE* edge, COEDGE* coedge)
{
	// 中间采样点个数（最好是奇数？其实这里没有要求，之前要求是奇数只是因为要取中点）
	const int N = 11;

	LOG_INFO("Checking edge and coedge points: edge: %d, coedge:%d, N: %d",
		MarkNum::GetId(edge),
		MarkNum::GetId(coedge),
		N
	);

	std::vector<SPAposition> edge_points_vec = GeometryUtils::SampleEdge(edge, N);
	std::vector<SPAposition> coedge_points_vec = GeometryUtils::SampleCoedge(coedge, N);

	// 先输出看看

	for (int i = 0; i < edge_points_vec.size(); i++)
	{
		double x = edge_points_vec[i].x();
		double y = edge_points_vec[i].y();
		double z = edge_points_vec[i].z();
		LOG_INFO("edge_points_vec[%d]: (%.5lf, %.5lf, %.5lf)", i, x, y, z);
	}

	for (int i = 0; i < coedge_points_vec.size(); i++)
	{
		double x = coedge_points_vec[i].x();
		double y = coedge_points_vec[i].y();
		double z = coedge_points_vec[i].z();
		LOG_INFO("coedge_points_vec[%d]: (%.5lf, %.5lf, %.5lf)", i, x, y, z);
	}
}

void Exp5::Exp5::FacesCheck()
{

	for (int i = 0; i < bodies_to_be_checked.count(); i++) {
		ENTITY* ibody_ptr = (bodies_to_be_checked[i]);

		ENTITY_LIST face_list;

		api_get_faces(ibody_ptr, face_list);


		// 获取面
		for (int j = 0; j < face_list.count(); j++) {
			FACE* face_ptr = dynamic_cast<FACE*>(face_list[j]);

			// 遍历边
			if (face_ptr) {
				LOG_INFO("Checking faces: %d", MarkNum::GetId(face_ptr));

				LOOP* iloop = face_ptr->loop();

				if (iloop) {
					COEDGE* icoedge = iloop->start();

					do {
						if (icoedge == nullptr) {
							break;
						}

						EDGE* iedge = icoedge->edge();

						if (iedge) {
							EdgeAndCoedgeGeometryCheck(iedge, icoedge);
						}

						icoedge = icoedge->next();
					} while (icoedge != nullptr && icoedge != iloop->start());

				}

			}

		}

	}




}

void Exp5::Exp5::StartExperiment()
{
	LOG_INFO("start.");

	this->FacesCheck();

	LOG_INFO("end.");
}
