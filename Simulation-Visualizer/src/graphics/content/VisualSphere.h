#pragma once
#include "VisualEntity.h"
#include "core/Definitions.h"

namespace graphics {
	class VisualSphere :
		public VisualEntity
	{
	public:
		float radius;

	public:
		VisualSphere(glm::vec3 position = glm::vec3(0.0f),
			glm::quat orientation = core::QUAT_IDENTITY,
			float radius = 1.0f,
			Style style = Style::MULTICOLOR, glm::vec4 color = glm::vec4(0.0f),
			float shadeFactor = 0.0f);

		virtual ~VisualSphere();

		virtual void render(graphics::Renderer& renderer) const;
	};
}