#include "core/stdafx.h"

#include "graphics/Window.h"
#include "entity/Box.h"
#include "entity/Sphere.h"
#include "core/Simulation.h"

int window()
{
	graphics::Window window;
	core::Simulation simulation;
	window.setSimulation(&simulation);

	simulation.add(new entity::Box("Box1",
		glm::vec3(0.0f), core::QUAT_ZERO, glm::vec3(0.0f), core::QUAT_ZERO
	));

	simulation.add(new entity::Sphere("Sphere1",
		glm::vec3(3.0f, 3.0f, 0.0f), core::QUAT_ZERO, glm::vec3(0.0f), core::QUAT_ZERO
	));

	while (!window.shouldClose())
	{
		window.startGUI();

		window.update();
		window.render();
		window.renderGUI();

		window.swapBuffers();
		glfwPollEvents();
	}

	return 0;
}

#ifdef DO_CUBE_TEST
int cubeTest();
#else
int pathTest();
#endif

int scrapbook();

int main()
{
	std::cout << "Enter 'p' or 'path' to run PATH test.\n";
	std::cout << "Enter 's' or 'scrap' or 'scrapbook' to run the scrapbook.\n";
	std::cout << "Otherwise enter anything to run the visualizer" << std::endl;

	std::string input;
	std::cin >> input;
	const char* cst = input.c_str();

	if (_strcmpi(cst, "p") == 0 || _strcmpi(cst, "path") == 0)
#ifdef DO_CUBE_TEST
		return cubeTest();
#else
		return pathTest();
#endif
	else if (_strcmpi(cst, "s") == 0 || _strcmpi(cst, "scrap") == 0 || _strcmpi(cst, "scrapbook") == 0)
		return scrapbook();
	else
		return window();
}
