#include "StdAfx.h"
#include "GetPatchType.h"
using namespace std;

void identify_if_type_2(Ohm_slice::Cell* cell, int patch_id)
{
	
	auto bfs = [&](int u,int end_p,  Ohm_slice::Patch* patch)->bool{
		bool inq[256] = {false};
		vector<Pixel*> v;
		int tag = patch->pixel_list[u/16][u%16]->order;
		queue<int> q;
		q.push(u);
		inq[u] = true;
		while(!q.empty()){
			int u = q.front();
			q.pop();
			int x = u / 16;
			int y = u % 16;
			v.push_back(patch->pixel_list[x][y]);
			if ((x-1 >= 0) && (patch->pixel_list[x-1][y]->order == tag) && (inq[(x-1)*16+y] == false)){
				q.push((x-1)*16+y);
				inq[(x-1)*16+y] = true;
			}
			if ((x+1 <= 15) && (patch->pixel_list[x+1][y]->order == tag) && (inq[(x+1)*16+y] == false)){
				q.push((x+1)*16+y);
				inq[(x+1)*16+y] = true;
			}
			if ((y-1 >= 0) && (patch->pixel_list[x][y-1]->order == tag) && (inq[x*16+y-1] == false)){
				q.push(x*16+y-1);
				inq[x*16+y-1] = true;
			}
			if ((y+1 <= 15) && (patch->pixel_list[x][y+1]->order == tag) && (inq[x*16+y+1] == false)){
				q.push(x*16+y+1);
				inq[x*16+y+1] = true;
			}
		}
		return inq[end_p];
	};



	bool width_connection = true;
	int first_order = cell->patch_list[patch_id]->pixel_list[0][0]->order;
	for (int i = 1; i < 16; ++i){
		if (cell->patch_list[patch_id]->pixel_list[0][i]->order != first_order){
			width_connection = false;
			break;
		}
	}

	switch(patch_id){
		case 0:
			if(width_connection){
				if(bfs(0,15,cell->patch_list[3])&&bfs(0,15,cell->patch_list[2])){
					//ohm connection
					handleCase2Conduction(cell,patch_id,2);
				}
			}else{
				if(bfs(0,15*16,cell->patch_list[4])&&bfs(0,15*16,cell->patch_list[5])){
					//ohm connection
					handleCase2Conduction(cell,patch_id,4);
				}
			}	
			break;
		case 1:
			if(width_connection){
				if(bfs(0+15*16,15+15*16,cell->patch_list[3])&&bfs(0+15*16,15+15*16,cell->patch_list[2])){
					//ohm connection
					handleCase2Conduction(cell,patch_id,2);
				}
			}else{
				if(bfs(0+15,15*16+15,cell->patch_list[4])&&bfs(0+15,15*16+15,cell->patch_list[5])){
					//ohm connection
					handleCase2Conduction(cell,patch_id,4);
				}
			}	
			break;
		case 2:
			if(width_connection){
				if(bfs(0,15,cell->patch_list[4])&&bfs(0,15,cell->patch_list[5])){
					//ohm connection
					handleCase2Conduction(cell,patch_id,4);
				}
			}else{
				if(bfs(0,15*16,cell->patch_list[0])&&bfs(0,15*16,cell->patch_list[1])){
					//ohm connection
					handleCase2Conduction(cell,patch_id,0);
				}
			}
			break;
		case 3:
			if(width_connection){
				if(bfs(0+15*16,15+15*16,cell->patch_list[4])&&bfs(0+15*16,15+15*16,cell->patch_list[5])){
					//ohm connection
					handleCase2Conduction(cell,patch_id,4);
				}
			}else{
				if(bfs(0+15,15*16+15,cell->patch_list[0])&&bfs(0+15,15*16+15,cell->patch_list[1])){
					//ohm connection
					handleCase2Conduction(cell,patch_id,0);
				}
			}
			break;
		case 4:
			if(width_connection){
				if(bfs(0,15,cell->patch_list[0])&&bfs(0,15,cell->patch_list[1])){
					//ohm connection
					handleCase2Conduction(cell,patch_id,0);

				}
			}else{
				if(bfs(0,15*16,cell->patch_list[2])&&bfs(0,15*16,cell->patch_list[3])){
					//ohm connection
					handleCase2Conduction(cell,patch_id,2);
				}
			}
			break;
		case 5:
			if(width_connection){
				if(bfs(0+15*16,15+15*16,cell->patch_list[0])&&bfs(0+15*16,15+15*16,cell->patch_list[1])){
					//ohm connection
					handleCase2Conduction(cell,patch_id,0);

				}
			}else{
				if(bfs(0+15,15*16+15,cell->patch_list[2])&&bfs(0+15,15*16+15,cell->patch_list[3])){
					//ohm connection
					handleCase2Conduction(cell,patch_id,2);
				}
			}
			break;
	}

}


void handleCase2Conduction(Ohm_slice::Cell* cell,int splitFaceId,int halfCircleId){



	cout<<splitFaceId <<endl;
	switch (splitFaceId){
	case 1:
		if(halfCircleId/2 == 1){
			cell->isConduction[4] = 1;
			cell->isConduction[5] = 1;
		}else{
			cell->isConduction[8] = 1;
			cell->isConduction[0] = 1;
		}
		break;
	case 0:
		if(halfCircleId/2 == 1){
			cell->isConduction[6] = 1;
			cell->isConduction[7] = 1;
		}else{
			cell->isConduction[2] = 1;
			cell->isConduction[10] = 1;
		}
		break;
	case 2:
		if(halfCircleId/2 == 0){
			cell->isConduction[4] = 1;
			cell->isConduction[7] = 1;
		}else{
			cell->isConduction[3] = 1;
			cell->isConduction[11] = 1;
		}
		break;
	case 3:
		if(halfCircleId/2 == 0){
			cell->isConduction[5] = 1;
			cell->isConduction[6] = 1;
		}else{
			cell->isConduction[1] = 1;
			cell->isConduction[9] = 1;
		}
		break;
	case 4:
		if(halfCircleId/2 == 0){
			cell->isConduction[2] = 1;
			cell->isConduction[0] = 1;
		}else{
			cell->isConduction[1] = 1;
			cell->isConduction[3] = 1;
		}
		break;
	case 5:
		if(halfCircleId/2 == 0){
			cell->isConduction[8] = 1;
			cell->isConduction[10] = 1;
		}else{
			cell->isConduction[9] = 1;
			cell->isConduction[11] = 1;
		}
		break;


	}
}


BODY* getCellBox(Ohm_slice::Cell *cell){
	BODY *body=NULL;
	api_solid_block(SPAposition(cell->leftDown[0],cell->leftDown[1],cell->leftDown[2]),SPAposition(cell->rightUp[0],cell->rightUp[1],cell->rightUp[2]),body);
	return body;
}
std::vector<BODY*> intersectPart(BODY* cellBox,ENTITY_LIST &bodies){
	std::vector<BODY*> result;
	for(int i=0;i<bodies.count();i++){
		bool flag = isInterfering(cellBox,(BODY*)bodies[i]);
		if(flag){
			std::cout<<i<<" intersect!\n";
			BODY *bb = NULL;
			api_copy_body((BODY*)bodies[i], bb);
			result.push_back((BODY*)bb);
		}

	}
	std::cout<<"end\n";
	return result;
}
bool isInterfering(BODY* body1,BODY* body2){
	BODY *b1=NULL,*b2=NULL;
	api_copy_body(body1,b1);
	api_copy_body(body2,b2);
	body_clash_result result;
	api_clash_bodies(b1,b2,result,CLASH_EXISTENCE_ONLY,CLASH_IGNORE_WIRES,NULL);
	api_del_entity(b1);
	api_del_entity(b2);
	if(result.clash_type()==CLASH_NONE){
		return false;
	}else return true;
}
bool isInterferingFace(FACE* cellFace,LUMP* lump){
	ENTITY *tempCellFace,*tempLump;
	api_copy_entity(cellFace,tempCellFace);
	api_copy_entity(lump,tempLump);
	FACE *tempFace = lump->shell()->face();
	bool flag = false;
	//printf("isInterfering face test1\n");
	while(tempFace){
		entity_clash_type result;
		api_clash_faces((FACE*)tempCellFace,tempFace,result);
		//printf("isInterfering face test\n");
		if(result!=ENTITY_CLASH_NONE){
			flag = true;
			break;
		}
		tempFace = tempFace->next_face();
	}
	api_del_entity(tempCellFace);
	api_del_entity(tempFace);
	return flag;
}
void handleCase1(Ohm_slice::Cell *cell,BODY* cellBox,std::vector<BODY*> partList,int faceId1,int faceId2){
	FACE* face1 = getCellFace(cellBox,faceId1); 
	FACE* face2 = getCellFace(cellBox,faceId2);
	for(int i = 0;i<partList.size();i++){
		BODY *b1=NULL,*b2=NULL;
		api_copy_body(cellBox,b1);
		api_copy_body(partList[i],b2);
		api_boolean(b1,b2,INTERSECTION);

		LUMP* lump = b2->lump();

		while(lump!=NULL){
			bool flag1 = false;
			bool flag2 = false;
			if(!flag1)flag1 = isInterferingFace(face1,lump);
			if(!flag2)flag2 = isInterferingFace(face2,lump);
			if(flag1&&flag2){
				//std::cout<<"case 1\n";
				handleCase1Conduction(cell,faceId1/2);
				break;
			}
			lump = lump->next();
		}

	}
	return;
}

void handleCase1Conduction(Ohm_slice::Cell* cell,int face_dir){
	int left = INT_MAX,right = INT_MIN,up = INT_MIN,down = INT_MAX;
	Ohm_slice::Patch* patch1 = cell->patch_list[face_dir*2];
	Ohm_slice::Patch* patch2 = cell->patch_list[face_dir*2+1];
	for(int i=0;i<16;i++){
		for(int j=0;j<16;j++){
			if(patch1->pixel_list[i][j]->order > 0){
				left = min(left,j);
				right = max(right,j);
				up = max(up,i);
				down = min(down,i);
			}
			if(patch2->pixel_list[i][j]->order > 0){
				left = min(left,j);
				right = max(right,j);
				up = max(up,i);
				down = min(down,i);

			}
		}
	}
	printf("left:%d right:%d down:%d up:%d\n",left,right,down,up);
	if(face_dir == 0){
		/*cell->isConduction[1] = true;
		cell->isConduction[3] = true;
		cell->isConduction[9] = true;
		cell->isConduction[11] = true;*/
		if(left < MIND && down < MIND){
			cell->isConduction[3] = true;
		}
		if(15 - right < MIND && down < MIND){
			cell->isConduction[1] = true;
		}
		if(left < MIND && 15 - up < MIND){
			cell->isConduction[11] = true;
		}
		if(15 - right < MIND && 15 - up < MIND){
			cell->isConduction[9] = true;
		}
	}else if(face_dir == 1){
		/*cell->isConduction[0] = true;
		cell->isConduction[2] = true;
		cell->isConduction[8] = true;
		cell->isConduction[10] = true;*/
		if(left < MIND && down < MIND){
			cell->isConduction[2] = true;
		}
		if(15 - right < MIND && down < MIND){
			cell->isConduction[10] = true;
		}
		if(left < MIND && 15 - up < MIND){
			cell->isConduction[0] = true;
		}
		if(15 - right < MIND && 15 - up < MIND){
			cell->isConduction[8] = true;
		}
	}else{
		std::cout<<"face 4\n";
		/*cell->isConduction[4] = true;
		cell->isConduction[5] = true;
		cell->isConduction[6] = true;
		cell->isConduction[7] = true;*/
		if(left < MIND && down < MIND){
			cell->isConduction[7] = true;
		}
		if(15 - right < MIND && down < MIND){
			cell->isConduction[4] = true;
		}
		if(left < MIND && 15 - up < MIND){
			cell->isConduction[6] = true;
		}
		if(15 - right < MIND && 15 - up < MIND){
			cell->isConduction[5] = true;
		}
	}
	return;
}



FACE* getCellFace(BODY* cellBox,int faceId){
	FACE* face = cellBox->lump()->shell()->face_list();
	SPAunit_vector direct;
	switch (faceId){
	case 0:
		direct.set_x(-1);
		direct.set_y(0);
		direct.set_z(0);
		break;
	case 1:
		direct.set_x(1);
		direct.set_y(0);
		direct.set_z(0);
		break;
	case 2:
		direct.set_x(0);
		direct.set_y(-1);
		direct.set_z(0);
		break;
	case 3:
		direct.set_x(0);
		direct.set_y(1);
		direct.set_z(0);
		break;
	case 4:
		direct.set_x(0);
		direct.set_y(0);
		direct.set_z(-1);
		break;
	case 5:
		direct.set_x(0);
		direct.set_y(0);
		direct.set_z(1);
		break;
	}

	while(face){
		SPAunit_vector faceDir;
		get_face_normal(face,faceDir);
		if(faceDir==direct){
			break;
		}
		face = face->next();
	}
	return face;

}
std::vector<EDGE*> showEdge(Ohm_slice::Cell* cell){
	double leftDownX = cell->leftDown[0];
	double leftDownY = cell->leftDown[1];
	double leftDownZ = cell->leftDown[2];
	double rightUpX = cell->rightUp[0];
	double rightUpY = cell->rightUp[1];
	double rightUpZ = cell->rightUp[2];
	SPAposition pos0(rightUpX,leftDownY,leftDownZ);
	SPAposition pos1(rightUpX,rightUpY,leftDownZ);
	SPAposition pos2(leftDownX,rightUpY,leftDownZ);
	SPAposition pos3(leftDownX,leftDownY,leftDownZ);

	SPAposition pos4(rightUpX,leftDownY,rightUpZ);
	SPAposition pos5(rightUpX,rightUpY,rightUpZ);
	SPAposition pos6(leftDownX,rightUpY,rightUpZ);
	SPAposition pos7(leftDownX,leftDownY,rightUpZ);


	EDGE *edge[12];
	api_curve_line(pos0,pos1,edge[0]);
	api_curve_line(pos1,pos2,edge[1]);
	api_curve_line(pos2,pos3,edge[2]);
	api_curve_line(pos3,pos0,edge[3]);
	api_curve_line(pos0,pos4,edge[4]);
	api_curve_line(pos1,pos5,edge[5]);
	api_curve_line(pos2,pos6,edge[6]);
	api_curve_line(pos3,pos7,edge[7]);
	api_curve_line(pos4,pos5,edge[8]);
	api_curve_line(pos5,pos6,edge[9]);
	api_curve_line(pos6,pos7,edge[10]);
	api_curve_line(pos7,pos4,edge[11]);
	std::vector<EDGE*> edgeList;
	
	for(int i=0;i<12;i++){
		if(cell->isConduction[i]){
			edgeList.push_back(edge[i]);
		}
	}
	return edgeList;
} 



void get_patch_attribute(Ohm_slice::Patch* patch)
{
	vector<vector<pixel_node>> body_list(16, vector<pixel_node>(16));
	for (int i = 0; i < 16; ++i) {
		for (int j = 0; j < 16; ++j) {
			body_list[i][j].order = patch->pixel_list[i][j]->order;
			body_list[i][j].index = 16 * i + j;
			body_list[i][j].xPos = i;
			body_list[i][j].yPos = j;
			body_list[i][j].bound_tag = 0;

			patch->metal_region_num = -1;
			patch->non_metal_region_num = -1;
			patch->patch_type = -1;
		}
	}
	bfs_patch_attri(patch, body_list);
	//cout << "metal num of patch is: " << patch->metal_region_num << endl;
	//cout << "nonmetal num of patch is: " << patch->non_metal_region_num << endl;
	//cout << "metal of patch is circle : " << patch->patch_type << endl;
	//cout << patch->metal_region_num << "   " << patch->non_metal_region_num << "   " << patch->patch_type << endl;
}

void bfs_patch_attri(Ohm_slice::Patch* patch, vector<vector<pixel_node>>& body_list)
{
	// metal_part是后续要判断形状的区域  nonmetal_part是其余区域
	vector<pixel_node> metal_part, nonmetal_part;

	for (int i = 0; i < 16; ++i) {
		for (int j = 0; j < 16; ++j)
			if (body_list[i][j].order > 0) {
				metal_part.push_back(body_list[i][j]);
			}
			else {
				nonmetal_part.push_back(body_list[i][j]);
			}
	}
	//cout << "metal " << metal_part.size() << "   nonmetal  " << nonmetal_part.size() << endl;
	// 一共有几个块
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

	for (int i = 0; i < 256; ++i) {
		if (!inq[i]) {
			bfs(i);
		}
	}

	int num_of_metal_region = 0;
	int num_of_nonmetal_region = 0;

	for (int i = 0; i < block.size(); ++i) {
		if (block[i][0].order > 0) {
			++num_of_metal_region;
		}
		else {
			++num_of_nonmetal_region;
		}
	}
	//标记patch金属和非金属区域的数量
	patch->metal_region_num = num_of_metal_region;
	patch->non_metal_region_num = num_of_nonmetal_region;

	//if (num_of_metal_region != 1 || num_of_nonmetal_region != 1)
	//	return;
	//bool flag = false;
	if (!num_of_metal_region || !num_of_nonmetal_region)
		return;

	if (num_of_metal_region) {
		for (int i = 0; i < 16; ++i) {
			if (patch->pixel_list[0][i] > 0)
				return;
		}
		for (int i = 0; i < 16; ++i) {
			if (patch->pixel_list[15][i] > 0)
				return;
		}
		for (int i = 0; i < 16; ++i) {
			if (patch->pixel_list[i][0] > 0)
				return;
		}
		for (int i = 0; i < 16; ++i) {
			if (patch->pixel_list[i][15] > 0)
				return;
		}
	}
	patch->patch_type = 1;

	//vector<pixel_node> boundary;
	//get_part_boundary_lj(body_list, metal_part, boundary);

	////cout << "bound size " << boundary.size() << endl;
	//map<int, bool> visited;
	//vector<pixel_node> unvisited_boundary_pixel;

	//for (int i = 0; i < boundary.size(); ++i) {
	//	pixel_node & temp = boundary[i];
	//	visited[temp.index] = false;
	//	if (temp.xPos == 0 || temp.xPos == 15) {
	//		unvisited_boundary_pixel.push_back(temp);
	//	}
	//	else if (temp.yPos == 0 || temp.yPos == 15) {
	//		unvisited_boundary_pixel.push_back(temp);
	//	}
	//}

	//if (unvisited_boundary_pixel.size() == 0) {
	//	patch->patch_type = 1;//是圆区域
	//	return;
	//}

	//int num_of_dfs = 0;

	//vector<int> first_node_index;
	//vector<int> last_node_index;
	////int last_node_index = -1;

	//for (int i = 0; i < unvisited_boundary_pixel.size(); ++i) {
	//	if (!visited[unvisited_boundary_pixel[i].index]) {
	//		++num_of_dfs;
	//		//cout << "第 " <<num_of_dfs << " 次dfs"<< endl;
	//		first_node_index.push_back(unvisited_boundary_pixel[i].index);
	//		last_node_index.push_back(dfs(unvisited_boundary_pixel[i], visited, body_list));
	//	}
	//}

	//if (num_of_dfs == 1) {
	//	int x1 = last_node_index[0] / 16;
	//	int y1 = last_node_index[0] % 16;
	//	int x2 = first_node_index[0] / 16;
	//	int y2 = first_node_index[0] % 16;

	//	if (abs(x1 - x2) <= 1 && abs(y1 - y2) <= 1) {
	//		patch->patch_type = 1;//圆类型，一次遍历，而且首尾相连
	//	}
	//}
}


int dfs(pixel_node & temp, map<int, bool>& visited, vector<vector<pixel_node> > & body_list)
{
	queue<pixel_node> q;
	q.push(temp);
	int last_node_index;

	while (!q.empty()) {
		pixel_node temp_node = q.front();
		q.pop();
		int index = temp_node.index;
		last_node_index = index;
		visited[index] = true;

		//cout << "访问node   "<<index << endl;

		int x = temp_node.xPos; int y = temp_node.yPos;

		//bound_tag==1表示该节点为边界节点
		for (int i = 0; i < 8; ++i) {
			if (i == 0) {
				if (y + 1 <= 15 && body_list[x][y + 1].bound_tag == 1 && !visited[index + 1]) {
					q.push(body_list[x][y + 1]);
					break;
				}
			}
			else if (i == 1) {
				if (y + 1 <= 15 && x + 1 <= 15 && body_list[x + 1][y + 1].bound_tag == 1 && !visited[index + 17]) {
					q.push(body_list[x + 1][y + 1]);
					break;
				}
			}
			else if (i == 2) {
				if (x + 1 <= 15 && body_list[x + 1][y].bound_tag == 1 && !visited[index + 16]) {
					q.push(body_list[x + 1][y]);
					break;
				}
			}
			else if (i == 3) {
				if (x + 1 <= 15 && y - 1 >= 0 && body_list[x + 1][y - 1].bound_tag == 1 && !visited[index + 15]) {
					q.push(body_list[x + 1][y - 1]);
					break;
				}
			}
			else if (i == 4) {
				if (y - 1 >= 0 && body_list[x][y - 1].bound_tag == 1 && !visited[index - 1]) {
					q.push(body_list[x][y - 1]);
					break;
				}
			}
			else if (i == 5) {
				if (y - 1 >= 0 && x - 1 >= 0 && body_list[x - 1][y - 1].bound_tag == 1 && !visited[index - 17]) {
					q.push(body_list[x - 1][y - 1]);
					break;
				}
			}
			else if (i == 6) {
				if (x - 1 >= 0 && body_list[x - 1][y].bound_tag == 1 && !visited[index - 16]) {
					q.push(body_list[x - 1][y]);
					break;
				}
			}
			else if (i == 7) {
				if (x - 1 >= 0 && y + 1 <= 15 && body_list[x - 1][y + 1].bound_tag == 1 && !visited[index - 15]) {
					q.push(body_list[x - 1][y + 1]);
					break;
				}

			}
		}
	}
	return last_node_index;
}

void get_part_boundary_lj(vector<vector<pixel_node> >& body_list, vector<pixel_node>& part, vector<pixel_node>& boundary)
{
	//cout << part.size()<<endl;
	//vector<node> boundary;
	for (int i = 0; i < part.size(); ++i) {
		int x = part[i].xPos; int y = part[i].yPos; int tag = part[i].order;
		//cout << x <<" " << y <<endl;
		if ((x - 1 >= 0) && (body_list[x - 1][y].order != tag)) {
			boundary.push_back(part[i]);
			//body_list[x][y].tag = -1;
			continue;
		}
		if ((x + 1 <= 15) && (body_list[x + 1][y].order != tag)) {
			boundary.push_back(part[i]);
			//body_list[x][y].tag = -1;
			continue;
		}
		if ((y - 1 >= 0) && (body_list[x][y - 1].order != tag)) {
			boundary.push_back(part[i]);
			//body_list[x][y].tag = -1;
			continue;
		}
		if ((y + 1 <= 15) && (body_list[x][y + 1].order != tag)) {
			boundary.push_back(part[i]);
			//body_list[x][y].tag = -1;
			continue;
		}
	}

	for (int i = 0; i < boundary.size(); ++i) {
		int x = boundary[i].xPos; int y = boundary[i].yPos;
		body_list[x][y].bound_tag = 1;
		//body_list[x][y].tag = body_list[x][y].tag;
	}

	//cout << "over" << endl;
	//return boundary;
}

