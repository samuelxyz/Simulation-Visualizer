#include <iostream>
#include "graphics/Window.h"

int pathTest();

int window()
{
	std::cout << "Hello World!" << std::endl;

	graphics::Window window;
	while (!window.shouldClose())
	{
		window.render();
		window.swapBuffers();
		glfwPollEvents();
	}

	return 0;
}

int main()
{
	return pathTest();
}