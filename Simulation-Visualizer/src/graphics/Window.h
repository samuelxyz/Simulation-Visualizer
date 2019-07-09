#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "core/Definitions.h"
#include "core/Simulation.h"
#include "graphics/Renderable.h"
#include "graphics/Camera.h"
#include "graphics/content/VisualBox.h"

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
			virtual ~GUIOverlay();

			virtual void render(const Window& window, graphics::Renderer& renderer) const;

		private:
			graphics::Camera& camera;
			static constexpr float axisLength = 0.05f;
			static constexpr float axisThickness = 0.001f;
			mutable std::array<graphics::VisualBox, 3> axes;
			mutable graphics::VisualBox cursorDot;
		} guiOverlay;
		//glm::mat4 guiVPMatrix;
		core::Simulation* simulation;

		struct MouseTracker
		{
			float prevX, prevY;
			bool dragging;
			const entity::Entity* orbitingEntity;

			// Whether the mouse has been moved since the last mouse-click-down event
			bool isJustClickAndNotDrag;

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

		// from window user pointer
		static Window* getWindow(GLFWwindow*);

		// Custom equivalent of ImGui::GetIO().WantCaptureMouse
		static bool guiMouseCheck(GLFWwindow*);

	};

} /* namespace graphics */