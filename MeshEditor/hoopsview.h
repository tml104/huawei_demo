#ifndef SIMPLEHQWIDGET_H 
#define SIMPLEHQWIDGET_H 

// Qt Includes 
#include <QLabel> 
#include <QWidget> 
#include <QMenu> 
#include <QSlider> 
#include <QContextMenuEvent>
#include <QAction>
#include <QActionGroup>
#include <QThread>
#include <QTimerEvent>
#include <QProgressDialog>
#include <QList>
#include <QVector>
#include <QMap>
#include <QFile>
#include <QFileInfo>
#include <sstream>
#include <string>
// HOOPS/Qt Includes 
#include "HQWidget.h" 
#include "FuncDefs.h"
#include "MeshDefs.h"

#define POINTER_SIZED_HIGHBIT ((HC_KEY)1 << (8 * sizeof (HC_KEY) - 1))
#define POINTER_TO_KEY(p) ((((HC_KEY)p)>>1) & ~(POINTER_SIZED_HIGHBIT))
#define KEY_TO_POINTER(k) (k<<1)

#define OPEN_MESHES_SEG HC_Open_Segment_By_Key (m_pHView->GetModelKey ());{ \
HC_Open_Segment ("meshes");{

#define CLOSE_MESHES_SEG }HC_Close_Segment ();\
}HC_Close_Segment ();

#define OPEN_GROUPS_SEG HC_Open_Segment_By_Key (m_pHView->GetModelKey ());{ \
HC_Open_Segment ("groups");{

#define CLOSE_GROUPS_SEG }HC_Close_Segment (); \
}HC_Close_Segment ();

#define OPEN_HIGHLIGHT_SEG HC_Open_Segment_By_Key (m_pHView->GetModelKey ());{ \
	HC_Open_Segment ("highlights");{

#define CLOSE_HIGHLIGHT_SEG }HC_Close_Segment (); \
}HC_Close_Segment ();

struct MeshGroupRenderParam {
	QString vertex_color, edge_color, face_color, cell_color, text_color;
	double vertex_size, edge_weight, cell_shrink_ratio;
	bool vertex_visible, edge_visible, face_visible, cell_visible, text_visible;
	MeshGroupRenderParam() {
		vertex_color = edge_color = face_color = cell_color = "";
		vertex_size = edge_weight = cell_shrink_ratio = 0.0f;
		vertex_visible = false; edge_visible = true; face_visible = true; cell_visible = true; text_visible = true;
	}
};

class HoopsView : public HQWidget
{
	Q_OBJECT

public:
	HoopsView(QWidget* parent);
	~HoopsView();
public:
	//void render_hexamesh (VolumeMesh *mesh);
	//void render_tetmesh(VMesh* mesh);
	//void render_tiranglemesh(OP_Mesh *mesh);
	//void render_quadmesh(OP_Mesh *mesh);
	//void rerender_hexamesh (VolumeMesh *mesh);
	//void rerender_tetmesh (VMesh* mesh);
	//void derender_hexamesh ();//这个函数同样可以用于将三角形，四面体网格显示去掉
	//void show_hexamesh (bool show);
	//void show_tetmesh (bool show);
	void show_Ohm_order_is_negative_9999(ENTITY * ent);
	void show_Ohm_order_is_9999(ENTITY * ent);
	void set_cam(double x, double y, double z, double x2, double y2, double z2);


	void show_vertex(VERTEX * pVertex);
	void show_edge(ENTITY * ipEntiEdge);

	void show_body_edges(ENTITY * ent);
	void show_body_edges_1(EDGE* e);
	void show_body_faces(ENTITY * ent);
	void show_body_face_group(std::unordered_set<FACE*> faces, std::string s, bool if_show);
	void show_body_faces1(FACE* f);
	void show_body_vertices(ENTITY * ent);
	void show_nonmetal_region(std::string seg_name, ENTITY * ent, short order);
	void show_metal_region(ENTITY * ent);
	void render_point(OvmVec3d point);

	void render_point_position(SPAposition point);
	void render_text(SPAposition point, char* str);

	void render_streamline(OvmVec3d s, OvmVec3d d);
	void render_streamline(OvmVec3d s, OvmVec3d d, int color);
	void render_cross(OvmVec3d s, OvmVec3d e1, OvmVec3d e2, bool full = false);
	void render_face(OvmVec3d point, OvmVec3d normal);
	void render_quad(std::vector<OvmVec3d> points);
	void render_triangle(std::vector<OP_Mesh::Point> points);
	void render_position(OvmVec3d point, char* str);
	void render_frame(OvmVec3d point, OvmVec3d x_vec, OvmVec3d y_vec, OvmVec3d z_vec, bool full = false);//这里讲render_frame改为只显示六个方向中的三个
	void render_frames(std::vector<OvmVec3d> point, std::vector<OvmVec3d> x_vecs, std::vector<OvmVec3d> y_vecs, std::vector<OvmVec3d> z_vecs, bool full = false);
	void render_frame_update(OvmVec3d point, OvmVec3d x_vec, OvmVec3d y_vec, OvmVec3d z_vec);
	void render_frame_with_arrow(OvmVec3d point, OvmVec3d x_vec, OvmVec3d y_vec, OvmVec3d z_vec);
	void render_arrow(OvmVec3d start_point, OvmVec3d end_point);
	void render_x_vec(OvmVec3d point, OvmVec3d x_vec);
	void render_y_vec(OvmVec3d point, OvmVec3d y_vec);
	void render_z_vec(OvmVec3d point, OvmVec3d z_vec);
	void render_x_vec_update(OvmVec3d point, OvmVec3d x_vec);
	void render_y_vec_update(OvmVec3d point, OvmVec3d y_vec);
	void render_z_vec_update(OvmVec3d point, OvmVec3d z_vec);
	void render_test1(OvmVec3d point, double dis);
	void flush_segment(std::string s);
	void render_SPAposition(SPAposition p1);


public slots:
	void begin_camera_manipulate();
	void begin_select_by_click();
	void begin_select_by_rectangle();
	void begin_select_by_polygon();
	void clear_selection();
	void set_vertices_selectable(bool selectable);
	void set_edges_selectable(bool selectable);
	void set_faces_selectable(bool selectable);
	void show_boundary(bool show);
	void show_boundary_vertices(bool show);
	void show_boundary_edges(bool show);
	void show_boundary_faces(bool show);
	void show_boundary_cells(bool show);
	void show_boundary_vertices_indices(bool show);
	void show_boundary_edges_indices(bool show);
	void show_boundary_faces_indices(bool show);
	void show_boundary_cells_indices(bool show);
	void show_inner(bool show);
	void show_inner_vertices(bool show);
	void show_inner_edges(bool show);
	void show_inner_faces(bool show);
	void show_inner_cells(bool show);
	void show_inner_vertices_indices(bool show);
	void show_inner_edges_indices(bool show);
	void show_inner_faces_indices(bool show);
	void show_inner_cells_indices(bool show);
public:
	//////////////////////////////////////////////////////////////////////////
	//group operations
	HC_KEY render_mesh_group(VolumeMeshElementGroup *group, bool show_indices = false, const char *vertex_color = NULL,
		const char *edge_color = NULL, const char *face_color = NULL,
		const char *cell_color = NULL, const char *text_color = NULL);
	//用于显示块结构的函数
	HC_KEY render_mesh_group_1(VolumeMeshElementGroup *group, bool show_indices = false, double _v_value = -1,
		double _e_value = -1, double _f_value = -1,
		double _c_value = -1, double _t_value = -1);
	HC_KEY render_mesh_group_semitransparent(VolumeMeshElementGroup *group, bool show_indices = false, double _v_value = -1,
		double _e_value = -1, double _f_value = -1,
		double _c_value = -1, double _t_value = -1);
	HC_KEY render_mesh_block(VolumeMeshElementGroup *group, double _value_);
	HC_KEY render_mesh_block_boom(VolumeMeshElementGroup *group, double _value_, OvmVec3d center_pos);
	HC_KEY render_mesh_group_gen_ver(GenMeshElementGroup *group, bool show_indices = false, const char *vertex_color = NULL,
		const char *edge_color = NULL, const char *face_color = NULL,
		const char *cell_color = NULL, const char *text_color = NULL);
	HC_KEY render_mesh_group_gen_ver1(GenMeshElementGroup *group, bool show_indices = false, double _v_value = -1,
		double _e_value = -1, double _f_value = -1,
		double _c_value = -1, double _t_value = -1);
	void render_mesh_groups(std::set<VolumeMeshElementGroup*> &groups);
	void derender_one_mesh_group(VolumeMeshElementGroup *group);
	void derender_one_genmesh_group(GenMeshElementGroup *group);
	void derender_mesh_groups(std::set<VolumeMeshElementGroup*> &groups);
	void derender_mesh_groups(const char *group_type, const char *group_name = NULL, bool delete_groups = false);
	void derender_mesh_groups_gen_ver(const char *group_type, const char *group_name = NULL, bool delete_groups = false);
	void derender_all_mesh_groups();
	bool mesh_group_exists(VolumeMeshElementGroup *group);
	bool mesh_group_exists_gen_ver(GenMeshElementGroup *group);
	void get_mesh_groups(std::vector<VolumeMeshElementGroup*> &groups,
		std::vector<VolumeMeshElementGroup*> &invisible_groups,
		std::vector<VolumeMeshElementGroup*> &highlighted_groups,
		const char *group_type = NULL, const char *group_name = NULL);
	void show_mesh_group(VolumeMeshElementGroup *group, bool show);
	void update_mesh_group(VolumeMeshElementGroup *group);
	void update_mesh_groups();
	void update_mesh_group_rendering(VolumeMeshElementGroup *group, MeshGroupRenderParam *param);
	void highlight_mesh_group(VolumeMeshElementGroup *group);
	void dehighlight_mesh_group(VolumeMeshElementGroup *group);
	bool is_mesh_group_highlighted(VolumeMeshElementGroup *group);

	int get_selected_elements(std::vector<OvmVeH> &vertices, std::vector<OvmEgH> &edges, std::vector<OvmFaH> &faces);
public slots:

	void OnLoad();
	void OnSaveFileAs();

	void OnZoomToExtents();
	void OnZoomToWindow();

	void OnZoom();
	void OnOrbit();
	void OnPan();

	void OnRunMyCode();

private:
	void createActions();
	void createModels();
	void local_set_operator(HBaseOperator *new_operator);
protected:

	void SetupView();
	void Init();
	void contextMenuEvent(QContextMenuEvent *);
	void timerEvent(QTimerEvent *);
	void wheelEvent(QWheelEvent * e);
	void passiveWheelEvent(QWheelEvent *e);
private:
	void init_hoops_db();
};
#endif 

