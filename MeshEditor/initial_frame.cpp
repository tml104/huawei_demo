#include "StdAfx.h"
#include "topologyoptwidget.h"
#include <dlib/optimization.h>
using namespace dlib;
typedef matrix<double,0,1> column_vec;
#define s 0.0001

column_vec input_array_frame;
double frame_fun(const column_vec& coef)
{
	double result = 0;
	double x1(coef(0)), y1(coef(1)), z1(coef(2)), x2(coef(3)), y2(coef(4)), z2(coef(5)), x3(coef(6)), y3(coef(7)), z3(coef(8));
	double a1(input_array_frame(0)), b1(input_array_frame(1)), c1(input_array_frame(2)), w1(input_array_frame(3)), a2(input_array_frame(4)), b2(input_array_frame(5)), c2(input_array_frame(6)), 
		w2(input_array_frame(7)), a3(input_array_frame(8)), b3(input_array_frame(9)), c3(input_array_frame(10)), w3(input_array_frame(11));
	result += w1*pow(a1*x1+b1*y1+c1*z1, 2)/(x1*x1+y1*y1+z1*z1+s);
	result += w2*pow(a2*x2+b2*y2+c2*z2, 2)/(x2*x2+y2*y2+z2*z2+s);
	result += w3*pow(a3*x3+b3*y3+c3*z3, 2)/(x3*x3+y3*y3+z3*z3+s);
	result -= 1000*pow(x1*x2+y1*y2+z1*z2, 2)/((x1*x1+y1*y1+z1*z1+s)*(x2*x2+y2*y2+z2*z2+s));
	result -= 1000*pow(x2*x3+y2*y3+z2*z3, 2)/((x3*x3+y3*y3+z3*z3+s)*(x2*x2+y2*y2+z2*z2+s));
	result -= 1000*pow(x1*x3+y1*y3+z1*z3, 2)/((x3*x3+y3*y3+z3*z3+s)*(x1*x1+y1*y1+z1*z1+s));
	return result;
}
column_vec frame_fun_deri(const column_vec& coef)
{
	column_vec result(9);
	for(int i = 0; i < 9; i++)
		result(i) = 0;
	double x1(coef(0)), y1(coef(1)), z1(coef(2)), x2(coef(3)), y2(coef(4)), z2(coef(5)), x3(coef(6)), y3(coef(7)), z3(coef(8));
	double a1(input_array_frame(0)), b1(input_array_frame(1)), c1(input_array_frame(2)), w1(input_array_frame(3)), a2(input_array_frame(4)), b2(input_array_frame(5)), c2(input_array_frame(6)),
		w2(input_array_frame(7)),a3(input_array_frame(8)), b3(input_array_frame(9)), c3(input_array_frame(10)), w3(input_array_frame(11));
	result(0) += w1*2*(a1*(a1*x1+b1*y1+c1*z1)*(x1*x1+y1*y1+z1*z1+s)-x1*pow(a1*x1+b1*y1+c1*z1, 2))/pow(x1*x1+y1*y1+z1*z1+s, 2);
	result(0) -= 2000*(x2*(x1*x2+y1*y2+z1*z2)*(x1*x1+y1*y1+z1*z1+s)*(x2*x2+y2*y2+z2*z2+s)-x1*(x2*x2+y2*y2+z2*z2+s)*pow(x1*x2+y1*y2+z1*z2, 2))/pow((x1*x1+y1*y1+z1*z1+s)*(x2*x2+y2*y2+z2*z2+s), 2);
	result(0) -= 2000*(x3*(x1*x3+y1*y3+z1*z3)*(x1*x1+y1*y1+z1*z1+s)*(x3*x3+y3*y3+z3*z3+s)-x1*(x3*x3+y3*y3+z3*z3+s)*pow(x1*x3+y1*y3+z1*z3, 2))/pow((x1*x1+y1*y1+z1*z1+s)*(x3*x3+y3*y3+z3*z3+s), 2);

	result(1) += w1*2*(b1*(a1*x1+b1*y1+c1*z1)*(x1*x1+y1*y1+z1*z1+s)-y1*pow(a1*x1+b1*y1+c1*z1, 2))/pow(x1*x1+y1*y1+z1*z1+s, 2);
	result(1) -= 2000*(y2*(x1*x2+y1*y2+z1*z2)*(x1*x1+y1*y1+z1*z1+s)*(x2*x2+y2*y2+z2*z2+s)-y1*(x2*x2+y2*y2+z2*z2+s)*pow(x1*x2+y1*y2+z1*z2, 2))/pow((x1*x1+y1*y1+z1*z1+s)*(x2*x2+y2*y2+z2*z2+s), 2);
	result(1) -= 2000*(y3*(x1*x3+y1*y3+z1*z3)*(x1*x1+y1*y1+z1*z1+s)*(x3*x3+y3*y3+z3*z3+s)-y1*(x3*x3+y3*y3+z3*z3+s)*pow(x1*x3+y1*y3+z1*z3, 2))/pow((x1*x1+y1*y1+z1*z1+s)*(x3*x3+y3*y3+z3*z3+s), 2);

	result(2) += w1*2*(c1*(a1*x1+b1*y1+c1*z1)*(x1*x1+y1*y1+z1*z1+s)-z1*pow(a1*x1+b1*y1+c1*z1, 2))/pow(x1*x1+y1*y1+z1*z1+s, 2);
	result(2) -= 2000*(z2*(x1*x2+y1*y2+z1*z2)*(x1*x1+y1*y1+z1*z1+s)*(x2*x2+y2*y2+z2*z2+s)-z1*(x2*x2+y2*y2+z2*z2+s)*pow(x1*x2+y1*y2+z1*z2, 2))/pow((x1*x1+y1*y1+z1*z1+s)*(x2*x2+y2*y2+z2*z2+s), 2);
	result(2) -= 2000*(z3*(x1*x3+y1*y3+z1*z3)*(x1*x1+y1*y1+z1*z1+s)*(x3*x3+y3*y3+z3*z3+s)-z1*(x3*x3+y3*y3+z3*z3+s)*pow(x1*x3+y1*y3+z1*z3, 2))/pow((x1*x1+y1*y1+z1*z1+s)*(x3*x3+y3*y3+z3*z3+s), 2);

	result(3) += w2*2*(a2*(a2*x2+b2*y2+c2*z2)*(x2*x2+y2*y2+z2*z2+s)-x2*pow(a2*x2+b2*y2+c2*z2, 2))/pow(x2*x2+y2*y2+z2*z2+s, 2);
	result(3) -= 2000*(x1*(x1*x2+y1*y2+z1*z2)*(x1*x1+y1*y1+z1*z1+s)*(x2*x2+y2*y2+z2*z2+s)-x2*(x1*x1+y1*y1+z1*z1+s)*pow(x1*x2+y1*y2+z1*z2, 2))/pow((x1*x1+y1*y1+z1*z1+s)*(x2*x2+y2*y2+z2*z2+s), 2);
	result(3) -= 2000*(x3*(x3*x2+y3*y2+z3*z2)*(x3*x3+y3*y3+z3*z3+s)*(x2*x2+y2*y2+z2*z2+s)-x2*(x3*x3+y3*y3+z3*z3+s)*pow(x3*x2+y3*y2+z3*z2, 2))/pow((x3*x3+y3*y3+z3*z3+s)*(x2*x2+y2*y2+z2*z2+s), 2);

	result(4) += w2*2*(b2*(a2*x2+b2*y2+c2*z2)*(x2*x2+y2*y2+z2*z2+s)-y2*pow(a2*x2+b2*y2+c2*z2, 2))/pow(x2*x2+y2*y2+z2*z2+s, 2);
	result(4) -= 2000*(y1*(x1*x2+y1*y2+z1*z2)*(x1*x1+y1*y1+z1*z1+s)*(x2*x2+y2*y2+z2*z2+s)-y2*(x1*x1+y1*y1+z1*z1+s)*pow(x1*x2+y1*y2+z1*z2, 2))/pow((x1*x1+y1*y1+z1*z1+s)*(x2*x2+y2*y2+z2*z2+s), 2);
	result(4) -= 2000*(y3*(x3*x2+y3*y2+z3*z2)*(x3*x3+y3*y3+z3*z3+s)*(x2*x2+y2*y2+z2*z2+s)-y2*(x3*x3+y3*y3+z3*z3+s)*pow(x3*x2+y3*y2+z3*z2, 2))/pow((x3*x3+y3*y3+z3*z3+s)*(x2*x2+y2*y2+z2*z2+s), 2);

	result(5) += w2*2*(c2*(a2*x2+b2*y2+c2*z2)*(x2*x2+y2*y2+z2*z2+s)-z2*pow(a2*x2+b2*y2+c2*z2, 2))/pow(x2*x2+y2*y2+z2*z2+s, 2);
	result(5) -= 2000*(z1*(x1*x2+y1*y2+z1*z2)*(x1*x1+y1*y1+z1*z1+s)*(x2*x2+y2*y2+z2*z2+s)-z2*(x1*x1+y1*y1+z1*z1+s)*pow(x1*x2+y1*y2+z1*z2, 2))/pow((x1*x1+y1*y1+z1*z1+s)*(x2*x2+y2*y2+z2*z2+s), 2);
	result(5) -= 2000*(z3*(x3*x2+y3*y2+z3*z2)*(x3*x3+y3*y3+z3*z3+s)*(x2*x2+y2*y2+z2*z2+s)-z2*(x3*x3+y3*y3+z3*z3+s)*pow(x3*x2+y3*y2+z3*z2, 2))/pow((x3*x3+y3*y3+z3*z3+s)*(x2*x2+y2*y2+z2*z2+s), 2);

	result(6) += w3*2*(a3*(a3*x3+b3*y3+c3*z3)*(x3*x3+y3*y3+z3*z3+s)-x3*pow(a3*x3+b3*y3+c3*z3, 2))/pow(x3*x3+y3*y3+z3*z3+s, 2);
	result(6) -= 2000*(x2*(x3*x2+y3*y2+z3*z2)*(x3*x3+y3*y3+z3*z3+s)*(x2*x2+y2*y2+z2*z2+s)-x3*(x2*x2+y2*y2+z2*z2+s)*pow(x3*x2+y3*y2+z3*z2, 2))/pow((x3*x3+y3*y3+z3*z3+s)*(x2*x2+y2*y2+z2*z2+s), 2);
	result(6) -= 2000*(x1*(x1*x3+y1*y3+z1*z3)*(x1*x1+y1*y1+z1*z1+s)*(x3*x3+y3*y3+z3*z3+s)-x3*(x1*x1+y1*y1+z1*z1+s)*pow(x1*x3+y1*y3+z1*z3, 2))/pow((x1*x1+y1*y1+z1*z1+s)*(x3*x3+y3*y3+z3*z3+s), 2);

	result(7) += w3*2*(b3*(a3*x3+b3*y3+c3*z3)*(x3*x3+y3*y3+z3*z3+s)-y3*pow(a3*x3+b3*y3+c3*z3, 2))/pow(x3*x3+y3*y3+z3*z3+s, 2);
	result(7) -= 2000*(y2*(x3*x2+y3*y2+z3*z2)*(x3*x3+y3*y3+z3*z3+s)*(x2*x2+y2*y2+z2*z2+s)-y3*(x2*x2+y2*y2+z2*z2+s)*pow(x3*x2+y3*y2+z3*z2, 2))/pow((x3*x3+y3*y3+z3*z3+s)*(x2*x2+y2*y2+z2*z2+s), 2);
	result(7) -= 2000*(y1*(x1*x3+y1*y3+z1*z3)*(x1*x1+y1*y1+z1*z1+s)*(x3*x3+y3*y3+z3*z3+s)-y3*(x1*x1+y1*y1+z1*z1+s)*pow(x1*x3+y1*y3+z1*z3, 2))/pow((x1*x1+y1*y1+z1*z1+s)*(x3*x3+y3*y3+z3*z3+s), 2);

	result(8) += w3*2*(c3*(a3*x3+b3*y3+c3*z3)*(x3*x3+y3*y3+z3*z3+s)-z3*pow(a3*x3+b3*y3+c3*z3, 2))/pow(x3*x3+y3*y3+z3*z3+s, 2);
	result(8) -= 2000*(z2*(x3*x2+y3*y2+z3*z2)*(x3*x3+y3*y3+z3*z3+s)*(x2*x2+y2*y2+z2*z2+s)-z3*(x2*x2+y2*y2+z2*z2+s)*pow(x3*x2+y3*y2+z3*z2, 2))/pow((x3*x3+y3*y3+z3*z3+s)*(x2*x2+y2*y2+z2*z2+s), 2);
	result(8) -= 2000*(z1*(x1*x3+y1*y3+z1*z3)*(x1*x1+y1*y1+z1*z1+s)*(x3*x3+y3*y3+z3*z3+s)-z3*(x1*x1+y1*y1+z1*z1+s)*pow(x1*x3+y1*y3+z1*z3, 2))/pow((x1*x1+y1*y1+z1*z1+s)*(x3*x3+y3*y3+z3*z3+s), 2);
	
	return result;
}
void ini_frame_arr(OvmVec3d x_vec, OvmVec3d y_vec, OvmVec3d z_vec)
{
	input_array_frame.set_size(12);
	std::vector<double> weight;
	double w1 = x_vec.length();
	double w2 = y_vec.length();
	double w3 = z_vec.length();
	double w = w1+w2+w3;
	w1 = w1/w;
	w2 = w2/w;
	w3 = w3/w;

	std::vector<OvmVec3d> vec;
	vec.push_back(x_vec.normalize_cond());
	vec.push_back(y_vec.normalize_cond());
	vec.push_back(z_vec.normalize_cond());

	input_array_frame(0) = vec[0][0];
	input_array_frame(1) = vec[0][1];
	input_array_frame(2) = vec[0][2];
	input_array_frame(3) = w1;
	input_array_frame(4) = vec[1][0];
	input_array_frame(5) = vec[1][1];
	input_array_frame(6) = vec[1][2];
	input_array_frame(7) = w2;
	input_array_frame(8) = vec[2][0];
	input_array_frame(9) = vec[2][1];
	input_array_frame(10) = vec[2][2];
	input_array_frame(11) = w3;
}
Frame get_frame(OvmVec3d center, OvmVec3d x_vec, OvmVec3d y_vec, OvmVec3d z_vec)
{
	column_vec coef(9);
	//设定初值
	OvmVec3d x(x_vec), y(y_vec), z(z_vec);
	std::vector<OvmVec3d> vec;
	vec.push_back(x.normalize_cond());
	vec.push_back(y.normalize_cond());
	vec.push_back(z.normalize_cond());
	std::vector<int> order;
	order.push_back(0);
	order.push_back(1);
	order.push_back(2);
	if(abs(vec[order[0]][0]) < abs(vec[order[1]][0])){
		int temp = order[1];
		order[1] = order[0];
		order[0] = temp;
	}
	if(abs(vec[order[0]][0]) < abs(vec[order[2]][0])){
		int temp = order[2];
		order[2] = order[0];
		order[0] = temp;
	}
	if(abs(vec[order[1]][1]) < abs(vec[order[2]][1])){
		int temp = order[1];
		order[1] = order[2];
		order[2] = temp;
	}
	std::vector<int> initial_values;
	if(order[0] == 0){
		coef(0) = 1;
		coef(1) = 0;
		coef(2) = 0;
	}
	else if(order[1] == 0){
		coef(0) = 0;
		coef(1) = 1;
		coef(2) = 0;
	}
	else if(order[2] == 0){
		coef(0) = 0;
		coef(1) = 0;
		coef(2) = 1;
	}
	if(order[0] == 1){
		coef(3) = 1;
		coef(4) = 0;
		coef(5) = 0;
	}
	else if(order[1] == 1){
		coef(3) = 0;
		coef(4) = 1;
		coef(5) = 0;
	}
	else if(order[2] == 1){
		coef(3) = 0;
		coef(4) = 0;
		coef(5) = 1;
	}
	if(order[0] == 2){
		coef(6) = 1;
		coef(7) = 0;
		coef(8) = 0;
	}
	else if(order[1] == 2){
		coef(6) = 0;
		coef(7) = 1;
		coef(8) = 0;
	}
	else if(order[2] == 2){
		coef(6) = 0;
		coef(7) = 0;
		coef(8) = 1;
	}
	ini_frame_arr(x_vec, y_vec, z_vec);
	auto t_before = frame_fun(coef);
	//std::cout << "before: " << t_before;
	find_max(bfgs_search_strategy(),objective_delta_stop_strategy(1e-7),frame_fun, frame_fun_deri, coef, 2);
	auto t_after = frame_fun(coef);
	//std::cout << "  after: " << t_after << '\n';
	Frame f(center, OvmVec3d(coef(0), coef(1), coef(2)), OvmVec3d(coef(3), coef(4), coef(5)), OvmVec3d(coef(6), coef(7), coef(8)));
	f.x_vec = f.x_vec.normalize_cond();
    f.y_vec = f.y_vec.normalize_cond();
	f.z_vec = f.z_vec.normalize_cond();
	return f;
}
Frame get_frame_1_1(OvmVec3d center, OvmVec3d x_vec, OvmVec3d y_vec, OvmVec3d z_vec)
{
	column_vec coef(9);
	OvmVec3d x_vec_nor = x_vec/x_vec.length();
	coef(0) = x_vec_nor[0];
	coef(1) = x_vec_nor[1];
	coef(2) = x_vec_nor[2];
	if(y_vec.length() > z_vec.length()){
		OvmVec3d y_vec_nor = y_vec - dot(y_vec, x_vec_nor)*x_vec_nor;
		y_vec_nor = y_vec_nor.normalize();
		OvmVec3d T = cross(x_vec_nor, y_vec_nor);
		coef(3) = y_vec_nor[0];
		coef(4) = y_vec_nor[1];
		coef(5) = y_vec_nor[2];
		coef(6) = T[0];
		coef(7) = T[1];
		coef(8) = T[2];
	}
	else{
		OvmVec3d z_vec_nor = z_vec - dot(z_vec, x_vec_nor)*x_vec_nor;
		z_vec_nor = z_vec_nor.normalize();
		OvmVec3d T = cross(z_vec_nor, x_vec_nor);
		coef(3) = T[0];
		coef(4) = T[1];
		coef(5) = T[2];
		coef(6) = z_vec_nor[0];
		coef(7) = z_vec_nor[1];
		coef(8) = z_vec_nor[2];
	}
	ini_frame_arr(x_vec, y_vec, z_vec);
	auto t_before = frame_fun(coef);
	//std::cout << "1before: " << t_before;
	find_max(bfgs_search_strategy(),objective_delta_stop_strategy(1e-7),frame_fun, frame_fun_deri, coef, 2);
	auto t_after = frame_fun(coef);
	//std::cout << "  after: " << t_after << '\n';
	Frame f(center, OvmVec3d(coef(0), coef(1), coef(2)), OvmVec3d(coef(3), coef(4), coef(5)), OvmVec3d(coef(6), coef(7), coef(8)));
	f.x_vec = f.x_vec.normalize_cond();
	f.y_vec = f.y_vec.normalize_cond();
	f.z_vec = f.z_vec.normalize_cond();
	return f;
}
Frame get_frame_1_2(OvmVec3d center, OvmVec3d x_vec, OvmVec3d y_vec, OvmVec3d z_vec)
{
	column_vec coef(9);
	OvmVec3d y_vec_nor = y_vec/y_vec.length();
	coef(3) = y_vec_nor[0];
	coef(4) = y_vec_nor[1];
	coef(5) = y_vec_nor[2];
	if(x_vec.length() > z_vec.length()){
		OvmVec3d x_vec_nor = x_vec - dot(x_vec, y_vec_nor)*y_vec_nor;
		x_vec_nor = x_vec_nor.normalize();
		OvmVec3d T = cross(x_vec_nor, y_vec_nor);
		coef(0) = x_vec_nor[0];
		coef(1) = x_vec_nor[1];
		coef(2) = x_vec_nor[2];
		coef(6) = T[0];
		coef(7) = T[1];
		coef(8) = T[2];
	}
	else{
		OvmVec3d z_vec_nor = z_vec - dot(z_vec, y_vec_nor)*y_vec_nor;
		z_vec_nor = z_vec_nor.normalize();
		OvmVec3d T = cross(z_vec_nor, y_vec_nor);
		coef(0) = T[0];
		coef(1) = T[1];
		coef(2) = T[2];
		coef(6) = z_vec_nor[0];
		coef(7) = z_vec_nor[1];
		coef(8) = z_vec_nor[2];
	}
	ini_frame_arr(x_vec, y_vec, z_vec);
	auto t_before = frame_fun(coef);
	//std::cout << "1before: " << t_before;
	find_max(bfgs_search_strategy(),objective_delta_stop_strategy(1e-7),frame_fun, frame_fun_deri, coef, 2);
	auto t_after = frame_fun(coef);
	//std::cout << "  after: " << t_after << '\n';
	Frame f(center, OvmVec3d(coef(0), coef(1), coef(2)), OvmVec3d(coef(3), coef(4), coef(5)), OvmVec3d(coef(6), coef(7), coef(8)));
	f.x_vec = f.x_vec.normalize_cond();
	f.y_vec = f.y_vec.normalize_cond();
	f.z_vec = f.z_vec.normalize_cond();
	return f;
}
Frame get_frame_1_3(OvmVec3d center, OvmVec3d x_vec, OvmVec3d y_vec, OvmVec3d z_vec)
{
	column_vec coef(9);
	OvmVec3d z_vec_nor = z_vec/z_vec.length();
	coef(6) = z_vec_nor[0];
	coef(7) = z_vec_nor[1];
	coef(8) = z_vec_nor[2];
	if(x_vec.length() > y_vec.length()){
		OvmVec3d x_vec_nor = x_vec - dot(x_vec, z_vec_nor)*z_vec_nor;
		x_vec_nor = x_vec_nor.normalize();
		OvmVec3d T = cross(x_vec_nor, z_vec_nor);
		coef(0) = x_vec_nor[0];
		coef(1) = x_vec_nor[1];
		coef(2) = x_vec_nor[2];
		coef(3) = T[0];
		coef(4) = T[1];
		coef(5) = T[2];
	}
	else{
		OvmVec3d y_vec_nor = y_vec - dot(y_vec, z_vec_nor)*z_vec_nor;
		y_vec_nor = y_vec_nor.normalize();
		OvmVec3d T = cross(y_vec_nor, z_vec_nor);
		coef(0) = T[0];
		coef(1) = T[1];
		coef(2) = T[2];
		coef(3) = y_vec_nor[0];
		coef(4) = y_vec_nor[1];
		coef(5) = y_vec_nor[2];
	}
	ini_frame_arr(x_vec, y_vec, z_vec);
	auto t_before = frame_fun(coef);
	//std::cout << "1before: " << t_before;
	find_max(bfgs_search_strategy(),objective_delta_stop_strategy(1e-7),frame_fun, frame_fun_deri, coef, 2);
	auto t_after = frame_fun(coef);
	//std::cout << "  after: " << t_after << '\n';
	Frame f(center, OvmVec3d(coef(0), coef(1), coef(2)), OvmVec3d(coef(3), coef(4), coef(5)), OvmVec3d(coef(6), coef(7), coef(8)));
	f.x_vec = f.x_vec.normalize_cond();
	f.y_vec = f.y_vec.normalize_cond();
	f.z_vec = f.z_vec.normalize_cond();
	return f;
}
Frame get_frame_2_1(OvmVec3d center, OvmVec3d x_vec, OvmVec3d y_vec, OvmVec3d z_vec)
{
	column_vec coef(9);
	OvmVec3d z_vec_nor = z_vec/z_vec.length();
	auto T = cross(z_vec_nor, x_vec);
	coef(0) = x_vec[0];
	coef(1) = x_vec[1];
	coef(2) = x_vec[2];
	coef(3) = T[0];
	coef(4) = T[1];
	coef(5) = T[2];
	coef(6) = z_vec_nor[0];
	coef(7) = z_vec_nor[1];
	coef(8) = z_vec_nor[2];
	ini_frame_arr(x_vec, y_vec, z_vec);
	auto t_before = frame_fun(coef);
	//std::cout << "2before: " << t_before;
	find_max(bfgs_search_strategy(),objective_delta_stop_strategy(1e-7),frame_fun, frame_fun_deri, coef, 2);
	auto t_after = frame_fun(coef);
	//std::cout << "  after: " << t_after << '\n';
	Frame f(center, OvmVec3d(coef(0), coef(1), coef(2)), OvmVec3d(coef(3), coef(4), coef(5)), OvmVec3d(coef(6), coef(7), coef(8)));
	f.x_vec = f.x_vec.normalize_cond();
	f.y_vec = f.y_vec.normalize_cond();
	f.z_vec = f.z_vec.normalize_cond();
	return f;
}
Frame get_frame_2_2(OvmVec3d center, OvmVec3d x_vec, OvmVec3d y_vec, OvmVec3d z_vec)
{
	column_vec coef(9);
	OvmVec3d y_vec_nor = y_vec/y_vec.length();
	auto T = cross(x_vec, y_vec_nor);
	coef(0) = x_vec[0];
	coef(1) = x_vec[1];
	coef(2) = x_vec[2];
	coef(3) = y_vec_nor[0];
	coef(4) = y_vec_nor[1];
	coef(5) = y_vec_nor[2];
	coef(6) = T[0];
	coef(7) = T[1];
	coef(8) = T[2];
	ini_frame_arr(x_vec, y_vec, z_vec);
	auto t_before = frame_fun(coef);
	//std::cout << "2before: " << t_before;
	find_max(bfgs_search_strategy(),objective_delta_stop_strategy(1e-7),frame_fun, frame_fun_deri, coef, 2);
	auto t_after = frame_fun(coef);
	//std::cout << "  after: " << t_after << '\n';
	Frame f(center, OvmVec3d(coef(0), coef(1), coef(2)), OvmVec3d(coef(3), coef(4), coef(5)), OvmVec3d(coef(6), coef(7), coef(8)));
	f.x_vec = f.x_vec.normalize_cond();
	f.y_vec = f.y_vec.normalize_cond();
	f.z_vec = f.z_vec.normalize_cond();
	return f;
}
Frame get_frame_2_3(OvmVec3d center, OvmVec3d x_vec, OvmVec3d y_vec, OvmVec3d z_vec)
{
	column_vec coef(9);
	OvmVec3d x_vec_nor = x_vec/x_vec.length();
	auto T = cross(x_vec_nor, y_vec);
	coef(0) = x_vec_nor[0];
	coef(1) = x_vec_nor[1];
	coef(2) = x_vec_nor[2];
	coef(3) = y_vec[0];
	coef(4) = y_vec[1];
	coef(5) = y_vec[2];
	coef(6) = T[0];
	coef(7) = T[1];
	coef(8) = T[2];
	ini_frame_arr(x_vec, y_vec, z_vec);
	auto t_before = frame_fun(coef);
	//std::cout << "2before: " << t_before;
	find_max(bfgs_search_strategy(),objective_delta_stop_strategy(1e-7),frame_fun, frame_fun_deri, coef, 2);
	auto t_after = frame_fun(coef);
	//std::cout << "  after: " << t_after << '\n';
	Frame f(center, OvmVec3d(coef(0), coef(1), coef(2)), OvmVec3d(coef(3), coef(4), coef(5)), OvmVec3d(coef(6), coef(7), coef(8)));
	f.x_vec = f.x_vec.normalize_cond();
	f.y_vec = f.y_vec.normalize_cond();
	f.z_vec = f.z_vec.normalize_cond();
	return f;
}

void get_initial_frame(VolumeMesh*mesh, std::hash_map<OvmCeH, Frame> & cell_frame_mapping){
	for(auto c_iter = mesh->cells_begin(); c_iter != mesh->cells_end(); c_iter++){
		OvmVec3d center_point(0,0,0);
		OvmVec3d X_vec(0,0,0), Y_vec(0,0,0), Z_vec(0,0,0);
		std::vector<OvmVec3d> fh_center;
		std::vector<OvmHaFaH> hfhs = mesh->cell(*c_iter).halffaces();
		for(int i = 0; i < 6; i++){
			OvmVec3d temp(0,0,0);
			for(auto hfv_iter = mesh->hfv_iter(hfhs[i]); hfv_iter; hfv_iter++){
				temp += mesh->vertex(*hfv_iter);
			}
			temp /= 4;
			fh_center.push_back(temp);
		}
		//计算出六面体的对面连线
		X_vec = fh_center[1]-fh_center[0];
		Y_vec = fh_center[3]-fh_center[2];
		Z_vec = fh_center[5]-fh_center[4];
		//计算出六面体中心点位置
		foreach(auto temp, fh_center)
			center_point += temp;
		center_point /= 6;

		/*std::cout<<"\n"<<*c_iter<<"\n";
		std::cout<<"x_vec:"<<X_vec[0]<<","<<X_vec[1]<<","<<X_vec[2]<<"\n";
		std::cout<<"y_vec:"<<Y_vec[0]<<","<<Y_vec[1]<<","<<Y_vec[2]<<"\n";
		std::cout<<"z_vec:"<<Z_vec[0]<<","<<Z_vec[1]<<","<<Z_vec[2]<<"\n";*/
		Frame f;
		if(!is_boundary_cell(mesh, *c_iter)){
			f = get_frame(center_point, X_vec, Y_vec, Z_vec);
		    f.set_tag(-1);
		}
		else{
			//判断该cell和几个几何面连接
			std::vector<int> boundary_fh_index;
			for(int i = 0; i < 3; i++){
				if(mesh->is_boundary(mesh->face_handle(hfhs[2*i])))
					boundary_fh_index.push_back(2*i);
				else if(mesh->is_boundary(mesh->face_handle(hfhs[2*i+1])))
					boundary_fh_index.push_back(2*i+1);
			}

			//cell只与一个面邻接的情况
			if(boundary_fh_index.size() == 1){
				FACE *f_temp = get_associated_geometry_face_of_boundary_fh(mesh, mesh->face_handle(hfhs[boundary_fh_index[0]]));
				OvmVec3d c(0,0,0);
				SPAposition closet_pos;
				double dis;
				foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[0]]).halfedges()){
					auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
					c += vh;
				}
				c /= 4;
				api_entity_point_distance(f_temp, POS2SPA(c), closet_pos, dis);
				auto pram = f_temp->geometry()->trans_surface()->param(closet_pos);
				auto n = f_temp->geometry()->trans_surface()->eval_normal(pram);
				OvmVec3d N = OvmVec3d(n.x(), n.y(), n.z());
				double max_length(X_vec.length());
				if(Y_vec.length() > max_length)
					max_length = Y_vec.length();
				if(Z_vec.length() > max_length)
					max_length = Z_vec.length();
				//另外两个向量求解
				if(boundary_fh_index[0] == 0 || boundary_fh_index[0] == 1){
					f = get_frame_1_1(center_point, N*max_length*100, Y_vec, Z_vec);
					f.set_tag(0);
				}
				else if(boundary_fh_index[0] == 2 || boundary_fh_index[0] == 3){
					f = get_frame_1_2(center_point, X_vec, N*max_length*100, Z_vec);
					f.set_tag(1);
				}
				else{
					f = get_frame_1_3(center_point, X_vec, Y_vec, N*max_length*100);
					f.set_tag(2);
				}
			}
			//cell与两个几何面邻接
			else if(boundary_fh_index.size() == 2){
				std::vector<OvmVec3d> normals;

				//有一类特殊情况，一个六面体的网格面位于同一个几何面上,仅保持一个
				FACE *f0 = get_associated_geometry_face_of_boundary_fh(mesh, mesh->face_handle(hfhs[boundary_fh_index[0]]));
				FACE *f1 = get_associated_geometry_face_of_boundary_fh(mesh, mesh->face_handle(hfhs[boundary_fh_index[1]]));
				if(f0 == f1){
					OvmVec3d c(0,0,0);
					SPAposition closet_pos;
					double dis;
					foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[0]]).halfedges()){
						auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
						c += vh;
					}
					foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[1]]).halfedges()){
						auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
						c += vh;
					}
					c /= 8;
					api_entity_point_distance(f0, POS2SPA(c), closet_pos, dis);
					auto pram = f0->geometry()->trans_surface()->param(closet_pos);
					auto n = f0->geometry()->trans_surface()->eval_normal(pram);
					OvmVec3d N = OvmVec3d(n.x(), n.y(), n.z());
					double max_length(X_vec.length());
					if(Y_vec.length() > max_length)
						max_length = Y_vec.length();
					if(Z_vec.length() > max_length)
						max_length = Z_vec.length();
					//另外两个向量求解
					if(boundary_fh_index[0] == 0 || boundary_fh_index[0] == 1){
						f = get_frame_1_1(center_point, N*max_length*100, Y_vec, Z_vec);
						f.set_tag(0);
					}
					else if(boundary_fh_index[0] == 2 || boundary_fh_index[0] == 3){
						f = get_frame_1_2(center_point, X_vec, N*max_length*100, Z_vec);
						f.set_tag(1);
					}
					else{
						f = get_frame_1_3(center_point, X_vec, Y_vec, N*max_length*100);
						f.set_tag(2);
					}
				}
				else{
					for(int i = 0; i < 2; i++){
						FACE *f_temp = get_associated_geometry_face_of_boundary_fh(mesh, mesh->face_handle(hfhs[boundary_fh_index[i]]));
						OvmVec3d c(0,0,0);
						SPAposition closet_pos;
						double dis;
						foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[i]]).halfedges()){
							auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
							c += vh;
						}
						c /= 4;
						api_entity_point_distance(f_temp, POS2SPA(c), closet_pos, dis);
						auto pram = f_temp->geometry()->trans_surface()->param(closet_pos);
						auto n = f_temp->geometry()->trans_surface()->eval_normal(pram);
						normals.push_back(OvmVec3d(n.x(), n.y(), n.z()));
					}
					auto N = cross(normals[0], normals[1]);
					//另外两个向量求解
					if((boundary_fh_index[0] == 0 || boundary_fh_index[0] == 1) && (boundary_fh_index[1] == 2 || boundary_fh_index[1] == 3)){
						f = get_frame_2_1(center_point, normals[0], normals[1], N*100);
						f.set_tag(5);
					}
					else if((boundary_fh_index[0] == 0 || boundary_fh_index[0] == 1) && (boundary_fh_index[1] == 4 || boundary_fh_index[1] == 5)){
						f = get_frame_2_2(center_point, normals[0], N*100, normals[1]);
						f.set_tag(4);
					}
					else if((boundary_fh_index[0] == 2 || boundary_fh_index[0] == 3) && (boundary_fh_index[1] == 4 || boundary_fh_index[1] == 5)){
						f = get_frame_2_3(center_point, N*100, normals[0], normals[1]);
						f.set_tag(3);
					}
				}
			}
			//cell与三个几何面相交
			else{
				std::vector<OvmVec3d> normals;
	            //情况分析不够
				//有几类特殊情况，一个六面体的不同网格面位于同一个几何面上
				std::vector<FACE*> fs;
				for(int i= 0; i < 3; i++)
					fs.push_back(get_associated_geometry_face_of_boundary_fh(mesh, mesh->face_handle(hfhs[boundary_fh_index[i]])));
				std::vector<OvmVec3d> normals_test;
				for(int i = 0; i < 3; i++){
					OvmVec3d c(0,0,0);
					SPAposition closet_pos;
					double dis;
					foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[i]]).halfedges()){
						auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
						c += vh;
					}
					c /= 4;
					api_entity_point_distance(fs[i], POS2SPA(c), closet_pos, dis);
					auto pram = fs[i]->geometry()->trans_surface()->param(closet_pos);
					auto n = fs[i]->geometry()->trans_surface()->eval_normal(pram);
					OvmVec3d N = OvmVec3d(n.x(), n.y(), n.z());
					normals_test.push_back(N);
				}
				//三个面相同
				if(fs[0] == fs[1] && fs[0] == fs[2]){
					OvmVec3d c(0,0,0);
					SPAposition closet_pos;
					double dis;
					foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[0]]).halfedges()){
						auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
						c += vh;
					}
					foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[1]]).halfedges()){
						auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
						c += vh;
					}
					foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[2]]).halfedges()){
						auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
						c += vh;
					}
					c /= 12;
					api_entity_point_distance(fs[0], POS2SPA(c), closet_pos, dis);
					auto pram = fs[0]->geometry()->trans_surface()->param(closet_pos);
					auto n = fs[0]->geometry()->trans_surface()->eval_normal(pram);
					OvmVec3d N = OvmVec3d(n.x(), n.y(), n.z());
					double max_length(X_vec.length());
					if(Y_vec.length() > max_length)
						max_length = Y_vec.length();
					if(Z_vec.length() > max_length)
						max_length = Z_vec.length();
					//另外两个向量求解
					f = get_frame_1_1(center_point, N*max_length*100, Y_vec, Z_vec);
					f.set_tag(0);
				}
				//前两个面相同
				else if((fs[0] == fs[1]) || (abs(dot(normals_test[0], normals_test[1])) > 0.95)){
					OvmVec3d c(0,0,0);
					SPAposition closet_pos;
					double dis;
					foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[0]]).halfedges()){
						auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
						c += vh;
					}
					foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[1]]).halfedges()){
						auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
						c += vh;
					}
					c /= 8;
					api_entity_point_distance(fs[0], POS2SPA(c), closet_pos, dis);
					auto pram = fs[0]->geometry()->trans_surface()->param(closet_pos);
					auto n = fs[0]->geometry()->trans_surface()->eval_normal(pram);
					normals.push_back(OvmVec3d(n.x(), n.y(), n.z()));
				    c = OvmVec3d(0,0,0);
					foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[2]]).halfedges()){
						auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
						c += vh;
					}
					c /= 4;
					api_entity_point_distance(fs[2], POS2SPA(c), closet_pos, dis);
					pram = fs[2]->geometry()->trans_surface()->param(closet_pos);
					n = fs[2]->geometry()->trans_surface()->eval_normal(pram);
					normals.push_back(OvmVec3d(n.x(), n.y(), n.z()));
					auto N = cross(normals[0], normals[1]);
					//另外两个向量求解
					if((boundary_fh_index[0] == 0 || boundary_fh_index[0] == 1) && (boundary_fh_index[1] == 2 || boundary_fh_index[1] == 3)){
						f = get_frame_2_1(center_point, normals[0], normals[1], N*100);
						f.set_tag(5);
					}
					else if((boundary_fh_index[0] == 0 || boundary_fh_index[0] == 1) && (boundary_fh_index[1] == 4 || boundary_fh_index[1] == 5)){
						f = get_frame_2_2(center_point, normals[0], N*100, normals[1]);
						f.set_tag(4);
					}
					else if((boundary_fh_index[0] == 2 || boundary_fh_index[0] == 3) && (boundary_fh_index[1] == 4 || boundary_fh_index[1] == 5)){
						f = get_frame_2_3(center_point, N*100, normals[0], normals[1]);
						f.set_tag(3);
					}
				}
				else if((fs[0] == fs[2]) || (abs(dot(normals_test[0], normals_test[2])) > 0.95)){
					OvmVec3d c(0,0,0);
					SPAposition closet_pos;
					double dis;
					foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[0]]).halfedges()){
						auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
						c += vh;
					}
					foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[2]]).halfedges()){
						auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
						c += vh;
					}
					c /= 8;
					api_entity_point_distance(fs[0], POS2SPA(c), closet_pos, dis);
					auto pram = fs[0]->geometry()->trans_surface()->param(closet_pos);
					auto n = fs[0]->geometry()->trans_surface()->eval_normal(pram);
					normals.push_back(OvmVec3d(n.x(), n.y(), n.z()));
					c = OvmVec3d(0,0,0);
					foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[1]]).halfedges()){
						auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
						c += vh;
					}
					c /= 4;
					api_entity_point_distance(fs[1], POS2SPA(c), closet_pos, dis);
					pram = fs[1]->geometry()->trans_surface()->param(closet_pos);
					n = fs[1]->geometry()->trans_surface()->eval_normal(pram);
					normals.push_back(OvmVec3d(n.x(), n.y(), n.z()));
					auto N = cross(normals[0], normals[1]);
					//另外两个向量求解
					if((boundary_fh_index[0] == 0 || boundary_fh_index[0] == 1) && (boundary_fh_index[1] == 2 || boundary_fh_index[1] == 3)){
						f = get_frame_2_1(center_point, normals[0], normals[1], N*100);
						f.set_tag(5);
					}
					else if((boundary_fh_index[0] == 0 || boundary_fh_index[0] == 1) && (boundary_fh_index[1] == 4 || boundary_fh_index[1] == 5)){
						f = get_frame_2_2(center_point, normals[0], N*100, normals[1]);
						f.set_tag(4);
					}
					else if((boundary_fh_index[0] == 2 || boundary_fh_index[0] == 3) && (boundary_fh_index[1] == 4 || boundary_fh_index[1] == 5)){
						f = get_frame_2_3(center_point, N*100, normals[0], normals[1]);
						f.set_tag(3);
					}
				}
				else if((fs[1] == fs[2]) || (abs(dot(normals_test[1], normals_test[2])) > 0.95)){
					OvmVec3d c(0,0,0);
					SPAposition closet_pos;
					double dis;
					foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[1]]).halfedges()){
						auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
						c += vh;
					}
					foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[2]]).halfedges()){
						auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
						c += vh;
					}
					c /= 8;
					api_entity_point_distance(fs[1], POS2SPA(c), closet_pos, dis);
					auto pram = fs[1]->geometry()->trans_surface()->param(closet_pos);
					auto n = fs[1]->geometry()->trans_surface()->eval_normal(pram);
					normals.push_back(OvmVec3d(n.x(), n.y(), n.z()));
					c = OvmVec3d(0,0,0);
					foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[0]]).halfedges()){
						auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
						c += vh;
					}
					c /= 4;
					api_entity_point_distance(fs[0], POS2SPA(c), closet_pos, dis);
					pram = fs[0]->geometry()->trans_surface()->param(closet_pos);
					n = fs[0]->geometry()->trans_surface()->eval_normal(pram);
					normals.push_back(OvmVec3d(n.x(), n.y(), n.z()));
					auto N = cross(normals[0], normals[1]);
					//另外两个向量求解
					if((boundary_fh_index[0] == 0 || boundary_fh_index[0] == 1) && (boundary_fh_index[1] == 2 || boundary_fh_index[1] == 3)){
						f = get_frame_2_1(center_point, normals[0], normals[1], N*100);
						f.set_tag(5);
					}
					else if((boundary_fh_index[0] == 0 || boundary_fh_index[0] == 1) && (boundary_fh_index[1] == 4 || boundary_fh_index[1] == 5)){
						f = get_frame_2_2(center_point, normals[0], N*100, normals[1]);
						f.set_tag(4);
					}
					else if((boundary_fh_index[0] == 2 || boundary_fh_index[0] == 3) && (boundary_fh_index[1] == 4 || boundary_fh_index[1] == 5)){
						f = get_frame_2_3(center_point, N*100, normals[0], normals[1]);
						f.set_tag(3);
					}
				}
				else{
					for(int i = 0; i < 3; i++){
						FACE *f_temp = fs[i];
						OvmVec3d c(0,0,0);
						SPAposition closet_pos;
						double dis;
						foreach(auto heh, mesh->halfface(hfhs[boundary_fh_index[i]]).halfedges()){
							auto vh = mesh->vertex(mesh->halfedge(heh).from_vertex());
							c += vh;
						}
						c /= 4;
						api_entity_point_distance(f_temp, POS2SPA(c), closet_pos, dis);
						auto pram = f_temp->geometry()->trans_surface()->param(closet_pos);
						auto n = f_temp->geometry()->trans_surface()->eval_normal(pram);
						normals.push_back(OvmVec3d(n.x(), n.y(), n.z()));
					}
					f = get_frame(center_point, normals[0], normals[1], normals[2]);
					f.set_tag(6);
				}
			}
		}
		f.set_ch(*c_iter);

		//判断是否符合右手法则
		if(dot(cross(f.x_vec, f.y_vec), f.z_vec) < 0){
			f.z_vec = f.z_vec * -1;
		}
		cell_frame_mapping[*c_iter] = f;
	}        
	std::cout<<"done initial frames\n";
}