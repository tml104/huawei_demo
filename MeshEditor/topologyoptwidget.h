#ifndef TOPOLOGYOPTWIDGET_H
#define TOPOLOGYOPTWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QTabWidget>
#include "ui_topologyoptwidget.h"
#include "hoopsview.h"
//#include "meshrendercontrolwidget.h"
#include "mousecontrolwidget.h"
#include "filecontrolwidget.h"
#include "groupcontrolwidget.h"
//#include "OneSimpleSheetInflation.h"
//#include "OneSimpleSheetExtraction.h"
//#include "MeshDefs.h"
//#include "FuncDefs.h"
//#include "DualDefs.h"
//#include "DualOperations.h"
//#include "GeometryFuncs.h"
//#include "TopologyQualityPredict.h"
//#include "PrioritySetManager.h"
//#include "MeshOptimization.h"
//#include "quality_evaluation.h"
#include "ohm_struct.h"
#include "pixel.h"
#include "Cell.h"

#include "HQHEntrance.h"

class Frame;


class TopologyOptWidget : public QWidget
{
	Q_OBJECT
		
public:
	TopologyOptWidget(QWidget *parent = 0);
	~TopologyOptWidget();
public:
	void load_model ( QString model_path);
	std::vector<std::vector<QToolBar*> > get_toolbars () {return toolbars;}
	//VolumeMesh *get_mesh () {return mesh;}
private:
	void setup_actions ();
	void show_one_cell_real(Ohm_slice::Cell& cell);
private slots:
	void on_open_file (QString file_path);
	void on_save_file ();
	void on_save_file_as (QString file_path);
	void on_file_close ();


private:
	Ui::TopologyOptWidget ui;
	HoopsView *hoopsview;
	//VolumeMesh *mesh;
	//OP_Mesh *bs_mesh;
	BODY *body;
	QString xml_file_path, /*mesh_file_path, */model_file_path;
	//SheetSet sheet_set;
	//ChordSet chord_set;

	std::vector<std::vector<QToolBar*> > toolbars;
	MouseControlWidget *mouse_controller;
	GroupControlWidget *group_controller;
	FileControlWidget *file_controller;

};

void get_initial_frame(VolumeMesh *mesh, std::hash_map<OvmCeH, Frame> & cell_frame_mapping);
#endif // BLOCKGenWIDGET_H
