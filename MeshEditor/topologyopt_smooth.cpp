#include "StdAfx.h"
#include "topologyoptwidget.h"
#include "mesh_optimization.h"

void TopologyOptWidget::on_start_smoothing ()
{
	int smooth_rounds = ui.spinBox_Smooth_Rounds->value ();
	smooth_volume_mesh (mesh, body, smooth_rounds);
	hoopsview->rerender_hexamesh (mesh);
	hoopsview->update_mesh_groups ();
}

void TopologyOptWidget::on_start_optimization ()
{
	int smooth_rounds = ui.spinBox_Smooth_Rounds_2->value ();
	smooth_volume_mesh_yws_inner (mesh, body, smooth_rounds);
	hoopsview->rerender_hexamesh (mesh);
	hoopsview->update_mesh_groups ();
}

void TopologyOptWidget::on_start_optimization_all ()
{
	int smooth_rounds = ui.spinBox_Smooth_Rounds_3->value ();
	smooth_volume_mesh_yws_all (mesh, body, smooth_rounds);
	hoopsview->rerender_hexamesh (mesh);
	hoopsview->update_mesh_groups ();
}