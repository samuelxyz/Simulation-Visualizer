#include "graphics/Window.h"

#include <cassert>
#include <iostream>

namespace graphics {

	Window::Window()
	{
		glfwSetErrorCallback(printGLFWError);

		/* Initialize the library */
		assert(glfwInit());

		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

		/* Create a windowed mode window and its OpenGL context */
		glfwWindow = glfwCreateWindow(
			800, 600,
			"Simulation-Visualizer", nullptr, nullptr);

		if (!glfwWindow)
		{
			glfwTerminate();
			assert(false && "GLFW window creation failed");
		}

		glfwSetWindowUserPointer(glfwWindow, this);

		glfwSetKeyCallback(glfwWindow, handleKey);
		glfwSetMouseButtonCallback(glfwWindow, handleMouseButton);

		glfwSetWindowPos(glfwWindow, 50, 50);
		glfwShowWindow(glfwWindow);

		/* Make the window's context current */
		glfwMakeContextCurrent(glfwWindow);

		/* Set refresh rate to once per frame (60 Hz) */
		glfwSwapInterval(1);

		/* Initialize GLEW */
		assert(glewInit() == GLEW_OK);

		std::cout << "Running in debug mode with OpenGL version " <<
			glGetString(GL_VERSION) << std::endl;
		glDebugMessageCallback(printGLDebug, nullptr);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_POLYGON_SMOOTH);

		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1);

		glEnable(GL_MULTISAMPLE);

		glClearColor(0.2f, 0.4f, 0.3f, 1.0f);
	}

	Window::~Window()
	{
		glfwTerminate();
	}

	bool Window::shouldClose() const
	{
		return glfwWindowShouldClose(glfwWindow);
	}

	void Window::render() const
	{
		// temporary debug triangle using immediate mode
		glClear(GL_COLOR_BUFFER_BIT);
		glBegin(GL_TRIANGLES);
		glVertex2f(-0.5f, -0.5f);
		glVertex2f( 0.0f,  0.5f);
		glVertex2f( 0.5f, -0.5f);
		glEnd();
	}

	void Window::swapBuffers() const
	{
		glfwSwapBuffers(glfwWindow);
	}

	void Window::printGLFWError(int error, const char* description)
	{
		std::cerr << "[GLFW Error]" << description << std::endl;
	}

	void APIENTRY Window::printGLDebug(GLenum source,
		GLenum type, GLuint id, GLenum severity, GLsizei length,
		const GLchar* message, const void* userParam)
	{
		std::cout << "[GL] " << message << std::endl;
	}

	void Window::handleKey(GLFWwindow*, int key, int scancode, int action,
		int mods)
	{
		if (action == GLFW_PRESS)
			std::cout << "Key pressed! " << "(" << key << ")" << std::endl;
		else if (action == GLFW_REPEAT)
			std::cout << "Key repeated! " << "(" << key << ")" << std::endl;
		else if (action == GLFW_RELEASE)
			std::cout << "Key released! " << "(" << key << ")" << std::endl;
	}

	void Window::handleMouseButton(GLFWwindow* glfwWindow, int button, int action,
		int mods)
	{
		Window* window = getWindow(glfwWindow);

		// TODO
	}

	Window* Window::getWindow(GLFWwindow* glfwWindow)
	{
		return (Window*)glfwGetWindowUserPointer(glfwWindow);
	}

} /* namespace graphics */