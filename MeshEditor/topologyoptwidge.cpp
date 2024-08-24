#include "StdAfx.h"
#include "topologyoptwidget.h"

//#include "DualOperations.h"
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QDockWidget>
#include <boolapi.hxx>
#include <curextnd.hxx>
#include <cstrapi.hxx>
#include <face.hxx>
#include <lump.hxx>
#include <shell.hxx>
#include <split_api.hxx>
#include <kernapi.hxx>

#include <eulerapi.hxx>

#include <io.h>
#include <time.h>
#include <fstream>
#include "test.h"

#include <stdio.h>

//#include<tchar.h>
//#include <iostream>
//#include <fstream>
//#include <sstream>
//#include <stdio.h>
//#include <string>
using namespace std;

#include "json/json.h"
#include "FileManagement.h"
#include "ohmConnection.h"
#include "pixel.h"
#include "GetPatchType.h"
#include "JsonHandle.h"
#include "GeometricFill.h"
#include "setAttr.h"
using namespace Ohm_slice;

#include "DegeneratedFaces.h"
#include "NonManifold2.h"
#include "StitchGap.h"
#include "SingleSideFaces.h"

#include "MarkNum.h"

#include "ConstructModel.h"
#include "Exp1.h"
#include "Exp2.h"
#include "Experiment240621.h"

#include "logger44/CoreOld.h"

TopologyOptWidget::TopologyOptWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	hoopsview = ui.hoopsview;
	setup_actions ();
	//mesh = NULL;
	body = NULL;
	ui.toolBox->setEnabled (false);
	//ossi_handler = NULL;
	//osse_handler = NULL;
	//depth_cut_handler = NULL;
	//quality_evaluation_ptr = new quality_evaluation;

	//读取几何信息的测试数据
	/*string strModelFile = "E:\\Projects\\Train\\TestCases\\ey";
	Pixels *pPixelz = new Pixels(0,0,0,0);
	pPixelz->Load(strModelFile);*/
}

TopologyOptWidget::~TopologyOptWidget()
{
	//delete quality_evaluation_ptr;
	delete file_controller;
	//delete mesh_render_controller;
	delete mouse_controller;
	delete group_controller;
	//delete mesh;
	//delete osse_handler;
}

void TopologyOptWidget::load_model (QString model_path)
{
	//mesh = load_volume_mesh (mesh_path);
	//bs_mesh = create_boundary_mesh(mesh);
	//init_volume_mesh (mesh, body, SPAresabs * 1000);
	//hoopsview->render_hexamesh (mesh);
	//hoopsview->render_tiranglemesh(bs_mesh);
	//retrieve_sheets (mesh, sheet_set);

	body = load_acis_model (model_path);
	//api_split_periodic_faces(body);
    //api_clean_entity(body);
	//save_acis_entity(body, "../this_entity.sat");
	
	hoopsview->show_body_edges(body);
	hoopsview->show_boundary (true);
	hoopsview->show_boundary_vertices (false);
	hoopsview->show_boundary_edges (true);
	hoopsview->show_boundary_faces (true);
	hoopsview->show_boundary_cells (false);
	hoopsview->show_boundary_vertices_indices (false);
	hoopsview->show_boundary_edges_indices (false);
	hoopsview->show_boundary_faces_indices (false);
	hoopsview->show_boundary_cells_indices (false);

	hoopsview->show_inner (false);
	hoopsview->show_inner_vertices (false);
	hoopsview->show_inner_edges (true);
	hoopsview->show_inner_faces (false);
	hoopsview->show_inner_cells (false);
	hoopsview->show_inner_vertices_indices (false);
	hoopsview->show_inner_edges_indices (false);
	hoopsview->show_inner_faces_indices (false);
	hoopsview->show_inner_cells_indices (false);
	hoopsview->set_edges_selectable (true);
}

void TopologyOptWidget::setup_actions ()
{
	file_controller = new FileControlWidget (this);
	file_controller->set_directory_and_filters ("../testdata/meshoptimization", tr("MeshEditor文件(*.sat)"));
	//mesh_render_controller = new MeshRenderControlWidget (hoopsview, this);
	mouse_controller = new MouseControlWidget (hoopsview, this);
	group_controller = new GroupControlWidget (hoopsview, this);


	std::vector<QToolBar*> one_line_toolbar;

	auto toolbar = new QToolBar (tr("工具栏"), this);
	toolbar->addWidget (file_controller);
	toolbar->addSeparator ();
	//toolbar->addWidget (mesh_render_controller);
	toolbar->addSeparator ();
	toolbar->addWidget (mouse_controller);
	toolbar->addSeparator ();
	toolbar->addWidget (group_controller);
	toolbar->setAllowedAreas (Qt::TopToolBarArea | Qt::BottomToolBarArea);
	one_line_toolbar.push_back (toolbar);
	toolbars.push_back (one_line_toolbar);


	connect (file_controller, SIGNAL (open_file (QString)), SLOT (on_open_file (QString)));
	connect (file_controller, SIGNAL (save_file ()), SLOT (on_save_file ()));
	connect (file_controller, SIGNAL (save_file_as (QString)), SLOT (on_save_file_as (QString)));
	connect (file_controller, SIGNAL (file_close ()), SLOT (on_file_close ()));

	connect (ui.pushButton_Start_Generation, SIGNAL (clicked ()), SLOT (CreatePLMicrostructure()));

}

/*
	接下来要修改的主要部分 BEGIN
*/
void TopologyOptWidget::on_open_file (QString file_path)
{
	/*
		[选项开关]
	*/
	bool option_change_body_trans = true;

	bool option_marknum_init = true;
	bool option_marknum_showedgemark = true;
	bool option_marknum_showedgemark_with_set = false;
	bool option_marknum_showfacemark = false;

	bool option_solve_remove_degenerated_faces = true;
	bool option_solve_nonmanifold = true;
	bool option_solve_stitch = false; 
	bool option_solve_stitch_for_each_bodies = false;
	bool option_solve_single_side_faces = false;

	bool option_exp1 = false;
	bool option_exp2 = false;
	bool option_exp3 = false;
	bool option_exp4 = false;
	//bool option_exp4_2 = true;

	bool option_construct = false;
	bool option_construct240710 = false;

	bool option_save_bodies = true;
	bool option_save_bodies_respectly = false;
	/*
		[选项开关] 结束
	*/

	api_start_modeller(0);
	
	FILE *f = fopen(file_path.toAscii().data(), "r");

	ENTITY_LIST bodies;
	if (!f) {
		QMessageBox::critical(NULL, QObject::tr("打开错误！"), QObject::tr("打开模型文件失败！"));
		return;
	}
	api_restore_entity_list(f, TRUE, bodies);
	fclose(f);

	// 文件路径预处理
	LOG_INFO("file_path: %s", file_path.toAscii().data());
	std::string file_path_string(file_path.toAscii().data());
	auto split_path_tuple = Utils::SplitPath(file_path_string);

	// 变换物体
	if (option_change_body_trans) {

		for (int j = 0; j < bodies.count();j++) {
			BODY* ibody = dynamic_cast<BODY*>(bodies[j]);
			api_change_body_trans(ibody, nullptr);
			LOG_INFO("api_change_body_trans for: bodies[%d]", j);
		}
	}

	// 调用计数init 
	if (option_marknum_init) {
		MarkNum::Init(bodies);

		GeometryUtils::TopoChecker topochecker(bodies);
		topochecker.PrintTopo();
	}

	// （性能开销大）标记每个边的EDGE编号
	// 注意：这个在load较大的模型的时候会非常慢，大模型慎用
	if (option_marknum_showedgemark) {
		MarkNum::ShowEdgeMark(hoopsview);
	}

	if (option_marknum_showfacemark){
		MarkNum::ShowFaceMark(hoopsview);
	}

	// 选择一个要解决的问题

	if (option_solve_remove_degenerated_faces) {
		DegeneratedFaces::DegeneratedFacesFixer degeneratedFaceFixer(bodies);
		degeneratedFaceFixer.Start();
	}


	if (option_solve_nonmanifold) {
		// A_ent(2).sat, cyl3
		//NonManifold::NonManifoldFixer nonManifoldFixer(bodies);
		//nonManifoldFixer.Start();

		NonManifold::NonManifoldFixer2 nonManifoldFixer2(bodies);
		nonManifoldFixer2.Start();
	}

	if (option_solve_stitch) {
		//B_single.sat -> B_single_mod.sat
		Stitch::StitchGapFixer stitchGapFixer(bodies);
		stitchGapFixer.Start(true, true);

		stitchGapFixer.Clear();
		stitchGapFixer.Start(true, false);


		if (option_marknum_showedgemark_with_set) {
			std::set<int> show_edge_marknum_set;
			for (int i = 0; i < stitchGapFixer.poor_coedge_pair_vec.size(); i++)
			{
				auto pair = stitchGapFixer.poor_coedge_pair_vec[i];
				show_edge_marknum_set.insert(MarkNum::GetId(pair.first.coedge->edge()));
				show_edge_marknum_set.insert(MarkNum::GetId(pair.second.coedge->edge()));
			}
			show_edge_marknum_set.insert(491); // 这个491究竟是什么玩意居然能导致warn?? (好像是那个绿色的边，没事了)
			MarkNum::ShowEdgeMark(hoopsview, show_edge_marknum_set);
		}
	}

	if (option_solve_stitch_for_each_bodies) // TODO: 修改逻辑
	{
		Exp1::Exp1 exp1;
		exp1.Init(bodies);
		bool stitch_call_fix = false;
		for (int i = 0; i < bodies.count(); i++) { // 对每个零件单独执行
			// 备注：如果卡死的话试一下看是不是api_construct……之类的问题
			ENTITY* ibody = bodies[i];
			ENTITY_LIST ibody_list;
			ibody_list.add(ibody);
	
			LOG_INFO("Stitching for body: [%d]", i);
	
			Stitch::StitchGapFixer stitchGapFixer(ibody_list);
			stitchGapFixer.Start(stitch_call_fix, false);
	
			// 保存请使用选项：option_save_bodies_respectly
		}
	}

	if (option_solve_single_side_faces) {
		// 设定边的双边
		SingleSideFaces::SingleSideFacesFixer singleSideFacesFixer(bodies);
		singleSideFacesFixer.Start();
	}

	/* [实验] */
	if (option_exp1){
		// EXp1...
	}

	if (option_exp2) {
		Exp2::Exp2 exp2;
		exp2.Init(bodies);
		exp2.ShowBadLoopEdgeMark(hoopsview);
	}

	if (option_exp3)
	{
		Exp3::Exp3 exp3;
		exp3.Init(static_cast<BODY*>(bodies[0]));
	}

	if (option_exp4) {
		Exp4::Exp4 exp4(bodies);
		exp4.StartExperiment();
	}

	if (option_marknum_init) {
		MarkNum::Clear();
		MarkNum::Init(bodies);

		GeometryUtils::TopoChecker topochecker(bodies);
		topochecker.PrintTopo();
	}
	/* [实验结束] */


	// 控制渲染内容
	for (int i = 0; i < bodies.count(); i++) {
		hoopsview->show_body_edges(bodies[i]);
		hoopsview->show_body_faces(bodies[i]);
	}

	// 原点标架（测一下渲染字体）
	hoopsview->render_point_position(SPAposition(0.0, 0.0, 0.0));
	hoopsview->render_point_position(SPAposition(1.0, 0.0, 0.0));
	hoopsview->render_point_position(SPAposition(0.0, 1.0, 0.0));
	hoopsview->render_point_position(SPAposition(0.0, 0.0, 1.0));

	// Construct my model
	if (option_construct)
	{
		ConstructModel::MyModelConstructor my_model_constructor("C:\\Users\\AAA\\Documents\\WeChat Files\\wxid_4y4wts4jkg9h21\\FileStorage\\File\\2024-07\\two_blocks\\");
		my_model_constructor.Construct240708("two_blocks_240710.sat");
	}

	if (option_construct240710) {
		ConstructModel::MyModelConstructor my_model_constructor("C:\\Users\\AAA\\Documents\\WeChat Files\\wxid_4y4wts4jkg9h21\\FileStorage\\File\\2024-07\\two_blocks\\");
		my_model_constructor.Construct240710TotallyCoincident("two_blocks_240710_totally_coincident.sat");
	}

	// 保存整个bodies
	if (option_save_bodies)
	{
		Utils::SaveModifiedBodies(split_path_tuple, bodies);
	}

	// （240408）保存bodies list中的各个body
	if (option_save_bodies_respectly)
	{
		Utils::SaveModifiedBodiesRespectly(split_path_tuple, bodies);
	}

	// 不能在文件保存之前调用此api_stop_modeller
	api_stop_modeller();

	ui.toolBox->setEnabled (true);
}
/*
	接下来要修改的主要部分 END
*/



void TopologyOptWidget::on_save_file ()
{
	//if (!save_volume_mesh (mesh, mesh_file_path)){
	//	QMessageBox::warning (this, tr("警告"), tr("网格模型保存失败！"), QMessageBox::Ok);
	//	return;
	//}else{
	//	QMessageBox::information (this, tr("提示"), tr("网格模型保存成功！"), QMessageBox::Ok);
	//}

}

void TopologyOptWidget::on_save_file_as (QString file_path)
{
	//if (!save_volume_mesh (mesh, file_path)){
	//	QMessageBox::warning (this, tr("警告"), tr("网格模型保存失败！"), QMessageBox::Ok);
	//	return;
	//}else
	//{
	//	QMessageBox::information (this, tr("提示"), tr("网格模型保存成功！"), QMessageBox::Ok);
	//}
}

void TopologyOptWidget::on_file_close ()
{
	//hoopsview->derender_hexamesh ();
	//delete mesh;
	//mesh = NULL;
	ui.toolBox->setEnabled (false);
}

//#include "ZSRCreatePLS.h"
//void  TopologyOptWidget::CreatePLMicrostructure()
//{
//	ZSRCreatePLS *pPLS=new ZSRCreatePLS(10.0);
//	
//	////show all key points
//	//for(int i=0;i<27;i++)
//	//{
//	//	APOINT *pTempPoint= new APOINT(pPLS->m_kpSet[i].dCoord[0],pPLS->m_kpSet[i].dCoord[1],pPLS->m_kpSet[i].dCoord[2]);
//	//	VERTEX* pTempVertex=new VERTEX(pTempPoint);
//	//	hoopsview->show_vertex(pTempVertex);
//	//}
//
//	//generate PLS
//	pPLS->GenerateCandidatePLS();
//	
//	CreateReferenceHexahedrons(pPLS->m_dCubeLength);
//
//	//bulid PLS Model
//
//}

//void  TopologyOptWidget::CreateReferenceHexahedrons(const double &idCubeLength)
//{
//		//bulid three symmetry plans
//	outcome result;
//	FACE * pTempFace = NULL;
//	BODY* pTempBody=NULL;
//	SPAposition * pos_array = ACIS_NEW SPAposition[5];
//	WIRE * pTempWire = NULL;
//
//	pos_array[0] = SPAposition(0, 0.5*idCubeLength, 0);
//    pos_array[1] = SPAposition(0, 0.5*idCubeLength, idCubeLength);
//    pos_array[2] = SPAposition( idCubeLength, 0.5*idCubeLength, idCubeLength);
//    pos_array[3] = SPAposition(idCubeLength, 0.5*idCubeLength,0);
//	pos_array[4] =  SPAposition(0, 0.5*idCubeLength, 0);
//    result = api_make_wire (pTempBody, 5, pos_array, pTempBody);
//    pTempWire = pTempBody->lump()->shell()->wire();
//    result = api_cover_wire (pTempWire, *(surface*)NULL_REF, pTempFace);
//	hoopsview->show_body_faces1(pTempFace);
//
//    pos_array[0] = SPAposition(0.5*idCubeLength, 0, 0);
//    pos_array[1] = SPAposition(0.5*idCubeLength, 0,idCubeLength);
//    pos_array[2] = SPAposition( 0.5*idCubeLength, idCubeLength,idCubeLength);
//    pos_array[3] = SPAposition( 0.5*idCubeLength, idCubeLength, 0);
//	pos_array[4] =  SPAposition(0.5*idCubeLength, 0, 0);
//    result = api_make_wire (pTempBody, 5, pos_array, pTempBody);
//    pTempWire = pTempBody->lump()->shell()->wire();
//    result = api_cover_wire (pTempWire, *(surface*)NULL_REF, pTempFace);
//	hoopsview->show_body_faces1(pTempFace);
//
//	pos_array[0] = SPAposition(0, 0, 0.5*idCubeLength);
//    pos_array[1] = SPAposition(0,  idCubeLength,  0.5*idCubeLength);
//    pos_array[2] = SPAposition( idCubeLength,  idCubeLength, 0.5*idCubeLength);
//    pos_array[3] = SPAposition( idCubeLength, 0,  0.5*idCubeLength);
//	pos_array[4] =  SPAposition(0, 0, 0.5*idCubeLength);
//    result = api_make_wire (pTempBody, 5, pos_array, pTempBody);
//    pTempWire = pTempBody->lump()->shell()->wire();
//    result = api_cover_wire (pTempWire, *(surface*)NULL_REF, pTempFace);
//	hoopsview->show_body_faces1(pTempFace);
//
//	ACIS_DELETE [] pos_array;  // De-allocate the array.
//
//	//show 8 sub-hexahedrons
//	ENTITY_LIST listBody;
//	BODY* pCubic1=NULL;
//	api_solid_block(SPAposition(0,0,0),SPAposition(0.5*idCubeLength,0.5*idCubeLength,0.5*idCubeLength),pCubic1);
//	listBody.add(pCubic1);
//	BODY* pCubic3=NULL;
//	api_solid_block(SPAposition(0.5*idCubeLength,0,0),SPAposition(idCubeLength,0.5*idCubeLength,0.5*idCubeLength),pCubic3);
//	listBody.add(pCubic3);
//	BODY* pCubic4=NULL;
//	api_solid_block(SPAposition(0.5*idCubeLength,0.5*idCubeLength,0),SPAposition(idCubeLength,idCubeLength,0.5*idCubeLength),pCubic4);
//	listBody.add(pCubic4);
//	BODY* pCubic5=NULL;
//	api_solid_block(SPAposition(0,0.5*idCubeLength,0),SPAposition(0.5*idCubeLength,idCubeLength,0.5*idCubeLength),pCubic5);
//	listBody.add(pCubic5);
//	BODY* pCubic6=NULL;
//	api_solid_block(SPAposition(0,0,0.5*idCubeLength),SPAposition(0.5*idCubeLength,0.5*idCubeLength,idCubeLength),pCubic6);
//	listBody.add(pCubic6);
//	BODY* pCubic7=NULL;
//	api_solid_block(SPAposition(0.5*idCubeLength,0,0.5*idCubeLength),SPAposition(idCubeLength,0.5*idCubeLength,idCubeLength),pCubic7);
//	listBody.add(pCubic7);
//	BODY* pCubic2=NULL;
//	api_solid_block(SPAposition(0.5*idCubeLength,0.5*idCubeLength,0.5*idCubeLength),SPAposition(idCubeLength,idCubeLength,idCubeLength),pCubic2);
//	listBody.add(pCubic2);
//	BODY* pCubic8=NULL;
//	api_solid_block(SPAposition(0,0.5*idCubeLength,0.5*idCubeLength),SPAposition(0.5*idCubeLength,idCubeLength,idCubeLength),pCubic8);
//	listBody.add(pCubic8);
//
//	for(int i=0;i<listBody.count();i++)
//	{
//		BODY *pBody=(BODY *)listBody[i];
//		hoopsview->show_body_faces(pBody);
//		ENTITY_LIST edgelist;
//		api_get_edges(pBody,edgelist);
//		for(int j=0;j<edgelist.count();j++)  hoopsview->show_body_edges(edgelist[j]);
//	}
//
//	//generate keyfaces
//	EDGE* pXMeshEdge = NULL;
//	api_curve_line(SPAposition(0,0,0),SPAposition(10.0,10.0,10.0), pXMeshEdge);
//	hoopsview->show_body_edges_1(pXMeshEdge);
//}

//void TopologyOptWidget::on_highlight_mesh_elements ()
//{
//	auto fSepStr = [] (QString str, std::vector<int> &indices){
//		if (str == "")
//			return;
//		indices.clear ();
//		auto strlist = str.split (",");
//		foreach (auto str, strlist)
//			indices.push_back (str.toInt ());
//	};
//	std::unordered_set<OvmVeH> vertices;
//	std::unordered_set<OvmEgH> edges;
//	std::unordered_set<OvmFaH> faces;
//	std::unordered_set<OvmCeH> hexas;
//
//	auto str = ui.lineEdit_Vertices->text ();
//	std::vector<int> indices;
//	if (str != ""){
//		fSepStr (str, indices);
//		
//		foreach (int idx, indices){
//			vertices.insert (OvmVeH (idx));
//		}
//	}
//
//
//	str = ui.lineEdit_Edges->text ();
//	if (str != ""){
//		fSepStr (str, indices);
//		
//		foreach (int idx, indices){
//			edges.insert (OvmEgH (idx));
//		}
//	}
//
//	str = ui.lineEdit_Faces->text ();
//	if (str != ""){
//		fSepStr (str, indices);
//		
//		foreach (int idx, indices){
//			faces.insert (OvmFaH (idx));
//		}
//	}
//
//	str = ui.lineEdit_Hexas->text ();
//	if (str != ""){
//		fSepStr (str, indices);
//		
//		foreach (int idx, indices){
//			hexas.insert (OvmCeH (idx));
//		}
//	}
//
//	str = ui.lineEdit_HalfEdges->text ();
//	if (str != ""){
//		fSepStr (str, indices);
//
//		foreach (int idx, indices){
//
//			edges.insert (OvmEgH (mesh->edge_handle(idx)));
//		}
//	}
//
//	str = ui.lineEdit_HalfFaces->text ();
//	if (str != ""){
//		fSepStr (str, indices);
//
//		foreach (int idx, indices){
//			faces.insert (OvmFaH (mesh->face_handle(idx)));
//		}
//	}
//
//	//render
//	auto group = new VolumeMeshElementGroup (mesh, "highlight", "highlight elements");
//	group->vhs = vertices;
//	group->ehs = edges;
//	group->fhs = faces;
//	group->chs = hexas;
//	hoopsview->render_mesh_group (group, true);
//	ui.lineEdit_Vertices->clear ();
//	ui.lineEdit_Edges->clear ();
//	ui.lineEdit_Faces->clear ();
//	ui.lineEdit_Hexas->clear ();
//	ui.lineEdit_HalfEdges->clear();
//	ui.lineEdit_HalfFaces->clear();
//
//	mesh_render_controller->render_wireframe ();
//}
//
//void TopologyOptWidget::on_clear_highlight_mesh_elements ()
//{
//	ui.lineEdit_Vertices->clear ();
//	ui.lineEdit_Edges->clear ();
//	ui.lineEdit_Faces->clear ();
//	ui.lineEdit_Hexas->clear ();
//	
//	hoopsview->derender_mesh_groups ("highlight", NULL, true);
//}

void TopologyOptWidget::show_one_cell_real(Ohm_slice::Cell& cell){

	BODY* body;

	double begin_x = cell.leftDown[0];
	double begin_y = cell.leftDown[1];
	double begin_z = cell.leftDown[2];

	double end_x = cell.rightUp[0];
	double end_y = cell.rightUp[1];
	double end_z = cell.rightUp[2];

	cout << begin_x << " " << end_x <<endl;
	cout << begin_y << " " << end_y <<endl;
	cout << begin_z << " " << end_z <<endl;

	// 各个方向的每个区间的大小
	double x_offset = abs(begin_x - end_x) / 16.0;
	double y_offset = abs(begin_y - end_y) / 16.0;
	double z_offset = abs(begin_z - end_z) / 16.0;

	double positon_offset = 0.00001;
	
	auto show_the_pixel = [&](Ohm_slice::Pixel* pixel, BODY*& body){
		if (pixel->order > 0){
			hoopsview->show_metal_region(body);
		}
		else {
			string seg_name = "non-metal" + to_string((long long)pixel->order);
			hoopsview->show_nonmetal_region(seg_name,body,pixel->order);
		}
	};

	// x
	cout << "开始绘制x方向"<<endl;
	for (int i = 0; i < 16; ++i){
		for (int j = 0; j < 16; ++j){
			// 6*256
			api_solid_block(SPAposition(begin_x,begin_y+y_offset*j,begin_z+z_offset*i), SPAposition(begin_x+positon_offset,begin_y+y_offset*j+y_offset,begin_z+z_offset*i+z_offset), body);
			show_the_pixel(cell.patch_list[0]->pixel_list[i][j], body);
			//hoopsview->show_Ohm_order_is_9999(body);
			api_solid_block(SPAposition(begin_x+16*x_offset,begin_y+y_offset*j,begin_z+z_offset*i), SPAposition(begin_x+16*x_offset-positon_offset,begin_y+y_offset*j+y_offset,begin_z+z_offset*i+z_offset), body);
			show_the_pixel(cell.patch_list[1]->pixel_list[i][j], body);
			//hoopsview->show_Ohm_order_is_negative_2(body);

		}
	}
	cout << "绘制x方向结束"<<endl;

	// y
	cout << "开始绘制y方向"<<endl;
	for (int i = 0; i < 16; ++i){
		for (int j = 0; j < 16; ++j){
			api_solid_block(SPAposition(begin_x+x_offset*i,begin_y,begin_z+z_offset*j), SPAposition(begin_x+x_offset*i+x_offset,begin_y+positon_offset,begin_z+z_offset*j+z_offset), body);
			show_the_pixel(cell.patch_list[2]->pixel_list[i][j], body);
			//hoopsview->show_Ohm_order_is_negative_1(body);

			api_solid_block(SPAposition(begin_x+x_offset*i,begin_y+16*y_offset,begin_z+z_offset*j), SPAposition(begin_x+x_offset*i+x_offset,begin_y+16*y_offset+positon_offset,begin_z+z_offset*j+z_offset), body);
			show_the_pixel(cell.patch_list[3]->pixel_list[i][j], body);
			//hoopsview->show_Ohm_order_is_negative_9999(body);

		}
	}
	cout << "绘制y方向结束"<<endl;

	// z
	cout << "开始绘制z方向"<<endl;
	for (int i = 0; i < 16; ++i){
		for (int j = 0; j < 16; ++j){
			api_solid_block(SPAposition(begin_x+x_offset*j,begin_y+y_offset*i,begin_z), SPAposition(begin_x+x_offset*j+x_offset,begin_y+y_offset*i+y_offset,begin_z+positon_offset), body);
			show_the_pixel(cell.patch_list[4]->pixel_list[i][j], body);
			//hoopsview->show_Ohm_order_is_9999(body);

			api_solid_block(SPAposition(begin_x+x_offset*j,begin_y+y_offset*i,begin_z+16*z_offset), SPAposition(begin_x+x_offset*j+x_offset,begin_y+y_offset*i+y_offset,begin_z+16*z_offset-positon_offset), body);
			show_the_pixel(cell.patch_list[5]->pixel_list[i][j], body);
			//hoopsview->show_Ohm_order_is_negative_2(body);

		}
	}
	cout << "绘制z方向结束"<<endl;
}