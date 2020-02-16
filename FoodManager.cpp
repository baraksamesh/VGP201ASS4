#include "FoodManager.h"

igl::opengl::glfw::FoodManager::FoodManager(Viewer* _scn)
{
	srand((unsigned)time(0));

	spawner_positions[0] = Eigen::Vector3f(10, 0, 10);
	spawner_positions[1] = Eigen::Vector3f(-10, 0, 10);
	spawner_positions[2] = Eigen::Vector3f(-10, 0, -10);
	spawner_positions[3] = Eigen::Vector3f(10, 0, -10);

	ttl = 5;

	base_spawn_rate = 1;
	base_chance_of_regular = 4;
	base_chance_of_bouncy = 2;
	base_chance_of_bazier = 1;
	base_speed = 7;

	gravity = 9.8;
	ground = 0;

	scn = _scn;

	ResetLevel();
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
		cool_down = spawn_rate;
		int lucky_num = (rand() % (base_chance_of_regular + chance_of_bouncy + chance_of_bazier));

		if (lucky_num < base_chance_of_regular)
			AddRegularFood(deltaTime);
		else if (lucky_num >= base_chance_of_regular &&
			lucky_num < base_chance_of_regular+ chance_of_bouncy)
			AddBouncyFood(deltaTime);
		else if (lucky_num >= base_chance_of_regular + chance_of_bouncy &&
			lucky_num < base_chance_of_regular + chance_of_bouncy + chance_of_bazier)
			AddBazierFood(deltaTime);
	}
}
void igl::opengl::glfw::FoodManager::AddRegularFood(float deltaTime) {
	int iSpawner = (rand() % 4);
	int angle = (rand() % 90) - 45;
	Eigen::Vector3f dir = rotateVector(-spawner_positions[iSpawner], angle);
	int elevation = (rand() % 2) + 4;
	dir[1] = elevation;

	int idx = scn->load_mesh_from_file("C:/VGP201/EngineForAnimationCourse/tutorial/data/cube.obj");
	food.push_back(new Food(speed, dir.normalized(), 10, idx, 1, scn, ttl));
	
	scn->data(idx).MyTranslate(spawner_positions[iSpawner]);
}


void igl::opengl::glfw::FoodManager::AddBouncyFood(float deltaTime) {
	int iSpawner = (rand() % 4);

	int angle = (rand() % 90) - 45;
	Eigen::Vector3f dir = rotateVector(-spawner_positions[iSpawner], angle);
	int elevation = (rand() % 2) + 4;
	dir[1] = elevation;

	int idx = scn->load_mesh_from_file("C:/VGP201/EngineForAnimationCourse/tutorial/data/cube.obj");
	food.push_back(new BouncyFood(speed, dir.normalized(), 15, idx, 1, scn, ttl, elevation * 10, gravity, ground));

	scn->data(idx).MyTranslate(spawner_positions[iSpawner]);
}

void igl::opengl::glfw::FoodManager::AddBazierFood(float deltaTime) {
	int iSpawner = (rand() % 4);
	float spawnerX = spawner_positions[iSpawner][0];
	float spawnerZ = spawner_positions[iSpawner][2];

	int angle = (rand() % 90) - 45;

	switch (iSpawner) {
	case 0:
		angle += -135;		break;
	case 1:
		angle += -45;		break;
	case 2:
		angle += 45;		break;
	case 3:
		angle += 135;		break;
	}
	
	Eigen::Matrix <float, 4, 3> points = Eigen::Matrix <float, 4, 3>::Zero();

	int curve = (rand() % 4);

	Eigen::Vector4f p0;
	Eigen::Vector4f p1;
	Eigen::Vector4f p2;
	Eigen::Vector4f p3;

	switch (curve) {
	case 0:
		p0 = Eigen::Vector4f(0, 0, 0, 1);
		p1 = Eigen::Vector4f(6, 10, 0, 1);
		p2 = Eigen::Vector4f(12, -1, 0, 1);
		p3 = Eigen::Vector4f(18, 4, 0, 1);
		break;
	case 1:
		p0 = Eigen::Vector4f(0, 0, 0, 1);
		p1 = Eigen::Vector4f(6, 3, -6, 1);
		p2 = Eigen::Vector4f(12, 1, 6, 1);
		p3 = Eigen::Vector4f(18, 3, -6, 1);
		break;
	case 2:
		p0 = Eigen::Vector4f(0, 0, 0, 1);
		p1 = Eigen::Vector4f(15, 2, 0, 1);
		p2 = Eigen::Vector4f(15, 4, 15, 1);
		p3 = Eigen::Vector4f(0, 6, 15, 1);
		break;
	default:
		p0 = Eigen::Vector4f(0, 0, 0, 1);
		p1 = Eigen::Vector4f(0, 2, 15, 1);
		p2 = Eigen::Vector4f(15, 4, 15, 1);
		p3 = Eigen::Vector4f(15, 6, 0, 1);
		break;
	}


	float deg2rad = 0.017453292;
	float angel_rad = angle * deg2rad;

	Eigen::Matrix <float, 3, 4> trans;		//	x rotation and translation to spawner
	trans << cosf(angel_rad), 0, -sinf(angel_rad), spawnerX,
			0, 1, 0, 0,
			sinf(angel_rad), 0, cosf(angel_rad), spawnerZ;

	points.row(0) = trans * p0;
	points.row(1) = trans * p1;
	points.row(2) = trans * p2;
	points.row(3) = trans * p3;

	int idx = scn->load_mesh_from_file("C:/VGP201/EngineForAnimationCourse/tutorial/data/cube.obj");
	food.push_back(new BazierFood(speed, Eigen::Vector3f::Zero(), 20, idx, 1, scn, ttl, points));
	scn->data(idx).MyTranslate(spawner_positions[iSpawner]);
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
	spawn_rate = base_spawn_rate;
	chance_of_regular = base_chance_of_regular;
	chance_of_bouncy = base_chance_of_bouncy;
	chance_of_bazier = base_chance_of_bazier;
	speed = base_speed;
}

void igl::opengl::glfw::FoodManager::NextLevel() {
	level++;
	spawn_rate *= 0.8;
	chance_of_bazier++;
	chance_of_bouncy++;
	speed += 2;
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


