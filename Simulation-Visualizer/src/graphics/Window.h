#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "core/Simulation.h"
#include "graphics/Renderable.h"
#include "graphics/Camera.h"
#include "graphics/content/GUIOverlay.h"

namespace graphics {

	class Window
	{
	private:
		static constexpr int
			DEFAULT_WIDTH = 1200,
			DEFAULT_HEIGHT = 800;

		bool initialized;

		struct Initializer	// this workaround is needed because 
		{					// things need to be done before the renderer is constructed
			Initializer();
			GLFWwindow* glfwWindow;
		} initializer;

		GLFWwindow* glfwWindow;
		Renderer renderer;
		Camera camera;
		core::GUIOverlay guiOverlay;
		//glm::mat4 guiVPMatrix;
		core::Simulation* simulation;

		struct MouseTracker
		{
			float prevX, prevY;
			bool dragging;
		} mouseTracker;


	public:
		Window();
		virtual ~Window();

		void setSimulation(core::Simulation*);

		bool shouldClose() const;

		void startGUI();
		void update();
		void render(); // includes our GUI
		void renderGUI(); // ImGui

		void directRender(Renderable& renderable);
		void directRender(const graphics::Triangle& tri);
		void directRender(const graphics::Quad& quad);
		void directRender(const graphics::CenteredPoly& cp);

		void swapBuffers();
		void getDimensions(int* widthOut, int* heightOut);

		static void printGLFWError(int error, const char* description);
		static void APIENTRY printGLDebug(GLenum source, GLenum type, GLuint id,
			GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

		static void handleResize(GLFWwindow*, int width, int height);
		static void handleKey(GLFWwindow*, int key, int scancode, int action, int mods);
		static void handleMouseMotion(GLFWwindow*, double xpos, double ypos);
		static void handleMouseButton(GLFWwindow*, int button, int action, int mods);
		static void handleScroll(GLFWwindow*, double xoffset, double yoffset);

	private:

		static Window* getWindow(GLFWwindow*); // from window user pointer

	};

} /* namespace graphics */