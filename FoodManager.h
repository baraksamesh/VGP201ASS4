#pragma once

#include <igl\opengl\glfw\Viewer.h>
#include <time.h>
#include "../../../Workspace/tutorial/sandBox/Food.h"

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
				void ClearFood();
				void ResetLevel();
				void NextLevel();
				
			private:

				int level;
				Eigen::Vector3f rotateVector(Eigen::Vector3f dir, int angle);


				Viewer* scn;

				Eigen::Vector3f spawner_positions[4];
				int min_velocity;
				int max_velocity;
				float spawn_rate;

				Eigen::MatrixXd V;
				Eigen::MatrixXi F;

				float cool_down;
			};

		}
	}
}

