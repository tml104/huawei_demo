#ifndef MOUSECONTROLWIDGET_H
#define MOUSECONTROLWIDGET_H

#include <QWidget>
#include "ui_mousecontrolwidget.h"
#include "hoopsview.h"

class BlockGenWidget;
class MouseControlWidget : public QWidget
{
	Q_OBJECT

public:
	MouseControlWidget(HoopsView *_hoopsview, QWidget *parent = 0);
	~MouseControlWidget();
private:
	void setup_actions ();
signals:
	void begin_selection_by_click ();
	void begin_selection_by_rectangle ();
	void begin_selection_by_polygon ();
	void begin_camera_manipulate ();
	
	void select_vertices (bool onoff);
	void select_edges (bool onoff);
	void select_faces (bool onoff);
private slots:
	void on_begin_selection ();
	void on_begin_camera_manipulate ();
private:
	Ui::MouseControlWidget ui;
	HoopsView *hoopsview;
	BlockGenWidget *block_gen_widget;
};

#endif // MOUSECONTROLWIDGET_H
