#include <core/stdafx.h>

#include "graphics/Camera.h"

#include "Entity.h" 
#include "graphics/content/VisualBox.h"
#include "core/Definitions.h"
#include "math/MathUtils.h"

#define VELOCITY_VECTOR_COLOR glm::vec4(0.0f, 1.0f, 1.0f, 1.0f) // cyan
#define ROTATION_AXIS_COLOR glm::vec4(1.0f, 0.0f, 1.0f, 1.0f) // magenta

namespace entity {

	Entity::Entity(std::string entityName, glm::vec3 position, glm::quat orientation,
		glm::vec3 velocity, glm::quat angVel,
		float mass, glm::mat3 rotInertia, std::string typeName)
		: entityName(entityName), position(position), orientation(orientation),
		velocity(velocity), angVelocity(angVel),
		mass(mass), rotInertia(rotInertia), typeName(typeName),
		shouldShow { false, false, false, false }
	{
	}

	Entity::~Entity()
	{
	}

	glm::vec3 Entity::toLocalFrame(glm::vec3 worldVec) const
	{
		//return glm::inverse(glm::toMat3(orientation)) * (worldVec - position);
		return glm::transpose(glm::toMat3(orientation)) * (worldVec - position);
		// inverse == transpose for rotation matrices
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
			ImGui::Checkbox("Show label", &shouldShow.label);
			ImGui::Text("Mass: %.3f kg", mass);
			ImGui::Text("Inertia matrix: diag(%.3f, %.3f, %.3f) kg*m^2", 
				rotInertia[0][0], rotInertia[1][1], rotInertia[2][2]);
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
				ImGui::PushStyleColor(ImGuiCol_CheckMark, VELOCITY_VECTOR_COLOR);
				ImGui::Checkbox("Show velocity vector", &(shouldShow.velocityVector));
				ImGui::PopStyleColor();
				ImGui::SameLine();
				if (ImGui::Button("Zero velocity"))
					velocity *= 0.0f;
			}
			{
				ImGui::Text("Angular velocity: %.3f rad/s", glm::angle(angVelocity));
				// next line
				ImGui::PushStyleColor(ImGuiCol_CheckMark, ROTATION_AXIS_COLOR);
				ImGui::Checkbox("Show rotation axis  ", &(shouldShow.rotationAxis));
				ImGui::PopStyleColor();
				ImGui::SameLine();
				if (ImGui::Button("Zero rotation"))
					angVelocity = glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
			}
			{
				ImGui::Text("Apply impulse:");
				ImGui::InputFloat3("##linImpulseInput", &linImpulse.x);
				ImGui::SameLine();
				if (ImGui::ArrowButton("##linImpulseButton", ImGuiDir_Right))
					applyLinearImpulse(linImpulse);
			}
			{
				ImGui::Text("Apply angular impulse:");
				ImGui::InputFloat("##angImpulseQuantityInput", &angImpulseQuantity);
				ImGui::InputFloat3("##angImpulseAxisInput", &angImpulseAxis.x);
				ImGui::SameLine();
				if (ImGui::ArrowButton("##angImpulse", ImGuiDir_Right))
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

	void Entity::renderVelocityVector(graphics::Renderer & renderer) const
	{
		static constexpr float thickness = 0.01f;

		static graphics::VisualBox box(position, velocity,
			thickness, glm::vec3(0.0f, 1.0f, 0.0f),
			graphics::VisualBox::Style::SOLID_COLOR, VELOCITY_VECTOR_COLOR);
		box.position = position;
		box.xMax = thickness + glm::length(velocity);
		box.orientation = math::lookToward(velocity);

		box.render(renderer);
	}

	void Entity::renderRotationAxis(graphics::Renderer & renderer) const
	{
		static constexpr float thickness = 0.01f, length = 1.0f;

		static graphics::VisualBox axis(position, glm::axis(angVelocity) * length,
			thickness, glm::vec3(0.0f, 1.0f, 0.0f),
			graphics::VisualBox::Style::SOLID_COLOR, ROTATION_AXIS_COLOR);
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