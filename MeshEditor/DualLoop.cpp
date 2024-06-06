#include "StdAfx.h"
#include "DualLoop.h"
#include<stack>
#include<iostream>
#include<queue>
#include<fstream>

P_edge::P_edge()
{
	next = -1;//-1表示没有后继的边
	support.clear();
	weight = 0.0;
	theta_con = false;
}

double P_edge::cos_theta()
{
	return meshtools::scalar(e, Fd);
}

P_node::P_node()
{
	Fd = -1;
	edge_begin = -1;
	edge_end = -1;
}

int P_edge::idx()
{
	return _idx;
}
void P_edge::set_idx(int idx)
{
	_idx = idx;
}

int P_node::idx()
{
	return _idx;
}
void P_node::set_idx(int idx)
{
	_idx = idx;
}

//返回新加入边的标号
int ExpMap::add_edge(P_node &p1, P_node &p2)
{
	P_edge *ne;
	P_edge last;
	ne = new P_edge;
	int n = edges.size();
	int i;
	//ne从p1到p2
	ne->to_vertice = p2.idx();
	ne->from_vertice = p1.idx();
	edges.push_back(*ne);
	//ne = &(edges[n]);
	//设置边的起点终点
	//将新边加入顶点的邻接表
	if(p1.edge_end == -1)
	{
		p1.edge_begin = n;
		p1.edge_end = n;
	}
	else
	{
		ne = &(edges[p1.edge_end]);
		ne->next = n;
		p1.edge_end = n;
	}
	return n;
}

bool ExpMap::theta_constraint(P_edge &e)
{
	double fdL, eL;
	double cosin;
	double lim = 0.707107;
	for(auto i = e.support.begin(); i != e.support.end();++i)
	{
		if(edges[edge2P[*i]].cos_theta() < 1e-6)
			return false;
	}
	fdL = meshtools::length(e.Fd);
	eL = meshtools::length(e.e);
	lim *= fdL * eL;
	cosin = meshtools::scalar(e.Fd, e.e);
	return cosin > lim;
}

void ExpMap::setHoopsshow(hoopsshow *hoops)
{
	hs = hoops;
}

void ExpMap::show_edge(int eidx, int color)
{
	m4->show_edge(edges[eidx].support, color);
}

ExpMap::ExpMap(M4 *branchedmap, int degree)
{
	m4 = branchedmap;
	eps = 1.0e-6;
	nodes.clear();
	edges.clear();
	_degree = degree;

	mesh = m4->pr_mesh;

	for(int i = 0;i < m4->n_vertices();++i)
	{
		mesh2P[i] = -1;
	}
}

void ExpMap::construct_1ring()
{
	OmVeH mesh_v;
	CoveringNode *m4_v;
	CoveringEdge *ce;
	VECTOR p1, p2;
	VECTOR se, e1, e2, e3, n;
	int idx, v2_P, v1_P;
	int cnt = 0;
	int v1, v2;
	double weight;
	P_node *q;
	int eit;
	for(int i = 0; i < nodes.size();i++)
	{
		q = &(nodes[i]);
		//nodes[i]在M4上的对应点
		idx = q->idx();
		//nodes[i]在pr_mesh上的对应点的handle
		m4_v = &(m4->vertices[idx]);
		n = q->orth;

		cnt++;
		mesh_v = mesh->vertex_handle(m4_v->meshIdx);
		p1 = mesh->point(mesh_v);

		for(auto j = m4_v->edge_begin; j != -1; j = m4->edges[j].next)
		{
			//v1 -> v2
			ce = &(m4->edges[j]);
			v2 = ce->to_vertex;
			v2_P = mesh2P[v2];

			

			eit = add_edge(nodes[i], nodes[v2_P]);
			
			edges[eit].support.push_back(j);
			//P上边在M4上的对应边
			edges[eit].set_idx(j);
			//M4上的边在P上对应边
			edge2P[j] = eit;
			edges[eit].e = meshtools::project(n, ce->e);
			//edges[eit].e = ce->e;
			edges[eit].Fd = meshtools::project(n, ce->Fd);
			//edges[eit].Fd = ce->Fd;
			edges[eit].theta_con = theta_constraint(edges[eit]);

			//get w(e)
			//weight = meshtools::w_e(edges[eit].e, edges[eit].Fd);
			weight = meshtools::w_e(ce->e, ce->Fd);
			edges[eit].weight = weight;
		}
	}
}

void ExpMap::construct_kring()
{
	dijkstraOnCS dp(m4);
	std::vector<int> k_vert;
	std::stack<int> stac;
	int ev, eidx, vidx, pe, pv, ov;
	int pe1, pe2;
	int neidx;
	VECTOR e, e1;
	VECTOR Fd, Fd1;
	double Le, Ls, w, sigmaL;
	double testL;

	/*eidx, vidx, ev在M4上
		v, pe, pv, ov在P上*/
	for(int v = 0; v < nodes.size();++v)
	{
		//std::cout<<"v = "<<v<<std::endl;
		dp.dijkstra(nodes[v].idx(), _degree, k_vert);
		for(auto iter = k_vert.begin();iter != k_vert.end();++iter)
		{
			ev = *iter;
			while(!stac.empty()) stac.pop();
			eidx = dp.e_records[ev];
			vidx = dp.v_records[ev];
			e = VECTOR(0.0, 0.0, 0.0);
			Fd = VECTOR(0.0, 0.0, 0.0);
			sigmaL = 0.0;
			while(vidx >= 0)
			{
				stac.push(eidx);

				pv = mesh2P[vidx];
				pe = edge2P[eidx];
				//edges[pe].e将e指数映射到v的切平面上
				e1 = meshtools::rotation(nodes[v].orth, nodes[pv].orth,edges[pe].e);
				//
				Fd1 = meshtools::rotation(nodes[v].orth, nodes[pv].orth,edges[pe].Fd);
				//Fd1 = edges[pe].Fd;
				e += e1;
				
				Le = meshtools::length(e1);
				Fd += meshtools::mul(Fd1, Le);
				sigmaL += Le;

				eidx = dp.e_records[vidx];
				vidx = dp.v_records[vidx];
			}

			neidx = add_edge(nodes[v], nodes[mesh2P[ev]]);
			
			while(!stac.empty())
			{
				edges[neidx].support.push_back(stac.top());
				stac.pop();
			}
			
			//计算w(e)
			testL = 0.0;
			/*for(auto i = edges[neidx].support.begin();i != edges[neidx].support.end();++i)
			{
				Ls = dp.length[*i];
				Fd += meshtools::mul(m4->edges[*i].Fd, Ls);
			}*/

			//std::cout<<"Le : Σ Ls ；"<<Le<<" "<<testL<<std::endl;
			//edges[neidx].e = e;
			e = meshtools::unitize(e);
			edges[neidx].e = meshtools::mul(e, sigmaL);

			Fd = meshtools::unitize(Fd);
			//edges[neidx].Fd = meshtools::project(nodes[v].orth, Fd);
			edges[neidx].Fd = Fd;
			edges[neidx].weight = meshtools::w_e(edges[neidx].e,edges[neidx].Fd);
			edges[neidx].theta_con = theta_constraint(edges[neidx]);
		}
	}
}

void ExpMap::construct()
{
	int n = 0;
	OmVeH v1, v2;
	OmHaEgH hfe;
	P_node *p;
	for(int i = 0;i < m4->n_vertices();i++)
	{
		/*if(m4->vertices[i].edge_begin == -1)
			continue;*/
		p = new P_node();
		p->set_idx(i);
		p->orth = meshtools::unitize(m4->vertices[i].orth);
		////
		mesh2P[i] = n;
		nodes.push_back(*p);
		n++;
	}
	construct_1ring();
	construct_kring();
}

//返回eidx的support所对应的mesh上的半边
void ExpMap::getMeshHaEg(int eidx, std::vector<OmHaEgH> &ret)
{
	int meshidx;
	ret.clear();
	for(auto iter = edges[eidx].support.begin(); iter != edges[eidx].support.end(); ++iter)
	{
		meshidx = m4->edges[*iter].meshIdx;
		ret.push_back(mesh->halfedge_handle(meshidx));
	}
}

void ExpMap::getHaEgStack(int eidx, std::stack<int> &ret)
{
	int meshidx;
	while(!ret.empty()) ret.pop();
	for(auto iter = edges[eidx].support.begin(); iter != edges[eidx].support.end(); ++iter)
	{
		meshidx = m4->edges[*iter].meshIdx;
		ret.push(meshidx);
	}
}

void ExpMap::test_e(int v)
{
	int eidx;
	int pidx, meshidx, m4idx;
	int meshedge;
	VECTOR p;
	m4idx = nodes[v].idx();
	meshidx = m4->vertices[m4idx].meshIdx;
	p = mesh->point(mesh->vertex_handle(meshidx));
	for(eidx = nodes[v].edge_begin; eidx != -1; eidx = edges[eidx].next)
	{
		hs->show_vector(p, edges[eidx].e, 3);
		for(auto i = edges[eidx].support.begin(); i != edges[eidx].support.end();++i )
		{
			meshedge = m4->edges[*i].meshIdx;
			hs->show_edge(mesh->halfedge_handle(meshedge), 2);
		}
	}
}

/*
***************************对偶环路类**************************************************
*/

DualLoop::DualLoop()
{
	mesh_halfedges.clear();
	M4_edges.clear();
	M4_vertices.clear();
	crossed_si.clear();
	length = 0.0;
}

//将M4上的一条边加入dual loop
void DualLoop::add_path(int eidx, M4 *m4)
{
	int v_m4_idx;
	v_m4_idx = m4->edges[eidx].to_vertex;
	M4_edges.push_back(eidx);
	mesh_halfedges.push_back(m4->edges[eidx].meshIdx);
	if(M4_vertices.find(v_m4_idx) == M4_vertices.end())
		M4_vertices.insert(v_m4_idx);
}

//将P上的一条边加入dual loop
void DualLoop::add_path(int eidx, ExpMap *P)
{
	for(auto i = P->edges[eidx].support.begin();i != P->edges[eidx].support.end();++i)
	{
		M4_edges.push_back(*i);
		mesh_halfedges.push_back(P->m4->edges[*i].meshIdx);
		//
		M4_vertices.insert(P->m4->edges[*i].to_vertex);
	}
	//测试用
	P_edges.push_back(eidx);
}

//将一个M4上点加入dualloop
void DualLoop::add_vertex(int vidx)
{
	M4_vertices.insert(vidx);
}

std::vector<int>::iterator DualLoop::M4_edge_begin()
{
	return M4_edges.begin();
}
std::vector<int>::iterator DualLoop::M4_edge_end()
{
	return M4_edges.end();
}

std::vector<size_t>::iterator DualLoop::mesh_begin()
{
	return mesh_halfedges.begin();
}
std::vector<size_t>::iterator DualLoop::mesh_end()
{
	return mesh_halfedges.end();
}

std::set<int>::iterator DualLoop::M4_vertex_begin()
{
	return M4_vertices.begin();
}

std::set<int>::iterator DualLoop::M4_vertex_end()
{
	return M4_vertices.end();
}

std::set<int>::iterator DualLoop::si_begin()
{
	return crossed_si.begin();
}
std::set<int>::iterator DualLoop::si_end()
{
	return crossed_si.end();
}

std::vector<OmHaEgH> DualLoop::getHalfedgesHandle(OP_Mesh *mesh)
{
	std::vector<OmHaEgH> route;
	route.clear();
	for(int i = 0; i < mesh_halfedges.size(); ++i)
	{
		route.push_back(mesh->halfedge_handle(mesh_halfedges[i]));
	}
	return route;
}


bool DualLoop::M4_vertex_cross(int vidx)
{
	return M4_vertices.find(vidx) != M4_vertices.end();
}

//判断vidx是否在dual loops上，vidx在M4上
bool DualLoop::M2_vertex_cross(int vidx)
{
	//v2为与vidx在同一M2层的M4顶点
	int v2 = vidx +2;
	if((v2 / 4) != (vidx / 4))
		v2 -= 4;
	return (M4_vertices.find(vidx) != M4_vertices.end()) || (M4_vertices.find(v2) != M4_vertices.end());
}

bool DualLoop::M2_edge_cross(int eidx, ExpMap *P)
{
	int v;
	v = P->m4->edges[eidx].to_vertex;
	return M2_vertex_cross(v);
}


int DualLoop::size()
{
	return M4_edges.size();
}

bool DualLoop::crossed(int si_idx)
{
	return crossed_si.find(si_idx) != crossed_si.end();
}

void DualLoop::cross(int si_idx)
{
	crossed_si.insert(si_idx);
}

bool DualLoop::in_left(size_t idx)
{
	return left.find(idx) != left.end();
}

bool DualLoop::in_right(size_t idx)
{
	return right.find(idx) != right.end();
}

bool DualLoop::whiskers_exist()
{
	return !(left.empty() && right.empty());
}

bool DualLoop::operator<(DualLoop &y)
{
	return this->length < y.length;
}

void DualLoop::getWhiskers(OP_Mesh *mesh)
{
	auto route = getHalfedgesHandle(mesh);

	auto e1 = route.begin();
	auto e2 = e1 + 1;
	OmVeH v1, testv1, testv2, testv3;
	OmHaEgH fr1, fr2;
	OmEgH eh;
	bool flag = false;
	for(;e1 != route.end();++e1, e2 = e1+1)
	{
		//由于是环路，端点要特别处理一下
		if(e2 == route.end())
			e2 = route.begin();

		v1 = mesh->to_vertex_handle(*e1);
		fr1 = *e2;
		fr2 = mesh->opposite_halfedge_handle(*e1);
		testv1 = mesh->to_vertex_handle(fr1);
		testv2 = mesh->from_vertex_handle(fr1);
		testv3= mesh->to_vertex_handle(fr2);

		//std::cout<<"e1 : "<<e1->idx()<<std::endl;
		//std::cout<<"e2 : "<<e2->idx()<<std::endl;

		auto ei = mesh->voh_begin(v1);
		for(; ei.is_valid(); ++ei)
		{
			//std::cout<<"ei : "<<ei->idx()<<std::endl;
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
			//right.insert((mesh->opposite_halfedge_handle(*ei)).idx());
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
			//left.insert((mesh->opposite_halfedge_handle(*ei)).idx());
		}while(ei.is_valid());
	}
}

void DualLoop::show_whiskers(hoopsshow *hs, OP_Mesh *mesh)
{
	for(auto i = left.begin(); i != left.end(); ++i)
	{
		hs->show_edge(mesh->edge_handle(*i) , 2);
	}

	for(auto i = right.begin(); i != right.end(); ++i)
	{
		hs->show_edge(mesh->edge_handle(*i) , 3);
	}
}

//将dualloop写入到文件
void DualLoop::write_to_file(std::ofstream &fout)
{
	//保存loop的长度
	fout<<this->length<<' ';
	////保存M4边
	//fout<<M4_edges.size()<<' ';
	//for(auto iter = M4_edges.begin(); iter != M4_edges.end(); ++iter)
	//{
	//	fout<<*iter<<' ';
	//}
	//fout<<std::endl;

	//保存P边
	fout<<P_edges.size()<<' ';
	for(auto iter = P_edges.begin(); iter != P_edges.end(); ++iter)
	{
		fout<<*iter<<' ';
	}
	fout<<std::endl;
}
//从文件中读取dualloop
void DualLoop::read_from_file(std::ifstream &fin, M4 *m4)
{
	int n_m4edges;
	int edge_idx;
	fin>>this->length;
	fin>>n_m4edges;
	for(int i = 0;i < n_m4edges; ++i)
	{
		fin>>edge_idx;
		add_path(edge_idx, m4);
	}
}

void DualLoop::read_from_file(std::ifstream &fin, ExpMap *P)
{
	int n_Pedges;
	int edge_idx;
	fin>>this->length;
	//读入P边
	fin>>n_Pedges;
	for(int i = 0;i < n_Pedges; ++i)
	{
		fin>>edge_idx;
		add_path(edge_idx, P);
	}
}

/*
**********************Propagation***********************************************
*/

Propagation::Propagation(ExpMap *expmap)
{
	P = expmap;
	maxe = P->edges.size();
	maxv = P->nodes.size();
	e_records = new int[maxv];
	v_records = new int[maxv];
	list = new double[maxv];
	inS = new bool[maxv];
	existingLoops.clear();

	//init();
}

void Propagation::clear()
{
	for(int i = 0;i < maxv;++i)
	{
		e_records[i] = -1;
		v_records[i] = -1;
		list[i] = -1;
		inS[i] = false;
	}
}
void Propagation::show_dualloop(DualLoop &loop, int color)
{
	int cnt = 0;

	//测试用
	if(color < 0)
	{
		int c = 0;
		for(auto i = loop.P_edges.begin(); i != loop.P_edges.end(); ++i)
		{
			P->show_edge(*i, (c++)%4);
		}
	}

	for(auto iter = loop.M4_edge_begin(); iter != loop.M4_edge_end(); ++iter)
	{
		P->m4->show_edge(*iter, color);
	}
}
void Propagation::show_existingLoops(int color)
{
	int cnt = 0;
	for(auto iter = existingLoops.begin(); iter != existingLoops.end();++iter)
	{
		if(color < 0)
		{
			show_dualloop(*iter, cnt);
			cnt++;
			cnt %= 5;
		}
		else
			show_dualloop(*iter, color);
	}
}

void Propagation::init(int sv)
{
	clear();
	list[sv] = 0.0;
	Labels flags;
	flags.clear();
	heap.clear();
	heap.push(sv, list[sv], flags);
	source_vertex = sv;
}


void Propagation::reset()
{
	list[source_vertex] = 0.0;
}

double Propagation::promote()
{
	bool crossed;
	int cnt = 0, er, vr;
	int v, v2;
	int flag;
	int eidx;
	double vlen, elen;
	double eps = 1e-6;
	pHeapNode pv;
	Labels NewFlags;

	for(pv = heap.pop();pv.idx != -1;pv = heap.pop())
	{
		v = pv.idx;

		if(inS[v])
		{
			if(v == source_vertex)
				return list[source_vertex];
			else
				continue;
		}


		er = e_records[v];
		vr = v_records[v];
		
		inS[v] = true;
		vlen = list[v];

		cnt = 0;
		for(eidx = P->nodes[v].edge_begin; eidx != -1; eidx = P->edges[eidx].next)
		{
			//不满足θ-constriant则不能选取该边
			if(P->edges[eidx].theta_con == false)
				continue;

			//
			v2 = P->edges[eidx].to_vertice;
			elen = P->edges[eidx].weight;

			//判断是否与已存在的loops相交
			/*if(Pedge_M2_cross(P->edges[eidx]) == -1)
				continue;*/

			NewFlags = pv.flags;
			if(Pedge_M2_cross(P->edges[eidx], NewFlags) == -1)
				continue;

			/*if(NewFlags.size() > 0)
				std::cout<<"NewFlags size : "<<NewFlags.size()<<std::endl;*/
			//
			if(v2 == source_vertex)
			{
				if(list[v2] < eps ||  vlen + elen < list[v2] )
				{
					list[v2] = vlen + elen;
					e_records[v2] = eidx;
					v_records[v2] = v;
					heap.push(v2, list[v2], NewFlags);
				}
			}
			else
			if((!inS[v2]) && ((list[v2] < 0)  || (vlen + elen < list[v2])))
			{
				list[v2] = vlen + elen;
				e_records[v2] = eidx;
				v_records[v2] = v;
				heap.push(v2, list[v2], NewFlags);
			}
		}
		
	}
	return -1;
}

double Propagation::promote(std::vector<DualLoop> &featureCurves)
{
	int cnt = 0, er, vr;
	int v, v2;
	int eidx;
	double vlen, elen;
	double eps = 1e-6;
	char s;
	pHeapNode pv;
	Labels NewFlags;

	for(pv = heap.pop();pv.idx != -1;pv = heap.pop())
	{
		v = pv.idx;

		if(inS[v])
		{
			if(v == source_vertex)
				return list[source_vertex];
			else
				continue;
		}

		er = e_records[v];
		vr = v_records[v];
		
		inS[v] = true;
		vlen = list[v];

		cnt = 0;
		for(eidx = P->nodes[v].edge_begin; eidx != -1; eidx = P->edges[eidx].next)
		{
			//不满足θ-constriant则不能选取该边
			if(P->edges[eidx].theta_con == false)
				continue;

			//
			v2 = P->edges[eidx].to_vertice;
			elen = P->edges[eidx].weight;

			//判断是否与已存在的loops相交
			/*if(Pedge_M2_cross(P->edges[eidx]) == -1)
				continue;*/

			NewFlags = pv.flags;
			if(Pedge_M2_cross(P->edges[eidx], NewFlags) == -1)
			{
				continue;
			}

			//判断是否与feature curves相交
			if(Pedge_M2_cross(P->edges[eidx], featureCurves) == -1)
				continue;

			if(v2 == source_vertex)
			{
				if(list[v2] < eps ||  vlen + elen < list[v2] )
				{
					list[v2] = vlen + elen;
					e_records[v2] = eidx;
					v_records[v2] = v;
					heap.push(v2, list[v2], NewFlags);
				}
			}
			else
			if((!inS[v2]) && ((list[v2] < 0)  || (vlen + elen < list[v2])))
			{
				list[v2] = vlen + elen;
				e_records[v2] = eidx;
				v_records[v2] = v;
				heap.push(v2, list[v2], NewFlags);
			}
		}
		
	}
	return -1;
}

DualLoop Propagation::getLoop()
{
	DualLoop DL;
	std::stack<int> sta;
	int v, e;

	if(list[source_vertex] < 1e-6)
		return DL;

	while(!sta.empty())
		sta.pop();

	v = source_vertex;
	do{
		e = e_records[v];
		sta.push(e);
		//DL.add_path(e, P);
		v = v_records[v];
	}while(v != source_vertex);
	
	while(!sta.empty())
	{
		e = sta.top();
		sta.pop();
		DL.add_path(e, P);
	}

	DL.length = list[source_vertex];
	return DL;
}

int Propagation::Pedge_M2_cross(P_edge &pedge, DualLoop &loop)
{
	int eidx, vidx1, vidx2;
	bool d1, d2;
	int rd = 0;

	/*P边的起点要特别计算*/
	auto i = pedge.support.begin();
	vidx1 = (P->m4->edges[*i]).from_vertex;
	vidx2 = vidx1 + 1;
	if(vidx2 / 4 != vidx1 /4)
		vidx2 -= 4;
	d1 = loop.M2_vertex_cross(vidx1);
	d2 = loop.M2_vertex_cross(vidx2);
	//d1 = true,则P边与环路M2相交
	if(d1)
		return -1;
	//d2 = true,则P边与环路M2不相交但在mesh上相交
	if(d2)
		rd = 1;

	for(; i !=pedge.support.end();++i)
	{
		vidx1 = (P->m4->edges[*i]).to_vertex;
		vidx2 = vidx1 + 1;
		if(vidx2 / 4 != vidx1 /4)
			vidx2 -= 4;
		d1 = loop.M2_vertex_cross(vidx1);
		d2 = loop.M2_vertex_cross(vidx2);
		//d1 = true,则P边与环路M2相交
		if(d1)
			return -1;
		//d2 = true,则P边与环路M2不相交但在mesh上相交
		if(d2)
			rd = 1;
	}
	return rd;
}

int Propagation::Pedge_M2_cross(P_edge &pedge)
{
	int vidx = pedge.to_vertice;
	int d, rd;
	int j;
	j = 0;
	rd = 0;
	for(auto loop = existingLoops.begin(); loop != existingLoops.end(); ++loop, ++j)
	{
		d = Pedge_M2_cross(pedge, *loop);
		//若M2不相交但在mesh上相交，则记录该dualloop的标号
		if(d == 1)
			rd = j+1;
		//
		if(d == -1)
			return -1;
	}
	return rd;
}

int Propagation::Pedge_M2_cross(P_edge &pedge, Labels &flags)
{
	int vidx = pedge.to_vertice;
	int d, rd;
	int dualloop_idx;
	int flag;
	int crossed;
	promoteLabel label;
	dualloop_idx = 0;
	rd = 0;
	for(auto loop = existingLoops.begin(); loop != existingLoops.end(); ++loop, ++dualloop_idx)
	{
		d = Pedge_M2_cross(pedge, *loop);

		//若与某条dualloop在mesh上相交但不在M2上相交
		if(d == 1)
		{
			rd = dualloop_idx + 1;
		}
		//若与某条dualloop在M2相交，或者和该边相交过则判断是穿越还是贴合
		label.dualloop_idx = dualloop_idx;
		auto label_iter = flags.find(label);
		if(d == -1 || label_iter != flags.end())
		{
			if(label_iter != flags.end())
			{
				//如果保存了关于该dualloop的flag的信息，则使用该flag
				flag = label_iter->flag;
			}
			else
			{
				flag = 0;
			}

			crossed = get_flags(pedge, *loop, flag);
			//crossed == -1表示穿越dualloop，该P边不合法
			if(crossed == -1)
			{
				return -1;
			}
			else
			{
				if(flag != 0 && label_iter == flags.end())
				{
					//flag不为0则增加一条记录
					label.set(dualloop_idx, flag);
					flags.insert(label);
				}
			}
		}
	}
	return rd;
}

int Propagation::Pedge_M2_cross(P_edge &pedge, std::vector<DualLoop> &featureCurves)
{
	int d, rd;
	int j = 0;
	for(auto loop = featureCurves.begin(); loop != featureCurves.end(); ++loop, ++j)
	{
		d = Pedge_M2_cross(pedge, *loop);
		//若M2不相交但在mesh上相交，则记录该dualloop的标号
		if(d == 1)
			rd = j+1;
		//
		if(d == -1)
			return -1;
	}
	return rd;
}

bool Propagation::M4vertex_loops_cross(int vidx)
{
	for(auto eloop = existingLoops.begin(); eloop != existingLoops.end(); ++eloop)
	{
		if(eloop->M2_vertex_cross(vidx))
		{
			return true;
		}
	}
	return false;
}

int Propagation::get_flags(P_edge &pedge, DualLoop &loop, int &flag)
{
	size_t halfedge_idx, op_idx;
	size_t edge_idx;
	OmHaEgH hf_handle;
	OmEgH eh_handle;
	for(auto i = pedge.support.begin(); i !=pedge.support.end();++i)
	{
		halfedge_idx = P->m4->edges[*i].meshIdx;
		hf_handle = P->mesh->halfedge_handle(halfedge_idx);
		eh_handle = P->mesh->edge_handle(hf_handle);
		edge_idx = eh_handle.idx();

		if(loop.in_left(edge_idx))
		{
			if(flag == -1)
				return -1;
			flag = 1;
		}
		else
			if(loop.in_right(edge_idx))
			{
				if(flag == 1)
					return -1;
				flag = -1;
			}
	}
	return 1;
}

void Propagation::add2existingloops(DualLoop &s)
{
	if(!s.whiskers_exist())
		s.getWhiskers(P->mesh);
	existingLoops.push_back(s);
}

void Propagation::read_from_file(std::string filename)
{
	int n_loops;
	std::ifstream fin(filename);
	fin>>n_loops;
	for(int i = 0;i < n_loops; ++i)
	{
		DualLoop loop;
		loop.read_from_file(fin, P);
		existingLoops.push_back(loop);
	}
}

void Propagation::write_to_file(std::string filename)
{
	std::ofstream fout(filename);
	fout<<existingLoops.size()<<std::endl;
	for(auto iter = existingLoops.begin(); iter != existingLoops.end(); ++iter)
	{
		iter->write_to_file(fout);
	}     
}

bool Propagation::loop_cross_type(int loop1_idx, int loop2_idx)
{
	DualLoop &dl1 = existingLoops[loop1_idx];
	DualLoop &dl2 = existingLoops[loop2_idx];
	int v1_idx, v2_idx;
	bool not_crossed = true;
	for(auto viter = dl1.M4_vertex_begin(); viter != dl1.M4_vertex_end(); ++viter)
	{
		v1_idx = *viter + 1;
		if(v1_idx / 4 != (*viter) / 4)
			v1_idx -= 4;
		if(dl2.M4_vertex_cross(*viter))
			return false;
		if(dl2.M4_vertex_cross(v1_idx))
		{
			not_crossed = false;
		}
	}
	return !not_crossed;
}

//判断一条dualloop是否是冗余的
bool Propagation::dualloop_is_redundant(int dualloop_idx)
{
	DualLoop &loop = existingLoops[dualloop_idx];
	return false;
}