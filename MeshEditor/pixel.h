//#pragma once
//#include <fstream>
//#include <string>
//#include <iostream>
//#include <set>
////#include <execution>
//#include <vector>
//#include <unordered_set>
//#include <fstream>
//
//namespace Ohm_slice {
//
//	using namespace std;
//
//	//一个pixel
//	struct Pixel {
//		short order, region;
//	};
//
//
//	class Patch
//	{
//	public:
//		//int xPos, yPos;
//		std::vector<std::vector<Ohm_slice::Pixel*> > pixel_list;
//		Patch() {
//			pixel_list.resize(16);
//			for (int i = 0; i < 16; i++) {
//				pixel_list[i].resize(16);
//			}
//		}
//	};
//
//
//	class Cell
//	{
//	public:
//		//int xPos, yPos, zPos;
//		std::vector<Patch*> patch_list;
//		Cell() {
//			patch_list.resize(6);
//		}
//	};
//
//
//	// 二维数组pixel构成的一个texture面
//	struct Texture {
//		int width, height;
//
//		Texture(int width, int height, int init, Pixel* buf) : buf(buf), width(width), height(height) {
//			data = new Pixel *[height];
//			for (int i = 0; i < height; i++) {
//				data[i] = buf + i * width;
//				for (int j = 0; j < width; j++) {
//					data[i][j].order = init;
//					data[i][j].region = -1;
//				}
//			}
//
//			//patch_list.resize(height / 16);
//			//for (int i = 0; i < patch_list.size(); ++i) {
//			//	patch_list[i].resize(width / 16);
//			//}
//
//			//for (int i = 0; i < height; ++i) {
//			//	for (int j = 0; j < width; ++j) {
//			//		int rowId = i / 16;
//			//		int colId = j / 16;
//			//		patch_list[rowId][colId].pixel_list[i % 16][j % 16] = &data[i][j];
//			//		//patch_list[rowId][colId].pixel_list[i % 16][j % 16].region = data[i][j].region;
//			//	}
//			//	//cout << "i: "<<i << endl;
//			//}
//		
//		}
//
//
//		void show() {
//			for (int i = 816; i < 848; ++i) {
//				for (int j = 640; j < 672; ++j) {
//					cout << data[i][j].order << " ";
//				}
//				cout << endl;
//			}
//		}
//
//
//		~Texture() {
//			delete data;
//		}
//		Pixel*& operator[] (int i) {
//			return data[i];
//		}
//
//		vector<vector<Patch> > patch_list;
//
//	//private:
//	public:
//		Pixel** data;
//		Pixel* buf;
//		Texture() { }
//	};
//
//	// 二维数组texture构成的pixels
//	struct Pixels {
//		int width, height, thickness;
//
//		Pixels(int thickness, int width, int height, int init) :
//			width(width), height(height), thickness(thickness) {
//			//cout << thickness << "  " << width << "   " << height << endl;
//			//cout << thickness * width * height << endl;
//
//			buf = new Pixel[thickness * width * height];
//			cout << "构建Pixels的buf完成" << endl;
//
//			data = new Texture * [thickness];
//			cout << "构建Pixels的data完成" << endl;
//
//			for (int i = 0; i < thickness; i++) {
//				data[i] = new Texture(width, height, init, buf + i * width * height);
//				cout << "构建Pixels的第 " << i << " 个Texture完成" << endl;
//			}
//		}
//
//		Texture& operator [](int i) {
//			return *data[i];
//		}
//		void Dump(string file) {
//			ofstream out(file);
//			out.write((char*)&thickness, sizeof(int));
//			out.write((char*)&width, sizeof(int));
//			out.write((char*)&height, sizeof(int));
//			out.write((char*)buf, sizeof(Pixel) * thickness * width * height);
//		}
//
//		static Pixels* Load(string file) {
//			ifstream out(file);
//			int width, height, thickness;
//			out.read((char*)&thickness, sizeof(int));
//			out.read((char*)&width, sizeof(int));
//			out.read((char*)&height, sizeof(int));
//
//			//thickness = 100;
//
//			//cout << typeid(thickness).name() << endl;
//			std::cout << "thickness is: " << thickness << " width is: " << width << " height is: " << height << std::endl;
//
//			Pixels& pixels = *(new Pixels(thickness, width, height, 0));
//
//			cout << "构建pixels完成" << endl;
//			out.read((char*)pixels.buf, sizeof(Pixel) * thickness * width * height);
//			cout << "读取pixels数据完成" << endl;
//			return &pixels;
//		}
//		
//
//
//		~Pixels() {
//			for (int i = 0; i < thickness; i++) {
//				delete data[i];
//			}
//			delete data;
//			delete buf;
//		}
//	//private:
//	public:
//		Texture** data;
//		Pixel* buf;
//		Pixels() { }
//	};
//
//}
#pragma once
#include <cstdlib>
#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_set>
using std::string;
using std::ofstream;
using std::ifstream;
namespace Ohm_slice{
struct Pixel {

    short order, region, geom;
	Pixel(){}
	Pixel(short order1,short region1,short geom1):order(order1),region(region1),geom(geom1){}
};

struct Texture {

    int width, height;

    Texture(size_t width, size_t height, int init, Pixel *buf): buf(buf) {

        this->width = width;

        this->height = height;

        data = new Pixel *[height];

        for (int i = 0; i < height; i ++) {

            data[i] = buf + i * width;

            for (int j = 0; j < width; j ++) {

                data[i][j].order = init;

                data[i][j].region = -1;

                data[i][j].geom = -1;

            }

        }

    }

    ~Texture() {

        delete data;

    }

    Pixel *&operator[] (int i) {

        return data[i];

    }
    Pixel* GetPixel(size_t h,size_t w){
        return &data[h][w];
    }
private:

    Pixel **data;

    Pixel *buf;

    Texture() { }

};

struct Pixels {

    int width, height, thickness;

    Pixels(size_t thickness, size_t width, size_t height, int init):

            width(width), height(height), thickness(thickness) {

        buf = new Pixel[thickness * width * height];

        data = new Texture *[thickness];

        for (int i = 0; i < thickness; i ++) {

            data[i] = new Texture(width, height, init, buf + i * width * height);

        }

    }

    Texture &operator [](int i) {

        return *data[i];

    }

    void Dump(string file) {

        ofstream fn(file);

        fn.write((char *) buf, sizeof(Pixel) * thickness * width * height);

    }

    void Load(string file) {

        ifstream fn(file,std::ios::binary);
        if(!fn){
            std::cout<<"fail to open\n"<<std::endl;
            return;
        }
        //fn.read((char*)buf,sizeof(Pixel) * thickness * width *height);
        for(int i=0;i<thickness;i++){
            fn.read((char*)(buf+i * width * height),sizeof(Pixel) * width * height);
        }
    }

    ~Pixels() {

        for (int i = 0; i < thickness; i ++) {

            delete data[i];

        }

        delete data;

        delete buf;

    }
	Texture** GetData(){
		return data;
	}
	
private:

    Texture **data;

    Pixel *buf;

    Pixels() { }

};
}