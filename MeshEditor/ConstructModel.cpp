#include "StdAfx.h"
#include "ConstructModel.h"

void OldConstructModel::Test1()
{
	//api_start_modeller(0);
	api_initialize_constructors();

	BODY *block;
	api_make_cuboid(100, 50, 200, block);
	FILE *output = fopen("C:\\Users\\AAA\\Documents\\WeChat Files\\wxid_4y4wts4jkg9h21\\FileStorage\\File\\2023-08\\cube.dbg","w");

	if (output == NULL) {
		printf("[ConstructModel::Test1] output file ptr NULL\n");
	}
	else {
		debug_size(block, output);
		fclose(output);
	}

	// save
	ENTITY_LIST save_list;
	save_list.add(block);

	//Utils::SaveToSAT(
	//	"C:\\Users\\AAA\\Documents\\WeChat Files\\wxid_4y4wts4jkg9h21\\FileStorage\\File\\2023-08\\cube.sat",
	//	save_list
	//);

	api_terminate_constructors();
	//api_stop_modeller();

	printf("[ConstructModel::Test1] Done \n");
}

void OldConstructModel::Test2() {

	//api_start_modeller(0);
	api_initialize_constructors();

	BODY* hat;
	api_solid_cylinder_cone(
		SPAposition(40, 40, 40),
		SPAposition(140, 140, 140),
		10 * M_PI,
		20 * M_PI,
		0,
		NULL,
		hat
	);

	double area = 0;
	double accuracy = 0;
	api_ent_area((ENTITY*)hat, 0.01, area, accuracy);

	printf("[ConstructModel::Test2] area: %f\n", area);

	// save
	//Utils::SaveToSATBody(
	//	"C:\\Users\\AAA\\Documents\\WeChat Files\\wxid_4y4wts4jkg9h21\\FileStorage\\File\\2023-08\\hat.sat",
	//	hat
	//);

	api_terminate_constructors();
	//api_stop_modeller();
	printf("[ConstructModel::Test2] Done \n");

}

void OldConstructModel::Test3() {
	//api_start_modeller(0);
	api_initialize_constructors();
	api_initialize_booleans();

	BODY *cyl1, *cyl2; 

	api_make_frustum(100, 20, 20, 20, cyl1);
	api_make_frustum(100, 20, 20, 20, cyl2);

	SPAtransf rotX = rotate_transf(M_PI / 2, SPAvector(1, 0, 0));
	api_apply_transf(cyl1, rotX);

	api_intersect(cyl1, cyl2);
	//Utils::SaveToSATBody(
	//	"C:\\Users\\AAA\\Documents\\WeChat Files\\wxid_4y4wts4jkg9h21\\FileStorage\\File\\2023-08\\rot_cyl.sat",
	//	cyl2
	//);

	printf("[ConstructModel::Test3] Done \n");

	api_terminate_constructors();
	api_terminate_booleans();
	//api_stop_modeller();
}

void OldConstructModel::Test4() {
	//api_start_modeller(0);
	api_initialize_constructors();
	api_initialize_booleans();

	BODY *cyl1, *cyl2, *cyl3;

	api_make_frustum(100, 20, 20, 20, cyl1);
	api_make_frustum(100, 10, 10, 10, cyl2);
	api_make_frustum(100, 10, 10, 10, cyl3);

	//SPAtransf rotX = rotate_transf(M_PI / 2, SPAvector(1, 0, 0));
	//api_apply_transf(cyl1, rotX);

	// transform cyl3
	SPAtransf movee = translate_transf(SPAvector(0, 20,0 ));
	api_apply_transf(cyl3, movee);


	api_subtract(cyl2, cyl1);
	api_subtract(cyl3, cyl1);
	//Utils::SaveToSATBody(
	//	"C:\\Users\\AAA\\Documents\\WeChat Files\\wxid_4y4wts4jkg9h21\\FileStorage\\File\\2023-08\\rot_cyl3.sat",
	//	cyl1
	//);

	printf("[ConstructModel::Test4] Done \n");

	api_terminate_constructors();
	api_terminate_booleans();
	//api_stop_modeller();
}

/*
	重叠立方体
*/
BODY* ConstructModel::MyModelConstructor::Construct240708(const std::string& file_name)
{
	BODY *block1, *block2;
	api_make_cuboid(10.0, 10.0, 10.0, block1);
	api_make_cuboid(10.0, 10.0, 10.0, block2);

	// 变换block2
	SPAtransf t2 = translate_transf(SPAvector(5.0, 5.0, 0.0));
	api_apply_transf(block2, t2);

	// eulerapi.hxx: api_combine_body
	api_combine_body(block1, block2);

	api_change_body_trans(block2, nullptr);

	this->save_constructed_body(file_name, block2);
	return block2;
}

BODY * ConstructModel::MyModelConstructor::Construct240710TotallyCoincident(const std::string & file_name)
{
	BODY *block1, *block2;
	api_make_cuboid(10.0, 10.0, 10.0, block1);
	api_make_cuboid(10.0, 10.0, 10.0, block2);

	api_combine_body(block1, block2);

	this->save_constructed_body(file_name, block2);
	return block2;
}

void ConstructModel::MyModelConstructor::save_constructed_body(const std::string & file_name, BODY* body)
{
	Utils::SaveToSATBody(this->rt_save_path + file_name, body);
}
