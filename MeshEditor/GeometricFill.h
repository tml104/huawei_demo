#pragma once
#include "pixel.h"
#include "Cell.h"
#include <vector>
#include <queue>
#include "setAttr.h"
using namespace Ohm_slice;
struct FillInfo {
	int patch_id;
	int x;
	int y;
	int fill_id;
	int fill_x;
	int fill_y;
	FillInfo() {}
	FillInfo(int patch_id_, int x_, int y_, int fill_id_, int fill_x_, int fill_y_) : patch_id(patch_id_), x(x_), y(y_), fill_id(fill_id_), fill_x(fill_x_), fill_y(fill_y_) {}
};
bool geometric_fill(Ohm_slice::Patch* patch);
//void geometric_fill(Ohm_slice::Patch* patch, std::vector<std::vector<node> > & body_list);
//void geometric_fill(std::vector<std::vector<node> > & body_list);

//void test_geo_fill();
//void show_patch(Patch* path);
//void gf_set_metal_part(Patch* patch);


void geometric_fill_the_pixels(Ohm_slice::Pixels* pixels);
void geometric_fill_the_texture(Ohm_slice::Texture* texture);


void geometryFillWithSide(Ohm_slice::Cell *cell);