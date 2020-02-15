#include "BouncyFood.h"
#include "BouncyFood.h"
#include "BouncyFood.h"


igl::opengl::glfw::BouncyFood::BouncyFood(float speed, Eigen::Vector3f dir, int price, int mesh_index, int radious, Viewer* scn, float ttl,
											float vertical_speed, float gravity, float ground) : igl::opengl::glfw::Food(speed, dir, price, mesh_index, radious, scn, ttl),
	 vertical_speed(vertical_speed), gravity(gravity), ground(ground)
{
	dir[1] = 0;	//mute the vertical component of dir so vertical movementd would be affected only by "gravity"
	curr_velocity = dir * speed;
	curr_velocity[1] = vertical_speed;
	Eigen::Vector3f tmp_dir = dir;
	tmp_dir[1] = 0;
	scn->data(mesh_index).LookAt(tmp_dir);
}

void igl::opengl::glfw::BouncyFood::Move(double delta_time)
{
	//delta_time /= 2;
	scn->data(mesh_index).MyTranslate(curr_velocity * delta_time);
	curr_velocity[1] -= gravity * delta_time * 17;
	strechY();
	float dist_from_ground = scn->data(mesh_index).GetTranslation()[1] - ground;
	if (dist_from_ground <= 0) {
		scn->data(mesh_index).MyTranslate(Eigen::Vector3f(0,-dist_from_ground,0));
		curr_velocity[1] = vertical_speed;
	}
}

void igl::opengl::glfw::BouncyFood::strechY() {
	float speed = abs(curr_velocity[1]);
	float strech_factor = ((speed)/27 > 1) ? (speed)/27 : 1;
	float height = scn->data(mesh_index).GetTranslation()[1] - ground;
	height = height < 0.5 ? 0.5 : height;
	float squeeze_factor = (height > 1) ? 1 : (height) * 0.7;
	scn->StrechCubeY(mesh_index, strech_factor * squeeze_factor);
}
