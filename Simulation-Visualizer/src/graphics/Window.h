#pragma once

#include <GL/glew.h>

#include "graphics/Renderable.h"
#include "graphics/Camera.h"

namespace graphics {

	class Window
	{

	public:
		Window();
		virtual ~Window();

		bool shouldClose() const;
		void update();
		void render();
		void directRender(Renderable& renderable);
		void directRender(const graphics::Triangle& tri);
		void directRender(const graphics::Quad& quad);
		void directRender(const graphics::CenteredPoly& cp);
		void swapBuffers();

		static void printGLFWError(int error, const char* description);
		static void APIENTRY printGLDebug(GLenum source, GLenum type, GLuint id,
			GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

		static void handleKey(GLFWwindow*, int key, int scancode, int action, int mods);
		static void handleMouseMotion(GLFWwindow*, double xpos, double ypos);
		static void handleMouseButton(GLFWwindow*, int button, int action, int mods);
		static void handleScroll(GLFWwindow*, double xoffset, double yoffset);

	private:
		bool initialized;

		struct Initializer	// unfortunately this workaround is needed because 
		{					// things need to be done before the renderer is constructed
			Initializer();
			GLFWwindow* glfwWindow;
		} initializer;

		GLFWwindow* glfwWindow;
		Renderer renderer;
		Camera camera;

		struct MouseTracker
		{
			float prevX, prevY;
			bool dragging;
		} mouseTracker;

		static Window* getWindow(GLFWwindow*); // from window user pointer

	};

} /* namespace graphics */