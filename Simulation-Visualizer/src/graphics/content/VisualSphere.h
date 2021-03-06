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
		VisualSphere(const glm::vec3& position = glm::vec3(0.0f),
			const glm::quat& orientation = core::QUAT_IDENTITY,
			float radius = 1.0f,
			Style style = Style::MULTICOLOR, const glm::vec4& color = glm::vec4(0.0f),
			float shadeFactor = 0.5f);

		virtual ~VisualSphere();

		virtual void render(graphics::Renderer& renderer) const;
	};
}