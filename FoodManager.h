#pragma once

#include <igl\opengl\glfw\Viewer.h>
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

				FoodManager();
				void SetViewer(Viewer* scn);
				void MoveAll(double delta_time);
				void AddFood(float speed, Eigen::Vector3f dir, int price, int radious, std::string name);
				bool CollisionDetection(int foodIndx);
				
			private:

				Viewer* scn;
			};

		}
	}
}

