#include <core/stdafx.h>

#include "graphics/Camera.h"

#include "Entity.h" 
#include "graphics/content/VisualBox.h"
#include "core/Definitions.h"
#include "math/MathUtils.h"

namespace entity {

	Entity::Entity(std::string entityName, glm::vec3 position, glm::quat orientation,
		glm::vec3 velocity, glm::quat angVel,
		float mass, glm::mat3 rotInertia, std::string typeName)
		: entityName(entityName), position(position), orientation(orientation),
		velocity(velocity), angVelocity(angVel),
		mass(mass), rotInertia(rotInertia), typeName(typeName),
		showGUI(false), showRotationAxis(false)
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
		static glm::vec3 linImpulse(0.0f);
		static glm::vec3 angImpulseAxis(1.0f, 0.0f, 0.0f);
		static float angImpulseQuantity;

		if (ImGui::Begin((typeName + ": " + entityName).c_str()))
		{
			ImGui::Text("Mass: %.3f kg", mass);
			//ImGui::Text("Position: (%.3f, %.3f, %.3f)", position.x, position.y, position.z);
			//ImGui::Text("Velocity: (%.3f, %.3f, %.3f)", velocity.x, velocity.y, velocity.z);
			{
				ImGui::Text("Position:");
				ImGui::SameLine();
				ImGui::InputFloat3("m", &position.x);
			}
			{
				ImGui::Text("Velocity:");
				ImGui::SameLine();
				ImGui::InputFloat3("m/s", &velocity.x);
				// next line
				if (ImGui::Button("Zero velocity"))
					velocity *= 0.0f;
			}
			{
				ImGui::Text("Angular velocity: %.3f rad/s", glm::angle(angVelocity));
				ImGui::SameLine();
				if (ImGui::Button("Zero rotation"))
					angVelocity = glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				// next line
				ImGui::Checkbox("Show rotation axis", &showRotationAxis);
			}
			{
				ImGui::Text("Apply impulse:");
				ImGui::InputFloat3("", &linImpulse.x);
				ImGui::SameLine();
				if (ImGui::ArrowButton(("##linImpulse_" + typeName + entityName).c_str(), ImGuiDir_Right))
					applyLinearImpulse(linImpulse);
			}
			{
				ImGui::Text("Apply angular impulse:");
				ImGui::InputFloat("", &angImpulseQuantity);
				ImGui::InputFloat3("", &angImpulseAxis.x);
				ImGui::SameLine();
				if (ImGui::ArrowButton(("##angImpulse_" + typeName + entityName).c_str(), ImGuiDir_Right))
					applyAngularImpulse(glm::normalize(angImpulseAxis) * angImpulseQuantity);
			}
		}
		ImGui::End();
	}

	void Entity::renderLabel(const graphics::Camera& camera) const
	{
		glm::vec2 labelPos = camera.toScreenSpace(position);
		ImGui::SetNextWindowPos(labelPos, 0, glm::vec2(0.5f, 0.5f));
		if (ImGui::Begin(("##label_" + typeName + entityName).c_str(), nullptr,
			ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoInputs |
			ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text((typeName + ": " + entityName).c_str());
			//ImGui::Text("Label position: (%.2f, %.2f, %.2f, %.2f)", screenPos.x, screenPos.y, screenPos.z, screenPos.w);
		}
		ImGui::End();
	}

	void Entity::renderRotationAxis(graphics::Renderer & renderer) const
	{
		static constexpr float thickness = 0.01f, length = 1.0f;

		static graphics::VisualBox axis(position, glm::axis(angVelocity) * length, thickness, glm::vec3(0.0f, 1.0f, 0.0f));
		axis.position = position;
		axis.orientation = math::lookToward(glm::axis(angVelocity));

		axis.render(renderer);
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