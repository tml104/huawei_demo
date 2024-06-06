#include "StdAfx.h"
#include "topologyoptwidget.h"

void TopologyOptWidget::on_select_sheet_for_lse ()
{
	if (osse_handler) delete osse_handler;
	osse_handler = new OneSimpleSheetExtractionHandler (mesh, body, hoopsview);
	//	osse_handler->init ();

	std::vector<OvmEgH> selected_ehs;
	if (!hoopsview->get_selected_elements (std::vector<OvmVeH>(), selected_ehs, std::vector<OvmFaH> ())){
		QMessageBox::warning (this, tr("警告"), tr("未选中任何网格边！"), QMessageBox::Ok);
		return;
	}
	retrieve_sheets (mesh, sheet_set);
	OvmEgH eh = selected_ehs.front ();
	auto E_SHEET_PTR = mesh->request_edge_property<unsigned long> ("sheetptr");
	DualSheet *sheet = (DualSheet*)(E_SHEET_PTR[eh]);
	osse_handler->set_sheet_to_extract (sheet);
	hoopsview->render_mesh_group (sheet, false, 0, 0, 0, "red");

	std::unordered_set<OvmEgH> ehs_in_sheet(sheet->ehs);
	std::unordered_set<OvmVeH> vhs_in_sheet;
	foreach(auto &eh, ehs_in_sheet){
		vhs_in_sheet.insert(mesh->edge(eh).from_vertex());
		vhs_in_sheet.insert(mesh->edge(eh).to_vertex());
	}
	std::vector<OvmVeH> vhs1_vec;
	vhs1_vec.push_back(*(vhs_in_sheet.begin()));
	vhs_in_sheet.erase(*(vhs_in_sheet.begin()));
	for(int i = 0; i < vhs1_vec.size(); i++){
		std::unordered_set<OvmVeH> adj_vhs;
		get_adj_vertices_around_vertex(mesh, vhs1_vec[i], adj_vhs);
		foreach(auto &vh, adj_vhs){
			if(!contains(vhs_in_sheet, vh))
				continue;
			if(contains(ehs_in_sheet, mesh->edge_handle(mesh->halfedge(vh, vhs1_vec[i]))))
				continue;
			vhs1_vec.push_back(vh);
			vhs_in_sheet.erase(vh);
		}
	}
	std::unordered_set<OvmVeH> vhs_one_side;
	vector_to_unordered_set(vhs1_vec, vhs_one_side);
	auto group1 = new VolumeMeshElementGroup (mesh, "vhs", "vhs");
	group1->vhs = vhs_one_side;
	hoopsview->render_mesh_group (group1, false, "yellow", NULL, NULL, NULL);
}

void TopologyOptWidget::on_extract_sheet_for_lse ()
{
	std::unordered_set<OvmFaH> result_fhs;
	one_simple_sheet_extraction (mesh, osse_handler->get_sheet_to_extract (), result_fhs);
	osse_handler->update_constant_fhs ();
	osse_handler->update_interface_fhs ();
	std::unordered_set<OvmEgH> result_ehs, ehs_on_constant_fhs, ehs_on_result_fhs;
	foreach (auto &fh, osse_handler->get_constant_fhs ()){
		auto heh_vec = mesh->face (fh).halfedges ();
		foreach (auto &heh, heh_vec){
			auto eh = mesh->edge_handle (heh);
			ehs_on_constant_fhs.insert (eh);
		}
	}
	foreach (auto &fh, result_fhs){
		auto heh_vec = mesh->face (fh).halfedges ();
		foreach (auto &heh, heh_vec){
			auto eh = mesh->edge_handle (heh);
			ehs_on_result_fhs.insert (eh);
		}
	}
	foreach (auto &eh, ehs_on_result_fhs){
		if (contains (ehs_on_constant_fhs, eh))
			result_ehs.insert (eh);
	}
	osse_handler->set_extract_result_ehs (result_ehs);

	hoopsview->derender_all_mesh_groups ();
	//smooth_volume_mesh (mesh, body, 3);
	hoopsview->rerender_hexamesh (mesh);

	auto group = new VolumeMeshElementGroup (mesh, "lse", "result faces");
	group->fhs = result_fhs;
	hoopsview->render_mesh_group (group, false, NULL, NULL, "(diffuse=red,transmission = (r=0.2 g=0.2 b=0.2))");

	group = new VolumeMeshElementGroup (mesh, "lse", "interface faces");
	group->fhs = osse_handler->get_interface_fhs ();
	hoopsview->render_mesh_group (group, false, NULL, NULL, "(diffuse=pink,transmission = (r=0.2 g=0.2 b=0.2))");

	group = new VolumeMeshElementGroup (mesh, "lse", "constant faces");
	group->fhs = osse_handler->get_constant_fhs ();
	hoopsview->render_mesh_group (group, false, NULL, NULL, "(diffuse=green,transmission = (r=0.2 g=0.2 b=0.2))");

	group = new VolumeMeshElementGroup (mesh, "lse", "result edges");
	group->ehs = result_ehs;
	hoopsview->render_mesh_group (group, false, NULL, NULL, "(diffuse=green,transmission = (r=0.2 g=0.2 b=0.2))");
}