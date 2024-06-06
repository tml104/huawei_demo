#include "StdAfx.h"
#include <math.h>
#include <iostream>
#include "MyHeap.h"


/*
***********************合并集*********************************************
*/
Con_set::Con_set(int maxn)
{
	_maxn = maxn;
	for(int i = 0;i < maxn;i++)
		_set.push_back(i);
}
int Con_set::idx(int x)
{
	int ret = x;
	while(_set[ret] != ret)
	{
		ret = _set[ret];
	}
	_set[x] = ret;
	return ret;
}
void Con_set::connect(int x, int y)
{
	int i_x = idx(x);
	int i_y = idx(y);
	_set[i_x] = i_y;
}

/*
**********************绘图类*******************************************************
*/
hoopsshow::hoopsshow(OP_Mesh *mesh, HoopsView *hoopsview)
{
	pr_mesh = mesh;
	my_hoops_view = hoopsview;
}

void hoopsshow::show_dot(VECTOR p, int color)
{
	OvmVec3d p1(p[0], p[1], p[2]);
	my_hoops_view->render_point(p1);
}

void hoopsshow::show_edge(OmHaEgH hf_handle, int color)
{
	OP_Mesh::Point p1,p2;
	p1 = pr_mesh->point(pr_mesh->to_vertex_handle(hf_handle));
	p2 = pr_mesh->point(pr_mesh->from_vertex_handle(hf_handle));
	OvmVec3d p3(p1[0],p1[1],p1[2]);
	OvmVec3d p4(p2[0],p2[1],p2[2]);
	my_hoops_view->render_streamline(p3, p4, color);
}

void hoopsshow::show_edge(OmEgH e_handle, int color)
{
	OP_Mesh::Point p1,p2;
	OmHaEgH hf_handle;
	hf_handle = pr_mesh->halfedge_handle(e_handle,0);
	p1 = pr_mesh->point(pr_mesh->to_vertex_handle(hf_handle));
	p2 = pr_mesh->point(pr_mesh->from_vertex_handle(hf_handle));
	OvmVec3d p3(p1[0],p1[1],p1[2]);
	OvmVec3d p4(p2[0],p2[1],p2[2]);
	//if(color <= 3 && color >= 1)
	my_hoops_view->render_streamline(p3, p4, color);
}

void hoopsshow::show_edges(std::vector<OmHaEgH> es, int color)
{
	for(auto i = es.begin(); i != es.end(); ++i)
	{
		show_edge(*i, color);
	}
}

void hoopsshow::show_vector(VECTOR a, VECTOR b, int color)
{
	OvmVec3d p3(a[0], a[1], a[2]);
	OvmVec3d p4(b[0], b[1], b[2]);
	p4 += p3;
	my_hoops_view->render_streamline(p3, p4, color);
}

void hoopsshow::show_path(std::vector<OmHaEgH> &path, int color)
{
	OP_Mesh::Point p1,p2;
	OmHaEgH hf_handle;
	for(auto i  = path.begin(); i != path.end();++i)
	{
		show_edge(*i, color);
	}
}

void hoopsshow::show_face(OmFaH fa, VECTOR ds, int color)
{
	int i = 0;
	double pi = 3.1415926;
	OmVeH v[3];
	VECTOR p[3], centre, norm;
	norm = pr_mesh->normal(fa);
	for(auto vit = pr_mesh->fv_begin(fa); vit.is_valid();++vit)
	{
		//v[i] = *vit;
		p[i] = pr_mesh->point(*vit);
		i++;
	}
	centre = (p[0] + p[1] + p[2]) / 3;
	ds = meshtools::unitize(ds);
	ds = meshtools::mul(ds, 2.0);
	show_vector(centre, ds, 3);
	ds = meshtools::rotation(norm, pi / 2, ds);
	show_vector(centre, ds, 2);
}

void hoopsshow::show_edge_Fd(OmHaEgH hfe, VECTOR Fd, int color)
{
	OmVeH v1, v2;
	VECTOR p1, p2;
	v2 = pr_mesh->to_vertex_handle(hfe);
	v1 = pr_mesh->from_vertex_handle(hfe);
	p2 = pr_mesh->point(v2);
	p1 = pr_mesh->point(v1);
	show_vector((p2+p1)/2, Fd, color);
}

/*
****************************工具函数类*******************************
*/

double meshtools::distance(OmHaEgH hf, OP_Mesh *pr_mesh)
{
	double ret;
	OP_Mesh::Point v1, v2;
	v1 = pr_mesh->point(pr_mesh->to_vertex_handle(hf));
	v2 = pr_mesh->point(pr_mesh->from_vertex_handle(hf));
	ret = (v1[0] - v2[0]) * (v1[0] - v2[0]) + (v1[1] - v2[1]) * (v1[1] - v2[1]) + (v1[2] - v2[2]) * (v1[2] - v2[2]);
	return std::sqrt(ret);
}

VECTOR meshtools::getVector(OmHaEgH hf, OP_Mesh *pr_mesh)
{
	OmVeH v1, v2;
	VECTOR p1, p2;
	v2 = pr_mesh->to_vertex_handle(hf);
	v1 = pr_mesh->from_vertex_handle(hf);
	p1 = pr_mesh->point(v1);
	p2 = pr_mesh->point(v2);
	return p2 - p1;
}

void meshtools::standalize(VECTOR &v)
{
	double eps = 1e-6;
	for(int i = 0;i < 3;i++)
	{
		if(v[i] < eps && v[i] > -eps)
			v[i] = 0.0;
	}
}

double meshtools::scalar(VECTOR &a, VECTOR &b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] ;
}
double meshtools::length(VECTOR &a)
{
	double ret =  std::sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	if(ret < 1e-6)
		return 0.0;
	else
		return ret;
}
VECTOR meshtools::unitize(VECTOR &x)
{
	double eps = 1e-6;
	double L = x[0] * x[0] + x[1] * x[1] + x[2] * x[2] ;
	if(L < eps)
	{
		return VECTOR(0.0, 0.0, 0.0);
	}
	else
	{
		L = std::sqrt(L);
		return VECTOR(x[0]/L, x[1]/L, x[2]/L);
	}
}
VECTOR meshtools::mul(VECTOR &x, double k)
{
	VECTOR e(x[0] * k, x[1] * k, x[2] * k);
	standalize(e);
	return e;
}
VECTOR meshtools::tensor_production(VECTOR &a, VECTOR &b)
{
	return VECTOR(a[1]*b[2] - a[2]*b[1], a[2]*b[0] - a[0]*b[2], a[0]*b[1] - a[1]*b[0]);
}
VECTOR meshtools::rot_x(VECTOR &v, double theta)
{
	double costh, sinth;
	costh  = std::cos(theta);
	sinth  = std::sin(theta);
	return VECTOR(v[0], v[1] * costh - v[2] * sinth, v[1] * sinth + v[2] * costh);
}
VECTOR meshtools::rot_y(VECTOR &v, double theta)
{
	double costh, sinth;
	costh  = std::cos(theta);
	sinth  = std::sin(theta);
	return VECTOR(v[0] * costh - v[2] * sinth, v[1], v[0] * sinth + v[2] * costh);
}
VECTOR meshtools::rot_z(VECTOR &v, double theta)
{
	double costh, sinth;
	costh  = std::cos(theta);
	sinth  = std::sin(theta);
	return VECTOR(v[0] * costh - v[1] * sinth, v[0] * sinth + v[1] * costh, v[2]);
}

double meshtools::acos(VECTOR &a, VECTOR &b)
{
	double eps = 1e-6;
	double la, lb, cab, theta;
	la = length(a);
	lb = length(b);
	cab = scalar(a, b);
	if(la < eps || lb < eps)
		return 0.0;
	theta = std::acos(cab / (la * lb));
	return theta;
}

VECTOR meshtools::rotation(VECTOR a, VECTOR b, VECTOR tar)
{
	double eps = 1e-6;
	double fai1, fai2, theta, eta;
	double La, Lb, Ln_xy;
	VECTOR tmp1, tmp2, ret;
	VECTOR norm = unitize(tensor_production(b, a));

	//若a，b平行，同向时ret不变，反向时ret也反向
	if(length(norm) < eps)
	{
		if(scalar(a, b) < 0)
		{
			return VECTOR(-tar[0], -tar[1], -tar[2]);
		}
		else
		{
			return tar;
		}
	}
	//
	La = length(a);
	Lb = length(b);
	Ln_xy = std::sqrt(norm[0] * norm[0] + norm[1] * norm[1]);
	if(La < eps || Lb < eps)
	{
		return VECTOR(0,0,0);
	}
	//ψ1表示旋转轴在xOy平面的投影与x轴的夹角，ψ2表示旋转轴和xOy平面的夹角
	if(Ln_xy < eps)
		fai1 = 0.0;
	else
	{
		/*fai1 = std::acos(norm[0] / Ln_xy);
		if(norm[1] < 0)
			fai1 = -fai1;*/
		fai1 = std::atan2(norm[1], norm[0]);
	}
	fai2 = std::asin(norm[2] / std::sqrt(norm[0] * norm[0] + norm[1] * norm[1] + norm[2] * norm[2]));
	//
	theta = std::acos(scalar(a, b) / (La * Lb));
	tmp1 = rot_z(tar, -fai1);//
	tmp2 = rot_y(tmp1, -fai2);
	tmp1 = rot_x(tmp2, theta);
	tmp2 = rot_y(tmp1, fai2);
	ret = rot_z(tmp2, fai1);
	standalize(ret);
	return ret;

}

VECTOR meshtools::rotation(VECTOR norm, double theta, VECTOR tar)
{
	//double eps = 1e-6;
	//double fai1, fai2;
	//double Ln_xy;
	//VECTOR tmp1, tmp2, ret;
	//Ln_xy = std::sqrt(norm[0] * norm[0] + norm[1] * norm[1]);
	////ψ1表示旋转轴在xOy平面的投影与x轴的夹角，ψ2表示旋转轴和xOy平面的夹角
	//if(Ln_xy < eps)
	//	fai1 = 0.0;
	//else
	//{
	//	fai1 = std::acos(norm[0] / Ln_xy);
	//	if(norm[1] < 0)
	//		fai1 = -fai1;
	//}
	//fai2 = std::asin(norm[2] / std::sqrt(norm[0] * norm[0] + norm[1] * norm[1] + norm[2] * norm[2]));
	//tmp1 = rot_z(tar, -fai1);//
	//tmp2 = rot_y(tmp1, -fai2);
	//tmp1 = rot_x(tmp2, theta);
	//tmp2 = rot_y(tmp1, fai2);
	//ret = rot_z(tmp2, fai1);
	//standalize(ret);
	//return ret;

	OvmVec3d fnormal(norm[0],norm[1],norm[2]);
	OvmVec3d reference_vec(tar[0],tar[1],tar[2]);
	OvmVec3d result_vec;
	from_theta2vector_respecting_reference_vec(theta,reference_vec,fnormal,result_vec);
	VECTOR ret(result_vec[0],result_vec[1],result_vec[2]);
	return ret;
}

VECTOR meshtools::project(VECTOR &norm, VECTOR &obj)
{
	double eps = 1e-6;
	VECTOR v1, v2, ret;
	double k, t, theta;
	k = scalar(norm, obj);
	/*if(k < eps && k > -eps)
		return obj;*/
	t = length(obj);
	v1 = mul(norm, k);
	v2 = unitize(obj - v1);
	ret = mul(v2, t);
	return ret;
}

double meshtools::w_e(VECTOR e, VECTOR Fd_e, double c_alpha)
{
	double t1, t2, t3, correction;
	//t2 = e*e
	t2 = scalar(e, e);
	//e * Fd
	t1 = scalar(e, Fd_e);
	//当e和Fd_e交角大于90度要进行修正
	if(t1 < 0)
	{
		correction = c_alpha * c_alpha * t2;
	}
	else
	{
		correction = 0;
	}
	//t1 = (e*Fd)^2
	t1 = t1 * t1;
	
	//t3 = α^2 * (e*e - (e*Fd)^2)
	t3 = (t2 - t1) * c_alpha * c_alpha;
	return std::sqrt(t1 + t3 + correction);
}

double meshtools::w_e(VECTOR norm, VECTOR e, VECTOR Fd_e, double c_alpha)
{
	e = project(norm, e);
	Fd_e = project(norm, Fd_e);
	return w_e(e, Fd_e, c_alpha);
}

//若a在b左侧，则返回-1，右侧返回1
int meshtools::side(VECTOR a, VECTOR b, VECTOR norm)
{
	VECTOR c = tensor_production(a, b);
	if(scalar(c, norm) < 0)
		return -1;
	else
		return 1;
}

VECTOR meshtools::rev(VECTOR x)
{
	return VECTOR(-x[0], -x[1], -x[2]);
}

/*
**********最小堆*************************
*/

baseHeap::baseHeap()
{
	num = 0;
	/*max_n = n;
	arr = new HeapNode[n+1];*/
}

void baseHeap::swap(HeapNode &x, HeapNode &y)
{
	int tmp1;
	double tmp2;
	tmp1 = x.idx;
	x.idx = y.idx;
	y.idx = tmp1;
	tmp2 = x.value;
	x.value = y.value;
	y.value = tmp2;
}

bool baseHeap::cmp(HeapNode &x, HeapNode &y)
{
	return x.value < y.value;
}

void baseHeap::push(int idx, double value)
{
	if(num > max_n)
		return ;
	num++;
	arr[num].idx = idx;
	arr[num].value = value;
	int i = num, j;
	while(i > 1)
	{
		j = i / 2;
		//if value(i) < value(j)
		if(cmp(arr[i], arr[j]))
		{
			swap(arr[i], arr[j]);
		}
		i = j;
	}
}

int baseHeap::pop()
{
	if(num <= 0)
		return -1;
	int i, j, k, tmp;
	int ret = arr[1].idx;
	arr[1] = arr[num];
	num--;
	i = 1;
	while(i < num)
	{
		tmp = i;
		j = i * 2;
		k = j + 1;
		if(j <= num && cmp(arr[j], arr[i]))
		{
			i = j;
		}
		if(k <= num && cmp(arr[k], arr[i]))
		{
			i = k;
		}
		if(i == tmp)
			break;
		swap(arr[tmp], arr[i]);
	}
	return ret;
}

int baseHeap::size()
{
	return num;
}

void baseHeap::clear()
{
	num = 0;
}

/*
***************最大堆***************************************
*/

bool maxHeap::cmp(HeapNode &x, HeapNode &y)
{
	return x.value > y.value;
}

/*
********************最短路算法类********************************
*/

dijkPath::dijkPath(OmVeH source_vertex, OP_Mesh *mesh)
{
	pr_mesh = mesh;
	sv = source_vertex;
	_maxv = mesh->n_vertices();
	_maxe = mesh->n_edges();
	OmEgH eh = mesh->edge_handle(0);
	OmHaEgH heh = mesh->halfedge_handle(0);
	list  = new double[_maxv];
	dk  = new int[_maxv];
	e_records = new OmHaEgH[_maxv];
	v_records = new OmVeH[_maxv];
	for(int i = 0;i < _maxv;i++)
	{
		list[i] = -1.0;
		dk[i] = -1;
	}
}

dijkPath::~dijkPath()
{
	delete[] list;
	delete[] dk;
	delete[] e_records;
	delete[] v_records;
}

void dijkPath::setHoopsshow(hoopsshow *hs_)
{
	hs = hs_;
}

void dijkPath::showPath(int color)
{
	OmVeH v;
	OmHaEgH e;
	if(hs != NULL)
	{
		for(auto iter = pr_mesh->vertices_begin(); iter != pr_mesh->vertices_end();++iter)
		{
			v = *iter;
			if(v != sv)
			{
				e = e_records[v.idx()];
				hs->show_edge(e, color);
			}
		}
	}
}

void dijkPath::dijkstra(double *edge_length)
{
	bool  *in_S;
	in_S = new bool[_maxv];
	double elen, vlen;
	int iter, v2;
	OmVeH v_handle, v2_handle;
	OmEgH e_handle;
	baseHeap heap_min;

	for(int i = 0;i < pr_mesh->n_vertices();i++)
	{
		in_S[i] = false;
		list[i] = -1;
	}
	list[sv.idx()] = 0;
	//入堆
	heap_min.push(sv.idx(), 0.0);
	for(iter = heap_min.pop();iter != -1;iter = heap_min.pop())
	{
		if(in_S[iter])
			continue;
		vlen = list[iter];
		v_handle = pr_mesh->vertex_handle(iter);
		//将顶点加入集合S
		in_S[iter] = true;
		//更新list
		//遍历从v_handle出发的半边
		for(auto e_it = pr_mesh->voh_begin(v_handle);e_it.is_valid();++e_it)
		{
			v2_handle = pr_mesh->to_vertex_handle(*e_it);
			v2 = v2_handle.idx();
			e_handle = pr_mesh->edge_handle(*e_it);
			elen = edge_length[e_handle.idx()];//边的长度
			//更新list
			if(!in_S[v2] && ((list[v2] < 0)  || (vlen + elen < list[v2])))
			{
				list[v2] = vlen + elen;//更新list
				heap_min.push(v2, list[v2]);//顶点入堆
				e_records[v2] = *e_it;//通向iter的最短路包含了e
				v_records[v2] = v_handle;
			}
		}
	}
}

void dijkPath::dijkstra(double *edge_length, std::vector<OmVeH> &k_vert, bool *illegal, int thrall)
{
	bool  *in_S;
	in_S = new bool[_maxv];
	//dk = new int[_maxv];
	double vlen;
	double edge_len;
	int iter, v2;
	OmVeH v_handle;
	OmEgH e_handle;
	//Heap heap_min(list, _maxv, false);
	baseHeap heap_min;

	//初始化
	for(int i = 0;i < pr_mesh->n_vertices();i++)
	{
		in_S[i] = false;
		//dk[i] = -1;
		list[i] = -1.0;
	}
	list[sv.idx()] = 0;
	dk[sv.idx()] = 0;
	//源点入堆
	heap_min.push(sv.idx(), 0);
	for(int i = 1;i <= pr_mesh->n_vertices();i++)
	{
		//推排序找出特殊路径最短的顶点
		for(iter = heap_min.pop();iter != -1 && in_S[iter];iter = heap_min.pop());
		if(iter == -1) break;
		vlen = list[iter];
		v_handle = pr_mesh->vertex_handle(iter);
		//将顶点加入集合S
		in_S[v_handle.idx()] = true;
		//将顶点保存
		k_vert.push_back(v_handle);
		//更新list
		//遍历从v_handle出发的半边
		for(auto e_it = pr_mesh->voh_begin(v_handle);e_it.is_valid();++e_it)
		{
			v2 = (pr_mesh->to_vertex_handle(*e_it)).idx();//该半边指向的顶点的下标

			if(illegal[v2]) continue;//若该点是一个奇点则跳过

			e_handle = pr_mesh->edge_handle(*e_it);//该半边所在的边
			edge_len = edge_length[e_handle.idx()];//边的长度
			//更新list
			if(!in_S[iter] && ((list[v2] < 0)  || (vlen + edge_len < list[v2])))
			{
				list[v2] = vlen + edge_len;//更新list
				dk[v2] = dk[iter] + 1;
				if(dk[iter] <= thrall)
				{
					heap_min.push(v2, list[v2]);//顶点入堆
					e_records[iter] = (*e_it);//记录半边
					v_records[v2] = v_handle;
				}		
			}
		}
	}
}

void dijkPath::path(OmVeH des, std::vector<OmHaEgH> &route)
{
	int iter;
	route.clear();
	OmVeH v_handle = des;
	while(v_handle != sv)
	{
		iter = v_handle.idx();
		route.push_back(e_records[iter]);
		v_handle = v_records[iter];
	}
}

/*
**********************SI集合类****************************************
*/

void Primal_Separatrice::add_edge(int idx, OP_Mesh *mesh)
{
	OmHaEgH hfe = mesh->halfedge_handle(idx);
	edges.push_back(hfe);
}

//初始化stack
void Primal_Separatrice::stack_inital()
{
	while(!sta.empty()) sta.pop();
}
//将一条半边的idx入栈
void Primal_Separatrice::stack_push(int eidx)
{
	sta.push(eidx);
}

void Primal_Separatrice::stack_push(std::stack<int> &arg)
{
	while(!arg.empty())
	{
		sta.push(arg.top());
		arg.pop();
	}
}

//将栈中的边加入edges
void Primal_Separatrice::stack2edges(OP_Mesh *mesh)
{
	int eidx;
	while(!sta.empty())
	{
		eidx = sta.top();
		sta.pop();
		edges.push_back(mesh->halfedge_handle(eidx));
	}
}

void Primal_Separatrice::show_path(hoopsshow *hs, int color)
{
	for(auto iter = edges.begin(); iter != edges.end(); ++iter)
	{
		hs->show_edge(*iter, color);
	}
}

/*
************************promoteLabel****************************
*/
void promoteLabel::set(int _dualloop_idx, int _flag)
{
	dualloop_idx = _dualloop_idx;
	flag = _flag;
}

promoteLabel::promoteLabel()
{
	dualloop_idx = -1;
	flag = 0;
}
promoteLabel::promoteLabel(int idx)
{
	dualloop_idx = idx;
	flag = 0;
}

bool promoteLabel::operator<(const promoteLabel &y) const
{
	return this->dualloop_idx < y.dualloop_idx;
}

pHeapNode::pHeapNode()
{
	idx = -1;
	flags.clear();
}

promoteHeap::promoteHeap()
{
	num = 0;
}

void promoteHeap::swap(pHeapNode &x, pHeapNode &y)
{
	/*int tmp1;
	double tmp2;
	tmp1 = x.idx;
	x.idx = y.idx;
	y.idx = tmp1;
	tmp2 = x.value;
	x.value = y.value;
	y.value = tmp2;*/
	pHeapNode tmp;
	tmp = x;
	x = y;
	y = tmp;
}

bool promoteHeap::cmp(pHeapNode &x, pHeapNode &y)
{
	return x.value < y.value;
}

void promoteHeap::push(int idx, double value, Labels flags)
{
	if(num > max_n)
		return ;
	num++;
	arr[num].idx = idx;
	arr[num].value = value;
	arr[num].flags = flags;
	int i = num, j;
	while(i > 1)
	{
		j = i / 2;
		//if value(i) < value(j)
		if(cmp(arr[i], arr[j]))
		{
			swap(arr[i], arr[j]);
		}
		i = j;
	}
}

pHeapNode promoteHeap::pop()
{
	pHeapNode ret;
	if(num <= 0)
		return ret;
	int i, j, k, tmp;
	ret = arr[1];
	arr[1] = arr[num];
	num--;
	i = 1;
	while(i < num)
	{
		tmp = i;
		j = i * 2;
		k = j + 1;
		if(j <= num && cmp(arr[j], arr[i]))
		{
			i = j;
		}
		if(k <= num && cmp(arr[k], arr[i]))
		{
			i = k;
		}
		if(i == tmp)
			break;
		swap(arr[tmp], arr[i]);
	}
	return ret;
}

int promoteHeap::size()
{
	return num;
}

void promoteHeap::clear()
{
	num = 0;
}