#pragma once
#include "VisualEntity.h"

namespace graphics {
	class VisualSphere :
		public VisualEntity
	{
	public:
		float radius;

	public:
		VisualSphere(glm::vec3 position = glm::vec3(0.0f),
			glm::quat orientation = glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f)),
			float radius = 1.0f,
			Style style = Style::MULTICOLOR, glm::vec4 color = glm::vec4(0.0f));

		virtual ~VisualSphere();

		virtual void render(graphics::Renderer& renderer) const;
	};
}