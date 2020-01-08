
#include "igl/opengl/glfw/renderer.h"
#include "tutorial/sandBox/inputManager.h"

int main(int argc, char* argv[])
{
	Display* disp = new Display(1000, 800, "Wellcome");
	Renderer renderer;
	igl::opengl::glfw::BetterViewer viewer;

	//viewer.loadIk(4);
	viewer.load_collision();
	Init(*disp);
	renderer.init(&viewer);	
	disp->SetRenderer(&renderer);
	disp->launch_rendering(true);


	delete disp;
}
