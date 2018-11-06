#include <iostream>
#include "graphics/Window.h"
#include "entity/Box.h"

int window()
{
	std::cout << "Hello World!" << std::endl;

	graphics::Window window;
	entity::Box box(glm::vec3(0.0f), glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f)), 
					glm::vec3(0.0f), glm::angleAxis(0.01f, glm::vec3(1.0f, 1.0f, 1.0f)));
	while (!window.shouldClose())
	{
		box.update();
		window.quickRender(box);
		window.render();
		window.swapBuffers();
		glfwPollEvents();
	}

	return 0;
}

int main()
{
	return window();
}
