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
		// Returns true if it wants to be captured
		virtual bool handleLeftMouseDrag(graphics::Camera&, const glm::vec2&) { return false; }
		virtual bool handleMiddleMouseDrag(graphics::Camera&, const glm::vec2&) { return false; } // currently unused
		virtual bool handleRightMouseDrag(graphics::Camera&, const glm::vec2&) { return false; }
	};

}