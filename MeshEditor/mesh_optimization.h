#ifndef __MESH_OPTIMIZATION_H__
#define __MESH_OPTIMIZATION_H__
#define Inwards true
#define Outwards false

#include <dlib/optimization.h>
#include "acis_headers.h"
#include "MeshDefs.h"
#include "FuncDefs.h"

typedef dlib::matrix<double,0,1> column_vector;

class Mesh_optimization{
public:
	Mesh_optimization(VolumeMesh *_mesh, BODY *_body, std::hash_map<OvmCeH, std::map<OvmVeH, std::vector<OvmVeH>>> cell_vertex_vertices);
	~Mesh_optimization(void);
	void boundary_points_optim();
	void interior_points_optim();
	double quality_evaluation(unsigned int &n_outlier);
	double quality_evaluation_Jacobian();
	unsigned int outliercellcount();

private:
	inline double Frobenius_product(double A[][3], double B[][3]) const;
	inline double det(double A[][3]) const;
	double CEta_star(const column_vector &m, const curve &crv);
	const column_vector CEta_star_derivative (const column_vector &m, const curve &crv);
	//dlib::matrix<double> CEta_star_hessian (const column_vector &m, const curve &crv);
	double SEta_star(const column_vector &m, const surface &surf);
	const column_vector SEta_star_derivative (const column_vector &m, const surface &surf);
	//dlib::matrix<double> SEta_star_hessian (const column_vector &m);

	double IEta_star(const column_vector &m);
	const column_vector IEta_star_derivative (const column_vector &m);

	dlib::matrix<double> IEta_star_hessian (const column_vector &m);
	double CalcJacobian(OvmVeH vh, OvmCeH ch, double J[][3]);
	double StdJacobian(OvmVeH vh, OvmCeH ch, double J[][3]);

private:
	VolumeMesh *mesh;
	BODY *body;
	int nv;
	double delta;
	OvmVeH m_vh;
	std::hash_map<OvmCeH, std::map<OvmVeH, std::vector<OvmVeH>>> cell_vertex_vertices;
};

bool estimate_orientation(VolumeMesh *mesh);
std::hash_map<OvmCeH, std::map<OvmVeH, std::vector<OvmVeH>>>preprocess(VolumeMesh *mesh, bool orientation);

void smooth_volume_mesh_yws_inner (VolumeMesh *mesh, BODY *body, int optim_rounds);
void smooth_volume_mesh_yws_all (VolumeMesh *mesh, BODY *body, int optim_rounds);

#endif /* !defined(__MESH_OPTIMIZATION_H__) */