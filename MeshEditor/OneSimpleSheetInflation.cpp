#include "stdafx.h"
#include "OneSimpleSheetInflation.h"
#include <limits>
#include <algorithm>
#include <iterator>

OneSimpleSheetInflationHandler::OneSimpleSheetInflationHandler (VolumeMesh *_mesh, BODY *_body, HoopsView *_hoopsview)
	: mesh (_mesh), body (_body), hoopsview (_hoopsview)
{
	inter_face = NULL;
	base_handler = NULL;
}

void OneSimpleSheetInflationHandler::init ()
{
	last_err_str = "";
	end_vhs.clear (); int_vhs.clear ();
	constant_fhs.clear ();
	inflation_fhs.clear ();
	interface_fhs.clear ();
	passed_analyzed = false;
	input_ehs.clear ();
}


std::unordered_set<OvmFaH> OneSimpleSheetInflationHandler::get_inflation_fhs ()
{
	inflation_fhs.clear ();
	auto inflation_fhs = base_handler->get_inflation_fhs ();
	if (inflation_fhs.empty ()){
		last_err_str = base_handler->get_last_err ();
		delete base_handler;
	}
	return inflation_fhs;
}

DualSheet * OneSimpleSheetInflationHandler::sheet_inflation ()
{
	if (!base_handler) return NULL;
	//下面进行sheet生成
	DualSheet *new_sheet = base_handler->inflate_new_sheet ();
	delete base_handler;
	return new_sheet;
}

void OneSimpleSheetInflationHandler::reprocess ()
{
	auto V_PREV_HANDLE = mesh->request_vertex_property<OvmVeH> ("prevhandle");
	for (auto v_it = mesh->vertices_begin (); v_it != mesh->vertices_end (); ++v_it)
		V_PREV_HANDLE[*v_it] = *v_it;

	auto E_PREV_HANDLE = mesh->request_edge_property<OvmEgH> ("prevhandle");
	for (auto e_it = mesh->edges_begin (); e_it != mesh->edges_end (); ++e_it)
		E_PREV_HANDLE[*e_it] = *e_it;

	auto F_PREV_HANDLE = mesh->request_face_property<OvmFaH> ("prevhandle");
	for (auto c_it = mesh->faces_begin (); c_it != mesh->faces_end (); ++c_it)
		F_PREV_HANDLE[*c_it] = *c_it;

	auto C_PREV_HANDLE = mesh->request_cell_property<OvmCeH> ("prevhandle");
	for (auto c_it = mesh->cells_begin (); c_it != mesh->cells_end (); ++c_it)
		C_PREV_HANDLE[*c_it] = *c_it;
}

void OneSimpleSheetInflationHandler::set_constant_fhs (std::unordered_set<OvmFaH> &_constant_fhs)
{
	constant_fhs = _constant_fhs;
}


void OneSimpleSheetInflationHandler::set_interface (FACE *_inter_face)
{
	inter_face = _inter_face;
}

void OneSimpleSheetInflationHandler::update_inter_face_fhs ()
{
	interface_fhs.clear ();
	get_fhs_on_acis_face (mesh, inter_face, interface_fhs);
}

void OneSimpleSheetInflationHandler::update_const_face_fhs ()
{
	constant_fhs.clear ();
	get_fhs_on_acis_face (mesh, const_face, constant_fhs);
}