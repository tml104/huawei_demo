#pragma once
#include "MeshDefs.h"
#include "DualDefs.h"
#include "FuncDefs.h"

class HoopsView;

class OneSimpleHandlerBase
{
public:
	OneSimpleHandlerBase(VolumeMesh *_mesh, std::unordered_set<OvmEgH> &_input_ehs);
	~OneSimpleHandlerBase(void);
public:
	virtual std::unordered_set<OvmCeH> get_hexa_set () = 0;
	virtual std::unordered_set<OvmFaH> get_inflation_fhs () = 0;
	virtual DualSheet * inflate_new_sheet () = 0;
	QString get_last_err () {return last_err_str;}
	void set_inter_face_fhs (std::unordered_set<OvmFaH> &_inter_fhs){interface_fhs = _inter_fhs;}
protected:
	void get_inflation_fhs_from_hexa_set (std::unordered_set<OvmCeH> &hexa_set);
protected:
	VolumeMesh *mesh;
	BODY *body;
	HoopsView *hoopsview;
	FACE *inter_face;

	std::unordered_set<OvmEgH> input_ehs;
	std::vector<std::unordered_set<OvmFaH> > separated_fhs_patches;
	QString last_err_str;
	std::unordered_set<OvmFaH> interface_fhs, boundary_fhs, optimize_allowed_fhs, inflation_fhs;
	std::unordered_set<OvmFaH> *constant_fhs_ptr;
	std::unordered_set<OvmVeH> end_vhs, intersect_allowed_vhs;
	std::vector<OvmVeH> int_vhs;
	std::hash_map<OvmVeH, std::vector<OvmEgH> > *vh_adj_ehs_mapping;
};

