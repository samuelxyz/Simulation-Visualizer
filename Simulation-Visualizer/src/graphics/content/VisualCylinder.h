#pragma once
#include "graphics/content/VisualEntity.h"

namespace graphics {

	class VisualCylinder :
		public VisualEntity
	{
	public:
		float height, radius;

	public:
		VisualCylinder(glm::vec3 position = glm::vec3(0.0f),
			glm::quat orientation = core::QUAT_IDENTITY,
			float radius = 1.0f, float height = 1.0f,
			Style style = Style::MULTICOLOR, glm::vec4 color = glm::vec4(0.0f),
			float shadeFactor = 0.0f);
		virtual ~VisualCylinder();

		virtual void render(graphics::Renderer& renderer) const override;
	};

}