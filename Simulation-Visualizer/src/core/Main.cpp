#include <iostream>
#include "graphics/Window.h"
#include "entity/Box.h"

int window()
{
	std::cout << "Hello World!" << std::endl;

	//glm::quat zero = glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	//glm::vec3 euler = glm::eulerAngles(zero);
	//std::cout << "Zero: " << euler.x << ", " << euler.y << ", " << euler.z << std::endl;

	graphics::Window window;

	entity::Box box(glm::vec3(0.0f), glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f)), 
					glm::vec3(0.0f), glm::angleAxis(0.0f, glm::vec3(1.0f, 1.0f, 1.0f)));

	glm::vec4 colorWhite(1.0f);
	glm::vec4 colorGray(0.8f, 0.8f, 0.8f, 1.0f);
	graphics::CenteredPoly floor;
	floor.push_back( graphics::ColoredVertex { colorWhite, glm::vec3( 0.0f, -50.0f,  0.0f) } );
	floor.push_back( graphics::ColoredVertex { colorGray , glm::vec3(-5.0f, -50.0f, -5.0f) } );
	floor.push_back( graphics::ColoredVertex { colorGray , glm::vec3(-5.0f, -50.0f,  5.0f) } );
	floor.push_back( graphics::ColoredVertex { colorGray , glm::vec3( 5.0f, -50.0f,  5.0f) } );
	floor.push_back( graphics::ColoredVertex { colorGray , glm::vec3( 5.0f, -50.0f, -5.0f) } );

	while (!window.shouldClose())
	{
		window.startGUI();

		// simulation stuff
		box.update();
		window.directRender(box);
		window.directRender(floor);

		window.update();
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
