#include <imgui/imgui.h>

#include "Entity.h" 
#include "core/Definitions.h"

namespace entity {

	Entity::Entity(std::string entityName, glm::vec3 position, glm::quat orientation,
		glm::vec3 velocity, glm::quat angVel,
		float mass, glm::mat3 rotInertia, std::string typeName)
		: entityName(entityName), position(position), orientation(orientation),
		velocity(velocity), angVelocity(angVel),
		mass(mass), rotInertia(rotInertia), typeName(typeName),
		showGUI(false)
	{
	}

	Entity::~Entity()
	{
	}

	glm::vec3 Entity::toLocalFrame(glm::vec3 worldVec) const
	{
		return glm::inverse(glm::toMat3(orientation)) * (worldVec - position);
	}

	glm::vec3 Entity::toWorldFrame(glm::vec3 localVec) const
	{
		//return glm::rotate(orientation, localVec) + position;
		return glm::toMat3(orientation) * localVec + position;
	}

	void Entity::renderGUI()
	{
		if (ImGui::Begin((typeName + ": " + entityName).c_str()))
		{
			ImGui::Text("Mass: %.3f kg", mass);
			ImGui::Text("Position: (%.3f, %.3f, %.3f)", position.x, position.y, position.z);
			ImGui::Text("Velocity: (%.3f, %.3f, %.3f)", velocity.x, velocity.y, velocity.z);
		}
		ImGui::End();


	}

	void Entity::renderLabel(glm::mat4& vpMatrix, int windowWidth, int windowHeight) const
	{
		glm::vec4 pos4 = glm::vec4(position, 1.0f);
		//glm::vec4 pos4 = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec4 screenPos = vpMatrix * pos4;

		float labelX = (screenPos.x + 10.0f) * (windowWidth/20.0f);
		float labelY = (-screenPos.y + 10.0f) * (windowHeight/20.0f);

		ImGui::SetNextWindowPos(ImVec2(labelX, labelY), 0, ImVec2(0.5f, 0.5f));
		if (ImGui::Begin(("##label_" + typeName + entityName).c_str(), nullptr,
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoInputs |
			ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text((typeName + ": " + entityName).c_str());
		}
		ImGui::End();
	}

	void Entity::update(core::Simulation::Parameters& parameters)
	{
		if (parameters.gravityEnabled)
			velocity += glm::vec3(0.0f, -core::GRAVITY * core::TIME_STEP, 0.0f);

		position += velocity * core::TIME_STEP;
		orientation = glm::normalize(
			glm::angleAxis(
				glm::angle(angVelocity) * core::TIME_STEP, 
				glm::axis(angVelocity)
			) * orientation
		);

	}

	void Entity::applyLinearImpulse(glm::vec3 impulse)
	{
		velocity += impulse/mass;
	}

	void Entity::applyAngularImpulse(glm::vec3 impulse)
	{
		glm::vec3 delta_omega = toWorldFrame(glm::inverse(rotInertia) * toLocalFrame(impulse));
		float angle = static_cast<float>(delta_omega.length());

		angVelocity = glm::angleAxis(angle, delta_omega) * angVelocity;
	}

	void Entity::applyWrenchImpulse(glm::vec3 worldPos, glm::vec3 impulse)
	{
		applyLinearImpulse(impulse);
		applyAngularImpulse(glm::cross(worldPos - position, impulse));
	}

}