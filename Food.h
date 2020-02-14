#pragma once

#include <igl/opengl/Movable.h>
#include <igl\opengl\glfw\Viewer.h>

namespace igl
{
	namespace opengl
	{
		namespace glfw
		{
			class Food
			{
			public:
				int mesh_index;
				Eigen::Vector3f dir;
				float speed;
				int price;
				int radious;
				Viewer* scn;
				float ttl;

				Food(float speed, Eigen::Vector3f dir, int price, int mesh_index, int radious, Viewer* scn, float ttl);
				virtual void Move(double delta_time);
				void BeConsumed();
				float ReduceTTL(double deltaTime);


			private:


			};

		}
	}
}