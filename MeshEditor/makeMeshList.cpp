#include "stdafx.h"
#include "meshHeader.h"

void makeMeshList(std::priority_queue<Interval*, std::vector<Interval*>, compareNodePointer> &priority_q, std::set<double> &point_position){
	std::vector<Interval*> intervals = intervalSort(priority_q);
	unionInterval(intervals);
	for ( int i=0;i<intervals.size();i++){
		if(intervals[i]->right-intervals[i]->left < intervals[i]->meshSize){
			intervals.erase(intervals.begin()+i);
			i--;
		}
	}

	for (int i = 0; i < intervals.size()-1; ++i){
		if (intervals[i]->right != intervals[i+1]->left){
			intervals[i]->right = intervals[i+1]->left;
		}
	}
	double r = 1.2;
	generationNonuniformMesh(intervals, r);
	getMeshPoint(intervals, r);
	for(int i = 0; i < intervals.size(); ++i){
		for (int j = 0 ; j < intervals[i]->meshPosition.size(); ++j){
			point_position.insert(intervals[i]->meshPosition[j]);
		}
	}
}
//auto readMeshInfo(std::string filename)->std::vector<Interval*>*{
//	std::vector<Interval*> intervals[3];
//	std::ifstream ifs;
//	//ifs.open("D:/meshSize.json");
//	//ifs.open("C:/Users/Administrator/Desktop/HuaweiTest/meshSize.json");
//	ifs.open(filename);
//    assert(ifs.is_open());
//    Json::Reader reader;
//    Json::Value root;
//    reader.parse(ifs, root, false);
//	std::vector<string> nameList;
//	for(int i=0;i<root["solids"].size();i++){
//		string str = root["solids"][i]["name"].asString();
//		for(int j=0;j<str.length();j++){
//			if(str[j]==':'){
//				str[j] = '/';
//			}
//		}
//		nameList.push_back(str);
//	}
//	std::map<string,double> mapx,mapy,mapz;
//	
//	for(int i=0;i<root["meshInfo"].size();i++){
//		printf("test\n");
//		double x,y,z;
//		int xid = 0,yid=1,zid=2;
//		string meshSize = root["meshInfo"][i]["meshSize"].asString();
//		int xpos = meshSize.find(",",0);
//		int ypos = meshSize.find(",",xpos+1);
//		string xs = meshSize.substr(0,xpos);
//		string ys = meshSize.substr(xpos+1,ypos-xpos-1);
//		string zs = meshSize.substr(ypos+1,meshSize.length()-ypos-1);
//		std::cout<<xs<<" "<<ys<<" "<<zs<<std::endl;
//		x = atof(const_cast<const char *>(xs.c_str()));
//		y = atof(const_cast<const char *>(ys.c_str()));
//		z = atof(const_cast<const char *>(zs.c_str()));
//		std::cout<<x<<" "<<y<<" "<<z<<std::endl;
//		std::cout<<root["meshInfo"][i]["SolidName"].size()<<std::endl;
//		for(int j=0;j<root["meshInfo"][i]["SolidName"].size();j++){
//			string name = root["meshInfo"][i]["SolidName"][j].asString();
//			std::cout<<name<<std::endl;
//			mapx[name]=x;
//			mapy[name]=y;
//			mapz[name]=z;
//		}
//	}
//	double min_x = 1000000000, min_y = 10000000000, min_z = 10000000000, max_x = -100000000, max_y = -1000000000,max_z = -100000000;
//
//	for (int i = 0; i < body_list.count(); ++i){	
//		LUMP* lump = ((BODY*)body_list[i]) ->lump();
//		while(lump){
//			SPAposition low, high;
//			api_get_entity_box(lump, low, high);
//			double x1 = low.x();double y1 = low.y();double z1 = low.z();
//			double x2 = high.x();double y2 = high.y();double z2 = high.z();
//
//			min_x = min(min_x, x1);
//			max_x = max(max_x, x2);
//			min_y = min(min_y, y1);
//			max_y = max(max_y, y2);
//			min_z = min(min_z, z1);
//			max_z = max(max_z, z2);
//			//if (x2-x1 > 0.1 && y2-y1 > 0.1){
//				double meshSizex = mapx[nameList[i]];
//				double meshSizey = mapy[nameList[i]];
//				double meshSizez = mapz[nameList[i]];
//				if(meshSizex != 0){
//					Interval* interval = new Interval(x1,x2,meshSizex);
//					priority_q.push(interval);
//				}
//				if(meshSizey != 0){
//					Interval* interval = new Interval(y1,y2,meshSizey);
//					priority_qy.push(interval);
//				}
//				if(meshSizez != 0){
//					Interval* interval = new Interval(z1,z2,meshSizez);
//					priority_qz.push(interval);
//				}
//			//}
//			lump = lump->next();
//			//number++;
//		}
//		//cout << number<<endl;
//	}
//
//
//	priority_q.push(new Interval(min_x, max_x, 0.4));
//	priority_qy.push(new Interval(min_y, max_y, 0.4));
//	priority_qz.push(new Interval(min_z, max_z, 0.4));
//}