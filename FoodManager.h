#pragma once

#include <igl\opengl\glfw\Viewer.h>
#include <time.h>
#include "../../../Workspace/tutorial/sandBox/Food.h"
#include "../../../Workspace/tutorial/sandBox/BouncyFood.h"
#include "../../../Workspace/tutorial/sandBox/BazierFood.h"

namespace igl
{
	namespace opengl
	{
		namespace glfw
		{
			class FoodManager
			{
			public:
				std::vector <igl::opengl::glfw::Food*> food;

				//FoodManager();
				FoodManager(Viewer* _scn);
				void SetViewer(Viewer* scn);
				void MoveAll(double delta_time);
				void AddFood(float deltaTime); // float speed, Eigen::Vector3f dir, int price, int radious, std::string name);
				bool CollisionDetection(int foodIndx);
				void ClearAllFood();
				void ResetLevel();
				void NextLevel();
				void ReduceAllTTL(float deltaTime);
				
			private:

				int level;
				float gravity;
				float ground;

				float base_spawn_rate;
				int base_chance_of_regular;
				int base_chance_of_bouncy;
				int base_chance_of_bazier;
				float base_speed;

				float cool_down;
				float spawn_rate;
				int chance_of_regular;
				int chance_of_bouncy;
				int chance_of_bazier;
				float speed;


				Viewer* scn;
				Eigen::Vector3f spawner_positions[4];
				float ttl;

				Eigen::MatrixXd V;
				Eigen::MatrixXi F;

				void AddRegularFood(float deltaTime);
				void AddBouncyFood(float deltaTime);
				void AddBazierFood(float deltaTime);
				Eigen::Vector3f rotateVector(Eigen::Vector3f dir, int angle);
			};

		}
	}
}

