#pragma once
#include "graphics/content/VisualEntity.h"

namespace graphics {

	class VisualBox : public VisualEntity
	{
	public:
		float xMin, xMax, yMin, yMax, zMin, zMax;

	public:
		VisualBox(const glm::vec3& position = glm::vec3(0.0f),
			const glm::quat& orientation = core::QUAT_IDENTITY,
			float xMin = -1.0f, float xMax = 1.0f,
			float yMin = -1.0f, float yMax = 1.0f,
			float zMin = -1.0f, float zMax = 1.0f,
			Style style = Style::MULTICOLOR, const glm::vec4& color = glm::vec4(0.0f),
			float shadeFactor = 0.0f);

		VisualBox(const glm::vec3& start, const glm::vec3& length, 
			float thickness, const glm::vec3& up = core::VECTOR_UP,
			Style style = Style::MULTICOLOR, const glm::vec4& color = glm::vec4(0.0f),
			float shadeFactor = 0.0f);

		virtual ~VisualBox();

		virtual void render(graphics::Renderer& renderer) const override;

		// for DragHandle
		bool containsPoint(const glm::vec3&, bool useCachedOrientation = false) const;
	};

}