#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "graphics/Renderable.h"

namespace graphics {

	class Window
	{

	public:
		Window();
		virtual ~Window();

		bool shouldClose() const;
		void render();
		void quickRender(Renderable& renderable);
		void swapBuffers();

		static void printGLFWError(int error, const char* description);
		static void APIENTRY printGLDebug(GLenum source, GLenum type, GLuint id,
			GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

		static void handleKey(GLFWwindow*, int key, int scancode, int action, int mods);
		static void handleMouseButton(GLFWwindow*, int button, int action, int mods);

	private:
		struct Initializer	// unfortunately this workaround is needed because 
		{					// things need to be done before the renderer is constructed
			Initializer();
			GLFWwindow* glfwWindow;
		} initializer;

		GLFWwindow* glfwWindow;
		Renderer renderer;

		static Window* getWindow(GLFWwindow*); // from window user pointer

	};

} /* namespace graphics */