#include "StdAfx.h"
#include "DegeneratedFace.h"


void DegeneratedFace::DegeneratedFaceFixer::FindDegeneratedFaces() 
{
	for (int i = 0; i < bodies.count(); i++) {

		ENTITY* ibody_ptr = (bodies[i]);
		ENTITY_LIST face_list;

		api_get_faces(ibody_ptr, face_list);

		// face
		for (int j = 0; j < face_list.count(); j++) {
			ENTITY* ptr = face_list[j];

			// �������
			double area;
			double est_rel_accy_achieved;
			api_ent_area(ptr, REQ_REL_ACCY, area, est_rel_accy_achieved);

			LOG_DEBUG("area of face %d: %.5lf", MarkNum::GetId(ptr), area);

			if (area <= THRESHOLD_AREA) {
				this->degenerated_faces.insert(dynamic_cast<FACE*>(ptr));
				LOG_DEBUG("face %d: degenerated.", MarkNum::GetId(ptr));
			}
		}
	}
}

/* ���� api_remove_face ���Ƴ��˻��� */
void DegeneratedFace::DegeneratedFaceFixer::RemoveDegeneratedFaces() 
{
	for (auto it = this->degenerated_faces.begin(); it != this->degenerated_faces.end(); it++) {
		LOG_DEBUG("Removing face %d", MarkNum::GetId(*it));
		api_remove_face(*it);
	}
}

void DegeneratedFace::DegeneratedFaceFixer::Start()
{
	api_initialize_constructors();
	api_initialize_booleans();

	FindDegeneratedFaces();
	RemoveDegeneratedFaces();

	api_terminate_constructors();
	api_terminate_booleans();
}

void DegeneratedFace::DegeneratedFaceFixer::Clear() 
{
	this->degenerated_faces.clear();
}
