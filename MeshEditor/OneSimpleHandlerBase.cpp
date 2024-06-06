#include "StdAfx.h"
#include "OneSimpleHandlerBase.h"

OneSimpleHandlerBase::OneSimpleHandlerBase(VolumeMesh *_mesh, std::unordered_set<OvmEgH> &_input_ehs)
	: mesh (_mesh), input_ehs (_input_ehs)
{
	constant_fhs_ptr = NULL;
}


OneSimpleHandlerBase::~OneSimpleHandlerBase(void)
{
}

void OneSimpleHandlerBase::get_inflation_fhs_from_hexa_set (std::unordered_set<OvmCeH> &hexa_set)
{
	inflation_fhs.clear ();
	std::unordered_set<OvmFaH> bound_fhs;
	collect_boundary_element (mesh, hexa_set, NULL, NULL, &bound_fhs);
	foreach (auto &fh, bound_fhs){
		if (!mesh->is_boundary (fh)) inflation_fhs.insert (fh);
	}
}