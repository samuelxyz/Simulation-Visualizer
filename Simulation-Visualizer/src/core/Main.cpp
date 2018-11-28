#include "graphics/Window.h"
#include "entity/Box.h"
#include "core/Simulation.h"

int window()
{
	graphics::Window window;
	core::Simulation simulation;
	window.setSimulation(&simulation);

	simulation.add(new entity::Box(
		"Box1",
		glm::vec3(0.0f), glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f)), 
		glm::vec3(0.0f), glm::angleAxis(0.0f, glm::vec3(1.0f, 1.0f, 1.0f))
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

int main()
{
	return window();
}
