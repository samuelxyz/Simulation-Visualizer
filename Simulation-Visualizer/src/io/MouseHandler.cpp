#include "core/stdafx.h"
#include "imgui/imgui_internal.h"
#include "MouseHandler.h"
#include "graphics/Window.h"
#include "entity/Entity.h"

namespace io {

	MouseHandler::MouseHandler(graphics::Window& window)
		: window(window), prevMousePos(0.0f), dragging(false), 
		isClickLeft(false), isClickRight(false), isClickMiddle(false),
		leftDragTracked(nullptr), rightDragTracked(nullptr), middleDragTracked(nullptr)
	{
	}

	void MouseHandler::handleMouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods)
	{
		if (!guiMouseCheck())
		{
			double xd, yd;
			glfwGetCursorPos(glfwWindow, &xd, &yd);
			int windowHeight;
			window.getDimensions(nullptr, &windowHeight);
			glm::vec2 mousePos(static_cast<float>(xd), windowHeight - static_cast<float>(yd));
			if (action == GLFW_PRESS)
			{
				// start tracking dragging
				if (!dragging)
				{
					prevMousePos = mousePos;
					dragging = true;
				}

				// start dragging
				if (button == GLFW_MOUSE_BUTTON_LEFT)
				{
					isClickLeft = true;
					MouseDragTarget* target = window.getSimulation()->getLeftMouseDragTarget(window.getCamera());
					if (target != nullptr)
						leftDragTracked = target;
					else
						leftDragTracked = &(window.getCamera());
				}
				else if (button == GLFW_MOUSE_BUTTON_RIGHT)
				{
					isClickRight = true;
					entity::Entity* focusedEntity = window.getSimulation()->getFocusedEntity(window.getCamera());
					if (focusedEntity != nullptr)
						rightDragTracked = focusedEntity;
					else
						rightDragTracked = &(window.getCamera());
				}
				else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
				{
					isClickMiddle = true;
					// nothing right now
				}
			}
			else // action == GLFW_RELEASE
			{
				if (glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE &&
					glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE &&
					glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_RELEASE)
				dragging = false;

				// remove drag target, and handle clicks if clicks

				if (button == GLFW_MOUSE_BUTTON_LEFT)
				{
					leftDragTracked = nullptr;
					if (isClickLeft)
						handleLeftClick(mousePos);
				}
				else if (button == GLFW_MOUSE_BUTTON_RIGHT)
				{
					rightDragTracked = nullptr;
					if (isClickRight)
						handleRightClick(mousePos);	
				}
				else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
				{
					middleDragTracked = nullptr;
					if (isClickMiddle)
						handleMiddleClick(mousePos);
				}
			}
		}
	}

	void MouseHandler::handleMouseMotionCallback(GLFWwindow* glfwWindow, double xpos, double ypos)
	{
		if (!guiMouseCheck() && dragging)
		{
			int windowHeight;
			window.getDimensions(nullptr, &windowHeight);

			float x = static_cast<float>(xpos);
			float y = windowHeight - static_cast<float>(ypos);

			float dx = x - prevMousePos.x;
			float dy = y - prevMousePos.y;

			if (glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			{
				isClickLeft = false;
				handleLeftDrag(glm::vec2(dx, dy));
			}
			if (glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			{
				isClickRight = false;
				handleRightDrag(glm::vec2(dx, dy));
			}
			if (glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
			{
				isClickMiddle = false;
				handleMiddleDrag(glm::vec2(dx, dy));
			}

			prevMousePos.x = x;
			prevMousePos.y = y;
		}
	}

	void MouseHandler::handleScrollCallback(GLFWwindow* glfwWindow, double xoffset, double yoffset)
	{
		if (!guiMouseCheck())
		{
			handleScroll(static_cast<float>(yoffset), glfwWindow);
		}
	}

	entity::Entity * MouseHandler::getOrbitingEntity() const
	{
		return dynamic_cast<entity::Entity*>(rightDragTracked);
	}

	void MouseHandler::handleLeftClick(const glm::vec2& mousePos)
	{
		const entity::Entity* e = window.getSimulation()->getHoveredEntity(window.getCamera());
		if (e != nullptr)
			e->shouldShow.gui = !(e->shouldShow.gui);
	}

	void MouseHandler::handleMiddleClick(const glm::vec2& mousePos)
	{
		window.getSimulation()->startStop();
	}

	void MouseHandler::handleRightClick(const glm::vec2& mousePos)
	{
		// nothing right now
	}

	void MouseHandler::handleLeftDrag(const glm::vec2& dx)
	{
		if (leftDragTracked != nullptr)
			leftDragTracked->handleLeftMouseDrag(window.getCamera(), dx);
	}

	void MouseHandler::handleMiddleDrag(const glm::vec2& dx)
	{
		// nothing right now
	}

	void MouseHandler::handleRightDrag(const glm::vec2& dx)
	{
		if (rightDragTracked != nullptr && !rightDragTracked->handleRightMouseDrag(window.getCamera(), dx))
			window.getCamera().orbit(window.getCamera().getPosition() + window.getCamera().getLookVec(), dx.x, dx.y);
	}

	void MouseHandler::handleScroll(float yoffset, GLFWwindow* glfwWindow)
	{
		if (!window.getCamera().handleScroll(static_cast<float>(yoffset), glfwWindow,
			getOrbitingEntity()))
		{
			// plain scrolling scrolls the simulation timeline
			window.getSimulation()->scrollTimeline(static_cast<int>(yoffset));
		}
	}

	bool MouseHandler::guiMouseCheck()
	{
		if (ImGui::GetIO().WantCaptureMouse)
		{
			if (ImGui::GetCurrentContext()->HoveredWindow != nullptr &&
				(ImGui::GetCurrentContext()->HoveredWindow->Flags & ImGuiWindowFlags_NoBackground))
				// we are hovering over a label, doesn't count
				return false;
			else if (ImGui::GetCurrentContext()->ActiveIdWindow != nullptr &&
				(ImGui::GetCurrentContext()->ActiveIdWindow->Flags & ImGuiWindowFlags_NoBackground))
				// same here
				return false;
			else
				return true;
		}
		return false;
	}

}