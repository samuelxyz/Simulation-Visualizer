#pragma once
#include "graphics/content/VisualEntity.h"

namespace graphics {

	class VisualBox : public VisualEntity
	{
	public:
		float xMin, xMax, yMin, yMax, zMin, zMax;

	public:
		VisualBox(glm::vec3 position = glm::vec3(0.0f),
			glm::quat orientation = glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f)),
			float xMin = -1.0f, float xMax = 1.0f,
			float yMin = -1.0f, float yMax = 1.0f,
			float zMin = -1.0f, float zMax = 1.0f);

		virtual ~VisualBox();

		virtual void render(graphics::Renderer& renderer) const;
	};

}