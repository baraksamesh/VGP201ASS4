#pragma once
#include "../../../Workspace/tutorial/sandBox/Food.h"

namespace igl
{
	namespace opengl
	{
		namespace glfw
		{
			class BazierFood : public Food
			{
			public:
				BazierFood(float speed, Eigen::Vector3f dir, int price, int mesh_index, int radious, Viewer* scn, float ttl,
					Eigen::Matrix <float, 4, 3> points);

				void Move(double delta_time);

			private:
				float t;
				Eigen::Matrix <float, 4, 3> MG;
				Eigen::RowVector4f T;
				Eigen::Vector3f final_dir;
				Eigen::Vector3f last_pos;
				Eigen::Vector3f curr_pos;

				void calcT();

			};

		}
	}
}
