#ifndef _ONE_SIMPLE_SHEET_H_
#define _ONE_SIMPLE_SHEET_H_

#include <string>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <algorithm>
#include <hash_map>
#include <queue>
#include "MeshDefs.h"
#include "DualDefs.h"
#include "FuncDefs.h"
#include "acis_headers.h"
#include "hoopsview.h"
#include "OneSimpleHandlerBase.h"

class OneSimpleSheetInflationHandler
{
public:
	OneSimpleSheetInflationHandler (VolumeMesh *_mesh, BODY *_body, HoopsView *_hoopsview);
	~OneSimpleSheetInflationHandler(){};
public:
	void init ();
	bool analyze_input_edges (std::unordered_set<OvmEgH> &_input_ehs);
	std::unordered_set<OvmEgH> complete_loop ();
	void set_constant_fhs (std::unordered_set<OvmFaH> &_constant_fhs);
	void set_minimum_depth (int _min_depth);
	void set_interface (FACE *_inter_face);
	QString get_last_err () {return last_err_str;}
	bool is_complete () {return end_vhs.empty ();}
	
	std::unordered_set<OvmCeH> get_hexa_set ();
	std::unordered_set<OvmFaH> get_inflation_fhs ();
	DualSheet * sheet_inflation ();
	void reprocess ();
	void set_inter_face (FACE *_inter_face) {inter_face = _inter_face;}
	FACE *get_inter_face () {return inter_face;}
	void set_const_face (FACE *_const_face) {const_face = _const_face;}
	FACE *get_const_face () {return const_face;}
	void update_inter_face_fhs ();
	void update_const_face_fhs ();
public:

private:
	VolumeMesh *mesh;
	BODY *body;
	HoopsView *hoopsview;
	FACE *inter_face, *const_face;
	bool passed_analyzed;
	std::unordered_set<OvmEgH> input_ehs;
	std::unordered_set<OvmFaH> constant_fhs, interface_fhs, inflation_fhs;
	QString last_err_str;
	std::unordered_set<OvmVeH> end_vhs;
	std::vector<OvmVeH> int_vhs;
	std::hash_map<OvmVeH, std::vector<OvmEgH> > vh_adj_ehs_mapping;
	OneSimpleHandlerBase *base_handler;
	std::unordered_set<OvmCeH> hexa_set;
};

#endif