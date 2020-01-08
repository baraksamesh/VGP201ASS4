#pragma once
#include "igl/opengl/glfw/Viewer.h"
#include <igl/circulation.h>
#include <igl/collapse_edge.h>
#include <igl/edge_flaps.h>
#include <igl/shortest_edge_and_midpoint.h>
#include <igl/read_triangle_mesh.h>
#include <igl/opengl/glfw/Viewer.h>
#include <Eigen/Core>
#include <iostream>
#include <set>

namespace igl
{
	namespace opengl
	{
		namespace glfw
		{
			class BetterViewer : public Viewer
			{
			public:
				void load();
				void initData();
				void simplification(int idx);


				bool calc_quads(int mesh_id);
				bool collapse_edge(int idx, Eigen::MatrixXd& V, Eigen::MatrixXi& F);
				void calc_cost_and_position(const int mesh_id, const int e, const Eigen::MatrixXd& V, double& cost, Eigen::Vector3d& p);

				void load_collision();
				void BetterViewer::simplify_V_F(Eigen::MatrixXd& V, Eigen::MatrixXi& F, int idx, float percentage);

			private:
				// everything is vector for each mesh in data_list
				std::vector <Eigen::VectorXi> EMAP;		//connects faces to edges
				std::vector < Eigen::MatrixXi> E;			//edges <index of source vertex, index of destination vertex>
				std::vector < Eigen::MatrixXi> EF;			//connects edges to faces
				std::vector < Eigen::MatrixXi> EI;			//connects edge to vertex index in triangle (0,1,2) 
				typedef std::set<std::pair<double, int> > PriorityQueue;
				std::vector < PriorityQueue> Q;		//priority queue containing cost for every edge
				std::vector < std::vector<PriorityQueue::iterator > > Qit;
				std::vector < Eigen::MatrixXd> C;			//position of the new vertex after collapsing the corresponding edge (in model coordinates)
				std::vector < std::vector <Eigen::Matrix4d> > Quads; //quads of each vertice of each mesh
				std::string to_print;

			};

		}
	}
}