#pragma once
#include "graphics/Renderable.h"
#include "core/Definitions.h"

namespace graphics {

	class VisualEntity : public Renderable
	{
	public:
		glm::vec3 position;
		glm::quat orientation;
		enum class Style {
			MULTICOLOR,
			SOLID_COLOR
		} style;
		glm::vec4 color; // only used for Style::SOLID_COLOR
		float shadeFactor;

	public:
		VisualEntity(glm::vec3 position = glm::vec3(0.0f),
			glm::quat orientation = core::QUAT_IDENTITY,
			Style style = Style::MULTICOLOR, glm::vec4 color = glm::vec4(0.0f),
			float shadeFactor = 0.0f
		);
		virtual ~VisualEntity();

		glm::vec3 toLocalFrame(glm::vec3 worldVec, bool useCachedOrientation = false) const;
		glm::vec3 toWorldFrame(glm::vec3 localVec, bool useCachedOrientation = false) const;

		virtual void render(graphics::Renderer& renderer) const = 0;
	};

}