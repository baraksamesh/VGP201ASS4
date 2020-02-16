#include "BazierFood.h"

igl::opengl::glfw::BazierFood::BazierFood(float speed, Eigen::Vector3f dir, int price, int mesh_index, int radious, Viewer* scn, float ttl,
	Eigen::Matrix <float, 4, 3> points) : igl::opengl::glfw::Food(speed, dir, price, mesh_index, radious, scn, ttl)
{			//			SLIDES 15, 16
	points = points;
	Eigen::Matrix4f	M;					// Blending functions matrix
	M << -1, 3, -3, 1,
		3, -6, 3, 0,
		-3, 3, 0, 0,
		1, 0, 0, 0;

	MG = M * points;
	T << 0, 0, 0, 1;

	t = 0;
	final_dir = (points.row(3) - points.row(2)).normalized();
	drawCurve();
}

void igl::opengl::glfw::BazierFood::Move(double delta_time)	//calculate according to Coefficients approach T(MG)
{
	t += delta_time * speed / 20;
	if (t <= 1) {
		calcT();
		curr_pos = T * MG;
		Eigen::Vector3f tangent = (curr_pos - last_pos).normalized();
		scn->data(mesh_index).LookAt(tangent);
		scn->data(mesh_index).SetTranslation(curr_pos);
	}
	else {
		scn->data(mesh_index).MyTranslate(final_dir * speed * delta_time);
	}
	last_pos = curr_pos;
}

void igl::opengl::glfw::BazierFood::calcT() {
	T[0] = powf(t, 3);
	T[1] = powf(t, 2);
	T[2] = t;
	//T[3] = 1;
}

void igl::opengl::glfw::BazierFood::drawCurve()
{
	//Eigen::RowVector3d color = Eigen::RowVector3d(0, 1, 0);
	//
	//scn->data(mesh_index).add_edges(points.row(0).cast<double>(), points.row(1).cast<double>(), color);
	//scn->data(mesh_index).add_edges(points.row(1).cast<double>(), points.row(2).cast<double>(), color);
	//scn->data(mesh_index).add_edges(points.row(2).cast<double>(), points.row(3).cast<double>(), color);
	//
	//scn->data(mesh_index).line_width = 2;
	//scn->data(mesh_index).show_lines = false;
	//scn->data(mesh_index).show_overlay_depth = false;
}
