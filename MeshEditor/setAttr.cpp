#include "stdafx.h"
#include "setAttr.h"

void autoPosMap2(int face_id, int x, int y, int &target_id1, int &target_x1, int &target_y1, int &target_id2, int &target_x2, int &target_y2) {
	if (face_id == 0) {
		if (x == 0 && y == 0) {
			target_id1 = 2;
			target_x1 = 0;
			target_y1 = 0;
			target_id2 = 4;
			target_x2 = 0;
			target_y2 = 0;
		}
		else if (x == 0 && y == 15) {
			target_id1 = 3;
			target_x1 = 0;
			target_y1 = 0;
			target_id2 = 4;
			target_x2 = 15;
			target_y2 = 0;
		}
		else if (x == 15 && y == 0) {
			target_id1 = 2;
			target_x1 = 0;
			target_y1 = 15;
			target_id2 = 5;
			target_x2 = 0;
			target_y2 = 0;
		}
		else if (x == 15 && y == 15) {
			target_id1 = 3;
			target_x1 = 0;
			target_y1 = 15;
			target_id2 = 5;
			target_x2 = 15;
			target_y2 = 0;
		}
	}
	else if (face_id == 1) {
		if (x == 0 && y == 0) {
			target_id1 = 2;
			target_x1 = 15;
			target_y1 = 0;
			target_id2 = 4;
			target_x2 = 0;
			target_y2 = 15;
		}
		else if (x == 0 && y == 15) {
			target_id1 = 3;
			target_x1 = 15;
			target_y1 = 0;
			target_id2 = 4;
			target_x2 = 15;
			target_y2 = 15;
		}
		else if (x == 15 && y == 0) {
			target_id1 = 2;
			target_x1 = 15;
			target_y1 = 15;
			target_id2 = 5;
			target_x2 = 0;
			target_y2 = 15;
		}
		else if (x == 15 && y == 15) {
			target_id1 = 3;
			target_x1 = 15;
			target_y1 = 15;
			target_id2 = 5;
			target_x2 = 15;
			target_y2 = 15;
		}
	}
	else if (face_id == 2) {
		if (x == 0 && y == 0) {
			target_id1 = 0;
			target_x1 = 0;
			target_y1 = 0;
			target_id2 = 4;
			target_x2 = 0;
			target_y2 = 0;
		}
		else if (x == 0 && y == 15) {
			target_id1 = 0;
			target_x1 = 15;
			target_y1 = 0;
			target_id2 = 5;
			target_x2 = 0;
			target_y2 = 0;
		}
		else if (x == 15 && y == 0) {
			target_id1 = 1;
			target_x1 = 0;
			target_y1 = 0;
			target_id2 = 4;
			target_x2 = 0;
			target_y2 = 15;
		}
		else if (x == 15 && y == 15) {
			target_id1 = 1;
			target_x1 = 15;
			target_y1 = 0;
			target_id2 = 5;
			target_x2 = 0;
			target_y2 = 15;
		}
	}
	else if (face_id == 3) {
		if (x == 0 && y == 0) {
			target_id1 = 0;
			target_x1 = 0;
			target_y1 = 15;
			target_id2 = 4;
			target_x2 = 15;
			target_y2 = 0;
		}
		else if (x == 0 && y == 15) {
			target_id1 = 0;
			target_x1 = 15;
			target_y1 = 15;
			target_id2 = 5;
			target_x2 = 15;
			target_y2 = 0;
		}
		else if (x == 15 && y == 0) {
			target_id1 = 1;
			target_x1 = 0;
			target_y1 = 15;
			target_id2 = 4;
			target_x2 = 15;
			target_y2 = 15;
		}
		else if (x == 15 && y == 15) {
			target_id1 = 1;
			target_x1 = 15;
			target_y1 = 15;
			target_id2 = 5;
			target_x2 = 15;
			target_y2 = 15;
		}
	}
	else if (face_id == 4) {
		if (x == 0 && y == 0) {
			target_id1 = 0;
			target_x1 = 0;
			target_y1 = 0;
			target_id2 = 2;
			target_x2 = 0;
			target_y2 = 0;
		}
		else if (x == 0 && y == 15) {
			target_id1 = 1;
			target_x1 = 0;
			target_y1 = 0;
			target_id2 = 2;
			target_x2 = 15;
			target_y2 = 0;
		}
		else if (x == 15 && y == 0) {
			target_id1 = 0;
			target_x1 = 0;
			target_y1 = 15;
			target_id2 = 3;
			target_x2 = 0;
			target_y2 = 0;
		}
		else if (x == 15 && y == 15) {
			target_id1 = 1;
			target_x1 = 0;
			target_y1 = 15;
			target_id2 = 3;
			target_x2 = 15;
			target_y2 = 0;
		}
	}
	else if (face_id == 5) {
		if (x == 0 && y == 0) {
			target_id1 = 0;
			target_x1 = 15;
			target_y1 = 0;
			target_id2 = 2;
			target_x2 = 0;
			target_y2 = 15;
		}
		else if (x == 0 && y == 15) {
			target_id1 = 1;
			target_x1 = 15;
			target_y1 = 0;
			target_id2 = 2;
			target_x2 = 15;
			target_y2 = 15;
		}
		else if (x == 15 && y == 0) {
			target_id1 = 0;
			target_x1 = 15;
			target_y1 = 15;
			target_id2 = 3;
			target_x2 = 0;
			target_y2 = 15;
		}
		else if (x == 15 && y == 15) {
			target_id1 = 1;
			target_x1 = 15;
			target_y1 = 15;
			target_id2 = 3;
			target_x2 = 15;
			target_y2 = 15;
		}
	}
}
void autoPosMap(int face_id, int x, int y, int &target_id, int &target_x, int &target_y) {
	if (face_id == 0 || face_id == 1) {
		if (x == 0) {
			target_id = 4;
			posMap(face_id, 4, x, y, target_x, target_y);
		}
		else if (x == 15) {
			target_id = 5;
			posMap(face_id, 5, x, y, target_x, target_y);
		}

		if (y == 0) {
			target_id = 2;
			posMap(face_id, 2, x, y, target_x, target_y);
		}
		else if (y == 15) {
			target_id = 3;
			posMap(face_id, 3, x, y, target_x, target_y);
		}
	}
	else if (face_id == 2 || face_id == 3) {
		if (x == 0) {
			target_id = 0;
			posMap(face_id, 0, x, y, target_x, target_y);
		}
		else if (x == 15) {
			target_id = 1;
			posMap(face_id, 1, x, y, target_x, target_y);
		}

		if (y == 0) {
			target_id = 4;
			posMap(face_id, 4, x, y, target_x, target_y);
		}
		else if (y == 15) {
			target_id = 5;
			posMap(face_id, 5, x, y, target_x, target_y);
		}
	}
	else {
		if (x == 0) {
			target_id = 2;
			posMap(face_id, 2, x, y, target_x, target_y);
		}
		else if (x == 15) {
			target_id = 3;
			posMap(face_id, 3, x, y, target_x, target_y);
		}

		if (y == 0) {
			target_id = 0;
			posMap(face_id, 0, x, y, target_x, target_y);
		}
		else if (y == 15) {
			target_id = 1;
			posMap(face_id, 1, x, y, target_x, target_y);
		}
	}
}
void posMap(int face_id, int target_face_id, int x, int y, int &target_x, int &target_y) {
	if (face_id == 0) {
		if (target_face_id == 2) {
			target_x = 0;
			target_y = x;
		}
		else if (target_face_id == 3) {
			target_x = 0;
			target_y = x;
		}
		else if (target_face_id == 4) {
			target_x = y;
			target_y = 0;
		}
		else if (target_face_id == 5) {
			target_x = y;
			target_y = 0;
		}
	}
	else if (face_id == 1) {
		if (target_face_id == 2) {
			target_x = 15;
			target_y = x;
		}
		else if (target_face_id == 3) {
			target_x = 15;
			target_y = x;
		}
		else if (target_face_id == 4) {
			target_x = y;
			target_y = 15;
		}
		else if (target_face_id == 5) {
			target_x = y;
			target_y = 15;
		}
	}
	else if (face_id == 2) {
		if (target_face_id == 0) {
			target_x = y;
			target_y = 0;
		}
		else if (target_face_id == 1) {
			target_x = y;
			target_y = 0;
		}
		else if (target_face_id == 4) {
			target_x = 0;
			target_y = x;
		}
		else if (target_face_id == 5) {
			target_x = 0;
			target_y = x;
		}
	}
	else if (face_id == 3) {
		if (target_face_id == 0) {
			target_x = y;
			target_y = 15;
		}
		else if (target_face_id == 1) {
			target_x = y;
			target_y = 15;
		}
		else if (target_face_id == 4) {
			target_x = 15;
			target_y = x;
		}
		else if (target_face_id == 5) {
			target_x = 15;
			target_y = x;
		}
	}
	else if (face_id == 4) {
		if (target_face_id == 0) {
			target_x = 0;
			target_y = x;
		}
		else if (target_face_id == 1) {
			target_x = 0;
			target_y = x;
		}
		else if (target_face_id == 2) {
			target_x = y;
			target_y = 0;
		}
		else if (target_face_id == 3) {
			target_x = y;
			target_y = 0;
		}
	}
	else if (face_id == 5) {
		if (target_face_id == 0) {
			target_x = 15;
			target_y = x;
		}
		else if (target_face_id == 1) {
			target_x = 15;
			target_y = x;
		}
		else if (target_face_id == 2) {
			target_x = y;
			target_y = 15;
		}
		else if (target_face_id == 3) {
			target_x = y;
			target_y = 15;
		}
	}
}

void copyPixelInfo(Ohm_slice::Pixel *to, Ohm_slice::Pixel *from) {
	to->geom = from->geom;
	to->order = from->order;
	to->region = from->region;
}

void setPatchNonMetalInfo(Ohm_slice::Patch *patch) {
	std::vector<std::vector<bool>> vis(16, std::vector<bool>(16, false));
	std::queue<std::pair<int, int>> q;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			printf("loop[ %d %d]\n", i, j);
			if (patch->pixel_list[i][j]->order < 0 && !vis[i][j]) {
				int non_metal_pixel = 0;
				q.push(std::pair<int, int>(i, j));
				int fill_x = 0, fill_y = 0;
				vis[i][j] = true;
				while (!q.empty()) {
					std::pair<int, int> pos = q.front();
					printf("%d %d\n", pos.first, pos.second);
					q.pop();
					non_metal_pixel++;
					if (pos.first > 0 && patch->pixel_list[pos.first - 1][pos.second]->order < 0 && !vis[pos.first - 1][pos.second]) {
						vis[pos.first - 1][pos.second] = true;
						q.push(std::pair<int, int>(pos.first - 1, pos.second));
					}
					if (pos.first < 15 && patch->pixel_list[pos.first + 1][pos.second]->order < 0 && !vis[pos.first + 1][pos.second]) {
						vis[pos.first + 1][pos.second] = true;
						q.push(std::pair<int, int>(pos.first + 1, pos.second));
					}
					if (pos.second > 0 && patch->pixel_list[pos.first][pos.second - 1]->order < 0 && !vis[pos.first][pos.second - 1]) {
						vis[pos.first][pos.second - 1] = true;
						q.push(std::pair<int, int>(pos.first, pos.second - 1));
					}
					if (pos.second < 15 && patch->pixel_list[pos.first][pos.second + 1]->order < 0 && !vis[pos.first][pos.second + 1]) {
						vis[pos.first][pos.second + 1] = true;
						q.push(std::pair<int, int>(pos.first, pos.second + 1));
					}
					if (pos.first > 0 && patch->pixel_list[pos.first - 1][pos.second]->order > 0) {
						fill_x = pos.first - 1;
						fill_y = pos.second;
					}
					if (pos.first < 15 && patch->pixel_list[pos.first + 1][pos.second]->order > 0) {
						fill_x = pos.first + 1;
						fill_y = pos.second;
					}
					if (pos.second > 0 && patch->pixel_list[pos.first][pos.second - 1]->order > 0) {
						fill_x = pos.first;
						fill_y = pos.second - 1;
					}
					if (pos.second < 15 && patch->pixel_list[pos.first][pos.second + 1]->order > 0) {
						fill_x = pos.first;
						fill_y = pos.second + 1;
					}
					printf("q.size: %d\n", q.size());
				}
				patch->non_metal_info_list.push_back(Ohm_slice::NonMetalInfo(i, j, non_metal_pixel, fill_x, fill_y));
				patch->non_metal_region_num++;
			}
		}
	}
	std::sort(patch->non_metal_info_list.begin(), patch->non_metal_info_list.end(), [](Ohm_slice::NonMetalInfo &a, Ohm_slice::NonMetalInfo &b) {
		return a.num < b.num;
	});
}
void countPatchInnerLoopAndMetalArea(Ohm_slice::Patch *patch) {
	int loop_cnt = 0;
	std::vector<std::vector<bool>> vis(16, std::vector<bool>(16, false));
	std::queue<std::pair<int, int>> q;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			if (patch->pixel_list[i][j]->order > 0 && !vis[i][j]) {
				bool is_inner = true;
				q.push(std::pair<int, int>(i, j));
				while (!q.empty()) {
					std::pair<int, int> pos = q.front();
					q.pop();
					vis[pos.first][pos.second] = true;
					if (pos.first == 0 || pos.first == 15 || pos.second == 0 || pos.second == 15) {
						is_inner = false;
					}
					if (pos.first > 0 && patch->pixel_list[pos.first - 1][pos.second]->order > 0 && !vis[pos.first - 1][pos.second]) {
						q.push(std::pair<int, int>(pos.first - 1, pos.second));
					}
					if (pos.first < 15 && patch->pixel_list[pos.first + 1][pos.second]->order > 0 && !vis[pos.first + 1][pos.second]) {
						q.push(std::pair<int, int>(pos.first + 1, pos.second));
					}
					if (pos.second > 0 && patch->pixel_list[pos.first][pos.second - 1]->order > 0 && !vis[pos.first][pos.second - 1]) {
						q.push(std::pair<int, int>(pos.first, pos.second - 1));
					}
					if (pos.second < 15 && patch->pixel_list[pos.first][pos.second + 1]->order > 0 && !vis[pos.first][pos.second + 1]) {
						q.push(std::pair<int, int>(pos.first, pos.second + 1));
					}
				}
				if (is_inner) {
					loop_cnt++;
				}
				patch->metal_region_num++;
			}
		}
	}
	patch->patch_type = loop_cnt;
}

void countUnflodMetalArea(Ohm_slice::Cell *cell) {
	std::vector<std::vector<std::vector<bool>>> vis(6, std::vector<std::vector<bool>>(16, std::vector<bool>(16, false)));
	struct pos {
		int patch;
		int x;
		int y;
		pos() {}
		pos(int patch_, int x_, int y_) : patch(patch_), x(x_), y(y_) {}
	};
	std::queue<pos> q;
	int unflodeMetalAreaNum = 0;
	for (int patch_id = 0; patch_id < 6; patch_id++) {
		for (int x = 0; x < 16; x++) {
			for (int y = 0; y < 16; y++) {
				if (cell->patch_list[patch_id]->pixel_list[x][y]->order > 0 && !vis[patch_id][x][y]) {
					q.push(pos(patch_id, x, y));
					vis[patch_id][x][y] = true;
					unflodeMetalAreaNum++;
					while (!q.empty()) {
						pos temp = q.front();
						q.pop();
						if (temp.x > 0) {
							if (!vis[temp.patch][temp.x - 1][temp.y]) {
								if (cell->patch_list[temp.patch]->pixel_list[temp.x - 1][temp.y]->order > 0) {
									vis[temp.patch][temp.x - 1][temp.y] = true;
									q.push(pos(temp.patch, temp.x - 1, temp.y));
								}
								
							}
						}
						else{
							pos new_pos;
							autoPosMap(temp.patch, temp.x, temp.y, new_pos.patch, new_pos.x, new_pos.y);
							if (!vis[new_pos.patch][new_pos.x][new_pos.y] && cell->patch_list[new_pos.patch]->pixel_list[new_pos.x][new_pos.y]->order > 0) {
								vis[new_pos.patch][new_pos.x][new_pos.y] = true;
								q.push(new_pos);
							}
						}
						if (temp.x < 15) {
							if (!vis[temp.patch][temp.x + 1][temp.y]) {
								if (cell->patch_list[temp.patch]->pixel_list[temp.x + 1][temp.y]->order > 0) {
									vis[temp.patch][temp.x + 1][temp.y] = true;
									q.push(pos(temp.patch, temp.x + 1, temp.y));
								}

							}
						}
						else {
							pos new_pos;
							autoPosMap(temp.patch, temp.x, temp.y, new_pos.patch, new_pos.x, new_pos.y);
							if (!vis[new_pos.patch][new_pos.x][new_pos.y] && cell->patch_list[new_pos.patch]->pixel_list[new_pos.x][new_pos.y]->order > 0) {
								vis[new_pos.patch][new_pos.x][new_pos.y] = true;
								q.push(new_pos);
							}
						}
						if (temp.y > 0) {
							if (!vis[temp.patch][temp.x][temp.y - 1]) {
								if (cell->patch_list[temp.patch]->pixel_list[temp.x][temp.y - 1]->order > 0) {
									vis[temp.patch][temp.x][temp.y - 1] = true;
									q.push(pos(temp.patch, temp.x, temp.y - 1));
								}

							}
						}
						else {
							pos new_pos;
							autoPosMap(temp.patch, temp.x, temp.y, new_pos.patch, new_pos.x, new_pos.y);
							if (!vis[new_pos.patch][new_pos.x][new_pos.y] && cell->patch_list[new_pos.patch]->pixel_list[new_pos.x][new_pos.y]->order > 0) {
								vis[new_pos.patch][new_pos.x][new_pos.y] = true;
								q.push(new_pos);
							}
						}
						if (temp.y < 15) {
							if (!vis[temp.patch][temp.x][temp.y + 1]) {
								if (cell->patch_list[temp.patch]->pixel_list[temp.x][temp.y + 1]->order > 0) {
									vis[temp.patch][temp.x][temp.y + 1] = true;
									q.push(pos(temp.patch, temp.x, temp.y + 1));
								}

							}
						}
						else {
							pos new_pos;
							autoPosMap(temp.patch, temp.x, temp.y, new_pos.patch, new_pos.x, new_pos.y);
							if (!vis[new_pos.patch][new_pos.x][new_pos.y] && cell->patch_list[new_pos.patch]->pixel_list[new_pos.x][new_pos.y]->order > 0) {
								vis[new_pos.patch][new_pos.x][new_pos.y] = true;
								q.push(new_pos);
							}
						}
						if ((temp.x == 0 && temp.y == 0) || (temp.x == 0 && temp.y == 15) || (temp.x == 15 && temp.y == 0) || (temp.x == 15 && temp.y == 15)) {
							pos new_pos1, new_pos2;
							autoPosMap2(temp.patch, temp.x, temp.y, new_pos1.patch, new_pos1.x, new_pos1.y, new_pos2.patch, new_pos2.x, new_pos2.y);
							if (!vis[new_pos1.patch][new_pos1.x][new_pos1.y] && cell->patch_list[new_pos1.patch]->pixel_list[new_pos1.x][new_pos1.y]->order > 0) {
								vis[new_pos1.patch][new_pos1.x][new_pos1.y] = true;
								q.push(new_pos1);
							}
							if (!vis[new_pos2.patch][new_pos2.x][new_pos2.y] && cell->patch_list[new_pos2.patch]->pixel_list[new_pos2.x][new_pos2.y]->order > 0) {
								vis[new_pos2.patch][new_pos2.x][new_pos2.y] = true;
								q.push(new_pos2);
							}
						}
					}
				}

			}
		}
	}
	cell->unflodMetalNum = unflodeMetalAreaNum;
	return;
}

void markFullConnect(Ohm_slice::Cell *cell) {
	double leftDownX = cell->leftDown[0];
	double leftDownY = cell->leftDown[1];
	double leftDownZ = cell->leftDown[2];
	double rightUpX = cell->rightUp[0];
	double rightUpY = cell->rightUp[1];
	double rightUpZ = cell->rightUp[2];

	Ohm_slice::NodePosition pos0(rightUpX, leftDownY, leftDownZ);
	Ohm_slice::NodePosition pos1(rightUpX, rightUpY, leftDownZ);
	Ohm_slice::NodePosition pos2(leftDownX, rightUpY, leftDownZ);
	Ohm_slice::NodePosition pos3(leftDownX, leftDownY, leftDownZ);
	Ohm_slice::NodePosition pos4(rightUpX, leftDownY, rightUpZ);
	Ohm_slice::NodePosition pos5(rightUpX, rightUpY, rightUpZ);
	Ohm_slice::NodePosition pos6(leftDownX, rightUpY, rightUpZ);
	Ohm_slice::NodePosition pos7(leftDownX, leftDownY, rightUpZ);

	cell->connect_info_list.push_back(Ohm_slice::ConnectInfo(pos0, pos1));
	cell->connect_info_list.push_back(Ohm_slice::ConnectInfo(pos1, pos2));
	cell->connect_info_list.push_back(Ohm_slice::ConnectInfo(pos2, pos3));
	cell->connect_info_list.push_back(Ohm_slice::ConnectInfo(pos3, pos0));
	cell->connect_info_list.push_back(Ohm_slice::ConnectInfo(pos0, pos4));
	cell->connect_info_list.push_back(Ohm_slice::ConnectInfo(pos1, pos5));
	cell->connect_info_list.push_back(Ohm_slice::ConnectInfo(pos2, pos6));
	cell->connect_info_list.push_back(Ohm_slice::ConnectInfo(pos3, pos7));
	cell->connect_info_list.push_back(Ohm_slice::ConnectInfo(pos4, pos5));
	cell->connect_info_list.push_back(Ohm_slice::ConnectInfo(pos5, pos6));
	cell->connect_info_list.push_back(Ohm_slice::ConnectInfo(pos6, pos7));
	cell->connect_info_list.push_back(Ohm_slice::ConnectInfo(pos7, pos4));
}

void markOneEdge(Ohm_slice::Cell *cell, Ohm_slice::NodePosition &a, Ohm_slice::NodePosition &b) {
	cell->connect_info_list.push_back(Ohm_slice::ConnectInfo(a, b));
}

void mapRealPos(Ohm_slice::Cell *cell, int patch_id, int x, int y, double &real_x, double &real_y, double &real_z) {
	if (x == 15) x++;
	if (y == 15) y++;
	double len_x = (cell->rightUp[0] - cell->leftDown[0]) / 16.0;
	double len_y = (cell->rightUp[1] - cell->leftDown[1]) / 16.0;
	double len_z = (cell->rightUp[2] - cell->leftDown[2]) / 16.0;
	if (patch_id == 0) {
		real_x = cell->leftDown[0];
		real_y = cell->leftDown[1] + y * len_y;
		real_z = cell->leftDown[2] + x * len_z;
	}
	else if (patch_id == 1) {
		real_x = cell->rightUp[0];
		real_y = cell->leftDown[1] + y * len_y;
		real_z = cell->leftDown[2] + x * len_z;
	}
	else if (patch_id == 2) {
		real_y = cell->leftDown[1];
		real_x = cell->leftDown[0] + x * len_x;
		real_z = cell->leftDown[2] + y * len_z;
	}
	else if (patch_id == 3) {
		real_y = cell->rightUp[1];
		real_x = cell->leftDown[0] + x * len_x;
		real_z = cell->leftDown[2] + y * len_z;
	}
	else if (patch_id == 4) {
		real_z = cell->leftDown[2];
		real_x = cell->leftDown[0] + y * len_y;
		real_y = cell->leftDown[1] + x * len_x;
	}
	else if (patch_id == 5) {
		real_z = cell->rightUp[2];
		real_x = cell->leftDown[0] + y * len_y;
		real_y = cell->leftDown[1] + x * len_x;
	}

}

void setCellAttr(Ohm_slice::Cell *cell) {
	/*for (int i = 0; i < 6; i++) {
		get_patch_attribute(cell.patch_list[i]);
	}*/
	for (int i = 0; i < 6; i++) {
		printf("count inner loop and metal\n");
		countPatchInnerLoopAndMetalArea(cell->patch_list[i]);
		printf("count inner non metal\n");
		setPatchNonMetalInfo(cell->patch_list[i]);
		printf("end count\n");
	}
	countUnflodMetalArea(cell);
}