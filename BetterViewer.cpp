
#include "BetterViewer.h"
#include <igl\vertex_triangle_adjacency.h>
using namespace std;
using namespace Eigen;
using namespace igl;
using namespace igl::opengl::glfw;

//load meshes from configuration.txt file
void BetterViewer::load()
{
	std::ifstream file("configuration.txt");
	std::string line;
	bool is_exe = false;
	while (std::getline(file, line)) {
		load_mesh_from_file(line);
		is_exe = true;
	}
	if (!is_exe) {
		load_mesh_from_file("C:/VGP201/EngineForAnimationCourse/tutorial/data/ycylinder.obj");
		load_mesh_from_file("C:/VGP201/EngineForAnimationCourse/tutorial/data/ycylinder.obj");
		load_mesh_from_file("C:/VGP201/EngineForAnimationCourse/tutorial/data/ycylinder.obj");
		load_mesh_from_file("C:/VGP201/EngineForAnimationCourse/tutorial/data/sphere.obj");
	}
}

// init the data structures for simplifying mesh for each mesh in data_list
void BetterViewer::initData()
{
	MatrixXd V;
	MatrixXi F;

	int data_size = data_list.size();
	E.resize(data_size);
	EMAP.resize(data_size);
	EF.resize(data_size);
	EI.resize(data_size);
	C.resize(data_size);
	Q.resize(data_size);
	Qit.resize(data_size);
	Quads.resize(data_size);
	to_print = "";


	for (int mesh_id = 0; mesh_id < data_list.size(); mesh_id++)
	{
		V = data_list[mesh_id].V;
		F = data_list[mesh_id].F;

		// calc the quadratic errors for each vertex
		Quads[mesh_id].resize(V.rows());
		calc_quads(mesh_id);

		edge_flaps(F, E[mesh_id], EMAP[mesh_id], EF[mesh_id], EI[mesh_id]); // takes the faces of mesh and returns all the other edges info
		Qit[mesh_id].resize(E[mesh_id].rows()); //Q size shall be the amount of edges

		C[mesh_id].resize(E[mesh_id].rows(), V.cols());
		VectorXd costs(E[mesh_id].rows());
		Q[mesh_id].clear();
		for (int e = 0; e < E[mesh_id].rows(); e++) // calculating costs of edges and saving them in Q
		{
			Eigen::Vector3d p(0, 0, 0);
			double cost;
			calc_cost_and_position(mesh_id, e, V, cost, p);
			C[mesh_id].row(e) = p;
			Qit[mesh_id][e] = Q[mesh_id].insert(std::pair<double, int>(cost, e)).first;
		}
		//num_collapsed = 0; mb later
	}

}

void BetterViewer::simplification(int idx)
{
	bool something_collapsed = false;
	int mesh_id = data(idx).id;
	MatrixXd V = data(idx).V;
	MatrixXi F = data(idx).F;
	to_print = "";

	// collapse edge
	const int max_iter = std::ceil(0.05 * Q[mesh_id].size()); //taking 5% of edges or till cant collapse anymore
	for (int j = 0; j < max_iter; j++)
	{
		if (!collapse_edge(mesh_id, V, F))
		{
			break;
		}
		something_collapsed = true;
		//num_collapsed++; still why?
	}

	// update new mesh in data
	if (something_collapsed)
	{
		//cout << to_print;
		data(idx).clear();
		data(idx).set_mesh(V, F);
		data(idx).set_face_based(true);
	}
}

bool BetterViewer::calc_quads(int mesh_id) {
	MatrixXd V = data_list[mesh_id].V;
	MatrixXi F = data_list[mesh_id].F;
	std::vector<std::vector<int> > VF;
	std::vector<std::vector<int> > VFi;

	igl::vertex_triangle_adjacency(V, F, VF, VFi);

	for (int vi = 0; vi < V.rows(); vi++) {
		// find edges with this verticie
		Quads[mesh_id][vi] = Eigen::Matrix4d::Zero();

		for (int fi = 0; fi < VF[vi].size(); fi++) {
			Eigen::Vector3d norm = data_list[mesh_id].F_normals.row(VF[vi][fi]).normalized();
			double d = V.row(vi) * norm;
			double a = norm[0], b = norm[1], c = norm[2];
			d *= -1;

			Eigen::Matrix4d Kp;
			Kp.row(0) = Eigen::Vector4d(a * a, a * b, a * c, a * d);
			Kp.row(1) = Eigen::Vector4d(a * b, b * b, b * c, b * d);
			Kp.row(2) = Eigen::Vector4d(a * c, b * c, c * c, c * d);
			Kp.row(3) = Eigen::Vector4d(a * d, d * b, d * c, d * d);

			Quads[mesh_id][vi] += Kp;
		}
	}
	return true;
}

bool BetterViewer::collapse_edge(int mesh_id, Eigen::MatrixXd& V, Eigen::MatrixXi& F) {
	using namespace Eigen;

	if (Q[mesh_id].empty())
	{
		// no edges to collapse
		return false;
	}
	std::pair<double, int> p = *(Q[mesh_id].begin());
	if (p.first == std::numeric_limits<double>::infinity())
	{
		// min cost edge is infinite cost
		return false;
	}
	Q[mesh_id].erase(Q[mesh_id].begin());

	int e = p.second;
	Qit[mesh_id][e] = Q[mesh_id].end();

	std::vector<int> N = circulation(e, true, EMAP[mesh_id], EF[mesh_id], EI[mesh_id]);
	std::vector<int> Nd = circulation(e, false, EMAP[mesh_id], EF[mesh_id], EI[mesh_id]);
	N.insert(N.begin(), Nd.begin(), Nd.end());

	int vid1 = E[mesh_id].row(e)[0], vid2 = E[mesh_id].row(e)[1];

	int e1, e2, f1, f2;
	Vector3d new_v = C[mesh_id].row(e);
	bool collapsed = igl::collapse_edge(e, C[mesh_id].row(e), V, F, E[mesh_id], EMAP[mesh_id], EF[mesh_id], EI[mesh_id], e1, e2, f1, f2);

	if (collapsed)
	{
		// update the quad of the new vertex		
		Quads[mesh_id][vid1] = Quads[mesh_id][vid1] + Quads[mesh_id][vid2];
		Quads[mesh_id][vid2] = Quads[mesh_id][vid1] + Quads[mesh_id][vid2];

		// Erase the two, other collapsed edges
		Q[mesh_id].erase(Qit[mesh_id][e1]);
		Qit[mesh_id][e1] = Q[mesh_id].end();
		Q[mesh_id].erase(Qit[mesh_id][e2]);
		Qit[mesh_id][e2] = Q[mesh_id].end();
		// update local neighbors
		// loop over original face neighbors
		for (auto n : N)
		{
			if (F(n, 0) != IGL_COLLAPSE_EDGE_NULL ||
				F(n, 1) != IGL_COLLAPSE_EDGE_NULL ||
				F(n, 2) != IGL_COLLAPSE_EDGE_NULL)
			{
				for (int v = 0; v < 3; v++)
				{
					// get edge id
					const int ei = EMAP[mesh_id](v* F.rows() + n);
					// erase old entry
					Q[mesh_id].erase(Qit[mesh_id][ei]);
					// compute cost and potential placement
					double cost;
					Vector3d place;
					calc_cost_and_position(mesh_id, ei, V, cost, place);
					// Replace in queue
					Qit[mesh_id][ei] = Q[mesh_id].insert(std::pair<double, int>(cost, ei)).first;
					C[mesh_id].row(ei) = place;
				}
			}
		}

		ostringstream os;
		os << to_print << "edge " << e << ", cost = " << p.first << ", new v position (" << new_v[0] << "," << new_v[1] << "," << new_v[2] << ")" << endl;
		to_print = os.str();
	}
	else
	{
		// reinsert with infinite weight (the provided cost function must **not**
		// have given this un-collapsable edge inf cost already)
		p.first = std::numeric_limits<double>::infinity();
		Qit[mesh_id][e] = Q[mesh_id].insert(p).first;
	}
	return true;
}

void BetterViewer::calc_cost_and_position(
	const int mesh_id,
	const int e,
	const Eigen::MatrixXd& V,
	double& cost,
	Eigen::Vector3d& p)
{
	int vid1 = E[mesh_id](e, 0), vid2 = E[mesh_id](e, 1);

	Eigen::Matrix4d qsum = Quads[mesh_id][vid1] + Quads[mesh_id][vid2];
	Eigen::Matrix4d qtag = qsum;
	qtag.row(3) = Eigen::Vector4d(0, 0, 0, 1);

	bool invertable;
	Eigen::Vector4d::Scalar det;
	double a;
	qtag.computeInverseAndDetWithCheck(qtag, det, invertable, a);

	Eigen::Vector4d p4(1, 2, 3, 4);
	if (invertable) {
		p4 = qtag * (Eigen::Vector4d(0, 0, 0, 1));
		p[0] = p4[0];
		p[1] = p4[1];
		p[2] = p4[2];
	}
	else {
		p = (V.row(vid1) + V.row(vid2)) / 2;
		p4 << p, 1;
	}

	cost = p4.transpose() * qsum * p4;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
/// COLLISION ////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

void BetterViewer::load_collision()
{
	speed = 0;
	dir = Eigen::Vector3f(-1, 0, 0);
	bool simplify = false;

	MyTranslate(Eigen::Vector3f(0, 0, -0.4));
	load_mesh_from_file("C:/VGP201/EngineForAnimationCourse/tutorial/data/bunny.off");
	data().MyTranslate(Eigen::Vector3f(0.4, 0, 0));
	load_mesh_from_file("C:/VGP201/EngineForAnimationCourse/tutorial/data/bunny.off");
	//data().MyTranslate(Eigen::Vector3f(-0.2	, 0, 0));

	
	trees.resize(data_list.size());

	if (simplify) {
		initData();
		for (int i = 0; i < data_list.size(); i++) {
			std::cout << "simplifying " << i << std::endl;
			Eigen::MatrixXd V = data(i).V;
			Eigen::MatrixXi F = data(i).F;
			simplify_V_F(V, F, i, 0.2f);
			make_tree(V, F, i);
		}
	}
	else {
		for (int i = 0; i < data_list.size(); i++)
			make_tree(data(i).V, data(i).F, i);
	}
}

void BetterViewer::simplify_V_F(Eigen::MatrixXd& V, Eigen::MatrixXi& F, int idx, float percentage)
{
	bool something_collapsed = false;

	// collapse edge
	const int max_iter = std::ceil(percentage * Q[idx].size()); //taking 5% of edges or till cant collapse anymore
	for (int j = 0; j < max_iter; j++)
	{
		if (!collapse_edge(idx, V, F))
		{
			break;
		}
		something_collapsed = true;
		//num_collapsed++; still why?
	}
}