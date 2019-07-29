#pragma once
#include "glm/vec2.hpp"

namespace graphics {
	class Camera;
}

namespace io {

	// currently header-only
	class MouseDragTarget
	{
	public:
		// Return true if this wants to be captured
		// The drag vector will be in glfw screen coordinates (x right, y downward, from top left corner)
		virtual bool handleLeftMouseDrag(graphics::Camera&, const glm::vec2&) { return false; }

		// Return true if this wants to be captured
		// The drag vector will be in glfw screen coordinates (x right, y downward, from top left corner)
		virtual bool handleMiddleMouseDrag(graphics::Camera&, const glm::vec2&) { return false; } // currently unused

		// Return true if this wants to be captured
		// The drag vector will be in glfw screen coordinates (x right, y downward, from top left corner)
		virtual bool handleRightMouseDrag(graphics::Camera&, const glm::vec2&) { return false; }
	};

}