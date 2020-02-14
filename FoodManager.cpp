#include "FoodManager.h"

igl::opengl::glfw::FoodManager::FoodManager(Viewer* _scn)
{
	srand((unsigned)time(0));

	spawner_positions[0] = Eigen::Vector3f(10, 0, 10);
	spawner_positions[1] = Eigen::Vector3f(-10, 0, 10);
	spawner_positions[2] = Eigen::Vector3f(-10, 0, -10);
	spawner_positions[3] = Eigen::Vector3f(10, 0, -10);

	min_velocity = 10;
	max_velocity = 50;
	spawn_rate = 10;
	ttl = 5;

	gravity = 9.8;
	ground = 0;

	scn = _scn;
	//int idx = scn->load_mesh_from_file("C:/Users/barak/Desktop/fuck.off");
	//V = scn->data(idx).V;
	//F = scn->data(idx).F;
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

void igl::opengl::glfw::FoodManager::AddFood(float deltaTime)
{

	if (cool_down > 0) {
		cool_down -= deltaTime;
	}
	else {
		int iSpawner = (rand() % 3);

		int angle = (rand() % 90) - 45;
		Eigen::Vector3f dir = rotateVector(-spawner_positions[iSpawner], angle);
		int elevation = (rand() % 2) + 4;//(rand() % 5)+3;

		//dir[1] = elevation;

		cool_down = spawn_rate;
		int idx = scn->load_mesh_from_file("C:/VGP201/EngineForAnimationCourse/tutorial/data/cube.obj");
		//food.push_back(new Food(10, dir.normalized(), 10, idx, 1, scn, ttl));
		//food.push_back(new BouncyFood(10, dir.normalized(), 10, idx, 1, scn, ttl, elevation*10, gravity, ground));
		
		Eigen::Matrix<float, 4, 3> points;
		/*points << -10, 0, -10,
				-5, 5, -10,
				0, -5, -10,
				5, 0, -10;*/
		/*points << -10, 0, -10,
				-10, 5, -5,
				-10, -5, 0,
				-10, 0, 5;*/
		points << -5, 0, -10,
				0, 5, -10,
				-15, -5, 0,
				0, 0, 0;
		food.push_back(new BazierFood(10, dir.normalized(), 10, idx, 1, scn, ttl, points));
		scn->data(idx).MyTranslate(spawner_positions[iSpawner]);
	}
}

bool igl::opengl::glfw::FoodManager::CollisionDetection(int foodIndx)
{

	Food* target = NULL;
	int index = 0;
	for(Food * f : food) {
		if (scn->iTarget == f->mesh_index) {
			target = f;
			break;
		}
		else {
			index++;
		}
	}

	if (target != NULL && scn->collision_detection(-1, scn->iTarget)) {
		scn->iking = false;	
		scn->fin_rotate();
		target->BeConsumed();
		food.erase(food.begin() + index);
		scn->erase_mesh(scn->mesh_index(scn->iTarget));
		delete target;
		scn->iTarget = -1;

		std::cout << scn->score << std::endl;
	}


	return false;
}

Eigen::Vector3f igl::opengl::glfw::FoodManager::rotateVector(Eigen::Vector3f dir, int angle) {
	float deg2rad = 0.017453292;
	float angel_rad = angle * deg2rad;

	float x = cosf(angel_rad) * dir[0] - sinf(angel_rad) * dir[2];
	float z = sinf(angel_rad) * dir[0] + cosf(angel_rad) * dir[2];


	return Eigen::Vector3f(x, dir[1], z);
}

void igl::opengl::glfw::FoodManager::ClearAllFood() {
	for (Food* f : food) {
		scn->erase_mesh(scn->mesh_index(f->mesh_index));
		delete f;
	}
	food.clear();

}

void igl::opengl::glfw::FoodManager::ResetLevel() {
	level = 0;
}

void igl::opengl::glfw::FoodManager::NextLevel() {
	level++;
}

void igl::opengl::glfw::FoodManager::ReduceAllTTL(float deltaTime) {
	int index = 0;
	float f_ttl = 0;
	for (Food* f : food) {
		f_ttl = f->ReduceTTL(deltaTime);
		if (f_ttl <= 0) {
			scn->erase_mesh(scn->mesh_index(f->mesh_index));
			food.erase(food.begin() + index);
			delete f;
		}
		index++;
	}
}


