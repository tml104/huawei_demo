#include "StdAfx.h"
#include "HQHEntrance.h"


#ifdef IN_HUAWEI

void HQHEntrance::Run(int model_id, int option1)
{
	// TODO: ...
}

#else


void HQHEntrance::Run(const std::string & file_path, HoopsView* hoopsview)
{
	/*
		[ѡ���]
	*/
	bool option_change_body_trans = true;

	bool option_marknum_init = true;
	bool option_marknum_showedgemark = false;
	bool option_marknum_showedgemark_with_set = false;
	bool option_marknum_showfacemark = false;
	bool option_print_topo = false;

	bool option_solve_remove_degenerated_faces = false;
	bool option_solve_stitch = false;
	bool option_solve_stitch_for_each_bodies = true;
	bool option_solve_nonmanifold = false;
	bool option_solve_nonmanifold_for_each_bodies = false;
	bool option_solve_single_side_faces = false;

	bool option_exp1 = false;
	bool option_exp2 = false;
	bool option_exp3 = false;
	bool option_exp4 = false;
	//bool option_exp4_2 = true;

	bool option_construct = false;
	bool option_construct240710 = false;

	bool option_save_bodies = true;
	bool option_save_bodies_respectly = true;
	bool option_save_stl_bodies_respectly = false;
	bool option_export_geometry_json = true;

	/*
		[ѡ���] ����
	*/

	api_start_modeller(0);

	FILE *f = fopen(file_path.c_str(), "r");

	ENTITY_LIST bodies;
	if (!f) {
		//QMessageBox::critical(NULL, QObject::tr("�򿪴���"), QObject::tr("��ģ���ļ�ʧ�ܣ�"));
		LOG_ERROR("��ģ���ļ�ʧ�ܣ�");
		return;
	}
	api_restore_entity_list(f, TRUE, bodies);
	fclose(f);

	// �ļ�·��Ԥ����
	LOG_INFO("file_path: %s", file_path.c_str());
	auto split_path_tuple = Utils::SplitPath(file_path);

	// �任����
	if (option_change_body_trans) {
		for (int j = 0; j < bodies.count(); j++) {
			BODY* ibody = dynamic_cast<BODY*>(bodies[j]);
			api_change_body_trans(ibody, nullptr);
			LOG_INFO("api_change_body_trans for: bodies[%d]", j);
		}
	}

	// ���ü���init 
	if (option_marknum_init) {
		MarkNum::Init(bodies);

		if (option_print_topo)
		{
			GeometryUtils::TopoChecker topochecker(bodies);
			topochecker.PrintTopo();
		}
	}

	// �����ܿ����󣩱��ÿ���ߵ�EDGE���
	// ע�⣺�����load�ϴ��ģ�͵�ʱ���ǳ�������ģ������
	if (option_marknum_showedgemark) {
		MarkNum::ShowEdgeMark(hoopsview);
	}

	if (option_marknum_showfacemark) {
		MarkNum::ShowFaceMark(hoopsview);
	}

	// ѡ��һ��Ҫ���������
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
			show_edge_marknum_set.insert(491); // ���491������ʲô�����Ȼ�ܵ���warn?? (�������Ǹ���ɫ�ıߣ�û����)
			MarkNum::ShowEdgeMark(hoopsview, show_edge_marknum_set);
		}
	}

	if (option_solve_stitch_for_each_bodies)
	{
		for (int i = 0; i < bodies.count(); i++) { // ��ÿ���������ִ��
			// ��ע����������Ļ���һ�¿��ǲ���api_construct����֮�������
			ENTITY* ibody = bodies[i];
			ENTITY_LIST ibody_list;
			ibody_list.add(ibody);

			LOG_INFO("Stitching for body: [%d]", i);

			Stitch::StitchGapFixer stitchGapFixer(ibody_list);
			stitchGapFixer.Start(true, true);
			stitchGapFixer.Clear();

			LOG_INFO("Stitching for body (second pass): [%d]", i);
			stitchGapFixer.Start(true, false);

			// ������ʹ��ѡ�option_save_bodies_respectly
		}
	}

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

	if (option_solve_nonmanifold_for_each_bodies) {
		for (int i = 0; i < bodies.count(); i++) { // ��ÿ���������ִ��
			ENTITY* ibody = bodies[i];
			ENTITY_LIST ibody_list;
			ibody_list.add(ibody);

			LOG_INFO("Solving Nonmanifold for body: [%d]", i);
			NonManifold::NonManifoldFixer2 nonManifoldFixer2(ibody_list);
			nonManifoldFixer2.Start();
			// ������ʹ��ѡ�option_save_bodies_respectly
		}
	}


	if (option_solve_single_side_faces) {
		// �趨�ߵ�˫��
		SingleSideFaces::SingleSideFacesFixer singleSideFacesFixer(bodies);
		singleSideFacesFixer.Start();
	}

	/* [ʵ��] */
	if (option_exp1) {
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

		if (option_print_topo)
		{
			GeometryUtils::TopoChecker topochecker(bodies);
			topochecker.PrintTopo();
		}
	}
	/* [ʵ�����] */


	// ������Ⱦ����
	for (int i = 0; i < bodies.count(); i++) {
		hoopsview->show_body_edges(bodies[i]);
		hoopsview->show_body_faces(bodies[i]);
	}

	// ԭ���ܣ���һ����Ⱦ���壩
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

	// ��������bodies
	if (option_save_bodies)
	{
		Utils::SaveModifiedBodies(split_path_tuple, bodies);
	}

	// ��240408������bodies list�еĸ���body
	if (option_save_bodies_respectly)
	{
		Utils::SaveModifiedBodiesRespectly(split_path_tuple, bodies);
	}

	if (option_save_stl_bodies_respectly) {
		Utils::SAT2STL(split_path_tuple, bodies);
	}

	if (option_export_geometry_json) {
		GeometryExporter::Exporter exporter(bodies);
		exporter.Start(split_path_tuple);
	}

	LOG_INFO("ALL DONE");

	// �������ļ�����֮ǰ���ô�api_stop_modeller
	api_stop_modeller();

}

#endif