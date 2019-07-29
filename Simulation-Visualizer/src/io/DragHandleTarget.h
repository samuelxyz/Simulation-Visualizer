#pragma once
#include "glm/vec3.hpp"

namespace io {
	class DragHandleTarget
	{
	public:
		virtual glm::vec3 getDragHandlePosition() const = 0;
		virtual void applyDragHandleResult(const glm::vec3&) = 0;
		virtual bool isDragHandleVisible() const = 0;
	};
}