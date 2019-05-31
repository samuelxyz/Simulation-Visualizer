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

int pathTest();

int main()
{
	return window();
	//return pathTest();
	
}
