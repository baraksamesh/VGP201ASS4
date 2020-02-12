#include "FoodManager.h"

igl::opengl::glfw::FoodManager::FoodManager()
{

}

void igl::opengl::glfw::FoodManager::SetViewer(Viewer* _scn)
{
	scn = _scn;
}

void igl::opengl::glfw::FoodManager::MoveAll(double delta_time)
{
	for(Food* f : food) {
		f->Move(delta_time);
	}
}

void igl::opengl::glfw::FoodManager::AddFood(float speed, Eigen::Vector3f dir, int price, int radious, std::string name)
{
	scn->load_mesh_from_file(name);
	int idx = scn->selected_data_index;
	food.push_back(new Food(speed, dir.normalized(), price, idx, radious, scn));
}

bool igl::opengl::glfw::FoodManager::CollisionDetection(int foodIndx)
{

	Food* target = NULL;
	int index = 0;
	for(Food * f : food) {
		if (foodIndx == f->mesh_index)
			target = f;
		else
			index++;
	}

	if (target != NULL && scn->collision_detection(-1, foodIndx)) {
		scn->iking = false;	
		target->BeConsumed();
		scn->erase_mesh(foodIndx);
		food.erase(food.begin() + index);
		delete target;
	}


	return false;
}
