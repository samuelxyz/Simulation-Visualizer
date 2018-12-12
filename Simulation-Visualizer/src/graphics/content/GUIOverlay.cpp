#include "core/stdafx.h"
#include "GUIOverlay.h"
#include "core/Definitions.h"

namespace core {

	GUIOverlay::GUIOverlay(graphics::Camera& camera)
		: camera(camera), axes(), cursorDot()
	{
		for (auto& axis: axes)
		{
			axis.style = graphics::VisualEntity::Style::SOLID_COLOR;
			axis.xMin = -axisThickness;
			axis.xMax = axisLength + axisThickness;
			axis.yMin = -axisThickness;
			axis.yMax = axisThickness;
			axis.zMin = -axisThickness;
			axis.zMax = axisThickness;
		}

		axes[0].color = graphics::COLOR_RED;
		axes[1].color = graphics::COLOR_GREEN;
		axes[2].color = graphics::COLOR_BLUE;

		axes[1].orientation = glm::angleAxis(core::HALF_PI, glm::vec3(0.0f, 0.0f, 1.0f));
		axes[2].orientation = glm::angleAxis(core::HALF_PI, glm::vec3(0.0f, -1.0f, 0.0f));

		static constexpr float dotSize = axisThickness * 1.01;
		cursorDot.style = graphics::VisualEntity::Style::SOLID_COLOR;
		cursorDot.xMin = -dotSize;
		cursorDot.xMax =  dotSize;
		cursorDot.yMin = -dotSize;
		cursorDot.yMax =  dotSize;
		cursorDot.zMin = -dotSize;
		cursorDot.zMax =  dotSize;
		cursorDot.color = graphics::COLOR_WHITE;
	}

	GUIOverlay::~GUIOverlay()
	{
	}

	void core::GUIOverlay::render(graphics::Renderer& renderer) const
	{
		if (!camera.shouldShowAxes())
			return;

		glm::vec3 cursorPos = camera.getPosition() + camera.getLookVec();

		for (auto& axis : axes)
		{
			axis.position = cursorPos;
			axis.render(renderer);
		}

		cursorDot.position = cursorPos;
		cursorDot.render(renderer);
	}
}