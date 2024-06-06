#include "stdafx.h"
#include "GeometricFill.h"


void geometric_fill_the_pixels(Ohm_slice::Pixels* pixels)
{	
	Ohm_slice::Texture** texture = pixels->GetData();
	for (int i = 0; i < pixels->thickness; ++i) {
		//cout << "pixels的thickness是： "<< i << endl;
		geometric_fill_the_texture(texture[i]);
	}
}

void geometric_fill_the_texture(Ohm_slice::Texture* texture)
{
	for (int h = 0; h < texture->height / 16; ++h) {
		for (int w = 0; w < texture->width / 16; ++w) {
			//这里就是每个patch
			Patch patch;
			int patch_x_index = 16 * w;
			int patch_y_index = 16 * h;
			for (int i = 0; i < 16; ++i) {
				for (int j = 0; j < 16; ++j) {
					patch.pixel_list[i][j] = texture->GetPixel(patch_y_index+i,patch_x_index+j);
				}
			}
			geometric_fill(&patch);
		}
	}
}


//对patch填充
bool geometric_fill(Ohm_slice::Patch* patch)
{
	// 一共有几个区域,包括金属和非金属
	std::vector<std::vector<Ohm_slice::Pixel*> > block;
	// 是否入过队列
	bool inq[256] = { false };
	// 小正方形的序号即为16*x+y
	auto bfs = [&](int u) {
		std::vector<Ohm_slice::Pixel*> v;
		int tag = patch->pixel_list[u / 16][u % 16]->order;
		std::queue<int> q;
		q.push(u);
		inq[u] = true;
		while (!q.empty()) {
			int u = q.front();
			q.pop();
			int x = u / 16;
			int y = u % 16;
			v.push_back(patch->pixel_list[x][y]);
			if ((x - 1 >= 0) && (patch->pixel_list[x - 1][y]->order == tag) && (inq[(x - 1) * 16 + y] == false)) {
				q.push((x - 1) * 16 + y);
				inq[(x - 1) * 16 + y] = true;
			}
			if ((x + 1 <= 15) && (patch->pixel_list[x + 1][y]->order == tag) && (inq[(x + 1) * 16 + y] == false)) {
				q.push((x + 1) * 16 + y);
				inq[(x + 1) * 16 + y] = true;
			}
			if ((y - 1 >= 0) && (patch->pixel_list[x][y - 1]->order == tag) && (inq[x * 16 + y - 1] == false)) {
				q.push(x * 16 + y - 1);
				inq[x * 16 + y - 1] = true;
			}
			if ((y + 1 <= 15) && (patch->pixel_list[x][y + 1]->order == tag) && (inq[x * 16 + y + 1] == false)) {
				q.push(x * 16 + y + 1);
				inq[x * 16 + y + 1] = true;
			}
		}
		block.push_back(v);
	};

	//对分金属区域进行bfs得到非金属区域的所有block
	for (int i = 0; i < 256; ++i) {
		//&& patch->pixel_list[i / 16][i % 16]->order != 9999
		if (!inq[i] ) {
			bfs(i);
		}
	}

	int num_of_metal_region = 0;
	int num_of_nonmetal_region = 0;
	int metal_order = 0;
	int metal_geom = 0;
	for (int i = 0; i < block.size(); ++i) {
		if (block[i][0]->order > 0) {
			++num_of_metal_region;
			metal_geom = block[i][0]->geom;
			metal_order = block[i][0]->order;
		}
		else {
			++num_of_nonmetal_region;
		}
	}

	//考虑必须存在一个金属区域
	//if (num_of_metal_region == 0 || num_of_nonmetal_region < 3) {
	//	cout << "不需要进行几何填充" << endl;
	//	return;
	//}

	//不考虑金属区域，只考虑非金属区域数量
	if (!num_of_metal_region || num_of_nonmetal_region < 3) {
		//cout << "不需要进行几何填充" << endl;
		return false;
	}

	//cout << "需要进行几何填充" << endl;
	while (num_of_nonmetal_region > 2) {
		int mintag = -1, minnum = 999999;
		for (int i = 0; i < block.size(); ++i) {
			if (block[i][0]->order <= 0 && block[i].size() < minnum) {
				mintag = i;
				minnum = block[i].size();
			}
		}

		//cout << "像素数最少的block的序号为： " << mintag << "   , 其像素个数为： " << block[mintag].size() << endl;

		for (int i = 0; i < block[mintag].size(); ++i) {
			block[mintag][i]->order = metal_order;
			block[mintag][i]->geom = metal_geom;
		}
		block.erase(block.begin() + mintag);
		num_of_nonmetal_region--;
	}
	
	return true;
}



void geometryFillWithSide(Ohm_slice::Cell *cell) {
	
	std::queue<FillInfo> first_fill_list,side_fill_list;
	std::vector<std::vector<std::vector<bool>>> vis(6, std::vector<std::vector<bool>>(16, std::vector<bool>(16, false)));
	for (int patch_id = 0; patch_id < 6; patch_id++) {
		while (cell->patch_list[patch_id]->non_metal_info_list.size() > 2) {
			printf("patch id: %d\n", patch_id);
			int x = cell->patch_list[patch_id]->non_metal_info_list[0].x;
			int y = cell->patch_list[patch_id]->non_metal_info_list[0].y;
			int fill_x = cell->patch_list[patch_id]->non_metal_info_list[0].fill_x;
			int fill_y = cell->patch_list[patch_id]->non_metal_info_list[0].fill_y;
			first_fill_list.push(FillInfo( patch_id, x, y, patch_id, fill_x, fill_y ));
			
			cell->patch_list[patch_id]->non_metal_info_list.erase(cell->patch_list[patch_id]->non_metal_info_list.begin());
		}
	}
	std::queue<FillInfo> q;
	
	while (!first_fill_list.empty()) {
		q.push(first_fill_list.front());
		vis[first_fill_list.front().patch_id][first_fill_list.front().x][first_fill_list.front().y] = true;
		first_fill_list.pop();
		while (!q.empty()) {
			FillInfo fillInfo = q.front();
			q.pop();
			Ohm_slice::Pixel *to = cell->patch_list[fillInfo.patch_id]->pixel_list[fillInfo.x][fillInfo.y];
			Ohm_slice::Pixel *from = cell->patch_list[fillInfo.fill_id]->pixel_list[fillInfo.fill_x][fillInfo.fill_y];
			copyPixelInfo(to, from);
			if (fillInfo.x > 0) {
				if (cell->patch_list[fillInfo.patch_id]->pixel_list[fillInfo.x - 1][fillInfo.y]->order < 0 &&
					!vis[fillInfo.patch_id][fillInfo.x - 1][fillInfo.y]) {
					vis[fillInfo.patch_id][fillInfo.x - 1][fillInfo.y] = true;
					q.push(FillInfo(fillInfo.patch_id, fillInfo.x - 1, fillInfo.y, fillInfo.fill_id, fillInfo.fill_x, fillInfo.fill_y));
				}
			}
			else {
				printf("side test1\n");
				int target_id = 0, target_x = 0, target_y = 0;
				autoPosMap(fillInfo.patch_id, fillInfo.x, fillInfo.y, target_id, target_x, target_y);
				printf("target:%d %d %d\n", target_id, target_x, target_y);
				if (cell->patch_list[target_id]->pixel_list[target_x][target_y]->order < 0 &&
					!vis[target_id][target_x][target_y]) {
					printf("side test11\n");
					side_fill_list.push(FillInfo(target_id, target_x, target_y, fillInfo.fill_id, fillInfo.fill_x, fillInfo.fill_y));
				}
				
			}
			if (fillInfo.x < 15) {
				if (cell->patch_list[fillInfo.patch_id]->pixel_list[fillInfo.x + 1][fillInfo.y]->order < 0 &&
					!vis[fillInfo.patch_id][fillInfo.x + 1][fillInfo.y]) {
					vis[fillInfo.patch_id][fillInfo.x + 1][fillInfo.y] = true;
					q.push(FillInfo(fillInfo.patch_id, fillInfo.x + 1, fillInfo.y, fillInfo.fill_id, fillInfo.fill_x, fillInfo.fill_y));
				}
			}
			else {
				printf("side test2\n");
				int target_id = 0, target_x = 0, target_y = 0;
				autoPosMap(fillInfo.patch_id, fillInfo.x, fillInfo.y, target_id, target_x, target_y);
				printf("target:%d %d %d\n", target_id, target_x, target_y);
				if (cell->patch_list[target_id]->pixel_list[target_x][target_y]->order < 0 &&
					!vis[target_id][target_x][target_y]) {
					printf("side test22\n");
					side_fill_list.push(FillInfo(target_id, target_x, target_y, fillInfo.fill_id, fillInfo.fill_x, fillInfo.fill_y));
				}

			}

			if (fillInfo.y > 0) {
				if (cell->patch_list[fillInfo.patch_id]->pixel_list[fillInfo.x][fillInfo.y - 1]->order < 0 &&
					!vis[fillInfo.patch_id][fillInfo.x][fillInfo.y - 1]) {
					vis[fillInfo.patch_id][fillInfo.x][fillInfo.y - 1] = true;
					q.push(FillInfo(fillInfo.patch_id, fillInfo.x, fillInfo.y - 1, fillInfo.fill_id, fillInfo.fill_x, fillInfo.fill_y));
				}
			}
			else {
				printf("side test3\n");
				int target_id = 0, target_x = 0, target_y = 0;
				autoPosMap(fillInfo.patch_id, fillInfo.x, fillInfo.y, target_id, target_x, target_y);
				printf("target:%d %d %d\n", target_id, target_x, target_y);
				if (cell->patch_list[target_id]->pixel_list[target_x][target_y]->order < 0 &&
					!vis[target_id][target_x][target_y]) {
					printf("side test33\n");
					side_fill_list.push(FillInfo(target_id, target_x, target_y, fillInfo.fill_id, fillInfo.fill_x, fillInfo.fill_y));
				}

			}
			if (fillInfo.y < 15) {
				if (cell->patch_list[fillInfo.patch_id]->pixel_list[fillInfo.x][fillInfo.y + 1]->order < 0 &&
					!vis[fillInfo.patch_id][fillInfo.x][fillInfo.y + 1]) {
					vis[fillInfo.patch_id][fillInfo.x][fillInfo.y + 1] = true;
					q.push(FillInfo(fillInfo.patch_id, fillInfo.x, fillInfo.y + 1, fillInfo.fill_id, fillInfo.fill_x, fillInfo.fill_y));
				}
			}
			else {
				printf("side test4\n");
				int target_id = 0, target_x = 0, target_y = 0;
				autoPosMap(fillInfo.patch_id, fillInfo.x, fillInfo.y, target_id, target_x, target_y);
				printf("target:%d %d %d\n", target_id, target_x, target_y);
				if (cell->patch_list[target_id]->pixel_list[target_x][target_y]->order < 0 &&
					!vis[target_id][target_x][target_y]) {
					printf("side test44\n");
					side_fill_list.push(FillInfo(target_id, target_x, target_y, fillInfo.fill_id, fillInfo.fill_x, fillInfo.fill_y));
				}

			}
			if ((fillInfo.x == 0 && fillInfo.y == 0) || (fillInfo.x == 0 && fillInfo.y == 15) || (fillInfo.x == 15 && fillInfo.y == 0) || (fillInfo.x == 15 && fillInfo.y == 15)) {
				struct pos {
					int patch;
					int x;
					int y;
					pos() {}
					pos(int patch_, int x_, int y_) : patch(patch_), x(x_), y(y_) {}
				};
				pos new_pos1, new_pos2;
				autoPosMap2(fillInfo.patch_id, fillInfo.x, fillInfo.y, new_pos1.patch, new_pos1.x, new_pos1.y, new_pos2.patch, new_pos2.x, new_pos2.y);
				if (!vis[new_pos1.patch][new_pos1.x][new_pos1.y] && cell->patch_list[new_pos1.patch]->pixel_list[new_pos1.x][new_pos1.y]->order < 0) {
					side_fill_list.push(FillInfo(new_pos1.patch, new_pos1.x, new_pos1.y, fillInfo.fill_id, fillInfo.fill_x, fillInfo.fill_y));
					
				}
				if (!vis[new_pos2.patch][new_pos2.x][new_pos2.y] && cell->patch_list[new_pos2.patch]->pixel_list[new_pos2.x][new_pos2.y]->order < 0) {
					side_fill_list.push(FillInfo(new_pos2.patch, new_pos2.x, new_pos2.y, fillInfo.fill_id, fillInfo.fill_x, fillInfo.fill_y));
				}
			}
			
		}
		printf("side fill size: %d\n", side_fill_list.size());
		while (!side_fill_list.empty()) {
			FillInfo fillInfo = side_fill_list.front();
			side_fill_list.pop();
			Ohm_slice::Pixel* to = cell->patch_list[fillInfo.patch_id]->pixel_list[fillInfo.x][fillInfo.y];
			Ohm_slice::Pixel* from = cell->patch_list[fillInfo.fill_id]->pixel_list[fillInfo.fill_x][fillInfo.fill_y];
			if (to->order < 0) {
				copyPixelInfo(to, from);
			}
		}
	}
}
