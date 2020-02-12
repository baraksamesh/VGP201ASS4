#include "Food.h"
using namespace std;
using namespace Eigen;
using namespace igl;
using namespace igl::opengl::glfw;

igl::opengl::glfw::Food::Food(float speed, Eigen::Vector3f dir, int price, int mesh_index,int radious, Viewer* scn) :
	speed(speed), dir(dir), price(price), mesh_index(mesh_index), radious(radious), scn(scn)
{
}

void igl::opengl::glfw::Food::Move(double delta_time)
{
	int id = scn->data(mesh_index).id;
	scn->data(mesh_index).MyTranslate(dir * speed * delta_time);
}

void igl::opengl::glfw::Food::BeConsumed()
{
	scn->score += price;
}
