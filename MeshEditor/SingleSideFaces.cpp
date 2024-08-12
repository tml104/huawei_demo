#include "StdAfx.h"
#include "SingleSideFaces.h"

void SingleSideFaces::SingleSideFacesFixer::FindSingleSideFaces()
{
	for (int i = 0; i < bodies.count(); i++) {

		ENTITY* ibody_ptr = (bodies[i]);
		ENTITY_LIST face_list;

		api_get_faces(ibody_ptr, face_list);

		// face
		for (int j = 0; j < face_list.count(); j++) {
			FACE* face_ptr = dynamic_cast<FACE*>(face_list[j]);
			bool is_single_side_face = true;

			// 遍历环
			LOOP* iloop = face_ptr->loop();
			do {
				if (iloop == nullptr) {
					LOG_ERROR("iloop is nullptr");
					break;
				}

				// 遍历边
				COEDGE* icoedge = iloop->start();
				do {
					if (icoedge == nullptr) {
						LOG_ERROR("icoedge is nullptr");
						break;
					}

					// 检查是否为单面边非流形边
					int count = Utils::PartnerCount(icoedge);
					if (count != 1) {
						is_single_side_face = false;
						break;
					}


					icoedge = icoedge->next();
				} while (icoedge != nullptr && icoedge != iloop->start());

				if (is_single_side_face == false) {
					break;
				}

				iloop = iloop->next();
			} while (iloop != nullptr && iloop != face_ptr->loop());
		
			if (is_single_side_face) {
				this->single_side_faces.insert(face_ptr);
				LOG_DEBUG("is single-side face: %d", MarkNum::GetId(face_ptr));
			}
		}
	}
}

void SingleSideFaces::SingleSideFacesFixer::SetFacesToDoubleSide()
{
	for (auto it = this->single_side_faces.begin(); it != this->single_side_faces.end(); it++) {
		(*it)->set_sides(DOUBLE_SIDED);
		LOG_DEBUG("Setting single-side face to double-side: %d", MarkNum::GetId(*it));
	}
}

void SingleSideFaces::SingleSideFacesFixer::Start()
{
	LOG_INFO("Start.");

	api_initialize_constructors();
	api_initialize_booleans();

	FindSingleSideFaces();
	SetFacesToDoubleSide();

	api_terminate_constructors();
	api_terminate_booleans();

	LOG_INFO("End.");

}

void SingleSideFaces::SingleSideFacesFixer::Clear()
{
	this->single_side_faces.clear();
}
