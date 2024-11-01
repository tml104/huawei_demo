#include "StdAfx.h"
#include "DegeneratedFaces.h"


void DegeneratedFaces::DegeneratedFacesFixer::FindDegeneratedFaces() 
{
	for (int i = 0; i < bodies.count(); i++) {

		ENTITY* ibody_ptr = (bodies[i]);
		ENTITY_LIST face_list;

		api_get_faces(ibody_ptr, face_list);

		// face
		for (int j = 0; j < face_list.count(); j++) {
			FACE* face_ptr = dynamic_cast<FACE*>(face_list[j]);


			// 先判断一下这个面是否确实存在非流形，然后再计算面积
			LOOP* loop = face_ptr->loop();
			bool has_nonmanifold = GeometryUtils::CheckLoopHasNonmanifoldEdge(loop);
			if (has_nonmanifold)
			{
				// 计算面积
				double area;
				double est_rel_accy_achieved;
				api_ent_area(face_ptr, REQ_REL_ACCY, area, est_rel_accy_achieved);

				if (area <= THRESHOLD_AREA && area >= -THRESHOLD_AREA) {
					this->degenerated_faces.insert(face_ptr);
					degenerated_face_count++;
					LOG_DEBUG("area of face %d (body: %d): %.8lf", MarkNum::GetId(face_ptr), MarkNum::GetBody(face_ptr), area);
					LOG_DEBUG("face %d (body: %d): degenerated.", MarkNum::GetId(face_ptr), MarkNum::GetBody(face_ptr));
				}

			}

		}
	}
}

/* 调用 api_remove_face 来移除退化面 */
void DegeneratedFaces::DegeneratedFacesFixer::RemoveDegeneratedFaces() 
{
	for (auto it = this->degenerated_faces.begin(); it != this->degenerated_faces.end(); it++) {
		api_remove_face(*it);
		LOG_DEBUG("Removing face: %d, body: %d", MarkNum::GetId(*it), MarkNum::GetBody(*it));
	}
}

void DegeneratedFaces::DegeneratedFacesFixer::Status()
{
	if (degenerated_face_count > 0) {
		LOG_INFO("=== Status for DegeneratedFacesFixer ===");
		LOG_INFO("xloop_count: %d", degenerated_face_count);
		LOG_INFO("=== End ===");
	}
}

bool DegeneratedFaces::DegeneratedFacesFixer::Start()
{
	LOG_INFO("Start.");

	api_initialize_constructors();
	api_initialize_booleans();

	FindDegeneratedFaces();
	RemoveDegeneratedFaces();

	Status();

	api_terminate_constructors();
	api_terminate_booleans();

	LOG_INFO("End.");

	return (degenerated_face_count > 0);
}

void DegeneratedFaces::DegeneratedFacesFixer::Clear() 
{
	degenerated_face_count = 0;
	this->degenerated_faces.clear();
}
