#include "core/stdafx.h"
#include "GUIOverlay.h"
#include "core/Definitions.h"

namespace core {

	GUIOverlay::GUIOverlay(graphics::Camera& camera)
		: camera(camera), axes()
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
	}

	GUIOverlay::~GUIOverlay()
	{
	}

	void core::GUIOverlay::render(graphics::Renderer& renderer) const
	{
		if (!camera.shouldShowAxes())
			return;

		for (auto& axis : axes)
		{
			axis.position = camera.getPosition() +
				camera.getLookVec();
			axis.render(renderer);
		}
	}
}