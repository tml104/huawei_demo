#pragma once
#include "pixel.h"
#include <unordered_set>
#include <iostream>
#include <vector>
namespace Ohm_slice {
	struct NonMetalInfo {
		int x;
		int y;
		int num;
		int fill_x;
		int fill_y;
		NonMetalInfo(){}
		NonMetalInfo(int x_,int y_,int num_):x(x_),y(y_),num(num_),fill_x(0),fill_y(0){}
		NonMetalInfo(int x_, int y_, int num_, int fill_x_, int fill_y_) :x(x_), y(y_), num(num_), fill_x(fill_x_), fill_y(fill_y_) {}
	};
    class Patch {
    public:
        Ohm_slice::Pixel *pixel_list[16][16];
		short metal_region_num, non_metal_region_num, patch_type;
		std::vector<NonMetalInfo> non_metal_info_list;
        Patch() {
			metal_region_num = 0; 
			non_metal_region_num = 0; 
			patch_type = 0;
        }
        ~Patch(){

        }
        Patch(Ohm_slice::Texture &texture, size_t h_id, size_t w_id) {
            for (size_t h = 0; h < 16; h++) {
                for (size_t w = 0; w < 16; w++) {
                    pixel_list[h][w] = texture.GetPixel(h_id * 16 + h, w_id * 16 + w);
                }
            }
        }


    };
	struct NodePosition {
		double x, y, z;
		NodePosition(double x_, double y_, double z_) :x(x_), y(y_), z(z_) {}
	};
	struct ConnectInfo {
		double x1, x2, y1, y2, z1, z2;
		ConnectInfo(NodePosition &a, NodePosition &b) {
			x1 = a.x;
			y1 = a.y;
			z1 = a.z;
			x2 = b.x;
			y2 = b.y;
			z2 = b.z;
		}
	};
	
    class Cell {
    public:
        int xPos, yPos, zPos;
        Patch *patch_list[6];
		double leftDown[3];
		double rightUp[3];
		bool isConduction[12];
		int unflodMetalNum;
		std::vector<ConnectInfo> connect_info_list;
        Cell() {
			memset((void*)isConduction, 0, 12 * sizeof(bool));
			unflodMetalNum = 0;
        }
        ~Cell(){
            for(int i=0;i<6;i++){
                delete patch_list[i];
            }
        }
        Cell(size_t xPos, size_t yPos, size_t zPos,
             Ohm_slice::Pixels &px, Ohm_slice::Pixels &py, Ohm_slice::Pixels &pz) {
            this->xPos = xPos;
            this->yPos = yPos;
            this->zPos = zPos;
            patch_list[0] = new Patch(px[xPos], zPos, yPos);
            patch_list[1] = new Patch(px[xPos + 1], zPos, yPos);
            patch_list[2] = new Patch(py[yPos], xPos, zPos);
            patch_list[3] = new Patch(py[yPos + 1], xPos, zPos);
            patch_list[4] = new Patch(pz[zPos], yPos, xPos);
            patch_list[5] = new Patch(pz[zPos + 1], yPos, xPos);

        }
        bool HasMetal(){
            for(int k = 0 ; k < 6 ; k++){
                for(int x = 0 ; x < 16 ; x++){
                    for(int y = 0 ; y < 16 ; y++){
                        if(patch_list[k]->pixel_list[x][y]->order > 0){
                            std::cout<<patch_list[k]->pixel_list[x][y]->order<<std::endl;
                            return true;
                        }
                    }
                }
            }
            return false;
        }
    };

	struct CellIdx{
		int x,y,z;
		CellIdx(){}
		CellIdx(int x_,int y_,int z_):x(x_),y(y_),z(z_){}
	};
	struct CellIdxHash{
		size_t operator()(const CellIdx& cell) const{
			return std::hash<int>()(cell.x) ^ std::hash<int>()(cell.y) ^ std::hash<int>()(cell.z);
		}
	};
	struct CellEqual{
		bool operator()(const CellIdx& c1,const CellIdx& c2) const{
			return c1.x == c2.x && c1.y == c2.y && c1.z == c2.z;
		}
	};
}