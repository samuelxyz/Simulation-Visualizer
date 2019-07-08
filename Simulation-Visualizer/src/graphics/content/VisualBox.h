#pragma once
#include "graphics/content/VisualEntity.h"

namespace graphics {

	class VisualBox : public VisualEntity
	{
	public:
		float xMin, xMax, yMin, yMax, zMin, zMax;

	public:
		VisualBox(glm::vec3 position = glm::vec3(0.0f),
			glm::quat orientation = core::QUAT_IDENTITY,
			float xMin = -1.0f, float xMax = 1.0f,
			float yMin = -1.0f, float yMax = 1.0f,
			float zMin = -1.0f, float zMax = 1.0f,
			Style style = Style::MULTICOLOR, glm::vec4 color = glm::vec4(0.0f),
			float shadeFactor = 0.0f);

		VisualBox(glm::vec3 start, glm::vec3 length, 
			float thickness, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
			Style style = Style::MULTICOLOR, glm::vec4 color = glm::vec4(0.0f),
			float shadeFactor = 0.0f);

		virtual ~VisualBox();

		virtual void render(graphics::Renderer& renderer) const override;
	};

}