#pragma once
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "graphics/content/VisualBox.h"
#include "io/MouseDragTarget.h"
namespace graphics {
	class Camera;
}

namespace io {

	class DragHandleTarget;

	// DragHandle with direction 0 is special and does nothing, essentially visual only
	struct DragHandle : public MouseDragTarget
	{
		DragHandle(DragHandleTarget& target, const glm::vec3& direction, const glm::vec4& color);
		virtual ~DragHandle();

		virtual bool handleLeftMouseDrag(graphics::Camera& camera, const glm::vec2& dx) override;
		bool containsPoint(const glm::vec3& worldPos, bool useCachedOrientation = false);

		void render(graphics::Renderer&) const;

		void setSize(float innerBoundRadius);

		float length;
		float thickness;
		DragHandleTarget& target;
		glm::vec3 direction;
		mutable graphics::VisualBox visualBox;
		// use target.getPosition() as this->position
	};

}