#pragma once
#include "graphics/Renderable.h"
#include "glm/gtx/quaternion.hpp"

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

	public:
		VisualEntity(glm::vec3 position = glm::vec3(0.0f),
			glm::quat orientation = glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f)),
			Style style = Style::MULTICOLOR, glm::vec4 color = glm::vec4(0.0f)
		);
		virtual ~VisualEntity();

		glm::vec3 toLocalFrame(glm::vec3 worldVec) const;
		glm::vec3 toWorldFrame(glm::vec3 localVec) const;

		virtual void render(graphics::Renderer& renderer) const = 0;
	};

}