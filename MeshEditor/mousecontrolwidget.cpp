#include "StdAfx.h"
#include "mousecontrolwidget.h"
#include "hoopsview.h"
#include "topologyoptwidget.h"
#include <geom_utl.hxx>

MouseControlWidget::MouseControlWidget(HoopsView *_hoopsview, QWidget *parent)
	: hoopsview (_hoopsview), QWidget(parent)
{
	ui.setupUi(this);
	setup_actions ();
	ui.stackedWidget->setCurrentIndex (1);
	block_gen_widget = (BlockGenWidget *)parent;
}

MouseControlWidget::~MouseControlWidget()
{

}

void MouseControlWidget::setup_actions ()
{
	connect (ui.radioButton_Selection, SIGNAL (clicked ()), SLOT (on_begin_selection ()));
	connect (ui.radioButton_Camera_Manipulate, SIGNAL (clicked ()), SLOT (on_begin_camera_manipulate ()));

	connect (ui.radioButton_Select_By_Click, SIGNAL (clicked ()), hoopsview, SLOT (begin_select_by_click ()));
	connect (ui.radioButton_Select_By_Rectangle, SIGNAL (clicked ()), hoopsview, SLOT (begin_select_by_rectangle ()));
	connect (ui.radioButton_Select_By_Polygon, SIGNAL (clicked ()), hoopsview, SLOT (begin_select_by_polygon ()));

	connect (ui.radioButton_Select_Vertices, SIGNAL (toggled (bool)), hoopsview, SLOT (set_vertices_selectable (bool)));
	connect (ui.radioButton_Select_Edges, SIGNAL (toggled (bool)), hoopsview, SLOT (set_edges_selectable (bool)));
	connect (ui.radioButton_Select_Faces, SIGNAL (toggled (bool)), hoopsview, SLOT (set_faces_selectable (bool)));
}

void MouseControlWidget::on_begin_selection ()
{
	ui.stackedWidget->setCurrentIndex (0);
	if (ui.radioButton_Select_By_Click->isChecked ())
		hoopsview->begin_select_by_click ();
	else if (ui.radioButton_Select_By_Rectangle->isChecked ())
		hoopsview->begin_select_by_rectangle ();
	else if (ui.radioButton_Select_By_Polygon->isChecked ())
		hoopsview->begin_select_by_polygon ();
}

void MouseControlWidget::on_begin_camera_manipulate ()
{
	ui.stackedWidget->setCurrentIndex (1);
	hoopsview->begin_camera_manipulate ();
}
