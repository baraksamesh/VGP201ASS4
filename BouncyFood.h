#pragma once

#include "../../../Workspace/tutorial/sandBox/Food.h"

namespace igl
{
	namespace opengl
	{
		namespace glfw
		{
			class BouncyFood : public Food
			{
			public:
				
				BouncyFood(float speed, Eigen::Vector3f dir, int price, int mesh_index, int radious, Viewer* scn, float ttl,
							float vertical_speed, float gravity, float ground);
				void Move(double delta_time);
				void strechY();

			private:

				float vertical_speed;
				Eigen::Vector3f curr_velocity;
				float gravity;
				float ground;
			};

		}
	}
}

