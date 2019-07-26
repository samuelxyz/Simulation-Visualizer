#pragma once
#include "glm/vec2.hpp"

namespace graphics {
	class Window;
}

namespace entity {
	class Entity;
}

struct GLFWwindow;

namespace io {

	class MouseDragTarget;

	class MouseHandler
	{
		graphics::Window& window;
		glm::vec2 prevMousePos;

		// whether to track mouse position
		bool dragging;

		// Whether the mouse has been moved since the last mouse-click-down event
		bool isClickLeft, isClickRight, isClickMiddle;

		MouseDragTarget * leftDragTracked, * rightDragTracked, * middleDragTracked;
		// middleDragTracked is currently unused

	public:
		MouseHandler(graphics::Window&);

		void handleMouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods);
		void handleMouseMotionCallback(GLFWwindow* glfwWindow, double xpos, double ypos);
		void handleScrollCallback(GLFWwindow* glfwWindow, double xoffset, double yoffset);

		entity::Entity* getOrbitingEntity() const;

	private:
		void handleLeftClick(const glm::vec2& mousePos);
		void handleMiddleClick(const glm::vec2& mousePos);
		void handleRightClick(const glm::vec2& mousePos);
		void handleLeftDrag(const glm::vec2& dx);
		void handleMiddleDrag(const glm::vec2& dx);
		void handleRightDrag(const glm::vec2& dx);
		void handleScroll(float, GLFWwindow*);

		// Custom equivalent of ImGui::GetIO().WantCaptureMouse
		static bool guiMouseCheck();

	};

}