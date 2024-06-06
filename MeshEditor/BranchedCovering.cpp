#include "StdAfx.h"
#include "BranchedCovering.h"
#include<stack>
#include<iostream>
#include<queue>

void surr_ds::cal_dr(OmVeH mesh_v, OP_Mesh *pr_mesh, Cross_Field *cf)
{
	int jps, dr, fidx;
	OmFaH fa;
	OmHaEgH hfe;
	
	//
	dr = 0;
	for(auto iter = pr_mesh->voh_begin(mesh_v);iter.is_valid();++iter)
	{
		hfe = *iter;
		fa = pr_mesh->face_handle(hfe);
		face_dr[fa.idx()] = dr;
		jps = (4 - cf->pjs[hfe.idx()]) % 4;
		dr = (dr + jps) % 4;
	}
	
	/*if(dr != 0)
	{
		std::cout<<"dr != 0  "<<mesh_v.idx()<<std::endl;
	}*/
}

BCtools::BCtools(OP_Mesh *mesh, Cross_Field *crossfield)
{
	pr_mesh = mesh;
	cf = crossfield;
	v_dr = new surr_ds[pr_mesh->n_vertices()];
	for(auto iter = pr_mesh->vertices_begin();iter != pr_mesh->vertices_end();++iter)
	{
		v_dr[(*iter).idx()].cal_dr(*iter, mesh, crossfield);
	}
}

int BCtools::ds(OmVeH s, OmFaH fa)
{
	return v_dr[s.idx()].face_dr[fa.idx()];
}

CoveringNode::CoveringNode()
{
	edge_begin = -1;
	edge_end = -1;
	meshIdx = -1;
	layer = -1;
}

CoveringNode::CoveringNode(int _meshIdx, int _Fd)
{
	edge_begin = -1;
	edge_end = -1;
	meshIdx = _meshIdx;
	layer = _Fd;
}

CoveringEdge::CoveringEdge()
{
	meshIdx = -1;
	next = -1;
}

CoveringSurface::CoveringSurface(OP_Mesh *mesh, Cross_Field *crossfield, std::vector<OmVeH> s)
{
	this->cf = crossfield;
	this->pr_mesh = mesh;

	pr_mesh->request_vertex_normals();
	pr_mesh->request_face_normals();
	pr_mesh->update_normals();

	singularities = new bool[pr_mesh->n_vertices()];
	for(int i = 0;i < pr_mesh->n_vertices();i++)
		singularities[i] = false;
	for(auto i = s.begin();i != s.end();++i)
	{
		singularities[(*i).idx()] = true;
	}
	vertices.clear();
	edges.clear();

	bct = new BCtools(pr_mesh, cf);
}

int CoveringSurface::add_edge(int from, int to, OmHaEgH hfe)
{
	int n = edges.size();
	CoveringEdge *ne;
	CoveringEdge *laste;
	CoveringNode *v1;
	v1 = &(vertices[from]);
	//
	ne = new CoveringEdge();
	ne->meshIdx = hfe.idx();
	ne->e = meshtools::getVector(hfe, pr_mesh);
	ne->to_vertex = to;
	ne->from_vertex = from;
	edges.push_back(*ne);

	if(v1->edge_begin == -1)
	{
		v1->edge_begin = n;
		v1->edge_end = n;
	}
	else
	{
		laste = &(edges[v1->edge_end]);
		laste->next = n;
		v1->edge_end = n;
	}
	return n;
}

int M4::mesh2covering(int idx, int layer)
{
	return idx * 4 + layer;
}
int M4::covering2mesh(int idx)
{
	return idx / 4;
}

void M4::setHoopsshow(hoopsshow *hs_)
{
	hs = hs_;
}

void M4::show_edge(int eidx, int color)
{
	OmHaEgH hfe;
	int meshIdx;
	hfe = pr_mesh->halfedge_handle(edges[eidx].meshIdx);
	hs->show_edge(hfe, color);
}

void M4::show_edge(std::vector<int> &s, int color)
{
	OmHaEgH hfe;
	int meshIdx;
	for(auto eidx = s.begin(); eidx != s.end();++eidx)
	{
		hfe = pr_mesh->halfedge_handle(edges[*eidx].meshIdx);
		hs->show_edge(hfe, color);
	}
}

void M4::show_vertex(int idx, int color)
{
	OmVeH v;
	v = pr_mesh->vertex_handle(vertices[idx].meshIdx);
	hs->show_dot(pr_mesh->point(v), color);
}

hoopsshow* M4::getHS()
{
	return hs;
}

M4::M4(OP_Mesh *mesh, Cross_Field *crossfield, std::vector<OmVeH> s) : CoveringSurface(mesh, crossfield, s)
{

}

int M4::find_layer(OmHaEgH hfe, int layer)
{
	OmVeH start, desti;
	OmFaH fa;
	int v1, v2;
	int d1, d2;

	start = pr_mesh->from_vertex_handle(hfe);
	v1 = start.idx();
	desti = pr_mesh->to_vertex_handle(hfe);
	v2 = desti.idx();
	fa = pr_mesh->face_handle(hfe);
	d1 = bct->ds(start, fa);
	d2 = bct->ds(desti, fa);
	return (layer + d1 - d2 + 4) % 4;
}

VECTOR M4::face_ds(OmFaH fa, int r)
{
	double pi = 3.1415926;
	double theta;
	VECTOR d0, norm, ret;
	OmHaEgH hfe;
	hfe = pr_mesh->halfedge_handle(cf->ds[fa.idx()]);
	theta = cf->thetas[fa.idx()];
	d0 = meshtools::getVector(hfe, pr_mesh);
	norm = pr_mesh->normal(fa);
	ret = meshtools::rotation(norm, theta + pi * r / 2, d0);
	meshtools::standalize(ret);
	return ret;
}

VECTOR M4::vertex_ds(OmVeH ve, int layer)
{
	OmHaEgH hfe_handle = pr_mesh->voh_begin(ve);
	OmFaH fa_handle = pr_mesh->face_handle(hfe_handle);
	return face_ds(fa_handle, layer);
}

VECTOR M4::edge_Fd(OmHaEgH hfe, int layer)
{
	int idx;
	int ro1, ro2;
	OmVeH toVertex = pr_mesh->to_vertex_handle(hfe);
	OmFaH fa1, fa2;
	VECTOR u1, u2;
	
	OmVeH fromVertex = pr_mesh->from_vertex_handle(hfe);
	idx = fromVertex.idx();

	fa1 = pr_mesh->face_handle(hfe);
	fa2 = pr_mesh->face_handle(pr_mesh->opposite_halfedge_handle(hfe));
	ro1 = bct->ds(fromVertex, fa1);
	ro2 = bct->ds(fromVertex, fa2);
	u1 = this->face_ds(fa1, (ro1+layer) % 4);
	u2 = this->face_ds(fa2, (ro2+layer) % 4);
	return meshtools::unitize((u1 + u2) / 2);
}

int M4::n_vertices()
{
	return vertices.size();
}

int M4::reverse_node(int idx)
{
	int ret = idx + 2;
	if((ret % 4) != (idx % 4))
		ret -= 4;
	return ret;
}

int M4::edge_idx(int eidx)
{
	int meshIdx = edges[eidx].meshIdx;
	OmHaEgH hfe = pr_mesh->halfedge_handle(meshIdx);
	OmEgH e = pr_mesh->edge_handle(hfe);
	return e.idx();
}

CoveringNode& M4::operator[](int i)
{
	return vertices[i];
}



const CoveringNode& M4::operator[](int i) const
{
	return vertices[i];
}

void M4::construct()
{
	int mesh_idx;
	int layer, layer2;
	int node2, new_edge;
	CoveringNode *p;
	OmVeH v1, v2;
	VECTOR norm;
	VECTOR Fd_e, e;
	double elength, scalar;
	OmHaEgH hfe;
	for(int i = 0;i < pr_mesh->n_vertices();i++)
	{
		v1 = pr_mesh->vertex_handle(i);
		norm = pr_mesh->normal(v1);
		for(int j = 0; j < 4;j++)
		{
			p = new CoveringNode(i, j);
			p->orth = norm;
			vertices.push_back(*p);
		}
	}
	for(int sv = 0; sv < vertices.size();++sv)
	{
		mesh_idx = covering2mesh(sv);
		if(singularities[mesh_idx] == true) 
			continue;
		v1 = pr_mesh->vertex_handle(mesh_idx);
		for(auto vit = pr_mesh->voh_begin(v1);vit.is_valid();++vit)
		{
			hfe = *vit;
			v2 = pr_mesh->to_vertex_handle(hfe);
			if(singularities[v2.idx()])
				continue;
			//确定应该连接v2哪一层上的点
			layer2 = find_layer(hfe, vertices[sv].layer);
			node2 = mesh2covering(v2.idx(), layer2);
			//Fd(e) 和 e 方向相反的则不连接边
			Fd_e = edge_Fd(hfe, vertices[sv].layer);
			e = meshtools::getVector(hfe, pr_mesh);
			elength = meshtools::length(e);
			scalar = meshtools::scalar(Fd_e, e);
			if(scalar < -0.14)
				continue;

			//
			new_edge = add_edge(sv, node2, hfe);
			edges[new_edge].Fd = Fd_e;
		}
	}
}

VECTOR M4::point(int idx)
{
	int meshidx = covering2mesh(idx);
	return pr_mesh->point(pr_mesh->vertex_handle(meshidx));
}

void M4::test()
{
	int vn = 37;
	CoveringNode *cn;
	cn = &(vertices[vn]);
	OmFaH fa;
	OmHaEgH hfe;
	VECTOR ds;
	int layer = 0;
	int r;
	int cnt = 0;
	std::cout<<"meshidx: "<<cn->meshIdx<<std::endl;
	OmVeH v = pr_mesh->vertex_handle(cn->meshIdx);
	
	/*for(auto iter = pr_mesh->voh_begin(v);iter.is_valid();++iter)
	{
		cnt++;
		
		hfe = *iter;
		fa = pr_mesh->face_handle(hfe);
		r = bct->ds(v, fa);
		std::cout<<"pjs: "<<cf->pjs[hfe.idx()]<<"  r: "<<r<<std::endl;
		ds = face_ds(fa, r);
		if(cnt)
		{
			hs->show_edge(hfe, cnt);
		}
		hs->show_face(fa, ds, 1);
	}*/

	int vidx = 0;
	CoveringNode *p;
	CoveringEdge *e;
	int eidx;
	std::queue<int> que;
	bool *visited;

	visited = new bool[n_vertices()];
	
	for(int i = 0;i < n_vertices();++i)
	{
		visited[i] = false;
	}
	while(!que.empty()) que.pop();
	que.push(vn);
	visited[vn] = true;
	while(!que.empty())
	{
		if(cnt > 300)
			break;

		vidx = que.front();
		que.pop();
		p = &(vertices[vidx]);
		eidx = p->edge_begin;
		
		while(eidx != -1)
		{
			std::cout<<"eidx: "<<eidx<<std::endl;
			e = &(edges[eidx]);
			hs->show_edge(pr_mesh->halfedge_handle(e->meshIdx), 0);
			hs->show_edge_Fd(pr_mesh->halfedge_handle(e->meshIdx), e->Fd, 3);
			if(!visited[e->to_vertex])
			{
					visited[e->to_vertex] = true;
					que.push(e->to_vertex);
					cnt++;
			}
			eidx = e->next;
			//break;
		}
	}
}

dijkstraOnCS::dijkstraOnCS(CoveringSurface *branchedcovering)
{
	cs = branchedcovering;
	maxv = cs->vertices.size();
	maxe = cs->edges.size();
	/*list = new double[maxv];
	e_records = new int[maxv];
	v_records = new int[maxv];
	dk = new int[maxv];
	length = new double[maxe];*/
	for(int i = 0;i < maxe;++i)
	{
		length[i] = meshtools::length(cs->edges[i].e);
	}
}

void dijkstraOnCS::clear()
{
	for(int i = 0;i < maxv;i++)
	{
		list[i] = -1;
		dk[i] = -1;
		e_records[i] = -1;
		v_records[i] = -1;
	}
}

void dijkstraOnCS::dijkstra(int start_v, int threshold, std::vector<int> &k_vert)
{
	int cnt = 0;
	int v, v2;
	bool inS[max_vertice];
	double vlen, elen;
	baseHeap heap;
	CoveringNode *ver;
	CoveringEdge *e;
	
	clear();
	k_vert.clear();;
	for(int i = 0;i < maxv;i++)
	{
		inS[i] = false;
	}
	
	dk[start_v] = 0;
	list[start_v] = 0.0;
	heap.push(start_v, list[start_v]);
	for(v = heap.pop(); v != -1; v = heap.pop())
	{
		if(inS[v])
			continue;
		inS[v] = true;
		//离散距离为1的点在1ring函数中已经构造出了
		if(dk[v] > 1 && dk[v] <= threshold)
		{
			k_vert.push_back(v);
		}
			
		vlen = list[v];
		ver = &(cs->vertices[v]);
		for(int j = ver->edge_begin;j != -1;j = cs->edges[j].next)
		{
			e = &(cs->edges[j]);
			v2 = e->to_vertex;
			elen = length[j];
			if(!inS[v2] && ((list[v2] < 0)  || (vlen + elen < list[v2])))
			{
				list[v2] = vlen + elen;
				dk[v2] = dk[v] + 1;
				e_records[v2] = j;
				v_records[v2] = v;
				if(dk[v2] <= threshold)
				{
					heap.push(v2, list[v2]);
				}
			}
		}
	}
}