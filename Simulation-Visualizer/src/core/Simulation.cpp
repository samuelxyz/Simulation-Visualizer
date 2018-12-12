#include "Simulation.h"

#include "core/Definitions.h"
#include "entity/Entity.h"
#include <imgui/imgui.h>

namespace core {

	Simulation::Simulation()
		: entities(), environment(), parameters()
	{
	}

	Simulation::Environment::Environment()
		: floor()
	{
		// floor
		glm::vec4 colorWhite(1.0f);
		glm::vec4 colorGray(0.8f, 0.8f, 0.8f, 1.0f);
		floor.push_back(graphics::ColoredVertex { colorWhite, glm::vec3(0.0f, -50.0f,  0.0f) });
		floor.push_back(graphics::ColoredVertex { colorGray , glm::vec3(-5.0f, -50.0f, -5.0f) });
		floor.push_back(graphics::ColoredVertex { colorGray , glm::vec3(-5.0f, -50.0f,  5.0f) });
		floor.push_back(graphics::ColoredVertex { colorGray , glm::vec3(5.0f, -50.0f,  5.0f) });
		floor.push_back(graphics::ColoredVertex { colorGray , glm::vec3(5.0f, -50.0f, -5.0f) });
	}

	Simulation::Parameters::Parameters()
		: gravityEnabled(false), timePaused(false), elapsedTime(0.0f)
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
		renderEnvironment(renderer);
		renderEntities(renderer);
	}

	void Simulation::renderGUI(glm::mat4& vpMatrix, int windowWidth, int windowHeight)
	{
		if (ImGui::Begin("Simulation"))
		{
			ImGui::Checkbox("Enable Gravity", &(parameters.gravityEnabled));
			ImGui::Checkbox("Pause Time", &(parameters.timePaused));
			ImGui::Text("Elapsed Time: %.3f s", parameters.elapsedTime);

			ImGui::Text("Show Entity Information:");
			{
				ImGui::BeginChild("Entity Scroll Pane");
				for (entity::Entity* e : entities)
				{
					ImGui::Checkbox(e->getName().c_str(), &(e->showGUI));
				}
				ImGui::EndChild();
			}
		}
		ImGui::End();

		for (entity::Entity* e : entities)
		{
			if (e->showGUI)
			{
				e->renderGUI();
				e->renderLabel(vpMatrix, windowWidth, windowHeight);
			}
		}
	}

	void Simulation::add(entity::Entity* e)
	{
		entities.emplace_back(e);
	}

	void Simulation::renderEntities(graphics::Renderer& renderer) const
	{
		for (entity::Entity* e : entities)
			e->render(renderer);
	}

	void Simulation::renderEnvironment(graphics::Renderer& renderer) const
	{
		// floor
		renderer.submit(environment.floor);
	}

}