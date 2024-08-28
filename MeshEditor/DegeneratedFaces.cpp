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
			ENTITY* ptr = face_list[j];

			// 计算面积
			double area;
			double est_rel_accy_achieved;
			api_ent_area(ptr, REQ_REL_ACCY, area, est_rel_accy_achieved);

			if (area <= THRESHOLD_AREA && area >= -THRESHOLD_AREA) {
				this->degenerated_faces.insert(dynamic_cast<FACE*>(ptr));
				LOG_DEBUG("area of face %d: %.5lf", MarkNum::GetId(ptr), area);
				LOG_DEBUG("face %d: degenerated.", MarkNum::GetId(ptr));
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

void DegeneratedFaces::DegeneratedFacesFixer::Start()
{
	LOG_INFO("Start.");

	api_initialize_constructors();
	api_initialize_booleans();

	FindDegeneratedFaces();
	RemoveDegeneratedFaces();

	api_terminate_constructors();
	api_terminate_booleans();

	LOG_INFO("End.");
}

void DegeneratedFaces::DegeneratedFacesFixer::Clear() 
{
	this->degenerated_faces.clear();
}
