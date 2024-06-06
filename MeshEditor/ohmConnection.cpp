#include "stdafx.h"
#include "ohmConnection.h"
BODY* getBodyInsideCell(BODY* body,BODY* cellBox){
	BODY *b1=NULL,*b2=NULL;
	api_copy_body(body,b1);
	api_copy_body(cellBox,b2);
	api_boolean(b2,b1,INTERSECTION);
	return b1;
}
double getAxialLength(AxialDir dir,BODY* body,Ohm_slice::Cell* cell){
	BODY* cellBox = getCellBox(cell);
	BODY* bic = getBodyInsideCell(body,cellBox);
	SPAposition st,ed;
	api_get_entity_box(bic,st,ed);
	double ratio = 0;
	switch (dir){
	case xDir:
		ratio = (ed.x()-st.x())/(cell->rightUp[0]-cell->leftDown[0]);
		break;
	case yDir:
		ratio = (ed.y()-st.y())/(cell->rightUp[1]-cell->leftDown[1]);
		break;
	case zDir:
		ratio = (ed.z()-st.z())/(cell->rightUp[2]-cell->leftDown[2]);
		break;
	}
	return ratio;
}
double getMuilAxialLength(AxialDir dir,ENTITY_LIST bodies,Ohm_slice::Cell* cell){
	double ratio = 0;
	for(int i = 0;i < bodies.count();i++){
		BODY* body = (BODY*)bodies[i];
		ratio += getAxialLength(dir,body,cell);
	}
	return ratio;
}
BodyPos locateAxialPos(double st,double ed,double cellst,double celled){
	double mid = (celled + cellst)/2;
	BodyPos bodyPos;
	if(st <= mid && ed >= mid){
		bodyPos = MID;
	}else if(st > mid){
		bodyPos = ED;
	}else if(st < mid){
		bodyPos = ST;
	}
	return bodyPos;
}

BodyPos locateBodyPos(AxialDir dir,BODY* body,Ohm_slice::Cell* cell){
	BodyPos bodyPos;
	SPAposition st,ed;
	api_get_entity_box(body,st,ed);
	switch (dir){
	case xDir:
		bodyPos = locateAxialPos(st.x(),ed.x(),cell->leftDown[0],cell->rightUp[0]);
		std::cout<<st.x()<<" "<<ed.x()<<" "<<cell->leftDown[0]<<" "<<cell->rightUp[0]<<std::endl;
		break;
	case yDir:
		bodyPos = locateAxialPos(st.y(),ed.y(),cell->leftDown[1],cell->rightUp[1]);
		std::cout<<st.y()<<" "<<ed.y()<<" "<<cell->leftDown[1]<<" "<<cell->rightUp[1]<<std::endl;
		break;
	case zDir:
		bodyPos = locateAxialPos(st.z(),ed.z(),cell->leftDown[2],cell->rightUp[2]);
		std::cout<<st.z()<<" "<<ed.z()<<" "<<cell->leftDown[2]<<" "<<cell->rightUp[2]<<std::endl;
		break;
	}
	return bodyPos;
}
void handleCellConnect(Ohm_slice::Cell& cell){
	std::unordered_set<short> geom_set[6];
	for(int face_idx = 0 ; face_idx < 6 ; face_idx++){
		for(int i=0;i<16;i++){
			for(int j=0;j<16;j++){
				if(cell.patch_list[face_idx]->pixel_list[i][j]->geom <= 0 || cell.patch_list[face_idx]->pixel_list[i][j]->order <= 0) continue;
				geom_set[face_idx].insert(cell.patch_list[face_idx]->pixel_list[i][j]->geom);
			}
		}
	}
	for(int i=0;i<6;i++){
		for(auto it = geom_set[i].begin() ; it != geom_set[i].end() ; it++){
			std::cout<<*it<<" ";
		}
		std::cout<<std::endl;
	}
	for(int face_dir = 0 ; face_dir < 3 ; face_dir++){
		for(auto it = geom_set[face_dir*2].begin();it != geom_set[face_dir*2].end() ; it++){
			if(geom_set[face_dir*2 + 1].find(*it) != geom_set[face_dir*2 + 1].end()){
				handleCase1Conduction(&cell,face_dir);
				break;
			}
		}
	}

}

void handleCellConnect(Ohm_slice::Cell& cell,ENTITY_LIST &bodies,std::vector<std::vector<std::set<std::pair<double,double>>>> &x_conduct,
	std::vector<std::vector<std::set<std::pair<double, double>>>> &y_conduct, std::vector<std::vector<std::set<std::pair<double, double>>>> &z_conduct) {
	std::unordered_set<short> geom_set[6];
	//printf("cell index %d %d %d\n", cell.xPos, cell.yPos, cell.zPos);
	for (int face_idx = 0; face_idx < 6; face_idx++) {
		for (int i = 0; i < 16; i++) {
			for (int j = 0; j < 16; j++) {
				if (cell.patch_list[face_idx]->pixel_list[i][j]->geom <= 0 || cell.patch_list[face_idx]->pixel_list[i][j]->order <= 0) continue;
				geom_set[face_idx].insert(cell.patch_list[face_idx]->pixel_list[i][j]->geom);
			}
		}
	}
	/*for (int i = 0; i < 6; i++) {
		for (auto it = geom_set[i].begin(); it != geom_set[i].end(); it++) {
			std::cout << *it << " ";
		}
		std::cout << std::endl;
	}*/
	for (int face_dir = 0; face_dir < 3; face_dir++) {
		bool flag = false;
		//std::cout <<" --------patch-----------"<< cell.patch_list[face_dir * 2] << " " << cell.patch_list[face_dir * 2 + 1] << std::endl;
		//std::cout << "face_dir:" << face_dir << std::endl;
		//std::cout << &cell << std::endl;
		for (auto it = geom_set[face_dir * 2].begin(); it != geom_set[face_dir * 2].end(); it++) {
			if (geom_set[face_dir * 2 + 1].find(*it) != geom_set[face_dir * 2 + 1].end()) {
				//找到相同零件id
				//std::cout << "metal\n";
				flag = true;
				if (face_dir == 0) {
					//printf("x start\n");
					connectMark(&cell, face_dir, x_conduct);
					//printf("x end\n");
				}
				else if (face_dir == 1) {
					//printf("y start\n");
					connectMark(&cell, face_dir, y_conduct);
					//printf("y end\n");
				}
				else {
					//printf("z start\n");
					connectMark(&cell, face_dir, z_conduct);
					//printf("z end\n");
				}
				break;
			}
		}
		if (!flag) {
			//相对面金属id不相等
			//printf("test !flag\n");
			//1.是否存在内环
			if (cell.patch_list[face_dir * 2]->patch_type == 1 || cell.patch_list[face_dir * 2 + 1]->patch_type == 1) {
				//内环
				
				if (face_dir == 0) {
					innerLoopConnectMark(&cell, face_dir, bodies, x_conduct);
				}
				else if (face_dir == 1) {
					innerLoopConnectMark(&cell, face_dir, bodies, y_conduct);
				}
				else {
					innerLoopConnectMark(&cell, face_dir, bodies, z_conduct);
				}
			}
			else if (geom_set[face_dir * 2].size() != 0 && geom_set[face_dir * 2 + 1].size() != 0) {
				//通过侧面连通
				//printf("test side connect\n");
				ofstream ofs("E:/0504/side_connect.txt", std::ios::app);
				if (face_dir == 0) {
					bool flag1 = false;
					flag1 = sideConnect(cell, face_dir, x_conduct);
					if (flag1) {
						ofs << "E:/0504/" + to_string((long long)cell.zPos) + "/" + to_string((long long)cell.xPos) + "/"
							+ to_string((long long)cell.yPos) + ".json" << std::endl;
						connectMark(&cell, face_dir, x_conduct);
					}
				}
				else if (face_dir == 1) {
					bool flag1 = false;
					flag1 = sideConnect(cell, face_dir, y_conduct);
					if (flag1) {
						ofs << "E:/0504/" + to_string((long long)cell.zPos) + "/" + to_string((long long)cell.xPos) + "/"
							+ to_string((long long)cell.yPos) + ".json" << std::endl;
						connectMark(&cell, face_dir, y_conduct);
					}
				}
				else {
					bool flag1 = false;
					flag1 = sideConnect(cell, face_dir, z_conduct);
					if (flag1) {
						ofs << "E:/0504/" + to_string((long long)cell.zPos) + "/" + to_string((long long)cell.xPos) + "/"
							+ to_string((long long)cell.yPos) + ".json" << std::endl;
						connectMark(&cell, face_dir, z_conduct);
					}
				}
				ofs.close();
				

			}
		}

	}
	

}

void handleFullConnect(AxialDir dir,BodyPos bodyPos1,BodyPos bodyPos2,double ratio,Ohm_slice::Cell* cell){
	if(ratio < MINFULLCONNECT) return;
	switch (dir){
	case xDir://y z
		if(bodyPos1 == MID && bodyPos2 == MID){
			cell->isConduction[1] = 1;
			cell->isConduction[3] = 1;
			cell->isConduction[9] = 1;
			cell->isConduction[11] = 1;
		}else if(bodyPos1 == ST && bodyPos2 == MID){
			cell->isConduction[3] = 1;
			cell->isConduction[11] = 1;
		}else if(bodyPos1 == ED && bodyPos2 == MID){
			cell->isConduction[1] = 1;
			cell->isConduction[9] = 1;
		}else if(bodyPos1 == MID && bodyPos2 == ST){
			cell->isConduction[1] = 1;
			cell->isConduction[3] = 1;
		}else if(bodyPos1 == MID && bodyPos2 == ED){
			cell->isConduction[9] = 1;
			cell->isConduction[11] = 1;
		}
		break;
	case yDir:// z x
		if(bodyPos1 == MID && bodyPos2 == MID){
			cell->isConduction[0] = 1;
			cell->isConduction[2] = 1;
			cell->isConduction[8] = 1;
			cell->isConduction[10] = 1;
		}else if(bodyPos1 == ST && bodyPos2 == MID){
			cell->isConduction[0] = 1;
			cell->isConduction[2] = 1;
		}else if(bodyPos1 == ED && bodyPos2 == MID){
			cell->isConduction[8] = 1;
			cell->isConduction[10] = 1;
		}else if(bodyPos1 == MID && bodyPos2 == ST){
			cell->isConduction[2] = 1;
			cell->isConduction[10] = 1;
		}else if(bodyPos1 == MID && bodyPos2 == ED){
			cell->isConduction[0] = 1;
			cell->isConduction[8] = 1;
		}
		break;
	case zDir:// x y
		if(bodyPos1 == MID && bodyPos2 ==MID){
			cell->isConduction[4] = 1;
			cell->isConduction[5] = 1;
			cell->isConduction[6] = 1;
			cell->isConduction[7] = 1;
		}else if(bodyPos1 == ST && bodyPos2 == MID){
			cell->isConduction[6] = 1;
			cell->isConduction[7] = 1;
		}else if(bodyPos1 == ED && bodyPos2 == MID){
			cell->isConduction[4] = 1;
			cell->isConduction[5] = 1;
		}else if(bodyPos1 == MID && bodyPos2 == ST){
			cell->isConduction[4] = 1;
			cell->isConduction[7] = 1;
		}else if(bodyPos1 == MID && bodyPos2 == ED){
			cell->isConduction[5] = 1;
			cell->isConduction[6] = 1;
		}
		break;
	}
}
void handleAllDirFullConnect(BODY* body,Ohm_slice::Cell* cell){
	double ratiox = getAxialLength(xDir,body,cell);
	double ratioy = getAxialLength(yDir,body,cell);
	double ratioz = getAxialLength(zDir,body,cell);
	BodyPos bpx = locateBodyPos(xDir,body,cell);
	BodyPos bpy = locateBodyPos(yDir,body,cell);
	BodyPos bpz = locateBodyPos(zDir,body,cell);
	std::cout<<bpy<<" "<<bpz<<std::endl;
	handleFullConnect(xDir,bpy,bpz,ratiox,cell);
	handleFullConnect(yDir,bpz,bpx,ratioy,cell);
	handleFullConnect(zDir,bpx,bpy,ratioz,cell);
}

void markEdge(Ohm_slice::Cell* cell){
	
}
ENTITY_LIST getMetal(ENTITY_LIST &body_list,string json_path){
	ifstream ifs;
	ifs.open(json_path);

    Json::Reader reader;
    Json::Value root;
	ENTITY_LIST metal_list;
	std::unordered_set<string> metal;
	metal.insert("Copper");
	if (!ifs.is_open() || !reader.parse(ifs, root, false))
    {
		std::cout<<"get metal error"<<std::endl;
		return metal_list;
    }
	int part_size = root["solids"].size();
	std::cout<<"solids size"<<part_size<<std::endl;
	for(int i = 0 ; i < part_size ; i++){
		string material = root["solids"][i]["material"].asString();
		//std::cout<<"material:"<<material<<std::endl;
		int index = root["solids"][i]["index"].asInt();
		if(metal.find(material) != metal.end()){
			//std::cout<<"add:"<<index<<std::endl;
			metal_list.add(body_list[index]);
		}
	}
	return metal_list;
}

std::unordered_set<Ohm_slice::CellIdx,Ohm_slice::CellIdxHash,Ohm_slice::CellEqual> getMetalCellIdx(ENTITY_LIST& metal_list,std::vector<double>& x_pos,std::vector<double>& y_pos,std::vector<double>& z_pos){
	std::unordered_set<Ohm_slice::CellIdx,Ohm_slice::CellIdxHash,Ohm_slice::CellEqual> st;
	for (int i = 0; i < metal_list.count(); ++i){	
		std::cout<<"test"<<std::endl;
		LUMP* lump = ((BODY*)metal_list[i]) ->lump();
		while(lump){
			SPAposition low, high;
			api_get_entity_box(lump, low, high);
			double x1 = low.x();double y1 = low.y();double z1 = low.z();
			double x2 = high.x();double y2 = high.y();double z2 = high.z();
			int x_left = upper_bound(x_pos.begin(),x_pos.end(),x1) - x_pos.begin() - 1 + 4;
			int y_left = upper_bound(y_pos.begin(),y_pos.end(),y1) - y_pos.begin() - 1 + 4;
			int z_left = upper_bound(z_pos.begin(),z_pos.end(),z1) - z_pos.begin() - 1 + 4;

			int x_right = upper_bound(x_pos.begin(),x_pos.end(),x2) - x_pos.begin()  + 4;
			int y_right = upper_bound(y_pos.begin(),y_pos.end(),y2) - y_pos.begin()  + 4;
			int z_right = upper_bound(z_pos.begin(),z_pos.end(),z2) - z_pos.begin()  + 4;
			std::cout<<x_left<<" "<<x_right<<std::endl;
			std::cout<<y_left<<" "<<y_right<<std::endl;
			std::cout<<z_left<<" "<<z_right<<std::endl;
			for(int x=x_left;x<=x_right;x++){
				for(int y=y_left;y<=y_right;y++){
					for(int z=z_left;z<=z_right;z++){
						st.insert(Ohm_slice::CellIdx(x,y,z));
					}
				}
			}
			lump = lump->next();
		}
		

	}
	return st;
}

void connectMark(Ohm_slice::Cell* cell, int face_dir, std::vector<std::vector<std::set<std::pair<double, double>>>> &conduct) {
	int left = INT_MAX, right = INT_MIN, up = INT_MIN, down = INT_MAX;
	Ohm_slice::Patch* patch1 = cell->patch_list[face_dir * 2];
	Ohm_slice::Patch* patch2 = cell->patch_list[face_dir * 2 + 1];
	//std::cout << "patch addr " << patch1 << " " << patch2 << std::endl;
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			if (patch1->pixel_list[i][j]->order > 0) {
				left = std::min(left, j);
				right = std::max(right, j);
				up = std::max(up, i);
				down = std::min(down, i);
			}
			if (patch2->pixel_list[i][j]->order > 0) {
				left = std::min(left, j);
				right = std::max(right, j);
				up = std::max(up, i);
				down = std::min(down, i);

			}
		}
	}
	//printf("left:%d right:%d down:%d up:%d\n", left, right, down, up);
	//printf("x pos : %d y pos %d z pos %d\n", cell->xPos, cell->yPos, cell->zPos);
	//std::cout << conduct.size() << "*" << conduct[0].size() << std::endl;
	//printf("face dir:%d\n", face_dir);
	//std::cout << "test1\n";
	if (cell->zPos - 4 < 0 || cell->yPos - 4 < 0 || cell->xPos - 4 < 0) {
		return;
	}
	if (face_dir == 0) {

		if (left < MIND && down < MIND) {
			cell->isConduction[3] = true;
			conduct[cell->zPos-4][cell->yPos-4].insert(std::make_pair(cell->leftDown[0], cell->rightUp[0]));
		}
		if (15 - right < MIND && down < MIND) {
			cell->isConduction[1] = true;
			conduct[cell->zPos-4][cell->yPos + 1-4].insert(std::make_pair(cell->leftDown[0], cell->rightUp[0]));
		}
		if (left < MIND && 15 - up < MIND) {
			cell->isConduction[11] = true;
			conduct[cell->zPos + 1-4][cell->yPos-4].insert(std::make_pair(cell->leftDown[0], cell->rightUp[0]));
		}
		if (15 - right < MIND && 15 - up < MIND) {
			cell->isConduction[9] = true;
			conduct[cell->zPos + 1-4][cell->yPos + 1-4].insert(std::make_pair(cell->leftDown[0], cell->rightUp[0]));
		}
	}
	else if (face_dir == 1) {
		if (left < MIND && down < MIND) {
			cell->isConduction[2] = true;
			conduct[cell->xPos-4][cell->zPos-4].insert(std::make_pair(cell->leftDown[1], cell->rightUp[1]));
		}
		if (15 - right < MIND && down < MIND) {
			cell->isConduction[10] = true;
			conduct[cell->xPos-4][cell->zPos + 1-4].insert(std::make_pair(cell->leftDown[1], cell->rightUp[1]));
		}
		if (left < MIND && 15 - up < MIND) {
			cell->isConduction[0] = true;
			conduct[cell->xPos + 1-4][cell->zPos-4].insert(std::make_pair(cell->leftDown[1], cell->rightUp[1]));
		}
		if (15 - right < MIND && 15 - up < MIND) {
			cell->isConduction[8] = true;
			conduct[cell->xPos + 1-4][cell->zPos + 1-4].insert(std::make_pair(cell->leftDown[1], cell->rightUp[1]));
		}
	}
	else {
		if (left < MIND && down < MIND) {
			cell->isConduction[7] = true;
			conduct[cell->yPos-4][cell->xPos-4].insert(std::make_pair(cell->leftDown[2], cell->rightUp[2]));
		}
		if (15 - right < MIND && down < MIND) {
			cell->isConduction[4] = true;
			conduct[cell->yPos-4][cell->xPos + 1-4].insert(std::make_pair(cell->leftDown[2], cell->rightUp[2]));
		}
		if (left < MIND && 15 - up < MIND) {
			cell->isConduction[6] = true;
			conduct[cell->yPos + 1-4][cell->xPos-4].insert(std::make_pair(cell->leftDown[2], cell->rightUp[2]));
		}
		if (15 - right < MIND && 15 - up < MIND) {
			cell->isConduction[5] = true;
			conduct[cell->yPos + 1-4][cell->xPos + 1-4].insert(std::make_pair(cell->leftDown[2], cell->rightUp[2]));
		}
	}
	//std::cout << "connectMark end\n";
	return;
}

void innerLoopConnectMark(Ohm_slice::Cell* cell, int face_dir, ENTITY_LIST &bodies, std::vector<std::vector<std::set<std::pair<double, double>>>> &conduct) {
	//std::cout << "innerLoopConnectMark\n";
	printf("cell index %d %d %d\n", cell->xPos, cell->yPos, cell->zPos);
	BODY* cell_box = getCellBox(cell);
	FACE* face1 = getCellFace(cell_box, face_dir * 2);
	FACE* face2 = getCellFace(cell_box, face_dir * 2 + 1);
	std::vector<BODY*> part_list = intersectPart(cell_box, bodies);


	//std::cout << "part_list\n";
	std::vector<BODY*> metal_in_face1, metal_in_face2;
	for (int i = 0; i < part_list.size(); i++) {
		BODY *b1 = NULL, *b2 = NULL;
		api_copy_body(cell_box, b1);
		api_copy_body(part_list[i], b2);
		api_boolean(b1, b2, INTERSECTION);
		//std::cout << "inner loop test1\n";
		LUMP* lump = b2->lump();
		//std::cout << "inner loop test12\n";
		while (lump != NULL) {
			bool flag1 = false;
			bool flag2 = false;
			//std::cout << "inner loop test22\n";
			if (!flag1) {
				flag1 = isInterferingFace(face1, lump);
				metal_in_face1.push_back((BODY*)lump);
			}
			if (!flag2) {
				flag2 = isInterferingFace(face2, lump);
				metal_in_face2.push_back((BODY*)lump);
			}
			//std::cout << "inner loop test2\n";
			if (flag1&&flag2) {
				api_del_entity(b1);
				api_del_entity(b2);
				api_del_entity(cell_box);
				connectMark(cell, face_dir, conduct);
				return;
			}
			//std::cout << "inner loop test3\n";
		}
		
		//std::cout << "inner loop test222\n";

	}
	api_del_entity(cell_box);
	for (int i = 0; i < part_list.size(); i++) {
		api_del_entity(part_list[i]);
	}
	for (int i = 0; i < metal_in_face1.size(); i++) {
		for (int j = 0; j < metal_in_face2.size(); j++) {
			BODY *b1 = NULL, *b2 = NULL;
			api_copy_body(metal_in_face1[i], b1);
			api_copy_body(metal_in_face2[j], b2);
			bool flag = isInterfering(b1, b2);
			api_del_entity(b1);
			api_del_entity(b2);
			if (flag) {
				for (int i = 0; i < metal_in_face1.size(); i++){
					api_del_entity(metal_in_face1[i]);
				}
				for (int i = 0; i < metal_in_face2.size(); i++) {
					api_del_entity(metal_in_face2[i]);
				}
				connectMark(cell, face_dir, conduct);
				return;
			}

		}
	}
	for (int i = 0; i < metal_in_face1.size(); i++) {
		api_del_entity(metal_in_face1[i]);
	}
	for (int i = 0; i < metal_in_face2.size(); i++) {
		api_del_entity(metal_in_face2[i]);
	}
	//std::cout << "inner loop not connect\n";

}

bool sideConnect(Ohm_slice::Cell &cell, int face_dir, std::vector<std::vector<std::set<std::pair<double, double>>>> &conduct) {
	Ohm_slice::Patch *patch1 = cell.patch_list[face_dir * 2];
	Ohm_slice::Patch *patch2 = cell.patch_list[face_dir * 2 + 1];
	vector<pair<int, int>> metal_edge1, metal_edge2;
	bool flag = false;
	if (face_dir == 0) {
		for (int i = 0; i < 15; i++) {
			if (patch1->pixel_list[i][15]->order > 0) {
				int x, y;
				posMap(0, 3, i, 15, x, y);
				if (cell.patch_list[3]->pixel_list[x][y]->order > 0) {
					metal_edge1.push_back(make_pair<int, int>(x, y));
				}
			}
			if (patch2->pixel_list[i][15]->order > 0) {
				int x, y;
				posMap(1, 3, i, 15, x, y);
				if (cell.patch_list[3]->pixel_list[x][y]->order > 0) {
					metal_edge2.push_back(make_pair<int, int>(x, y));
				}
			}
		}
		flag |= judgeSideConnect(metal_edge1, metal_edge2, &cell, 3, conduct);
		metal_edge1.clear();
		metal_edge2.clear();
		if (flag) return true;
		for (int i = 0; i < 15; i++) {
			if (patch1->pixel_list[i][0]->order > 0) {
				int x, y;
				posMap(0, 2, i, 0, x, y);
				if (cell.patch_list[2]->pixel_list[x][y]->order > 0) {
					metal_edge1.push_back(make_pair<int, int>(x, y));
				}
			}
			if (patch2->pixel_list[i][0]->order > 0) {
				int x, y;
				posMap(1, 2, i, 0, x, y);
				if (cell.patch_list[2]->pixel_list[x][y]->order > 0) {
					metal_edge2.push_back(make_pair<int, int>(x, y));
				}
			}
		}
		flag |= judgeSideConnect(metal_edge1, metal_edge2, &cell, 2, conduct);
		metal_edge1.clear();
		metal_edge2.clear();
		if (flag) return true;
		for (int i = 0; i < 15; i++) {
			if (patch1->pixel_list[0][i]->order > 0) {
				int x, y;
				posMap(0, 4, 0, i, x, y);
				if (cell.patch_list[4]->pixel_list[x][y]->order > 0) {
					metal_edge1.push_back(make_pair<int, int>(x, y));
				}
			}
			if (patch2->pixel_list[0][i]->order > 0) {
				int x, y;
				posMap(1, 4, 0, i, x, y);
				if (cell.patch_list[4]->pixel_list[x][y]->order > 0) {
					metal_edge2.push_back(make_pair<int, int>(x, y));
				}
			}
		}
		flag |= judgeSideConnect(metal_edge1, metal_edge2, &cell, 4, conduct);
		metal_edge1.clear();
		metal_edge2.clear();
		if (flag) return true;
		for (int i = 0; i < 15; i++) {
			if (patch1->pixel_list[15][i]->order > 0) {
				int x, y;
				posMap(0, 5, 15, i, x, y);
				if (cell.patch_list[5]->pixel_list[x][y]->order > 0) {
					metal_edge1.push_back(make_pair<int, int>(x, y));
				}
			}
			if (patch2->pixel_list[15][i]->order > 0) {
				int x, y;
				posMap(1, 5, 15, i, x, y);
				if (cell.patch_list[5]->pixel_list[x][y]->order > 0) {
					metal_edge2.push_back(make_pair<int, int>(x, y));
				}
			}
		}
		flag |= judgeSideConnect(metal_edge1, metal_edge2, &cell, 5, conduct);
		metal_edge1.clear();
		metal_edge2.clear();
		//std::cout << "flag:"<< flag << std::endl;
		if (flag) return true;
	}
	else if (face_dir == 1) {
		for (int i = 0; i < 15; i++) {
			if (patch1->pixel_list[i][15]->order > 0) {
				int x, y;
				posMap(2, 5, i, 15, x, y);
				if (cell.patch_list[5]->pixel_list[x][y]->order > 0) {
					metal_edge1.push_back(make_pair<int, int>(x, y));
				}
			}
			if (patch2->pixel_list[i][15]->order > 0) {
				int x, y;
				posMap(3, 5, i, 15, x, y);
				if (cell.patch_list[5]->pixel_list[x][y]->order > 0) {
					metal_edge2.push_back(make_pair<int, int>(x, y));
				}
			}
		}
		flag |= judgeSideConnect(metal_edge1, metal_edge2, &cell, 5, conduct);
		metal_edge1.clear();
		metal_edge2.clear();
		if (flag) return true;
		for (int i = 0; i < 15; i++) {
			if (patch1->pixel_list[i][0]->order > 0) {
				int x, y;
				posMap(2, 4, i, 0, x, y);
				if (cell.patch_list[4]->pixel_list[x][y]->order > 0) {
					metal_edge1.push_back(make_pair<int, int>(x, y));
				}
			}
			if (patch2->pixel_list[i][0]->order > 0) {
				int x, y;
				posMap(3, 4, i, 0, x, y);
				if (cell.patch_list[4]->pixel_list[x][y]->order > 0) {
					metal_edge2.push_back(make_pair<int, int>(x, y));
				}
			}
		}
		flag |= judgeSideConnect(metal_edge1, metal_edge2, &cell, 4, conduct);
		metal_edge1.clear();
		metal_edge2.clear();
		if (flag) return true;
		for (int i = 0; i < 15; i++) {
			if (patch1->pixel_list[0][i]->order > 0) {
				int x, y;
				posMap(2, 0, 0, i, x, y);
				if (cell.patch_list[0]->pixel_list[x][y]->order > 0) {
					metal_edge1.push_back(make_pair<int, int>(x, y));
				}
			}
			if (patch2->pixel_list[0][i]->order > 0) {
				int x, y;
				posMap(3, 0, 0, i, x, y);
				if (cell.patch_list[0]->pixel_list[x][y]->order > 0) {
					metal_edge2.push_back(make_pair<int, int>(x, y));
				}
			}
		}
		flag |= judgeSideConnect(metal_edge1, metal_edge2, &cell, 0, conduct);
		metal_edge1.clear();
		metal_edge2.clear();
		if (flag) return true;
		for (int i = 0; i < 15; i++) {
			if (patch1->pixel_list[15][i]->order > 0) {
				int x, y;
				posMap(2, 1, 15, i, x, y);
				if (cell.patch_list[1]->pixel_list[x][y]->order > 0) {
					metal_edge1.push_back(make_pair<int, int>(x, y));
				}
			}
			if (patch2->pixel_list[15][i]->order > 0) {
				int x, y;
				posMap(3, 1, 15, i, x, y);
				if (cell.patch_list[1]->pixel_list[x][y]->order > 0) {
					metal_edge2.push_back(make_pair<int, int>(x, y));
				}
			}
		}
		flag |= judgeSideConnect(metal_edge1, metal_edge2, &cell, 1, conduct);
		metal_edge1.clear();
		metal_edge2.clear();
		if (flag) return true;
	}
	else if (face_dir == 2) {
	for (int i = 0; i < 15; i++) {
		if (patch1->pixel_list[i][15]->order > 0) {
			int x, y;
			posMap(4, 1, i, 15, x, y);
			if (cell.patch_list[1]->pixel_list[x][y]->order > 0) {
				metal_edge1.push_back(make_pair<int, int>(x, y));
			}
		}
		if (patch2->pixel_list[i][15]->order > 0) {
			int x, y;
			posMap(5, 1, i, 15, x, y);
			if (cell.patch_list[1]->pixel_list[x][y]->order > 0) {
				metal_edge2.push_back(make_pair<int, int>(x, y));
			}
		}
	}
	flag |= judgeSideConnect(metal_edge1, metal_edge2, &cell, 1, conduct);
	metal_edge1.clear();
	metal_edge2.clear();
	if (flag) return true;
	for (int i = 0; i < 15; i++) {
		if (patch1->pixel_list[i][0]->order > 0) {
			int x, y;
			posMap(4, 0, i, 0, x, y);
			if (cell.patch_list[0]->pixel_list[x][y]->order > 0) {
				metal_edge1.push_back(make_pair<int, int>(x, y));
			}
		}
		if (patch2->pixel_list[i][0]->order > 0) {
			int x, y;
			posMap(5, 0, i, 0, x, y);
			if (cell.patch_list[0]->pixel_list[x][y]->order > 0) {
				metal_edge2.push_back(make_pair<int, int>(x, y));
			}
		}
	}
	flag |= judgeSideConnect(metal_edge1, metal_edge2, &cell, 0, conduct);
	metal_edge1.clear();
	metal_edge2.clear();
	if (flag) return true;
	for (int i = 0; i < 15; i++) {
		if (patch1->pixel_list[0][i]->order > 0) {
			int x, y;
			posMap(4, 2, 0, i, x, y);
			if (cell.patch_list[2]->pixel_list[x][y]->order > 0) {
				metal_edge1.push_back(make_pair<int, int>(x, y));
			}
		}
		if (patch2->pixel_list[0][i]->order > 0) {
			int x, y;
			posMap(5, 2, 0, i, x, y);
			if (cell.patch_list[2]->pixel_list[x][y]->order > 0) {
				metal_edge2.push_back(make_pair<int, int>(x, y));
			}
		}
	}
	flag |= judgeSideConnect(metal_edge1, metal_edge2, &cell, 2, conduct);
	metal_edge1.clear();
	metal_edge2.clear();
	if (flag) return true;
	for (int i = 0; i < 15; i++) {
		if (patch1->pixel_list[15][i]->order > 0) {
			int x, y;
			posMap(4, 3, 15, i, x, y);
			if (cell.patch_list[3]->pixel_list[x][y]->order > 0) {
				metal_edge1.push_back(make_pair<int, int>(x, y));
			}
		}
		if (patch2->pixel_list[15][i]->order > 0) {
			int x, y;
			posMap(5, 3, 15, i, x, y);
			if (cell.patch_list[3]->pixel_list[x][y]->order > 0) {
				metal_edge2.push_back(make_pair<int, int>(x, y));
			}
		}
	}
	flag |= judgeSideConnect(metal_edge1, metal_edge2, &cell, 3, conduct);
	metal_edge1.clear();
	metal_edge2.clear();
	if (flag) return true;
	}
	return false;
}

bool judgeSideConnect(vector<pair<int, int>> &metal_edge1, vector<pair<int, int>> &metal_edge2,Ohm_slice::Cell *cell, int face_id,std::vector<std::vector<std::set<std::pair<double, double>>>> &conduct) {
	//std::cout << "metal_edge size:" << metal_edge1.size() << " " << metal_edge2.size()<<std::endl;
	if (!metal_edge1.empty() && !metal_edge2.empty()) {
		if (cell->patch_list[face_id]->metal_region_num == 1) {
			
			return true;
		}
		else if (cell->patch_list[face_id]->metal_region_num > 1) {

			//bfs success
			vector<vector<pixel_node>> body_list(16, vector<pixel_node>(16));
			for (int i = 0; i < 16; ++i) {
				for (int j = 0; j < 16; ++j) {
					body_list[i][j].order = cell->patch_list[face_id]->pixel_list[i][j]->order;
					body_list[i][j].index = 16 * i + j;
					body_list[i][j].xPos = i;
					body_list[i][j].yPos = j;
					body_list[i][j].bound_tag = 0;
				}
			}
			vector<vector<pixel_node> > block;
			// 是否入过队列
			bool inq[256] = { false };
			// 小正方形的序号即为16*x+y
			auto bfs = [&](int u) {
				vector<pixel_node> v;
				int tag = body_list[u / 16][u % 16].order;
				queue<int> q;
				q.push(u);
				inq[u] = true;
				while (!q.empty()) {
					int u = q.front();
					q.pop();
					int x = u / 16;
					int y = u % 16;
					v.push_back(body_list[x][y]);
					if ((x - 1 >= 0) && (body_list[x - 1][y].order == tag) && (inq[(x - 1) * 16 + y] == false)) {
						q.push((x - 1) * 16 + y);
						inq[(x - 1) * 16 + y] = true;
					}
					if ((x + 1 <= 15) && (body_list[x + 1][y].order == tag) && (inq[(x + 1) * 16 + y] == false)) {
						q.push((x + 1) * 16 + y);
						inq[(x + 1) * 16 + y] = true;
					}
					if ((y - 1 >= 0) && (body_list[x][y - 1].order == tag) && (inq[x * 16 + y - 1] == false)) {
						q.push(x * 16 + y - 1);
						inq[x * 16 + y - 1] = true;
					}
					if ((y + 1 <= 15) && (body_list[x][y + 1].order == tag) && (inq[x * 16 + y + 1] == false)) {
						q.push(x * 16 + y + 1);
						inq[x * 16 + y + 1] = true;
					}
				}
				block.push_back(v);
			};

			for (int i = 0; i < metal_edge1.size(); i++) {
				if (!inq[metal_edge1[i].first * 16 + metal_edge1[i].second]) {
					bfs(metal_edge1[i].first * 16 + metal_edge1[i].second);
				}
			}
			for (int i = 0; i < metal_edge2.size(); i++) {
				if (inq[metal_edge2[i].first * 16 + metal_edge2[i].second]) {
					return true;
				}
			}
		}
	}
	return false;
}



void oneInnerLoopPartialConnect(Ohm_slice::Cell *cell, int face_id) {
	Ohm_slice::Patch *patch = cell->patch_list[face_id];
	
	int left = 15;
	int right = 0;
	int up = 0;
	int down = 15;
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 15; j++) {
			if (patch->pixel_list[i][j]->order > 0) {
				left = std::min(left, i);
				right = std::max(right, i);
				up = std::max(up, j);
				down = std::min(down, j);
			}
		}
	}
	printf("%d %d %d %d\n", left, right, up, down);
	int left_dist = left;
	int right_dist = 15 - right;
	int up_dist = 15 - up;
	int down_dist = down;
	int index_x = (left + right) / 2;
	int index_y = (up + down) / 2;
	double st_x, st_y, st_z;
	double ed_x1, ed_y1, ed_z1;
	double ed_x2, ed_y2, ed_z2;
	if (left_dist <= right_dist && down_dist <= up_dist) {
		printf("%d %d\n", index_x, index_y);
		mapRealPos(cell, face_id, 0, 0, st_x, st_y, st_z);
		mapRealPos(cell, face_id, index_x, 0, ed_x1, ed_y1, ed_z1);
		mapRealPos(cell, face_id, 0, index_y, ed_x2, ed_y2, ed_z2);
		printf("test1\n");
	}
	else if (left_dist > right_dist && down_dist <= up_dist) {
		mapRealPos(cell, face_id, 0, 15, st_x, st_y, st_z);
		mapRealPos(cell, face_id, index_x, 15, ed_x1, ed_y1, ed_z1);
		mapRealPos(cell, face_id, 0, index_y, ed_x2, ed_y2, ed_z2);
		printf("test1\n");
	}
	else if (left_dist <= right_dist && down_dist > up_dist) {
		mapRealPos(cell, face_id, 15, 0, st_x, st_y, st_z);
		mapRealPos(cell, face_id, index_x, 0, ed_x1, ed_y1, ed_z1);
		mapRealPos(cell, face_id, 15, index_y, ed_x2, ed_y2, ed_z2);
		printf("test1\n");
	}
	else {
		mapRealPos(cell, face_id, 15, 15, st_x, st_y, st_z);
		mapRealPos(cell, face_id, index_x, 15, ed_x1, ed_y1, ed_z1);
		mapRealPos(cell, face_id, 15, index_y, ed_x2, ed_y2, ed_z2);
		printf("test1\n");
	}
	markOneEdge(cell, Ohm_slice::NodePosition(st_x, st_y, st_z), Ohm_slice::NodePosition(ed_x1, ed_y1, ed_z1));
	markOneEdge(cell, Ohm_slice::NodePosition(st_x, st_y, st_z), Ohm_slice::NodePosition(ed_x2, ed_y2, ed_z2));
}


void handlePartialConnect(Ohm_slice::Cell *cell) {
	for (int i = 0; i < 6; i++) {
		if (cell->patch_list[i]->patch_type >= 2) {
			// 全连接
			markFullConnect(cell);
			return;
		}
	}

	int oneMetalCnt = 0;
	for (int i = 0; i < 6; i++) {
		if (cell->patch_list[i]->patch_type == 1) {
			oneInnerLoopPartialConnect(cell, i);
		}
		if (cell->patch_list[i]->metal_region_num == 1) {
			oneMetalCnt++;
		}
	}
	
	int unflodMetalCnt = cell->unflodMetalNum;
	if (oneMetalCnt == 4 && unflodMetalCnt == 2) {
		//全连接
		markFullConnect(cell);
		return;
	}
}

vector<EDGE*> transforToEdge(std::vector<Ohm_slice::Cell *> cell_list) {
	vector<EDGE*> edgeList;
	for (int i = 0; i < cell_list.size(); i++) {
		Ohm_slice::Cell *cell = cell_list[i];
		for (int j = 0; j < cell->connect_info_list.size(); j++) {
			SPAposition a(cell->connect_info_list[j].x1, cell->connect_info_list[j].y1, cell->connect_info_list[j].z1);
			SPAposition b(cell->connect_info_list[j].x2, cell->connect_info_list[j].y2, cell->connect_info_list[j].z2);
			EDGE *edge;
			api_curve_line(a, b, edge);
			edgeList.push_back(edge);
		}
	}
	return edgeList;
}

void saveToSat(std::vector<EDGE*> edge_list, string file_name) {
	ENTITY_LIST entities;
	for (int i = 0; i < edge_list.size(); i++) {
		entities.add((ENTITY*)edge_list[i]);
	}
	for (int r = 0; r < edge_list.size(); r++) {
		api_rh_set_entity_rgb(edge_list[r], rgb_color(1.0, 0.0, 0.0)); //标记边为红色
	}
	save_acis_entity_list(entities, file_name.c_str());
}



