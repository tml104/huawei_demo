#include "StdAfx.h"
#include "HQHEntrance.h"
#include "pthread.h"
#include <functional>

#ifdef IN_HUAWEI

void HQHEntrance::Run(int model_id, int option1)
{
	// TODO: ...
}

#else

static pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

struct LoadTaskArgs {
	ENTITY_LIST* bodies;
	std::string* multi_file_path;
};

void* LoadTask(void* args)
{
	LoadTaskArgs* args2 = (LoadTaskArgs*)args;
	ENTITY_LIST* bodies = args2->bodies;
	std::string* multi_file_path = args2->multi_file_path;

	FILE* f = fopen(multi_file_path->c_str(), "r");

	pthread_mutex_lock(&mutex1);

	api_restore_entity_list(f, TRUE, *bodies);
	pthread_mutex_unlock(&mutex1);


	fclose(f);

	LOG_INFO("LoadTask Done");
	return NULL;
};

struct StitchTaskArgs {
	//Stitch::StitchGapFixer* stitch_gap_fixer;
	ENTITY_LIST* entity_list;
};

void* StitchTask(void* args)
{
	ENTITY_LIST* args2 = (ENTITY_LIST*)args;

	Stitch::StitchGapFixer stitch_gap_fixer(*args2);

	stitch_gap_fixer.Start(false, false);
	return NULL;
}

void HQHEntrance::Run(const std::string & file_path, HoopsView* hoopsview)
{ 
	/*
		[选项开关]
	*/

	bool option_multithread_load = true;

	bool option_change_body_trans = false;

	bool option_marknum_init = true; // 必开
	bool option_marknum_showedgemark = false;
	bool option_marknum_showedgemark_with_set = false;
	bool option_marknum_showfacemark = false;
	bool option_print_topo = false;

	bool option_solve_stitch = false;
	bool option_solve_stitch_for_each_bodies = false;
	bool option_solve_stitch_for_each_bodies_multithread = true;
	bool option_solve_remove_degenerated_faces = false;
	bool option_solve_nonmanifold = false;
	bool option_solve_nonmanifold_for_each_bodies =	false;
	bool option_solve_single_side_faces = false; 

	bool option_make_double_model = false;

	bool option_exp1 = false;
	bool option_exp2 = false;
	bool option_exp3 = false;
	bool option_exp4 = false;
	bool option_exp5 = false;
	bool option_exp5_import = false;
	bool option_exp6 = false;

	bool option_exp250305 = false;
	bool option_marknum_init2 = false;

	//bool option_exp4_2 = true;

	bool option_construct = false;
	bool option_construct240710 = false;

	bool option_save_bodies = true;
	bool option_save_bodies_respectly = false;
	//bool option_save_stl_bodies_respectly = false;
	bool option_export_geometry_json_selected = false;
	bool option_export_geometry_json = false;
	bool option_export_debugshow_json = false;

	//bool option_export_entity_list_stl = true;

	/*
		[选项开关] 结束 
	*/

	api_start_modeller(0);
	
	pthread_mutex_init(&mutex1, NULL);

	Timer::Timer load_body_timer;
	FILE *f = fopen(file_path.c_str(), "r");

	ENTITY_LIST bodies;
	if (!f) {
		//QMessageBox::critical(NULL, QObject::tr("打开错误！"), QObject::tr("打开模型文件失败！"));
		LOG_ERROR("打开模型文件失败！");
		return;
	}
	api_restore_entity_list(f, TRUE, bodies);
	fclose(f);
	load_body_timer.Split("模型加载");

	// multi thread load test
	ENTITY_LIST bodies1, bodies2;
	auto multithread_load_test = [&]() {
		
		std::string file_path1 = "D:\\hqh_study\\ModelForFix\\ModelForFixChecked\\ComplexOverlap\\bodytest\\C_ent(1)_mod_body_0_case1.sat";
		std::string file_path2 = "D:\\hqh_study\\ModelForFix\\ModelForFixChecked\\ComplexOverlap\\bodytest\\C_ent(1)_mod_body_0_case2.sat";

		std::vector<pthread_t> thread_list(2);
		LoadTaskArgs args1;
		args1.bodies = &bodies1;
		args1.multi_file_path = &file_path1;

		LoadTaskArgs args2;
		args2.bodies = &bodies2;
		args2.multi_file_path = &file_path2;

		pthread_create(&thread_list[0], NULL, (void* (*)(void*))LoadTask, (void*)&args1);
		pthread_create(&thread_list[1], NULL, (void* (*)(void*))LoadTask, (void*)&args2);

		for (int i = 0; i < thread_list.size(); i++) {
			pthread_join(thread_list[i], NULL);
		}

		LOG_INFO("Multi thread load success!");
	};

	if (option_multithread_load)
	{
		multithread_load_test();
	}
	// [END] multi thread load test


	// 文件路径预处理
	LOG_INFO("file_path: %s", file_path.c_str());
	auto split_path_tuple = Utils::SplitPath(file_path);

	// 变换物体
	if (option_change_body_trans) {
		for (int j = 0; j < bodies.count(); j++) {
			BODY* ibody = dynamic_cast<BODY*>(bodies[j]);
			api_change_body_trans(ibody, nullptr);
			LOG_INFO("api_change_body_trans for: bodies[%d]", j);
		}
	}
	load_body_timer.Split("变换物体");
	load_body_timer.PrintTimes();

	// 调用计数init 
	Timer::Timer process_timer;

	if (option_marknum_init) {
		MarkNum::Init(bodies);

		if (option_multithread_load)
		{
			MarkNum::Init(bodies1);
			MarkNum::Init(bodies2);
		}

		if (option_print_topo)
		{
			GeometryUtils::TopoChecker topochecker(bodies);
			topochecker.PrintTopo();
		}
		process_timer.Split("MarkNum");
	}

	// （性能开销大）标记每个边的EDGE编号
	// 注意：这个在load较大的模型的时候会非常慢，大模型慎用
	if (option_marknum_showedgemark) {
		MarkNum::ShowEdgeMark(hoopsview);
		process_timer.Split("ShowEdgeMark");
	}

	if (option_marknum_showfacemark) {
		MarkNum::ShowFaceMark(hoopsview);
		process_timer.Split("ShowFaceMark");
	}

	std::set<int> selected_bodies;

	// 选择一个要解决的问题
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
		process_timer.Split("option_solve_stitch");
	}

	if (option_solve_stitch_for_each_bodies)
	{
		for (int i = 0; i < bodies.count(); i++) { // 对每个零件单独执行
			// 备注：如果卡死的话试一下看是不是api_construct……之类的问题
			ENTITY* ibody = bodies[i];
			ENTITY_LIST ibody_list;
			ibody_list.add(ibody);
			bool selected = false;

			LOG_INFO("Stitching for body: [%d]", i);

			Stitch::StitchGapFixer stitchGapFixer(ibody_list);
			selected |= stitchGapFixer.Start(true, true);
			stitchGapFixer.Clear();

			LOG_INFO("Stitching for body (second pass): [%d]", i);
			selected |= stitchGapFixer.Start(true, false);

			if (selected) {
				selected_bodies.insert(MarkNum::GetId(ibody));
			}

			// 保存请使用选项：option_save_bodies_respectly
		}
		process_timer.Split("option_solve_stitch_for_each_bodies");
	}

	if (option_solve_stitch_for_each_bodies_multithread)
	{
		LOG_INFO("Multithread stitch start");

		std::vector<pthread_t> thread_list;

		ENTITY_LIST ibody_list[99];
		int ibody_list_id = 0;

		for (int i = 0; i < bodies1.count(); i++) {
			ENTITY* ibody = bodies1[i];
			ibody_list[ibody_list_id].add(ibody);
			bool selected = false;

			//Stitch::StitchGapFixer stitchGapFixer(ibody_list[ibody_list_id]);
			
			thread_list.emplace_back(pthread_t());
			pthread_create(&thread_list.back(), NULL, (void* (*)(void *))StitchTask, (void*)&ibody_list[ibody_list_id]);

			ibody_list_id += 1;
		}

		for (int i = 0; i < bodies2.count(); i++) {
			ENTITY* ibody = bodies2[i];
			ibody_list[ibody_list_id].add(ibody);
			bool selected = false;

			//Stitch::StitchGapFixer stitchGapFixer(ibody_list[ibody_list_id]);

			thread_list.emplace_back(pthread_t());
			pthread_create(&thread_list.back(), NULL, (void* (*)(void *))StitchTask, (void*)&ibody_list[ibody_list_id]);

			ibody_list_id += 1;
		}

		for (int i = 0; i < thread_list.size(); i++) {
			pthread_join(thread_list[i], NULL);
		}

		LOG_INFO("Multithread stitch all done");
	}


	if (option_solve_remove_degenerated_faces) {
		DegeneratedFaces::DegeneratedFacesFixer degeneratedFaceFixer(bodies);
		degeneratedFaceFixer.Start();
		process_timer.Split("option_solve_remove_degenerated_faces");
	}

	if (option_solve_nonmanifold) {
		// A_ent(2).sat, cyl3
		//NonManifold::NonManifoldFixer nonManifoldFixer(bodies);
		//nonManifoldFixer.Start();

		NonManifold::NonManifoldFixer2 nonManifoldFixer2(bodies);
		nonManifoldFixer2.Start();

		process_timer.Split("option_solve_nonmanifold");
	}

	if (option_solve_nonmanifold_for_each_bodies) {
		for (int i = 0; i < bodies.count(); i++) { // 对每个零件单独执行
			ENTITY* ibody = bodies[i];
			ENTITY_LIST ibody_list;
			ibody_list.add(ibody);
			bool selected = false;

			LOG_INFO("Solving Nonmanifold for body: [%d]", i);
			NonManifold::NonManifoldFixer2 nonManifoldFixer2(ibody_list);
			selected |= nonManifoldFixer2.Start();

			if (selected) {
				selected_bodies.insert(MarkNum::GetId(ibody));
			}
			// 保存请使用选项：option_save_bodies_respectly
		}

		process_timer.Split("option_solve_nonmanifold_for_each_bodies");
	}

	if (option_solve_single_side_faces) {
		// 设定边的双边
		SingleSideFaces::SingleSideFacesFixer singleSideFacesFixer(bodies);
		singleSideFacesFixer.Start();
		process_timer.Split("option_solve_single_side_faces");
	}

	// 当模型中仍存在单面边的时候，尝试复制一遍对应的body，组合成double model
	if (option_make_double_model) {
		DoubleModel::DoubleModelMaker doubleModelMaker(bodies);
		doubleModelMaker.Start();

		process_timer.Split("option_make_double_model");
	}

	/* [实验] */
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

	if (option_exp5) {
		if (option_exp5_import)
		{
			GeometryImporter::Importer importer(bodies);
			importer.Start("D:\\hqh_study\\ModelForFix\\ModelForFixChecked\\GrosslyUndersize\\A_ent1(1)_cf_modified_geometry_json_0.json");
		}

		Exp5::Exp5 exp5(bodies);
		exp5.StartExperiment();
	}

	if (option_exp6) {
		Exp6::Exp6 exp6(bodies);

		exp6.StartExperiment();
	}


	if (option_exp250305)
	{
		Exp250305::Exp250305 exp250305(bodies);
		exp250305.Start();
	}

	if (option_marknum_init2) {
		MarkNum::Clear();
		MarkNum::Init(bodies);

		if (option_print_topo)
		{
			GeometryUtils::TopoChecker topochecker(bodies);
			topochecker.PrintTopo();
		}

		process_timer.Split("MarkNum");
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
		process_timer.Split("option_save_bodies");
	}

	// （240408）保存bodies list中的各个body
	if (option_save_bodies_respectly)
	{
		Utils::SaveModifiedBodiesRespectly(split_path_tuple, bodies);
		process_timer.Split("option_save_bodies_respectly");
	}

	//if (option_save_stl_bodies_respectly) {
	//	Utils::SAT2STL(split_path_tuple, bodies);
	//}

	if (option_export_geometry_json_selected) {
		GeometryExporter::Exporter exporter(bodies);
		exporter.Start(split_path_tuple, selected_bodies);
		process_timer.Split("option_export_geometry_json_selected");
	}

	if (option_export_geometry_json) {
		GeometryExporter::Exporter exporter(bodies);
		exporter.Start(split_path_tuple);
		process_timer.Split("option_export_geometry_json");

		if (option_export_debugshow_json) {
			exporter.ExportDebugPoints(split_path_tuple);
		}
	}


	// 这个没用，所以砍掉吧
	//if (option_export_entity_list_stl) {
	//	Utils::EntityList2STL(split_path_tuple, bodies);
	//}
	process_timer.PrintTimes();
	LOG_INFO("ALL DONE");

	// 不能在文件保存之前调用此api_stop_modeller
	pthread_mutex_destroy(&mutex1);

	api_stop_modeller();
}

#endif