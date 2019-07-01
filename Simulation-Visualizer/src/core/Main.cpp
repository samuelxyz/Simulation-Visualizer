#include "core/stdafx.h"

#include "graphics/Window.h"
#include "entity/Box.h"
#include "entity/Sphere.h"
#include "core/Simulation.h"
extern "C" {
#include "path_standalone/Standalone_Path.h"
}

core::Simulation* globalSim;

int window()
{
	graphics::Window window;
	core::Simulation simulation;
	window.setSimulation(&simulation);
	globalSim = &simulation;

	entity::Box* box = new entity::Box("Box1",
		glm::vec3(0.0f, 0.0f, 1.0f), core::QUAT_IDENTITY, glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f)
	);
	simulation.pathSim.setBox(box);
	simulation.add(box);

	//simulation.add(new entity::Sphere("Sphere1",
	//	glm::vec3(3.0f, 3.0f, 1.1f), core::QUAT_IDENTITY, glm::vec3(0.0f), glm::vec3(0.0f)
	//));

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
#ifndef LAUNCH_DIRECT
#ifndef DECLARE_PATH_FUNCS_IN_MAIN
	std::cout << "Enter 'p' or 'path' to run PATH test.\n";
#endif
	std::cout << "Enter 's' or 'scrap' or 'scrapbook' to run the scrapbook.\n";
	std::cout << "Otherwise enter anything to run the visualizer" << std::endl;

	std::string input;
	std::cin >> input;
	const char* cst = input.c_str();

#ifndef DECLARE_PATH_FUNCS_IN_MAIN
	if (_strcmpi(cst, "p") == 0 || _strcmpi(cst, "path") == 0)
#ifdef DO_CUBE_TEST
		return cubeTest();
#else
		return pathTest();
#endif
	else 
#endif
		if (_strcmpi(cst, "s") == 0 || _strcmpi(cst, "scrap") == 0 || _strcmpi(cst, "scrapbook") == 0)
			return scrapbook();
		else
#endif
			return window();
}

#ifdef DECLARE_PATH_FUNCS_IN_MAIN

int funcEval(int n, double* z, double* F)
{
	return globalSim->pathSim.cubeFuncEval(z, F);
}

int jacEval(int n, int nnz, double* z, int* column_starting_indices,
	int* len_of_each_column, int* row_index_of_each_value, double* values)
{
	return globalSim->pathSim.cubeJacEval(z, column_starting_indices, len_of_each_column, row_index_of_each_value, values);
}

#endif