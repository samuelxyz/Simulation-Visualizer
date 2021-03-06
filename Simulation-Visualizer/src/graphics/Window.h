#pragma once

#include "io/MouseHandler.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "core/Simulation.h"
#include "graphics/content/VisualBox.h"
#include "graphics/Camera.h"

namespace entity {
	class Entity;
}

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
		struct GUIOverlay
		{
		public:
			GUIOverlay(graphics::Camera& camera);
			~GUIOverlay();

			void render(const Window& window, graphics::Renderer& renderer) const;

		private:
			graphics::Camera& camera;
			static constexpr float axisLength = 0.05f;
			static constexpr float axisThickness = 0.001f;
			mutable std::array<graphics::VisualBox, 3> axes;
			mutable graphics::VisualBox cursorDot;
		} guiOverlay;
		core::Simulation* simulation;
		io::MouseHandler mouseHandler;

	public:
		Window();
		virtual ~Window();

		void setSimulation(core::Simulation*);
		core::Simulation* getSimulation() { return simulation; }
		Camera& getCamera() { return camera; }

		bool shouldClose() const;

		void startGUI();
		void update();
		void render(); // for everything render-related

		void swapBuffers();
		void getDimensions(int* widthOut, int* heightOut);

		static void printGLFWError(int error, const char* description);
		static void APIENTRY printGLDebug(GLenum source, GLenum type, GLuint id,
			GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

		static void handleResize(GLFWwindow*, int width, int height);
		static void handleMouseMotion(GLFWwindow*, double xpos, double ypos);
		static void handleMouseButton(GLFWwindow*, int button, int action, int mods);
		static void handleScroll(GLFWwindow*, double xoffset, double yoffset);

	private:

		// from window user pointer
		static Window* getWindow(GLFWwindow*);

	};

} /* namespace graphics */