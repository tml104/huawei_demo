#include <cstdio>
#include <vector>
#include <string>
#include <cmath>
#include "StdAfx.h"
#include "MeshDefs.h"
#include "FuncDefs.h"
#include "mesh_optimization.h"
#include "PrioritySetManager.h"
std::unordered_set<OvmVeH> vhs_local;

//std::ofstream out("output.txt", std::ofstream::out, _SH_DENYWR);

using namespace dlib;

#define set_row(objective, source, idx) (objective[idx][0] = source[idx][0], objective[idx][1] = source[idx][1], objective[idx][2] = source[idx][2])

std::hash_map<OvmCeH, std::map<OvmVeH, std::vector<OvmVeH>>>preprocess(VolumeMesh *mesh, bool orientation)
{
	std::hash_map<OvmCeH, std::map<OvmVeH, std::vector<OvmVeH>>> res;

	for (auto c_it = mesh->cells_begin(); c_it; ++c_it)
	{
		std::map<OvmVeH, std::vector<OvmVeH>> groups;
		std::vector<OvmVeH> vhs, adj_vhs;
		vhs = get_adj_vertices_around_hexa(mesh, *c_it);
		if (orientation)
		{
			//orientation_1
			//vhs[0]
			adj_vhs.clear();
			adj_vhs.push_back(vhs[3]), adj_vhs.push_back(vhs[1]), adj_vhs.push_back(vhs[4]);
			groups[vhs[0]] = adj_vhs;
			//vhs[1], vhs[2], vhs[3]
			for (int i = 1; i <= 3; ++i)
			{
				adj_vhs.clear();
				adj_vhs.push_back(vhs[i - 1]), adj_vhs.push_back(vhs[(i + 1) % 4]), adj_vhs.push_back(vhs[8 - i]);
				groups[vhs[i]] = adj_vhs;
			}

			//vhs[4]
			adj_vhs.clear();
			adj_vhs.push_back(vhs[7]), adj_vhs.push_back(vhs[5]), adj_vhs.push_back(vhs[0]);
			groups[vhs[4]] = adj_vhs;

			//vhs[5]
			adj_vhs.clear();
			adj_vhs.push_back(vhs[4]), adj_vhs.push_back(vhs[6]), adj_vhs.push_back(vhs[3]);
			groups[vhs[5]] = adj_vhs;

			//vhs[6]
			adj_vhs.clear();
			adj_vhs.push_back(vhs[5]), adj_vhs.push_back(vhs[7]), adj_vhs.push_back(vhs[2]);
			groups[vhs[6]] = adj_vhs;

			//vhs[7]
			adj_vhs.clear();
			adj_vhs.push_back(vhs[6]), adj_vhs.push_back(vhs[4]), adj_vhs.push_back(vhs[1]);
			groups[vhs[7]] = adj_vhs;
		}
		else
		{
			//orientation_0
			//vhs[0]
			adj_vhs.clear();
			adj_vhs.push_back(vhs[1]), adj_vhs.push_back(vhs[3]), adj_vhs.push_back(vhs[4]);
			groups[vhs[0]] = adj_vhs;

			//vhs[1], vhs[2], vhs[3]
			for (int i = 1; i <= 3; ++i)
			{
				adj_vhs.clear();
				adj_vhs.push_back(vhs[(i + 1) % 4]), adj_vhs.push_back(vhs[i - 1]), adj_vhs.push_back(vhs[8 - i]);
				groups[vhs[i]] = adj_vhs;
			}

			//vhs[4]
			adj_vhs.clear();
			adj_vhs.push_back(vhs[5]), adj_vhs.push_back(vhs[7]), adj_vhs.push_back(vhs[0]);
			groups[vhs[4]] = adj_vhs;

			//vhs[5]
			adj_vhs.clear();
			adj_vhs.push_back(vhs[6]), adj_vhs.push_back(vhs[4]), adj_vhs.push_back(vhs[3]);
			groups[vhs[5]] = adj_vhs;

			//vhs[6]
			adj_vhs.clear();
			adj_vhs.push_back(vhs[7]), adj_vhs.push_back(vhs[5]), adj_vhs.push_back(vhs[2]);
			groups[vhs[6]] = adj_vhs;

			//vhs[7]
			adj_vhs.clear();
			adj_vhs.push_back(vhs[4]), adj_vhs.push_back(vhs[6]), adj_vhs.push_back(vhs[1]);
			groups[vhs[7]] = adj_vhs;
		}

		res[*c_it] = groups;
	}

	return res;
}

Mesh_optimization::Mesh_optimization(VolumeMesh *_mesh, BODY *_body, std::hash_map<OvmCeH, std::map<OvmVeH, std::vector<OvmVeH>>> _cell_vertex_vertices)
	: mesh (_mesh), body (_body), cell_vertex_vertices(_cell_vertex_vertices)
{
	nv = 4;
	delta = 0.001;
	m_vh = mesh->InvalidVertexHandle;
}


Mesh_optimization::~Mesh_optimization(void)
{
}

double Mesh_optimization::Frobenius_product(double A[][3], double B[][3]) const
{
	return A[0][0] * B[0][0] + A[1][0] * B[1][0] + A[2][0] * B[2][0]
	+ A[0][1] * B[0][1] + A[1][1] * B[1][1] + A[2][1] * B[2][1]
	+ A[0][2] * B[0][2] + A[1][2] * B[1][2] + A[2][2] * B[2][2];
}

double Mesh_optimization::det(double A[][3]) const
{
	return A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1])
		- A[0][1] * (A[1][0] * A[2][2] - A[2][0] * A[1][2])
		+ A[0][2] * (A[1][0] * A[2][1] - A[2][0] * A[1][1]);
}

double Mesh_optimization::CalcJacobian(OvmVeH vh, OvmCeH ch, double J[][3])
{
	OvmHaEgH heh[3];
	OvmHaFaH hfh[3];
	auto group = cell_vertex_vertices[ch];
	auto adj_vhs = group[vh];

	// Get Jacobian Matrix
	OvmPoint3d this_pt = mesh->vertex(vh);
	OvmPoint3d pts[3];

	for(int i = 0; i != 3; ++i)
	{
		pts[i] = mesh->vertex(adj_vhs[i]) - this_pt;
	}

	for(int i = 0; i != 3; ++i)
	{
		for(int j = 0; j != 3; ++j)
		{
			J[i][j] = pts[j][i];
		}
	}
	return det(J);
}

double Mesh_optimization::StdJacobian(OvmVeH vh, OvmCeH ch, double J[][3])
{
	OvmHaEgH heh[3];
	OvmHaFaH hfh[3];
	auto group = cell_vertex_vertices[ch];
	auto adj_vhs = group[vh];

	// Get Jacobian Matrix
	OvmPoint3d this_pt = mesh->vertex(vh);
	OvmPoint3d pts[3];

	for (int i = 0; i < 3; ++i)
	{
		pts[i] = mesh->vertex(adj_vhs[i]) - this_pt;
		pts[i].normalize();
	}

	for (int i = 0; i != 3; ++i)
	{
		for (int j = 0; j != 3; ++j)
		{
			J[i][j] = pts[j][i];
		}
	}
	return det(J);
}

//each cell
unsigned int Mesh_optimization::outliercellcount()
{
	double J[3][3];
	double J_det;
	unsigned int count = 0;

	for(auto c_it = mesh->cells_begin(); c_it != mesh->cells_end(); c_it++)
	{
		for(auto cv_it = mesh->cv_iter(*c_it); cv_it.valid(); ++cv_it)
		{
			J_det = CalcJacobian(*cv_it, *c_it, J);
			if(J_det < 0)
			{
				++count;
				break;
			}
		}
	}
	return count;
}

double Mesh_optimization::quality_evaluation_Jacobian()
{
	double J[3][3];
	double J_det;
	double cell_min_J, min_J = 2, average_J = 0, max_J = -2;

	for(auto c_it = mesh->cells_begin(); c_it != mesh->cells_end(); c_it++)
	{
		cell_min_J = 2;
		//bool flag = false;
		for(auto cv_it = mesh->cv_iter(*c_it); cv_it; ++cv_it)
		{
			J_det = StdJacobian(*cv_it, *c_it, J);
			if(J_det < cell_min_J)
				cell_min_J = J_det;
		}

		average_J += cell_min_J;
		if(min_J > cell_min_J) min_J = cell_min_J;
		if(max_J < cell_min_J) max_J = cell_min_J;
	}

	average_J /= mesh->n_cells();
	std::cout << "min_J: " << min_J << ", max_J: " << max_J << ", average_J: " << average_J << "\n";

	return min_J;
}

// each point
double Mesh_optimization::quality_evaluation(unsigned int &n_outlier)
{
	double q = 2;
	int v_count = 0;
	bool outlierflag;
	for(auto v_it = mesh->vertices_begin(); v_it != mesh->vertices_end(); ++v_it)
	{
		outlierflag = true;
		auto vh = *v_it;
		auto pt = mesh->vertex(vh);
		double res = 0;
		int cell_count = 0;
		//  Iterate over all incident cells of a given vertex.
		for(auto c_it = mesh->vc_iter(vh); c_it; ++c_it)
		{
			double eta = 0;
			OvmVeH vhs[4];
			std::map<OvmVeH, std::vector<OvmVeH>> group = cell_vertex_vertices[*c_it];
			auto adj_vhs = group[vh];
			vhs[0] = vh;
			vhs[1] = adj_vhs[0], vhs[2] = adj_vhs[1], vhs[3] = adj_vhs[2];
			++cell_count;
			// Iterate over partial vertices of a given cell.
			for(int idx = 0; idx != 4; ++idx)
			{
				double J[3][3];
				double J_det;

				// Get Jacobian Matrix
				OvmPoint3d this_pt = mesh->vertex(vhs[idx]);
				OvmPoint3d pts[3];

				adj_vhs = group[vhs[idx]];
				for (int i = 0; i < 3; ++i)
					pts[i] = mesh->vertex(adj_vhs[i]) - this_pt;

				for(int i = 0; i != 3; ++i)
				{
					for(int j = 0; j != 3; ++j)
					{
						J[i][j] = pts[j][i];
					}
				}
				J_det = det(J);
				if(idx == 0 && outlierflag && J_det < 0) {outlierflag = false; ++v_count;/*std::cout << vhs[0].idx() << ' ';*/}
				eta += Frobenius_product(J, J) * pow( (sqrt(J_det * J_det + 4 * delta * delta) - J_det) / (2 * delta * delta), 2.0/3 ) / 3;
			}
			eta /= nv; // nv = 4
			res += eta * eta;
		}
		double t_q = 1 / sqrt(res / cell_count);
		if(q > t_q) q = t_q;
		//if(vh.idx() == 3441) std::cout << mesh->vertex(vh) << ": " << t_q << '\n';
	}
	n_outlier = v_count;
	std::cout << "\ncount of outlier vertices: " << v_count << '\n';
	return q;
}

double Mesh_optimization::IEta_star(const column_vector &m)
{
	double res = 0;

	//  Iterate over all incident cells of a given vertex.
	for(auto c_it = mesh->vc_iter(m_vh); c_it; ++c_it)
	{
		double eta = 0;
		OvmVeH vhs[4];
		std::map<OvmVeH, std::vector<OvmVeH>> group = cell_vertex_vertices[*c_it];
		auto adj_vhs = group[m_vh];
		vhs[0] = m_vh;
		vhs[1] = adj_vhs[0], vhs[2] = adj_vhs[1], vhs[3] = adj_vhs[2];

		// Iterate over partial vertices of a given cell.
		for(int idx = 0; idx != 4; ++idx)
		{
			double J[3][3];
			double J_det;

			// Get Jacobian Matrix
			OvmPoint3d this_pt = mesh->vertex(vhs[idx]);
			OvmPoint3d pts[3];
			OvmPoint3d new_pt(m(0),m(1),m(2));

			if(idx == 0) 
			{
				this_pt = new_pt;
				for(int i = 0; i != 3; ++i)
					pts[i] = mesh->vertex(adj_vhs[i]) - this_pt;
			}
			else
			{
				adj_vhs = group[vhs[idx]];
				for(int i = 0; i != 3; ++i)
				{
					auto temp_vh = adj_vhs[i];
					if(temp_vh == m_vh)
						pts[i] = new_pt - this_pt;
					else
						pts[i] = mesh->vertex(temp_vh) - this_pt;
				}
			}

			for(int i = 0; i != 3; ++i)
			{
				for(int j = 0; j != 3; ++j)
				{
					J[i][j] = pts[j][i];
				}
			}
			J_det = det(J);
			auto h_sigma = J_det > 0 ? 2 / (J_det + sqrt(J_det * J_det + 4 * delta * delta)) : 
				(sqrt(J_det * J_det + 4 * delta * delta) - J_det) / (2 * delta * delta);
			eta += Frobenius_product(J, J) * pow( h_sigma, 2.0/3 ) / 3;
			//eta += Frobenius_product(J, J) * pow( (sqrt(J_det * J_det + 4 * delta * delta) - J_det) / (2 * delta * delta), 2.0/3 ) / 3;
		}
		eta /= nv; // nv = 4
		res += eta * eta;
	}

	return res;
}

const column_vector Mesh_optimization::IEta_star_derivative (const column_vector &m)
{
	column_vector res = zeros_matrix<double>(3,1);

	for(int alpha_idx = 0; alpha_idx != 3; ++alpha_idx)
	{
		for(auto c_it = mesh->vc_iter(m_vh); c_it; ++c_it)
		{
			double eta = 0;
			double eta_alpha = 0;
			OvmVeH vhs[4];
			std::map<OvmVeH, std::vector<OvmVeH>> group = cell_vertex_vertices[*c_it];
			auto adj_vhs = group[m_vh];
			vhs[0] = m_vh;
			vhs[1] = adj_vhs[0], vhs[2] = adj_vhs[1], vhs[3] = adj_vhs[2];

			// Iterate over partial vertices of a given cell.
			for(int idx = 0; idx != 4; ++idx)
			{
				double J[3][3];
				double J_det;
				double eta_element;
				double eta_partial;
				int flag = 3; //标记PJ在哪个位置的元素不为0: flag=3表示PJ为零矩阵，flag=0,1,2表示PJ(alpha_idx,flag)=1; flag=-1表示rowm(PJ,alpha_idx)=1. 

				// Get Jacobian Matrix
				OvmPoint3d this_pt = mesh->vertex(vhs[idx]);
				OvmPoint3d pts[3];
				OvmPoint3d new_pt(m(0),m(1),m(2));

				if(idx == 0) 
				{
					this_pt = new_pt;
					for(int i = 0; i != 3; ++i)
					{
						pts[i] = mesh->vertex(adj_vhs[i]) - this_pt;
					}
					flag = -1;
				}
				else
				{
					adj_vhs = group[vhs[idx]];
					for(int i = 0; i != 3; ++i)
					{
						auto temp_vh = adj_vhs[i];
						if(temp_vh == m_vh)
						{
							pts[i] = new_pt - this_pt;
							flag = i;
						}
						else
							pts[i] = mesh->vertex(temp_vh) - this_pt;
					}
				}


				double temp[3][3];
				for(int i = 0; i != 3; ++i)
				{
					for(int j = 0; j != 3; ++j)
					{
						J[i][j] = temp[i][j] = pts[j][i];
					}
				}

				J_det = det(J);
				auto h_sigma = J_det > 0 ? 2 / (J_det + sqrt(J_det * J_det + 4 * delta * delta)) : 
					(sqrt(J_det * J_det + 4 * delta * delta) - J_det) / (2 * delta * delta);
				eta_element = Frobenius_product(J, J) * pow( h_sigma, 2.0/3 ) / 3;
				//eta_element = Frobenius_product(J, J) * pow( (sqrt(J_det * J_det + 4 * delta * delta) - J_det) / (2 * delta * delta), 2.0/3 ) / 3;
				eta += eta_element;


				double PJ[3][3] = {0};
				double PSigma;

				if(flag == -1)
					PJ[alpha_idx][0] = PJ[alpha_idx][1] = PJ[alpha_idx][2] = -1;
				else
					PJ[alpha_idx][flag] = 1;

				set_row(temp, PJ, alpha_idx);
				PSigma = det(temp);
				eta_partial = 2 * eta_element;
				eta_partial *= (Frobenius_product(PJ, J) / Frobenius_product(J, J) - PSigma / (3 * sqrt(J_det * J_det + 4 * delta * delta))) ;

				eta_alpha += eta_partial;
			}
			res(alpha_idx) += eta * eta_alpha;
		}
	}
	res = res / 8;

	return res;
}

dlib::matrix<double> Mesh_optimization::IEta_star_hessian (const column_vector &m)
{
	matrix<double> res = zeros_matrix<double>(3,3);

	for(int alpha_idx = 0; alpha_idx != 3; ++alpha_idx)
	{
		for(int beta_idx = 0; beta_idx <= alpha_idx; ++beta_idx)
		{
			for(auto c_it = mesh->vc_iter(m_vh); c_it; ++c_it)
			{
				double eta = 0;
				double eta_alpha = 0;
				double eta_beta = 0;
				double eta_alpha_beta = 0;
				OvmVeH vhs[4];
				std::map<OvmVeH, std::vector<OvmVeH>> group = cell_vertex_vertices[*c_it];
				auto adj_vhs = group[m_vh];
				vhs[0] = m_vh;
				vhs[1] = adj_vhs[0], vhs[2] = adj_vhs[1], vhs[3] = adj_vhs[2];

				// Iterate over partial vertices of a given cell.
				for(int idx = 0; idx != 4; ++idx)
				{
					double J[3][3];
					double J_det;
					double FNorm_square;
					double eta_element;
					double eta_partial_alpha;
					double eta_partial_beta;
					int flag = 3;


					// Get Jacobian Matrix
					OvmPoint3d this_pt = mesh->vertex(vhs[idx]);
					OvmPoint3d pts[3];
					OvmPoint3d new_pt(m(0), m(1), m(2));

					if(idx == 0) 
					{
						this_pt = new_pt;
						for(int i = 0; i != 3; ++i)
						{
							pts[i] = mesh->vertex(adj_vhs[i]) - this_pt;
						}
						flag = -1;
					}
					else
					{
						adj_vhs = group[vhs[idx]];
						for(int i = 0; i != 3; ++i)
						{
							auto temp_vh = adj_vhs[i];
							if(temp_vh == m_vh)
							{
								pts[i] = new_pt - this_pt;
								flag = i;
							}
							else
								pts[i] = mesh->vertex(temp_vh) - this_pt;
						}
					}

					double temp_alpha[3][3], temp_beta[3][3];
					for(int i = 0; i != 3; ++i)
					{
						for(int j = 0; j != 3; ++j)
						{
							J[i][j] = temp_alpha[i][j] = temp_beta[i][j] = pts[j][i];
						}
					}


					J_det = det(J);
					FNorm_square = Frobenius_product(J, J);
					eta_element = FNorm_square * pow( (sqrt(J_det * J_det + 4 * delta * delta) - J_det) / (2 * delta * delta), 2.0/3 ) / 3;
					eta += eta_element;

					double PJ_alpha[3][3] = {0};
					double PJ_beta[3][3] = {0};
					double PSigma_alpha, PSigma_beta;

					if(flag == -1)
					{
						PJ_alpha[alpha_idx][0] = PJ_alpha[alpha_idx][1] = PJ_alpha[alpha_idx][2] = -1;
						PJ_beta[beta_idx][0] = PJ_beta[beta_idx][1] = PJ_beta[beta_idx][2] = -1;
					}
					else
					{
						PJ_alpha[alpha_idx][flag] = 1;
						PJ_beta[beta_idx][flag] = 1;
					}

					set_row(temp_alpha, PJ_alpha, alpha_idx);
					PSigma_alpha = det(temp_alpha);
					eta_partial_alpha = 2 * eta_element;
					eta_partial_alpha *= (Frobenius_product(PJ_alpha, J) / FNorm_square - PSigma_alpha / (3 * sqrt(J_det * J_det + 4 * delta * delta))) ;

					set_row(temp_beta, PJ_beta, beta_idx);
					PSigma_beta = det(temp_beta);
					eta_partial_beta = 2 * eta_element;
					eta_partial_beta *= (Frobenius_product(PJ_beta, J) / FNorm_square - PSigma_beta / (3 * sqrt(J_det * J_det + 4 * delta * delta))) ;

					eta_alpha += eta_partial_alpha;
					eta_beta += eta_partial_beta;


					eta_alpha_beta += eta_partial_alpha * eta_partial_beta / eta_element + 2 * eta_element *
						(Frobenius_product(PJ_alpha, PJ_beta) / FNorm_square - 
						2 * Frobenius_product(PJ_alpha, J) * Frobenius_product(PJ_beta, J) /
						(FNorm_square * FNorm_square) + J_det * PSigma_alpha * PSigma_beta
						/ ( 3 * pow( J_det * J_det + 4 * delta * delta, 3.0/2 ) ));
				}
				res(alpha_idx, beta_idx) += eta_beta * eta_alpha + eta * eta_alpha_beta;
			}
		}
	}
	res(0,1) = res(1,0);
	res(0,2) = res(2,0);
	res(1,2) = res(2,1);
	res = res / 8;

	return res;
}

double Mesh_optimization::CEta_star(const column_vector &m, const curve &crv)
{
	double res = 0;
	column_vector point(3);
	SPAposition pos;
	crv.eval(m(0), pos);
	point = pos.coordinate(0), pos.coordinate(1), pos.coordinate(2);
	res = IEta_star(point);

	return res;
}

const column_vector Mesh_optimization::CEta_star_derivative(const column_vector &m, const curve &crv)
{
	column_vector res = zeros_matrix<double>(1,1);
	column_vector point(3), first_deriv_col(3);
	SPAposition pos;
	SPAvector first_deriv; 
	crv.eval(m(0), pos, first_deriv);
	point = pos.coordinate(0), pos.coordinate(1), pos.coordinate(2);
	first_deriv_col = first_deriv.x(), first_deriv.y(), first_deriv.z();
	res = dot(first_deriv_col, IEta_star_derivative(point));

	return res;
}

double Mesh_optimization::SEta_star(const column_vector &m, const surface &surf)
{
	double res = 0;
	column_vector point(3);
	SPAposition pos;
	SPApar_pos param_val(m(0), m(1));
	surf.eval(param_val, pos);
	point = pos.coordinate(0), pos.coordinate(1), pos.coordinate(2);
	res = IEta_star(point);
	//if(m_vh.idx() == 71)
	//{
	//	out << "parametric:" << m << ", Cartesian: " << point;
	//	out << "res:" << res << '\n';
	//	//std::cout << "parametric:" << m << ", Cartesian: " << point << '\n';
	//	//std::cout << "res:" << res << '\n';
	//}
	return res;
}

const column_vector Mesh_optimization::SEta_star_derivative (const column_vector &m, const surface &surf)
{
	column_vector res = zeros_matrix<double>(2,1);
	column_vector point(3), first_deriv_u(3), first_deriv_v(3), deriv_nonparam(3);
	SPAposition pos;
	SPAvector first_deriv[2];
	SPApar_pos param_val(m(0), m(1));
	surf.eval(param_val, pos, first_deriv);
	point = pos.coordinate(0), pos.coordinate(1), pos.coordinate(2);
	first_deriv_u = first_deriv[0].x(), first_deriv[0].y(), first_deriv[0].z();
	first_deriv_v = first_deriv[1].x(), first_deriv[1].y(), first_deriv[1].z();
	deriv_nonparam = IEta_star_derivative(point);
	res(0) = dot(first_deriv_u, deriv_nonparam);
	res(1) = dot(first_deriv_v, deriv_nonparam);

	return res;
}

void Mesh_optimization::boundary_points_optim()
{
	assert(mesh->vertex_property_exists<unsigned long> ("entityptr"));
	auto V_ENTITY_PTR = mesh->request_vertex_property<unsigned long>("entityptr");
	//对边的类型统计一下
	std::hash_map<VERTEX*, OvmVeH> vertices_mapping;
	std::hash_map<EDGE*, std::unordered_set<OvmVeH> > edges_vertices_mapping;
	std::hash_map<FACE*, std::unordered_set<OvmVeH> > faces_vertices_mapping;
	std::unordered_set<OvmVeH> volume_vhs, new_boundary_vhs;

	ENTITY_LIST vertices_list, edges_list, faces_list;
	api_get_vertices (body, vertices_list);
	api_get_edges (body, edges_list);
	api_get_faces (body, faces_list);

	for(auto v_it_ = mesh->vertices_begin(); v_it_ != mesh->vertices_end(); ++v_it_)
	{
		auto entity_ptr_uint = V_ENTITY_PTR[*v_it_];
		if(entity_ptr_uint == 0)
			volume_vhs.insert(*v_it_);
		else{
			if (entity_ptr_uint == -1){
				//判断一下这个新点是否在体内，如果是的话，则将他的entity_ptr设置为0
				if (!mesh->is_boundary(*v_it_)){
					V_ENTITY_PTR[*v_it_] = 0;
					volume_vhs.insert(*v_it_);
				}
				else
					new_boundary_vhs.insert(*v_it_);
			}
			else{
				ENTITY *entity = (ENTITY*)(entity_ptr_uint);
				if (is_VERTEX(entity))
					vertices_mapping[(VERTEX*)entity] = *v_it_;
				else{
					if (is_EDGE(entity))
						edges_vertices_mapping[(EDGE*)entity].insert(*v_it_);
					else
					{
						faces_vertices_mapping[(FACE*)entity].insert(*v_it_);
					}
				}
			}
		}
	}
	assert(vertices_list.count() == vertices_mapping.size());

	int np, count = 0, maxIter = 20;
	double DisEpsilon = 0.01;
	double displacement;
	//boundary_curves_optim
	//do{
	displacement = 0;
	for(int i = 0; i != edges_list.count(); ++i)
	{
		EDGE *acis_edge = (EDGE*)edges_list[i];
		auto &mesh_vhs_set = edges_vertices_mapping[acis_edge];
		const curve &crv = acis_edge->geometry()->equation();

		//SPAinterval inter = acis_edge->param_range();
		//double low = inter.start_pt(), high = inter.end_pt();
		//double low = acis_edge->start_param(), high = acis_edge->end_param();
		//out << "param_low: " << low << ", param_high: " << high << '\n';
		//if(low > high){double temp = low; low = high; high = temp;

		for(auto _v_it = mesh_vhs_set.begin(); _v_it != mesh_vhs_set.end(); ++_v_it)
		{
			//局部
			if(contains(vhs_local, *_v_it))
				continue;

			SPAposition pos = POS2SPA(mesh->vertex(*_v_it));
			SPAposition pos_old = pos;
			m_vh = *_v_it;

			column_vector starting_point(1);
			starting_point = crv.param(pos); // param_val
			auto _CEta_star = [this, &crv](const column_vector &m){return CEta_star(m, crv);};
			auto _CEta_star_derivative = [this, &crv](const column_vector &m){return CEta_star_derivative(m, crv);};

			find_min(bfgs_search_strategy(),  // Use BFGS search algorithm
				objective_delta_stop_strategy(1e-7,3), // Stop when the change in Eta_star() is less than 1e-7
				_CEta_star, _CEta_star_derivative, starting_point, 0);
			//find_min_box_constrained(bfgs_search_strategy(),  // Use BFGS search algorithm
			//	objective_delta_stop_strategy(1e-7,3), // Stop when the change in Eta_star() is less than 1e-7
			//	_CEta_star, _CEta_star_derivative, starting_point, low, high);
			crv.eval(starting_point, pos);
			//auto Dis = [pos_old, pos](){return std::sqrt((pos_old.x()-pos.x())*(pos_old.x()-pos.x()) + (pos_old.y()-pos.y())*(pos_old.y()-pos.y()) + (pos_old.z()-pos.z())*(pos_old.z()-pos.z()));};
			//double curDisplacement = Dis();
			//if(displacement < curDisplacement) displacement = curDisplacement;
			mesh->set_vertex(*_v_it, SPA2POS(pos));
		}
	}
	//if(++count > maxIter) break;
	//}while(displacement > DisEpsilon);
	//std::cout << "Curve: " << count << '\n';

	//boundary_surfaces_optim
	count = 0;
	//do{
	displacement = 0;
	for(int i = 0; i != faces_list.count(); ++i)
	{
		FACE *acis_face = (FACE*)(faces_list[i]);
		auto vhs_on_this_face = faces_vertices_mapping[acis_face];
		const surface &surf = acis_face->geometry()->equation();

		//SPAinterval u_inter = surf.param_range_u();
		//SPAinterval v_inter = surf.param_range_v();
		//column_vector low(2), high(2);

		//low = u_inter.start_pt(), v_inter.start_pt();
		//high = u_inter.end_pt(), v_inter.end_pt();
		//for(int j = 0; j < 2; ++j){ if(low(j) > high(j)){double temp = low(j); low(j) = high(j); high(j) = temp;} }

		for(auto _v_it = vhs_on_this_face.begin(); _v_it != vhs_on_this_face.end(); ++_v_it)
		{
			//局部
			if(contains(vhs_local, *_v_it))
				continue;


			SPAposition pos = POS2SPA(mesh->vertex(*_v_it));
			SPAposition pos_old = pos;
			m_vh = *_v_it;

			column_vector starting_point(2);
			SPApar_pos param_val = surf.param(pos);
			starting_point = param_val.u, param_val.v;
			//if(_v_it->idx() == 71) std::cout << "uv_before: " << starting_point << ", pos_before: " << (mesh->vertex(*_v_it)) << "\n";

			auto _SEta_star = [this, &surf](const column_vector &m){return SEta_star(m, surf);};
			auto _SEta_star_derivative = [this, &surf](const column_vector &m){return SEta_star_derivative(m, surf);};

			auto q_before = _SEta_star(starting_point);
			find_min(bfgs_search_strategy(),  // Use BFGS search algorithm
				objective_delta_stop_strategy(1e-7,3), // Stop when the change in Eta_star() is less than 1e-7
				_SEta_star, _SEta_star_derivative, starting_point, 0);
			auto q_after = _SEta_star(starting_point);
			param_val.u = starting_point(0);
			param_val.v = starting_point(1);
			surf.eval(param_val, pos);

			//auto Dis = [pos_old, pos](){return std::sqrt((pos_old.x()-pos.x())*(pos_old.x()-pos.x()) + (pos_old.y()-pos.y())*(pos_old.y()-pos.y()) + (pos_old.z()-pos.z())*(pos_old.z()-pos.z()));};			
			//double curDisplacement = Dis();
			//if(displacement < curDisplacement) displacement = curDisplacement;
			mesh->set_vertex(*_v_it, SPA2POS(pos));
			//if(_v_it->idx() == 71) 
			//{
			//	std::cout << "uv_after: " << starting_point << ", pos_after: " << (mesh->vertex(*_v_it)) << "\n";
			//	std::cout << "SEta_star_before: " << q_before << ", SEta_star_after: " << q_after << '\n';
			//	//return;
			//}
		}
	}

	//if(++count > maxIter) break;
	//}while(displacement > DisEpsilon);
	//std::cout << "Surface: " << count << '\n';
}

void Mesh_optimization::interior_points_optim()
{
	double DisEpsilon = 0.01;
	double displacement;
	int count = 0, maxIter = 20;
	//do{
	displacement = 0;
	for(auto _v_it = mesh->vertices_begin(); _v_it != mesh->vertices_end(); ++_v_it)
	{
		//局部
		if(contains(vhs_local, *_v_it))
			continue;


		// Inner points
		if(!mesh->is_boundary(*_v_it))
		{
			column_vector starting_point(3);
			auto pt = mesh->vertex(*_v_it);
			auto pt_old = pt;
			m_vh = *_v_it;
			starting_point = pt[0], pt[1], pt[2];
			auto _IEta_star = [this](const column_vector &m){return IEta_star(m);};
			auto _IEta_star_derivative = [this](const column_vector &m){return IEta_star_derivative(m);};

			find_min(bfgs_search_strategy(),  // Use BFGS search algorithm
				objective_delta_stop_strategy(1e-7,3), // Stop when the change in Eta_star() is less than 1e-7
				_IEta_star, _IEta_star_derivative, starting_point, 0);

			pt[0] = starting_point(0); pt[1] = starting_point(1); pt[2] = starting_point(2);

			auto Dis = [pt_old, pt](){return std::sqrt((pt_old[0]-pt[0])*(pt_old[0]-pt[0]) + (pt_old[1]-pt[1])*(pt_old[1]-pt[1]) + (pt_old[2]-pt[2])*(pt_old[2]-pt[2]));};			
			double curDisplacement = Dis();

			if(displacement < curDisplacement) displacement = curDisplacement;

			mesh->set_vertex(*_v_it, pt);
		}
	}
	//if(++count > maxIter) break;
	//}while(displacement > DisEpsilon);
	//std::cout << "Inner point: " << count << '\n';
}

bool estimate_orientation(VolumeMesh *mesh) 
{
	int OutCount = 0;
	int InCount = 0;
	bool res;
	// each cell
	for (auto c_it = mesh->cells_begin(); c_it != mesh->cells_end(); c_it++)
	{
		std::map<OvmVeH, std::vector<OvmVeH>> groups;
		std::vector<OvmVeH> vhs, adj_vhs;
		vhs = get_adj_vertices_around_hexa(mesh, *c_it);
		adj_vhs.clear();
		adj_vhs.push_back(vhs[3]), adj_vhs.push_back(vhs[1]), adj_vhs.push_back(vhs[4]);
		groups[vhs[0]] = adj_vhs;
		for (int i = 1; i <= 3; ++i)
		{
			adj_vhs.clear();
			adj_vhs.push_back(vhs[i - 1]), adj_vhs.push_back(vhs[(i + 1) % 4]), adj_vhs.push_back(vhs[8 - i]);
			groups[vhs[i]] = adj_vhs;
		}

		adj_vhs.clear();
		adj_vhs.push_back(vhs[7]), adj_vhs.push_back(vhs[5]), adj_vhs.push_back(vhs[0]);
		groups[vhs[4]] = adj_vhs;

		adj_vhs.clear();
		adj_vhs.push_back(vhs[4]), adj_vhs.push_back(vhs[6]), adj_vhs.push_back(vhs[3]);
		groups[vhs[5]] = adj_vhs;

		adj_vhs.clear();
		adj_vhs.push_back(vhs[5]), adj_vhs.push_back(vhs[7]), adj_vhs.push_back(vhs[2]);
		groups[vhs[6]] = adj_vhs;

		adj_vhs.clear();
		adj_vhs.push_back(vhs[6]), adj_vhs.push_back(vhs[4]), adj_vhs.push_back(vhs[1]);
		groups[vhs[7]] = adj_vhs;

		for (auto cv_it = mesh->cv_iter(*c_it); cv_it; ++cv_it)
		{
			double J[3][3];
			double J_det;
			auto adj_vhs = groups[*cv_it];

			// Get Jacobian Matrix
			OvmPoint3d this_pt = mesh->vertex(*cv_it);
			OvmPoint3d pts[3];

			for (int i = 0; i != 3; ++i)
			{
				pts[i] = mesh->vertex(adj_vhs[i]) - this_pt;
			}

			for (int i = 0; i != 3; ++i)
			{
				for (int j = 0; j != 3; ++j)
				{
					J[i][j] = pts[j][i];
				}
			}

			auto f_det = [&pts]() { return pts[0][0] * (pts[1][1] * pts[2][2] - pts[2][1] * pts[1][2])
				- pts[1][0] * (pts[0][1] * pts[2][2] - pts[0][2] * pts[2][1])
				+ pts[2][0] * (pts[0][1] * pts[1][2] - pts[0][2] * pts[1][1]);
			};
			J_det = f_det();

			if (J_det < 0) ++OutCount;
			if (J_det > 0) ++InCount;
		}
	}

	if (OutCount > InCount)
		res = Outwards;
	else
		res = Inwards;

	return res;
}

void smooth_volume_mesh_yws_inner (VolumeMesh *mesh, BODY *body, int optim_rounds)
	//void optimize_volume_mesh (VolumeMesh *mesh, BODY *body, int optim_rounds)
{	
	auto orientation = estimate_orientation(mesh); // true: inwards, false: outwards
	auto cell_vertex_vertices = preprocess(mesh, orientation);
	Mesh_optimization optimesh(mesh, body, cell_vertex_vertices);
	//unsigned int n_outlier = 0;
	//std::cout << optimesh.quality_evaluation(n_outlier) << std::endl;
	optimesh.quality_evaluation_Jacobian();

	for(int round = 0; round < optim_rounds; ++round){
		//optimesh.boundary_points_optim();
		optimesh.interior_points_optim();
		optimesh.quality_evaluation_Jacobian();
	}
	//std::cout << optimesh.quality_evaluation(n_outlier) << std::endl;
}

void smooth_volume_mesh_yws_all (VolumeMesh *mesh, BODY *body, int optim_rounds)
	//void optimize_volume_mesh (VolumeMesh *mesh, BODY *body, int optim_rounds)
{	
	auto orientation = estimate_orientation(mesh); // true: inwards, false: outwards
	auto cell_vertex_vertices = preprocess(mesh, orientation);
	Mesh_optimization optimesh(mesh, body, cell_vertex_vertices);
	//unsigned int n_outlier = 0;
	//std::cout << optimesh.quality_evaluation(n_outlier) << std::endl;
	optimesh.quality_evaluation_Jacobian();

	for(int round = 0; round < optim_rounds; ++round){
		optimesh.boundary_points_optim();
		optimesh.interior_points_optim();
		optimesh.quality_evaluation_Jacobian();
	}
	//std::cout << optimesh.quality_evaluation(n_outlier) << std::endl;
}