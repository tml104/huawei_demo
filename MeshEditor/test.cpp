#include "stdafx.h"
#include "test.h"
void DebugPatch(Ohm_slice::Patch *patch,int patch_id) {
	printf("                           patch: %d                           \n",patch_id);
	printf("metal_region_num: %d non_metal_region_num: %d inner_loop_num: %d\n", patch->metal_region_num, patch->non_metal_region_num, patch->patch_type);
	
	for (int i = 0; i < 16; i++) {
		printf("\n");
		for (int j = 0; j < 16; j++) {
			DebugPixelInfo(*patch->pixel_list[i][j]);
		}
	}
	printf("\n");
}
void DebugCell(Ohm_slice::Cell *cell) {
	printf("\n\n-----------------------cell-----------------------\n");
	printf("index: %d %d %d\n", cell->xPos, cell->yPos, cell->zPos);
	printf("position:[%lf %lf %lf] [%lf %lf %lf]\n", cell->leftDown[0], cell->leftDown[1], cell->leftDown[2], cell->rightUp[0], cell->rightUp[1], cell->rightUp[2]);
	printf("num of unflod metal: %d\n", cell->unflodMetalNum);
	printf("connect edge num: %d\n", cell->connect_info_list.size());
	for (int i = 0; i < 6; i++) {
		DebugPatch(cell->patch_list[i], i);
	}
	printf("======================================================\n");
	for (int i = 0; i < cell->connect_info_list.size(); i++) {
		DebugConnectInfo(cell->connect_info_list[i]);
	}

	printf("======================================================\n");
	
}
void DebugConnectInfo(Ohm_slice::ConnectInfo &info) {
	printf("[%lf %lf %lf]<---->[%lf %lf %lf]\n", info.x1, info.y1, info.z1, info.x2, info.y2, info.z2);
}
void DebugPixelInfo(Ohm_slice::Pixel &pixel) {
	printf("%05d ", pixel.order);
}
Ohm_slice::Patch *getNewPatch() {
	//printf("create patch\n");
	Ohm_slice::Patch *patch = new Ohm_slice::Patch();
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			patch->pixel_list[i][j] = new Ohm_slice::Pixel();
		}
	}
	return patch;
}
Ohm_slice::Cell *getNewCell() {
	//printf("create cell\n");
	Ohm_slice::Cell *cell = new Ohm_slice::Cell();
	cell->leftDown[0] = 0;
	cell->leftDown[1] = 0;
	cell->leftDown[2] = 0;
	cell->rightUp[0] = 32;
	cell->rightUp[1] = 32;
	cell->rightUp[2] = 32;
	cell->patch_list[0] = getNewPatch();
	cell->patch_list[1] = getNewPatch();
	cell->patch_list[2] = getNewPatch();
	cell->patch_list[3] = getNewPatch();
	cell->patch_list[4] = getNewPatch();
	cell->patch_list[5] = getNewPatch();
	return cell;
}
void setTestPatchOrder(Ohm_slice::Patch *patch,vector<vector<int>> &order) {
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			patch->pixel_list[i][j]->order = order[i][j];
		}
	}
}
void setTestPatchGeom(Ohm_slice::Patch *patch, vector<vector<int>> &geom) {
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			patch->pixel_list[i][j]->geom = geom[i][j];
		}
	}
}
void testOhmCellAttr() {
	Ohm_slice::Cell *cell = getNewCell();
	std::vector<std::vector<int>> order0(16,std::vector<int>(16,-1));
	setTestPatchOrder(cell->patch_list[0], order0);
	setTestPatchOrder(cell->patch_list[1], order0);
	setTestPatchOrder(cell->patch_list[2], order0);
	setTestPatchOrder(cell->patch_list[3], order0);
	setTestPatchOrder(cell->patch_list[4], order0);
	setTestPatchOrder(cell->patch_list[5], order0);
	setCellAttr(cell);
	DebugCell(cell);

	Ohm_slice::Cell *cell2 = getNewCell();
	std::vector<std::vector<int>> order1(16, std::vector<int>(16, -1));
	order1[0][7] = 1;
	std::vector<std::vector<int>> order2(16, std::vector<int>(16, -1));
	order2[7][0] = 1;
	setTestPatchOrder(cell2->patch_list[0], order1);
	setTestPatchOrder(cell2->patch_list[1], order0);
	setTestPatchOrder(cell2->patch_list[2], order0);
	setTestPatchOrder(cell2->patch_list[3], order0);
	setTestPatchOrder(cell2->patch_list[4], order2);
	setTestPatchOrder(cell2->patch_list[5], order0);
	setCellAttr(cell2);
	DebugCell(cell2);
	

	Ohm_slice::Cell *cell3 = getNewCell();
	std::vector<std::vector<int>> order3(16, std::vector<int>(16, -1));
	order3[0][0] = 1;
	setTestPatchOrder(cell3->patch_list[0], order3);
	setTestPatchOrder(cell3->patch_list[1], order3);
	setTestPatchOrder(cell3->patch_list[2], order3);
	setTestPatchOrder(cell3->patch_list[3], order3);
	setTestPatchOrder(cell3->patch_list[4], order3);
	setTestPatchOrder(cell3->patch_list[5], order3);
	setCellAttr(cell3);
	DebugCell(cell3);
}
void testOhm2InnerLoop() {
	Ohm_slice::Cell *cell = getNewCell();
	std::vector<std::vector<int>> order0(16, std::vector<int>(16, -1));
	std::vector<std::vector<int>> order1(16, std::vector<int>(16, -1));
	for (int i = 3; i < 5; i++) {
		for (int j = 3; j < 5; j++) {
			order1[i][j] = 1;
			order1[i + 5][j + 5] = 1;
		}
	}
	setTestPatchOrder(cell->patch_list[0], order1);
	setTestPatchOrder(cell->patch_list[1], order0);
	setTestPatchOrder(cell->patch_list[2], order0);
	setTestPatchOrder(cell->patch_list[3], order0);
	setTestPatchOrder(cell->patch_list[4], order0);
	setTestPatchOrder(cell->patch_list[5], order0);



	setCellAttr(cell);
	handlePartialConnect(cell);
	DebugCell(cell);
}

Ohm_slice::Cell* testUnflodConnect() {
	Ohm_slice::Cell *cell = getNewCell();
	std::vector<std::vector<int>> order0(16, std::vector<int>(16, -1));

	std::vector<std::vector<int>> order1(16, std::vector<int>(16, -1));
	order1[0][7] = 1;
	std::vector<std::vector<int>> order2(16, std::vector<int>(16, -1));
	order2[7][0] = 1;

	std::vector<std::vector<int>> order3(16, std::vector<int>(16, -1));
	order3[15][7] = 1;
	std::vector<std::vector<int>> order4(16, std::vector<int>(16, -1));
	order4[7][15] = 1;

	setTestPatchOrder(cell->patch_list[0], order1);
	setTestPatchOrder(cell->patch_list[1], order3);
	setTestPatchOrder(cell->patch_list[2], order0);
	setTestPatchOrder(cell->patch_list[3], order0);
	setTestPatchOrder(cell->patch_list[4], order2);
	setTestPatchOrder(cell->patch_list[5], order4);



	setCellAttr(cell);
	handlePartialConnect(cell);
	//DebugCell(cell);
	return cell;
}
Ohm_slice::Cell * testOhm1InnerLoop() {
	Ohm_slice::Cell *cell = getNewCell();
	std::vector<std::vector<int>> order0(16, std::vector<int>(16, -1));
	std::vector<std::vector<int>> order1(16, std::vector<int>(16, -1));
	for (int i = 3; i < 6; i++) {
		for (int j = 3; j < 6; j++) {
			order1[i][j] = 1;
		}
	}
	setTestPatchOrder(cell->patch_list[0], order1);
	setTestPatchOrder(cell->patch_list[1], order0);
	setTestPatchOrder(cell->patch_list[2], order0);
	setTestPatchOrder(cell->patch_list[3], order0);
	setTestPatchOrder(cell->patch_list[4], order0);
	setTestPatchOrder(cell->patch_list[5], order0);
	setCellAttr(cell);
	handlePartialConnect(cell);
	geometryFillWithSide(cell);
	DebugCell(cell);
	std::vector<Ohm_slice::Cell *> cell_list;
	cell_list.push_back(cell);
	std::vector<EDGE*> edge_list = transforToEdge(cell_list);
	saveToSat(edge_list, "E:/test_edge.sat");
	return cell;
	
}
void testFill() {
	Ohm_slice::Cell *cell = getNewCell();
	std::vector<std::vector<int>> order0(16, std::vector<int>(16, -1));
	std::vector<std::vector<int>> order1(16, std::vector<int>(16, -1));
	std::vector<std::vector<int>> geom0(16, std::vector<int>(16, 0));
	std::vector<std::vector<int>> geom1(16, std::vector<int>(16, 0));
	for (int i = 3; i < 6; i++) {
		for (int j = 0; j < 16; j++) {
			order1[i][j] = 1;
			geom1[i][j] = 100;
		}
	}
	for (int i = 2; i < 4; i++) {
		for (int j = 0; j < 16; j++) {
			order1[j][i] = 1;
			geom1[j][i] = 100;
		}
	}
	setTestPatchOrder(cell->patch_list[0], order1);
	setTestPatchOrder(cell->patch_list[1], order0);
	setTestPatchOrder(cell->patch_list[2], order0);
	setTestPatchOrder(cell->patch_list[3], order0);
	setTestPatchOrder(cell->patch_list[4], order0);
	setTestPatchOrder(cell->patch_list[5], order0);
	setTestPatchGeom(cell->patch_list[0], geom1);
	setTestPatchGeom(cell->patch_list[1], geom0);
	setTestPatchGeom(cell->patch_list[2], geom0);
	setTestPatchGeom(cell->patch_list[3], geom0);
	setTestPatchGeom(cell->patch_list[4], geom0);
	setTestPatchGeom(cell->patch_list[5], geom0);

	setCellAttr(cell);
	handlePartialConnect(cell);
	geometryFillWithSide(cell);
	DebugCell(cell);
}
void testMuilThread() {
	vector<Ohm_slice::Cell *> cell_list(100);
	for (int i = 0; i < 100; i++) {
		cell_list[i] = getNewCell();
		std::vector<std::vector<int>> order0(16, std::vector<int>(16, -1));
		for (int i = 3; i < 5; i++) {
			for (int j = 3; j < 5; j++) {
				order0[i][j] = 1;
				order0[i + 5][j + 5] = 1;
			}
		}
		setTestPatchOrder(cell_list[i]->patch_list[0], order0);
		setTestPatchOrder(cell_list[i]->patch_list[1], order0);
		setTestPatchOrder(cell_list[i]->patch_list[2], order0);
		setTestPatchOrder(cell_list[i]->patch_list[3], order0);
		setTestPatchOrder(cell_list[i]->patch_list[4], order0);
		setTestPatchOrder(cell_list[i]->patch_list[5], order0);
	}
	printf("order %d\n", cell_list[0]->patch_list[0]->pixel_list[0][0]->order);
	muilHandleConnectAndFill(5, cell_list);
	for (int i = 0; i < 100; i++) {
		DebugCell(cell_list[i]);
	}

}

