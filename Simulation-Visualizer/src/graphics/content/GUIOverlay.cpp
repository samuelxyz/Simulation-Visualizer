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

		static constexpr float dotSize = axisThickness * 1.01f;
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

		glm::vec3 lookTargetPos = camera.getPosition() + camera.getLookVec();

		for (unsigned int i = 0; i < axes.size(); ++i)
		{
			axes[i].position = lookTargetPos;
			axes[i].render(renderer);

			glm::vec3 labelPos3d(lookTargetPos);
			labelPos3d[i] += axisLength * 1.1f;
			//glm::vec2 labelPos = camera.toScreenSpace(labelPos3d);
			//labelPos.x += 4.0f;

			//ImGui::SetNextWindowPos(labelPos, 0, glm::vec2(0.5f, 0.5f));
			//const char* name;
			//if (i == 0)
			//	name = "x";
			//else if (i == 1)
			//	name = "y";
			//else
			//	name = "z";

			//ImGui::Begin((std::string("##labelAxis") + std::string(name)).c_str(), nullptr,
			//	ImGuiWindowFlags_NoDecoration |
			//	ImGuiWindowFlags_NoInputs |
			//	ImGuiWindowFlags_AlwaysAutoResize | 
			//	ImGuiWindowFlags_NoBackground);
			//if (i == 0)
			//	ImGui::PushStyleColor(ImGuiCol_Text, graphics::COLOR_RED);
			//else if (i == 1)
			//	ImGui::PushStyleColor(ImGuiCol_Text, graphics::COLOR_GREEN);
			//else
			//	ImGui::PushStyleColor(ImGuiCol_Text, graphics::COLOR_BLUE);
			//ImGui::Text(name);
			//ImGui::PopStyleColor();
			//ImGui::End();

			std::string name;
			const glm::vec4* color;
			if (i == 0)
			{
				name = "x";
				color = &(graphics::COLOR_RED);
			}
			else if (i == 1)
			{
				name = "y";
				color = &(graphics::COLOR_GREEN);
			}
			else
			{
				name = "z";
				color = &(graphics::COLOR_BLUE);
			}

			camera.renderLabel(labelPos3d, true, "labelAxis" + name, name, *color, ImVec2(0.47f, 0.5f));
		}

		cursorDot.position = lookTargetPos;
		cursorDot.render(renderer);
	}
}