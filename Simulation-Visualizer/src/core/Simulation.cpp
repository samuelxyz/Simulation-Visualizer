#include "Simulation.h"

#include "core/Definitions.h"
#include "entity/Entity.h"
#include <imgui/imgui.h>
#include "graphics/content/VisualSphere.h"

#define FPS_TRACKER_SMOOTHING 30

namespace core {

	Simulation::Simulation()
		: entities(), environment(), parameters()
	{
	}

	Simulation::Environment::Environment()
		: floor()
	{
		// floor
		glm::vec4 colorGray(0.8f, 0.8f, 0.8f, 1.0f);
		floor.push_back(graphics::ColoredVertex { graphics::COLOR_WHITE, glm::vec3( 0.0f,  0.0f, -50.0f) });
		floor.push_back(graphics::ColoredVertex { colorGray , glm::vec3(-5.0f, -5.0f, -50.0f) });
		floor.push_back(graphics::ColoredVertex { colorGray , glm::vec3(-5.0f,  5.0f, -50.0f) });
		floor.push_back(graphics::ColoredVertex { colorGray , glm::vec3( 5.0f,  5.0f, -50.0f) });
		floor.push_back(graphics::ColoredVertex { colorGray , glm::vec3( 5.0f, -5.0f, -50.0f) });
	}

	Simulation::Parameters::Parameters()
		: gravityEnabled(false), timePaused(false), elapsedTime(0.0f), 
		showShadows(true), showEnvironment(true)
	{
	}

	Simulation::~Simulation()
	{
		for (entity::Entity* e : entities)
			delete e;
	}

	void Simulation::update()
	{
		if (parameters.timePaused)
			return;

		for (entity::Entity* e : entities)
			e->update(parameters);

		parameters.elapsedTime += core::TIME_STEP;
	}

	void Simulation::render(graphics::Renderer& renderer) const
	{
		if (parameters.showEnvironment)
			renderEnvironment(renderer);
		renderEntities(renderer);
	}

	void Simulation::renderGUI(const graphics::Camera& camera)
	{
		// FPS calculations
		static auto previous = std::chrono::steady_clock::now();

		static unsigned short tickCount = 0;
		tickCount++;

		static float fps = 0;
		if (tickCount >= FPS_TRACKER_SMOOTHING)
		{
			auto now = std::chrono::steady_clock::now();
			auto diff = std::chrono::duration_cast<std::chrono::microseconds>(now - previous).count();
			previous = std::chrono::steady_clock::now();

			fps = 1.0e6f*tickCount / diff;
			tickCount = 0;
		}

		if (ImGui::Begin("Simulation"))
		{
			ImGui::Checkbox("Enable Gravity", &(parameters.gravityEnabled));
			ImGui::Checkbox("Pause Time", &(parameters.timePaused));
			ImGui::Text("Elapsed Time: %.3f s", parameters.elapsedTime);
			ImGui::Text("FPS: %.0f", fps);
			ImGui::Checkbox("Show Shadows", &(parameters.showShadows)); 
			ImGui::Checkbox("Show Environment", &(parameters.showEnvironment));

			ImGui::Text("Show Entity Information:");
			{
				ImGui::BeginChild("Entity Scroll Pane");
				for (entity::Entity* e : entities)
				{
					ImGui::Checkbox(e->getName().c_str(), &(e->shouldShow.gui));
				}
				ImGui::EndChild();
			}
		}
		ImGui::End();

		for (entity::Entity* e : entities)
		{
			if (e->shouldShow.gui)
				e->renderGUI();
			if(e->shouldShow.label)
				e->renderLabel(camera);
		}
	}

	void Simulation::renderGUIOverlay(graphics::Renderer& renderer) const
	{
		for (entity::Entity* e : entities)
		{
			if (e->shouldShow.velocityVector)
				e->renderVelocityVector(renderer);
			if (e->shouldShow.rotationAxis)
				e->renderRotationAxis(renderer);
		}
	}

	void Simulation::add(entity::Entity* e)
	{
		entities.emplace_back(e);
	}

	void Simulation::renderEntities(graphics::Renderer& renderer) const
	{
		for (entity::Entity* e : entities)
		{
			e->render(renderer);
			if (parameters.showShadows)
				e->renderShadow(renderer);
		}
	}

	void Simulation::renderEnvironment(graphics::Renderer& renderer) const
	{
		// floor
		//renderer.submit(environment.floor);
		glm::vec4 squareColor;
		float floorHalfwidth = 5.0f;
		float floorCheckerSize = 1.0f;
		for (float i = -floorHalfwidth; i < floorHalfwidth; i += floorCheckerSize)
		{
			for (float j = -floorHalfwidth; j < floorHalfwidth; j += floorCheckerSize)
			{
				if (std::abs(std::fmodf(i+j, 2.0f)) < 1e-3)
					squareColor = graphics::COLOR_WHITE;
				else
					squareColor = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f); // gray

				renderer.submit(graphics::Quad {{
					{ squareColor, glm::vec3(i, j, core::FLOOR_Z) },
					{ squareColor, glm::vec3(i, j+floorCheckerSize, core::FLOOR_Z) },
					{ squareColor, glm::vec3(i+floorCheckerSize, j+floorCheckerSize, core::FLOOR_Z) },
					{ squareColor, glm::vec3(i+floorCheckerSize, j,core::FLOOR_Z) }
				}});
			}
		}
	}

}