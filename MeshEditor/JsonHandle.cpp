#include "StdAfx.h"
#include "JsonHandle.h"
#include <fstream>
#include <io.h>
bool ReadSatJson(std::string path,std::vector<double> &x_pos,std::vector<double> &y_pos,std::vector<double> &z_pos){
	ifstream ifs;
	ifs.open(path);

	if(!ifs.is_open()){
		return false;
	}

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(ifs, root, false))
    {
        return false;
    }
	int x_size = root["grid"]["xs"].size();
	int y_size = root["grid"]["ys"].size();
	int z_size = root["grid"]["zs"].size();
	
	for(int i=0;i<x_size;i++){
		x_pos.push_back(root["grid"]["xs"][i].asDouble());
	}
	for(int i=0;i<y_size;i++){
		y_pos.push_back(root["grid"]["ys"][i].asDouble());
	}
	for(int i=0;i<z_size;i++){
		z_pos.push_back(root["grid"]["zs"][i].asDouble());
	}
	return true;
}


bool read_one_cell_json(string filename, Ohm_slice::Cell& cell)
{
	// 读取json文件中的数据
	ifstream ifs;
	ifs.open(filename);

	//assert(ifs.is_open());
	if(!ifs.is_open()) 
		return false;

	Json::Reader reader;
	Json::Value root;


	bool show_or_not = false;
	if(!reader.parse(ifs, root)){
		return false;
	}
	
	int x = root["xIndex"].asDouble();
	int y = root["yIndex"].asDouble();
	int z = root["zIndex"].asDouble();
	cell.xPos = x;
	cell.yPos = y;
	cell.zPos = z;
	cell.leftDown[0] = 0;
	cell.leftDown[1] = 0;
	cell.leftDown[2] = 0;
	cell.rightUp[0] = 10;
	cell.rightUp[1] = 10;
	cell.rightUp[2] = 10;
	for (int i = 0; i < 6; ++i){
		string xyz_position;
		if (i == 0){
			xyz_position = "xLeft";
		}
		else if (i == 1){
			xyz_position = "xRight";
		}
		else if (i ==2){
			xyz_position = "yLeft";
		}
		else if (i ==3){
			xyz_position = "yRight";
		}
		else if (i ==4){
			xyz_position = "zLeft";
		}
		else if (i ==5){
			xyz_position = "zRight";
		}
		Ohm_slice::Patch* temp = new Ohm_slice::Patch;
		for (int j = 0; j < 16; j++){
			for (int k = 0; k < 16; ++k){
				temp->pixel_list[j][k] = new Ohm_slice::Pixel(root[xyz_position][j*16+k]["order"].asInt(), root[xyz_position][j*16+k]["region"].asInt(), root[xyz_position][j*16+k]["geom"].asInt());
			}
		}
		cell.patch_list[i] = temp;
	}
	
	ifs.close();
	return true;
}


bool SetCellPos(Ohm_slice::Cell& cell,std::vector<double> &x_pos,std::vector<double> &y_pos,std::vector<double> &z_pos){
	//if(cell.xPos+1 >= x_pos.size() || cell.yPos+1 >= y_pos.size() || cell.zPos + 1 >= z_pos.size() || cell.xPos < 0 || cell.yPos < 0 || cell.zPos < 0){
		//return false;
	//}
	//printf("%d %d %d\n", x_pos.size(), y_pos.size(), z_pos.size());
	cell.leftDown[0] = x_pos[cell.xPos + 4];
	cell.leftDown[1] = y_pos[cell.yPos + 4];
	cell.leftDown[2] = z_pos[cell.zPos + 4];
	cell.rightUp[0] = x_pos[cell.xPos + 4 + 1];
	cell.rightUp[1] = y_pos[cell.yPos + 4 + 1];
	cell.rightUp[2] = z_pos[cell.zPos + 4 + 1];
	return true;
}

void Cell2Json(std::string filePath, std::unordered_set<Ohm_slice::CellIdx, Ohm_slice::CellIdxHash, Ohm_slice::CellEqual>& cellidx)
{
	Json::Value root;
	root["size"] = Json::Value(int(cellidx.size()));
	std::cout<<cellidx.size()<<std::endl;
	int count = 0;
	for (auto it = cellidx.begin(); it != cellidx.end(); ++it) {
		root[to_string((long long)count)].append(it->x);
		root[to_string((long long)count)].append(it->y);
		root[to_string((long long)count)].append(it->z);
		count++;
		/*if (count % 1000000 == 0) {
			ofstream os;
			Json::StyledWriter sw;
			std::string file_ = "D:/0504/";
			std::cout << file_ + to_string((long long)count / 1000000) + "cellInfo.json" << std::endl;
			os.open(file_ + to_string((long long)count / 1000000) + "cellInfo.json", std::ios::out | std::ios::trunc);
			if (!os.is_open()) {
				std::cout << "Error open file" << std::endl;
			}
			os << sw.write(root);
			os.close();
			root.clear();
		}*/
	}
	ofstream os;
	Json::StyledWriter sw;
	os.open(filePath, std::ios::out | std::ios::trunc);
	if (!os.is_open()) {
		std::cout << "Error open file" << std::endl;
	}
	os << sw.write(root);
	os.close();
}

void Json2Cell(std::string filePath, std::unordered_set<Ohm_slice::CellIdx, Ohm_slice::CellIdxHash, Ohm_slice::CellEqual>& cellidx)
{
	Json::Reader reader;
	Json::Value root;
	ifstream in(filePath, ios::binary);
	if (!in.is_open()) {
		std::cout << "Error open file" << std::endl;
		return;
	}

	if (reader.parse(in, root)) {
		int size = root["size"].asInt();
		for (int i = 0; i < size; ++i) {
			int x = root[to_string((long long)i)][unsigned(0)].asInt();
			int y = root[to_string((long long)i)][unsigned(1)].asInt();
			int z = root[to_string((long long)i)][unsigned(2)].asInt();
			cellidx.insert(Ohm_slice::CellIdx(x,y,z));
		}
	}
	in.close();
}


void getFiles(string path, vector<string>& files)
{
	//文件句柄
	long   hFile = 0;
	//文件信息，声明一个存储文件信息的结构体
	struct _finddata_t fileinfo;
	string p;//字符串，存放路径
	if ((hFile = _findfirst(p.assign(path).append("/*").c_str(), &fileinfo)) != -1)//若查找成功，则进入
	{
		do
		{
			//如果是目录,迭代之（即文件夹内还有文件夹）
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				//文件名不等于"."&&文件名不等于".."
				//.表示当前目录
				//..表示当前目录的父目录
				//判断时，两者都要忽略，不然就无限递归跳不出去了！
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("/").append(fileinfo.name), files);
			}
			//如果不是,加入列表
			else
			{
				files.push_back(p.assign(path).append("/").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		//_findclose函数结束查找
		_findclose(hFile);
	}
}
void DeleteOneCellPixel(Ohm_slice::Cell& cell) {
	for (int k = 0; k < 6; k++) {
		for (int i = 0; i < 16; i++) {
			for (int j = 0; j < 16; j++) {
				delete cell.patch_list[k]->pixel_list[i][j];
			}
		}
	}
}