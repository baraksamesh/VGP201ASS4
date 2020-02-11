#include "igl/opengl/glfw/renderer.h"
#include "tutorial/sandBox/inputManager.h"

int main(int argc, char* argv[])
{
	Display* disp = new Display(1000, 800, "Wellcome");
	igl::opengl::glfw::FoodManager fm;
	Renderer renderer;
	igl::opengl::glfw::BetterViewer viewer;

	viewer.loadIk(10);
	//viewer.load_collision();
	Init(*disp);
	renderer.init(&viewer);
	disp->SetRenderer(&renderer);
	fm.SetViewer(&viewer);
	fm.AddFood(0.2, Eigen::Vector3f(1, 0, 0), 10, 1, "C:/VGP201/EngineForAnimationCourse/tutorial/data/xcylinder.obj");
	fm.AddFood(0.2, Eigen::Vector3f(-1, 0, 1), 10, 1, "C:/VGP201/EngineForAnimationCourse/tutorial/data/zcylinder.obj");
	disp->SetFoodManager(&fm);
	disp->launch_rendering(true);


	delete disp;
}