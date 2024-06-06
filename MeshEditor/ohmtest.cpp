
#include "StdAfx.h"
//#include "pixel.h"
//#include "Cell.h"
//#include <iostream>
//#include "lodepng.h"
//#include <string>
//#include "GeometricFill.h"
//
//
//using namespace Ohm_slice;
//
// //a: 259  b: 291  c: 79
//void get_cell(Pixels& pixels1, Pixels& pixels2, Pixels& pixels3, int a, int b, int c)
//{
//	vector<vector<vector<Cell> > > cell_list;
//	cell_list.resize(a - 1);
//	for (int i = 0; i < a - 1; ++i) {
//		cell_list[i].resize(b - 1);
//		for (int j = 0; j < b - 1; ++j) {
//			cell_list[i][j].resize(c - 1);
//
//		}
//	}
//	// x z y
//	for (int i = 0; i < a - 1; i++) {
//		for (int j = 0; j < c - 1; j++) {
//			for (int k = 0; k < b - 1; k++) {
//				cell_list[i][k][j].patch_list[0] = &pixels1[i].patch_list[j][k];
//				cell_list[i][k][j].patch_list[1] = &pixels1[i + 1].patch_list[j][k];
//			}
//		}
//	}
//	// y x z
//	for (int i = 0; i < b - 1; i++) {
//		for (int j = 0; j < a - 1; j++) {
//			for (int k = 0; k < c - 1; k++) {
//				cell_list[j][i][k].patch_list[2] = &pixels2[i].patch_list[j][k];
//				cell_list[j][i][k].patch_list[3] = &pixels2[i + 1].patch_list[j][k];
//			}
//		}
//	}
//	// z y x
//	for (int i = 0; i < c - 1; i++) {
//		for (int j = 0; j < b - 1; j++) {
//			for (int k = 0; k < a - 1; k++) {
//				cell_list[k][j][i].patch_list[4] = &pixels3[i].patch_list[j][k];
//				cell_list[k][j][i].patch_list[5] = &pixels3[i + 1].patch_list[j][k];
//			}
//		}
//	}
//
//	cout << "generate cell have done" << endl;
//
//}
//
//
//void dump_pixels(string file, Texture &input, int ns, int type = 0,
//
//	int vmin = -20, int vmax = 20) {
//
//	// https://stackoverflow.com/questions/36288421/c-create-png-bitmap-from-array-of-numbers
//
//	auto pixels = vector<unsigned char>();
//
//	for (int j = 0; j < input.height; j++) {
//
//		for (int i = 0; i < input.width; i++) {
//
//			auto &value = input[j][i];
//			
//			//将-1和-2都归为-9999
//			//value.order = (value.order == 9999 ? 9999 : -9999);
//
//
//			auto v = type == 0 ? value.order : value.region * 10;
//
//			//// -9999:0  9999:255
//			////auto p = min(max((v - vmin) * 255 / (vmax - vmin), 0), 255);
//			//int p;
//			//if (v == 9999) {
//			//	p = 255;
//			//}
//			//else if (v == -9999) {
//			//	p = 0;
//			//}
//			//else if (v == -1) {
//			//	p = 90;
//			//}
//			//else if (v == -2) {
//			//	p = 180;
//			//}
//
//
//			// 是否是边界 0 || 16
//			auto k = i % ns == 0 || j % ns == 0 || i % ns == ns - 1 || j % ns == ns - 1;
//
//			if (k) {
//				pixels.push_back(0);
//				pixels.push_back(0);
//				pixels.push_back(0);
//			}
//			else {
//				if (v == 9999) {
//					pixels.push_back(255);
//					pixels.push_back(0);
//					pixels.push_back(0);
//				}
//				else if (v == -9999) {
//					pixels.push_back(0);
//					pixels.push_back(0);
//					pixels.push_back(255);
//				}
//				else if (v == -1) {
//					pixels.push_back(0);
//					pixels.push_back(255);
//					pixels.push_back(0);
//				}
//				else if (v == -2) {
//					pixels.push_back(0);
//					pixels.push_back(255);
//					pixels.push_back(255);
//				}
//			}
//			
//			//pixels.push_back(0);
//			//pixels.push_back(p);
//			//pixels.push_back(k ? 255 : p);
//
//		}
//
//	}
//
//	lodepng::encode(file, pixels.data(), input.width, input.height, LCT_RGB);
//
//}
//
////int main()
////{
////	//Pixels& pixels1 = *Pixels::Load("C:\\Users\\AAA\\Desktop\\数据结构与测试文件-by华为\\ex\\ex");
////	Pixels& pixels2 = *Pixels::Load("C:\\Users\\AAA\\Desktop\\数据结构与测试文件-by华为\\ey\\ey");
////	//Pixels& pixels3 = *Pixels::Load("C:\\Users\\AAA\\Desktop\\数据结构与测试文件-by华为\\ez\\ez");
////
////	//geometric_fill_the_pixels(&pixels1);
////
////	//string filename = "ex-";
////	//for (int i = 0; i < pixels1.thickness; i++) {
////	//	string file = filename + to_string(i) + ".png";
////	//	//file += ".png";
////	//	dump_pixels(file, pixels1[i], 16);
////	//}
////
////	string filename = "ey-";
////	for (int i = 0; i < pixels2.thickness; i++) {
////		string file = filename + to_string((long double)i) + ".png";
////		//file += ".png";
////		dump_pixels(file, pixels2[i], 16);
////	}
////
////	//string filename = "ez-";
////	//for (int i = 0; i < pixels3.thickness; i++) {
////	//	string file = filename + to_string(i) + ".png";
////	//	//file += ".png";
////	//	dump_pixels(file, pixels3[i], 16);
////	//}
////
////	//cout << "照片打印结束" << endl;
////	//get_cell(pixels1, pixels2, pixels3, pixels1.thickness, pixels2.thickness, pixels3.thickness);
////
////	//delete& pixels1;
////	delete& pixels2;
////	//delete& pixels3;
////
////	return 0;
////}
////
