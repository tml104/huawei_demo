#include "StdAfx.h"
#include "LoopConstruct.h"
#include<stack>
#include<iostream>
#include<queue>
#include<algorithm>

void LoopConstruct::show_loop(DualLoop &ss)
{
	int m4idx;
	int meshIdx;
	for(auto i = ss.M4_edge_begin(); i != ss.M4_edge_end();++i)
	{
		meshIdx = m4->edges[*i].meshIdx;
		hosh->show_edge(mesh->halfedge_handle(meshIdx), 0);
	}
}

//���ж�SI,loop�Ƿ��ཻ
bool LoopConstruct::loop_SI_cross_simple(DualLoop &loop, int si_idx, bool record)
{
	if(loop.crossed(si_idx))
		return true;
	if(loop.size() < 3)
		return false;

	int m4v;
	int cnt = 0;
	int cut_cnt = 0, left_cnt = 0, right_cnt = 0;
	int flag, cross;
	int side = 0;
	bool ret;
	size_t he_idx;
	OmHaEgH heh;
	OmEgH eh;
	SIpath &si = SIfinder->sp[si_idx];
	
	for(auto eiter = loop.M4_edge_begin(); eiter != loop.M4_edge_end(); ++eiter)
	{
		he_idx = m4->edges[*eiter].meshIdx;
		heh = mesh->halfedge_handle(he_idx);
		eh = mesh->edge_handle(heh);
		cross = si.side(eh.idx());
		cnt += cross;
		if(cross == 1)
			left_cnt ++;
		if(cross == -1)
			right_cnt++;
	}
	cut_cnt = left_cnt < right_cnt ? left_cnt : right_cnt;
	ret = (cut_cnt % 2 == 1);
	if(record && ret)
	{
		loop.cross(si_idx);
	}
	return ret;
}

//��ȷ�ж�SI,loop�Ƿ��ཻ
bool LoopConstruct::loop_SI_cross_whiskers(DualLoop &loop, int si_idx, bool record)
{
	if(loop.crossed(si_idx))
		return true;
	if(loop.size() < 3)
		return false;

	int m4v;
	int cnt = 0;
	int cut_cnt = 0;
	int flag, cross;
	int side = 0;
	bool ret;
	size_t he_idx;
	OmHaEgH heh;
	OmEgH eh;
	SIpath &si = SIfinder->sp[si_idx];
	
	for(auto eiter = loop.M4_edge_begin(); eiter != loop.M4_edge_end(); ++eiter)
	{
		he_idx = m4->edges[*eiter].meshIdx;
		heh = mesh->halfedge_handle(he_idx);
		eh = mesh->edge_handle(heh);

		cross = si.side(eh.idx());
		if(cross != 0)
		{
			//
			if(cross == cnt)
			{
				cnt = 0;
			}
			else
			{
				if(cnt != 0)
					cut_cnt++;
				cnt = cross;
			}
		}
	}
	ret = ((cnt == 0) && (cut_cnt % 2 == 1));
	if(record && ret)
	{
		loop.cross(si_idx);
	}
	return ret;
}

LoopConstruct::LoopConstruct(OP_Mesh *bs_mesh, Cross_Field* cross_field, HoopsView *hs)
{
	int n;
	VECTOR e;
	OmEgH ehandle;
	mesh = bs_mesh;
	hoopsview = hs;

	//������ص������ʼ��
	n = mesh->n_vertices();
	is_singularity = new bool[n];
	around_singularity = new int[n];

	for(int i = 0;i < n;++i)
	{
		is_singularity[i] = false;
		around_singularity[i] = -1;
	}

	//����ص������ʼ��
	n = mesh->n_edges();
	edge_block = new int[n];
	onLoops = new bool[n];
	//edge_length = new double[n];
	for(int i = 0;i < n;++i)
	{
		edge_block[i] = -1;
		onLoops[i] = false;
		//ehandle = mesh->edge_handle(i);
		//edge_length[i] = meshtools::distance(mesh->halfedge_handle(ehandle,0), mesh);
	}

	//vector ��ʼ��
	dualloops.clear();
	separatrice.clear();
	featureCurves.clear();

	mesh->request_face_normals();
	mesh->update_face_normals();

	this->cf = cross_field;
	//cf->render_singularities(hoopsview);
	cf->get_singularities(singularities);
	isolated = new bool[singularities.size()];
	//��������ڵĵ���Ϊ������idx
	n = 0;
	for(auto i = singularities.begin(); i != singularities.end();++i)
	{
		is_singularity[i->idx()] = true;
		isolated[n] = false;

		for(auto vj = mesh->vv_begin(*i); vj.is_valid(); ++vj)
		{
			around_singularity[vj->idx()] = i->idx();
		}
	}
	SIfinder = new Sep_Indicators(mesh, hoopsview);
	//ȡ������߳��ȵ�����
	edge_length = SIfinder->edgeLength();
	//����SI
	SIfinder->get_SIs(singularities);
	//SIfinder->get_whiskers();
	hosh = new hoopsshow(mesh, hoopsview);
	m4 = new M4(mesh, cf, singularities);
	m4->setHoopsshow(hosh);
	m4->construct();
	P = new ExpMap(m4, ExpMap_degree);
	P->setHoopsshow(hosh);
	P->construct();
	candidate = new Propagation(P);
}

void LoopConstruct::embedding()
{
	int mesh_i;
	OmHaEgH he;
	OmEgH e_handle;
	for(auto r = candidate->existingLoops.begin(); r != candidate->existingLoops.end();++r)
	{
		for(auto iter = r->M4_edge_begin(); iter != r->M4_edge_end();++iter)
		{
			mesh_i = m4->edges[*iter].meshIdx;
			he = mesh->halfedge_handle(mesh_i);
			e_handle = mesh->edge_handle(he);
			onLoops[e_handle.idx()] = true;
		}
	}
}

void LoopConstruct::embedding(DualLoop &loop)
{
	int mesh_i;
	OmHaEgH he;
	OmEgH e_handle;
	for(auto iter = loop.M4_edge_begin(); iter != loop.M4_edge_end();++iter)
	{
		mesh_i = m4->edges[*iter].meshIdx;
		he = mesh->halfedge_handle(mesh_i);
		e_handle = mesh->edge_handle(he);
		onLoops[e_handle.idx()] = true;
	}
}

//�����vs����ȫ�ָ������򷵻�true
bool LoopConstruct::bfs(OmVeH vs)
{
	int *v_records;
	OmHaEgH *e_records;
	std::queue<int> que;
	std::stack<OmHaEgH> sta;
	int v, ev;
	OmVeH vh, evh;
	int n = mesh->n_vertices();
	bool not_isolated = false;


	v_records = new int[n];
	e_records = new OmHaEgH[n];
	for(int i = 0;i  < n;++i)
	{
		v_records[i] = -1;
	}
	//��ն���
	while(!que.empty()) que.pop();

	v_records[vs.idx()] = vs.idx();

	que.push(vs.idx());
	while(!que.empty())
	{
		v = que.front();
		que.pop();
		vh = mesh->vertex_handle(v);
		//���������δ�ָ���
		if(is_singularity[vs.idx()] && is_singularity[v])
		{
			int vsi = vs.idx();
			int vi = v;
			SIpath sip;

			//���ջ
			while(!sta.empty()) sta.pop();

			not_isolated = true;
			while(vi != vsi)
			{
				sta.push(e_records[vi]);
				vi = v_records[vi];
			}
			if(sta.size() > 0)
			{
				while(!sta.empty())
				{
					sip.push_back(sta.top());
					sta.pop();
				}
				SIfinder->sp.push_back(sip);
			}
		}
		for(auto ei = mesh->voh_begin(vh); ei.is_valid();++ei)
		{
			//ei��ָ��ĵ��handle
			evh = mesh->to_vertex_handle(*ei);
			ev = evh.idx();
			//ev����dual loops ����δ����
			if((!onLoops[ev]) && (v_records[ev] == -1))
			{
				v_records[ev] = v;
				e_records[ev] = *ei;
				que.push(ev);
			}
		}
	}

	return !not_isolated;
}

bool LoopConstruct::post_validation()
{
	return false;
}

void LoopConstruct::process()
{
}

//�������������ı߽��б��
void LoopConstruct::primal_flood(OmVeH sv)
{
	OmVeH v, u;
	OmFaH fv,fu;
	OmHaEgH hv, hu;
	OmEgH ev, eu;
	int idx = sv.idx();

	std::queue<OmFaH> que;
	while(!que.empty()) que.pop();
	
	que.push(*(mesh->vf_begin(sv)));
	

	while(!que.empty())
	{
		fv = que.front();
		que.pop();
		//primal_node[v.idx()] = idx;
		for(auto iter = mesh->fh_begin(fv); iter.is_valid(); ++iter)
		{
			hv = *iter;
			ev = mesh->edge_handle(hv);
			//���ev���ڻ�·���һ�δ���
			if(onLoops[ev.idx()] == false && edge_block[ev.idx()] == -1)
			{
				edge_block[ev.idx()] = idx;
				//�����ڵ���Ҳ��ͬһ�����ķֿ���
				hu = mesh->opposite_halfedge_handle(hv);
				fu = mesh->face_handle(hu);
				que.push(fu);
			}
		}
	}
}

bool LoopConstruct::primal_Pedge_constraint(int eidx, int s_mesh, int last_block, int &block)
{
	int v1, v2;
	int idx1, idx2;
	int mesh_e, m4e;
	OmHaEgH hfe_handle;
	OmEgH e_handle;
	for(auto i = P->edges[eidx].support.begin(); i !=P->edges[eidx].support.end();++i)
	{
		idx1 = m4->edge_idx(*i);
		block = edge_block[idx1];
		if(block != last_block)
		{
			//���������飬�ǳ�ʼ�����Ͳ������뿪
			if(last_block != -1 && last_block != s_mesh)
			{
				return false;
			}
			last_block = block;
		}
	}
	return true;
}

void LoopConstruct::primal_promote(int s_mesh)
{
	int sv = m4->mesh2covering(s_mesh, 0);
	int m4_vidx, idx, eidx;
	int n = m4->n_vertices();
	double elen, vlen;
	OmVeH vi, vh;
	int v1, v2, v1_mesh;
	int  mesh_e, m4e;
	int block, last_block;
	int cnt;

	baseHeap heap;
	heap.clear();
	std::set<int> reached_s;
	reached_s.clear();
	std::stack<int> pstack;

	double *list;
	int *v_records;
	int *e_records;
	int *b_records;
	bool *in_S;
	list = new double[n];
	v_records = new int[n];
	e_records = new int[n];
	b_records = new int[n];
	in_S = new bool[n];
	for(int i = 0;i < n;++i)
	{
		list[i] = -1;
		in_S[i] = false;
	}

	list[sv] = 0;
	b_records[sv] = s_mesh;
	heap.push(sv, 0);

	//
	for(v1 = heap.pop(); v1 != -1; v1 = heap.pop())
	{
		if(in_S[v1])
			continue;

		in_S[v1] = true;
		vlen = list[v1];
		last_block = b_records[v1];
		v1_mesh = m4->covering2mesh(v1);
		
		if(is_singularity[v1_mesh] == true && v1_mesh != s_mesh)
		{
			//��֮ǰv1��������Ѿ�����������ظ�
			if(dealt_s.find(v1_mesh) != dealt_s.end())
				continue;

			//������һ���µķָ���
			Primal_Separatrice route;
			v2 = v1;
			while(!pstack.empty()) pstack.pop();
			while(v2 != sv)
			{
				pstack.push(e_records[v2]);
				v2 = v_records[v2];
			}

			while(!pstack.empty())
			{
				eidx = pstack.top();
				pstack.pop();
				for(auto iter = P->edges[eidx].support.begin(); iter != P->edges[eidx].support.end(); ++iter)
				{
					m4e = *iter;
					mesh_e = m4->edges[m4e].meshIdx;
					route.add_edge(mesh_e, mesh);
				}
			}

			separatrice.push_back(route);

			continue;
		}

		/*
		�ƽ�
		*/
		for(eidx = P->nodes[v1].edge_begin; eidx != -1; eidx = P->edges[eidx].next)
		{
			////�������-constriant����ѡȡ�ñ�
			if(P->edges[eidx].theta_con == false)
				continue;

			//��dual loops M2�ཻ����ѡȡ�ñ�
			if(candidate->Pedge_M2_cross(P->edges[eidx]))
				continue;

			v2 = P->edges[eidx].to_vertice;
			elen = P->edges[eidx].weight;

			if(!primal_Pedge_constraint(eidx, s_mesh, last_block, block))
				continue;

			if((list[v2] < 0) || (vlen + elen < list[v2]))
			{
				e_records[v2] = eidx;
				v_records[v2] = v1;
				b_records[v2] = block;
				list[v2] = vlen + elen;
				heap.push(v2, list[v2]);
			}
		}
	}
}

void LoopConstruct::Primalization()
{
	int cnt = 0;

	int idx;
	int m4s, m4v, m4u;
	int eidx, m4eidx;
	int mesh_u;
	OmVeH sv , su;
	OmHaEgH hfe;
	double elen;
	VECTOR e, Fd_e, nege, norm;

	//��dual loops����
	dualloops = candidate->existingLoops;
	candidate->show_existingLoops(2);

	embedding();

	for(auto v = singularities.begin(); v != singularities.end();++v)
	{
		primal_flood(*v);

		idx = v->idx();
		m4s = m4->mesh2covering(idx, 0);
		norm = mesh->normal(*v);
		for(auto iter = mesh->voh_begin(*v); iter.is_valid(); ++iter)
		{
			su = mesh->to_vertex_handle(*iter);
			//iter���������෴����
			e = meshtools::getVector(*iter, mesh);
			nege = meshtools::rev(e);
			//iter���෴���
			hfe = mesh->opposite_halfedge_handle(*iter);

			for(int j = 0; j < 4;++j)
			{
				m4u = m4->mesh2covering(su.idx(), j);
				Fd_e = m4->edge_Fd(hfe, j);

				/*����㵽�ڵ�ı�*/
				/*����m4�ı�*/
				m4eidx = m4->add_edge(m4s, m4u, *iter);
				m4->edges[m4eidx].Fd = meshtools::rev(Fd_e);
				m4->edges[m4eidx].e = e;

				eidx = P->add_edge(P->nodes[m4s], P->nodes[m4u]);
				//֧�ֱ�
				P->edges[eidx].support.push_back(m4eidx);
				//��Ȩ
				P->edges[eidx].weight = meshtools::w_e(norm, m4->edges[m4eidx].e, m4->edges[m4eidx].Fd);
				std::cout<<"W(e) : "<<P->edges[eidx].weight<<std::endl;
				//���������
				P->edges[eidx].theta_con = true;

				/*���ڵ㵽���ı�*/
				/*����m4�ı�*/
				
				m4eidx = m4->add_edge(m4u, m4s, hfe);
				m4->edges[m4eidx].Fd = Fd_e;
				m4->edges[m4eidx].e = nege;

				eidx = P->add_edge(P->nodes[m4u], P->nodes[m4s] );
				//֧�ֱ�
				P->edges[eidx].support.push_back(m4eidx);
				//��Ȩ
				P->edges[eidx].weight = meshtools::w_e(norm, m4->edges[m4eidx].e, m4->edges[m4eidx].Fd);
				std::cout<<"W(e) : "<<P->edges[eidx].weight<<std::endl;
				//���������
				P->edges[eidx].theta_con = true;
			}
		}
	}


	/*for(auto e = mesh->edges_begin(); e != mesh->edges_end();++e)
	{
		if(edge_block[e->idx()] != -1)
		{
			hosh->show_edge(*e, edge_block[e->idx()] % 4);
		}
	}*/

	for(auto v = singularities.begin(); v != singularities.end();++v)
	{
		primal_promote(v->idx());
		dealt_s.insert(v->idx());
	}

	std::cout<<"sep size: "<<separatrice.size()<<std::endl;
	for(auto i = separatrice.begin(); i != separatrice .end(); ++i)
	{
		i->show_path(hosh, 3);
	}
}

OmDualloop LoopConstruct::M4_loop_to_Om_loop(DualLoop &dloop)
{
	size_t meshidx;
	OmDualloop s;
	for(auto iter = dloop.M4_edge_begin(); iter != dloop.M4_edge_end(); ++iter)
	{
		meshidx = m4->edge_idx(*iter);
		s.push_back(mesh->edge_handle(meshidx));
	}
	return s;
}

Set_of_OmDualloop LoopConstruct::dualloops_OmEgH()
{
	
	Set_of_OmDualloop ret;
	for(auto dloop = dualloops.begin(); dloop != dualloops.end(); ++dloop)
	{
		ret.push_back(M4_loop_to_Om_loop(*dloop));
	}
	return ret;
}

DualLoop LoopConstruct::construct_feature_curve(std::set<size_t> feature_vhs)
{
	int sv, ev;
	int v1, v2;
	int v1_m4_idx, v2_m4_idx;
	int v1_mesh_idx, v2_mesh_idx;
	int feature_vhs_size = feature_vhs.size();
	double scalar, max_scalar, max_idx;
	OmVeH v1_handle, v2_handle;
	OmHaEgH he_handle;
	VECTOR Fd_e, e;
	DualLoop ret;
	/*�����Ѿ�������Ķ����mesh�ϵı��*/
	std::set<size_t> visited;

	visited.clear();
	sv = -1;
	ev = -1;
	max_scalar = -2;

	//
	for(auto i = feature_vhs.begin(); i != feature_vhs.end(); ++i)
	{
		//ȷ�����
		if(is_singularity[*i])
		{
			if(sv == -1)
			{
				sv = *i;
			}
			else
			{
				ev = *i;
			}
		}
	}

	if(sv == -1)
		sv = *(feature_vhs.begin());

	//visited.insert(sv);
	v1_handle = mesh->vertex_handle(sv);

	for(auto iter = mesh->voh_begin(v1_handle); iter.is_valid(); ++iter)
	{
		v2_handle = mesh->to_vertex_handle(*iter);
		v2_mesh_idx = v2_handle.idx();
		//��v2��������ϵĵ�
		if(feature_vhs.find(v2_mesh_idx) != feature_vhs.end())
		{
			he_handle = mesh->opposite_halfedge_handle(*iter);
			e = meshtools::getVector(*iter, mesh);
			e = meshtools::unitize(e);
			//ȷ��feature curveӦ����M4����һ����
			for(int j = 0;j < 4; ++j)
			{
				Fd_e = m4->edge_Fd(he_handle, j);
				scalar = meshtools::scalar(Fd_e, e);
				if(scalar > max_scalar)
				{
					max_scalar = scalar;
					max_idx = j;
				}
			}
			v2_m4_idx = m4->mesh2covering(v2_mesh_idx, max_idx);
			ret.add_vertex(v2_m4_idx);
			break;
		}
	}

	do{
		visited.insert(v2_mesh_idx);
		for(int eidx = m4->vertices[v2_m4_idx].edge_begin; eidx != -1; eidx = m4->edges[eidx].next)
		{
			v1_m4_idx = m4->edges[eidx].to_vertex;
			v1_mesh_idx = m4->covering2mesh(v1_m4_idx);
			//v1��������ϵĵ�
			if(feature_vhs.find(v1_mesh_idx) != feature_vhs.end())
			{
				if(visited.find(v1_mesh_idx) != visited.end() && v1_mesh_idx != sv)
					break;
				ret.add_path(eidx, m4);
				break;
			}
		}

		//v1�Ѿ������������ֹ
		if(visited.find(v1_mesh_idx) != visited.end())
		{
			break;
		}
		else
		{
			v2_m4_idx = v1_m4_idx;
			v2_mesh_idx = v1_mesh_idx;
		}
	}while(1);
	

	return ret;
}

DualLoop LoopConstruct::promote_from(int start_vertex)
{
	candidate->init(start_vertex);
	candidate->promote(featureCurves);
	return candidate->getLoop();
}

bool LoopConstruct::findLoop_with_featureCurves_simple()
{
	const double eps = 1e-6;
	DualLoop dl, maxloop, tdl;
	OmVeH v1, v2;
	OmHaEgH hfe;
	bool *suspended;
	double tmp, min, max;
	int size;
	int cnt = 0, s , s2;
	//
	size = SIfinder->sp.size();
	suspended = new bool[size];
	for(int i = 0; i < size;++i)
	{
		//
		suspended[i] = SIfinder->sp[i].cut;
		if(!suspended[i])
			cnt++;
	}
	std::cout<<"SIs remain :"<<cnt<<std::endl;

	//��ʼ��
	max = 0.0;
	s = -1;

	for(int sii = 0; sii < size;++sii)
	{
		min = -1;
		if(suspended[sii])
			continue;
		auto &r = SIfinder->sp[sii];
		for(auto i = r.begin(); i != r.end();++i)
		{
			for(int j = 0;j < 2;++j)
			{
				candidate->init(m4->mesh2covering(*i, j));
				tmp = candidate->promote(featureCurves);
				tdl = candidate->getLoop();

				//��û�ҵ�loop������
				if(tdl.size() < 3)
					continue;

				/*if(!loop_SI_cross2(tdl, sii))
					continue;*/

				//�������Ƿ�����̵�loop
				if(min < 0 || tmp < min)
				{
					min = tmp;
					dl = tdl;
				}
			}
		}

		//����loop�ཻ��SI��ʱ�Ƴ�
		for(int j = sii + 1;j < size;++j)
		{
			if(loop_SI_cross_simple(dl, j))
			{
				suspended[j] = true;
			}
		}

		//�ж��Ƿ������dual loop
		if(min > max)
		{
			//max = dl.length();
			max = min;
			maxloop = dl;
			s = sii;
		}
	}
	if(max < eps)
		return false;

	//����whiskers
	maxloop.getWhiskers(mesh);
	//�����loop����existing loops
	candidate->add2existingloops(maxloop);

	std::cout<<"l = "<<max<<std::endl;

	//����max loop �ཻ��SI���Ƴ�
	for(int i = 0;i < size; ++i)
	{
		auto &r = SIfinder->sp[i];
		if(!(r.cut))
		{
			r.cut = loop_SI_cross_simple(maxloop,i);
		}
	}
	return true;
}

int LoopConstruct::featureCurve_bfs(OmVeH sv, std::set<size_t> &feature_vhs)
{
	OmVeH v, u;
	OmFaH fv,fu;
	OmHaEgH hv, hu;
	OmEgH ev, eu;
	bool *visited;
	int idx = sv.idx();
	int n = mesh->n_faces();
	visited = new bool[n];

	for(int i = 0;i < n; ++i)
	{
		visited[i] = false;
	}	


	std::queue<OmFaH> que;
	while(!que.empty()) que.pop();
	
	que.push(*(mesh->vf_begin(sv)));
	visited[(mesh->vf_begin(sv))->idx()] = true;

	while(!que.empty())
	{
		fv = que.front();
		que.pop();
		

		//����Ƿ���������ϵĵ�
		for(auto iter = mesh->fv_begin(fv); iter.is_valid(); ++iter)
		{
			//sv���������ͬһ�ֿ��ڣ��򷵻����������ϵĵ�
			if(feature_vhs.find(iter->idx()) != feature_vhs.end())
			{
				SIfinder->add_SI(sv, *iter);
				return iter->idx();
			}
		}

		//primal_node[v.idx()] = idx;
		for(auto iter = mesh->fh_begin(fv); iter.is_valid(); ++iter)
		{
			hv = *iter;
			ev = mesh->edge_handle(hv);
			//���ev���ڻ�·��
			if(onLoops[ev.idx()] == false)
			{
				hu = mesh->opposite_halfedge_handle(hv);
				fu = mesh->face_handle(hu);
				//������fuδ���
				if(visited[fu.idx()] == false)
				{
					visited[fu.idx()] = true;
					que.push(fu);
				}
			}
		}
	}
	return -1;
}

int LoopConstruct::featureCurves_bfs(std::set<size_t> &feature_vhs1, std::set<size_t> &feature_vhs2)
{
	OmVeH v_handle;
	for(auto iter = feature_vhs1.begin(); iter != feature_vhs1.end(); ++iter)
	{
		v_handle = mesh->vertex_handle(*iter);
		if(featureCurve_bfs(v_handle, feature_vhs2) >= 0)
		{
			return 1;
		}
	}
	return -1;
}

DualLoop LoopConstruct::find_min_loop(int si_idx)
{
	auto &r = SIfinder->sp[si_idx];
	DualLoop dl, tdl;
	double tmp, min;
	int mid_vertex = r.size() / 2;
	int vertex_m4idx, v;
	min = -1;
	for(auto i = r.begin(); i != r.end();++i)
	{
		for(int j = 0;j < 2;++j)
		{
			vertex_m4idx = m4->mesh2covering(*i, j);
			tdl = promote_from(vertex_m4idx);
			tmp = tdl.length;

			//��û�ҵ�loop������
			if(tdl.size() < 3)
				continue;

			//���ҵ���loopû���и����������SI��������
			if(!loop_SI_cross_simple(tdl, si_idx))
				continue;

			//�������Ƿ�����̵�loop
			if(min < 0 || tmp < min)
			{
				min = tmp;
				dl = tdl;
				v = vertex_m4idx;
			}
		}
	}
	//***********����*********
	/*if(si_idx == 1896)
	{
		std::cout<<"v = "<<v<<std::endl;
	}*/
	//***************************
	return dl;
}

bool LoopConstruct::findLoop_with_featureCurves()
{
	const double eps = 1e-6;
	DualLoop dl, maxloop;
	OmVeH v1, v2;
	OmHaEgH hfe;
	bool *suspended;
	double tmp, min, max, mesh_length;
	int size;
	int cnt = 0, s, s2;
	size_t mesh_hfeidx;
	//
	size = SIfinder->sp.size();
	suspended = new bool[size];
	for(int i = 0; i < size;++i)
	{
		//
		suspended[i] = SIfinder->sp[i].cut;
		if(!suspended[i])
			cnt++;
	}
	std::cout<<"SIs remain :"<<cnt<<std::endl;

	//��ʼ��
	max = 0.0;
	s = -1;

	for(int sii = 0; sii < size;++sii)
	{
		/*******�ҵ��и��SI�ĳ�����С��loop*******/
		if(suspended[sii])
			continue;

		dl = find_min_loop(sii);
		min = dl.length;

		/*********�ҵ������loop��һ���Ϸ���dualloop���ҵ��������һ��*********/
		//����loop�ཻ��SI��ʱ�Ƴ�
		for(int j = sii + 1;j < size;++j)
		{
			if(loop_SI_cross_simple(dl, j, true))
			{
				suspended[j] = true;
			}
		}

		//�ж��Ƿ������dual loop
		if(min > max)
		{
			//max = dl.length();
			max = min;
			maxloop = dl;
			s = sii;
		}
	}
	//���Ҳ����Ϸ���loop���򷵻�false
	if(max < eps)
		return false;

	std::cout<<candidate->existingLoops.size()<<"  l = "<<max<<"  s="<<s<<std::endl;

	//����max loop �ཻ��SI���Ƴ�
	for(int i = 0;i < size; ++i)
	{
		auto &r = SIfinder->sp[i];
		if(!(r.cut))
		{
			r.cut = loop_SI_cross_simple(maxloop,i, true);
		}
	}
	//����whiskers
	maxloop.getWhiskers(mesh);
	//�����loop����existing loops
	candidate->add2existingloops(maxloop);

	return true;
}

void LoopConstruct::new_si_cut_check(int si_start, int si_end)
{
	int dln = 0;
	for(auto loop = candidate->existingLoops.begin(); loop != candidate->existingLoops.end(); ++loop, ++dln)
	{
		for(int i = si_start; i < si_end; ++i)
		{
			if(!SIfinder->sp[i].cut)
			{
				//bool b = SIfinder->sp[i].cut;
				SIfinder->sp[i].cut = this->loop_SI_cross_simple(*loop, i, true);
				/*if(SIfinder->sp[i].cut == true && dln == 1 && b == false)
				{
					SIfinder->show_SIpath(i, 3);
				}*/
			}
		}
	}
}

void LoopConstruct::promote_with_featureCurves(bool show)
{
	bool found;

	SIfinder->get_whiskers();

	do{
		found = findLoop_with_featureCurves();
	}while(found);
	
	std::cout<<"dual loops done : "<<candidate->existingLoops.size()<<std::endl;

	dualloops = candidate->existingLoops;
	if (show)
		candidate->show_existingLoops(-1);
	//embedding();
}

void LoopConstruct::process(std::vector<std::set<size_t>> hard_feature_vhs, 
																		std::vector<std::vector<int>> singularity_pair_he_idx, 
																		std::set<std::pair<int, int>> hardedge_pairing)
{
	int s_mesh_idx;
	int feature_idx;
	int test1, test2;
	int si1, si2, si3;
	int s2s_n;
	OmVeH vs_handle;

	for(int i = 0;i < hard_feature_vhs.size(); ++i)
	{
		featureCurves.push_back(construct_feature_curve(hard_feature_vhs[i]));
	}
	std::cout<<"feature curves :"<<featureCurves.size()<<std::endl;

	bool show = false;
	//�����feature curves��dualloops
	promote_with_featureCurves(show);

	//��¼����si������
	si1 = SIfinder->sp.size();
	std::cout<<"si1 : "<<si1<<std::endl;

	s2s_n = candidate->existingLoops.size();
	std::cout<<"��Ե�SI �� "<<s2s_n<<std::endl;

	//candidate->existingLoops[5].show_whiskers(hosh, mesh);

	//�����ߺ����֮������
	for(int i = 0;i < singularity_pair_he_idx.size(); ++i)
	{
		for(int j = 0;j < singularity_pair_he_idx[i].size(); ++j)
		{
			vs_handle = singularities[i];
			feature_idx = singularity_pair_he_idx[i][j];
			featureCurve_bfs(vs_handle, hard_feature_vhs[feature_idx]);
		}
	}

	//��¼���������������si֮���si����
	si2 = SIfinder->sp.size();
	std::cout<<"si2 : "<<si2 - si1<<std::endl;

	new_si_cut_check(si1, si2);
	show = false;
	promote_with_featureCurves(show);

	//test2 = candidate->existingLoops.size();
	//std::cout<<"���������SI �� "<<test2 - test1<<std::endl;

	for(auto i = hardedge_pairing.begin(); i != hardedge_pairing.end(); ++i)
	{
		featureCurves_bfs(hard_feature_vhs[i->first], hard_feature_vhs[i->second]);
	}

	//��¼������������������si֮���si����
	si3 = SIfinder->sp.size();
	std::cout<<"si3 : "<<si3 - si2<<std::endl;

	new_si_cut_check(si2, si3);

	show = false;
	promote_with_featureCurves(show);

	//test1 = candidate->existingLoops.size();
	//std::cout<<"�����߶�������SI �� "<<test1 - test2<<std::endl;

	//*********����*****************
	/*std::ofstream fout("testData.txt");
	DualLoop &dl19 = dualloops[19];
	DualLoop &dl20 = dualloops[20];
	DualLoop &dl21 = dualloops[21];
	for(auto eiter = dl19.M4_edge_begin(); eiter != dl19.M4_edge_end(); ++eiter)
	{
		fout<<m4->edges[*eiter].to_vertex<<' ';
	}
	fout<<std::endl;

	for(auto eiter = dl20.M4_edge_begin(); eiter != dl20.M4_edge_end(); ++eiter)
	{
		fout<<m4->edges[*eiter].to_vertex<<' ';
	}
	fout<<std::endl;*/
	//*******************************

	//remove_redundant_loops(s2s_n);
	remove_redundant_loops(dualloops.size());
}

double LoopConstruct::halfedge_length(int hf_idx)
{
	OmEgH ehandle = mesh->edge_handle(mesh->halfedge_handle(hf_idx));
	return edge_length[ehandle.idx()];
}

OmDualloop LoopConstruct::find_loop(OmVeH start_vertex, int dualloop_idx)
{
	size_t start_vertex_idx = start_vertex.idx();
	size_t v_idx;
	int m4_v_idx = -1;
	//DualLoop dualloop = candidate->existingLoops[dualloop_idx];
	DualLoop dualloop = dualloops[dualloop_idx];
	DualLoop new_dualloop;

	for(auto i = dualloop.M4_vertex_begin(); i != dualloop.M4_vertex_end(); ++i)
	{
		v_idx = m4->covering2mesh(*i);
		if(v_idx == start_vertex_idx)
		{
			//�ҵ���һ���ϵ�ı��
			m4_v_idx = *i + 1;
			//������4���̲�ͬ���ʾ��Ӧ�Ĳ���ͬһmesh�ϵĵ�
			if(m4_v_idx / 4 != (*i) / 4)
				m4_v_idx -= 4;
			break;
		}
	}

	new_dualloop = promote_from(m4_v_idx);
	candidate->existingLoops.push_back(new_dualloop);
	dualloops = candidate->existingLoops;
	//dualloops.push_back(new_dualloop);
	return M4_loop_to_Om_loop(new_dualloop);
}

OmDualloop LoopConstruct::find_loop_along_direction(OmVeH start_vertex, VECTOR tangent_dir)
{
	size_t start_vertex_idx = start_vertex.idx();
	int target_layer = -1;
	double closet_cosine = -1;
	//DualLoop dualloop = candidate->existingLoops[dualloop_idx];
	DualLoop new_dualloop;
	for (int l_iter = 0;l_iter < 4;l_iter++)
	{
		VECTOR vec_temp = m4->vertex_ds(start_vertex, l_iter);
		double current_cosine = vec_temp[0]*tangent_dir[0]+vec_temp[1]*tangent_dir[1]+vec_temp[2]*tangent_dir[2];
		if (current_cosine > closet_cosine)
		{
			closet_cosine = current_cosine;
			target_layer = l_iter;
		}
	}
	int m4_v_idx = m4->mesh2covering(start_vertex_idx,target_layer);

	new_dualloop = promote_from(m4_v_idx);
	candidate->existingLoops.push_back(new_dualloop);
	dualloops = candidate->existingLoops;
	//dualloops.push_back(new_dualloop);
	return M4_loop_to_Om_loop(new_dualloop);
}

OmDualloop  LoopConstruct::swap_loop(OmVeH start_vertex, int dualloop_idx, int target_idx)
{
	size_t start_vertex_idx = start_vertex.idx();
	size_t v_idx;
	int m4_v_idx = -1;
	//DualLoop dualloop = candidate->existingLoops[dualloop_idx];
	DualLoop dualloop = dualloops[dualloop_idx];
	DualLoop new_dualloop;

	for(auto i = dualloop.M4_vertex_begin(); i != dualloop.M4_vertex_end(); ++i)
	{
		v_idx = m4->covering2mesh(*i);
		if(v_idx == start_vertex_idx)
		{
			//�ҵ���һ���ϵ�ı��
			m4_v_idx = *i + 1;
			//������4���̲�ͬ���ʾ��Ӧ�Ĳ���ͬһmesh�ϵĵ�
			if(m4_v_idx / 4 != (*i) / 4)
				m4_v_idx -= 4;
			break;
		}
	}

	new_dualloop = promote_from(m4_v_idx);
	candidate->existingLoops.push_back(new_dualloop);
	std::vector<DualLoop> new_loops;
	for (int i = 0;i < candidate->existingLoops.size();i++)
	{
		if (i != target_idx)
		{
			new_loops.push_back(candidate->existingLoops[i]);
		}
	}
	dualloops = new_loops;
	//dualloops.push_back(new_dualloop);
	return M4_loop_to_Om_loop(new_dualloop);
}

void LoopConstruct::read_from_file(std::string filename, std::vector<std::set<size_t>> hard_feature_vhs)
{
	for(int i = 0;i < hard_feature_vhs.size(); ++i)
	{
		featureCurves.push_back(construct_feature_curve(hard_feature_vhs[i]));
	}
	//std::cout<<"read from"<<filename<<std::endl;
	candidate->read_from_file(filename);
	dualloops = candidate->existingLoops;
}

void LoopConstruct::write_to_file(std::string filename)
{
	//candidate->write_to_file(filename);
	std::ofstream fout(filename);
	fout<<dualloops.size()<<std::endl;
	for(auto iter = dualloops.begin(); iter != dualloops.end(); ++iter)
	{
		iter->write_to_file(fout);
	} 
}

//true��ʾ�ཻ��û��M2�ཻ
bool LoopConstruct::loop_cross_type(int loop1_idx, int loop2_idx)
{
	return candidate->loop_cross_type(loop1_idx, loop2_idx);
}

bool LoopConstruct::dualloop_is_redundant(int dualloop_idx, bool *removed)
{
	bool crossed;
	DualLoop &loop = dualloops[dualloop_idx];
	std::set<int> dps;
	std::vector<int> sis;
	for(auto si_iter = loop.si_begin(); si_iter != loop.si_end(); ++si_iter)
	{
		crossed = false;
		for(int i = 0; i < dualloops.size(); ++i)
		{
			if(i != dualloop_idx && !removed[i])
			{
				if(loop_SI_cross_simple(dualloops[i], *si_iter))
				{
					/*if(dualloop_idx == -1 && i == 2)
					{
						dps.insert(i);
						sis.push_back(*si_iter);
					}*/
					crossed = true;
					break;
				}
			}
		}
		//������SIû�б�������dualloop�и�򱻲��Ե�����dualloop���������
		if(crossed == false)
		{
			//**********����*************
			/*if(dualloop_idx == 14 || dualloop_idx == 19 || dualloop_idx == 20 || dualloop_idx == 21)
			{
				SIfinder->show_SIpath(*si_iter, 3);
			}*/
			//****************************
			return false;
		}
	}

	/*if(dualloop_idx == -1)
	{
		for(auto i = sis.begin(); i != sis.end(); ++i)
		{
			SIfinder->show_SIpath(*i, 3);
		}

		std::cout<<"redundant loop "<<dualloop_idx<<"--->";
		for(auto i = dps.begin(); i != dps.end(); ++i)
		{
			std::cout<<*i<<' ';
		}
		std::cout<<std::endl;
	}*/
	
	return true;
}

//�Ƴ������dualloop
void LoopConstruct::remove_redundant_loops(int n)
{
	int dualloop_idx = 0;
	int redundant_loops_cnt = 0;

	bool *removed;
	removed = new bool[dualloops.size()];
	for(int i = 0; i < dualloops.size(); ++i)
	{
		removed[i] = false;
	}

	std::vector<DualLoop> new_loops;
	auto dualloop_end = dualloops.begin()+n;
	std::sort(dualloops.begin(), dualloop_end);
	for(auto loop_iter = dualloops.begin(); loop_iter != dualloop_end; ++loop_iter, ++dualloop_idx)
	{
		//��dualloop��������������£�������
		if(!dualloop_is_redundant(dualloop_idx, removed))
		{
			new_loops.push_back(*loop_iter);
		}
		else
		{
			redundant_loops_cnt++;
			removed[dualloop_idx] = true;
			//������
			//new_loops.push_back(*loop_iter);
			//std::cout<<"Delete dualloop : "<<dualloop_idx<<std::endl;
		}
	}
	for(auto loop_iter = dualloop_end; loop_iter != dualloops.end(); ++loop_iter)
	{
		new_loops.push_back(*loop_iter);
	}
	//��������浽dualloops
	dualloops = new_loops;
	std::cout<<"ɾ������loop: "<<redundant_loops_cnt<<std::endl;
}


void LoopConstruct::remove_specified_loops(std::unordered_set<int> sp_ids)
{
	std::vector<DualLoop> new_loops;
	for (int lid = 0; lid < dualloops.size(); lid++)
	{
		if (sp_ids.find(lid) == sp_ids.end())
			new_loops.push_back(dualloops[lid]);
	}
	dualloops = new_loops;
}