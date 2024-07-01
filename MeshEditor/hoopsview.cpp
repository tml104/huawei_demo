#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
// qt includes
#include <QPrinter>
#include <QFileDialog>
#include <QLabel> 
#include <QMessageBox>
#include <QMenu> 
#include <QCursor> 
#include <QSlider>
#include <QLayout>
#include <QLineEdit>
#include <QTimer>
#include <QColorDialog>
#include <QColor>
#include <QMenu>
#include <QFileInfo>
#include <QToolBar>
#include <QAction>

//ACIS includes
#include "kernapi.hxx"
#include "ha_bridge.h"
// hoops_mvo includes
#include "HDB.h"

#include "HBaseModel.h"
#include "HBaseView.h"

#include "HModelInfo.h"
#include "HEventInfo.h"
#include "HOpCameraOrbit.h"
#include "HOpCameraZoom.h"
#include "HOpCameraZoomBox.h"
#include "HOpCameraPan.h"
#include "HOpCameraManipulate.h"
#include "HOpCreateCuttingPlane.h"
#include "HOpCreateSphere.h"
#include "HOpCreateCone.h"
#include "HOpCreateCylinder.h"
#include "HOpConstructRectangle.h"
#include "HOpCreateRectangle.h"
#include "HOpSelectAperture.h"
#include "HOpSelectPolygon.h"
#include "HOpSelectArea.h"
#include "HOpCameraOrbitTurntable.h"
#include "HOpCreateBoolean.h"
#include "HOpMoveHandle.h"
#include "HUtilityGeometryCreation.h"
#include "HUtilityGeomHandle.h"
#include <HMarkupManager.h>
#include "HUtility.h"
#include "HStream.h"
#include "HSelectionSet.h"
#include "HStreamFileToolkit.h"
#include "HIOManager.h"
#include "HUtilitySubwindow.h"
#include "HUtilityAnnotation.h"

#undef null

// the qt/hoops base class
#include "hoopsview.h"
#include <HSelectionItem.h>
// hoops include
#include "hc.h"


// this is setup in main
extern HDB * m_pHDB;


#define Debug_USE_QGL_CONTEXT           0x00000080

HoopsView::HoopsView(QWidget* parent)
	: HQWidget(parent)
{


	// Create and initialize HOOPS/MVO Model and View objects 
	m_pHBaseModel = new HBaseModel();
	m_pHBaseModel->Init();

	// Initialize View object to null ; gets created in SimpleHQWidget::Init 
	m_pHView = 0;

	// enable MouseMoveEvents  
	setMouseTracking(true);

	// enable key events 
	setEnabled(true);
	setFocusPolicy(Qt::StrongFocus);

	createActions();
}


HoopsView::~HoopsView()
{
	// Destructor  

	   // Clean up memory 
	if (m_pHView)        delete m_pHView;
	if (m_pHBaseModel)   delete m_pHBaseModel;

}

void HoopsView::createActions()
{

}

void HoopsView::createModels()
{
	HPoint pts[] = {
		HPoint(0, 0, 0), HPoint(1, 0, 0), HPoint(1, 1, 0), HPoint(0, 1, 0),
		HPoint(0, 0, 1), HPoint(1, 0, 1), HPoint(1, 1, 1), HPoint(0, 1, 1)
	};

	int face_list[] = {
		4, 0, 3, 2, 1,
		4, 0, 1, 5, 4,
		4, 1, 2, 6, 5,
		4, 2, 3, 7, 6,
		4, 3, 0, 4, 7,
		4, 4, 5, 6, 7
	};

	HC_Open_Segment_By_Key(m_pHView->GetModelKey());
	HC_Open_Segment("cube");
	HC_Set_Color("faces=grey");
	HC_Set_Visibility("vertices=on,markers=on");
	HC_Insert_Shell(8, pts, 30, face_list);
	HC_Close_Segment();
	HC_Close_Segment();
	m_pHView->ZoomToExtents();
	////m_pHView->RenderFakeHiddenLine ();
	//m_pHView->GetSelection ()->SetSubentityFaceSelection (false);
	//m_pHView->GetSelection ()->SetSubentityEdgeSelection (false);
	////m_pHView->GetSelection ()->SetAllowRegionSelection (false);
	//m_pHView->GetSelection ()->SetSubentityVertexSelection (true);
	////m_pHView->GetSelection ()->SetAllowSubentitySelection (false);
	////m_pHView->GetSelection ()->SetAllowRegionSelection (false);
	//HSubwindow subwindow;

	//if (!m_pHView->GetHObjectManager()->GetHObject("subwindow"))
	//	m_pHView->GetHObjectManager()->AddHObject(new HSubwindow(m_pHView));

	//HC_Open_Segment_By_Key(m_pHView->GetModelKey());

	//HC_Open_Segment("subwindows");
	//subwindow.Insert(-1.0f, -0.7f, 
	//	0.7f,1.0f,  SUBWINDOW_POINTER  );
	//HC_Close_Segment();
	//HC_Close_Segment();

	//subwindow.MakeCameraSnapshot(m_pHView);
	//m_pHView->Update();	

	m_pHView->Update();

}

void HoopsView::SetupView()
{

	// set initial HOOPS/3DGS segment tree attributes for the  
	// HOOPS/MVO View Object 

	m_pHView->FitWorld();  // fit the camera to the scene extents 
	m_pHView->RenderGouraud();  // set the render mode to gouraud 

	// configure view segment  
	HC_Open_Segment_By_Key(m_pHView->GetViewKey());
	HC_Set_Color_By_Index("windows", 0);
	HC_Set_Selectability("everything = off");
	HC_Close_Segment();

	// Configure scene/model segment 
	HC_Open_Segment_By_Key(m_pHView->GetSceneKey());
	HC_Set_Color_By_Index("faces", 2);
	HC_Set_Color_By_Index("text, lights", 1);
	HC_Set_Color_By_Index("edges, lines", 1);
	HC_Set_Color_By_Index("markers", 1);
	HC_Set_Rendering_Options
	("no color interpolation, no color index interpolation");
	HC_Set_Visibility
	("lights = (faces = on, edges = off, markers = off), markers = off, faces=on, edges=off, lines=on, text = on");

	HC_Set_Selectability("everything = off, faces = on");
	HC_Set_Text_Font("transforms = off");
	HC_Close_Segment();

	// configure segment for temporary construction geometry 
	HC_Open_Segment_By_Key(m_pHView->GetConstructionKey());
	HC_Set_Heuristics("quick moves");
	HC_Set_Visibility("faces = off, edges = on, lines = on");
	HC_Close_Segment();


	// configure windowspace segment for quickmoves 
	HC_Open_Segment_By_Key(m_pHView->GetWindowspaceKey());
	HC_Set_Color_By_Index("geometry", 3);
	HC_Set_Color_By_Index("window contrast", 1);
	HC_Set_Color_By_Index("windows", 1);

	HC_Set_Visibility("markers=on");
	HC_Set_Color("markers = green, lines = green");
	HC_Set_Marker_Symbol("+");
	HC_Set_Selectability("off");
	HC_Close_Segment();

}

void HoopsView::init_hoops_db()
{
	OPEN_MESHES_SEG
		HC_Set_Visibility("markers=on,lines=on,faces=on");
	HC_Set_Selectability("markers=off,lines=off,faces=off");
	HC_Set_Color("markers=white,lines=black,faces=grey,text=red");
	HC_Set_Line_Weight(1.2);
	HC_Set_Marker_Size(0.5);
	HC_Set_Text_Size(0.7);
	CLOSE_MESHES_SEG

		OPEN_GROUPS_SEG
		HC_Set_Visibility("markers=on,lines=on,faces=on");
	HC_Set_Color("markers=red,lines=green,faces=blue,text=white");
	HC_Set_Line_Weight(4);
	HC_Set_Marker_Size(0.6);
	//HC_Set_Line_Weight (1.2);
	//HC_Set_Marker_Size (0.5);
	//HC_Set_Text_Size (0.7);
	CLOSE_GROUPS_SEG

		OPEN_HIGHLIGHT_SEG
		HC_Set_Color("markers=yellow,lines=yellow,faces=yellow,text=red");
	HC_Set_Line_Weight(4);
	//HC_Set_Line_Weight (1.2);
	HC_Set_Rendering_Options("attribute lock = (color,line weight)");
	CLOSE_HIGHLIGHT_SEG
}

void HoopsView::Init()
{
	m_pHView = new HBaseView(m_pHBaseModel, NULL, NULL, NULL, GetWindowId(), GetColorMap(), GetClipOverride());

	m_pHView->Init();
	HSelectionSet *selectionSet = new HSelectionSet(m_pHView);
	selectionSet->Init();
	selectionSet->SetSelectionLevel(HSelectSubentity);
	HPixelRGBA rgb;
	rgb.Setf(1, 0, 0);
	selectionSet->SetSelectionEdgeColor(rgb);
	m_pHView->SetSelection(selectionSet);

	// Set up the HOOPS/MVO View's HOOPS/3DGS Attributes 
	SetupView();

	// Set View's current Operator 
	m_pHView->SetOperator(new HOpCameraManipulate(m_pHView));

	//DEBUG_STARTUP_CLEAR_BLACK = 0x00004000 clear ogl to black on init update
	HC_Open_Segment_By_Key(m_pHView->GetViewKey());
	HC_Set_Driver_Options("debug = 0x00004000");
	HC_Close_Segment();

	m_pHView->SetHandedness(HandednessRight);
	m_pHView->SetViewMode(HViewXY);
	//m_pHView->SetWindowColor (HPoint (1, 1, 1), HPoint (0.2, 0.2, 0.7));//渐变色
	m_pHView->SetWindowColor(HPoint(1, 1, 1), HPoint(1, 1, 1));//纯色
	m_pHView->SetLineAntialiasing(true);
	m_pHView->SetTextAntialiasing(true);
	m_pHView->SetAxisMode(AxisOn);
	m_pHView->GetSelection()->SetSelectionEdgeWeight(3);
	m_pHView->GetSelection()->SetSelectionMarkerSize(0.8);

	auto radius = m_pHView->GetDefaultSelectionProximity();
	m_pHView->SetDefaultSelectionProximity(radius * 2);
	m_pHView->SetVisibilitySelectionMode(true);
	m_pHView->SetViewSelectionLevel(HSelectionLevelEntity);
	m_pHView->GetSelection()->SetSelectionLevel(HSelectEntity);

	HC_Open_Segment_By_Key(m_pHView->GetSceneKey());
	HC_Set_Color("lines=black,text=black");
	HC_Close_Segment();
	// Call the Views Update Method - initiates HOOPS/3DGS Update_Display  
	init_hoops_db();
	//HC_Open_Segment_By_Key (m_pHView->GetViewKey ());{
	//	HC_Open_Segment ("myhud");
	//	HC_Set_Rendering_Options ("anti-alias = text");
	//	HC_Set_Visibility ("text=on");
	//	HC_Set_Text_Size (1);
	//	HC_Insert_Text (0.8, 0.8, 0, "hello\n world!");
	//	HC_Close_Segment ();
	//}HC_Close_Segment ();
	m_pHView->Update();
}

void HoopsView::contextMenuEvent(QContextMenuEvent *event)
{
}

void HoopsView::wheelEvent(QWheelEvent * e)
{
	HQWidget::wheelEvent(e);
}

void HoopsView::passiveWheelEvent(QWheelEvent *e)
{
	HQWidget::wheelEvent(e);
}

static void GetExtension(wchar_t const * filter, wchar_t * extension)
{
	const wchar_t *start = wcsstr(filter, L"(*.") + 3;
	wcscpy(extension, start);
	*(wcsstr(extension, L")")) = L'\0';
}

void HoopsView::OnSaveFileAs()
{
}


// open a load file dialog
void HoopsView::OnLoad()
{
}


void HoopsView::timerEvent(QTimerEvent *event)
{

}

void HoopsView::OnOrbit()
{
	// Set MVO View Object current Operator to HOpCameraOrbit  

	if (m_pHView->GetCurrentOperator())
		delete m_pHView->GetCurrentOperator();

	m_pHView->SetCurrentOperator(new HOpCameraOrbit(m_pHView));

}


// real time zoomer
void HoopsView::OnZoom()
{
	if (m_pHView->GetCurrentOperator())
		delete m_pHView->GetCurrentOperator();

	m_pHView->SetCurrentOperator(new HOpCameraZoom(m_pHView));
}

// window zoomer
void HoopsView::OnZoomToWindow()
{
	if (m_pHView->GetCurrentOperator())
		delete m_pHView->GetCurrentOperator();

	m_pHView->SetCurrentOperator(new HOpCameraZoomBox(m_pHView));
}

// resets the camera to view the world space extents of the model
void HoopsView::OnZoomToExtents()
{
	m_pHView->ZoomToExtents();
}

// panner
void HoopsView::OnPan()
{

	if (m_pHView->GetCurrentOperator())
		delete m_pHView->GetCurrentOperator();
	m_pHView->SetCurrentOperator(new HOpCameraPan(m_pHView));

}

void HoopsView::OnRunMyCode()
{
	HC_Open_Segment_By_Key(m_pHView->GetViewKey());
	HC_Close_Segment();

	m_pHView->Update();
}

//void HoopsView::render_hexamesh (VolumeMesh *mesh)
//{
//	HC_KEY old_key = INVALID_KEY;
//	HC_KEY new_key = POINTER_TO_KEY(mesh);
//	OPEN_MESHES_SEG
//		render_volume_mesh (mesh);
//	CLOSE_MESHES_SEG
//	m_pHView->ZoomToExtents ();
//	m_pHView->SetGeometryChanged ();
//	m_pHView->Update ();
//}

//void HoopsView::render_tetmesh(VMesh* mesh)
//{
//	HC_KEY old_key = INVALID_KEY;
//	HC_KEY new_key = POINTER_TO_KEY(mesh);
//	OPEN_MESHES_SEG
//		render_tet_mesh (mesh);
//	CLOSE_MESHES_SEG
//		m_pHView->ZoomToExtents ();
//	m_pHView->SetGeometryChanged ();
//	m_pHView->Update ();
//}

//void HoopsView::render_tiranglemesh(OP_Mesh *mesh)
//{
//	HC_KEY old_key = INVALID_KEY;
//	HC_KEY new_key = POINTER_TO_KEY(mesh);
//	OPEN_MESHES_SEG
//		render_triangle_mesh (mesh);
//	CLOSE_MESHES_SEG
//		m_pHView->ZoomToExtents ();
//	m_pHView->SetGeometryChanged ();
//	m_pHView->Update ();
//}
//void HoopsView::show_hexamesh (bool show)
//{
//	show_boundary (show);
//	show_inner (show);
//}
//
//void HoopsView::show_tetmesh (bool show)
//{
//	show_boundary(show);
//	show_inner(show);
//}
//
//void HoopsView::rerender_hexamesh (VolumeMesh *mesh)
//{
//	HC_KEY old_key = INVALID_KEY;
//	HC_KEY new_key = POINTER_TO_KEY(mesh);
//	OPEN_MESHES_SEG
//		HC_Flush_Segment ("...");
//		render_volume_mesh (mesh);
//	CLOSE_MESHES_SEG
//
//	m_pHView->SetGeometryChanged ();
//	m_pHView->Update ();
//}
//
//void HoopsView::rerender_tetmesh (VMesh* mesh)
//{
//	HC_KEY old_key = INVALID_KEY;
//	HC_KEY new_key = POINTER_TO_KEY(mesh);
//	OPEN_MESHES_SEG
//		HC_Flush_Segment ("...");
//	render_tetmesh (mesh);
//	CLOSE_MESHES_SEG
//
//		m_pHView->SetGeometryChanged ();
//	m_pHView->Update ();
//}
//
//void HoopsView::flush_segment(std::string s)
//{
//	const char* c = s.c_str();
//	OPEN_MESHES_SEG
//		HC_Flush_Segment (c);
//	CLOSE_MESHES_SEG
//		//m_pHView->SetGeometryChanged ();
//		m_pHView->Update ();
//}
//
//
//void HoopsView::derender_hexamesh ()
//{
//	OPEN_MESHES_SEG
//		HC_Flush_Segment ("...");
//	CLOSE_MESHES_SEG
//
//	m_pHView->SetGeometryChanged ();
//	m_pHView->Update ();
//}


void HoopsView::local_set_operator(HBaseOperator *new_operator)
{
	HBaseOperator *old_operator = m_pHView->GetOperator();
	m_pHView->SetOperator(new_operator);
	if (old_operator)
		delete old_operator;
}

void HoopsView::begin_camera_manipulate()
{
	local_set_operator(new HOpCameraManipulate(m_pHView));
}

void HoopsView::begin_select_by_click()
{
	local_set_operator(new HOpSelectAperture(m_pHView));

}

void HoopsView::begin_select_by_rectangle()
{
	local_set_operator(new HOpSelectArea(m_pHView));
}

void HoopsView::begin_select_by_polygon()
{
	auto sel = new HOpSelectPolygon(m_pHView);
	HC_Open_Segment_By_Key(m_pHView->GetWindowspaceKey()); {
		HC_Set_Line_Weight(2);
		HC_Set_Edge_Weight(2);
	}HC_Close_Segment();
	local_set_operator(sel);
}

void HoopsView::clear_selection()
{
	m_pHView->GetSelection()->DeSelectAll();
	m_pHView->Update();
}

void HoopsView::set_vertices_selectable(bool selectable)
{
	OPEN_MESHES_SEG
		QString options = QString("markers=%1").arg(selectable ? "on" : "off");
	HC_Set_Selectability(options.toAscii().data());
	CLOSE_MESHES_SEG

		m_pHView->SetGeometryChanged();
	m_pHView->Update();
}

void HoopsView::set_edges_selectable(bool selectable)
{
	OPEN_MESHES_SEG
		QString options = QString("lines=%1").arg(selectable ? "on" : "off");
	HC_Set_Selectability(options.toAscii().data());

	CLOSE_MESHES_SEG
		m_pHView->SetGeometryChanged();
	m_pHView->Update();
}

void HoopsView::set_faces_selectable(bool selectable)
{
	OPEN_MESHES_SEG
		QString options = QString("polygons=%1").arg(selectable ? "on" : "off");
	HC_Set_Selectability(options.toAscii().data());
	CLOSE_MESHES_SEG
		m_pHView->SetGeometryChanged();
	m_pHView->Update();
}

void HoopsView::show_boundary(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("boundary"); {
		if (!show) {
			HC_Set_Visibility("everything=off");
			HC_Set_Rendering_Options("attribute lock = visibility");
		}
		else {
			HC_UnSet_Visibility();
			HC_UnSet_One_Rendering_Option("attribute lock");
		}
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_boundary_vertices(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("boundary"); {
		HC_Open_Segment("meshvertices"); {
			if (!show) {
				HC_Set_Visibility("everything=off");
			}
			else {
				HC_UnSet_Visibility();
				HC_Set_Visibility("markers=on");
			}
		}HC_Close_Segment();
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_boundary_edges(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("boundary"); {
		HC_Open_Segment("meshedges"); {
			if (!show) {
				HC_Set_Visibility("everything=off");
			}
			else {
				HC_UnSet_Visibility();
				HC_Set_Visibility("lines=on");
			}
		}HC_Close_Segment();
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_boundary_faces(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("boundary"); {
		HC_Open_Segment("meshfaces"); {
			if (!show) {
				HC_Set_Visibility("everything=off");
			}
			else {
				HC_UnSet_Visibility();
				HC_Set_Visibility("faces=on");
			}
		}HC_Close_Segment();
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}
//void HoopsView::show_boundary_faces (bool show)
//{
//	OPEN_MESHES_SEG
//		HC_Open_Segment ("boundary");{
//			HC_Open_Segment ("meshfaces");{
//				if (!show){
//					HC_Set_Visibility ("everything=off");
//				}else{
//					HC_UnSet_Visibility ();
//					HC_Set_Visibility ("faces=on");
//					//HC_Set_Color("faces = (transmission = (r=0.1 g=0.1 b=0.1), diffuse = gray)");
//					HC_Set_Color("faces = (transmission = (r=0.1 g=0.1 b=0.1))");
//				}
//			}HC_Close_Segment ();
//	}HC_Close_Segment ();
//	CLOSE_MESHES_SEG
//		m_pHView->Update ();
//}
void HoopsView::show_boundary_cells(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("boundary"); {
		HC_Open_Segment("meshcells"); {
			if (!show) {
				HC_Set_Visibility("everything=off");
			}
			else {
				HC_UnSet_Visibility();
				HC_Set_Visibility("markers=off,edges=on,faces=on");
			}
		}HC_Close_Segment();
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_boundary_vertices_indices(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("boundary"); {
		HC_Open_Segment("meshvertices"); {
			HC_Open_Segment("indices"); {
				if (!show) {
					HC_Set_Visibility("everything=off");
				}
				else {
					HC_UnSet_Visibility();
					HC_Set_Visibility("text=on");
				}
			}HC_Close_Segment();
		}HC_Close_Segment();
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_boundary_edges_indices(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("boundary"); {
		HC_Open_Segment("meshedges"); {
			HC_Open_Segment("indices"); {
				if (!show) {
					HC_Set_Visibility("everything=off");
				}
				else {
					HC_UnSet_Visibility();
					HC_Set_Visibility("text=on");
				}
			}HC_Close_Segment();
		}HC_Close_Segment();
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_boundary_faces_indices(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("boundary"); {
		HC_Open_Segment("meshfaces"); {
			HC_Open_Segment("indices"); {
				if (!show) {
					HC_Set_Visibility("everything=off");
				}
				else {
					HC_UnSet_Visibility();
					HC_Set_Visibility("text=on");
				}
			}HC_Close_Segment();
		}HC_Close_Segment();
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_boundary_cells_indices(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("boundary"); {
		HC_Open_Segment("meshcells"); {
			HC_Open_Segment("indices"); {
				if (!show) {
					HC_Set_Visibility("everything=off");
				}
				else {
					HC_UnSet_Visibility();
					HC_Set_Visibility("text=on");
				}
			}HC_Close_Segment();
		}HC_Close_Segment();
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_inner(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("inner"); {
		if (!show) {
			HC_Set_Visibility("everything=off");
			HC_Set_Rendering_Options("attribute lock = visibility");
		}
		else {
			HC_UnSet_Visibility();
			HC_UnSet_One_Rendering_Option("attribute lock");
		}
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_inner_vertices(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("inner"); {
		HC_Open_Segment("meshvertices"); {
			if (!show) {
				HC_Set_Visibility("everything=off");
			}
			else {
				HC_UnSet_Visibility();
				HC_Set_Visibility("markers=on");
			}
		}HC_Close_Segment();
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_inner_edges(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("inner"); {
		HC_Open_Segment("meshedges"); {
			if (!show) {
				HC_Set_Visibility("everything=off");
			}
			else {
				HC_UnSet_Visibility();
				HC_Set_Visibility("lines=on");
			}
		}HC_Close_Segment();
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_inner_faces(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("inner"); {
		HC_Open_Segment("meshfaces"); {
			if (!show) {
				HC_Set_Visibility("everything=off");
			}
			else {
				HC_UnSet_Visibility();
				HC_Set_Visibility("faces=on");
			}
		}HC_Close_Segment();
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_inner_cells(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("inner"); {
		HC_Open_Segment("meshcells"); {
			if (!show) {
				HC_Set_Visibility("everything=off");
			}
			else {
				HC_UnSet_Visibility();
				HC_Set_Visibility("markers=off,edges=on,faces=on");
			}
		}HC_Close_Segment();
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_inner_vertices_indices(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("inner"); {
		HC_Open_Segment("meshvertices"); {
			HC_Open_Segment("indices"); {
				if (!show) {
					HC_Set_Visibility("everything=off");
				}
				else {
					HC_UnSet_Visibility();
					HC_Set_Visibility("text=on");
				}
			}HC_Close_Segment();
		}HC_Close_Segment();
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_inner_edges_indices(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("inner"); {
		HC_Open_Segment("meshedges"); {
			HC_Open_Segment("indices"); {
				if (!show) {
					HC_Set_Visibility("everything=off");
				}
				else {
					HC_UnSet_Visibility();
					HC_Set_Visibility("text=on");
				}
			}HC_Close_Segment();
		}HC_Close_Segment();
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_inner_faces_indices(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("inner"); {
		HC_Open_Segment("meshfaces"); {
			HC_Open_Segment("indices"); {
				if (!show) {
					HC_Set_Visibility("everything=off");
				}
				else {
					HC_UnSet_Visibility();
					HC_Set_Visibility("text=on");
				}
			}HC_Close_Segment();
		}HC_Close_Segment();
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_inner_cells_indices(bool show)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("inner"); {
		HC_Open_Segment("meshcells"); {
			HC_Open_Segment("indices"); {
				if (!show) {
					HC_Set_Visibility("everything=off");
				}
				else {
					HC_UnSet_Visibility();
					HC_Set_Visibility("text=on");
				}
			}HC_Close_Segment();
		}HC_Close_Segment();
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

int HoopsView::get_selected_elements(std::vector<OvmVeH> &vertices,
	std::vector<OvmEgH> &edges, std::vector<OvmFaH> &faces)
{
	HSelectionSet *selec_set = m_pHView->GetSelection();
	int selected_count = selec_set->GetSize();
	char keytype[100];
	selec_set->SetSortSelectionList(true);
	for (int i = 0; i < selected_count; i++)
	{
		HC_KEY key = selec_set->GetAt(i);
		int idx;
		HC_Show_Key_Type(key, keytype);
		if (streq(keytype, "marker")) {
			HC_Open_Geometry(key); {
				HC_Show_One_Net_User_Data(0, &idx, sizeof(int));
			}HC_Close_Geometry();
			vertices.push_back(OvmVeH(idx));
		}
		else if (streq(keytype, "line")) {
			HC_Open_Geometry(key); {
				HC_Show_One_Net_User_Data(0, &idx, sizeof(int));
			}HC_Close_Geometry();
			edges.push_back(OvmEgH(idx));
		}
		else if (streq(keytype, "polygon")) {
			HC_Open_Geometry(key); {
				HC_Show_One_Net_User_Data(0, &idx, sizeof(int));
			}HC_Close_Geometry();
			faces.push_back(OvmFaH(idx));
		}
		else {
			continue;
		}
	}
	selec_set->DeSelectAll();
	return selected_count;
}


void HoopsView::show_body_edges(ENTITY * ent)
{
	//HC_Flush_Geometry("meshes");
	//HC_Flush_Geometry(m_pHView->GetModelKey());
	OPEN_MESHES_SEG

		ENTITY_LIST edges;
	api_get_edges(ent, edges);
	HC_Flush_Geometry(".");
	HC_Open_Segment("just show2");
	{

		//HC_Set_Color("markers = red, lines = yellow");
		HC_Set_Line_Weight(3);
		HC_Set_Marker_Size(0.25);
		//HC_Set_Visibility("markers = off");
		for (int i = 0; i < edges.count(); ++i)
		{
			ENTITY * temp;
			api_copy_entity_contents(edges[i], temp);
			HA_Render_Entity(temp);
			api_del_entity(temp);
		}
	}
	HC_Close_Segment();

	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		m_pHView->Update();
}
void HoopsView::show_edge(ENTITY * ipEntiEdge)
{
	OPEN_MESHES_SEG
		//HC_Flush_Segment ("...");

		HC_Open_Segment("just show2");
	{
		HC_Set_Color("markers = green, lines = blue");
		ENTITY * temp;
		api_copy_entity_contents(ipEntiEdge, temp);
		HA_Render_Entity(temp);
		api_del_entity(temp);
		//HC_Set_Line_Weight (3);
		//HC_Set_Marker_Size (1);
		//HC_Set_Visibility("markers = off");
	}
	HC_Close_Segment();

	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		m_pHView->Update();
}
void HoopsView::show_body_edges_1(EDGE * e)
{
	OPEN_MESHES_SEG
		//HC_Flush_Segment ("...");
		ENTITY_LIST edges;
	HC_Open_Segment("just show4");
	{
		HC_Set_Color("markers = yellow, lines = red");
		//HC_Set_Line_Weight (3);
		//HC_Set_Marker_Size (1);
		//HC_Set_Visibility("markers = off");
		HA_Render_Entity(e);
	}
	HC_Close_Segment();

	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		m_pHView->Update();
}

void HoopsView::show_body_faces(ENTITY * ent)
{
	OPEN_MESHES_SEG
		//HC_Flush_Segment ("...");
		ENTITY_LIST faces;
	api_get_faces(ent, faces);
	HC_Open_Segment("just show");
	{
		HC_Set_Color("lines = blue");
		HC_Set_Line_Weight(0.6);
		HC_Set_Rendering_Options("transparency = (style = blended, hsra = none)");
		HC_Set_Rendering_Options("depth range = ( 0, 0.1)");
		HC_Set_Color("faces = (transmission = (r=0.7 g=0.7 b=0.7), diffuse = cyan)");
		HC_Set_Visibility("markers = off, lines = on");

		for (int i = 0; i < faces.count(); ++i)
		{
			ENTITY * temp;
			api_copy_entity_contents(faces[i], temp);
			HA_Render_Entity(temp);
			api_del_entity(temp);
		}
	}
	HC_Close_Segment();

	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		//m_pHView->Update ();
}

void HoopsView::show_body_face_group(std::unordered_set<FACE*> faces, std::string s, bool if_show)
{
	const char* c = s.c_str();
	OPEN_MESHES_SEG
		HC_Open_Segment(c);
	{
		if (if_show)
		{
			HC_Set_Color("lines = blue");
			HC_Set_Line_Weight(0.6);
			HC_Set_Rendering_Options("transparency = (style = blended, hsra = none)");
			HC_Set_Rendering_Options("depth range = ( 0, 0.1)");
			//HC_Set_Color("faces = (transmission = (r=0.5 g=0.5 b=0.5), diffuse = cyan)");
			HC_Set_Color("faces = (transmission = (r=0.8 g=0.8 b=0.8))");
			//HC_Set_Color("faces = (transmission = (r=0.8 g=0.8 b=0.8))");
			HC_Set_Visibility("markers = off, lines = off");

			for (auto f_it = faces.begin(); f_it != faces.end(); f_it++)
			{
				ENTITY* temp;
				api_copy_entity_contents(*f_it, temp);
				HA_Render_Entity(temp);
				api_del_entity(temp);
			}
		}
		else
		{
			HC_Flush_Contents("...", "everything");
		}
	}
	HC_Close_Segment();

	CLOSE_MESHES_SEG
}

void HoopsView::show_body_faces1(FACE* f)
{
	OPEN_MESHES_SEG
		//HC_Flush_Segment ("...");
		HC_Open_Segment("just show1");
	{
		//HC_Set_Color("markers = green, lines = blue");
		HC_Set_Rendering_Options("transparency = (style = blended, hsra = none)");
		HC_Set_Rendering_Options("depth range = ( 0, 0.1)");
		HC_Set_Color("faces = (transmission = (r=0 g=0 b=0), diffuse = green)");
		HC_Set_Visibility("markers = off, lines = off");

		HA_Render_Entity(f);
	}
	HC_Close_Segment();

	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		//m_pHView->Update ();
}

//void HoopsView::show_body_faces(ENTITY * ent)
//{
//	OPEN_MESHES_SEG
//		//HC_Flush_Segment ("...");
//		ENTITY_LIST faces;
//	api_get_faces(ent, faces);
//	ENTITY_LIST edges;
//	api_get_edges(ent, edges);
//	ENTITY * edge;
//	ENTITY_LIST vertices, vertices_temp;
//	api_get_vertices(ent, vertices);
//	for(int i = 0; i < edges.count(); i++){
//		if(is_linear_edge(edges[i])){
//			edge = edges[i];
//			break;
//		}
//	}
//	api_get_vertices(edge, vertices_temp);
//	HC_Open_Segment("just show1");
//	{
//		for (int i = 0; i < faces.count(); ++ i)
//		{
//			ENTITY * temp;
//			api_copy_entity_contents(faces[i], temp);
//			HC_Set_Color("markers = pink, lines = pink");
//			HC_Set_Rendering_Options("transparency = (style = blended, hsra = none)");
//			HC_Set_Rendering_Options("depth range = ( 0, 0.1)");
//			HC_Set_Color("faces = (transmission = (r=0.2 g=0.2 b=0.2), diffuse = pink)");
//			//HC_Set_Visibility("markers = off, lines=off");
//			HA_Render_Entity(temp);
//			api_del_entity(temp);
//		}
//	}
//	HC_Close_Segment();
//	HC_Open_Segment("just show2");
//	{
//		for(int i = 0; i < edges.count(); i++){
//			if(edges[i] == edge)
//				continue;
//			HC_Set_Color("markers = pink, lines = blue");
//			//HC_Set_Line_Weight (3);
//			//HC_Set_Marker_Size (1);
//			//HC_Set_Visibility("markers = off");
//			ENTITY * temp;
//			api_copy_entity_contents(edges[i], temp);
//			HA_Render_Entity(temp);
//			api_del_entity(temp);
//		}
//	}
//	HC_Close_Segment();
//	HC_Open_Segment("just show3");
//	{
//		for (int i = 0; i < vertices.count(); i++){
//			if(vertices_temp.lookup(vertices[i]) != -1)
//				continue;
//			HC_Set_Color("markers = green");
//			ENTITY * temp;
//			api_copy_entity_contents(vertices[i], temp);
//			HA_Render_Entity(temp);
//			api_del_entity(temp);
//		}
//	}
//	HC_Close_Segment();
//
//	CLOSE_MESHES_SEG
//		//m_pHView->SetGeometryChanged ();
//		//m_pHView->Update ();
//}

void HoopsView::show_body_vertices(ENTITY * ent)
{
	OPEN_MESHES_SEG
		//HC_Flush_Segment ("...");
		ENTITY_LIST vertices;
	api_get_vertices(ent, vertices);
	HC_Open_Segment("just show3");
	{
		HC_Set_Color("markers = green");
		for (int i = 0; i < vertices.count(); ++i)
		{
			ENTITY * temp;
			api_copy_entity_contents(vertices[i], temp);
			HA_Render_Entity(temp);
			api_del_entity(temp);
		}
	}
	HC_Close_Segment();

	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		//m_pHView->Update ();
}

void HoopsView::show_vertex(VERTEX * pVertex)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("just show3");
	{
		HC_Set_Color("markers = red");
		ENTITY * temp;
		api_copy_entity_contents((ENTITY *)pVertex, temp);
		HA_Render_Entity(temp);
		api_del_entity(temp);
	}
	HC_Close_Segment();

	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		m_pHView->Update();
}

void HoopsView::render_SPAposition(SPAposition pt)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show point"); {
		HC_Set_Color("faces=black");
		HC_Set_Color("markers=yellow");
		HC_Insert_Marker(pt.x(), pt.y(), pt.z());
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		m_pHView->Update();
}

void HoopsView::render_point(OvmVec3d point)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show point"); {
		HC_Set_Color("faces=black");
		HC_Set_Color("markers=yellow");
		HC_Insert_Marker(point[0], point[1], point[2]);
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		m_pHView->Update();
}

void HoopsView::render_point_position(SPAposition point)
{
	std::stringstream ss;
	ss << "(" << point.x() << "," << point.y() << "," << point.z() << ")";
	std::string str = ss.str();
	OPEN_MESHES_SEG
		HC_Open_Segment("just show2"); {
		HC_Set_Color("faces=black");
		HC_Set_Color("markers=green");
		HC_Set_Marker_Size(0.5);
		HC_Insert_Marker(point.x(), point.y(), point.z());
		HC_Insert_Text(point.x(), point.y(), point.z(), str.c_str());
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		m_pHView->Update();
}

void HoopsView::render_text(SPAposition point, char* str) {
	OPEN_MESHES_SEG
		HC_Open_Segment("just show2"); {
		//HC_Set_Text_Size(0.3);
		HC_Set_Color("text=blue");
		HC_Insert_Text(point.x(), point.y(), point.z(), str);
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		m_pHView->Update();
}

void HoopsView::render_streamline(OvmVec3d s, OvmVec3d d)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show streamline"); {
		HC_Set_Visibility("markers=off,lines=on");
		//HC_Set_Color("lines = (r = 0.5 g = 0.25 b = 0)");
		HC_Set_Color("lines=yellow");
		HC_Set_Line_Weight(5);
		HC_Insert_Line(s[0], s[1], s[2], d[0], d[1], d[2]);
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		m_pHView->Update();
}

void HoopsView::render_streamline(OvmVec3d s, OvmVec3d d, int color)
{
	std::string colorName;
	std::string segmentName;
	switch (color)
	{
	case 1: {
		colorName = "lines = yellow";
		segmentName = "show streamline1";
		break;
	}
	case 2: {
		colorName = "lines = blue";
		segmentName = "show streamline2";
		break;
	}
	case 3: {
		colorName = "lines = red";
		segmentName = "show streamline3";
		break;
	}
	case 4: {
		colorName = "lines = black";
		segmentName = "show streamline4";
		break;
	}
	default:
		colorName = "lines = green";
		segmentName = "show streamline5";
		break;
	}
	OPEN_MESHES_SEG
		HC_Open_Segment(segmentName.c_str()); {
		HC_Set_Visibility("markers=off,lines=on");
		HC_Set_Color(colorName.c_str());
		HC_Set_Line_Weight(5);
		HC_Insert_Line(s[0], s[1], s[2], d[0], d[1], d[2]);
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		m_pHView->Update();
}

void HoopsView::render_cross(OvmVec3d s, OvmVec3d e1, OvmVec3d e2, bool full)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show cross"); {
		HC_Set_Visibility("markers=off,lines=on");
		HC_Set_Color("lines = red");
		HC_Set_Line_Weight(5);
		HC_Insert_Line(s[0], s[1], s[2], e1[0], e1[1], e1[2]);
		HC_Insert_Line(s[0], s[1], s[2], e2[0], e2[1], e2[2]);
		if (full)
		{
			HC_Insert_Line(s[0], s[1], s[2], 2 * s[0] - e1[0], 2 * s[1] - e1[1], 2 * s[2] - e1[2]);
			HC_Insert_Line(s[0], s[1], s[2], 2 * s[0] - e2[0], 2 * s[1] - e2[1], 2 * s[2] - e2[2]);
		}
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		m_pHView->Update();
}

void HoopsView::render_face(OvmVec3d point, OvmVec3d normal)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show face"); {
		HC_Set_Color("markers=yellow");
		OvmVec3d normal_2(normal[1], -normal[0], 0);
		if (normal[0] == 0 && normal[1] == 0)
			normal_2 = OvmVec3d(1, 0, 0);
		normal_2 = normal_2.normalize_cond();
		OvmVec3d normal_3(cross(normal, normal_2));
		normal_3 = normal_3.normalize_cond();
		QVector<HPoint> pts;
		OvmVec3d centre = point;
		auto v1 = centre + normal_2 * sqrt(2.0) / 2 * 5;
		auto v2 = centre + normal_3 * sqrt(2.0) / 2 * 5;
		auto v3 = centre - normal_2 * sqrt(2.0) / 2 * 5;
		auto v4 = centre - normal_3 * sqrt(2.0) / 2 * 5;
		pts.push_back(HPoint(v1[0], v1[1], v1[2]));
		pts.push_back(HPoint(v2[0], v2[1], v2[2]));
		pts.push_back(HPoint(v3[0], v3[1], v3[2]));
		pts.push_back(HPoint(v4[0], v4[1], v4[2]));
		HC_Insert_Polygon(pts.size(), pts.data());
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		m_pHView->Update();
}

void HoopsView::render_quad(std::vector<OvmVec3d> quad)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show face"); {
		HC_Set_Color("faces=red");
		HC_Set_Visibility("markers=off,lines=off,faces=on");
		QVector<HPoint> pts;
		pts.push_back(HPoint(quad[0][0], quad[0][1], quad[0][2]));
		pts.push_back(HPoint(quad[1][0], quad[1][1], quad[1][2]));
		pts.push_back(HPoint(quad[2][0], quad[2][1], quad[2][2]));
		pts.push_back(HPoint(quad[3][0], quad[3][1], quad[3][2]));
		HC_Insert_Polygon(pts.size(), pts.data());
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		m_pHView->Update();
}

void HoopsView::render_triangle(std::vector<OP_Mesh::Point> points)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show face"); {
		HC_Set_Color("faces=red");
		HC_Set_Visibility("markers=off,lines=off,faces=on");
		QVector<HPoint> pts;
		pts.push_back(HPoint(points[0][0], points[0][1], points[0][2]));
		pts.push_back(HPoint(points[1][0], points[1][1], points[1][2]));
		pts.push_back(HPoint(points[2][0], points[2][1], points[2][2]));

		HC_Insert_Polygon(pts.size(), pts.data());
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		//m_pHView->SetGeometryChanged ();
		m_pHView->Update();
}

void HoopsView::render_position(OvmVec3d point, char* str)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show position"); {
		HC_Insert_Text(point[0], point[1], point[2], str);
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
}
//这里讲render_frame改为只显示六个方向中的三个
void HoopsView::render_frame(OvmVec3d point, OvmVec3d x_vec, OvmVec3d y_vec, OvmVec3d z_vec, bool full)
{
	OPEN_MESHES_SEG
		/*HC_Open_Segment ("show frame center");{
			HC_Set_Color("markers=yellow");
			HC_Insert_Marker (point[0], point[1], point[2]);
	}HC_Close_Segment ();*/
		HC_Open_Segment("show frame"); {
		HC_Set_Visibility("markers=off,lines=on");
		HC_Set_Color("lines=red");
		HC_Set_Line_Weight(4);
		HC_Insert_Line(point[0], point[1], point[2], x_vec[0], x_vec[1], x_vec[2]);
		HC_Insert_Line(point[0], point[1], point[2], y_vec[0], y_vec[1], y_vec[2]);
		HC_Insert_Line(point[0], point[1], point[2], z_vec[0], z_vec[1], z_vec[2]);
		if (full)
		{
			HC_Insert_Line(point[0], point[1], point[2], 2 * point[0] - x_vec[0], 2 * point[1] - x_vec[1], 2 * point[2] - x_vec[2]);
			HC_Insert_Line(point[0], point[1], point[2], 2 * point[0] - y_vec[0], 2 * point[1] - y_vec[1], 2 * point[2] - y_vec[2]);
			HC_Insert_Line(point[0], point[1], point[2], 2 * point[0] - z_vec[0], 2 * point[1] - z_vec[1], 2 * point[2] - z_vec[2]);
		}
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::render_frames(std::vector<OvmVec3d> point, std::vector<OvmVec3d> x_vecs, std::vector<OvmVec3d> y_vecs, std::vector<OvmVec3d> z_vecs, bool full /* = false */)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show frame_x"); {
		HC_Set_Visibility("markers=off, lines=on");
		HC_Set_Color("lines=red");
		HC_Set_Line_Weight(4);
		for (int li = 0; li < point.size(); li++)
		{
			HC_Insert_Line(point[li][0], point[li][1], point[li][2], x_vecs[li][0], x_vecs[li][1], x_vecs[li][2]);
			if (full)
				HC_Insert_Line(point[li][0], point[li][1], point[li][2], 2 * point[li][0] - x_vecs[li][0], 2 * point[li][1] - x_vecs[li][1], 2 * point[li][2] - x_vecs[li][2]);
		}
	}HC_Close_Segment();
	CLOSE_MESHES_SEG

		OPEN_MESHES_SEG
		HC_Open_Segment("show frame_y"); {
		HC_Set_Visibility("markers=off, lines=on");
		HC_Set_Color("lines=yellow");
		HC_Set_Line_Weight(4);
		for (int li = 0; li < point.size(); li++)
		{
			HC_Insert_Line(point[li][0], point[li][1], point[li][2], y_vecs[li][0], y_vecs[li][1], y_vecs[li][2]);
			if (full)
				HC_Insert_Line(point[li][0], point[li][1], point[li][2], 2 * point[li][0] - y_vecs[li][0], 2 * point[li][1] - y_vecs[li][1], 2 * point[li][2] - y_vecs[li][2]);
		}
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		OPEN_MESHES_SEG
		HC_Open_Segment("show frame_z"); {
		HC_Set_Visibility("markers=off, lines=on");
		HC_Set_Color("lines=blue");
		HC_Set_Line_Weight(4);
		for (int li = 0; li < point.size(); li++)
		{
			HC_Insert_Line(point[li][0], point[li][1], point[li][2], z_vecs[li][0], z_vecs[li][1], z_vecs[li][2]);
			if (full)
				HC_Insert_Line(point[li][0], point[li][1], point[li][2], 2 * point[li][0] - z_vecs[li][0], 2 * point[li][1] - z_vecs[li][1], 2 * point[li][2] - z_vecs[li][2]);
		}
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::render_frame_update(OvmVec3d point, OvmVec3d x_vec, OvmVec3d y_vec, OvmVec3d z_vec)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show frame vector update"); {
		HC_Set_Color("lines=red");
		HC_Set_Line_Weight(2);
		OvmVec3d xa(point + x_vec * 4), xb(point - x_vec * 4), ya(point + y_vec * 4), yb(point - y_vec * 4), za(point + z_vec * 4), zb(point - z_vec * 4);
		HC_Insert_Line(xa[0], xa[1], xa[2], xb[0], xb[1], xb[2]);
		HC_Insert_Line(ya[0], ya[1], ya[2], yb[0], yb[1], yb[2]);
		HC_Insert_Line(za[0], za[1], za[2], zb[0], zb[1], zb[2]);
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::render_frame_with_arrow(OvmVec3d point, OvmVec3d x_vec, OvmVec3d y_vec, OvmVec3d z_vec)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show frame vecotr with arrow1"); {
		HC_Set_Color("lines = red,faces = red");
		//float* p1 = new float[12];
		//float* p2 = new float[4]{0.1f,0.1f,0.4f,0.0f};
		float p1[12];
		float p2[4]; p2[0] = 0.1; p2[1] = 0.1; p2[2] = 0.3; p2[3] = 0;
		for (int i = 0; i < 3; i++)
		{
			p1[i] = point[i];
			p1[3 + i] = 0.74*x_vec[i] + 0.26*point[i];
			p1[6 + i] = 0.76*x_vec[i] + 0.24*point[i];
			p1[9 + i] = x_vec[i];
		}
		HC_Insert_PolyCylinder(4, p1, 4, p2, "first");
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
	OPEN_MESHES_SEG
		HC_Open_Segment("show frame vecotr with arrow2"); {
		HC_Set_Color("lines = blue,faces = blue");
		//float* p1 = new float[12];
		//float* p2 = new float[4]{0.1f,0.1f,0.4f,0.0f};
		float p1[12];
		float p2[4]; p2[0] = 0.1; p2[1] = 0.1; p2[2] = 0.3; p2[3] = 0;
		for (int i = 0; i < 3; i++)
		{
			p1[i] = point[i];
			p1[3 + i] = 0.74*y_vec[i] + 0.26*point[i];
			p1[6 + i] = 0.76*y_vec[i] + 0.24*point[i];
			p1[9 + i] = y_vec[i];
		}
		HC_Insert_PolyCylinder(4, p1, 4, p2, "first");
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
	OPEN_MESHES_SEG
		HC_Open_Segment("show frame vecotr with arrow3"); {
		HC_Set_Color("lines = green,faces = green");
		//float* p1 = new float[12];
		//float* p2 = new float[4]{0.1f,0.1f,0.4f,0.0f};
		float p1[12];
		float p2[4]; p2[0] = 0.1; p2[1] = 0.1; p2[2] = 0.4; p2[3] = 0;
		for (int i = 0; i < 3; i++)
		{
			p1[i] = point[i];
			p1[3 + i] = 0.74*z_vec[i] + 0.26*point[i];
			p1[6 + i] = 0.76*z_vec[i] + 0.24*point[i];
			p1[9 + i] = z_vec[i];
		}
		HC_Insert_PolyCylinder(4, p1, 4, p2, "first");
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::render_arrow(OvmVec3d start_point, OvmVec3d end_point)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show vecotr with arrow"); {
		HC_Set_Color("lines = green,faces = cyan");
		//float* p1 = new float[12];
		//float* p2 = new float[4]{0.1f,0.1f,0.4f,0.0f};
		float p1[12];
		float p2[4]; p2[0] = 0.1; p2[1] = 0.1; p2[2] = 0.4; p2[3] = 0;
		for (int i = 0; i < 3; i++)
		{
			p1[i] = start_point[i];
			p1[3 + i] = 0.74*end_point[i] + 0.26*start_point[i];
			p1[6 + i] = 0.76*end_point[i] + 0.24*start_point[i];
			p1[9 + i] = end_point[i];
		}
		HC_Insert_PolyCylinder(4, p1, 4, p2, "first");
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}


void HoopsView::render_x_vec(OvmVec3d point, OvmVec3d x_vec)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show x_vec"); {
		HC_Set_Color("lines=red");
		HC_Set_Line_Weight(2);
		OvmVec3d x(point + x_vec * 0.5);
		HC_Insert_Line(x[0], x[1], x[2], point[0], point[1], point[2]);
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::render_y_vec(OvmVec3d point, OvmVec3d y_vec)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show y_vec"); {
		HC_Set_Color("lines=green");
		HC_Set_Line_Weight(2);
		OvmVec3d y(point + y_vec * 0.5);
		HC_Insert_Line(y[0], y[1], y[2], point[0], point[1], point[2]);
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::render_z_vec(OvmVec3d point, OvmVec3d z_vec)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show z_vec"); {
		HC_Set_Color("lines=blue");
		HC_Set_Line_Weight(2);
		OvmVec3d z(point + z_vec * 0.5);
		HC_Insert_Line(z[0], z[1], z[2], point[0], point[1], point[2]);
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::render_x_vec_update(OvmVec3d point, OvmVec3d x_vec)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show x_vec update"); {
		HC_Set_Color("lines=red");
		HC_Set_Line_Weight(4);
		OvmVec3d x(point + x_vec * 0.25);
		HC_Insert_Line(x[0], x[1], x[2], point[0], point[1], point[2]);
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::render_y_vec_update(OvmVec3d point, OvmVec3d y_vec)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show y_vec update"); {
		HC_Set_Color("lines=green");
		HC_Set_Line_Weight(4);
		OvmVec3d y(point + y_vec * 0.25);
		HC_Insert_Line(y[0], y[1], y[2], point[0], point[1], point[2]);
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::render_z_vec_update(OvmVec3d point, OvmVec3d z_vec)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("show z_vec"); {
		HC_Set_Color("lines=blue");
		HC_Set_Line_Weight(4);
		OvmVec3d z(point + z_vec * 0.25);
		HC_Insert_Line(z[0], z[1], z[2], point[0], point[1], point[2]);
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::render_test1(OvmVec3d point, double dis)
{
	OPEN_MESHES_SEG
		HC_Open_Segment("indices1"); {
		char text_buf[50];
		sprintf(text_buf, "%.2f", dis);
		HC_Insert_Text(point[0], point[1], point[2], text_buf);
	}HC_Close_Segment();
	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_Ohm_order_is_9999(ENTITY * ent)
{
	OPEN_MESHES_SEG
		ENTITY_LIST faces;
	api_get_faces(ent, faces);
	HC_Open_Segment("Ohm_order_is_9999");
	{
		HC_Set_Color("markers = green, lines = black");
		//HC_Set_Line_Weight(4);
		//HC_Set_Rendering_Options("transparency = (style = blended, hsra = none)");
		//HC_Set_Rendering_Options("depth range = ( 0, 0.1)");
		HC_Set_Color("faces = red");
		HC_Set_Visibility("markers = off, lines = on");

		for (int i = 0; i < faces.count(); ++i)
		{
			ENTITY * temp;
			api_copy_entity_contents(faces[i], temp);
			HA_Render_Entity(temp);
			api_del_entity(temp);
		}
	}
	HC_Close_Segment();
	CLOSE_MESHES_SEG
}

void HoopsView::set_cam(double x, double y, double z, double x2, double y2, double z2) {
	OPEN_MESHES_SEG
		HC_Open_Segment("just show2");
	{
		HC_Set_Camera_Target(x, y, z);
		HC_Set_Camera_Position(x2, y2, z2);
	}
	HC_Close_Segment();

	CLOSE_MESHES_SEG
		m_pHView->Update();
}

void HoopsView::show_Ohm_order_is_negative_9999(ENTITY * ent)
{
	OPEN_MESHES_SEG
		ENTITY_LIST faces;
	api_get_faces(ent, faces);
	HC_Open_Segment("Ohm_order_is_-9999");
	{
		HC_Set_Color("markers = green, lines = black");
		//HC_Set_Line_Weight(4);
		//HC_Set_Rendering_Options("transparency = (style = blended, hsra = none)");
		//HC_Set_Rendering_Options("depth range = ( 0, 0.1)");
		HC_Set_Color("faces = blue");
		HC_Set_Visibility("markers = off, lines = on");

		for (int i = 0; i < faces.count(); ++i)
		{
			ENTITY * temp;
			api_copy_entity_contents(faces[i], temp);
			HA_Render_Entity(temp);
			api_del_entity(temp);
		}
	}
	HC_Close_Segment();
	CLOSE_MESHES_SEG
}


void HoopsView::show_nonmetal_region(std::string seg_name, ENTITY * ent, short order)
{
	OPEN_MESHES_SEG
		ENTITY_LIST faces;
	api_get_faces(ent, faces);
	HC_Open_Segment(seg_name.c_str());
	{
		HC_Set_Color("markers = green, lines = black");
		double r = abs(order % 55 % 13) / 13.0;
		double g = abs(order*order % 17) / 17.0;
		double b = abs(3 * order % 19) / 19.0;
		std::cout << r << " " << g << " " << b << std::endl;

		std::string s = "faces = (r=" + std::to_string((long double)r) + " g=" + std::to_string((long double)g) + " b=" + std::to_string((long double)b) + ")";
		HC_Set_Color(s.c_str());
		HC_Set_Visibility("markers = off, lines = on");

		for (int i = 0; i < faces.count(); ++i)
		{
			ENTITY * temp;
			api_copy_entity_contents(faces[i], temp);
			HA_Render_Entity(temp);
			api_del_entity(temp);
		}
	}
	HC_Close_Segment();
	CLOSE_MESHES_SEG
}

void HoopsView::show_metal_region(ENTITY * ent)
{
	OPEN_MESHES_SEG
		ENTITY_LIST faces;
	api_get_faces(ent, faces);
	HC_Open_Segment("metal_region");
	{
		HC_Set_Color("markers = green, lines = black");
		HC_Set_Color("faces = red");
		HC_Set_Visibility("markers = off, lines = on");

		for (int i = 0; i < faces.count(); ++i)
		{
			ENTITY * temp;
			api_copy_entity_contents(faces[i], temp);
			HA_Render_Entity(temp);
			api_del_entity(temp);
		}
	}
	HC_Close_Segment();
	CLOSE_MESHES_SEG
}
