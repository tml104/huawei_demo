#include "StdAfx.h"
#include "Sep_Indicators.h"
#include <math.h>
#include <stack>

SIpath::SIpath()
{
	cut = false;
	route.clear();
	vertices.clear();
	left.clear();
	right.clear();
}

void SIpath::getSI(OmVeH des, dijkPath &dp)
{
	int iter;
	route.clear();
	OmVeH v_handle = des;
	std::stack<OmHaEgH> sta;
	while(!sta.empty()) sta.pop();
	while(v_handle != dp.sv)
	{
		vertices.insert(v_handle.idx());
		iter = v_handle.idx();
		sta.push(dp.e_records[iter]);
		v_handle = dp.v_records[iter];
	}
	while(!sta.empty())
	{
		route.push_back(sta.top());
		sta.pop();
	}
}

bool SIpath::in_si(int idx)
{
	return vertices.find(idx) != vertices.end();
}

OmHaEgH SIpath::mid_edge()
{
	return route.at(route.size() / 2);
}

void SIpath::clear()
{
	route.clear();
}

void SIpath::push_back(OmHaEgH &he_handle)
{
	route.push_back(he_handle);
}

int SIpath::size()
{
	return route.size();
}

void SIpath::insert(int x)
{
	vertices.insert(x);
}

void SIpath::erase(int x, int y)
{
	vertices.erase(x);
	vertices.erase(y);
}

std::set<int>::iterator SIpath::begin()
{
	return vertices.begin();
}

std::set<int>::iterator SIpath::end()
{
	return vertices.end();
}

//老版本
void SIpath::getWhiskers(OP_Mesh *mesh)
{
	auto e1 = route.begin();
	auto e2 = e1 + 1;
	OmVeH v1, testv1, testv2, testv3;
	OmHaEgH fr1, fr2;
	OmEgH eh;
	bool flag = false;
	for(;e2 != route.end();++e1, e2 = e1+1)
	{
		
		v1 = mesh->to_vertex_handle(*e1);
		fr1 = *e2;
		fr2 = mesh->opposite_halfedge_handle(*e1);
		testv1 = mesh->to_vertex_handle(fr1);
		testv2 = mesh->from_vertex_handle(fr1);
		testv3= mesh->to_vertex_handle(fr2);

		auto ei = mesh->voh_begin(v1);
		for(; ei.is_valid(); ++ei)
		{
			if(*ei == fr1)
			{
				flag = true;
				break;
			}
		}
		
		//没有找到fr1
		if(!flag)
		{
			return ;
		}

		//计算右侧的边
		flag = false;
		do{
			ei++;
			if((!flag) && (!ei.is_valid()))
			{
				ei = mesh->voh_begin(v1);
				flag = true;
			}
			if(*ei == fr2)
			{
				break;
			}
			eh = mesh->edge_handle(*ei);
			//eh所代表的边在si的右侧
			right.insert(eh.idx());
		}while(ei.is_valid());

		//计算左侧的边
		do{
			ei++;
			//只环绕一次
			if((!flag) && (!ei.is_valid()))
			{
				ei = mesh->voh_begin(v1);
				flag = true;
			}
			if(*ei == fr1)
			{
				break;
			}
			eh = mesh->edge_handle(*ei);
			//eh所代表的边在si的右侧
			left.insert(eh.idx());
		}while(ei.is_valid());
	}
}

//新版本
//void SIpath::getWhiskers(OP_Mesh *mesh)
//{
//	auto e1 = route.begin();
//	auto e2 = e1 + 1;
//	OmVeH v1, testv1, testv2, testv3;
//	OmHaEgH fr1, fr2;
//	OmEgH eh;
//	VECTOR norm, vf1, vf2, ev;
//	bool flag = false;
//	int s, s1, s2;
//
//	for(;e2 != route.end();++e1, e2 = e1+1)
//	{
//		v1 = mesh->to_vertex_handle(*e1);
//		norm = mesh->normal(v1);
//		fr1 = *e2;
//		vf1 = meshtools::getVector(fr1, mesh);
//		vf1 = meshtools::project(norm, vf1);
//		fr2 = mesh->opposite_halfedge_handle(*e1);
//		vf2 = meshtools::getVector(fr2, mesh);
//		vf2 = meshtools::project(norm, vf2);
//		s = meshtools::side(vf2, vf1, norm);
//		
//		for(auto ei = mesh->voh_begin(v1); ei.is_valid(); ++ei)
//		{
//			if((*ei) == fr1 || (*ei) == fr2)
//				continue;
//			eh = mesh->edge_handle(*ei);
//			ev = meshtools::getVector(*ei, mesh);
//			ev = meshtools::project(norm, ev);
//			s1 = meshtools::side(ev, vf1, norm);
//			s2 = meshtools::side(ev, vf2, norm);
//			if(s < 0)
//			{
//				if(s1 < 0 && s2 > 0)
//				{
//					left.insert(eh.idx());
//				}
//				else
//				{
//					right.insert(eh.idx());
//				}
//			}
//			else
//			{
//				if(s1 > 0 && s2 < 0)
//				{
//					right.insert(eh.idx());
//				}
//				else
//				{
//					left.insert(eh.idx());
//				}
//			}
//			
//		}
//	}
//}

bool SIpath::left_cross(int eidx)
{
	return left.find(eidx) != left.end();
}

bool SIpath::right_cross(int eidx)
{
	return right.find(eidx) != right.end();
}

//若是一条左侧的边则返回-1， 若是右侧的边则返回1， 不想交则返回0
int SIpath::side(int eidx)
{
	int ret = 0;
	if(left.find(eidx) != left.end())
		//ret += -1;
		return -1;
	if(right.find(eidx) != right.end())
		//ret += 1;
		return 1;
	return 0;
}

bool SIpath::whiskers_exist()
{
	return !(left.empty() && right.empty());
}

/*
	separation indicators
*/

void Sep_Indicators::showloop(OmEgH e, dijkPath &a, dijkPath &b, int color)
{
	OmHaEgH tmp;
	OmHaEgH hfe;
	OmVeH v1, v2;
	hfe = pr_mesh->halfedge_handle(e, 0);
	v1 = pr_mesh->to_vertex_handle(hfe);
	v2 = pr_mesh->from_vertex_handle(hfe);
	int id;
	while(v1 != a.sv)
	{
		id = v1.idx();
		tmp = a.e_records[id];
		hs->show_edge(tmp,color);
		v1 = a.v_records[id];
	}
	while(v2 != b.sv)
	{
		id = v2.idx();
		tmp = b.e_records[id];
		hs->show_edge(tmp,color);
		v2 = b.v_records[id];
	}
}

//
void Sep_Indicators::show_SIpath(SIpath &path, int color)
{
	for(auto iter= path.route.begin(); iter != path.route.end();++iter)
	{
		hs->show_edge(*iter, color);
	}
}

void Sep_Indicators::show_SIpath(int sii, int color)
{
	show_SIpath(sp[sii], color);
}

void Sep_Indicators::show_whisker(int sii)
{
	show_SIpath(sp[sii], 1);
	for(auto e = pr_mesh->edges_begin(); e != pr_mesh->edges_end(); ++e)
	{
		if(sp[sii].left_cross(e->idx()))
			hs->show_edge(*e, 2);
		if(sp[sii].right_cross(e->idx()))
			hs->show_edge(*e, 3);
	}
}

//构造函数
Sep_Indicators::Sep_Indicators(OP_Mesh *bs_mesh, HoopsView *hoops_view)
{
	pr_mesh = bs_mesh;
	hs = new hoopsshow(bs_mesh, hoops_view);
	edge_length = new double[pr_mesh->n_edges()];
	//计算各个边的长度，保存到数组edge_length
	int iter;
	for(int i = 0;i < pr_mesh->n_edges();i++)
	{
		edge_length[i] = 0.0;
	}
	for(auto e_it = pr_mesh->edges_begin();e_it != pr_mesh->edges_end();++e_it)
	{
		iter = (*e_it).idx();//边的下标
		this->edge_length[iter] = meshtools::distance(pr_mesh->halfedge_handle(*e_it,0), pr_mesh);
	}
	//test_facehandle.clear();
}

//计算σ(e),e为不在集合T中的边
void Sep_Indicators::cal_sigma_e(double sigma_e[],dijkPath &bx,  int in_T[])
{
	OmEgH e;
	OmHaEgH hfe;
	OmVeH v1, v2;
	int iter;
	double s;
	for(auto e_iter = pr_mesh->edges_begin(); e_iter != pr_mesh->edges_end();++e_iter)
	{
		e = *e_iter;
		iter = e.idx();
		//in_T == -1 means e is in T
		if(in_T[iter] != -1)
		{
			hfe = pr_mesh->halfedge_handle(e, 0);
			v1 = pr_mesh->to_vertex_handle(hfe);
			v2 = pr_mesh->from_vertex_handle(hfe);
			sigma_e[iter] = edge_length[iter] + bx.list[v1.idx()] + bx.list[v2.idx()];
			in_T[iter] = 0;
		}
	}
}

void Sep_Indicators::cal_sigma_e(double sigma_e[],dijkPath &a, dijkPath &b, int in_T[])
{
	OmEgH e_handle;
	OmHaEgH hf1, hf2;
	OmVeH v1,v2;
	int iter;
	int cnt = 0;
	double s1,s2;
	for(auto e_it  = pr_mesh->edges_begin();e_it != pr_mesh->edges_end();++e_it)
	{
		e_handle = *e_it;
		iter = e_handle.idx();
		if(in_T[iter] != -1)
		{
			hf1 = pr_mesh->halfedge_handle(e_handle,0);
			//hf2 = pr_mesh->halfedge_handle(e_handle,1);
			//v1->a, v2->b
			v2 = pr_mesh->to_vertex_handle(hf1);
			v1 = pr_mesh->from_vertex_handle(hf1);
			s1 = a.list[v1.idx()] + b.list[v2.idx()] + edge_length[iter];
			s2 = a.list[v2.idx()] + b.list[v1.idx()] + edge_length[iter];

			cnt++;

			//0, 1 标记方向，0: to -> a, from -> b
			if(s1 < s2)
			{
				in_T[iter] = 0;
				sigma_e[iter] = s1;
			}
			else
			{
				in_T[iter] = 1;
				sigma_e[iter] = s2;
			}
		}
	}
}

//T*是（G\T）*的最大生成树

void Sep_Indicators::get_T_star(double sigma_e[], int in_T[], int T_star[])
{
	OmEgH e_handle;
	OmHaEgH hf_handle;
	OmFaH fa1,fa2;
	int iter;
	int v1,v2;
	int id_v1,id_v2;
	//Heap heap_max(sigma_e, pr_mesh->n_edges(), true);
	maxHeap heap_max;
	Con_set conn(pr_mesh->n_faces());
	//
	int cnt = 0;

	//
	for(auto e_it  = pr_mesh->edges_begin();e_it != pr_mesh->edges_end();++e_it)
	{
		e_handle = (*e_it);
		iter = e_handle.idx();
		//T*全置-1
		T_star[iter] = -1;
		//std::cout<<"edge  "<<iter<<" : "<<in_T[iter]<<std::endl;
		if(in_T[iter] != -1)
		{
			heap_max.push(iter, sigma_e[iter]);
		}
	}
	//std::cout<<"Heap n = "<<heap_max.size()<<std::endl;

	for(iter = heap_max.pop(); iter != -1;iter = heap_max.pop())
	{
		
		e_handle = pr_mesh->edge_handle(iter);
		hf_handle = pr_mesh->halfedge_handle(e_handle, 0);
		fa1 = pr_mesh->face_handle(hf_handle);
		hf_handle = pr_mesh->halfedge_handle(e_handle, 1);
		fa2 = pr_mesh->face_handle(hf_handle);
		//找出边相邻的两个面的编号，即是对偶图上点的编号
		v1 = fa1.idx();
		v2 = fa2.idx();
		//
		v1 = conn.idx(v1);
		v2 = conn.idx(v2);
		//std::cout<<"heap : "<<cnt++<<std::endl;
		if(v1 != v2) //若边e相邻的面在对偶图上不联通，则将e加入集合T*
		{
			conn.connect(v1,v2);
		}
		else
		{
			//
			T_star[iter] = in_T[iter];
			//hs->show_edge(e_handle, 0);
		}
	}
}

//给定两个奇异点a，b，找出连接他们的SI
void Sep_Indicators::find_SI(OmVeH singu_a, OmVeH singu_b)
{
	
	OmVeH basepoint_x;//基点
	double sigma_e[max_edge];
	int T[max_edge];
	int T_star[max_edge];
	bool homotoy_basis[max_edge];
	double tmp_min;
	double edge_len;
	int iter;
	int int_tmp, cnt;
	OmVeH v_handle, v1, v2;
	OmEgH e_handle;
	OmHaEgH hf_handle, hfe2, he_handle, he_tmp;
	SIpath *route;
	std::stack<OmHaEgH> stac;

	dijkPath sa(singu_a, pr_mesh);
	dijkPath sb(singu_b, pr_mesh);

	int tk = 0;
	
	//以a点为源点做一次最短路算法
	sa.dijkstra(edge_length);
	//以b点为源点做一次最短路算法
	sb.dijkstra(edge_length);
	//保存a到b的最短路
	route = new SIpath();
	route->getSI(singu_b, sa);
	route->erase(singu_a.idx(), singu_b.idx());
	sp.push_back(*route);
	
	//决定基点
	hf_handle = route->mid_edge();
	basepoint_x = pr_mesh->to_vertex_handle(hf_handle);
	if(basepoint_x == singu_a || basepoint_x== singu_b)
		basepoint_x = pr_mesh->from_vertex_handle(hf_handle);
	dijkPath bx(basepoint_x, pr_mesh);

	bx.setHoopsshow(hs);
	
	//以基点为源点做一次最短路算法
	bx.dijkstra(edge_length);

	for(int i = 0;i < pr_mesh->n_edges();i++)
	{
		T[i] = 0;
	}
	cnt = 0;
	for(auto v_it = pr_mesh->vertices_begin();v_it != pr_mesh->vertices_end();++v_it)
	{
		if((*v_it) != basepoint_x)
		{
			iter = (*v_it).idx();
			e_handle = pr_mesh->edge_handle(bx.e_records[iter]);
			//-1表示该边属于某一条最短路
			T[e_handle.idx()] = -1;
			cnt++;
		}
	}
	
	//计算σ(e)
	cal_sigma_e(sigma_e, sa, sb, T);

	//找出T*
	get_T_star(sigma_e, T, T_star);

	cnt = 0;
	for(auto e_it = pr_mesh->edges_begin();e_it != pr_mesh->edges_end();++e_it)
	{
		e_handle = *e_it;
		iter = e_handle.idx();
		//if(T[iter] != -1 && T_star[iter] != -1)
		if(T_star[iter] != -1)
		{
			cnt++;
			hf_handle = pr_mesh->halfedge_handle(e_handle,T_star[iter]);
			v2 = pr_mesh->to_vertex_handle(hf_handle);
			v1 = pr_mesh->from_vertex_handle(hf_handle);
			while(!stac.empty()) stac.pop(); //清空栈
			route = new SIpath();
			route->clear();//清空队列
			for(v_handle = v1;v_handle != singu_a;v_handle = sa.v_records[v_handle.idx()])
			{
				he_tmp = sa.e_records[v_handle.idx()];
				route->insert(v_handle.idx());
				//注意这里he_tmp和整个SI的方向是相反的，所以要取对边
				stac.push(he_tmp);
			}
			while(!stac.empty())
			{
				route->push_back(stac.top());
				stac.pop();
			}
			route->push_back(hf_handle);
			for(v_handle = v2;v_handle != singu_b;v_handle = sb.v_records[v_handle.idx()])
			{
				he_tmp = sb.e_records[v_handle.idx()];
				route->insert(v_handle.idx());
				route->push_back(pr_mesh->opposite_halfedge_handle(he_tmp));
			}
			//将奇点在点集中去除
			route->erase(singu_a.idx(), singu_b.idx());
			sp.push_back(*route);
		}
	}
}

void Sep_Indicators::add_SI(OmVeH start_vertex, OmVeH destination_vertex)
{
	SIpath route;
	dijkPath dp(start_vertex, pr_mesh);
	dp.dijkstra(edge_length);
	route.getSI(destination_vertex, dp);
	route.getWhiskers(pr_mesh);
	sp.push_back(route);
}

void Sep_Indicators::get_SIs(std::vector<OmVeH> &singularities)
{
	
	std::cout<<"faces : "<<pr_mesh->n_faces()<<std::endl;
	std::cout<<"edges : "<<pr_mesh->n_edges()<<std::endl;
	std::cout<<"vertices : "<<pr_mesh->n_vertices()<<std::endl;

	int th = 0;
	std::cout<<"singularities: "<<singularities.size()<<std::endl;
	sp.clear();
	auto sin_iter1 = singularities.begin();
	auto sin_iter2 = sin_iter1+1;
	for(;sin_iter1 != singularities.end();++sin_iter1)
	{
			for(sin_iter2 = sin_iter1+1;sin_iter2 != singularities.end();++sin_iter2)
			{
				find_SI(*sin_iter1, *sin_iter2);
			}
	}

	/*th = 0;
	for(auto iter = sp.begin(); iter != sp.end();++iter)
	{
		th++;
		show_SIpath(*iter, 3);
		if(th > 0)
			break;
	}*/

	std::cout<<"SIs: "<<sp.size()<<std::endl;
}

double* Sep_Indicators::edgeLength()
{
	return edge_length;
}

void Sep_Indicators::get_whiskers()
{
	int cnt = 0;
	for(auto i = sp.begin(); i != sp.end(); ++i)
	{
		if(!i->whiskers_exist())
			i->getWhiskers(pr_mesh);
	}
}