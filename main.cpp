#include "igl/opengl/glfw/renderer.h"
#include "tutorial/sandBox/inputManager.h"

int main(int argc, char* argv[])
{
	Display* disp = new Display(1000, 800, "Wellcome");
	Renderer renderer;
	igl::opengl::glfw::BetterViewer viewer;
	//viewer.load_collision();
	viewer.loadIk(10);
	igl::opengl::glfw::FoodManager fm(&viewer);
	Init(*disp);
	renderer.init(&viewer);
	disp->SetRenderer(&renderer);
	//fm.SetViewer(&viewer);
	disp->SetFoodManager(&fm);
	disp->launch_rendering(true);


	delete disp;
}