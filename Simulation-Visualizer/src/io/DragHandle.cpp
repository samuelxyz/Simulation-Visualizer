#include "core/stdafx.h"
#include "io/DragHandle.h"
#include "io/DragHandleTarget.h"
#include "graphics/Camera.h"

namespace io {

	DragHandle::DragHandle(DragHandleTarget& target, const glm::vec3 & direction, const glm::vec4 & color)
		: length(0.5f), thickness(0.1f), target(target), direction(direction),
		visualBox(target.getDragHandlePosition(), length*direction, thickness,
			core::VECTOR_UP, graphics::VisualEntity::Style::SOLID_COLOR, color)
	{
	}

	DragHandle::~DragHandle()
	{
	}

	bool DragHandle::handleLeftMouseDrag(graphics::Camera & camera, const glm::vec2& dx)
	{
		//static constexpr float sensitivity = 8e-4f;
		static constexpr float sensitivity = 10e-4f;

		if (glm::length2(direction) < 1e-6f)
			return false;

		glm::vec2 mouseDrag(dx.x, -dx.y); // dx is in glfw screen coords
		glm::vec3 toTarget = target.getDragHandlePosition() - camera.getPosition();
		float dist = glm::length(toTarget);
		glm::vec2 dirOnScreen = camera.toScreenSpace(target.getDragHandlePosition() + (dist * 1e-3f * direction)) -
			camera.toScreenSpace(target.getDragHandlePosition());
		glm::vec3 mouseRay = camera.getMouseRay();

		// if mouseRay perpendicular to direction then: 
		//	result of mouse movement perpendicular to dirOnScreen is zero
		//	result of mouse movement parallel to dirOnScreen is proportional to length(toTarget)
		// if mouseRay is almost parallel to direction then:
		//	result of mouse movement perpendicular to dirOnScreen is zero
		//	result of mouse movement parallel to dirOnScreen is unbounded

		// so overall expression is something like
		// dx = direction * (normalize(dirOnScreen) cdot mouseDrag) * length(toTarget) / length(mouseRay cross direction)
		// lets try it
		//target.applyDragHandleResult(direction * glm::dot(glm::normalize(dirOnScreen), mouseDrag) *
		//	dist / glm::length(glm::cross(/*mouseRay*/camera.getLookVec(), direction)) * sensitivity);

		target.applyDragHandleResult(direction * glm::dot(dirOnScreen, mouseDrag) / glm::length2(dirOnScreen) *
			dist * sensitivity);

		return true;
	}

	bool DragHandle::containsPoint(const glm::vec3 & worldPos, bool useCachedOrientation)
	{
		visualBox.position = target.getDragHandlePosition();
		return visualBox.containsPoint(worldPos, useCachedOrientation);
	}

	void DragHandle::render(graphics::Renderer & renderer) const
	{
		visualBox.position = target.getDragHandlePosition();
		visualBox.render(renderer);
	}

	void DragHandle::setSize(float innerBoundRadius)
	{
		length = innerBoundRadius/1.5f;
		thickness = length/7.5f;
		if (glm::length2(direction) <= 1e-6f)
			thickness *= 1.01f;

		visualBox.xMin = -thickness;
		visualBox.xMax = thickness;
		if (glm::length2(direction) > 1e-6f)
			visualBox.xMax += length;
		visualBox.yMin = -thickness;
		visualBox.yMax = thickness;
		visualBox.zMin = -thickness;
		visualBox.zMax = thickness;
	}
}