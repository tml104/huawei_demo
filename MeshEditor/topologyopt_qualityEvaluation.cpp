#include "StdAfx.h"
#include "topologyoptwidget.h"
#include "GeometryFuncs.h"
#include "TopologyQualityPredict.h"
#include "DualOperations.h"
#include <QDialog>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
std::hash_map<OvmCeH, double> jacobian_cell;

void TopologyOptWidget::on_mesh_quality_evaluation()
{
	int hex_num = mesh->n_cells();
	if (hex_num == 0) return;
	std::vector<double> vec;

	double max_jacobian = -1e20;
	double min_jacobian = 1e20;
	double mean_jacobian = 0;
	double std_dev_jacobian = 0;
	for(OpenVolumeMesh::CellIter c_iter = mesh->cells_begin();c_iter != mesh->cells_end();c_iter++){
		HexJacobianTensor jac_metric = jacobian_metric_tensor(mesh,*c_iter);
		double jac = scaled_jacobian_metric(jac_metric);
		if(jac > max_jacobian)
			max_jacobian = jac;
		if (jac < min_jacobian)
			min_jacobian = jac;
		mean_jacobian = mean_jacobian + jac;
		vec.push_back(jac);
		jacobian_cell[*c_iter] = jac;
	}

	mean_jacobian = mean_jacobian / hex_num;

	int topology_mark(0);
	std::map<OvmEgH, EdgeAttribute> edge_property;
	get_edge_property(mesh, edge_property);
	for(auto map_iter = edge_property.begin(); map_iter != edge_property.end(); map_iter++){
		if(mesh->is_boundary(map_iter->first))
			topology_mark += (map_iter->second.idealvalence-map_iter->second.valence)*(map_iter->second.idealvalence-map_iter->second.valence)*3;
		else
			topology_mark += (map_iter->second.idealvalence-map_iter->second.valence)*(map_iter->second.idealvalence-map_iter->second.valence);
	}

	for(int i = 0; i < hex_num;i++){
		std_dev_jacobian = std_dev_jacobian + (vec[i] - mean_jacobian) * (vec[i] - mean_jacobian);
	}
	std_dev_jacobian = sqrt(std_dev_jacobian / hex_num);

	int max_vertex_valence(-1000),min_vertex_valence(1000),max_edge_valence(-1000),min_edge_valence(1000),mean_vertex_valence(0),mean_edge_valence(0);
	double std_dev_vertex_valence(0),std_dev_edge_valence(0);

	for(auto vh_iter = mesh->vertices_begin(); vh_iter != mesh->vertices_end(); vh_iter++){
		int valence = mesh->valence(*vh_iter);
		if(max_vertex_valence < valence)
			max_vertex_valence = valence;
		if(min_vertex_valence > valence)
			min_vertex_valence = valence;
		mean_vertex_valence += valence;
	}
	mean_vertex_valence /= mesh->n_vertices();
	for(auto vh_iter = mesh->vertices_begin(); vh_iter != mesh->vertices_end(); vh_iter++){
		std_dev_vertex_valence += (mesh->valence(*vh_iter) - mean_vertex_valence) * (mesh->valence(*vh_iter) - mean_vertex_valence);
	}
	std_dev_vertex_valence = sqrt(std_dev_vertex_valence/mesh->n_vertices());

	for(auto eh_iter = mesh->edges_begin(); eh_iter != mesh->edges_end(); eh_iter++){
		int valence = mesh->valence(*eh_iter);
		if(max_edge_valence < valence)
			max_edge_valence = valence;
		if(min_edge_valence > valence)
			min_edge_valence = valence;
		mean_edge_valence += valence;
	}
	mean_edge_valence /= mesh->n_edges();
	for(auto eh_iter = mesh->edges_begin(); eh_iter != mesh->edges_end(); eh_iter++){
		std_dev_edge_valence += (mesh->valence(*eh_iter) - mean_edge_valence) * (mesh->valence(*eh_iter) - mean_edge_valence);
	}
	std_dev_edge_valence = sqrt(std_dev_edge_valence/mesh->n_edges());

	int singular_edge_count = 0;
	for (auto edge_iter = mesh->edges_begin(); edge_iter != mesh->edges_end(); ++edge_iter)
	{
		if (!mesh->is_boundary(*edge_iter))
		{
			if(edge_property[*edge_iter].valence != 4)
				singular_edge_count++;
		}
	}



	std::stringstream ss;
	ss << hex_num;
	std::string str = ss.str();
	QString s = QString::fromStdString(str);
	quality_evaluation_ptr->show();
	quality_evaluation_ptr->get_quality_evaluation_ui().hex_num->setText(s);

	ss.str("");
	ss << mesh->n_vertices();
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->show();
	quality_evaluation_ptr->get_quality_evaluation_ui().vertex_num->setText(s);

	ss.str("");
	ss << mesh->n_edges();
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->show();
	quality_evaluation_ptr->get_quality_evaluation_ui().edge_num->setText(s);

	ss.str("");
	SheetSet sheets;
	retrieve_sheets(mesh, sheets);
	ss << sheets.size();
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->show();
	quality_evaluation_ptr->get_quality_evaluation_ui().sheet_num->setText(s);

	ss.str("");
	ss << max_jacobian;
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->get_quality_evaluation_ui().max_jacobian->setText(s);

	ss.str("");
	ss << min_jacobian;
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->get_quality_evaluation_ui().min_jacobian->setText(s);

	ss.str("");
	ss << mean_jacobian;
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->get_quality_evaluation_ui().mean_jacobian->setText(s);

	ss.str("");
	ss << std_dev_jacobian;
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->get_quality_evaluation_ui().std_dev_jacobian->setText(s);

	ss.str("");
	ss << topology_mark;
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->get_quality_evaluation_ui().topology_mark->setText(s);

	ss.str("");
	ss << max_vertex_valence;
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->get_quality_evaluation_ui().max_vertex_valence->setText(s);

	ss.str("");
	ss << min_vertex_valence;
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->get_quality_evaluation_ui().min_vertex_valence->setText(s);

	ss.str("");
	ss << mean_vertex_valence;
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->get_quality_evaluation_ui().mean_vertex_valence->setText(s);

	ss.str("");
	ss << std_dev_vertex_valence;
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->get_quality_evaluation_ui().std_dev_vertex_valence->setText(s);

	ss.str("");
	ss << max_edge_valence;
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->get_quality_evaluation_ui().max_edge_valence->setText(s);

	ss.str("");
	ss << min_edge_valence;
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->get_quality_evaluation_ui().min_edge_valence->setText(s);

	ss.str("");
	ss << mean_edge_valence;
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->get_quality_evaluation_ui().mean_edge_valence->setText(s);

	ss.str("");
	ss << std_dev_edge_valence;
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->get_quality_evaluation_ui().std_dev_edge_valence->setText(s);

	ss.str("");
	ss << singular_edge_count;
	str = ss.str();
	s = QString::fromStdString(str);
	quality_evaluation_ptr->get_quality_evaluation_ui().singularEdge->setText(s);
}

void TopologyOptWidget::on_show_cells()
{
	std::unordered_set<OvmCeH> hexas;
	hoopsview->derender_all_mesh_groups();
	for(auto iter = jacobian_cell.begin(); iter != jacobian_cell.end(); iter++)
		if(iter->second <= ui.doubleSpinBox_jacobian_threshold->value())
			hexas.insert(iter->first);
	auto group = new VolumeMeshElementGroup (mesh, "highlight", "highlight elements");
	group->chs = hexas;
	hoopsview->render_mesh_group (group, false);
}
